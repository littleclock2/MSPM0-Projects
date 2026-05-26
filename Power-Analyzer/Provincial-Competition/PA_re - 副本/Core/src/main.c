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


#define REVISE_FACTOR_V 1637
#define REVISE_FACTOR_I 1410
/* Global variables and defines for FFT */
#define IFFTFLAG 0
#define BITREVERSE 1

/* Expected Maximum amplitude and frequency index of FFT */
#define GOLDEN_FFT_MAX_FREQ_INDEX (5)


void revise(void);

void sleep(void);
void menu_control(void);
void calculation(void);
void ADC_DMA_Resumption(ADC12_Regs * adc12,DMA_Regs *dma,uint8_t channelNum, uint16_t* Destination,uint16_t cnt);
void fill_complex_arr(bool cmd);//填充复数数组
/*
cmd = 0;填充电压数组
cmd = 1;填充电流数组
*/

bool is_shutdown = false;
bool key = 0; // 按键值
bool IDLE = 0; // 空闲状态
bool DMA_done = false; // 电压DMA完成标志
bool Current_DMA_done = false; // 电流DMA完成标志
bool clear = false;
u8 press = 0; // 按键值

int cnt; // 计数器
volatile uint16_t ADC_sig[NUM_SAMPLES * 2];//电压和电流交替分配，需要进一步整理，并进行FFT 

volatile float temp_amp[NUM_SAMPLES * 2]; // 临时幅值数组
volatile float complex_input[NUM_SAMPLES * 2]; // 复数数组
//////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
float I_phase = 0; // 电流相位
float V_phase = 0; // 电压相位
arm_cfft_radix4_instance_f32 scfft; // FFT实例

