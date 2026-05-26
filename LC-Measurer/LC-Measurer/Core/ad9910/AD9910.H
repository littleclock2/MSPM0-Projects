#ifndef __AD9910_H__
#define __AD9910_H__

#include "ti_msp_dl_config.h"

#define uchar unsigned char
#define uint  unsigned int	
#define ulong  unsigned long int

#define AD9910_PWR		DAC_PWR_PIN
#define AD9910_SDIO		DAC_PICO_PIN
#define DRHOLD			DAC_DRH_PIN
#define DROVER			DAC_DRO_PIN
#define UP_DAT			DAC_IOUP_PIN
#define PROFILE1			DAC_PF1_PIN  
#define MAS_REST			DAC_RST_PIN
#define SCLK			DAC_SCLK_PIN 
#define DRCTL			DAC_DRC_PIN  
#define OSK				DAC_OSK_PIN 
#define PROFILE0			DAC_PF0_PIN
#define PROFILE2			DAC_PF2_PIN
#define CS				DAC_CS0_PIN

void AD9110_IOInit(void);
void Init_AD9910(void);
void AD9910_FreWrite(ulong Freq);										//Ð´ÆµÂÊ
void AD9910_AmpWrite(uint16_t Amp);

#endif


