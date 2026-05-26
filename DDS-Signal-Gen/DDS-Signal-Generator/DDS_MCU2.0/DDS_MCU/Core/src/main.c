#include "eeprom_emulation_type_a.h"
#include "ti_msp_dl_config.h"
#include "oled.h"
//   #define  ADDRESS	(0x8000)
//	 EEPROM_TypeA_readData(saved);  读取之前储存的数据
void mode2read();
void f_big2read();
void f_small2read();
void A2read();
int KEY();                                                                //获取按键值

char read[12];
int pressed = 0;                                                          //确认是否按下按键
int num = 0;                                                              //确认被按下的按键
int x = 0;                                                                //数字横坐标
int sum = 0;                                                              //计算当前数字和
int menu = 0;                                                             //menu = 0界面0，menu = 1界面1

int mode = 0;int f_big = 0;int f_small = 0;int A = 0;

int main()
{  
	 read[8]=0x00;  read[9]=0x00;  read[10]=0x80;  read[11]=0x7F;      // 先加入永远不变的帧尾

   SYSCFG_DL_init();  //老三样
	 OLED_Init();
	 OLED_Clear();
   NVIC_ClearPendingIRQ(UART1_INT_IRQn);  
	 NVIC_EnableIRQ(UART1_INT_IRQn);
	while(1)
	{ 
		if(menu == 0){
		  num = KEY();
			if(num){
			  if(num == 1) sum = sum * 10 + 7;
			  if(num == 2) sum = sum * 10 + 8;
				if(num == 3)  sum = sum * 10 + 9;
				if(num == 5)  sum = sum * 10 + 4;
				if(num == 6)  sum = sum * 10 + 5;
				if(num == 7)  sum = sum * 10 + 6;
				if(num == 9)  sum = sum * 10 + 1;
				if(num == 10)  sum = sum * 10 + 2;
				if(num == 11)  sum = sum * 10 + 3;
				if(num == 14)  sum = sum * 10 + 0;
				if(num == 13) sum = 0;
			  if(num == 15) menu = !menu;
				if(num == 4) mode = sum,sum = 0;
				if(num == 8) A = sum,sum = 0;
				if(num == 12) f_big = sum,sum = 0;
				if(num == 16) f_small = sum,sum = 0;
			}
			OLED_ShowString(0,0,"set:");
		  OLED_ShowNum(32,0,sum,10,16);
			OLED_ShowString(0,2,"mod:");
			OLED_ShowNum(40,2,mode,3,16);
			OLED_ShowString(70,2,"A:");
			OLED_ShowNum(86,2,A,5,16);
			OLED_ShowString(0,4,"f_b:");
			OLED_ShowNum(48,4,f_big,10,16);
			OLED_ShowString(0,6,"f_s:");
			OLED_ShowNum(48,6,f_small,10,16);
			
		}
		 
    if(menu == 1){
		  num = KEY();
			if(num){
			  if(num == 15) menu = !menu;
			  if(num == 4) sum = sum + 1000000;
				if(num == 1) sum = sum - 1000000;
				if(num == 8) sum = sum + 1000;
				if(num == 5) sum = sum - 1000;
				if(num == 12) sum = sum + 10;
				if(num == 9) sum = sum - 10;
				if(num == 13) sum = 0;
			}
		}
		 
    mode2read();
    f_big2read();
    f_small2read();
    A2read();
		
		for(uint8_t i=0;i<=11;i++){
		   DL_UART_Main_transmitDataBlocking(UART1,read[i]);
     }             
	}
}


//按键扫描
int KEY(){           //这个扫描函数结构很简单所以没把大括号格式化书写(空多了看起来不美观)
      int x = 0;
      if(pressed){
        delay_cycles(9600000);
				pressed = 0;}
	    DL_GPIO_clearPins(H_PORT,H_H1_PIN);
			DL_GPIO_setPins(H_PORT,H_H2_PIN);
			DL_GPIO_setPins(H_PORT,H_H3_PIN);
			DL_GPIO_setPins(H_PORT,H_H4_PIN);
			delay_cycles(320000);
			if(!DL_GPIO_readPins(V_PORT,V_V1_PIN)){
			x = 1;
			pressed = 1;}
			else if(!DL_GPIO_readPins(V_PORT,V_V2_PIN)){
			x = 2;
			pressed = 1;}			
			else if(!DL_GPIO_readPins(V_PORT,V_V3_PIN)){
			x = 3;
			pressed = 1;}
			else if(!DL_GPIO_readPins(V_PORT,V_V4_PIN)){
			x = 4;
			pressed = 1;}
			
			DL_GPIO_setPins(H_PORT,H_H1_PIN);
			DL_GPIO_clearPins(H_PORT,H_H2_PIN);
			DL_GPIO_setPins(H_PORT,H_H3_PIN);
			DL_GPIO_setPins(H_PORT,H_H4_PIN);
			delay_cycles(320000);
			if(!DL_GPIO_readPins(V_PORT,V_V1_PIN)){
			x = 5;
			pressed = 1;}
			else if(!DL_GPIO_readPins(V_PORT,V_V2_PIN)){
			x = 6;
			pressed = 1;}
			else if(!DL_GPIO_readPins(V_PORT,V_V3_PIN)){
			x = 7;
			pressed = 1;}
			else if(!DL_GPIO_readPins(V_PORT,V_V4_PIN)){
			x = 8;
			pressed = 1;}
			
			DL_GPIO_setPins(H_PORT,H_H1_PIN);
			DL_GPIO_setPins(H_PORT,H_H2_PIN);
			DL_GPIO_clearPins(H_PORT,H_H3_PIN);
			DL_GPIO_setPins(H_PORT,H_H4_PIN);
			delay_cycles(320000);
			if(!DL_GPIO_readPins(V_PORT,V_V1_PIN)){
			x = 9;
			pressed = 1;}
			else if(!DL_GPIO_readPins(V_PORT,V_V2_PIN)){
			x = 10;
			pressed = 1;}
			else if(!DL_GPIO_readPins(V_PORT,V_V3_PIN)){
			x = 11;
			pressed = 1;			}
			else if(!DL_GPIO_readPins(V_PORT,V_V4_PIN)){
			x = 12;
			pressed = 1;			}
			
			DL_GPIO_setPins(H_PORT,H_H1_PIN);
			DL_GPIO_setPins(H_PORT,H_H2_PIN);
			DL_GPIO_setPins(H_PORT,H_H3_PIN);
			DL_GPIO_clearPins(H_PORT,H_H4_PIN);
			delay_cycles(320000);
			if(!DL_GPIO_readPins(V_PORT,V_V1_PIN)){
			x = 13;
			pressed = 1;}
			else if(!DL_GPIO_readPins(V_PORT,V_V2_PIN)){
			x = 14;
			pressed = 1;}
			else if(!DL_GPIO_readPins(V_PORT,V_V3_PIN)){
			x = 15;
			pressed = 1;			}
			else if(!DL_GPIO_readPins(V_PORT,V_V4_PIN)){
			x = 16;
			pressed = 1;}
			
			return x;
}

void mode2read(){
  read[0] = mode;
}

void f_big2read(){
  int num = f_big;
	for(int i = 4 ;i>=1;i--) read[i] = num % 256,num /= 256;
}

void f_small2read(){
  int num = f_small;
	for(int i = 6 ;i>=5;i--) read[i] = num % 256,num /= 256;
}
	
void A2read(){
  read[7] = A;
}