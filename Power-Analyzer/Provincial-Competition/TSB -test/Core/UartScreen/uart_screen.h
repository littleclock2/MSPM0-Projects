/**
 * @file         uart_screen.h
 * @details      
 * @author       YunSword
 */
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

void HMISends(UART_Regs *uart,char *buf1);

void HMISendb(UART_Regs *uart,uint8_t k);

void HMISendstart(UART_Regs *uart);