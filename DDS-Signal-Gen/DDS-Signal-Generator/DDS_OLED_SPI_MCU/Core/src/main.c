#include "ti_msp_dl_config.h"
#include "oled_spi.h"
#include <stdio.h>
#define BUFFER_CAPA 12
/***********************变量定义**************************/
int menu = 1;//菜单页面函数
int key_value;
int wave_form = 4;//波形选择
//矩阵键盘值
//波形设置: 1.正弦波 2.方波 3.三角波
uint32_t frequency = 1000;//波形频率(1kHz - 10MHz)
u8 modulation = 0;//调制设置: 1. AM  2.FM
uint16_t modulation_frequency = 1000;//调制频率(1kHz-5kHz)
uint8_t modulation_coefficient = 0;//调制系数(30%-100%)或1-5

/********************************************************/
/*以下变量由kbk声明*/
char buffer[BUFFER_CAPA];//数据缓冲区，用于将存储发送的数据,BUFFER_CAPA是宏定义的大小
/****************************************/
void delay_ms(unsigned int ms)
{                         
	unsigned int a;
	ms*=10;
	while(ms)
	{
		a=360;
		while(a--);
		ms--;
	}
	return;
}
/***********************函数声明**************************/
int Key();//矩阵键盘
void Limit();//限制器模块
void ModulationInit();//调制变量初始化
void OLEDShow();//OLED显示模块
void WaveFormShow();//波形显示模块
int NumInput();//限制最高位输入模块
int MatKeyToNum(int input);//矩阵键盘对应数字转换模块
uint32_t FrequencyInput();//频率输入转换模块
uint16_t ModulationFrequencyInput();//调制频率输入转换模块
uint8_t ModulationCoefficientInput();//调制系数输入转换模块
/*以下函数均为kbk添加*/
void to_byte(char* space,u8 byte_num,u8 capa,uint32_t data);//space是要储存到的位置，byte_num是该数据所占的字节数,capa是数组的容量
void send_data(void);//这个函数发送数据
/********************************************************/


/*******************************************/

