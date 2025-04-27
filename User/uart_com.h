/*
 * uart_com.h
 *
 *  Created on: 2024Äê10ÔÂ16ÈÕ
 *      Author: Reed
 */

#ifndef USER_UART_COM_H_
#define USER_UART_COM_H_

#define UART_REPORT_GAP (2)
#define START_CONNECTION_GAP (2000)

void uart_com_task();
void uart_conf_write(uint32_t addr,uint8_t* ptr,uint8_t size);

#endif /* USER_UART_COM_H_ */
