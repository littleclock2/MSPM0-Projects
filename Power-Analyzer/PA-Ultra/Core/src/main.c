#include "ti_msp_dl_config.h"
#include "oled_spi.h"
#include "arm_const_structs.h"
#include "arm_math.h"
#include "board.h"
#include "hc05.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "W25Q128.h"

//#include "ADS8688.h"//引脚未定义，故暂时注释屏蔽，c文件中同时进行屏蔽处理，定义完毕引脚后可解除屏蔽
#define BUFFER_SIZE 1024//定义缓冲区大小
//自动储存内存地址 
#define  ADDRESS	(0x00008000)
#define  CLK        (3200-0000)//时钟速度
//定义全局变量
uint8_t choice  =0;//通信接受变量，非必要误删，可根据需求更改
u8 mode = 1;//模式选择变量
u8 timer_1s_flag = 0;//1s定时标志位
u8 timer_3s_flag = 0;//3s定时标志位
u8 timer_3s_startcounter_flag = 0;//3s定时开始计数标志位



//函数定义
void Screen_Display(void);
void Bluetooth_Ctrl(void);
void Uart_Ctrl(void);
void Analysis(void);
void Learn(void);
void W25Q128_FlashCrtl(void);//暂时用不了
void Flash_Write(uint32_t address, uint32_t *data,uint32_t dataSize);//片内写
void Flash_Erase();//片内全部擦除
void Flash_Read(uint32_t address,uint32_t dataSize);//片内读
int main(void){

    /* 系统初始化 */
    SYSCFG_DL_init();
    
    //通信初始化
	NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN );//清除中断标志位
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN );
    
    //定时器初始化
    DL_TimerG_startCounter(TIMER_0_INST);
    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
    
    //中断初始化
    NVIC_EnableIRQ(KEY_INT_IRQN);//开启按键引脚的GPIOB端口中断

    //蓝牙初始化
    Bluetooth_Init();
    
    delay_ms(100);
    
    // OLED初始化
    OLED_Init();
    OLED_Clear();
    
    delay_ms(100);

    //显示开机画面
    OLED_ShowString(0,0,"TMX MSPM0G3507");
    OLED_ShowString(24,2,"EAAI Device");
    OLED_ShowString(0,4,"2025.3.20");
    OLED_ShowString(0,6,"Version:0.2");
    
    OLED_Refresh();//更新显示,这个很重要！！！每次操作完屏幕别忘了这个！！！

    delay_ms(2000);
    //清屏
    OLED_Clear();
    OLED_Refresh();
    
    //不知道为什么，我调了半小时没调好spi，先用片上Flash吧
    //FlashCrtl();
    
    while (1){  
        Screen_Display();//屏幕显示控制
        switch (mode){
        case 1://模式1对用电器进行分析
            
            break;
        case 2://模式2对用电器进行学习,学习结束后将信息存在Flash中并返回模式1
            
            break;
        default:
            break;
        }
        
    }
}

//通信接受中断函数
void  UART_0_INST_IRQHandler(){
   switch (DL_UART_getPendingInterrupt(UART_0_INST )){ //检测是否串口中断
        case DL_UART_MAIN_IIDX_RX:
            choice = DL_UART_receiveData(UART_0_INST);
            DL_UART_transmitData(UART_0_INST,1);
            break;
        default:
            break;
    }
}

//定时器中断函数
void TIMER_0_INST_IRQHandler(void){//该定时器的一个周期100ms
	switch(DL_TimerG_getPendingInterrupt(TIMER_0_INST)){
    case DL_TIMERG_IIDX_ZERO:
        //每秒发送数据部分
        timer_1s_flag ++;//1s定时标志位
        if(timer_1s_flag > 10){
            timer_1s_flag = 0;
            Bluetooth_Ctrl();	
            Uart_Ctrl();
        }
        break;
    default:
        break;
  }
}

void GROUP1_IRQHandler(void){//Group1的中断服务函数
    //读取Group1的中断寄存器并清除中断标志位
    switch( DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1) ){
        //检查是否是KEY的GPIOB端口中断，注意是INT_IIDX，不是PIN_22_IIDX
        case KEY_INT_IIDX:
        //如果按键按下变为高电平
            if( DL_GPIO_readPins(KEY_PORT, KEY_MODE_PIN) > 0 ){
                delay_ms(10);
                if( DL_GPIO_readPins(KEY_PORT, KEY_MODE_PIN) > 0 ){
                    mode = 2;//学习模式
                }
            }
        break;
    }
}
//屏幕显示函数
void Screen_Display(void){
    OLED_Clear();//清屏
    //中间进行屏幕控制
    switch (mode){
        case 1://模式1对用电器进行分析
            OLED_ShowString(0,0,"MODE:Analysis");
            OLED_Refresh();
            break;
        case 2://模式2对用电器进行学习,学习结束后将信息存在Flash中并返回模式1
            OLED_ShowString(0,0,"MODE:Learn");
            OLED_Refresh();
            break;
        default:
            break;
    }
    delay_ms(100);
    OLED_Refresh();//更新显示
}

