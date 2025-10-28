/*
 * uart_com.h
 *
 *  Created on: 2024Äê10ÔÂ16ÈÕ
 *      Author: Reed
 */

#ifndef USER_UART_COM_H_
#define USER_UART_COM_H_
#include "uart.h"
#define UART_REPORT_GAP (2)
#define START_CONNECTION_GAP (1000)
#define UART_RELIABLE_TIMEOUT (1000)
#define MAX_RELIABLE_UART_PKT_CNT (64)

typedef enum{
    BT_STATE_NOT_RDY=0,
    BT_STATE_RDY,
    BT_STATE_LISTENING,
    BT_STATE_CONNECTING,
    BT_STATE_CONNECTED,
    BT_STATE_DISCONNECTING,
    //bt_state_DISCONNECTED, --> BT STATUS RDY
    BT_STATE_ERROR
}BT_STATE;
typedef enum{
    BT_CMD_CONNECT=1,
    BT_CMD_DISCONNECT,
    BT_CMD_LISTEN,
    BT_CMD_RESET,
}BT_CMD;

enum RELIABLE_UART_STATUS{
    RURT_STS_OK,
    RURT_STS_TIMEOUT,
    RURT_STS_NO_CAP
};
typedef struct _reliable_uart_packet{
    uint32_t timestamp_ms;
    uint8_t r_id;
    void (*cb)(uint8_t,void*);
    void* context;
}reliable_uart_packet,*preliable_uart_packet;

void uart_com_task();
void uart_conf_write(uint32_t addr,uint8_t* ptr,uint8_t size);
void imu_buffer_reset_notifier();

void reliable_uart_send(uart_packet* pkt,void (*cb)(uint8_t,void*),void* context);
void reliable_uart_default_handler(uint8_t,void*);
void reliable_uart_task();
void reliable_uart_recv();
void set_bt_connect_mode(uint8_t);

#endif /* USER_UART_COM_H_ */
