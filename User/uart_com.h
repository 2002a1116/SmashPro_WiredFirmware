/*
 * uart_com.h
 *
 *  Created on: 2024Äê10ÔÂ16ÈÕ
 *      Author: Reed
 */

#ifndef USER_UART_COM_H_
#define USER_UART_COM_H_
#include "ring_buffer.h"

#pragma pack(push,1)
typedef struct __uart_packet{
            union{
                struct{
                    uint8_t id:4;
                    uint8_t typ:4;
                };
                uint8_t header;
            };
            union{
                struct{
                    uint8_t high_bit;
                    union{
                        uint8_t arr[3];
                        uint32_t load:24;
                    };
                };
                uint8_t data[4];
                uint32_t raw;
            };
            uint8_t cksum;
}uart_packet,*puart_packet;
enum UART_PACKET_TYP{
    UART_PKG_INPUT_DATA=0x01,
    UART_PKG_INPUT_REQ=0x02,
    UART_PKG_CONNECT_CONTROL=0x03,
    UART_PKG_RUMBLE_FRAME=0x04,
    UART_PKG_CONTROL_DATA=0x05,
    UART_PKG_PWR_CONTROL=0x06,
    UART_PKG_TEST_ECHO=0X07
};
#pragma pack(pop)

#define UART_PKG_ARR_LENGTH (3)
#define UART_PKG_HEADER_MASK (0x80)
#define UART_PKG_LOAD_MASK (~(UART_PKG_HEADER_MASK|(UART_PKG_HEADER_MASK<<8)|(UART_PKG_HEADER_MASK<<16)))
#define UART_BAUD_RATE (3000000)
//#define UART_BAUD_RATE (921600)
#define UART_PKG_SIZE (6)
#define UART_RINGBUFFER_PKG_CAP (32)
#define DEF_UART1_BUF_SIZE            2048
#define DEF_UART1_TOUT_TIME           30             // NOTE: the timeout time should be set according to the actual baud rate.
extern void UART1_Tx_Service( void );
extern void UART1_Rx_Service( void );
extern void UART1_Init( void );
extern void UART1_DMA_Init( void );
//extern void TIM2_Init( void );
extern void UART1_DMA_Tx(uint8_t *pbuf,uint16_t len);
extern uint8_t send_uart_pkt(uart_packet* pkt);
extern void encode_uart_pkt(uart_packet* pkt);
extern void decode_uart_pkt(uart_packet* pkt);
extern uint8_t check_uart_pkt(uart_packet* pkt);//if ok return false aka 0
extern uint8_t send_uart_large(uint8_t* buf,uint8_t len,uint8_t typ);

extern uint8_t uart_rx_rb_buf[UART_PKG_SIZE*UART_RINGBUFFER_PKG_CAP];
extern uint8_t uart_tx_rb_buf[UART_PKG_SIZE*UART_RINGBUFFER_PKG_CAP];
extern ring_buffer uart_rx_rb;
extern ring_buffer uart_tx_rb;
extern volatile uint8_t UART1_Tx_Flag;

#endif /* USER_UART_COM_H_ */
