/*
 * global_api.c
 *
 *  Created on: 2024Äê10ÔÂ20ÈÕ
 *      Author: Reed
 */

#include <stdint.h>
uint8_t u8_max(uint8_t a,uint8_t b){return a>b?a:b;}
uint8_t u8_min(uint8_t a,uint8_t b){return a<b?a:b;}
uint32_t fetch_uint32(uint8_t* src){
    uint32_t res=0;
    memcpy(&res,src,4);
    return res;
}
uint16_t fetch_uint16(uint8_t* src){
    uint16_t res;
    memcpy(&res,src,2);
    return res;
}
