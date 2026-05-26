#pragma once
#include "ti_msp_dl_config.h"

/*电平变化宏*/
#define WAIT __NOP();//__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();//可以设置延迟时间
#define CS_P        DL_GPIO_setPins(ADS8688_PORT,ADS8688_CS_PIN)
#define CS_N        DL_GPIO_clearPins(ADS8688_PORT,ADS8688_CS_PIN)
#define SCLK_P      DL_GPIO_setPins(ADS8688_PORT,ADS8688_SCLK_PIN)
#define SCLK_N      DL_GPIO_clearPins(ADS8688_PORT,ADS8688_SCLK_PIN)
#define PICO_P      DL_GPIO_setPins(ADS8688_PORT,ADS8688_PICO_PIN)
#define PICO_N      DL_GPIO_clearPins(ADS8688_PORT,ADS8688_PICO_PIN)
#define RST_P       DL_GPIO_setPins(ADS8688_PORT,ADS8688_RST_N_PIN)
#define RST_N       DL_GPIO_clearPins(ADS8688_PORT,ADS8688_RST_N_PIN)
#define Get_POCI    DL_GPIO_readPins(ADS8688_PORT,ADS8688_POCI_PIN)
    

/*command 寄存器*/
#define NO_OP       0x0000
#define STDBY       0x8200
#define PWR_DN      0x8300
#define RST         0x8500
#define AUTO_RST    0xA000
#define MAN_Ch_0    0xC000
#define MAN_Ch_1    0xC400
#define MAN_Ch_2    0xC800
#define MAN_Ch_3    0xCC00
#define MAN_Ch_4    0xD000
#define MAN_Ch_5    0xD400
#define MAN_Ch_6    0xD800
#define MAN_Ch_7    0xDC00
#define MAN_AUX     0xE000

/*program寄存器地址*/
#define REG_AUTO_SEQ_EN    0x01<<1<<8
#define REG_CHANNEL_DIS    0x02<<1<<8
#define REG_FEATURE SELECT 0x03<<1<<8

#define REG_CH0_RANGE 0x05<<1<<8
#define REG_CH1_RANGE 0x06<<1<<8
#define REG_CH2_RANGE 0x07<<1<<8
#define REG_CH3_RANGE 0x08<<1<<8
#define REG_CH4_RANGE 0x09<<1<<8
#define REG_CH5_RANGE 0x0A<<1<<8
#define REG_CH6_RANGE 0x0B<<1<<8
#define REG_CH7_RANGE 0x0C<<1<<8

#define REG_CH0_HYSTERESIS 0x15<<1<<8
#define REG_CH0_HIGH_THRESHOLD_MSB 0x16<<1<<8
#define REG_CH0_HIGH_THRESHOLD_LSB 0x17<<1<<8
#define REG_CH0_LOW_THRESHOLD_MSB 0x18<<1<<8
#define REG_CH0_LOW_THRESHOLD_LSB 0x19<<1<<8

#define REG_CH7_HYSTERESIS 0x38<<1<<8
#define REG_CH7_HIGH_THRESHOLD_MSB 0x39<<1<<8
#define REG_CH7_HIGH_THRESHOLD_LSB 0x3A<<1<<8
#define REG_CH7_LOW_THRESHOLD_MSB 0x3B<<1<<8
#define REG_CH7_LOW_THRESHOLD_LSB 0x3C<<1<<8

#define COMMAND_READ_BACK 0x3F<<1<<8

/*使能 控制寄存器地址需要加这个才能选择读和写*/
#define ADS8688_WRITE 0x0100
#define ADS8688_READ 0x0000


/*量程*/
#define RANGE_2_5_REF 0x00
#define RANGE_1_2_5_REF 0x01
#define RANGE_0_625_REF 0x02
#define RANGE_POS_2_5_REF 0x05
#define RANGE_POS_1_2_5_REF 0x06
/*通道号*/
#define SEQ_CH7 0x80
#define SEQ_CH6 0x40
#define SEQ_CH5 0x20
#define SEQ_CH4 0x10
#define SEQ_CH3 0x08
#define SEQ_CH2 0x04
#define SEQ_CH1 0x02
#define SEQ_CH0 0x01
#define SEQ_NONE 0x00
/**/
static void SoftSPI_Send(uint16_t data);
static uint16_t SoftSPI_Receive(void);
static uint16_t ADS8688_Channel2RangeAddress(uint16_t channel);
void ADS8688_SetChannel(uint16_t channel);
void ADS8688_cmd(uint16_t command);
void ADS8688_RST(void);
void ADS8688_SetRange(uint16_t channel,uint16_t range);
void ADS8688_SequentialReadConfig(uint16_t channel);
void ADS8688_CommandRegReset(void);
void ADS8688_CommandRegAutoReset(void);
int16_t ADS8688_ContinueGetData(void);
int16_t ADS8688_GetData(uint16_t command);
