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
#include <stdint.h>
#include <math.h>
#include "rms.h"
#include <IQmathLib.h>
#include "arm_const_structs.h"
#include "arm_math.h"

//键盘第一列为避免冲突，故屏蔽，剩余可用12个按键
int key_value = 0;//按键值

bool gCheckADC_0;//用于判断ADC0是否就绪的标志
bool gCheckADC_1;//用于判断ADC1是否就绪的标志

int menu  =0;//菜单选择
bool menu_flag = 0;//菜单标志

/*通信变量*/
uint16_t voltage=0;//电压
uint16_t current=0;//电流
uint16_t power=0;//有功功率
uint16_t power_F=0;//功率因数
uint16_t current_rms=0;//基波的有效值
uint16_t current_rms_n=0;//n次谐波的有效值
uint16_t turns=1;//匝数比
/*函数声明*/
void OLED_Control(void);
void KeyValueJudge(void);
int cnt;
uint16_t input_v[1024];
uint16_t input_i[1024];
//////////////////////////////////////////////////////////////////

void generate_sine_wave(uint32_t *buffer, int size, int peak_to_peak, int min_value) {
    int amplitude = peak_to_peak / 2;
    for (int i = 0; i < size; i++) {
        buffer[i] = (amplitude * sin(2 * PI * i / size) + amplitude + min_value);
        if(i==1) i =1;
    }
}
double rmsValue;
/* Number of samples */
#define NUM_SAMPLES 256
/* Global variables and defines for FFT */
#define IFFTFLAG 0
#define BITREVERSE 1
static uint16_t gDstBuffer[NUM_SAMPLES] = {0, 4091, 8021, 11639, 14810, 17421,
    19391, 20671, 21246, 21137, 20397, 19109, 17378, 15325, 13078, 10767, 8513,
    6421, 4577, 3040, 1839, 976, 424, 128, 16, 0, -16, -128, -424, -976, -1839,
    -3040, -4577, -6421, -8513, -10767, -13078, -15325, -17378, -19109, -20397,
    -21137, -21246, -20671, -19391, -17421, -14810, -11639, -8021, -4091, 0,
    4091, 8021, 11639, 14810, 17421, 19391, 20671, 21246, 21137, 20397, 19109,
    17378, 15325, 13078, 10767, 8513, 6421, 4577, 3040, 1839, 976, 424, 128,
    16, 0, -16, -128, -424, -976, -1839, -3040, -4577, -6421, -8513, -10767,
    -13078, -15325, -17378, -19109, -20397, -21137, -21246, -20671, -19391,
    -17421, -14810, -11639, -8021, -4091, 0, 4091, 8021, 11639, 14810, 17421,
    19391, 20671, 21246, 21137, 20397, 19109, 17378, 15325, 13078, 10767, 8513,
    6421, 4577, 3040, 1839, 976, 424, 128, 16, 0, -16, -128, -424, -976, -1839,
    -3040, -4577, -6421, -8513, -10767, -13078, -15325, -17378, -19109, -20397,
    -21137, -21246, -20671, -19391, -17421, -14810, -11639, -8021, -4091, 0,
    4091, 8021, 11639, 14810, 17421, 19391, 20671, 21246, 21137, 20397, 19109,
    17378, 15325, 13078, 10767, 8513, 6421, 4577, 3040, 1839, 976, 424, 128,
    16, 0, -16, -128, -424, -976, -1839, -3040, -4577, -6421, -8513, -10767,
    -13078, -15325, -17378, -19109, -20397, -21137, -21246, -20671, -19391,
    -17421, -14810, -11639, -8021, -4091, 0, 4091, 8021, 11639, 14810, 17421,
    19391, 20671, 21246, 21137, 20397, 19109, 17378, 15325, 13078, 10767, 8513,
    6421, 4577, 3040, 1839, 976, 424, 128, 16, 0, -16, -128, -424, -976, -1839,
    -3040, -4577, -6421, -8513, -10767, -13078, -15325, -17378, -19109, -20397,
    -21137, -21246, -20671, -19391, -17421, -14810, -11639, -8021, -4091, 0,
    4091, 8021, 11639, 14810, 17421};
q15_t inputData[NUM_SAMPLES * 2]; // 复数数据对的数组

// 函数原型
void prepareInputData(q15_t *inputData, q15_t *adSampledData, int length);
void performFFT(q15_t *inputData, int length);
    
