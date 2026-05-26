#include "ti_msp_dl_config.h"
#include "oled_spi.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
/******************************kbk定义*********************/
u8 gear = 0;//测量档位
uint32_t f_cnt = 0;//50M时钟下，计数50M次中计数待测信号的个数
uint16_t voltage_test = 0;//测得的电压：注意，衰减了一倍
/*这三个变量可以改成别的，只要你把中断函数里面的赋值对象改成别的*/
/************************************
gear:命令MCU计算的档位格式
	直流模式：0000->衰减5倍
			 0001->不衰减
			 

	电阻模式：0000：100kΩ档
			 0001：10kΩ档
			 0010：1kΩ档
			 0011：100Ω档
			 0100：10Ω档


***********************************/
/*******************************************************/ 
#define DATA_POINTS 100
uint16_t zero_cnt = 0;//零点个数	
uint16_t voltage[DATA_POINTS];  
uint16_t freq[DATA_POINTS]; 
int index = 0;// 当前数据点的索引  
int size = 0;// 当前缓冲区中有效数据的数量
/************************变量定义***********************/
int key_value = 0;//矩阵键盘键值
int signal_mode = 0;//测量模式(默认为0，从1~5分别为电阻、电容、电感、直流电压、交流电压) 
float SV;//电平转换次数
float T;//波形周期

uint16_t F;//波形频率
uint16_t Vpp;//电平峰峰值
uint16_t Vmid;//电平中位数
uint16_t min_voltage = 65535;//最小数字电平信号初始值
uint16_t max_voltage = 0; //最大电平信号初始值

int average_voltage = 0;//平均电压值
//下面写通信用的各种变量

bool gear_flag = 0;//档位切换标志位
/*******************************************************/

/************************函数定义***********************/
void SignalModeShow();//信号模式显示函数
void OLEDShow();//显示控制函数
void delay_ms(unsigned int ms);//延时函数
int Key();//矩阵键盘函数

void Gear_R();//电阻单位显示函数
void Gear_F();//电容单位显示函数
void Gear_H();//电感单位显示函数
void Gear_DCV();//直流单位显示计算函数
void Gear_ACV();//交流电压单位显示函数

//计算函数的相应变量附在计算函数定义旁边
void Calc_R();//电阻计算函数`
	float resist;//电阻计算结果
void Calc_F();//电容计算函数
	 int f_R1,f_R2;
	 double capacit;//电容计算结果
void Calc_H();//电感计算函数
	 int h_C1,h_C2;
	 float induct;//电感计算结果
void Calc_DCV();//直流电压计算函数
	 float DCV;//直流电压计算结果
void Calc_ACV();//交流电压计算函数
	 float ACV;//交流电压计算结果
void Calculate_Ave_V();//平均值计算
void OLED_ShowFloat(u8 x, u8 y, float num, u8 decimal_places) {
    // Buffer to hold the formatted number as a string
    char buffer[16];
    
    // 将数字num格式化为字符串并存储在buffer中。
    char format[10];
    snprintf(format, sizeof(format), "%%.%df", decimal_places);
    
    // 生成格式字符串，例如%.2f表示保留两位小数。
    snprintf(buffer, sizeof(buffer), format, num);
    
    // Display the formatted string on the OLED screen
    OLED_ShowString(x, y, buffer);
}
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
				DL_UART_transmitData(UART1,0b00000011);
				
				break;
			case 2://电容测量
				OLED_Clear();
				signal_mode = 2;//切换为电容测量模式
				DL_UART_transmitData(UART1,0b00000100);
				
				break;
			case 3://电感测量
				OLED_Clear();
				signal_mode = 3;//切换为电感测量模式	
				DL_UART_transmitData(UART1,0b00000101);
				
				break;
			case 4://直流电压测量
				OLED_Clear();
				signal_mode = 4;//切换为直流电压测量模式
				
				DL_UART_transmitData(UART1,0b00000010);
				break;
			case 5://交流电压测量
				OLED_Clear();
				signal_mode = 5;//切换为交流电压测量模式
				
				DL_UART_transmitData(UART1,0b00000001);
				break;
			case 16:
				OLED_Clear();
				signal_mode = 0;      	                                                                               
				break;
			default:
				break;
		}
		if (size < DATA_POINTS) { 
			// 动态添加数据点到波形，实现循环覆盖 
			voltage[index] = voltage_test; 
			index = (index + 1) % DATA_POINTS;
			size++;  
		}
		else {  
			// 如果缓冲区已满，则覆盖最旧的数据  
			voltage[index] = voltage_test;
			index = (index + 1) % DATA_POINTS;  
		}
		Calculate_Ave_V();//计算平均电压值
		OLEDShow();
		zero_cnt = 0; // 用于计算零点的个数
	}
}


