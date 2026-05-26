#include "ti_msp_dl_config.h"
#include "oled_spi.h"
#include "AD9910.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define CABLE_VELOCITY (2.0/3.0 * 299792458)  // 电缆中信号传播速度
#define COARSE_STEPS   150      // 粗扫步数
#define FINE_STEPS     150     // 精扫步数
#define VOLTAGE_THRESHOLD  0.3 // 电压变化阈值 (需要根据实际校准)
#define BUFFER_SIZE  COARSE_STEPS
#define EXTRA_LENGTH 0
uint8_t choice  =0;
volatile uint16_t mode = 0;
uint8_t uart_cmd = 0;
//uart数据接收变量
u8 warning_data = 0;
float frequency[BUFFER_SIZE] = {};
float Amp[BUFFER_SIZE] = {};	

float V1 = 0;
float V2 = 0;
float C_frequency = 1e5;//测量电容时的频率
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
float FreqSweep(uint32_t start_freq, uint32_t end_freq,  uint32_t steps, uint8_t find_min, bool find_first);
uint8_t MeasureCableLength(void);
void SendData(void);
void Reaction(void);
float ADS1118_get(uint8_t MUX,uint8_t PGA);
void Pre_Capacitor_Measure(void);
void Load_Measure(void);
/*
MUX为选择通道
0: Ain0 + Ain1-（差分）
1: Ain0 + Ain3-（差分）
2: Ain1 + Ain3-（差分）
3: Ain2 + Ain3-（差分）
4：Ain0
5: Ain1
6: Ain2 
7: Ain3


PGA为量程选择
0:6.144V
1:4.096V
2:2.048V
3:1.024V
4:0.512V
5:0.256V
*/


// 主函数整合
int main(void) {
	uint16_t temp_mode = mode;
		float temp_data = 0;
    SYSCFG_DL_init();
    //通信初始化
	NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN );//清除中断标志位
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
    //adc初始化
    NVIC_ClearPendingIRQ(ADC_Volt_INST_INT_IRQN );//清除中断标志位
    NVIC_EnableIRQ(ADC_Volt_INST_INT_IRQN);
    
		DL_ADC12_enableConversions(ADC_Volt_INST);
    // 外设初始化
    OLED_Init();
    Init_AD9910();
		DL_SPI_enable(SPI_1118_INST);
    // 初始参数
    AD9910_AmpWrite(15383); // 设置幅度
    AD9910_FreWrite(10000); // 设置频率
    DL_GPIO_clearPins(Relay_PORT,Relay_R1_PIN | Relay_R2_PIN | Relay_R3_PIN);
    while(1) {
//				delay_ms(1);
//				temp_data = ADS1118_get(7,2);
        if(mode != 0) {
						
						switch (mode) {//根据模式选择测量
							case 1:// 测量模式1,长度测量
									MeasureCableLength();// 测量电缆长度
									if(cable_measurement.length < 10){
										if(cable_measurement.length < 1){
											C_frequency = 1e6;
										}
										else{
											C_frequency = 3e5;
										}
									}
									else
										C_frequency = 1e5;
									Pre_Capacitor_Measure();
									break;
							case 2:// 测量模式2,终端负载测量
									Load_Measure();// 测量终端负载
									break;
							default:
									break;
						}
            SendData();// 发送数据
            mode = 0; // 重置模式
        }
    }
}

// // 改进的扫频函数
// float FreqSweep(uint32_t start_freq, uint32_t end_freq,  uint32_t steps, uint8_t find_min) {
//    float extremum_value = find_min ? 1.0f : 0.0f;// 初始值
//    float extremum_freq = 0.0f;// 初始频率
//    float log_start = log10f(start_freq);// 计算对数频率
//    float log_end = log10f(end_freq);// 计算对数频率
//    float log_step = (log_end - log_start)/steps;// 计算对数步长

//    // 频率扫描
//    for(uint32_t i = 0; i <= steps; ++i) {
//         const uint32_t freq = (uint32_t)powf(10.0f, log_start + i*log_step);// 计算当前频率

//         AD9910_FreWrite(freq);      // 设置频率
//         delay_ms(5);              // 稳定时间

//         float voltage = adc_getValue()/4095.0 * 3.3;// 读取电压值
//         frequency[i] = freq;
//         Amp[i] = voltage;
//         // 寻找极值点
//         if((find_min && (voltage < extremum_value)) || (!find_min && (voltage > extremum_value))) {
//             extremum_value = voltage;
//             extremum_value = freq;
//         }
//    }
//    return extremum_freq;
// }