float Apparent_power = 0; // 视在功率
float Active_power = 0; // 有功功率
float Power_factor = 0; // 功率因数
double I_rams_arr[10]; // 电流有效值数组
double V_rams = 0; // 电压有效值
double I_rams = 0; // 电流有效值
double THD = 0; // 总谐波失真
int main(void){
  
  /* System initialization */
  SYSCFG_DL_init();
  

  OLED_Init();
  DL_TimerG_startCounter(TIMER_0_INST);
  NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
  OLED_ShowString(20, 0, "Single-phase ");
  OLED_ShowString(8, 2, "Power Analyzer");
  OLED_ShowString(12, 6, "Tap to start");
  NVIC_EnableIRQ(KEY_INT_IRQN);
  
  /* ADC interrupt enable */
  NVIC_EnableIRQ(ADC12_0_INST_INT_IRQN);
  
  // Start ADC conversion
  DL_ADC12_startConversion(ADC12_0_INST);
  ADC_DMA_Resumption(ADC12_0_INST,DMA,DMA_CH0_CHAN_ID,(uint16_t*)&ADC_sig[0],NUM_SAMPLES);
  
  while (1){
		if(key)
			menu_control();
      if(DMA_done){
        calculation();
        DMA_done = false;
        ADC_DMA_Resumption(ADC12_0_INST,DMA,DMA_CH0_CHAN_ID,(uint16_t*)&ADC_sig[0],NUM_SAMPLES);
      }
        
  }
     
  
}
void fill_complex_arr(bool cmd){
  if(cmd){//填充电流数组
    for(uint16_t i = 0; i < NUM_SAMPLES * 2 - 1;i += 2){
      complex_input[i] = (ADC_sig[i + 1] - REVISE_FACTOR_I) * 33.0f / 4096;//这里多乘十倍 为了增加精度
      complex_input[i + 1] = 0;
    }
  }
  else{//填充电压数组
    for(uint16_t i = 0; i < NUM_SAMPLES * 2 - 1;i += 2){
      complex_input[i] = (ADC_sig[i] - REVISE_FACTOR_V) * 33.0f / 4096;//这里多乘十倍 为了增加精度
      complex_input[i + 1] = 0;
    }
  }
}
void calculation(void){

  V_rams  = 0;
  I_rams  = 0;
  THD     = 0;
    /*有效值计算*/
  //电压：
  float temp = 0;
  for(uint16_t i = 0; i < NUM_SAMPLES * 2 ;i += 2){
    temp = (ADC_sig[i] - REVISE_FACTOR_V) * 3.3f / 4096;
    V_rams +=  temp * temp / NUM_SAMPLES;
  }
  V_rams = sqrt(V_rams);//开方 得到有效值
  //电流：
  for(uint16_t i = 1; i < NUM_SAMPLES * 2 ;i += 2){
    temp = (ADC_sig[i] - REVISE_FACTOR_I) * 3.3f /4096;
    I_rams +=  temp * temp / NUM_SAMPLES;
  }
  I_rams = sqrt(I_rams);//开方 得到有效值

  
  fill_complex_arr(0);//填充电压数组
  hanning_window(complex_input,NUM_SAMPLES);
  arm_cfft_radix4_init_f32(&scfft,NUM_SAMPLES,0,1);//初始化
  arm_cfft_radix4_f32(&scfft,complex_input);//傅里叶计算  转化为实部 虚部的点集 其中 需要测出50HZ的电流的初相位
  arm_cmplx_mag_f32(complex_input,temp_amp,NUM_SAMPLES); //得到通过FFT得到  频谱
  hanning_window_revise(temp_amp,2*NUM_SAMPLES);
  float v_real = complex_input[12] ,v_imag = complex_input[13];//实部虚部
  V_phase = atan(v_imag / v_real);//测得电压相位角（弧度制）





  fill_complex_arr(1);//填充电流数组
	hanning_window(complex_input,NUM_SAMPLES);
  arm_cfft_radix4_init_f32(&scfft,NUM_SAMPLES,0,1);//初始化
  arm_cfft_radix4_f32(&scfft,complex_input);//傅里叶计算  转化为实部 虚部的点集 其中 需要测出50HZ的电流的初相位
  arm_cmplx_mag_f32(complex_input,temp_amp,NUM_SAMPLES); //得到通过FFT得到  频谱
  hanning_window_revise(temp_amp,2*NUM_SAMPLES);
  /*测量各个分量的有效值*/
  for(int i = 0 ; i < 4 ; i++)
      I_rams_arr[i] = temp_amp[6 * (i + 1)] * 1.414213562373095 / NUM_SAMPLES;
  for(int i = 4 ; i < 10 ; i++)
      I_rams_arr[i] = temp_amp[6 * (i + 1) - 1] * 1.41421356237309 / NUM_SAMPLES;
  float i_real = complex_input[12] ,i_imag = complex_input[13];//实部虚部    
  I_phase = atan(i_imag / i_real);//测得电流相位角（弧度制）


  Power_factor = cos(I_phase - V_phase);//得到功率因数
  Apparent_power = I_rams * V_rams;//视在功率
  Active_power = Apparent_power * Power_factor;//有功功率
  for(u8 i = 1; i < 10; i++)
      THD += I_rams_arr[i];
  revise();
  THD = 100 * sqrt(THD)/I_rams_arr[0];//得到THD的百分比

}
void menu_control(void){
	if(clear){
		OLED_Clear();
		clear = false;
	}
	switch(press){
			case 1:
				OLED_ShowString(0, 2, "Irams: ");
				OLED_ShowFloat(56, 2, I_rams, 4);
     
				OLED_ShowString(0, 0, "Vrams: ");
				OLED_ShowFloat(56, 0, V_rams, 4);
				OLED_ShowChar(122,0,'V');
				OLED_ShowChar(122,2,'A');
				
      break;
			case 2:
				OLED_ShowString(0,0,"Power_F:");
				OLED_ShowFloat(72,0,Power_factor,2);
				OLED_ShowString(0,2,"Active_P:");
				OLED_ShowFloat(48,4,Active_power,3);
				OLED_ShowChar(122,4,'W');
			break;
      case 3:
      OLED_ShowString(0, 0, "I1: ");
      OLED_ShowFloat(64, 0, I_rams_arr[0], 4);
      OLED_ShowString(0,2 , "I2: ");
      OLED_ShowFloat(64, 2, I_rams_arr[1], 4);
			OLED_ShowChar(122,0,'A');
			OLED_ShowChar(122,2,'A');
      break;
			case 4:
			OLED_ShowString(0, 0, "I3: ");
      OLED_ShowFloat(64, 0, I_rams_arr[2], 4);
      OLED_ShowString(0, 2, "I4: ");
      OLED_ShowFloat(64, 2, I_rams_arr[3], 4);
			OLED_ShowChar(122,0,'A');
			OLED_ShowChar(122,2,'A');
			break;
      case 5:
      OLED_ShowString(0, 0, "I5: ");
      OLED_ShowFloat(64, 0, I_rams_arr[4], 4);
      OLED_ShowString(0, 2, "I6: ");
      OLED_ShowFloat(64, 2, I_rams_arr[5], 4);
			OLED_ShowChar(122,0,'A');
			OLED_ShowChar(122,2,'A');
      break;
			case 6:
				OLED_ShowString(0, 0, "I7: ");
				OLED_ShowFloat(64, 0, I_rams_arr[6], 4);
				OLED_ShowString(0, 2, "I8: ");
				OLED_ShowFloat(64, 2, I_rams_arr[7], 4);
				OLED_ShowChar(122,0,'A');
			  OLED_ShowChar(122,2,'A');
			break;
			case 7:
				OLED_ShowString(8, 0, "I9: ");
				OLED_ShowFloat(64, 0, I_rams_arr[8], 4);
				OLED_ShowString(0, 2, "I10: ");
				OLED_ShowFloat(64, 2, I_rams_arr[9], 4);
				OLED_ShowChar(122,0,'A');
				OLED_ShowChar(122,2,'A');
			break;
      case 0:
      OLED_ShowString(56, 0, "THD ");
      OLED_ShowFloat(32, 2, THD, 3);
			OLED_ShowChar(128,2,'%');
      break;
      break;
      case 8:
      // Add code for case 8 here
      break;
      case 9:
      // Add code for case 9 here
      break;
      case 10:
      // Add code for case 10 here
      break;
      case 11:
      // Add code for case 11 here
      break;
      case 12:
      // Add code for case 12 here
      break;
      case 13:
      // Add code for case 13 here
      break;
      case 14:
      // Add code for case 14 here
      break;
      case 15:
      // Add code for case 15 here
      break;
      case 16:
      break;
      default :
      break;
      }

  return;
}
void ADC_DMA_Resumption(ADC12_Regs * adc12,DMA_Regs *dma,uint8_t channelNum, uint16_t* Destination,uint16_t cnt){
        DL_ADC12_startConversion(adc12);
        DL_ADC12_enableDMA(adc12);
		DL_DMA_setSrcAddr(dma,channelNum,DL_ADC12_getFIFOAddress(adc12));
        DL_DMA_setDestAddr(dma,channelNum,(uint32_t)&Destination[0]);
        DL_DMA_setTransferSize(dma,channelNum,cnt);
        DL_DMA_enableChannel(dma,channelNum);
        DL_ADC12_startConversion(adc12);
}