int main(void){
    SYSCFG_DL_init(); // Initialize the device
	
	DL_TimerG_startCounter(TIMER_0_INST);//启动计时器
	NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);//中断函数启动
	
	NVIC_ClearPendingIRQ(UART1_INT_IRQn);//UART清除中断标志位  
	NVIC_EnableIRQ(UART1_INT_IRQn);//UART中断使能
	
	OLED_Init();//初始OLED
	OLED_Clear();//清除缓存
	delay_ms(10);//延时
	//加载界面
	OLED_ShowString(48,0,"DDS");
	OLED_ShowString(16,4,"Loading......");
	delay_ms(500);//显示时间
	OLED_Clear();
	
	wave_form = 1;
	while(1){
		//获取按键输入值
		key_value = Key();
		//**********************给我好好读完这里啊*******************************//
		//菜单规划设置
		//	·第一行左三个为波形设置，分别对应正弦、方、三角波
		//	·最右边一列的上面两个为AM,FM的模式选择
		//	·左下角九个和最右下角一个为数据输入 
		//	·S15该按键在数据输入时解锁，按下后将会结束数据输入，并保留当前输入数据(不代表最后输出数据，该输入数据仍会受限制函数影响)
		//********************************************************************//
		//菜单逻辑设置:
		//	先选择一个波形输出，选择后解锁数据输入区域
		//	调制时先选择调制类型，再选择载波波形并设置频率，
		//	接下来设置调制频率和调制系数
		//   所有项目设置完后显示一秒
		//********************************************************************//
		//UART发送注意事项:
		//	相关变量: 
		//		frequency,wave_form,
		// 		modulation,
		// 		isTransmit
		// 		modulation_frequency,
		//		modulation_coefficient
		//********************************************************************//
		switch (key_value){
			case 1://以此为例对相似代码结构说明
				OLED_Clear();
				wave_form = 1;//调整波形种类
				OLED_ShowString(48,0,"Sine");
				frequency = FrequencyInput();//输入频率
				//调制变量恢复初始值
				ModulationInit();
				OLED_Clear();
			 	wave_form = 1;//调整波形种类，防止其变化
				break;
			case 2:
				OLED_Clear();
				wave_form = 2;
				OLED_ShowString(48,0,"Square");
				frequency = FrequencyInput();
				//调制变量恢复初始值
				ModulationInit();
				OLED_Clear();
				wave_form = 2;
				break;
			case 3:
				OLED_Clear();
				wave_form = 3;
				OLED_ShowString(48,0,"Triangle");
				frequency = FrequencyInput();
				//调制变量恢复初始值
				ModulationInit();
				OLED_Clear();
				wave_form = 3;	
			break;
			case 4:
				modulation = 1;
				//载波波形
				OLED_Clear();
				OLED_ShowString(2,0,"Waveform:");
				OLED_ShowString(2,2,"Choose one of");
				OLED_ShowString(2,4,"the wave forms.");
				do{
					key_value = Key();
				} while (key_value>3 || key_value == 0);
				wave_form = key_value;
				
				//载波信号频率
				OLED_Clear();
				WaveFormShow();//载波波形显示
				OLED_ShowString(48,0,"AM");
				frequency = FrequencyInput();

				//调制信号频率
				OLED_Clear();
				WaveFormShow();//载波波形显示
				OLED_ShowString(48,0,"AM");
				modulation_frequency = ModulationFrequencyInput();

				//调制系数
				OLED_Clear();
				WaveFormShow();//载波波形显示
				OLED_ShowString(48,0,"AM");
				modulation_coefficient = ModulationCoefficientInput();
				OLED_Clear();
				break;
			case 8:
				OLED_Clear();
				modulation = 2;
				//载波波形
				OLED_ShowString(2,0,"Waveform:");
				OLED_ShowString(2,2,"Choose one of");
				OLED_ShowString(2,4,"the wave forms.");
				do{
					key_value = Key();
				} while (key_value>3 || key_value == 0);
				wave_form = key_value;
				
				//载波信号频率
				OLED_Clear();
				WaveFormShow();//载波波形显示
				OLED_ShowString(48,0,"FM");
				frequency = FrequencyInput();
				
				//调制信号频率
				OLED_Clear();
				WaveFormShow();//载波波形显示
				OLED_ShowString(48,0,"FM");
				modulation_frequency = ModulationFrequencyInput();

				//调制系数
				OLED_Clear();
				WaveFormShow();//载波波形显示
				OLED_ShowString(48,0,"FM");
				modulation_coefficient = ModulationCoefficientInput();
				OLED_Clear();
				break;
			default:
				break;
		}
		//刷新OLED
		Limit();
		send_data();
		//isTransmit = true;//记得写uart的时候每次发送完要把这个变为false		
		OLEDShow();
	}
}

//中断函数
void TIMER_0_INST_IRQHandler (void){
	switch( DL_TimerG_getPendingInterrupt (TIMER_0_INST)){
		case DL_TIMERG_IIDX_ZERO:
			
			DL_GPIO_togglePins(LED_PORT,LED_LED0_PIN);//1s的定时器系统,顺便检测系统是否正常运行
			break;
		default:
			break;
	}
}

void ModulationInit(){//调制变量初始化
	modulation = 0;
	modulation_frequency = 1000;
	modulation_coefficient = 0;
}

void WaveFormShow(){//波形种类显示模块
	switch (wave_form){
		case 1:
			OLED_ShowString(72,0,"Sine");
			break;
		case 2:
			OLED_ShowString(72,0,"Square");
			break;
		case 3:
			OLED_ShowString(72,0,"Tri");
			break;
	default:
		break;
	}
}

