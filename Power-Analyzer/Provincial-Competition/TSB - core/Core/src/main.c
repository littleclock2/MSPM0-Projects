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
#include "window.h"

/* Number of samples */
#define NUM_SAMPLES 1024


#define REVISE_FACTOR_V 1627
#define REVISE_FACTOR_I 1627
/* Global variables and defines for FFT */
#define IFFTFLAG 0
#define BITREVERSE 1

/* Expected Maximum amplitude and frequency index of FFT */
#define GOLDEN_FFT_MAX_FREQ_INDEX (5)


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
volatile uint16_t input_v[NUM_SAMPLES];
volatile uint16_t input_i[NUM_SAMPLES];

volatile float temp_amp[NUM_SAMPLES * 2];//作为存放幅度值的缓冲区
volatile float f_input[NUM_SAMPLES * 2];//
//////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
float I_phase = 0,V_phase = 0;//电压的初相位 电流初相位 用于计算功率因数 有功功率
arm_cfft_radix4_instance_f32  scfft;//定义scfft结构体
uint16_t ADC_cnt0 = 0, ADC_cnt1 = 0;//NUM_SAMPLES时 停止采样 ADC0存电压  ADC1存电流
/*下面是题目要求的的全局变量*/

float Apparent_power = 0 , Active_power = 0 , Power_factor = 0;//按顺序是视在功率 有功功率 功率因数 
//题目要求显示有功功率 和 功率因数
float I_rams_arr[10];//储存十个电流的有效值   这个值最要在后续进行加权 进行精确计算 
//要求显示电流有效值，即[0]，还有2-10次电流谐波的有效值 也就是[1]~[9]
double V_rams = 0,I_rams = 0;//储存电压,电流的有效值
double THD = 0;
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

    OLED_Init();
    OLED_Clear();

    OLED_ShowString(20, 0, "Single-phase ");//在OLED上显示单相电路功率分析仪
    OLED_ShowString(8, 2, "Power Analyzer");
    delay_ms(500);
    OLED_Clear();
    
     OLED_ShowString(0, 0, "ADC val 0:");/* 在OLED上显示ADC0采样值 */
     OLED_ShowString(0, 2, "voltage:");
     OLED_ShowString(104, 2, "mV");

     OLED_ShowString(0, 4, "ADC val 1:");/* 在OLED上显示ADC1采样值 */
     OLED_ShowString(0, 6, "voltage:");
     OLED_ShowString(104, 6, "mV");

    /* 开启 adc 中断 */
    NVIC_EnableIRQ(ADC12_0_INST_INT_IRQN);
    NVIC_EnableIRQ(ADC12_1_INST_INT_IRQN);
    
    //启动adc转换
    DL_ADC12_startConversion(ADC12_0_INST);
    DL_ADC12_startConversion(ADC12_1_INST);
    
    
    
    /* 变量初始化 */
     gCheckADC_0 = false;
     gCheckADC_1 = false;
    /* 确保内部参考电压在adc转换前已完成配置 */
    //while (DL_VREF_CTL1_READY_NOTRDY == DL_VREF_getStatus(VREF));
    /* 等待adc转换完成 */
