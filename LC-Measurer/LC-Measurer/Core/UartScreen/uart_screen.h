/**
 * @file         uart_screen.h
 * @details      
 * @author       YunSword
 */
#ifndef _KEY_H_
#define _KEY_H_
/*
===========================
头文件包
===========================
*/
#include "ti_msp_dl_config.h"
#include "oled_spi.h"
/*
===========================
函数声明
===========================
*/

void HMISends(char *buf1);

void HMISendb(uint8_t k);

void HMISendstart(void);
#endif