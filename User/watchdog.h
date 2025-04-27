/*
 * watchdog.h
 *
 *  Created on: 2025��4��14��
 *      Author: Reed
 */

#ifndef USER_WATCHDOG_H_
#define USER_WATCHDOG_H_

extern uint8_t watchdog_pos;
void sofw_watchdog_init();
void soft_watchdog_feed();
void TIM4_IRQHandler(void);

#endif /* USER_WATCHDOG_H_ */
