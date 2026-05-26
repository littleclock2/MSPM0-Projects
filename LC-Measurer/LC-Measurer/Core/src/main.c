/*

      ___           ___           ___           ___           ___           ___           ___           ___     
     |\__\         /\__\         /\__\         /\  \         /\__\         /\  \         /\  \         /\  \    
     |:|  |       /:/  /        /::|  |       /::\  \       /:/ _/_       /::\  \       /::\  \       /::\  \   
     |:|  |      /:/  /        /:|:|  |      /:/\ \  \     /:/ /\__\     /:/\:\  \     /:/\:\  \     /:/\:\  \  
     |:|__|__   /:/  /  ___   /:/|:|  |__   _\:\~\ \  \   /:/ /:/ _/_   /:/  \:\  \   /::\~\:\  \   /:/  \:\__\ 
     /::::\__\ /:/__/  /\__\ /:/ |:| /\__\ /\ \:\ \ \__\ /:/_/:/ /\__\ /:/__/ \:\__\ /:/\:\ \:\__\ /:/__/ \:|__|
    /:/~~/~    \:\  \ /:/  / \/__|:|/:/  / \:\ \:\ \/__/ \:\/:/ /:/  / \:\  \ /:/  / \/_|::\/:/  / \:\  \ /:/  /
   /:/  /       \:\  /:/  /      |:/:/  /   \:\ \:\__\    \::/_/:/  /   \:\  /:/  /     |:|::/  /   \:\  /:/  / 
   \/__/         \:\/:/  /       |::/  /     \:\/:/  /     \:\/:/  /     \:\/:/  /      |:|\/__/     \:\/:/  /  
                  \::/  /        /:/  /       \::/  /       \::/  /       \::/  /       |:|  |        \::/__/   
                   \/__/         \/__/         \/__/         \/__/         \/__/         \|__|         ~~       

 * @brief       main
 * @language    C
 * @harfware   Template
 * @version     v1.0
 * @date        29-July-2024
 * @author      YunSword
 
*/


#include "ti_msp_dl_config.h"
#include <stdio.h>
#include "uart_screen.h"
#include "oled_spi.h"
#include "key.h"
#include "AD9910.h"
#define PI 3.1415926

/* 检查adc是否完成转换 */
volatile bool gCheckADC;
uint8_t mode = 0x00;
uint8_t choice = 0;
bool m0 = false;
bool m1= false;
bool m2= false;
bool range = false;
bool reset = false;
 /* adc转换结果 */
    uint16_t adcResult_0[60];         // adc转换结果（12bit，数字量）
    uint16_t adcResult_1[60];         // adc转换结果（12bit，数字量）
    uint16_t adcResult_2[60];         // adc转换结果（12bit，数字量）
    uint16_t adcResult_0f = 0;//adc0最终平均值
    uint16_t adcResult_1f = 0;
    uint16_t adcResult_2f = 0;
    
    float adcResult_voltage0 = 0;    // adc转换结果对应的电压值0（单位:mV）
    float adcResult_voltage1 = 0;    // adc转换结果对应的电压值0（单位:mV）
    float adcResult_voltage2 = 0;    // adc转换结果对应的电压值0（单位:mV）
    
//计算结果
float Xc;//容抗
float C ;//电容值
float Xl;//感抗
float H;//电感值

void UART_Send(uint8_t *Data_Temp){
	uint8_t i;

	for(i=0;i<2;i++){
		DL_UART_transmitData(UART0,*(Data_Temp+1));
		while(DL_UART_isTXFIFOFull(UART0)){
			
		}
		Data_Temp--;
	}
}

