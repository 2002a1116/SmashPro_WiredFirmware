/*
 * spi.h
 *
 *  Created on: 2025Äê1ÔÂ29ÈÕ
 *      Author: Reed
 */

#ifndef USER_SPI_H_
#define USER_SPI_H_

#define INDICATE_LED_BRIGHTNESS (25)

int rgb_init(void);
void flush_rgb();
void flush_rgb_slow();
void _force_rgb(uint8_t status);
void set_indicate_led_status(uint8_t status);
void rgb_task(uint8_t is_on_usb);
void set_indicate_led_mode(uint8_t mode);
void led_pwr_ctrl(uint8_t state);

extern float rgb_slow_start_div;
extern uint8_t bat_warn;

#endif /* USER_SPI_H_ */