float FreqSweep(uint32_t start_freq, uint32_t end_freq,  uint32_t steps, uint8_t find_min,bool find_first) {
    float extremum_freq = 0.0f;// 初始频率
    float log_start = log10f(start_freq);// 计算对数频率
    float log_end = log10f(end_freq);// 计算对数频率
    float log_step = (log_end - log_start)/steps;// 计算对数步长
    float voltage = 0;
    // 频率扫描
    for(uint32_t i = 0; i < steps; ++i) {
        uint32_t freq = (uint32_t)powf(10.0f, log_start + i*log_step);// 计算当前频率
        AD9910_FreWrite(freq);      // 设置频率
        delay_ms(5);              // 稳定时间
        voltage = (float)adc_getValue()/4095.0 * 3.3;// 读取电压值
				frequency[i] = freq;
				Amp[i] = voltage;

    }
		
		if(find_first){
			for(uint32_t i = 3; i < steps - 3; ++i) {// 寻找极值点
					if(find_min){
							if(Amp[i] < 0.3 &&
								(Amp[i] <= Amp[i-3]) && (Amp[i] <= Amp[i-2]) && (Amp[i] <= Amp[i-1]) && (Amp[i] <= Amp[i+1]) && (Amp[i] <= Amp[i+2]) && (Amp[i] <= Amp[i+3])){
									extremum_freq = frequency[i];
									break;
							}
					}
					else{
							if(	Amp[i] > 0.2 &&
								(Amp[i] >= Amp[i-3]) && (Amp[i] >= Amp[i-2]) && (Amp[i] >= Amp[i-1]) && (Amp[i] >= Amp[i+1]) && (Amp[i] >= Amp[i+2]) && (Amp[i] >= Amp[i+3])){
									extremum_freq = frequency[i];
									break;
							}
					}
			}
		}
		else{
			float temp = 999;
			if(find_min){	
				for(uint16_t i = 0;i<steps;i++){
					if(temp >= Amp[i]){
						temp = Amp[i];
						extremum_freq = frequency[i];
					}
				}
			}
				
			else{
				for(uint16_t i = 0;i<steps;i++){
					if(temp <= Amp[i]){
						temp = Amp[i];
						extremum_freq = frequency[i];
					}
				}
			}
			
		}
		return extremum_freq;
}

void Pre_Capacitor_Measure(void){//预测量电容处电压
		AD9910_FreWrite(C_frequency);
    DL_GPIO_setPins(Relay_PORT,Relay_R1_PIN);
    DL_GPIO_clearPins(Relay_PORT,Relay_R2_PIN | Relay_R3_PIN);
    delay_ms(300);
    V1 = ADS1118_get(4,2);
    DL_GPIO_clearPins(Relay_PORT,Relay_R1_PIN);
    return;
}

// 分阶段测量函数
uint8_t MeasureCableLength(void) {
    // 第一阶段：粗扫寻找1/4波长波节（电压最小值）
    cable_measurement.coarse_freq = FreqSweep(2000000,80000000,COARSE_STEPS,1,1);// 2.5MHz到50MHz,步数50，找极小值模式
    // 有效性检查
    if(cable_measurement.coarse_freq < 1.5e6 || cable_measurement.coarse_freq > 80e6) {
			DL_GPIO_clearPins(LED_PORT,LED_PIN_1_PIN);
			return 0; // 测量失败
    }

    // 第二阶段：精扫寻找1/2波长波节（电压最大值）
    const uint32_t fine_band = cable_measurement.coarse_freq * 0.1; // ±10%带宽
    cable_measurement.fine_freq = FreqSweep(
        cable_measurement.coarse_freq - fine_band,//扫描区域下界
        cable_measurement.coarse_freq + fine_band,//扫描区域上界
        FINE_STEPS,                               // 精细扫描步数
        1,                                         // 寻找最小值
				0																					// 寻找最小值
    );
    // 计算两种模式下的长度
    float lambda1 = CABLE_VELOCITY / cable_measurement.coarse_freq;// 波长1
    cable_measurement.length = lambda1 / 4.0f;//长度1
		cable_measurement.length = 0.964 * cable_measurement.length + 
															 0.00119; 
		cable_measurement.length += (cable_measurement.length/175.0 + 0.0150285714285);
    return 1; // 测量成功
}

void Load_Measure(void) {
    double C_cable = 0,
          C_sum = 0;
    double R_cable = 0,
           R_sum = 0;

    DL_GPIO_setPins(Relay_PORT,Relay_R2_PIN);
    DL_GPIO_clearPins(Relay_PORT,Relay_R1_PIN | Relay_R3_PIN);
    delay_ms(100);
    V2 = ADS1118_get(7,2);
    if(V2 >1.3){//电容为负载
				AD9910_FreWrite(C_frequency);
				DL_GPIO_setPins(Relay_PORT,Relay_R1_PIN);
				DL_GPIO_clearPins(Relay_PORT,Relay_R2_PIN | Relay_R3_PIN);
				delay_ms(300);
        V2 = ADS1118_get(4,2);
        C_cable = 15/(2*3.1415926535*V1)*1e5/C_frequency;//电缆电容 单位 nF
        C_sum = 15/(2*3.1415926535*V2)*1e5/C_frequency;//负载电容+电缆电容 单位 nF
        cable_measurement.value = (C_sum - C_cable) * 1000;//单位pF
				//y = 1.4763x + 5.0543
				cable_measurement.value = cable_measurement.value *1.4763+5.0543;
			
        cable_measurement.type = 'C';//电容
        DL_GPIO_clearPins(Relay_PORT,Relay_R1_PIN);
    }
    else{//电阻为负载
        cable_measurement.type = 'R';//电阻
        cable_measurement.value = (15*V2)/(1.5-V2);//电阻值 单位 Ω
        DL_GPIO_clearPins(Relay_PORT,Relay_R2_PIN);
			//y = 0.0035x2 + 1.0331x - 0.6532
				cable_measurement.value = cable_measurement.value*cable_measurement.value*0.0035+
																	cable_measurement.value * 1.0331+
																	-0.6532;
				cable_measurement.length = cable_measurement.value + 0.6/20.0*cable_measurement.value;

    }
    DL_GPIO_clearPins(Relay_PORT,Relay_R1_PIN | Relay_R2_PIN | Relay_R3_PIN);
    delay_ms(1);
    return;
}

