#include "ti_msp_dl_config.h"
#include "oled_spi.h"
#include "AD9910.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define CABLE_VELOCITY (2.0/3.0 * 299792458)  // 电缆中信号传播速度
#define COARSE_STEPS   50      // 粗扫步数
#define FINE_STEPS     100     // 精扫步数
#define VOLTAGE_THRESHOLD  0.3 // 电压变化阈值 (需要根据实际校准)

uint8_t choice  =0;
uint8_t mode =1;//初始应该为0！！！
//uart数据接收变量
u8 uart_data[30];
u8 warning_data = 0;
//接收数据标志位
volatile bool gCheckADC;        //ADC采集成功标志位
unsigned int adc_getValue(void);//读取ADC的数据

// 全局测量结果存储
typedef struct {
    float coarse_freq;
    float fine_freq;
    float length;
    char type;//电容——C；电阻——R
    float value;
} MeasurementResult;//测量结果结构体
MeasurementResult cable_measurement;//测量结果变量

//函数定义
float FreqSweep(uint32_t start_freq, uint32_t end_freq,  uint32_t steps, uint8_t find_min);
uint8_t MeasureCableLength(void);
void SendData(void);
void Reaction(void);

// 主函数整合
int main(void) {
    SYSCFG_DL_init();
    //通信初始化
	NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN );//清除中断标志位
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
    //adc初始化
    NVIC_ClearPendingIRQ(ADC_Volt_INST_INT_IRQN );//清除中断标志位
    NVIC_EnableIRQ(ADC_Volt_INST_INT_IRQN);
    //定时器初始化
    NVIC_ClearPendingIRQ(TIMER_0_INST_INT_IRQN );//清除中断标志位
    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
    

    // 外设初始化
    OLED_Init();
    delay_ms(1000);
    Init_AD9910();

    // 初始参数
    AD9910_AmpWrite(10383); // 设置幅度
    AD9910_FreWrite(10000); // 设置频率
    delay_ms(3000);
    while(1) {
        switch (mode) {//根据模式选择测量
            case 1:// 测量模式1,长度测量
                MeasureCableLength();// 测量电缆长度
                break;
            case 2:// 测量模式2,终端负载测量

                break;
            default:
                break;
        }
        delay_ms(3000); // 等待时间,最后记得去掉
        if(mode) {
            SendData();// 发送数据
            mode = 0; // 重置模式
        }
    }
}

// 改进的扫频函数
float FreqSweep(uint32_t start_freq, uint32_t end_freq,  uint32_t steps, uint8_t find_min) {
    float extremum_value = find_min ? 1.0f : 0.0f;// 初始值
    float extremum_freq = 0.0f;// 初始频率
    const float log_start = log10f(start_freq);// 计算对数频率
    const float log_end = log10f(end_freq);// 计算对数频率
    const float log_step = (log_end - log_start)/steps;// 计算对数步长

    // 频率扫描
    for(uint32_t i = 0; i <= steps; ++i) {
        const uint32_t freq = (uint32_t)powf(10.0f, log_start + i*log_step);// 计算当前频率
        
        AD9910_FreWrite(freq);      // 设置频率
        delay_us(500);              // 稳定时间
        
        float voltage = adc_getValue()/4095.0 * 3.3;// 读取电压值
        
        // 寻找极值点
        if((find_min && (voltage < extremum_value)) || (!find_min && (voltage > extremum_value))) {
            extremum_value = voltage;
            extremum_freq = freq;
        }
    }
    return extremum_freq;
}

// 分阶段测量函数
uint8_t MeasureCableLength(void) {
    // 第一阶段：粗扫寻找1/4波长波节（电压最小值）
    cable_measurement.coarse_freq = FreqSweep(2500000,50000000,COARSE_STEPS,1);// 2.5MHz到50MHz,步数50，找最小值模式

    // 有效性检查
    if(cable_measurement.coarse_freq < 2.6e6 || cable_measurement.coarse_freq > 50e6) {
        return 0; // 测量失败
    }

    // 第二阶段：精扫寻找1/2波长波腹（电压最大值）
    const uint32_t fine_band = cable_measurement.coarse_freq * 0.1; // ±10%带宽

    cable_measurement.fine_freq = FreqSweep(
        cable_measurement.coarse_freq - fine_band,//扫描区域下界
        cable_measurement.coarse_freq + fine_band,//扫描区域上界
        FINE_STEPS,                               // 精细扫描步数
        0                                         // 寻找最大值
    );

    // 计算两种模式下的长度
    const float lambda1 = CABLE_VELOCITY / cable_measurement.coarse_freq;// 波长1
    const float length1 = lambda1 / 4.0f;//长度1

    const float lambda2 = CABLE_VELOCITY / cable_measurement.fine_freq;// 波长2
    const float length2 = lambda2 / 2.0f;//长度2

    // 取加权平均（可根据实际情况调整权重）
    cable_measurement.length = (length1 * 0.4f) + (length2 * 0.6f);

    return 1; // 测量成功
}

