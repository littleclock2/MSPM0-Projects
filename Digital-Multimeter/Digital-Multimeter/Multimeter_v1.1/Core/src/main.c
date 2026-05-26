#include "ti_msp_dl_config.h"
#include "oled_spi.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/*******************************************************/ 
#define DATA_POINTS 400
typedef struct {  
    uint16_t voltage[DATA_POINTS];  
    uint32_t time[DATA_POINTS];  
    uint16_t index; // 当前数据点的索引  
    uint16_t size;  // 当前缓冲区中有效数据的数量  
} WaveformData;  
/************************变量定义************************/
int key_value = 0;//矩阵键盘键值
int signal_mode = 0;//测量模式(默认为0，从1~5分别为电阻、电容、电感、直流电压、交流电压) 
float SV;//电平转换次数
float T;//波形周期
int gear = 0;//测量档位
uint16_t F;//波形频率
uint16_t Vpp;//电平峰峰值
uint16_t Vmid;//电平中位数
uint16_t min_voltage = 65535;//最小数字电平信号初始值
uint16_t max_voltage = 0; //最大电平信号初始值
//下面写通信用的各种变量
WaveformData waveform;
/*******************************************************/

/************************函数定义************************/
void SignalModeShow();//信号模式显示函数
void OLEDShow();//显示控制函数
void delay_ms(unsigned int ms);//延时函数
int Key();//矩阵键盘函数
/*******************************************************/
int main(void){
    SYSCFG_DL_init(); //设备初始化
	
	DL_TimerG_startCounter(TIMER_0_INST);//时钟开始计时
	NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);//计时中断使能
	
	NVIC_ClearPendingIRQ(UART1_INT_IRQn);//UART使能
	NVIC_EnableIRQ(UART1_INT_IRQn);//UART中断使能
	
	OLED_Init();//OLED初始化
	OLED_Clear();//清屏
	delay_ms(10);//延时
	//启动界面
	OLED_ShowString(24,0,"Multimeter");
	OLED_ShowString(16,4,"Loading......");
	delay_ms(500);//显示0.5秒
	OLED_Clear();
	
	while(1){
		key_value = Key();//循环读取矩阵键盘输入
		switch (key_value){
			case 1://电阻测量
				OLED_Clear();
				signal_mode = 1;//切换为电阻测量模式
				Calc_R();
				break;
			case 2://电容测量
				OLED_Clear();
				signal_mode = 2;//切换为电容测量模式
				Calc_F();
				break;
			case 3://电感测量
				OLED_Clear();
				signal_mode = 3;//切换为电感测量模式	
				Calc_H();
				break;
			case 4://直流电压测量
				OLED_Clear();
				signal_mode = 4;//切换为直流电压测量模式
				Calc_DCV();
				break;
			case 5://交流电压测量
				OLED_Clear();
				signal_mode = 5;//切换为交流电压测量模式
				Calc_ACV();
				break;
			case 16:
				OLED_Clear();
				signal_mode = 0;      	                                                                               
				break;
			default:
				break;
		}	
		OLEDShow();
	}
}


void OLEDShow(){//OLED显示函数
	OLED_ShowString(4,0,"Mode:");
	SignalModeShow();
}

void SignalModeShow(){//测量模式显示
	switch (signal_mode){
		case 0:
			OLED_ShowString(72,0,"\\");
			OLED_ShowString(48,2,"StandBy");
			break;
		case 1://电阻
			OLED_ShowString(48,0,"Resist");
			Gear_R();
			break;
		case 2://电容
			OLED_ShowString(48,0,"Capacity");
			Gear_F();
			break;
		case 3://电感
			OLED_ShowString(48,0,"Induct");
			Gear_H();
			break;
		case 4://直流电压
			OLED_ShowString(48,0,"DCV");
			Gear_DCV();
			break;
		case 5://交流电压
			OLED_ShowString(48,0,"ACV");
			Gear_DCV();
			break;
	default:
		break;
	}
}

void Calc_R(){

}

void Calc_F(){

}

void Calc_H(){

}
void Calc_DCV(){

}
void Calc_ACV(){
	Vpp = Calculate_Vpp();
	Vmid = Vpp / 2;
	T = (SV * 2 * 1000 / 10000);//周期计算
	F = 1000/T;//频率计算
}

void Gear_R(){
	switch (gear){
		case 0:
			OLED_ShowString(0,2,"R");
			break;
		case 1:
			OLED_ShowString(0,2,"kR");
			break;
		default:
			break;
	}
}

void Gear_F(){
	OLED_ShowString(0,2,"pF");
}

void Gear_H(){
	switch (gear){
		case 0:
			OLED_ShowString(0,2,"uH");
			break;
		case 1:
			OLED_ShowString(0,2,"mH");
			break;
		default:
			break;
	}
}

void Gear_DCV(){
	OLED_ShowString(0,2,"V");
}

void Gear_ACV(){
	switch (gear){
		case 0:
			OLED_ShowString(0,2,"mV");
			break;
		case 1:
			OLED_ShowString(0,2,"V");
			break;
		default:
			break;
	}
}



