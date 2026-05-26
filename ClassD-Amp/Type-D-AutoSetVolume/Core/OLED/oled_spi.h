#ifndef __OLED_SPI__
#define __OLED_SPI__
#include <stdlib.h>
#include "ti_msp_dl_config.h"
#define OLED_CMD 0  // ????
#define OLED_DATA 1 // ????
#define OLED_MODE 0

#define  u8  unsigned char 
#define  u32 unsigned int 


//-----------------OLED????----------------
// ??SPI??????????????,??????
#define SPI_OLED_INST SPI_0_INST

//-----------------OLED¶Ë¿Ú¶¨Òå----------------

#define OLED_RES_Clr()  DL_GPIO_clearPins(OLED_PORT,OLED_RES_PIN)//RES
#define OLED_RES_Set()  DL_GPIO_setPins(OLED_PORT,OLED_RES_PIN)

#define OLED_DC_Clr()   DL_GPIO_clearPins(OLED_PORT,OLED_DC_PIN)//DC
#define OLED_DC_Set()   DL_GPIO_setPins(OLED_PORT,OLED_DC_PIN)

#define OLED_CS_Clr()   DL_GPIO_clearPins(OLED_PORT,OLED_CS_PIN)//CS
#define OLED_CS_Set()   DL_GPIO_setPins(OLED_PORT,OLED_CS_PIN)

// OLED????
// 0:4??????
// 1:??8080??

#define SIZE 16
#define XLevelL 0x02
#define XLevelH 0x10
#define Max_Column 128
#define Max_Row 64
#define Brightness 0xFF
#define X_WIDTH 128
#define Y_WIDTH 64


// OLED??????
void OLED_WR_Byte(u8 dat,u8 cmd);	  
void delay_ms(unsigned int ms);
void delay_us(unsigned int us);
void OLED_Display_On(void);
void OLED_Display_Off(void);	   							   		    
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawPoint(u8 x,u8 y,u8 t);
void OLED_Refresh(void);
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot);
void OLED_ShowChar(u8 x,u8 y,char chr);
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size2);
void OLED_ShowString(u8 x,u8 y, char *p);	 
void OLED_Set_Pos(unsigned char x, unsigned char y);
void OLED_ShowCHinese(u8 x,u8 y,u8 no);
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,const unsigned char BMP[]);

#endif
