/*
 * affine.h
 *
 *  Created on: 2026Äê5ÔÂ30ÈÕ
 *      Author: Reed
 */

#ifndef USER_AFFINE_H_
#define USER_AFFINE_H_
#include "joystick.h"
#pragma pack(push,1)
typedef struct{
    union{
        struct{
            float a;
            float b;
            float c;
            float d;
        };
        float mat[2][2];
    };
}affine_matrix;
#pragma pack(pop)
typedef struct{
    coord notch;
    coord angle;
}_affine_map;

extern _affine_map affine_map[2][16];
extern affine_matrix affine_trans[2][16];
extern coord affine_max[2];
extern coord affine_min[2];

void affine_init();
coord cal_affine(coord x,uint8_t id);
void joystick_affine_scale(int32_t *x,int32_t *y,uint8_t id);

extern uint8_t affine_rdy;

#endif /* USER_AFFINE_H_ */
