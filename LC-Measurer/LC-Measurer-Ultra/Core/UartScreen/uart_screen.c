#include "uart_screen.h"


void HMISends(char *buf1){		  //字符串发送函数
	uint8_t i=0;
	while(1){
	if(buf1[i]!=0){
//		DL_UART_transmitData(UART_0_INST,buf1[i]);  //发送一个字节
//		while(DL_UART_isBusy(UART_0_INST)==true){};//等待发送结束
            
        DL_UART_transmitData(UART_0_INST,buf1[i]);  //发送一个字节
		while(DL_UART_isBusy(UART_0_INST)==true){};//等待发送结束
		i++;
	}
	else 
		return ;
	}
}

void HMISendb(uint8_t k){		         //字节发送函数
	uint8_t i;
	for(i=0;i<3;i++){
	if(k!=0){
//		DL_UART_transmitData(UART_0_INST,k);  //发送一个字节
//		while(DL_UART_isBusy(UART_0_INST)==true){};//等待发送结束
//            
        DL_UART_transmitData(UART_0_INST,k);  //发送一个字节
		while(DL_UART_isBusy(UART_0_INST)==true){};//等待发送结束    
	}
	else 
	 	return ;
	} 
}

void HMISendstart(void){//屏幕初始化
	delay_ms(200);
	HMISendb(0xff);
	delay_ms(200);
}