//蓝牙控制函数(放到定时器中断中定时发送)
void Bluetooth_Ctrl(void){
    
    
}

//串口控制函数(将蓝牙控制函数中蓝牙发送改为串口发送，便于进行调试)
void Uart_Ctrl(void){
    
    
}

void Flash_Write(uint32_t address, uint32_t *data,uint32_t dataSize){//写函数
    DL_FlashCTL_unprotectSector( FLASHCTL, ADDRESS, DL_FLASHCTL_REGION_SELECT_MAIN);
	DL_FlashCTL_programMemoryFromRAM( FLASHCTL, address, data,dataSize, DL_FLASHCTL_REGION_SELECT_MAIN);
    DL_FlashCTL_waitForCmdDone(FLASHCTL);
}
void Flash_Erase(){//擦除函数
    DL_FlashCTL_unprotectSector( FLASHCTL, ADDRESS, DL_FLASHCTL_REGION_SELECT_MAIN);
	DL_FlashCTL_eraseMemoryFromRAM( FLASHCTL, ADDRESS, DL_FLASHCTL_COMMAND_SIZE_SECTOR);
	DL_FlashCTL_waitForCmdDone( FLASHCTL);
}
void Flash_Read(uint32_t address,uint32_t dataSize){
    uint32_t temp[dataSize];
    for(int i=0;i<4*dataSize;i+=4){
			temp[i/4]=*(int *)(address+i);
	}
}

//数据分析函数(参数没填，按需要填写)
void Analysis(void){
    
    
}

//学习函数(参数没填，按需要填写)
void Learn(void){
    
    
}

//Flash控制函数
void W25Q128_FlashCrtl(void){
    
	/*
	与ADS1118对应的SPI设置
	总之就是一大堆宏定义
	做出的变化：
	1、片选从OLED切换到了ADS1118的片选
	2、8bit一帧变成了16bit一帧
	3、相位极性变成第二个跳变沿读取
	*/
	static DL_SPI_Config gSPI_0_config = {
    .mode        = DL_SPI_MODE_CONTROLLER,
    .frameFormat = DL_SPI_FRAME_FORMAT_MOTO4_POL0_PHA0,
    .parity      = DL_SPI_PARITY_NONE,
    .dataSize    = DL_SPI_DATA_SIZE_8,
    .bitOrder    = DL_SPI_BIT_ORDER_MSB_FIRST,
    .chipSelectPin = DL_SPI_CHIP_SELECT_0,
	};
	/*
	时钟配置
	*/
	static const DL_SPI_ClockConfig gSPI_0_clockConfig = {
    .clockSel    = DL_SPI_CLOCK_BUSCLK,
    .divideRatio = DL_SPI_CLOCK_DIVIDE_RATIO_1
	};
	/*
	以下操作用来重置spi寄存器，然后就可以切换到和ADS1118相同配置的spi设置
	*/
	DL_SPI_reset(SPI_0_INST); //reset之后才能修改配置
	DL_SPI_enablePower(SPI_0_INST);//先上电 再修改
	DL_SPI_setClockConfig(SPI_0_INST, (DL_SPI_ClockConfig *) &gSPI_0_clockConfig); //修改spi时钟配置
	DL_SPI_init(SPI_0_INST, (DL_SPI_Config *) &gSPI_0_config);//修改spi基础设置
	DL_SPI_setBitRateSerialClockDivider(SPI_0_INST, 1);//设置分频
	DL_SPI_setFIFOThreshold(SPI_0_INST, DL_SPI_RX_FIFO_LEVEL_1_2_FULL, DL_SPI_TX_FIFO_LEVEL_1_2_EMPTY);//设置FIFO
	DL_SPI_enable(SPI_0_INST);
	
	/*写数据并接受*/
	unsigned char buff[10] = {0};
    //读取W25Q128的ID
	printf("ID = %X\r\n",W25Q128_readID());
    //读取0地址的5个字节数据到buff
	W25Q128_read(buff, 0, 5);
	//串口输出读取的数据
	printf("buff = %s\r\n",buff);

	//往0地址写入5个字节长度的数据 ABCD
	W25Q128_write("ABCD", 0, 5);

	delay_ms(1);//等待稳定

	//读取0地址的5个字节数据到buff
	W25Q128_read(buff, 0, 5);

	//串口输出读取的数据
	printf("buff = %s\r\n",buff);
	
	
	/*将spi恢复到OLED的显示设置上*/
	DL_SPI_reset(SPI_0_INST);
	DL_SPI_enablePower(SPI_0_INST);
	SYSCFG_DL_SPI_0_init();//这个函数是Sysconfig配置好之后生成，调用这个函数，会让SPI恢复到Sysconfig里面配置的状态
	
	
}

