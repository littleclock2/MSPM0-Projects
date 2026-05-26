#include "ti_msp_dl_config.h"
#include "oled_spi.h"
#include "AD9910.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "uart_screen.h"

//接收数据标志位
volatile bool gCheckADC;        //ADC采集成功标志位
unsigned int adc_getValue(void);//读取ADC的数据
bool m0 = false;
bool m1= false;
bool m2= false;
bool range = false;
bool reset = false;
uint8_t io_flag  =false;
int fputc(int ch,FILE *p) { 
	 DL_UART_transmitData(UART_0_INST,(u8)ch);  //发送一个字节
	 while(DL_UART_isBusy(UART_0_INST)==true){};//等待发送结束    
	 return ch; 
}

//全局变量定义
uint8_t mode = 0x00;
uint8_t choice = 0;
uint8_t uart_temp_receive = 0;
/* adc转换结果 */
	uint8_t adcResult_index = 0;
	bool adcResult_first_flag = false;
    uint16_t adcResult_0[20];         // adc转换结果（12bit，数字量）
    uint16_t adcResult_1[20];         // adc转换结果（12bit，数字量）
    uint16_t adcResult_2[20];         // adc转换结果（12bit，数字量）

    uint16_t adcResult_0_avg = 0;//adc0最终平均值
    uint16_t adcResult_1_avg = 0;
    uint16_t adcResult_2_avg = 0;
    
    float adcResult_voltage_0 = 0;    // adc转换结果对应的电压值0（单位:mV）
    float adcResult_voltage_1 = 0;    // adc转换结果对应的电压值0（单位:mV）
    float adcResult_voltage_2 = 0;    // adc转换结果对应的电压值0（单位:mV）

//计算结果
float Xc =0;//容抗
float C =0;//电容值
float Xl = 0;//感抗
float H=0;//电感值

//函数定义
void SendData(void);
float ADS1118_get(uint8_t MUX,uint8_t PGA);
void UART_Send(uint8_t *Data_Temp){//uart发送函数
	uint8_t i;
	for(i=0;i<2;i++){
		DL_UART_transmitData(UART0,*(Data_Temp+1));
		while(DL_UART_isTXFIFOFull(UART0)){			
		}
		Data_Temp--;
	}
}




// 主函数整合
int main(void) {
	
    SYSCFG_DL_init();
	
    //通信初始化
	NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN );//清除中断标志位
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
	
    //adc初始化
    NVIC_ClearPendingIRQ(ADC_Volt_INST_INT_IRQN );//清除中断标志位
    NVIC_EnableIRQ(ADC_Volt_INST_INT_IRQN);    
	DL_ADC12_enableConversions(ADC_Volt_INST);
	
	//时钟初始化
	DL_TimerG_startCounter(TIMER_0_INST);//启动计时??
	NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);//中断函数启动
	
    // 外设初始化
    OLED_Init();
    Init_AD9910();
	DL_SPI_enable(SPI_1118_INST);
	
	/* 变量初始化 */
    gCheckADC = false;
	
	/* 确保内部参考电压在adc转换前已完成配置 */
    //while (DL_VREF_CTL1_READY_NOTRDY == DL_VREF_getStatus(VREF))
	
	delay_ms(300);
	
    // 初始参数
    AD9910_AmpWrite(16383); // 设置幅度
    AD9910_FreWrite(100000); // 设置频率
	
    DL_GPIO_clearPins(Relay_PORT,Relay_R1_PIN | Relay_R2_PIN | Relay_R3_PIN);
	
    while(1) {
		AD9910_FreWrite(100000);//dac恒定输出100k正弦信号
		
		//这里启用ADS118的ADC进行接收ADC信号
		adcResult_0[adcResult_index] = ADS1118_get(4,1);
		adcResult_1[adcResult_index] = ADS1118_get(5,1);
		adcResult_2[adcResult_index] = ADS1118_get(6,1);
		adcResult_index++;
		//接收完第一遍数据后，之后对其中数据进行依次替换即可
		if(adcResult_index >= 20) {
			adcResult_index= 0; 
			adcResult_first_flag = true;
		}
		//计算ADC接收数据的平均值
		if(adcResult_first_flag){
			for(int t = 0;t<60;t++){
                adcResult_0_avg += adcResult_0[t];
                adcResult_1_avg += adcResult_1[t];
                adcResult_2_avg += adcResult_2[t];
            }
			
			adcResult_0_avg /=20;
			adcResult_1_avg /=20;
			adcResult_2_avg /=20;
			
			/* 根据公式计算对应的电压值，参考电压2.5V（sysconfig里设置） */
            adcResult_voltage_0 = adcResult_0_avg * 3300.0 / (4096.0 - 1)  + 30;//计算测出的电压加上30mv的偏移
            adcResult_voltage_1= adcResult_1_avg * 3300.0 / (4096.0 - 1)  + 30;//计算电流对应的电压加上30mv的偏移
            adcResult_voltage_2 = adcResult_2_avg * 3300.0 / (4096.0 - 1) ;//鉴相器传过来的电压
		
			adcResult_0_avg = 0;
			adcResult_1_avg = 0;
			adcResult_2_avg = 0;
		}
		//下面是详细的计算过程
		
		 //printf("114514");
		 /* 准备下一次采样 */
        gCheckADC = false;
    }
	return 0;
}