void prepareInputData(q15_t *inputData, q15_t *adSampledData, int length) {
    for (int i = 0; i < length; i++) {
        inputData[2 * i] = (q15_t)(adSampledData[i] * 32767); // 实部
        inputData[2 * i + 1] = 0; // 虚部
    }
}

void performFFT(q15_t *inputData, int length) {
    arm_cfft_instance_q15 fftInstance;
    arm_cfft_init_q15(&fftInstance, length); // 初始化 FFT 实例
    arm_cfft_q15(&fftInstance, inputData, 0, 1); // 执行 FFT
}    
    
/* Expected Maximum amplitude and frequency index of FFT */
#define GOLDEN_FFT_MAX_FREQ_INDEX (5)
int* p  =  gDstBuffer;
/* Array with FFT results */
volatile int16_t gFFTOutput[NUM_SAMPLES*2];
/* Maximum amplitude of FFT */
volatile uint32_t gFFTmaxValue;
/* Index of maximum frequency */
volatile uint32_t gFFTmaxFreqIndex;
////////////////////////////////////////////////////////////////////////////////////
int main(void){
    
    /* adc转换结果 */
    uint16_t adcResult_0;         // adc转换结果（12bit，数字量）
    uint16_t adcResult_1;         // adc转换结果（12bit，数字量）
    float adcResult_voltage_0;    // adc转换结果对应的电压值（单位:mV）
    float adcResult_voltage_1;    // adc转换结果对应的电压值（单位:mV）
    /* 系统初始化 */
    SYSCFG_DL_init();
    
    DL_TimerG_startCounter(TIMER_0_INST);//启动计时器
	NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);//中断函数启动
    
    NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);//顺序不能改，不然queue有问题
	NVIC_EnableIRQ(UART_0_INST_INT_IRQN);//中断函数启动

    OLED_Init();
    OLED_Clear();

//    OLED_ShowString(20, 0, "Single-phase ");//在OLED上显示单相电路功率分析仪
//    OLED_ShowString(8, 2, "Power Analyzer");
//    delay_ms(500);
//    OLED_Clear();
//    
//     OLED_ShowString(0, 0, "ADC val 0:");/* 在OLED上显示ADC0采样值 */
//     OLED_ShowString(0, 2, "voltage:");
//     OLED_ShowString(104, 2, "mV");

//     OLED_ShowString(0, 4, "ADC val 1:");/* 在OLED上显示ADC1采样值 */
//     OLED_ShowString(0, 6, "voltage:");
//     OLED_ShowString(104, 6, "mV");

//    /* 开启 adc 中断 */
//    NVIC_EnableIRQ(ADC12_0_INST_INT_IRQN);
//    NVIC_EnableIRQ(ADC12_1_INST_INT_IRQN);

//    /* 变量初始化 */
//     gCheckADC_0 = false;
//     gCheckADC_1 = false;
//    /* 确保内部参考电压在adc转换前已完成配置 */
//    //while (DL_VREF_CTL1_READY_NOTRDY == DL_VREF_getStatus(VREF));

