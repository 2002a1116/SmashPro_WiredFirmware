/********************************** (C) COPYRIGHT *******************************
 * File Name          : system_ch32v10x.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2020/04/30
 * Description        : CH32V10x Device Peripheral Access Layer System Header File.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#ifndef __SYSTEM_CH32V10x_H
#define __SYSTEM_CH32V10x_H

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t SystemCoreClock; /* System Clock Frequency (Core Clock) */

/* System_Exported_Functions */
extern void SystemInit(void);
extern void SystemCoreClockUpdate(void);
extern void SetSysClock(void);

#ifdef __cplusplus
}
#endif

#endif /*__CH32V10x_SYSTEM_H */
