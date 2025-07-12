/********************************** (C) COPYRIGHT *******************************
 * File Name  :usbd_compatibility_hid.h
 * Author     :OWNER
 * Version    : v0.01
 * Date       : 2022Äê7ÔÂ8ÈÕ
 * Description:
*******************************************************************************/

#ifndef USER_USBD_COMPATIBILITY_HID_H_
#define USER_USBD_COMPATIBILITY_HID_H_

#include "ns_com.h"

#define SET_REPORT_DEAL_OVER          0x00
#define SET_REPORT_WAIT_DEAL          0x01

extern uint8_t  HID_Report_Buffer[64];               // HID Report Buffer
extern volatile uint8_t HID_Set_Report_Flag;

extern void HID_Set_Report_Deal( void );

extern imu_report_pack* imu_report_buffer_ptr;
extern peripheral_data global_input_data;

void ns_set_peripheral_data_getter(void (*getter)(peripheral_data*));
void update_peripheral_data();
void hid_tx_service();
void hid_rx_service();
void hid_init();
void set_imu_available(imu_report_pack* p);

#endif /* USER_USBD_COMPATIBILITY_HID_H_ */
