/*
 * ns_com_mux.h
 *
 *  Created on: 2024Äê10ÔÂ15ÈÕ
 *      Author: Reed
 */

#ifndef USER_NS_COM_MUX_H_
#define USER_NS_COM_MUX_H_

#include <inttypes.h>
#include "ns_com.h"
#include "ring_buffer.h"

#define NS_STD_REPORT_BASIC_LENGTH (13)

#define BT_LTK_LENGTH (16)


struct __connection_state{
    uint8_t esp32_connected;
    uint8_t esp32_sleep;
    uint8_t esp32_paired;
    uint8_t esp32_bt_state;
    uint8_t usb_enumed;
    uint8_t usb_paired;
    uint8_t bd_addr[BD_ADDR_LEN];
    uint8_t con_addr[BD_ADDR_LEN];
    uint8_t bd_addr_set;
    uint8_t con_addr_set;
    uint8_t bt_ltk[BT_LTK_LENGTH];
    uint8_t bt_ltk_set;
};
extern struct __connection_state connection_state;
extern ring_buffer ns_usb_send_rb;
extern void (*ns_hid_packet_dispatch_tb[NS_PACKET_TYPE_MAX_VALUE])(cmd_packet*);
extern uint8_t is_rumble_start;
extern uint32_t rts_cnt,rts_tcnt;
void ns_hid_register_packet_dispatch(int typ,void (*handler)(cmd_packet*));
void ns_mux_init();
uint8_t ns_send_report(report_packet* rpt);
uint8_t ns_send_full64byte_report(report_packet* rpt);
uint8_t hid_send_full64byte_report(uint8_t* buf,uint8_t len);
void hid_dispatch(cmd_packet* pkt);

#endif /* USER_NS_COM_MUX_H_ */
