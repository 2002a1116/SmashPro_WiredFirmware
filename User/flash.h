/*
 * flash.h
 *
 *  Created on: 2024Äê11ÔÂ1ÈÕ
 *      Author: Reed
 */

#ifndef USER_FLASH_H_
#define USER_FLASH_H_
uint8_t flash_write(uint8_t id,uint8_t* data,uint8_t len);//length==128
uint8_t* get_raw_flash_buf();
uint8_t raw_flash_write(uint8_t id);

uint8_t flash_read(uint8_t id,uint8_t* data,uint8_t len);

#endif /* USER_FLASH_H_ */
