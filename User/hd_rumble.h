/*
 * hd_rumble.h
 *
 *  Created on: 2024Äê12ÔÂ3ÈÕ
 *      Author: Reed
 */

#ifndef USER_HD_RUMBLE_H_
#define USER_HD_RUMBLE_H_
#define HD_RUMBLE
#ifdef HD_RUMBLE
#include "ring_buffer.h"
#include "debug.h"
#define HD_RUMBLE_FRAME_TIME_MS (5)
#define HD_RUMBLE_FRAME_TIMEOUT_MS (32)
#define HD_RUMBLE_HIGH (0)
#define HD_RUMBLE_LOW (1)
#define HD_RUMBLE_PWM_L (0)
#define HD_RUMBLE_PWM_R (1)
#define HD_RUMBLE_SAMPLERATE (2048)
#define HD_RUMBLE_SAMPLERATE_SHIFT (11)
#define HD_RUMBLE_STEP (100000)
#define HD_RUMBLE_TIM_PRESCALER (1)
#define HD_RUMBLE_TIM_PERIOD (1440)
#define HD_RUMBLE_TIM_PERIOD_MID (720)
#define HD_RUMBLE_CLK (50000)//
#define HD_RUMBLE_AMP_FIXED_RATIO (1)
/*typedef struct _rumble_frame{
    uint32_t arr[2][2];
    //uint16_t ccr[2][2];
    int32_t amp[2][2];
    uint32_t step[2][2];
    //uint16_t pos[2][2];
}hd_rumble_frame;
#define HD_RUMBLE_FRAME_SIZE (sizeof(hd_rumble_frame))
#define HD_RUMBLE_RINGBUFFER_CAP (32)
#define HD_RUMBLE_SAMPLING_LEN (HD_RUMBLE_CLK*HD_RUMBLE_FRAME_TIME_MS/1000)
extern ring_buffer rumble_rb;
extern int32_t tmax,sa1,sa2,sp1,sp2,ss1,ss2,sc1,sc2,sr1,sr2;
void hd_rumble_init(uint8_t force_disable,uint8_t use_dma);
void hd_rumble_set_status(uint8_t status);
uint8_t push_rumble_frame(hd_rumble_frame* ptr);
void set_rumble_status(hd_rumble_frame* rumble_data);
*/
extern uint8_t rumble_state;
void next_rumble_frame();
void hd_rumble_init(uint8_t force_disable);
void TIM3_DMA_Init(DMA_Channel_TypeDef *DMA_CHx, uint32_t ppadr, uint16_t memadr, uint16_t bufsize);
#endif
#endif /* USER_HD_RUMBLE_H_ */
