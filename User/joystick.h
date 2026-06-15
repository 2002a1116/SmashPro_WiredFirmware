/*
 * joystick.c
 *
 *  Created on: 2026Äê6ÔÂ7ÈÕ
 *      Author: Reed
 */

#ifndef USER_JOYSTICK_C_
#define USER_JOYSTICK_C_

#include <stdint.h>

#define ADC_CHANNEL_CNT (4)
enum ADC_CHANNEL_ID{
    ADC_CHANNEL_LJOYS_HORI=0,
    ADC_CHANNEL_LJOYS_VERT,
    ADC_CHANNEL_RJOYS_HORI,
    ADC_CHANNEL_RJOYS_VERT
};
#define JOYSTICK_CENTER (2048)
#define JOYSTICK_PRANGE (2047)
#define JOYSTICK_NRANGE (-2047)

#pragma pack(push,1)
typedef struct{
    union{
        uint32_t data:24;
        struct{
            int16_t ix:12;
            int16_t iy:12;
        };
        struct{
            uint16_t x:12;
            uint16_t y:12;
        };
    };
}coord_compact;
typedef struct{
    int32_t x;
    int32_t y;
}coord;
#pragma pack(pop)

extern coord_compact sts_joy[2];
extern coord sts_joy_raw[2];

void joystick_upd();

#endif /* USER_JOYSTICK_C_ */
