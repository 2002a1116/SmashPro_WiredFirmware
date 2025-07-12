/*
 * ring_buffer.h
 *
 *  Created on: 2024Äê10ÔÂ15ÈÕ
 *      Author: Reed
 */

#ifndef USER_RING_BUFFER_H_
#define USER_RING_BUFFER_H_

#include <inttypes.h>

//#define RING_BUFFER_MAX_PKG_CNT (64)
#define RING_BUFFER_MAX_PKG_SIZE (64)
#pragma pack(push,4)
typedef struct __ring_buffer{
    uint8_t* buf;
    //uint8_t len[RING_BUFFER_MAX_PKG_CNT];
    //uint8_t typ[RING_BUFFER_MAX_PKG_CNT];
    uint8_t* len;
    uint8_t top;
    uint8_t end;
    uint8_t capcity;
    int32_t size;//we use u32 for risc-v atomic instruction,as its need operand to be 4byte aligned
    uint8_t full;
    uint8_t pkt_size;
}ring_buffer,*pring_buffer;
#pragma pack(pop)

uint8_t ring_buffer_init(pring_buffer rb,uint8_t* buf,uint8_t* len,uint8_t cap,uint8_t pkt_size);
uint8_t ring_buffer_push(pring_buffer rb,uint8_t* ptr,uint8_t len);
uint8_t ring_buffer_push_with_hdr(pring_buffer rb,uint8_t* ptr,uint8_t len,uint8_t typ);
uint8_t ring_buffer_pop(pring_buffer rb);

#endif /* USER_RING_BUFFER_H_ */