void Limit(){//限制变量范围
	
	if(!modulation){//正常输出时的限制条件
		//限制频率
		if(frequency < 1000)frequency = 1000;
		else if(frequency >1000000)frequency = 1000000;
	}
	else if(modulation == 1){//AM输出的限制条件
		//限制频率
		if(frequency < 5000)frequency = 5000;
		else if(frequency > 10000000)frequency = 10000000;
		//限制调制频率
		if(modulation_frequency <1000)modulation_frequency =1000;
		else if(modulation_frequency >5000)modulation_frequency =5000;
		//限制调制系数30%-100%
		if(modulation_coefficient <30)modulation_coefficient =30;
		else if(modulation_coefficient >100)modulation_coefficient = 100;
	}
	else if(modulation == 2){//FM输出的限制条件
		//限制频率
		if(frequency < 5000)frequency = 5000;
		else if(frequency > 10000000)frequency = 10000000;
		//限制调制频率
		if(modulation_frequency <1000)modulation_frequency =1000;
		else if(modulation_frequency >5000)modulation_frequency =5000;
		//限制调制系数1-5(因为要求为分辨率0.1，因此限制为10-50，需进行缩放)
		if(modulation_coefficient <10)modulation_coefficient =10;
		else if(modulation_coefficient >50)modulation_coefficient = 50;
	}
}

void OLEDShow(){//OLED显示模块
	//波形显示
	OLED_ShowString(2,0,"Waveform:");
	WaveFormShow();
	//调幅/调频以及其系数显示
	if(modulation)OLED_ShowString(2,6,"Modu:");
	if(modulation){
		OLED_ShowString(16,4,"M_F:");
		OLED_ShowNum(64,4,modulation_frequency,4,16);
		OLED_ShowString(96,4,"Hz");
	}
	switch (modulation){
		case 1:
			OLED_ShowString(48,6,"AM");
			OLED_ShowNum(72,6,modulation_coefficient,3,16);
			OLED_ShowString(98,6,"%");
			break;
		case 2:
			OLED_ShowString(48,6,"FM");
			OLED_ShowNum(80,6,(modulation_coefficient/10)%10,1,16);
			OLED_ShowNum(96,6,modulation_coefficient,1,16);
			OLED_ShowString(88,6,".");
			break;
	default:
		break;
	}
	//频率显示
	OLED_ShowString(16,2,"F:");
	OLED_ShowNum(32,2,frequency,8,16);
	OLED_ShowString(96,2,"Hz");
}

uint8_t ModulationCoefficientInput(){//调制波系数输入
	uint8_t temp_freq = 0;
	uint8_t temp = 0;
	int temp_key_value;
	int i ;
	OLED_ShowString(2,2,"Input  Coef:");
	for(i = 0;i<3;i++){
		//数字输入模块
		temp_key_value = NumInput();
		//提前结束确认模块
		if(temp_key_value == 12){
			OLED_ShowString(2,6,"Ensure!");
			break;
		}
		//矩阵键盘对应数字转换
		temp = MatKeyToNum(temp_key_value);
		//频率累加计算
		temp_freq = temp + temp_freq*10;
		OLED_ShowNum(10,4,temp_freq,3,16);
		if(modulation == 1)OLED_ShowString(38,4,"%");
		else OLED_ShowString(42,4,"*10^-2");
	}
	delay_ms(1000);
	return temp_freq;
}

uint16_t ModulationFrequencyInput(){//调制波频率输入
	uint16_t temp_freq = 0;
	uint16_t temp = 0;
	int temp_key_value;
	int i ;
	OLED_ShowString(2,2,"Input M_freq:");
	for(i = 0;i<4;i++){
		//数字输入模块
		temp_key_value = NumInput();
		//提前结束确认模块
		if(temp_key_value == 12){
			OLED_ShowString(2,6,"Ensure!");
			break;
		}
		//矩阵键盘对应数字转换
		temp = MatKeyToNum(temp_key_value);
		//频率累加计算	
		temp_freq = temp + temp_freq*10;
		OLED_ShowNum(20,4,temp_freq,4,16);
		OLED_ShowString(64,4,"Hz");
	}
	delay_ms(1000);
	return temp_freq;
}

