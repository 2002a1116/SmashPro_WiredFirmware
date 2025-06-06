/********************************** (C) COPYRIGHT  *******************************
 * File Name          : debug.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2020/04/30
 * Description        : This file contains all the functions prototypes for UART
 *                      //printf , Delay functions.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#ifndef __DEBUG_H
#define __DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "ch32v10x.h"

/*  HID function ,1 close HID function ,0 open HID function */
#define ch32v10x_usb_hid       0

/* UART //printf Definition */
#define DEBUG_UART1    1
#define DEBUG_UART2    2
#define DEBUG_UART3    3

/* DEBUG UATR Definition */
#ifndef DEBUG
//#define DEBUG   DEBUG_UART2
#define DEBUG   DEBUG_NONE
#endif

/* SDI //printf Definition */
#define SDI_PR_CLOSE   0
#define SDI_PR_OPEN    1

#ifndef SDI_PRINT
#define SDI_PRINT   SDI_PR_CLOSE
#endif

void USART_printf_Init(uint32_t baudrate);
void SDI_printf_Enable(void);

#ifdef __cplusplus
}
#endif

#endif /* __DEBUG_H */
