/*

 * @brief       main
 * @language    C
 * @harfware   单相电路功率分析仪
 * @version     v1.0
 * @date        29-July-2024
 * @author     
 
*/
#include "ti_msp_dl_config.h"
#include "oled_spi.h"
#include "key.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "rms.h"
#include <IQmathLib.h>

int cnt = 0;//计数器
//键盘第一列为避免冲突，故屏蔽，剩余可用12个按键
int key_value = 0;//按键值

bool gCheckADC_0;//用于判断ADC0是否就绪的标志
bool gCheckADC_1;//用于判断ADC1是否就绪的标志

int menu  =0;//菜单选择
bool menu_flag = 0;//菜单标志

/*通信变量*/
int16_t voltage=0;//电压
int16_t current=0;//电流
int16_t power=0;//有功功率
int16_t power_F=0;//功率因数
int16_t voltage_rms=0;//基波的有效值
int16_t voltage_rms_n=0;//n次谐波的有效值
int16_t turns=1;//匝数比
/*函数声明*/
void OLED_Control(void);
void KeyValueJudge(void);

// 定义测试函数
// 初始化输入数据

        int size = 1;  
        float math_output;
        _iq iq_output;
        
void test_math_sqrt(float input) {
    for(int i = 0;i<1000;i++){
        math_output = sin(math_output*0.001);
    }
}

void test_iqmath_sqrt(_iq input) {
    for(int i = 0;i<1000;i++){
        iq_output= _IQsin(iq_output*0.001);
    }
}

int main(void){
    /* adc转换结果 */
    uint16_t adcResult_0;         // adc转换结果（12bit，数字量）
    uint16_t adcResult_1;         // adc转换结果（12bit，数字量）
    float adcResult_voltage_0;    // adc转换结果对应的电压值（单位:mV）
    float adcResult_voltage_1;    // adc转换结果对应的电压值（单位:mV）
    /* 系统初始化 */
    SYSCFG_DL_init();
    
    
    
    NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);//顺序不能改，不然queue有问题
	NVIC_EnableIRQ(UART_0_INST_INT_IRQN);//中断函数启动

    OLED_Init();
    OLED_Clear();

    OLED_ShowString(20, 0, "Single-phase ");//在OLED上显示单相电路功率分析仪
    OLED_ShowString(8, 2, "Power Analyzer");
    delay_ms(500);
    OLED_Clear();

    // OLED_ShowString(0, 0, "ADC val 0:");/* 在OLED上显示ADC0采样值 */
    // OLED_ShowString(0, 2, "voltage:");
    // OLED_ShowString(104, 2, "mV");

    // OLED_ShowString(0, 4, "ADC val 1:");/* 在OLED上显示ADC1采样值 */
    // OLED_ShowString(0, 6, "voltage:");
    // OLED_ShowString(104, 6, "mV");

    /* 开启 adc 中断 */
   // NVIC_EnableIRQ(ADC12_0_INST_INT_IRQN);
   // NVIC_EnableIRQ(ADC12_1_INST_INT_IRQN);

    /* 变量初始化 */
    gCheckADC_0 = false;

    /* 确保内部参考电压在adc转换前已完成配置 */
    //while (DL_VREF_CTL1_READY_NOTRDY == DL_VREF_getStatus(VREF));

    /* 启动adc转换 */
    ///DL_ADC12_startConversion(ADC12_0_INST);
    //DL_ADC12_startConversion(ADC12_1_INST);
    
        
    float input = 3.1415926;
    _iq24 iq_input;
    iq_input  = _IQ24(3.1415926);
    
    while (1){
        // 使用 math 库进行测试
        
        NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);//中断函数启动
        DL_TimerG_startCounter(TIMER_0_INST);//启动计时器
        test_math_sqrt(input);
        DL_TimerG_stopCounter(TIMER_0_INST);//启动计时器
        NVIC_DisableIRQ(TIMER_0_INST_INT_IRQN);//中断函数关闭
        OLED_ShowNum(0, 0, cnt, 12, 16);//显示计数器
        OLED_ShowNum(0, 2, math_output, 12, 16);//显示计数器()

        delay_ms(1000);
        cnt  = 0;
        // 使用 IQmath 库进行测试
        DL_TimerG_startCounter(TIMER_0_INST);//启动计时器
        NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);//中断函数启动
        test_iqmath_sqrt(iq_input);
        NVIC_DisableIRQ(TIMER_0_INST_INT_IRQN);//中断函数关闭
        DL_TimerG_stopCounter(TIMER_0_INST);//启动计时器
        OLED_ShowNum(0, 6, iq_output, 12, 16);//显示计数器()
        OLED_ShowNum(0, 4, cnt, 12, 16);//显示计数器
        delay_ms(1000);
        cnt  = 0;
    }
}

