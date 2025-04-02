/*
 * gpio_adc.h
 *
 *  Created on: 2024年10月11日
 *      Author: Reed
 */

#ifndef USER_GPIO_ADC_H_
#define USER_GPIO_ADC_H_

/*
 * gpio_adc.c
 *
 *  Created on: 2024年10月10日
 *      Author: Reed
 */

#include "debug.h"
#include "string.h"
#include "ch32v10x_rcc.h"

/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2020/04/30
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *ADC uses DMA sampling routine:
 *ADC channel 2 (PA2), the rule group channel obtains ADC conversion data
 *for 1024 consecutive times through DMA.
 *  Note: Take 3.3V as an example.
 */

#include "debug.h"

/* Global Variable */
extern uint16_t adc_data[4];
extern s16 Calibrattion_Val;
void ADC_Function_Init(void);
u16 Get_ADC_Val(u8 ch);
void DMA_Tx_Init(DMA_Channel_TypeDef *DMA_CHx, u32 ppadr, u32 memadr, u16 bufsize);
u16 Get_ConversionVal_3_3V(s16 val);
u16 Get_ConversionVal_5V(s16 val);

void adc_init(void);

#endif /* USER_GPIO_ADC_H_ */
