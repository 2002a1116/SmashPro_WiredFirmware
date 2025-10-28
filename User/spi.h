/*
 * spi.h
 *
 *  Created on: 2025Äê1ÔÂ29ÈÕ
 *      Author: Reed
 */

#ifndef USER_SPI_H_
#define USER_SPI_H_

#define INDICATE_LED_BRIGHTNESS (50)

int spi_init(void);
void flush_rgb(uint8_t status);
void set_indicate_led_status(uint8_t status);
void indicate_rgb_task(uint8_t is_on_usb);
void set_indicate_led_mode(uint8_t mode);

#endif /* USER_SPI_H_ */
