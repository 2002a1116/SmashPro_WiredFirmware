/*
 * global_api.h
 *
 *  Created on: 2024Äê10ÔÂ20ÈÕ
 *      Author: Reed
 */

#ifndef USER_GLOBAL_API_H_
#define USER_GLOBAL_API_H_

uint8_t u8_max(uint8_t a,uint8_t b);
uint8_t u8_min(uint8_t a,uint8_t b);
int32_t i32_max(int32_t a,int32_t b);
int32_t i32_min(int32_t a,int32_t b);
uint32_t u32_max(uint32_t a,uint32_t b);
uint32_t u32_min(uint32_t a,uint32_t b);

uint32_t fetch_uint32(uint8_t* src);
uint16_t fetch_uint16(uint8_t* src);
int32_t i32_clamp(int32_t v,int32_t min,int32_t max);
void trigger_hardfault();
//workarounds for alignment issue.
//dont know why this happen,it shouldnt and works perfectly well on pc or esp32,probabliy be well on stm32,too
//use these api instead of type casting to fix it

#endif /* USER_GLOBAL_API_H_ */
