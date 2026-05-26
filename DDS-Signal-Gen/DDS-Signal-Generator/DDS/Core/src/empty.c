#include "ti_msp_dl_config.h"
#include "oled.h"
uint8_t TxPacket[4] = {0x90, 0x00, 0x00, 0x00};  //发送数据
uint8_t RxPacket[4]={0x00, 0x00, 0x00, 0x00};   //接收数据
uint8_t RxTemp; //临时数据，清空接收FIFO用

int main(void)
{
	SYSCFG_DL_init();                      //初始化
	OLED_Init();			//初始化OLED  
	OLED_Clear();
	OLED_ShowCHinese(0,0,3);//
	OLED_ShowCHinese(18,0,4);//
  OLED_ShowNum(0,2,100,3,18);				//
	OLED_ShowString(1,4,"2024.1.15");  //显示一个字符号串
  OLED_ShowString(2,6,"OLED TEST");//显示汉字
	//delay_ms(100000);this expression may be wrong! the sysclk doesn't recognize this function in right way!
	DL_Common_delayCycles(60000000);
	OLED_Clear();
	int row = 0;
  while (1) 
	{
		OLED_ShowString(3,row,"HELLO OLED");
		DL_Common_delayCycles(30000000);
		OLED_Clear();
		if(row <= 4)
			row += 2;
		else
			row = 0;
  }
}


