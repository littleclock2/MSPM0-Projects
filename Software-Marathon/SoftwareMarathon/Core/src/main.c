//此项目可实现ADC通道0读取外接信号，DA产生指定信号、ADC内部通道12读取核心板产生的DA信号
//ADC通道0对应管脚PA27
//DA输出对应管脚PA16
#include "ti_msp_dl_config.h"
#include "oled.h"
#include "oledpicture.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define COMP_0_INST_REF_VOLTAGE_mV (3300)//代表参考电压,其值为3300mV
#define COMP_0_INST_DAC8_OUTPUT_VOLTAGE_mV (800)//设置初始直流输出电压值
volatile bool gCheckADC;//用于判断ADC是否就绪的标志
volatile uint16_t gAdcResult;
#define MAX_DATA_POINTS 200  //表示最多的取样点数
#define SamplingV 50
#define MAX_TOLERANCE 10 // 假设的电压容差  
#define FILTER_WINDOW_SIZE 5 // 滑动平均滤波器的窗口大小

typedef struct {  
    uint16_t voltage[MAX_DATA_POINTS];  
    uint32_t time[MAX_DATA_POINTS];  
    uint16_t index; // 当前数据点的索引  
    uint16_t size;  // 当前缓冲区中有效数据的数量  
} WaveformData;  
WaveformData waveform; 
//uint16_t adcData[400]={0};//存储ADC通道0读取的数据
uint16_t Tcnt = 0,Vpp,Vmid;
uint16_t Duty_Cycle = 0,minpos;
float SV,T;
uint32_t RMS;
uint32_t utick = 0,F;
uint16_t min_voltage = 65535;  
uint16_t max_voltage = 0;  
bool up = 1,down = 0;
// 初始化波形数据  
void init_waveform() {  
    waveform.index = 0;  
    waveform.size = 0;  
}
uint16_t calculate_Vpp();
float calculate_rms();

bool Judgeup(uint16_t pos){
	if(pos - 3 < 0)return 0;
	if(waveform.voltage[pos - 3] <= waveform.voltage[pos - 2]
		&&waveform.voltage[pos - 2] <= waveform.voltage[pos - 1]
		&&waveform.voltage[pos - 1] <= waveform.voltage[pos]) 
		return 1;
	return 0;
}
bool Judgedown(uint16_t pos){
	if(pos - 3 < 0)return 0;
	if(waveform.voltage[pos - 3] >= waveform.voltage[pos - 2]
		&&waveform.voltage[pos - 2] >= waveform.voltage[pos - 1]
		&&waveform.voltage[pos - 1] >= waveform.voltage[pos]) 
		return 1;
	return 0;
}

uint16_t max(uint16_t a,uint16_t b){
	return a > b ? a : b;
}
uint16_t min(uint16_t a,uint16_t b){
	return a > b ? b : a;
}

float sqrt(float n) {
    if (n < 0) return -1;  // 对于负数返回-1，因为平方根没有实数解
    const float err = 1e-5;   // 设置误差界限
    float t = n;
    while (t - n / t > err * t)
        t = (n / t + t) / 2.0;
    return t;
}

uint32_t ABS(uint32_t x){
	if(x < 0) return -x;
	else return x;
}

void SV_Calc();
uint32_t estimate_period(uint16_t * v, uint32_t* t, uint16_t size, uint8_t tolerance);
uint32_t findPeriod(WaveformData * wfd,uint16_t threshold);
uint16_t calculateDutyCycle(WaveformData * wfd, uint32_t period, uint16_t threshold);
uint8_t Judgement(){
//	if(minpos >=190) return 0;
//	uint16_t delta[50],FBcnt = 0,TRcnt = 0;
//	uint16_t max=0;
//	//while(waveform.voltage[minpos] - waveform.voltage[minpos + 1] > 0 ) ++minpos;
//	//while(waveform.voltage[minpos] - waveform.voltage[minpos + 1] < 100){++minpos;}
//	for(uint8_t i = 0;i< 49;++i){
//		delta[i] = 100*ABS(waveform.voltage[minpos + i] - waveform.voltage[minpos + i + 1]);
//	}
//	
//	for(uint8_t i = 0;i< 49;++i)
//	if(delta[i]>max)
//	max=delta[i];
//	
//	
//	for(uint8_t i = 0;i< 49;++i){
//		
//		
//		if(delta[i]>max/2 &&delta[i+1] >max/2) ++FBcnt;
//		if(FBcnt >= 30)return 3;
//		if(delta[i] - delta[i + 1] < 1) ++TRcnt;
//		if(TRcnt >= 10)return 1;
//		
//		
//		
//		
//	}
//	return 2;
if(Vpp*1000/(RMS/1000)>1350)
	return 2;
if(Vpp*1000/(RMS/1000)<1350&&Vpp*1000/(RMS/1000)>1300)
return 3;
return 1;

};

