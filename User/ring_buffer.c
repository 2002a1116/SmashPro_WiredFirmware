/*
 * ring_buffer.c
 *
 *  Created on: 2024Äê10ÔÂ15ÈÕ
 *      Author: Reed
 */

#include "ring_buffer.h"
#include "debug.h"
#include <string.h>

uint8_t ring_buffer_init(pring_buffer rb,uint8_t* buf,uint8_t cap,uint8_t pkt_size){
    if(cap>RING_BUFFER_MAX_PKG_CNT)return -1;
    if(!rb||!buf)return 1;
    memset(rb,0,sizeof(ring_buffer));
    rb->buf=buf;
    rb->capcity=cap;
    rb->top=1;
    rb->end=0;
    rb->full=0;
    rb->pkt_size=pkt_size;
    return 0;
}
uint8_t ring_buffer_push(pring_buffer rb,uint8_t* ptr,uint8_t len,uint8_t typ){
    if(!rb||!ptr||!rb->buf)return -1;
    if(rb->full||rb->capcity<=rb->size)return 1;
    if(len>rb->pkt_size)len=rb->pkt_size;
    //++rb->size;
    __AMOADD_W(&rb->size,1);
    ++rb->end;
    if(rb->end>=rb->capcity)rb->end=0;
    memcpy(&rb->buf[rb->pkt_size*rb->end],ptr,len);
    rb->len[rb->end]=len;
    rb->typ[rb->end]=typ;
    if(rb->size>=rb->capcity)
        rb->full=1;
    return 0;
}

uint8_t ring_buffer_pop(pring_buffer rb){
    if(!rb||!rb->buf)return -1;
    if(rb->size==0)return 0;
    //--rb->size;
    __AMOADD_W(&rb->size,-1);
    ++rb->top;
    if(rb->top>=rb->capcity)rb->top=0;
    if(rb->full)
        rb->full=(uint8_t)(rb->capcity<=rb->size);
}
