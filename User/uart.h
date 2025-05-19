/*
 * uart.h
 *
 *  Created on: 2025Äê4ÔÂ24ÈÕ
 *      Author: Reed
 */

#ifndef USER_UART_H_
#define USER_UART_H_

#include "ring_buffer.h"

#pragma pack(push,1)/*
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
*/
typedef struct __uart_packet{
    struct{
        uint8_t id:7;
        uint8_t starter:1;
    };
    union{
        struct{
            uint8_t typ:5;
            uint8_t hb2:2;
            uint8_t _empty:1;
            uint8_t hb1:7;
            uint8_t _empty2:1;
            uint8_t load[9];
        };
        uint8_t data[11];
    };
    struct{
        uint8_t cksum:7;
        uint8_t _empty3:1;
    };
}uart_packet,*puart_packet;
enum UART_PACKET_TYP{
    UART_PKG_INPUT_DATA=0x01,
    UART_PKG_INPUT_REQ=0x02,
    UART_PKG_CONNECT_CONTROL=0x03,
    UART_PKG_RUMBLE_FRAME=0x04,
    UART_PKG_CONTROL_DATA=0x05,
    UART_PKG_PWR_CONTROL=0x06,
    UART_PKG_TEST_ECHO=0X07,
    UART_PKG_CH32_FLASH_READ=0x08,
    UART_PKG_CH32_FLASH_WRITE=0x09,
    UART_PKG_ESP32_EROM_BUFFER_RESET=0X0A
};
#pragma pack(pop)

#define UART_PKG_LOAD_LENGTH (9)
#define UART_PKG_HEADER_MASK (0x80)
#define UART_BAUD_RATE (3000000)
//#define UART_BAUD_RATE (921600)
#define UART_PKG_SIZE (13)
#define UART_RINGBUFFER_PKG_CAP (64)
#define DEF_UART1_BUF_SIZE            384
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

#endif /* USER_UART_H_ */