int main(void){

    int i = 0;//循环采集计数器
    /* 系统初始化 */
    SYSCFG_DL_init();
    //通信初始化
	NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN );//清除中断标志??
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN );
    
    NVIC_ClearPendingIRQ(UART_1_INST_INT_IRQN );//清除中断标志??
    NVIC_EnableIRQ(UART_1_INST_INT_IRQN );
     /* 开启 adc 中断 */
    NVIC_EnableIRQ(ADC12_0_INST_INT_IRQN);
    
    DL_TimerG_startCounter(TIMER_0_INST);//启动计时??
	NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);//中断函数启动
    
    /* 变量初始化 */
    gCheckADC = false;
    
    /* 确保内部参考电压在adc转换前已完成配置 */
    while (DL_VREF_CTL1_READY_NOTRDY == DL_VREF_getStatus(VREF))
        
    delay_ms(300);	//延时一会儿，等待上电稳定    
    Init_AD9910();					//AD9910控制脚及寄存器初始化
    
	AD9910_FreWrite(1000000);	//写输出频率100KHz。范围：0~420000000，对应频率0Hz~420MHz
	AD9910_AmpWrite(16383);	//写输出幅度最大。范围：0~16383对应峰峰值0mv~800mv(左右)
    
    DL_GPIO_setPins(GPIOA,MAS_REST);
    DL_GPIO_clearPins(GPIOA,MAS_REST);
    while (1){
        AD9910_FreWrite(100000);	//写输出频率1KHz。范围：0~420000000，对应频率0Hz~420MHz
        
        /* 启动adc转换 */
        DL_ADC12_startConversion(ADC12_0_INST);
        //AD9910_FreWrite(10000);	//写输出频率1KHz。范围：0~420000000，对应频率0Hz~420MHz
        // /* 等待adc转换完成 */
        // while (false == gCheckADC){
        //     /* 进入低功耗模式 wait for event */
        //     __WFE();
        // }
        //AD9910_FreWrite(10000);	//写输出频率1KHz。范围：0~420000000，对应频率0Hz~420MHz
        
		/* 将adc采样值读出 */
        adcResult_0[i] = DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_0);
        adcResult_1[i] = DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_1);
        adcResult_2[i] = DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_2);
        i++;
        //AD9910_FreWrite(10000);	//写输出频率1KHz。范围：0~420000000，对应频率0Hz~420MHz
        if(i>=60){
            i = 0;
            for(int t = 0;t<60;t++){
                adcResult_0f += adcResult_0[t];
                adcResult_1f += adcResult_1[t];
                adcResult_2f += adcResult_2[t];
            }
            //求平均值
            adcResult_0f /=60;
            adcResult_1f /=60;
            adcResult_2f /=60;
           
            /* 根据公式计算对应的电压值，参考电压2.5V（sysconfig里设置） */
            adcResult_voltage0 = adcResult_0f * 3300.0 / (4096.0 - 1)  + 30;//计算测出的电压加上30mv的偏移
            adcResult_voltage1 = adcResult_1f * 3300.0 / (4096.0 - 1)  + 30;//计算电流对应的电压加上30mv的偏移
            adcResult_voltage2 = adcResult_2f * 3300.0 / (4096.0 - 1) ;//鉴相器传过来的电压
            //清零临时变量
            adcResult_0f =0;
            adcResult_1f =0;
            adcResult_2f =0;
        }
         // AD9910_FreWrite(10000);	//写输出频率1KHz。范围：0~420000000，对应频率0Hz~420MHz
        float real_v;
        float real_i;
        //这里写计算公式
        DL_GPIO_clearPins(SW_PORT,SW_PA19_PIN);//电容放电函数
        if(reset == true){
            DL_GPIO_setPins(GPIOA,AD9910_PWR);
            reset = false;
            DL_GPIO_setPins(SW_PORT,SW_PA19_PIN);
            delay_ms(200);
            DL_GPIO_clearPins(SW_PORT,SW_PA19_PIN);
            DL_GPIO_clearPins(GPIOA,AD9910_PWR);
        }
        //AD9910_FreWrite(10000);	//写输出频率1KHz。范围：0~420000000，对应频率0Hz~420MHz
        if(mode == 0){//电容模式
            //大继电器换档
            DL_GPIO_clearPins(GPIOB,DAC_PA18_PIN);//切换为电容测量 011
            DL_GPIO_setPins(GPIOA,DAC_PA17_PIN);
            DL_GPIO_setPins(GPIOA,DAC_PA15_PIN);
           //默认从高到低进行量程切换
            if(!range){
                //先切换大量程 11
                DL_GPIO_setPins(SW_PORT,SW_S1_PIN);
                DL_GPIO_setPins(SW_PORT,SW_S2_PIN);
                
                //总之这里是一串计算过程
                real_v = adcResult_voltage0;//先算真实电压，该处电压计算不准确，有待调整
                real_i = adcResult_voltage1;//计算此时的电流

                C = real_i/(2*PI*100000*real_v);//计算电容值，目前i和v单位为mV和mA
                // if(adcResult_voltage0 < 100 ){
                //     //切换小量程 10
                //     DL_GPIO_setPins(SW_PORT,SW_S1_PIN);//电容测量小量程 3220欧
                //     DL_GPIO_clearPins(SW_PORT,SW_S2_PIN);
                //     range = true;
                // }
                // if(C*1000000000<=1000){//电容于10nF
                //     //再切换小量程 10
                //     DL_GPIO_setPins(SW_PORT,SW_S1_PIN);//电容测量小量程 3220欧
                //     DL_GPIO_clearPins(SW_PORT,SW_S2_PIN);
                //     range = true;
                // }
            }           
            else if(range){
                //总之这里也是一串计算过程
                real_v = adcResult_voltage0;//先算真实电压，该处电压计算不准确，有待调整
                real_i = adcResult_voltage1;
                
                C = real_i/(2*PI*100000*real_v);//计算电容值，目前i和v单位为mV和mA
                if(adcResult_voltage0 > 100 ){
                    //先切换大量程 11
                    DL_GPIO_setPins(SW_PORT,SW_S1_PIN);
                    DL_GPIO_setPins(SW_PORT,SW_S2_PIN);
                    range = false;
                }
                // if(C*1000000000>1000){
                //     DL_GPIO_setPins(SW_PORT,SW_S1_PIN);
                //     DL_GPIO_setPins(SW_PORT,SW_S2_PIN);
                //     range = false;
                // }
            }                 
        }
        else if(mode == 1){//电感模式
            //大继电器换档
            DL_GPIO_clearPins(GPIOA,DAC_PA15_PIN);//切换为电感测量 000
            DL_GPIO_clearPins(GPIOA,DAC_PA17_PIN);
            DL_GPIO_clearPins(GPIOB,DAC_PA18_PIN);
            //小继电器换挡
            DL_GPIO_setPins(SW_PORT,SW_S1_PIN);//与电容共用240欧量程
            DL_GPIO_clearPins(SW_PORT,SW_S2_PIN);
            
            //总之这里也是一串计算过程
            real_v = adcResult_voltage0/2;//先算真实电压
            real_i = adcResult_voltage1/240;//计算电感对应的电流
            
            H = real_v/(2*PI*100000*real_i) * 3/4;//计算电感值，目前i和v单位为mV和mA
            
        }
        //AD9910_FreWrite(10000);	//写输出频率1KHz。范围：0~420000000，对应频率0Hz~420MHz
        /* 准备下一次采样 */
        gCheckADC = false;
        DL_ADC12_enableConversions(ADC12_0_INST);
    }
}