int main(void){
	{	
		SYSCFG_DL_init();//初始化
		OLED_Init();
		NVIC_EnableIRQ(ADC12_0_INST_INT_IRQN);	//ADC中断使能
		gCheckADC = false;
		DL_COMP_enable(COMP_0_INST);	//比较器与AD模块启动
		DL_ADC12_startConversion(ADC12_0_INST);	//ADC开始转换
		const	uint32_t dacValue = (COMP_0_INST_DAC8_OUTPUT_VOLTAGE_mV * 255) /COMP_0_INST_REF_VOLTAGE_mV;	//计算输出的直流电压值对应的DACCode0
		DL_COMP_setDACCode0(COMP_0_INST, dacValue);//写入dacValue
		init_waveform();
	}
	{	
		OLED_Clear();
		DL_Common_delayCycles(32000000);
		OLED_Clear();
	}  
	while (false == gCheckADC) {
		__WFE();
	}
	
	uint8_t Pcnt = 0;uint16_t TTcnt = 0;
	while (1) {
		gAdcResult = waveform.voltage[waveform.index];	//检测PA27引脚的输入电压;
		Vpp = calculate_Vpp();
		Vmid = Vpp / 2;
		RMS = calculate_rms()*1000;
		// 计算自相关函数  
		uint8_t TYflag;
    
		SV_Calc();
		T = (SV * 2 * 1000 / 10000);//周期计算
		F = 1000/T;//频率计算

		Duty_Cycle = calculateDutyCycle(&waveform,T*1000,3000);
		//T = estimate_period_with_noise_using_waveform(&waveform,MAX_TOLERANCE,FILTER_WINDOW_SIZE);
		if(Pcnt >= 64 ) {
			OLED_Clear();
			OLED_ShowString(0,0,"Vpp:");
			OLED_ShowString(64,0,"F:");OLED_ShowString(112,0,"Hz");
			OLED_ShowString(0,6,"Vrms:");
			OLED_ShowString(0,2,"Vt:");
			
			Pcnt = 0;
			TYflag = Judgement();
			switch(TYflag){
				case 1:
					OLED_ShowString(0,4,"Fang");
					break;
				case 2:
					OLED_ShowString(0,4,"Juchi");
					break;
				case 3:
					OLED_ShowString(0,4,"ZhengXian");
					break;
				default:
					break;
				
			}
			OLED_ShowNum(32,0,Vpp*1.17,4,16);
			OLED_ShowNum(80,0,F,4,16);
			//OLED_ShowNum(80,0,Vpp*1000/(RMS/1000),4,16);
			OLED_ShowNum(38,6,RMS/1000,4,16);OLED_ShowChar(72,6,'.');OLED_ShowNum(76,6,RMS%1000,3,16);
			OLED_ShowNum(24,2,gAdcResult,4,16);
			
			//OLED_ShowNum(80,6,T,4,16);
			max_voltage = min_voltage = 0;
		}
		++Pcnt;++TTcnt;
	}
}

// 计算并返回波形的峰峰值  
uint16_t calculate_Vpp() {  
    for (uint16_t i = 0; i < waveform.size; ++i) {  
        if (waveform.voltage[i] < min_voltage) {  
            min_voltage = waveform.voltage[i];
        }  
        if (waveform.voltage[i] > max_voltage) {  
            max_voltage = waveform.voltage[i];  
        }  
    }
    return (max_voltage - min_voltage)/4096.0*3.3*1000;  
}  
// 计算并返回波形的有效值（RMS）  
float calculate_rms() {  
    float sum_of_squares = 0.0;  
    for (uint16_t i = 0; i < waveform.size; ++i) {  
        sum_of_squares += (float)waveform.voltage[i] * waveform.voltage[i];  
    }  
    float rms = sqrt(sum_of_squares / waveform.size);  
    return rms;  
}
void SV_Calc(){
	bool abmid = (waveform.voltage[0] > Vmid);
	uint8_t Ccnt = 0;
	for(uint16_t i = 1;i < waveform.size;++i){
		if(waveform.voltage[i] > Vmid&&!abmid){
			abmid = true;
			++Ccnt;
		} 
		else if(waveform.voltage[i] < Vmid&&abmid){
			abmid = false;
			++Ccnt;
		} 
	}
	SV = (float)(waveform.size - 1)*1.0 / (float)Ccnt * 2.0;
}

uint32_t estimate_period(uint16_t * v, uint32_t* t, uint16_t size, uint8_t tolerance) {  
	for (int i = 0; i < size - 1; ++i) {  
		for (int j = i + 4; j < size; ++j) {  
			if (ABS(v[i] - v[j]) < tolerance) {  
				// 找到匹配，计算周期  
				uint32_t time_diff = t[j] - t[i];  
				if(time_diff > 100000) return 0;
				return ABS(time_diff);
			}  
		}  
   }
}