void OLED_Control(void){
    switch (menu){
        case 0:{//电压、电流、有功功率、功率因数 
            /* 在OLED上显示电压值 */
            OLED_ShowString(4, 0, "Voltage:");
            OLED_ShowNum(72, 0, voltage, 4, 16);
            OLED_ShowString(104, 0, "mV");
            /*显示电流值*/
            OLED_ShowString(4, 2, "Current:");
            OLED_ShowNum(72, 2, current, 4, 16);
            OLED_ShowString(104, 2, "mA");
            /*显示有功功率值*/
            OLED_ShowString(4, 4, "Power:");
            OLED_ShowNum(72, 4, power, 4, 16);
            OLED_ShowString(104, 4, "mW");
            /*显示功率因数*/
            OLED_ShowString(4, 6, "Power_F:");
            OLED_ShowNum(72, 6, power_F, 4, 16);
            break;
        }
        case 1:{
            /*显示基波的有效值*/
            OLED_ShowString(4, 0, "U_1:    ");
            OLED_ShowNum(72, 0, voltage_rms, 4, 16);
            OLED_ShowString(104, 0, "mV");
            /*显示n次谐波的有效值*/
            OLED_ShowString(4, 2, "U_n:    ");
            OLED_ShowNum(72, 2, voltage_rms_n, 4, 16);
            OLED_ShowString(104, 2, "mV");
            /*当前匝数比*/
            OLED_ShowString(4, 4, "Turns:    ");
            OLED_ShowNum(72, 4, turns, 4, 16);
            OLED_ShowString(104, 4, "   ");
            /*清除旧的未显示部分*/
            OLED_ShowString(4, 6, "               ");
            break;
        }
        default:
            break;
    }
}

///* adc中断服务函数 */
//void ADC12_0_INST_IRQHandler(void){
//    switch (DL_ADC12_getPendingInterrupt(ADC12_0_INST)){
//    /* adc转换完成后进入中断 */
//    case DL_ADC12_IIDX_MEM0_RESULT_LOADED:
//        //gCheckADC_0 = true;
//        break;
//    default:
//        break;
//    }
//}

void KeyValueJudge(void){
    if(getKeyValue()!=0){
        key_value = getKeyValue();
    }
    switch (key_value){
        case 2:
            menu = 0;
            menu_flag = 1;
            break;
        case 3:
            menu = 1;
            menu_flag = 1;
            break;
        case 4:
            menu_flag = 0;
            break;
        case 6:
            turns = 1;
            break;
        case 7:
            turns = 2;
            break;
        case 8:
            turns = 3;
            break;
        case 10:
            turns = 4;
            break;
        case 11:
            turns = 5;
            break;
        case 12:
            turns = 6;
            break;
        case 14:
            turns = 7;
            break;
        case 15:
            turns = 8;
            break;
        case 16:
            turns = 9;
            break;
        default:
            break;
    }
}
// void UART_0_INST_IRQHandler(void){
// 	static bool cnt=0;
// 	static int16_t temp_Voltage=0;
    
// 	u8 data=DL_UART_receiveData(UART_0_INST);//接收数据
//     DL_UART_transmitData(UART_0_INST,data);//发送数据
//     if(!cnt){//判断是否对准
//         if((data & 0b10000000) != 0){//对准
//             cnt = 1;
//             temp_Voltage += (data & 0b01111111) << 7;
//         }
//     }
//     else{
//         if((data & 0b10000000) == 0){//对准
//             cnt = 0;
//             voltage = temp_Voltage + (data & 0b01111111);
//             temp_Voltage = 0;//清零
//         }
//         else{
//             cnt = 0;
//             temp_Voltage = 0;
//         }
//     }
// } 

//中断函数
void TIMER_0_INST_IRQHandler (void){
	switch( DL_TimerG_getPendingInterrupt (TIMER_0_INST)){
		case DL_TIMERG_IIDX_ZERO:
            cnt++;
			break;
		default:
			break;
	}
}
