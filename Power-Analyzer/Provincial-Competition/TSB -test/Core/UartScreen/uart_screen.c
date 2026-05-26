#include "uart_screen.h"


void HMISends(UART_Regs *uart,char *buf1){		  //字符串发送函数
	uint8_t i=0;
	while(1){
	if(buf1[i]!=0){
		DL_UART_transmitData(uart,buf1[i]);  //发送一个字节
		while(DL_UART_isBusy(uart)==true){};//等待发送结束
		i++;
	}
	else 
		return ;
	}
}

void HMISendb(UART_Regs *uart,uint8_t k){		         //字节发送函数
	uint8_t i;
	for(i=0;i<3;i++){
	if(k!=0){
		DL_UART_transmitData(uart,k);  //发送一个字节
		while(DL_UART_isBusy(uart)==true){};//等待发送结束
	}
	else 
	 	return ;
	} 
}

void HMISendstart(UART_Regs *uart){//屏幕初始化
	delay_ms(200);
	HMISendb(uart,0xff);
	delay_ms(200);
}