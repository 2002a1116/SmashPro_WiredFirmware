/*
 * hd_rumble_high_accuracy.h
 *
 *  Created on: 2025Äê1ÔÂ7ÈÕ
 *      Author: Reed
 */

#ifndef USER_HD_RUMBLE_HIGH_ACCURACY_H_
#define USER_HD_RUMBLE_HIGH_ACCURACY_H_
#include "ring_buffer.h"
#include "hd_rumble.h"
#include "conf.h"
#include "board_type.h"

typedef struct _hd_rumble_high_accurary_pack{
    uint32_t step;
    int32_t amp;
    //uint32_t tick;
}hd_rumble_high_accurary_pack;
extern ring_buffer left_high_rb,left_low_rb,right_high_rb,right_low_rb;
extern uint8_t rumble_rb_overflow;
extern uint16_t hd_rumble_cvr_max;
extern uint16_t hd_rumble_cvr_min;
void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void hd_rumble_high_accurary_init();
void push_waveform_into_buffer_task();
void push_waveform(uint8_t channel,hd_rumble_high_accurary_pack* ptr);
#define HD_RUMBLE_HIGH_ACC_RINGBUFFER_CAP (32)
#define HD_RUMBLE_HIGH_ACC_PACK_SIZE (sizeof(hd_rumble_high_accurary_pack))
#define HD_RUMBLE_AMP_SHIFT_1 (10)
#define HD_RUMBLE_AMP_SHIFT_2 (17)
#define HD_RUMBLE_OLD_PCB_MAX_CVR_OFFSET (500)
#define HD_RUMBLE_NEW_PCB_MAX_CVR_OFFSET (700)
/*#if (PCB_TYPE==PCB_TYPE_1_0)
#define CHLCVR (TIM3->CH1CVR)
#define CHRCVR (TIM3->CH2CVR)
#else*/
#define CHLCVR (TIM3->CH3CVR)
#define CHRCVR (TIM3->CH4CVR)
//#endif
#endif /* USER_HD_RUMBLE_HIGH_ACCURACY_H_ */