void OLEDShow(){//OLED显示函数
	OLED_ShowString(4,0,"Mode:");
	//显示当前电压平均值
	OLED_ShowString(4,4,"Ave_V:");
	if(zero_cnt > 40 ){
		OLED_ShowString(64,4,"OL    ");
	}
	else{
		OLED_ShowNum(64,4,average_voltage,5,16);
	}
	//显示当前输入的频率
	OLED_ShowString(4,6,"Freq:");
	OLED_ShowNum(64,6,f_cnt,6,16);
}

void SignalModeShow(){//测量模式显示
	switch (signal_mode){
		case 0:
			OLED_ShowString(72,2,"\\");
			OLED_ShowString(48,0,"StandBy");
			break;
		case 1://电阻
			OLED_ShowString(48,0,"Resist");
			Gear_R();
			Calc_R();
			break;
		case 2://电容
			OLED_ShowString(48,0,"Capacity");
			Gear_F();
			Calc_F();
			break;
		case 3://电感
			OLED_ShowString(48,0,"Induct");
			Gear_H();
			Calc_H();
			break;
		case 4://直流电压
			OLED_ShowString(48,0,"DCV");
			Gear_DCV();
			Calc_DCV();
			break;
		case 5://交流电压
			OLED_ShowString(48,0,"ACV");
			Gear_ACV();
			Calc_ACV();
			break;
	default:
		break;
	}
}
//计算函数
void Calc_R(){
	float resist = 0;
	switch (gear){
		case 0://100kΩ档
			if(!(gear_flag || voltage_test >2053)){
				if( average_voltage < 1320){
					resist = 8635.7 * pow(2.7182818,0.0023*average_voltage);
				}
				else if(average_voltage > 1320 && average_voltage <= 1700){
					resist = 627.37*pow(2.7182818,0.0039*average_voltage);
				}
				else if(average_voltage > 1700){
					resist = 135837*pow(2.7182818,0.001*average_voltage);
				}
				resist = resist / 100000;
			}
			
			break;
		case 1://10kΩ档
			if(!(gear_flag || voltage_test >1900)){
				resist = 982.28 * pow(2.7182818,0.0021*average_voltage);
				resist = resist / 10000;
			}
			break;
		case 2://1kΩ档
			if(!(gear_flag || voltage_test >1900)){
				resist = 103.63 * pow(2.7182818,0.0021*average_voltage);
				resist = resist / 1000;
			}
			break;
		case 4://100Ω档
			if(!(gear_flag || voltage_test >1900)){
				// resist = 0.0000009*pow(average_voltage,3) ;
				// resist = resist  - 0.0031*pow(average_voltage,2);
				// resist = resist  + 3.5803*average_voltage;
				// resist = resist - 1356;
				// resist = resist / 100;
				resist = 10.782*pow(2.7182818,0.0021*average_voltage);
				resist = resist / 100;
			}
			break;
		case 8://10Ω档
			if(!(gear_flag || voltage_test >1900)){
				resist = 1.8326*pow(2.7182818,0.0022*average_voltage);
				resist = resist / 10;
			}
			break;
		default:
			break;
	}
	OLED_ShowFloat(4,2,resist,3);
	OLED_ShowString(52,2,"   ");
}

void Calc_F(){
	//测量电容
	capacit = 1700.6 * pow(f_cnt, -0.947);
	OLED_ShowFloat(4,2,capacit,3);
}

void Calc_H(){
	// Calculate inductance
	induct = 16.782 * pow(2.7182818, -2*pow(10,-5)*f_cnt);
	OLED_ShowFloat(4,2,induct,3);
}

void Calc_DCV(){
	//计算直流
	switch (gear){
		case 1:// 3v档
			DCV = -0.025*average_voltage + 5.1737;
			break;
		default:// 15v档
			DCV = -1*pow(10,-6)*pow(average_voltage,2);
			DCV = DCV - 0.0229*average_voltage + 53.134;
			break;
	}
	OLED_ShowFloat(4,2,DCV,3);
}

void Calc_ACV(){
	//计算交流
	ACV = -0.0022*average_voltage + 4.6654;
	OLED_ShowFloat(4,2,ACV,3);
}
//以下为档位显示函数
void Gear_R(){
	switch (gear){
		case 0:
			OLED_ShowString(72,2," *100kR");
			break;
		case 1:
			OLED_ShowString(72,2," *10kR ");
			break;
		case 2:
			OLED_ShowString(72,2," *1kR  ");
			break;
		case 4:
			OLED_ShowString(72,2," *100R");
			break;
		case 8:
			OLED_ShowString(72,2," *10R ");
			break;
		default:
			OLED_ShowString(72,2," *R  ");
			break;
	}
	if(gear_flag){
		size = 0;
		gear_flag = 0;
		OLED_ShowString(80,2,"      ");
	}
}

