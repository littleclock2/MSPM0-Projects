//#include "ADS8688.h"
//#include "ti_msp_dl_config.h"
////输出时，上升沿更新，低电平延迟
////输入时，高电平延迟，下降沿读取

///*******************************************************************************
//* 函数名: SoftSPI_Send
//* 输入参数:	data:发送的数据
//* 输出参数:	无
//*******************************************************************************/
//static void SoftSPI_Send(uint16_t data){//不改变片选的情况下发送数据
//	uint16_t mask = 0x8000;
//	for(uint8_t i = 0;i<16 ; i++){
//		if(mask&data)
//			PICO_P;
//		else
//			PICO_N;
//		WAIT;
//		SCLK_N;
//		WAIT;
//		SCLK_P;
//		mask >>= 1;
//	}
//}
///*******************************************************************************
//* 函数名: SoftSPI_Receive
//* 输入参数:	无
//* 输出参数:	接收到的数据
//*******************************************************************************/
//static uint16_t SoftSPI_Receive(void){//不改变片选的情况下接收数据
//	uint16_t mask = 0x8000,data = 0;
//	for(uint8_t i = 0;i<16 ; i++){
//		if(Get_POCI)
//			data|=mask;
//		SCLK_N;
//		WAIT;
//		SCLK_P;
//		WAIT;
//		mask >>= 1;
//	}
//	return data;
//}

///*******************************************************************************
//* 函数名: ADS8688_RST
//* 输入参数:	无
//* 输出参数:	无
//* 功能描述:	复位ADS8688
//*******************************************************************************/
//void ADS8688_RST(void){
//	RST_N;
//	WAIT;
//	RST_P;
//}

///*******************************************************************************
//* 函数名: ADS8688_cmd
//* 输入参数:	command:命令
//* 输出参数:	无
//* 功能描述:	通用函数，发送命令
//*******************************************************************************/
//void ADS8688_cmd(uint16_t command){
//	CS_N;
//	SoftSPI_Send(command);
//	CS_P;
//	return;
//}
///*
//* 函数名: ADS8688_CommandRegStandBy
//* 输入参数:	无
//* 输出参数:	无
//* 功能描述:	将ADS8688置于待机模式 下次发送指令恢复
//*/
//void ADS8688_CommandRegStandBy(void){
//	ADS8688_cmd(STDBY);
//	PICO_N;
//}
///*
//* 函数名: ADS8688_CommandRegPWDN
//* 输入参数:	无
//* 输出参数:	无
//* 功能描述:	将ADS8688置于断电模式 下次发送指令恢复
//*/
//void ADS8688_CommandRegPWDN(void){
//	ADS8688_cmd(PWR_DN);
//	PICO_N;
//}
///*
//* 函数名: ADS8688_CommandRegReset
//* 输入参数:	无
//* 输出参数:	无
//* 功能描述:	将Command寄存器复位 如果需要重新配置ADS8688，尽量调用这个函数
//*/
//void ADS8688_CommandRegReset(void){
//	ADS8688_cmd(RST);
//}
///*
//* 函数名: ADS8688_CommandRegAutoReset
//* 输入参数:	无
//* 输出参数:	无
//* 功能描述:	重置连续转化 就是将转化序列重新开始 一定要再调用这个函数之后再调用ADS8688_GetData，否则数据出错，恒为65535
//*/
//void ADS8688_CommandRegAutoReset(void){
//	ADS8688_cmd(AUTO_RST);
//}
///*******************************************************************************
//* 函数名: ADS8688_GetData
//* 输入参数:	command:命令
//* 					NO_OP:继续上一步操作 (连续读取必须使用)
//* 					MAN_Ch_0:读通道0
//* 					MAN_Ch_1:读通道1
//*					MAN_Ch_2:读通道2
//*					MAN_Ch_3:读通道3
//* 					MAN_Ch_4:读通道4
//* 					MAN_Ch_5:读通道5
//*					MAN_Ch_6:读通道6
//*					MAN_Ch_7:读通道7
//* 输出参数:	数据
//* 功能描述:	通用函数，获取数据
//* ******************************************************************************/
//int16_t ADS8688_GetData(uint16_t command){
//	CS_N;
//	SoftSPI_Send(command);
//	int16_t data = SoftSPI_Receive();
//	CS_P;
//	return data;
//}
///*
//* 函数名: ADS8688_ContinueGetData
//* 输入参数:	无
//* 输出参数:	数据
//* 功能描述:	在连续转化模式下继续读取数据
//*/
//int16_t ADS8688_ContinueGetData(void){
//	return ADS8688_GetData(NO_OP);
//}