void SendData(void){
	char str[30]={};
	char *pt = str;
    if(mode == 1){
        //长度结果发送
        sprintf(str,"t3.txt=\"%.2f\"\xff\xff\xff",cable_measurement.length*100 - EXTRA_LENGTH);//长度单位转换为cm
        while(*pt){
            DL_UART_transmitDataBlocking(UART_0_INST,*pt++);
            delay_us(100);
        }
    }
        
    else if(mode == 2){
        //终端负载数据发送
        pt = str;
        delay_ms(1);
        sprintf(str,"t9.txt=\"%c\"\xff\xff\xff",cable_measurement.type);//发送负载类型
        while(*pt){
            DL_UART_transmitDataBlocking(UART_0_INST,*pt++);
            delay_us(100);
        }

        pt = str;
        delay_ms(1);
        sprintf(str,"t8.txt=\"%.2f\"\xff\xff\xff",cable_measurement.value);//发送负载值
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
		switch(uart_cmd){
			case '0':
          mode = 1;//切换为测量模式0，长度测量
					DL_GPIO_setPins(LED_PORT,LED_PIN_1_PIN);
			break;
			case '1':
					mode = 2;//切换为测量模式1，终端负载测量
					DL_GPIO_clearPins(LED_PORT,LED_PIN_1_PIN);
					break;
			default:
				mode = mode;
				break;
      }
    
}
//通信接受中断函数
void  UART_0_INST_IRQHandler(){
    switch (DL_UART_getPendingInterrupt(UART_0_INST )){ //检测是否串口中断
        case DL_UART_MAIN_IIDX_RX:
            uart_cmd = DL_UART_receiveData(UART_0_INST);
						Reaction();
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
	DL_ADC12_enableConversions(ADC_Volt_INST);
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


float ADS1118_get(uint8_t MUX,uint8_t PGA){
	uint16_t command = 0b1000000110101010 + (MUX << 12) + (PGA << 9);
	int16_t data = 0;
    float V = 0;
	DL_SPI_transmitDataBlocking16(SPI_1118_INST,command);
	data = DL_SPI_receiveDataBlocking16(SPI_1118_INST);
	delay_ms(10);
	DL_SPI_transmitDataBlocking16(SPI_1118_INST,command);
	data = DL_SPI_receiveDataBlocking16(SPI_1118_INST);
	delay_ms(10);
	DL_SPI_transmitDataBlocking16(SPI_1118_INST,command);
	data = DL_SPI_receiveDataBlocking16(SPI_1118_INST);
	delay_ms(10);
	DL_SPI_transmitDataBlocking16(SPI_1118_INST,command);
	data = DL_SPI_receiveDataBlocking16(SPI_1118_INST);
	DL_SPI_transmitDataBlocking16(SPI_1118_INST,command);
	data = DL_SPI_receiveDataBlocking16(SPI_1118_INST);
	delay_ms(10);
	DL_SPI_transmitDataBlocking16(SPI_1118_INST,command);
	data = DL_SPI_receiveDataBlocking16(SPI_1118_INST);
	delay_ms(10);
	DL_SPI_transmitDataBlocking16(SPI_1118_INST,command);
	data = DL_SPI_receiveDataBlocking16(SPI_1118_INST);
	delay_ms(10);
	DL_SPI_transmitDataBlocking16(SPI_1118_INST,command);
	data = DL_SPI_receiveDataBlocking16(SPI_1118_INST);
    switch(PGA){
        case 0:
           V = (float)data/(1<<15)*6.144;
            break;
        case 1:
            V = (float)data/(1<<15)*4.096;
            break;
        case 2:
            V = (float)data/(1<<15)*2.048;
            break;
        case 3:
            V = (float)data/(1<<15)*1.024;
            break;
        case 4:
            V = (float)data/(1<<15)*0.512;
            break;
        case 5:
            V = (float)data/(1<<15)*0.256;
            break;
        default:
            V = (float)data/(1<<15)*0.256;
            break;
    }
    V = 1.0049 * V + 0.0012;
    return V;
}