void Gear_F(){
	OLED_ShowString(100,2,"nF  ");
}

void Gear_H(){
	OLED_ShowString(100,2,"mH  ");
}

void Gear_DCV(){
	switch (gear){
		case 1:
			OLED_ShowString(72,2,"V    ");
			break;
		default:
			OLED_ShowString(72,2,"*0.2V");
			break;
	}
	
}

void Gear_ACV(){
	OLED_ShowString(100,2,"mV  ");
}




// 计算并返回波形的平均值，并忽略最大和最小值  
void Calculate_Ave_V() {  
	uint32_t sum_voltage = 0; // 用于累加电压值的和
	uint16_t idx = (index + DATA_POINTS - size) % DATA_POINTS;
	for (uint16_t i = 0; i < size; i++) {
		if(voltage[idx] <= 300 || voltage[idx] >= 2054) 
			++zero_cnt; // 计算零点的个数
		else{
			sum_voltage += voltage[idx]; // 累加电压值
		}
		idx = (idx + 1) % DATA_POINTS;
	}
	average_voltage = sum_voltage / (size - zero_cnt); // 计算电压平均值
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
			SignalModeShow();
			break;
		default:
			break;
	}
}
/******************************************
此函数为KBK写
注意这个函数的接口，需要在case 9的if（data==0xff）里面的修改左值为需要的赋值对象

同步序列(情况0-2)：
	它首先查找三个特定字节的序列：0xCC 、0xDD和0xEE。该序列指示有效数据包的开始。
提取模式和电压(情况3-4)：
	在同步序列之后，下一个字节在其高4位中包含模式，在其低4位中包含电压的高4位。
	以下字节通过提供低8位来完成电压值。
提取频率计数器(情况5-8)：
	接下来的四个字节表示频率计数器(temp_f_cnt)
	每个字节都添加到temp_f_cnt，然后将值向左移动，
	为下一个字节腾出空间
验证结束字节并赋值(用例9)：
	最后一个字节应该是 0xFF 。
	如果是，则将提取的值(temp_mode、temp_Voltage和temp_f_cnt)
	分配给各个全局变量(mode、voltage和f_cnt)。
	所有的临时变量和计数器都被重置
	如果最后一个字节不是 0xFF，则该函数将计数器和temp_f_cnt重置为零，丢弃数据包
*******************************************/
void UART1_IRQHandler(void){
	 // 静态变量，用于在多次调用之间保持状态
	static u8 cnt=0,temp_mode=0;
	static uint16_t temp_Voltage=0;
	static uint32_t temp_f_cnt=0;
	// 从UART1接收一个字节的数据
	u8 data=DL_UART_receiveDataBlocking(UART1);
	switch(cnt){
		// 情况0-2：检查同步序列0xCC, 0xDD, 0xEE
		case 0:
			if(data!=0xcc)
				cnt=0;// 如果不匹配则重置计数器
			else
				cnt++;
			break;
		case 1:
			if(data!=0xdd)
				cnt=0;// 如果不匹配则重置计数器
			else
				cnt++;
			break;
		case 2:
			if(data!=0xee)
				cnt=0;// 如果不匹配则重置计数器
			else
				cnt++;
			break;
		// 情况3：提取模式和电压的高4位
		case 3:
			temp_mode = data>>4;
			// 数据的低4位左移到temp_Voltage的高8位
			temp_Voltage = (0b0000000000001111 & (uint16_t)data)<<8;
			cnt++;
			break;
		// 情况4：完成电压的提取
		case 4:
			temp_Voltage += data;// 添加电压的低8位
			cnt++;
			break;
		//情况5-7：逐字节提取频率计数器（temp_f_cnt）
		case 5:
		case 6:
		case 7:
			temp_f_cnt += data;// 添加字节到temp_f_cnt
			temp_f_cnt <<= 8; // 左移为下一个字节腾出空间
			cnt++;
			break;
		 // 情况8：频率计数器的最后一个字节
		case 8:
			temp_f_cnt += data;
			cnt++;
			break;
		 // 情况9：验证结束字节并赋值
		case 9:
			if(data == 0xff){
				// 将提取的值赋值给全局变量
				if(gear!=temp_mode){
					gear=temp_mode;
					gear_flag = 1;
				}

				voltage_test =temp_Voltage;
				f_cnt=temp_f_cnt;
				// 重置临时变量和计数器
				temp_mode=temp_Voltage=temp_f_cnt=cnt=0;
			}
			else{
				// 如果结束字节不是0xff则重置
				temp_f_cnt=0;
				cnt=0;
			}
	}
	
	return;
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