void ADC12_0_INST_IRQHandler(void){	//ADC的中断函数
	switch (DL_ADC12_getPendingInterrupt(ADC12_0_INST)) {
		case DL_ADC12_IIDX_MEM0_RESULT_LOADED://如果MEM0（对应通道0）中有数据写入，则存储其数据进入数组adcdata
			gCheckADC = true;	
		
			// 动态添加数据点到波形，实现循环覆盖
			if (waveform.size < MAX_DATA_POINTS) {  
				waveform.voltage[waveform.index] = DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_0)*20/23; 
				waveform.time[waveform.index] = utick;
				waveform.size++;  
			} 
			else {  // 如果缓冲区已满，则覆盖最旧的数据  
				waveform.voltage[waveform.index] = DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_0)*20/23;
				waveform.time[waveform.index] = utick;
				waveform.index = (waveform.index + 1) % MAX_DATA_POINTS;  
			}
			//if(down&&Judgeup(waveform.index)) mintick = waveform.time[waveform.index - 5],down = 0,up = 1;
			//if(up&&Judgedown(waveform.index)) maxtick = waveform.time[waveform.index - 5],down = 1,up = 0;
			//adcData[Tcnt] = DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_0)*20/23;
			/*if(Vmax < adcData[Tcnt] - 50){
				Vmax = adcData[Tcnt];maxtick = utick;
			}
			if(Vmin > adcData[Tcnt] + 50){
				Vmin = adcData[Tcnt];mintick = utick;
			}
			++Tcnt;
			if (Tcnt >= 400) Tcnt = 0;
			*/
			break;
		default:
			break;
	}
}
void SysTick_Handler(){
	//Milisecond Source
	SysTick->CTRL &= ~(1<<16);
	++utick;
	if(utick >= 4294967295ul) utick = 0;
}
uint32_t findPeriod(WaveformData * wfd,uint16_t threshold){
	uint32_t minP = 1145141888;
	for(uint16_t i = 0;i < wfd->size - 1;++i){
		if(ABS(wfd->voltage[i] - wfd->voltage[i + 1] > threshold)){
			for(uint16_t j = i + 1;j < wfd->size - 1; ++j){
				if(ABS(wfd->voltage[j] - wfd->voltage[j + 1]) > threshold 
					&&ABS(wfd->time[i] - wfd->time[j]) > minP 
					&&ABS(wfd->voltage[i] - wfd->voltage[j]) < threshold){
						uint32_t period = ABS(wfd->time[j] - wfd->time[i]);  
						if (period < minP)  minP = period;  
				}
			}
		}
	}
	return (minP == 1145141888) ? 0 : minP;
}
// 计算占空比  
uint16_t calculateDutyCycle(WaveformData * wfd, uint32_t period, uint16_t threshold) {  
    uint32_t highTime = 0;  
    for (uint16_t i = 0; i < wfd->size; ++i) {  
        // 查找高电平的开始  
        if (wfd->voltage[i] > threshold) {  
            int start = i;  
            // 查找高电平的结束  
            while (i < wfd->size && wfd->voltage[i + 1] > threshold) {  
                ++i;  
            }  
            // 计算高电平时间并累加到highTime  
            highTime += (wfd->time[i] - wfd->time[start]);  
        }  
    }  
    // 计算占空比  
    return (highTime / period);  
}
/*
// 查找周期  
double findPeriod(SignalPoint *points, int size, double threshold) {  
    // 首先对时间戳进行排序  
    qsort(points, size, sizeof(SignalPoint), compareSignalPoints);  
  
    // 遍历数据点以找到可能的周期  
    double minPeriod = INFINITY;  
    for (int i = 1; i < size - 1; ++i) {  
        if (fabs(points[i].voltage - points[i + 1].voltage) > threshold) {  
            // 电压变化超过阈值，可能是周期的开始  
            for (int j = i + 1; j < size - 1; ++j) {  
                if (fabs(points[j].voltage - points[j + 1].voltage) > threshold &&  
                    fabs(points[i].timestamp - points[j].timestamp) > minPeriod &&  
                    fabs(points[i].voltage - points[j].voltage) < threshold) {  
                    // 找到了另一个电压变化，且时间差大于当前最小周期，且电压值相近  
                    double period = fabs(points[j].timestamp - points[i].timestamp);  
                    if (period < minPeriod) {  
                        minPeriod = period;  
                    }  
                }  
            }  
        }  
    }  
    return (minPeriod == INFINITY) ? 0 : minPeriod; // 如果没有找到周期，则返回0  
}  
  
// 计算占空比  
double calculateDutyCycle(SignalPoint *points, int size, double period, double threshold) {  
    double highTime = 0;  
    for (int i = 0; i < size; ++i) {  
        // 查找高电平的开始  
        if (points[i].voltage > threshold) {  
            int start = i;  
            // 查找高电平的结束  
            while (i < size - 1 && points[i + 1].voltage > threshold) {  
                ++i;  
            }  
            // 计算高电平时间并累加到highTime  
            highTime += (points[i].timestamp - points[start].timestamp);  
        }  
    }  
    // 计算占空比  
    return (highTime / period);  
}
*/