////通信接受中断函数
//void  UART_0_INST_IRQHandler(){
//   switch (DL_UART_getPendingInterrupt(UART_0_INST )){ //检测是否串口中断
//        case DL_UART_MAIN_IIDX_RX:
//            choice = DL_UART_receiveData(UART_0_INST);
//            HMISendb(0xff);
//            if(choice==0x01) mode=1;
//            else if(choice==0x00) mode=0;
//            break;
//        default:
//            break;
//    }
//}

//通信接受中断函数
void  UART_1_INST_IRQHandler(){
   switch (DL_UART_getPendingInterrupt(UART_1_INST )){ //检测是否串口中断
        case DL_UART_MAIN_IIDX_RX:
            choice = DL_UART_receiveData(UART_1_INST);
            HMISendb(0xff);
            if(choice==0x01) mode=1;
            else if(choice==0x00) mode=0;
            else if(choice == 0x03)reset = true;
            break;
        default:
            break;
    }
}

//中断函数
void TIMER_0_INST_IRQHandler (void){
	switch( DL_TimerG_getPendingInterrupt (TIMER_0_INST)){
		case DL_TIMERG_IIDX_ZERO:
			{//向外界实时发送信??
                int temp = 0;
				unsigned char  buf[64];
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
                
                temp = adcResult_voltage0;
                sprintf((char *)buf,"n3.val=%d",temp);  //发送0
				HMISends((char *)buf);
				HMISendb(0xff);
                 
                temp = adcResult_voltage1;
                sprintf((char *)buf,"n4.val=%d",temp);  //发送1
				HMISends((char *)buf);
				HMISendb(0xff);
                 
                temp = adcResult_voltage2;
                sprintf((char *)buf,"n5.val=%d",temp);  //发送2
				HMISends((char *)buf);
				HMISendb(0xff);
                
			}
			break;
		default:
			break;
	}
}
/* adc中断服务函数 */
void ADC12_0_INST_IRQHandler(void){
    
    switch (DL_ADC12_getPendingInterrupt(ADC12_0_INST)){
        /* adc转换完成后进入中断 */
        case DL_ADC12_IIDX_MEM0_RESULT_LOADED:
            
        case DL_ADC12_IIDX_MEM1_RESULT_LOADED:
            
        case DL_ADC12_IIDX_MEM2_RESULT_LOADED:
            gCheckADC = true;
            break;
        default:
            break;
    }
}