//通信接受中断函数
void  UART_0_INST_IRQHandler(){
    switch (DL_UART_getPendingInterrupt(UART_0_INST )){ //检测是否串口中断
        case DL_UART_MAIN_IIDX_RX:
			choice = DL_UART_receiveData(UART_0_INST);
            HMISendb(0xff);
            if(choice==0x01) 
				mode=1;
            else if(choice==0x00) 
				mode=0;
            else if(choice == 0x03)
				reset = true;
            break;
        default:
            break;
    }
}

//中断函数
void TIMER_0_INST_IRQHandler (void){
	switch( DL_TimerG_getPendingInterrupt (TIMER_0_INST)){
		case DL_TIMERG_IIDX_ZERO:{//向外界实时发送信息并检查IO口状态
			   io_flag = DL_GPIO_readPins(REC_PORT,REC_PIN_1_PIN);
                int temp = 0;
				unsigned char  buf[64] ;
                if(mode==0){
                    temp = (int)(C*1000000000);//转换为int型
                    sprintf((char *)buf,"n0.val=%d",temp); 
                }
                else if(mode==1) {
                    temp = (int)(H*100000000);//转换为int型
                    sprintf((char *)buf,"n0.val=%d",temp);
                }
				HMISends((char *)buf);
				HMISendb(0xff);
                
                sprintf((char *)buf,"n1.val=%d",1);  //发送D值
				HMISends((char *)buf);
				HMISendb(0xff);
                
                sprintf((char *)buf,"n2.val=%d",1);  //发送Q值
				HMISends((char *)buf);
				HMISendb(0xff);
                //调试用发送信号，用完记得注释掉
                
                temp = adcResult_voltage_0;
                sprintf((char *)buf,"n3.val=%d",temp);  //发送0
				HMISends((char *)buf);
				HMISendb(0xff);
                 
                temp = adcResult_voltage_1;
                sprintf((char *)buf,"n4.val=%d",temp);  //发送1
				HMISends((char *)buf);
				HMISendb(0xff);
                 
                temp = adcResult_voltage_2;
                sprintf((char *)buf,"n5.val=%d",temp);  //发送2
				HMISends((char *)buf);
				HMISendb(0xff);
                
			}
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

/*
MUX为选择通道
	0: Ain0 + Ain1-（差分）	1: Ain0 + Ain3-（差分）
	2: Ain1 + Ain3-（差分）	3: Ain2 + Ain3-（差分）
	4：Ain0					5: Ain1
	6: Ain2 					7: Ain3

PGA为量程选择
	0:6.144V		1:4.096V
	2:2.048V		3:1.024V
	4:0.512V		5:0.256V
*/
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