void SendData(void){
	char str[30]={};
	char *pt = str;
    if(mode == 1){
        //长度结果发送
        sprintf(str,"t3.txt=\"%.0f\"\xff\xff\xff",cable_measurement.length*100);//长度单位转换为cm
        while(*pt){
            DL_UART_transmitDataBlocking(UART_0_INST,*pt++);
            delay_us(100);
        }
    }
        
    else if(mode == 2){
        //终端负载数据发送
        pt = str;
        delay_ms(1);
        sprintf(str,"t8.txt=\"%c\"\xff\xff\xff",cable_measurement.type);//发送负载类型
        while(*pt){
            DL_UART_transmitDataBlocking(UART_0_INST,*pt++);
            delay_us(100);
        }

        pt = str;
        delay_ms(1);
        sprintf(str,"t9.txt=\"%.0f\"\xff\xff\xff",cable_measurement.value*100);//发送负载值
        while(*pt){
            DL_UART_transmitDataBlocking(UART_0_INST,*pt++);
            delay_us(100);
        }
    }
        
    else
        return;
    pt = str;
    delay_ms(1);
    sprintf(str,"t2.txt=\"%s\"\xff\xff\xff","结果保持");//发送负载值
    while(*pt){
        DL_UART_transmitDataBlocking(UART_0_INST,*pt++);
        delay_us(100);
    }    
	return;
}

void Reaction(void){//
	if(isdigit(uart_data[0])){
		switch(uart_data[0]){
			case '0':
                mode = 1;//切换为测量模式0，长度测量
			    break;
			case '1':
                mode = 2;//切换为测量模式1，终端负载测量
                break;
        }
    }
}
//通信接受中断函数
void  UART_0_INST_IRQHandler(){
    static u8 uart_cnt = 0,warning_cnt = 0,warning_temp = 0;
    u8 data = 0;
    switch (DL_UART_getPendingInterrupt(UART_0_INST )){ //检测是否串口中断
        case DL_UART_MAIN_IIDX_RX:
            data = DL_UART_receiveData(UART_0_INST);
            // 处理接收到的数据
            if(data != 0xff){
                warning_data = data;
                warning_cnt = 0;
            }
            else{
                if(++warning_cnt == 3){
                    warning_data = warning_temp;
                    warning_cnt = 0;
                    uart_cnt = 0;
                }
            }
            // 处理接收到的数据
            if(data == '\n'){
                uart_data[uart_cnt] = '\0';
                Reaction();
                uart_cnt = 0;
            }
            else if(!isprint(data) || data == '#');
            else
                uart_data[uart_cnt++] = data; 
            break;
        default:
            break;
    }
}

//读取ADC的数据
unsigned int adc_getValue(void){
	unsigned int gAdcResult = 0;

	//软件触发ADC开始转换
	DL_ADC12_startConversion(ADC_Volt_INST);
	//如果当前状态为正在转换中则等待转换结束
	while (false == gCheckADC) {
			__WFE();
	}
	//获取数据
	gAdcResult = DL_ADC12_getMemResult(ADC_Volt_INST, ADC_Volt_ADCMEM_0);

	//清除标志位
	gCheckADC = false;

	return gAdcResult;
}

//ADC中断服务函数
void ADC_Volt_INST_IRQHandler(void){
	//查询并清除ADC中断
	switch (DL_ADC12_getPendingInterrupt(ADC_Volt_INST)){
		//检查是否完成数据采集
		case DL_ADC12_IIDX_MEM0_RESULT_LOADED:
			gCheckADC = true;//将标志位置1
			break;
		default:
			break;
	}
}

void TIMER_0_INST_IRQHandler(void){
    //清除定时器中断标志位
    switch (DL_Timer_getPendingInterrupt(TIMER_0_INST)){
        case DL_TIMERA_IIDX_ZERO:
            if (mode==1)
                DL_GPIO_togglePins(LED_PORT, LED_PIN_1_PIN);//翻转LED状态
            break;
        
        default:
            break;
    }
}