//        while (false == gCheckADC_0){
//            /* 进入低功耗模式 wait for event */
//            __WFE();
//        }
//        /* 等待adc转换完成 */
//        while (false == gCheckADC_1){
//            /* 进入低功耗模式 wait for event */
//            __WFE();
//        }
    while (1){
        KeyValueJudge();

        if(ADC_cnt0 == NUM_SAMPLES)
            NVIC_DisableIRQ(ADC12_0_INST_INT_IRQN);
        if(ADC_cnt1 == NUM_SAMPLES)
            NVIC_DisableIRQ(ADC12_1_INST_INT_IRQN);

        if(ADC_cnt0 == NUM_SAMPLES && ADC_cnt1 == NUM_SAMPLES){//开始计算
            V_rams  = 0;
            I_rams  = 0;
            THD     = 0;
            ADC_cnt0 = ADC_cnt1 = 0;//归零
            
            
            
//            /*有效值计算*/
//            //电压：
//            for(uint16_t i = 0; i < NUM_SAMPLES ;i++)
//                V_rams += pow(input_v[i],2) / NUM_SAMPLES;
//            V_rams = sqrt(V_rams);//开方 得到有效值
            
//            //电流：
//            int temp = 0;
//            for(uint16_t i = 0; i < NUM_SAMPLES ;i++){
//                temp = input_i[i] - REVISE_FACTOR;
//                I_rams += pow(temp,2) / NUM_SAMPLES;
//                
//            }
//            I_rams = sqrt(I_rams);//开方 得到有效值
            
   


            for(int i=0; i < NUM_SAMPLES; i++)
        {
            f_input[2*i]=(input_i[i]-REVISE_FACTOR_I)*3.3f/4095.0; //实部
            f_input[2*i+1]=0; 
        }       
        //电流：
            for(uint16_t i = 0; i < NUM_SAMPLES * 2 ;i += 2){
                I_rams += f_input[i] * f_input[i];   
            }
            I_rams = sqrt(I_rams / NUM_SAMPLES);//开方 得到有效值
        
        
            
            /*可加！！ 汉宁窗修正 和下面注释的的修正搭配可以提高精度（可能）*/
            /*当然可以替换成其他的加窗函数 最终多种加窗方案对比 得出最好的*/
            
            hanning_window (f_input,NUM_SAMPLES);
            
            arm_cfft_radix4_init_f32(&scfft,NUM_SAMPLES,0,1);//初始化
            arm_cfft_radix4_f32(&scfft,f_input);//傅里叶计算  转化为实部 虚部的点集 其中 需要测出50HZ的电流的初相位
            
            
            
            arm_cmplx_mag_f32(f_input,temp_amp,NUM_SAMPLES); //得到通过FFT得到  频谱
            
            hanning_window_revise(temp_amp,NUM_SAMPLES);
            
            
            /*测量电流相位*/
            /*不要管这个警告*/
            float i_real = f_input[20] ,i_imag = f_input[21];//实部虚部
            I_phase = atan(i_imag / i_real);//测得电流相位角（弧度制）

            /*测量各个电流分量的有效值*/
            for(u8 i = 0 ; i < 10 ; i++)//测量10个电流的有效值
                I_rams_arr[i] = temp_amp[10*(i+1)] * 2.0 / (NUM_SAMPLES * 1.414213562373095);//为了写清楚 我这里写得难看 这句话就是把幅度转化为幅值 然后再转成有效值（除以根号二）
            
            
            
            
            /* 测量THD*/
            for(u8 i = 1; i < 10; i++)
                THD += pow(temp_amp[10 * (i+1)] * 2.0 / NUM_SAMPLES , 2);//累加幅度的平方
            THD = 100 * sqrt(THD) / pow(temp_amp[5]* 2 / NUM_SAMPLES , 2) ;//得到THD的百分比

           /*测量电压相位*/
            for(int i=0; i < NUM_SAMPLES; i++){
                f_input[2*i]= (input_v[i] - REVISE_FACTOR_V) * 3.3f / 4095; //实部
                f_input[2 * i + 1]=0; 
            }
            
            
            for(uint16_t i = 0; i < NUM_SAMPLES ;i += 2){
                V_rams += f_input[i] * f_input[i];   
            }
            V_rams = sqrt(V_rams / NUM_SAMPLES);//开方 得到有效值
            
            /*可加！！ 汉宁窗修正 和下面注释的的修正搭配可以提高精度（可能）*/
            /*当然可以替换成其他的加窗函数 最终多种加窗方案对比 得出最好的*/
            
            hanning_window (f_input,NUM_SAMPLES);
            
            
            arm_cfft_radix4_init_f32(&scfft,NUM_SAMPLES,0,1);//初始化
            arm_cfft_radix4_f32(&scfft,f_input);//傅里叶计算  转化为实部 虚部的点集 其中 需要测出50HZ的电流的初相位
        
        
            arm_cmplx_mag_f32(f_input,temp_amp,NUM_SAMPLES); //得到通过FFT得到  频谱
        
            hanning_window_revise(temp_amp,NUM_SAMPLES);
            float v_real = f_input[20] ,v_imag = f_input[21];//实部虚部
            V_phase = atan(v_imag / v_real);//测得电压相位角（弧度制）
            
            /*完成了电流有效值和电压有效值的测量 现在我们直接求出 视在功率*/
            Apparent_power = I_rams * V_rams;
            /*完成了电压 和电流相角的测量 这里得出有功功率的值 以及功率因数*/
            Power_factor = cos(I_phase - V_phase);
            Active_power = Apparent_power * Power_factor;
            NVIC_EnableIRQ(ADC12_0_INST_INT_IRQN);
            NVIC_EnableIRQ(ADC12_1_INST_INT_IRQN);
        }
        /* 
        至此 所有的数字信号的信号都测量完成  
        现在 需要注意的是 除了功率因数之外 所有的数据都需要标定 就连从AD转成正确电压值的步骤都没完成
        */
         
        /* 在OLED上显示采样值 */
        OLED_ShowNum(72, 0, adcResult_0, 4, 16);
        OLED_ShowNum(72, 2, (uint16_t)adcResult_voltage_0, 4, 16);
        /* 在OLED上显示采样值 */
        OLED_ShowNum(72, 4, adcResult_1, 4, 16);
        OLED_ShowNum(72, 6, (uint16_t)adcResult_voltage_1, 4, 16);
        //OLED_Control();
        /* 控制大概0.2s更新一次 */
        delay_ms(200);      
       
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
void ADC12_0_INST_IRQHandler(void){//处理电压
    int16_t data = 0;
    switch (DL_ADC12_getPendingInterrupt(ADC12_0_INST)){
    /* adc转换完成后进入中断 */
			case DL_ADC12_IIDX_MEM0_RESULT_LOADED:
                gCheckADC_0 = true;
                if(ADC_cnt0 < NUM_SAMPLES){
                    data = DL_ADC12_getMemResult(ADC12_0_INST,DL_ADC12_MEM_IDX_0);
                    if(data == 0)//byd零点
                        if(ADC_cnt0 > 1)
                            input_v[ADC_cnt0] = 2 * input_v[ADC_cnt0 - 1] - input_v[ADC_cnt0 - 2];//尽量取一个近似的值 而不是忽略零点 不然傅里叶变换中会得到一些散落的零点 非常有影响
                        else if (ADC_cnt0 > 0)
                            input_v[ADC_cnt0] = input_v[ADC_cnt0 - 1];
                        else//第一个就碰上零点 算倒霉
                            input_v[ADC_cnt0] = 0;
                    else//正常取值
                        input_v[ADC_cnt0] = data;
                    ADC_cnt0++;
                }
				break;
			default:
				break;
    }
}

/* adc中断服务函数 */
void ADC12_1_INST_IRQHandler(void){
    int16_t data = 0;
    switch (DL_ADC12_getPendingInterrupt(ADC12_1_INST)){
    /* adc转换完成后进入中断 */
			case DL_ADC12_IIDX_MEM1_RESULT_LOADED:
                gCheckADC_1 = true;
                if(ADC_cnt1 < NUM_SAMPLES){
                    data = DL_ADC12_getMemResult(ADC12_1_INST,DL_ADC12_MEM_IDX_1);
                    if(data == 0)//byd零点
                        if(ADC_cnt1 > 1)
                            input_i[ADC_cnt1] = 2 * input_i[ADC_cnt1 - 1] - input_i[ADC_cnt1 - 2];//尽量取一个近似的值 而不是忽略零点 不然傅里叶变换中会得到一些散落的零点 非常有影响
                        else if (ADC_cnt1 > 0)
                            input_i[ADC_cnt1] = input_i[ADC_cnt1 - 1];
                        else//第一个就碰上零点 算倒霉
                            input_i[ADC_cnt1] = 0;
                    else//正常取值
                        input_i[ADC_cnt1] = data; 
                    ADC_cnt1++;
                }
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
