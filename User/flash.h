/*
 * flash.h
 *
 *  Created on: 2024Äê11ÔÂ1ÈÕ
 *      Author: Reed
 */

#ifndef USER_FLASH_H_
#define USER_FLASH_H_
/*
uint8_t flash_write(uint8_t id,uint8_t* data,uint8_t len);//length==128
uint8_t* get_raw_flash_buf();
uint8_t raw_flash_write(uint8_t id);

uint8_t flash_read(uint8_t id,uint8_t* data,uint8_t len);*/

uint8_t read_flash(uint32_t addr,uint8_t* data,uint32_t size);
uint8_t write_flash(uint32_t addr,uint8_t* data,uint32_t size);
#endif /* USER_FLASH_H_ */
