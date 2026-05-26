#ifndef __AD9910_H__
#define __AD9910_H__
#include "ti_msp_dl_config.h"
#include "oled_spi.h"

#define uchar unsigned char
#define uint  unsigned int	
#define ulong  unsigned long int
    
typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;

//#define AD9910_PWR		AD9910_PWR_PIN
//#define AD9910_SDIO		AD9910_SDIO_PIN
//#define DRHOLD			AD9910_DRH_PIN
//#define DROVER			AD9910_DRO_PIN
//#define UP_DAT			AD9910_IO_UP_PIN
//#define PROFILE1			AD9910_PF1_PIN
//#define MAS_REST			AD9910_REST_PIN
//#define SCLK				AD9910_SCLK_PIN
//#define DRCTL			AD9910_DRC_PIN
//#define OSK				AD9910_OSK_PIN
//#define PROFILE0			AD9910_PF0_PIN
//#define PROFILE2			AD9910_PF2_PIN
//#define CS				AD9910_CS_AD9910_PIN

#define CS(a)     ((a) ? DL_GPIO_setPins(AD9910_CS_AD9910_PORT, AD9910_CS_AD9910_PIN) : DL_GPIO_clearPins(AD9910_CS_AD9910_PORT, AD9910_CS_AD9910_PIN))
/* 电源控制 */
#define AD9910_PWR(a)       ((a) ? DL_GPIO_setPins(AD9910_PWR_PORT, AD9910_PWR_PIN) \
                        : DL_GPIO_clearPins(AD9910_PWR_PORT, AD9910_PWR_PIN))

/* 数据IO */
#define AD9910_SDIO(a)      ((a) ? DL_GPIO_setPins(AD9910_SDIO_PORT, AD9910_SDIO_PIN) \
                        : DL_GPIO_clearPins(AD9910_SDIO_PORT, AD9910_SDIO_PIN))

/* 保持控制 */
#define DRHOLD(a)    ((a) ? DL_GPIO_setPins(AD9910_DRH_PORT, AD9910_DRH_PIN) \
                        : DL_GPIO_clearPins(AD9910_DRH_PORT, AD9910_DRH_PIN))

/* 数据溢出 */
#define DROVER(a)    ((a) ? DL_GPIO_setPins(AD9910_DRO_PORT, AD9910_DRO_PIN) \
                        : DL_GPIO_clearPins(AD9910_DRO_PORT, AD9910_DRO_PIN))

/* 数据更新 */
#define UP_DAT(a)    ((a) ? DL_GPIO_setPins(AD9910_IO_UP_PORT, AD9910_IO_UP_PIN) \
                        : DL_GPIO_clearPins(AD9910_IO_UP_PORT, AD9910_IO_UP_PIN))

/* Profile引脚组 */
#define PROFILE0(a)  ((a) ? DL_GPIO_setPins(AD9910_PF0_PORT, AD9910_PF0_PIN) \
                        : DL_GPIO_clearPins(AD9910_PF0_PORT, AD9910_PF0_PIN))
#define PROFILE1(a)  ((a) ? DL_GPIO_setPins(AD9910_PF1_PORT, AD9910_PF1_PIN) \
                        : DL_GPIO_clearPins(AD9910_PF1_PORT, AD9910_PF1_PIN))
#define PROFILE2(a)  ((a) ? DL_GPIO_setPins(AD9910_PF2_PORT, AD9910_PF2_PIN) \
                        : DL_GPIO_clearPins(AD9910_PF2_PORT, AD9910_PF2_PIN))

/* 主复位 */
#define MAS_REST(a)  ((a) ? DL_GPIO_setPins(AD9910_REST_PORT, AD9910_REST_PIN) \
                        : DL_GPIO_clearPins(AD9910_REST_PORT, AD9910_REST_PIN))

/* 串行时钟 */
#define SCLK(a)      ((a) ? DL_GPIO_setPins(AD9910_SCLK_PORT, AD9910_SCLK_PIN) \
                        : DL_GPIO_clearPins(AD9910_SCLK_PORT, AD9910_SCLK_PIN))

/* 数据速率控制 */
#define DRCTL(a)     ((a) ? DL_GPIO_setPins(AD9910_DRC_PORT, AD9910_DRC_PIN) \
                        : DL_GPIO_clearPins(AD9910_DRC_PORT, AD9910_DRC_PIN))

/* 幅度整形控制 */
#define OSK(a)       ((a) ? DL_GPIO_setPins(AD9910_OSK_PORT, AD9910_OSK_PIN) \
                        : DL_GPIO_clearPins(AD9910_OSK_PORT, AD9910_OSK_PIN))
////#define AD9910_CSN_Set CS = 1
////#define AD9910_CSN_Clr CS = 0

////#define AD9910_IUP_Set UP_DAT = 1     
////#define AD9910_IUP_Clr UP_DAT = 0

typedef enum {
	TRIG_WAVE = 0,
	SQUARE_WAVE,
	SINC_WAVE,
} AD9910_WAVE_ENUM;

void AD9110_IOInit(void);
void Init_AD9910(void);
void AD9910_FreWrite(ulong Freq);										//D′?μ?ê
void AD9910_AmpWrite(uint16_t Amp);


void AD9910_RAM_WAVE_Set(AD9910_WAVE_ENUM wave);

void AD9910_DRG_AMP_Init(void);

void AD9910_DRG_FreInit_AutoSet(FunctionalState autoSweepEn);
void AD9910_DRG_FrePara_Set(u32 lowFre, u32 upFre, u32 posStep, u32 negStep, uint16_t posRate, uint16_t negRate);


#endif

