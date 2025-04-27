/*
 * tick.h
 *
 *  Created on: 2025Äê2ÔÂ27ÈÕ
 *      Author: Reed
 */

#ifndef USER_TICK_H_
#define USER_TICK_H_



uint32_t Get_Systick_US();
uint64_t Get_Systick_US64();
uint32_t Get_Systick_MS();
void SysTick_Init(void);
uint8_t wait_nonblocking_us(uint8_t (*func)(void*),void* param,uint32_t us);
uint8_t wait_nonblocking_ms(uint8_t (*func)(void*),void* param,uint32_t ms);
void Delay_Us(uint32_t n);
void Delay_Ms(uint32_t n);
//void Delay_Clk(uint32_t n);

void HighPrecisionTimer_Init(void);
void HighPrecisionTimerCmd(uint8_t status);
void HighPrecisionTimerStart();
uint64_t HighPrecisionTimerCnt();
uint64_t HighPrecisionTimerUs();
void HighPrecisionTimerDelayUs(uint32_t us);

#endif /* USER_TICK_H_ */