//    /* 启动adc转换 */
//    DL_ADC12_startConversion(ADC12_0_INST);
//    DL_ADC12_startConversion(ADC12_1_INST);
    
    //generate_sine_wave(gDstBuffer, 1024, 4096,0);
    
    
     // 初始化输入数据
    prepareInputData(inputData, (q15_t *)gDstBuffer, NUM_SAMPLES);

    // 执行 FFT
    performFFT(inputData, NUM_SAMPLES);

    // 提取实部和虚部
    q15_t realPart[NUM_SAMPLES];
    q15_t imagPart[NUM_SAMPLES];

    for (int i = 0; i < NUM_SAMPLES; i++) {
        realPart[i] = inputData[2 * i];
        imagPart[i] = inputData[2 * i + 1];
    }
    
    
    
    arm_cfft_q15(&arm_cfft_sR_q15_len256, (q15_t *) gDstBuffer, IFFTFLAG, BITREVERSE);
    arm_cmplx_mag_q15((q15_t *) gDstBuffer, (q15_t *) gFFTOutput, NUM_SAMPLES);
    arm_max_q15((q15_t *) gFFTOutput, NUM_SAMPLES, (q15_t *) &gFFTmaxValue,(uint32_t *) &gFFTmaxFreqIndex);

    /*
     * The following results are expected:
     * gFFTOutput = FFT output of the signal
     * gFFTmaxFreqIndex = 5
     *  Note that bin 5 corresponds to ~1,953Hz +- ~195Hz, which represents
     *  the 2kHz major component.
     *  The next higher component in the FFT is bin 10 which corresponds to
     *  the ~3,906Hz +- ~195Hz, representing the 4kHz component.
     */
    if (gFFTmaxFreqIndex != GOLDEN_FFT_MAX_FREQ_INDEX) {
        __BKPT(0);
    }
    /* 等待adc转换完成 */
        while (false == gCheckADC_0){
            /* 进入低功耗模式 wait for event */
            __WFE();
        }
        /* 等待adc转换完成 */
        while (false == gCheckADC_1){
            /* 进入低功耗模式 wait for event */
            __WFE();
        }
    while (1){
        KeyValueJudge();
{
        
        /* 将adc采样值读出 */
        adcResult_0 = DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_0);
        adcResult_1 = DL_ADC12_getMemResult(ADC12_1_INST, DL_ADC12_MEM_IDX_1);
        /* 根据公式计算对应的电压值，参考电压3.3V（sysconfig里设置） */
        adcResult_voltage_0 = adcResult_0 * 3300.0 / (4096.0 - 1) - 0.5 * 3300.0 / 4096.0;
        adcResult_voltage_1 = adcResult_1 * 3300.0 / (4096.0 - 1) - 0.5 * 3300.0 / 4096.0;
        /* 在OLED上显示采样值 */
        OLED_ShowNum(72, 0, adcResult_0, 4, 16);
        OLED_ShowNum(72, 2, (uint16_t)adcResult_voltage_0, 4, 16);
        /* 在OLED上显示采样值 */
        OLED_ShowNum(72, 4, adcResult_1, 4, 16);
        OLED_ShowNum(72, 6, (uint16_t)adcResult_voltage_1, 4, 16);
         /* 准备下一次采样 */
        gCheckADC_0 = false;
        gCheckADC_1 = false;
        
    }        
        //OLED_Control();
        /* 控制大概0.2s更新一次 */
        delay_ms(200);
        //启动串口中断
        NVIC_EnableIRQ(UART_0_INST_INT_IRQN);//中断函数启动
        while (cnt <= 511){
            
            //input_v[cnt] = voltage;
            cnt++;
        }
  
        NVIC_DisableIRQ(UART_0_INST_INT_IRQN);//中断函数启动
        rmsValue = get_Vrms(input_v,1);
        //计算各类东西
        
       
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
            /*显示基波电流的有效值*/
            OLED_ShowString(4, 0, "I:    ");
            OLED_ShowNum(72, 0, current_rms, 4, 16);
            OLED_ShowString(104, 0, "mA");
            /*显示n次谐波的有效值*/
            OLED_ShowString(4, 2, "I_n:    ");
            OLED_ShowNum(72, 2, current_rms_n, 4, 16);
            OLED_ShowString(104, 2, "mA");
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

/* adc中断服务函数 */
void ADC12_0_INST_IRQHandler(void){
    switch (DL_ADC12_getPendingInterrupt(ADC12_0_INST)){
    /* adc转换完成后进入中断 */
    case DL_ADC12_IIDX_MEM0_RESULT_LOADED:
        gCheckADC_0 = true;
        break;
    default:
        break;
    }
}

/* adc中断服务函数 */
void ADC12_1_INST_IRQHandler(void){
    switch (DL_ADC12_getPendingInterrupt(ADC12_1_INST)){
    /* adc转换完成后进入中断 */
    case DL_ADC12_IIDX_MEM1_RESULT_LOADED:
        gCheckADC_1 = true;
        break;
    default:
        break;
    }
}

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
void UART_0_INST_IRQHandler(void){
	static bool cnt=0;
	static int16_t temp_Voltage=0;
    
	u8 data=DL_UART_receiveData(UART_0_INST);//接收数据
    //DL_UART_transmitData(UART_0_INST,data);//发送数据
    if(!cnt){//判断是否对准
        if((data & 0b10000000) != 0){//对准
            cnt = 1;
            temp_Voltage += (data & 0b01111111) << 7;
        }
    }
    else{
        if((data & 0b10000000) == 0){//对准
            cnt = 0;
            voltage = temp_Voltage + (data & 0b01111111);
            temp_Voltage = 0;//清零
        }
        else{
            cnt = 0;
            temp_Voltage = 0;
        }
    }
} 

//中断函数
void TIMER_0_INST_IRQHandler (void){
	switch( DL_TimerG_getPendingInterrupt (TIMER_0_INST)){
		case DL_TIMERG_IIDX_ZERO:
        if(!menu_flag){
            if(menu == 0){
                menu = 1;
            }
            else{
                menu = 0;
            }
        }
			break;
		default:
			break;
	}
}