///*******************************************************************************
//* 函数名: ADS8688_SetRange
//* 输入参数:
//* channel:通道号 1，2，3，4，5，6，7
//* range:量程选择：	 
//*					
//*					RANGE_2_5_REF:		 ±2.5*REF
//*					RANGE_1_2_5_REF:	±1.25*REF
//*					RANGE_0_625_REF:	±0.625*REF
//*					RANGE_POS_2_5_REF:	0~2.5*REF
//*					RANGE_POS_1_2_5_REF:0~1.25*REF
//*
//* 输出参数:
//* 					无
//*******************************************************************************/
//void ADS8688_SetRange(uint16_t channel,uint16_t range){
//	uint16_t address = ADS8688_Channel2RangeAddress(channel)|ADS8688_WRITE;
//	ADS8688_cmd(address|range);
//}

///*******************************************************************************
//* 函数名: ADS8688_Channel2RangeAddress
//* 输入参数:
//* channel:通道号 1，2，3，4，5，6，7
//*
//* 输出参数:通道量程寄存器地址
//********************************************************************************/
//static uint16_t ADS8688_Channel2RangeAddress(uint16_t channel){//辅助函数，将通道号转换为对应的寄存器地址
//	uint16_t address = 0;
//	switch(channel){
//		case 0:
//			address = REG_CH0_RANGE;
//			break;
//		case 1:
//			address = REG_CH1_RANGE;
//			break;
//		case 2:
//			address = REG_CH2_RANGE;
//			break;
//		case 3:
//			address = REG_CH3_RANGE;
//			break;
//		case 4:
//			address = REG_CH4_RANGE;
//			break;
//		case 5:
//			address = REG_CH5_RANGE;
//			break;
//		case 6:
//			address = REG_CH6_RANGE;
//			break;
//		case 7:
//			address = REG_CH7_RANGE;
//			break;
//		default:
//			address = REG_CH0_RANGE;
//			break;
//	}
//	return address;
//}

///*******************************************************************************
//* 函数名: ADS8688_SequentialReadConfig
//* 输入参数: channel:通道号 SEQ_CH7,SEQ_CH6,SEQ_CH5,SEQ_CH4,SEQ_CH3,SEQ_CH2,SEQ_CH1,SEQ_CH0
//* 函数功能: 配置连续读取的通道
//*******************************************************************************/
//void ADS8688_SequentialReadConfig(uint16_t channel){
//	ADS8688_cmd(REG_AUTO_SEQ_EN|ADS8688_WRITE|channel);
//}

///*******************************************************************************  
//* 函数名: ADS8688_StartSequentialRead
//* 输入参数: 无
//* 函数功能: 开始连续读取 不过会让通道序列从第二个通道开始，主要是防止有人瞎用ADS8688_CommandRegAutoReset函数，导致数据出错，而编写的轮椅函数
//*******************************************************************************/
//void ADS8688_StartSequentialRead(void){
//	ADS8688_cmd(AUTO_RST);
//	ADS8688_GetData(NO_OP);
//}

///*******************************************************************************
//* 函数名: ADS8688_ChannelPowerDown
//* 输入参数: channel:通道号 SEQ_CH7,SEQ_CH6,SEQ_CH5,SEQ_CH4,SEQ_CH3,SEQ_CH2,SEQ_CH1,SEQ_CH0
//* 函数功能: 通道断电
//*******************************************************************************/
//void ADS8688_ChannelPowerDown(uint16_t channel){
//	ADS8688_cmd(REG_CHANNEL_DIS|ADS8688_WRITE|channel);
//}



