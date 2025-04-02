/*
 * rumble.h
 *
 *  Created on: 2024Äê11ÔÂ25ÈÕ
 *      Author: Reed
 */

#ifndef USER_RUMBLE_H_
#define USER_RUMBLE_H_
#include <stdint.h>
#include "hd_rumble.h"
#ifndef HD_RUMBLE
#define RUMBLE_MODE (RUMBLE_MODE_SIN)

#define PWM_MODE (PWM_MODE1)
#define RUMBLE_FRAME_SIZE (sizeof(rumble_frame))
#define RUMBLE_RINGBUFFER_CAP (16)
#define RUMBLE_FRAME_TIME_MS (8)
#define RUMBLE_FRAME_TIMEOUT_MS (128)
#define RUMBLE_STEP (512)
#define FREQ_HIGH_MASK (0x800)
typedef struct _rumble_frame{
    //uint16_t high_freq;
    //uint16_t low_Freq;
    //double high_amp;
    //double low_amp;
    uint32_t high_arr;
    uint32_t high_ccr;
    uint32_t low_arr;
    uint32_t low_ccr;
    double high_amp;
    double low_amp;
    double high_sin;
    double low_sin;
}rumble_frame;

void decode_rumble(uint8_t* ptr,rumble_frame* rumble_data);
void set_rumble_status(rumble_frame* rumble_data);
uint8_t push_rumble_frame(rumble_frame* ptr);
void next_rumble_frame();
void rumble_init(uint8_t);

extern ring_buffer rumble_rb;
extern uint8_t rumble_state;
extern uint8_t rumble_enabled;
#endif
#endif /* USER_RUMBLE_H_ */