void ADC12_0_INST_IRQHandler(void){
  
  switch (DL_ADC12_getPendingInterrupt(ADC12_0_INST)){
    case DL_ADC12_IIDX_DMA_DONE:
      DMA_done = true;
			cnt++;
      break;
    default :
      break;
  }
  
  return ;
}

void GROUP1_IRQHandler(void){
	switch(DL_GPIO_getPendingInterrupt(KEY_PORT)){
		case KEY_USER_IIDX:
			if (!DL_GPIO_readPins(KEY_PORT, KEY_USER_PIN)){
				delay_ms(10);
				if(!DL_GPIO_readPins(KEY_PORT, KEY_USER_PIN)){
					if(is_shutdown){//从屏幕关闭苏醒
						key = 1;
						is_shutdown =false;
            DL_TimerG_startCounter(TIMER_0_INST);
            OLED_Display_On();
					}
					else{
						key = 1;
						press = (press +1)% 8; 
						clear = true;
						DL_TimerG_startCounter(TIMER_0_INST);
						
						
					}
				}
			}
			break;
		default:
			break;
	}
}

void TIMER_0_INST_IRQHandler(void){
	switch(DL_TimerG_getPendingInterrupt(TIMER_0_INST)){
    case DL_TIMERG_IIDX_ZERO:
			sleep();
			DL_TimerG_stopCounter(TIMER_0_INST);
      break;
    default:
      break;
  }

}
void sleep(void){
  OLED_Display_Off();
	DL_SYSCTL_setPowerPolicySHUTDOWN();
	is_shutdown = true;
}
void revise(void){
  V_rams *= 630.5714f;
  I_rams *= 1.0f;//没标定
  I_rams_arr[0] *= 0.095507f;
  I_rams_arr[1] *= 0.023651f;
  I_rams_arr[2] *= 0.226969f;
  I_rams_arr[3] *= 0.096236f;
  I_rams_arr[4] *= 0.362070f;
  I_rams_arr[5] *= 0.107825f;
  I_rams_arr[6] *= 0.103048f;
  I_rams_arr[7] *= 0.077972f;
  I_rams_arr[8] *= 0.088145f;
  I_rams_arr[9] *= 0.076190f;

}