/*
 * hd_rumble_high_accurary_packed.c
 *
 *  Created on: 2025Äê7ÔÂ11ÈÕ
 *      Author: Reed
 */
#include "hd_rumble_high_accurary_packed.h"

uint16_t hd_rumble_sample_cvr_buffer[3][2][HD_RUMBLE_SAMPLE_LENGTH];
uint8_t hd_rumble_sample_nxt;//[buffer1,buffer2,new wave buffer],works as ringbuffer
uint8_t hd_rumble_sample_new_rdy;
//when we start buffer1,we process buffer2.
//when a new wave came,we process it after buffer2 wave ending phase,and save it into new wave zone as new buffer 1.
//when new wave rdy,we switch,and than cal rdy2.
//this feature must use with dma and context switch feature.

//current rev 3.2x & 3.50 dont suppert dma as tim3_channel_4 conflict with spi1 tx.
//its hard to get a good pin layout
uint16_t hd_rumble_cvr_buffer_index[2];
uint32_t hd_rumble_starter_phase[2][2];
uint32_t hd_rumble_start_timestamp_us;
/*reserved*/
