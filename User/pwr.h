/*
 * pwr.h
 *
 *  Created on: 2024Äê11ÔÂ12ÈÕ
 *      Author: Reed
 */

#ifndef USER_PWR_H_
#define USER_PWR_H_

#include <stdint.h>
extern uint8_t force_esp32_active;
void setup_exti(uint8_t state);
void set_pwr_mode_sleep(void);
uint8_t set_pwr_mode_stop(void);
void input_intr_handler(void);

#endif /* USER_PWR_H_ */
