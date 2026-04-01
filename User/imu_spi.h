/*
 * imu_spi.h
 *
 *  Created on: 2026Äê3ÔÂ11ÈÕ
 *      Author: Reed
 */

#ifndef USER_IMU_SPI_H_
#define USER_IMU_SPI_H_

#define SPI2_TX_TIMEOUT_US (200)
#define SPI2_RX_TIMEOUT_US (500)

void imu_spi_init();
uint8_t spi2_check_flag(uint16_t flag);
uint8_t spi_write(uint8_t value);
uint8_t spi_read(uint8_t* value);
void spi_clear();
uint8_t spi_get_reg(uint8_t addr,uint8_t* value);
uint8_t spi_get_multi_reg(uint8_t addr,uint8_t* value,uint8_t size);
uint8_t spi_set_reg(uint8_t addr,uint8_t value);
uint8_t spi_set_multi_reg(uint8_t addr,uint8_t* value,uint8_t size);

#endif /* USER_IMU_SPI_H_ */