uint32_t FrequencyInput(){//频率输入
	uint32_t temp_freq = 0;
	uint32_t temp = 0;
	int temp_key_value;
	int i ;
	OLED_ShowString(2,2,"Input Freq:");
	for(i = 0;i<8;i++){
		//数字输入模块
		temp_key_value = NumInput();
		//提前结束确认模块
		if(temp_key_value == 12){
			OLED_ShowString(2,6,"Ensure!");
			break;
		}
		//矩阵键盘对应数字转换
		temp = MatKeyToNum(temp_key_value);
		//频率累加计算
		temp_freq = temp + temp_freq*10;
		OLED_ShowNum(2,4,temp_freq,8,16);
		OLED_ShowString(90,4,"Hz");
	}
	delay_ms(1000);
	return temp_freq;
}

int NumInput(){//数字输入模块
	int temp_key_value = 0;
	do{
		temp_key_value = Key();		
		if((0 < temp_key_value && temp_key_value<5)|| temp_key_value == 8 ){//不按数字键就确认
			return 12;
		}
	}while(temp_key_value < 5);

	return temp_key_value;
}
int MatKeyToNum(int input){//矩阵键盘对应数字转换
	int temp = 0;
	switch (input){	
		case 5:temp = 1;break;
		case 6:temp = 2; break;
		case 7:temp = 3;break;
		case 9:temp = 4;break;
		case 10:temp = 5;break;
		case 11:temp = 6;break;
		case 12:temp = 12;break;//确定输入控制信号
		case 13:temp = 7;break;
		case 14:temp = 8;break;
		case 15:temp = 9;break;
		case 16:temp = 0;break;
		default:break;
	}
	return temp;
}
/*下面两个函数由kbk编写*/
void to_byte(char* space,u8 byte_num,u8 capa,uint32_t data){//space是要储存到的位置，byte_num是该数据所占的字节数,capa是数组的容量
	static u8 cnt = 0;//cnt是已经存入数据的个数 
	char* p = (char*) & data;
	for(u8 i = 0;i < byte_num;i++){
		space[cnt] = * p;
		p++;
		cnt = (cnt + 1 ) % capa;
	}
}
void send_data(void){
/*在verilog代码里面，mode占4位，一个byte就够了，
	其中高两位是：
				00：不调制
				01：AM
				02: FM
				低两位：
				00: 没有波形
				01：sine
				10: 三角
				11：方波
	wave_form+4*modulation得到传输数据
*/
	to_byte(buffer,1,BUFFER_CAPA,(wave_form == 4 ? 0 : wave_form) + 4 * modulation);//存储mode
	to_byte(buffer,4,BUFFER_CAPA,frequency);//存储frequency
	to_byte(buffer,2,BUFFER_CAPA,modulation_frequency);//存储modulation_frequency
	to_byte(buffer,1,BUFFER_CAPA,modulation_coefficient);//存储modulation_coefficient
	to_byte(buffer,4,BUFFER_CAPA,0x7f800000);//存储尾帧
	//连续发送信号
	for(u8 i = 0;i < BUFFER_CAPA;i++)
		DL_UART_Main_transmitDataBlocking(UART1,buffer[i]);
}

//矩阵键盘函数
int Key(){
	int key_num = 0;        // 按键1-16，默认为0
	static uint32_t key_flag = 0; // 按下按键标志

	if (key_flag){
		delay_ms(300); // 300ms延迟，防止按下一次按键却被认为按下了多次按键，导致得到了多个相同的按键 ?
		key_flag = 0;  // 按下按键标志清零
	}
	// 行扫描
	// ROW 0111
	DL_GPIO_clearPins(MAT_KEY_PORT, MAT_KEY_ROW0_PIN);
	DL_GPIO_setPins(MAT_KEY_PORT, MAT_KEY_ROW1_PIN | MAT_KEY_ROW2_PIN | MAT_KEY_ROW3_PIN);
	delay_ms(10); 
	if (!DL_GPIO_readPins(MAT_KEY_PORT, MAT_KEY_COL0_PIN)){
		key_num = 1;
		key_flag = 1; // 按键已按下
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
	delay_ms(10);
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
	return key_num;//返回值
}

