#include "ti_msp_dl_config.h"
#include "oled_spi.h"
u8 mode = 0,start = 0;
uint32_t f_cnt=0;
uint16_t voltage=0;
int main(void){   
	  SYSCFG_DL_init();
	  OLED_Init();
	  
	  NVIC_ClearPendingIRQ(UART_1_INST_INT_IRQN);//顺序不能改，不然queue有问题
	  NVIC_EnableIRQ(UART_1_INST_INT_IRQN);
    while(1)
		{
			OLED_ShowNum(0,2,voltage,10,16);
    }
		
}




void UART_1_INST_IRQHandler(void){
	static bool cnt=0;
	static int16_t temp_Voltage=0;
	u8 data=DL_UART_receiveData(UART_1_INST);
		if(!cnt){
			if((data & 0b10000000) != 0){
					cnt = 1;
					temp_Voltage += (data & 0b01111111) << 7;
			}
		}
		else
			if((data & 0b10000000) == 0){//对准
					cnt = 0;
					voltage = temp_Voltage + (data & 0b01111111);
					temp_Voltage = 0;
				
			}
			else{
					cnt = 0;
					temp_Voltage = 0;
			}
	return;
} 