void SV_Calc(){//计算采样区间内的电平转换次数
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

// 计算并返回波形的峰峰值  
uint16_t Calculate_Vpp() {  
    for (uint16_t i = 0; i < waveform.size; ++i) {  
        if (waveform.voltage[i] < min_voltage) {//历遍找最小值  
            min_voltage = waveform.voltage[i];
        }  
        if (waveform.voltage[i] > max_voltage) {//历遍找最大值    
            max_voltage = waveform.voltage[i];  
        }  
    }
    return (max_voltage - min_voltage)/4096.0*3.3*1000;//峰峰值后续处理  
}  

uint16_t Compare_Max(uint16_t a,uint16_t b){//返回比较大的值
	return a > b ? a : b;
}
uint16_t Compare_Min(uint16_t a,uint16_t b){//返回较小的值
	return a > b ? b : a;
}

float Sqrt(float n) {//开平方(牛顿迭代法)
    if (n < 0) return -1;  // 对于负数返回-1，因为平方根没有实数解
    const float err = 1e-5;   // 设置误差界限
    float t = n;
    while (t - n / t > err * t)
        t = (n / t + t) / 2.0;
    return t;
}

uint32_t Abs(uint32_t x){//绝对值计算函数
	if(x < 0) return -x;
	else return x;
}

void delay_ms(unsigned int ms){//延时函数                  
	unsigned int a;
	ms*=10;
	while(ms){
		a=360;
		while(a--);
		ms--;
	}
	return;
}

void TIMER_0_INST_IRQHandler (void){//中断函数(用于检测MCU是否正常运行)
	switch( DL_TimerG_getPendingInterrupt (TIMER_0_INST)){
		case DL_TIMERG_IIDX_ZERO:
			DL_GPIO_togglePins(LED_PORT,LED_LED0_PIN);
			break;
		default:
			break;
	}
}

//矩阵键盘函数
int Key(){
	int key_num = 0;//
	static uint32_t key_flag = 0;//按下标志位

	if (key_flag){
		delay_ms(200); //300ms消抖
		key_flag = 0;  //flag归0
	}
	//行扫描
	// ROW 0111
	DL_GPIO_clearPins(MAT_KEY_PORT, MAT_KEY_ROW0_PIN);
	DL_GPIO_setPins(MAT_KEY_PORT, MAT_KEY_ROW1_PIN | MAT_KEY_ROW2_PIN | MAT_KEY_ROW3_PIN);
	delay_ms(30); 
	if (!DL_GPIO_readPins(MAT_KEY_PORT, MAT_KEY_COL0_PIN)){
		key_num = 1;
		key_flag = 1; 
	}
	else if (!DL_GPIO_readPins(MAT_KEY_PORT, MAT_KEY_COL1_PIN)){
		key_num = 2;
		key_flag = 1;
	}
	else if (!DL_GPIO_readPins(MAT_KEY_PORT, MAT_KEY_COL2_PIN)){
		key_num = 3;
		key_flag = 1;
	}
	else if (!DL_GPIO_readPins(MAT_KEY_PORT, MAT_KEY_COL3_PIN)){
		key_num = 4;//(1,4)
		key_flag = 1;
	}
	// ROW 1011
	DL_GPIO_clearPins(MAT_KEY_PORT, MAT_KEY_ROW1_PIN);
	DL_GPIO_setPins(MAT_KEY_PORT, MAT_KEY_ROW0_PIN | MAT_KEY_ROW2_PIN | MAT_KEY_ROW3_PIN);
	delay_ms(30);
	if (!DL_GPIO_readPins(MAT_KEY_PORT, MAT_KEY_COL0_PIN)){
		key_num = 5;
		key_flag = 1;
	}
	else if (!DL_GPIO_readPins(MAT_KEY_PORT, MAT_KEY_COL1_PIN)){
		key_num =6;
		key_flag = 1;
	}
	else if (!DL_GPIO_readPins(MAT_KEY_PORT, MAT_KEY_COL2_PIN)){
		key_num = 7;
		key_flag = 1;
	}
	else if (!DL_GPIO_readPins(MAT_KEY_PORT, MAT_KEY_COL3_PIN)){
		key_num = 8;
		key_flag = 1;//(2,4)
	}
	// ROW 1101
	DL_GPIO_clearPins(MAT_KEY_PORT, MAT_KEY_ROW2_PIN);
	DL_GPIO_setPins(MAT_KEY_PORT, MAT_KEY_ROW0_PIN | MAT_KEY_ROW1_PIN | MAT_KEY_ROW3_PIN);
	delay_ms(10);
	if (!DL_GPIO_readPins(MAT_KEY_PORT, MAT_KEY_COL0_PIN)){
		key_num = 9;
		key_flag = 1;
	}
	else if (!DL_GPIO_readPins(MAT_KEY_PORT, MAT_KEY_COL1_PIN)){
		key_num = 10;
		key_flag = 1;
	}
	else if (!DL_GPIO_readPins(MAT_KEY_PORT, MAT_KEY_COL2_PIN)){
		key_num = 11;
		key_flag = 1;
	}
	else if (!DL_GPIO_readPins(MAT_KEY_PORT, MAT_KEY_COL3_PIN)){
		key_num = 12;
		key_flag = 1;//(3,4)
	}
	DL_GPIO_clearPins(MAT_KEY_PORT, MAT_KEY_ROW3_PIN);
	DL_GPIO_setPins(MAT_KEY_PORT, MAT_KEY_ROW0_PIN | MAT_KEY_ROW1_PIN | MAT_KEY_ROW2_PIN);
	delay_ms(10);
	// ROW 1110
	if (!DL_GPIO_readPins(MAT_KEY_PORT, MAT_KEY_COL0_PIN)){
		key_num = 13;
		key_flag = 1;
	}
	else if (!DL_GPIO_readPins(MAT_KEY_PORT, MAT_KEY_COL1_PIN)){
		key_num = 14;
		key_flag = 1;//(4,2)
	}
	else if (!DL_GPIO_readPins(MAT_KEY_PORT, MAT_KEY_COL2_PIN)){
		key_num = 15;
		key_flag = 1;//(4,3)
	}
	else if (!DL_GPIO_readPins(MAT_KEY_PORT, MAT_KEY_COL3_PIN)){
		key_num = 16;
		key_flag = 1;//(4,4)
	}
	return key_num;//返回按下的数字
}

