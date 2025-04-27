/*
 * i2c.h
 *
 *  Created on: 2024Äê11ÔÂ22ÈÕ
 *      Author: Reed
 */

#ifndef USER_I2C_H_
#define USER_I2C_H_

#define IMU_I2C_TIMEOUT_US (200)
#define IMU_I2C_EXECUTE_FUNC (uint8_t (*)(void*))
#define IMU_I2C_MAX_RESET_ATTMEPT (9);

#define IMU_SIZE (6)
#define imu_raw_buf_SIZE (12)
#define CH32V_I2C_ADDR (0)
#define IMU_I2C_FREQ (800000)
#define IMU_ADDR (0b11010100)
#define IMU_ID (0x0F)

void i2c_hardware_init(u32 baudrate, u16 address, u8 allow_reset);
void i2c_init();
uint8_t i2c_read_byte(uint32_t reg,uint8_t *ret);
uint8_t i2c_write_byte(uint8_t reg,uint8_t v);
uint8_t i2c_read_continuous(uint8_t addr,uint8_t* buf,uint8_t len);

extern uint8_t imu_read_buf[imu_raw_buf_SIZE];
extern uint8_t imu_write_buf[imu_raw_buf_SIZE];
extern uint8_t i2c_status;
extern uint8_t i2c_error_code;
extern uint8_t i2c_intr_error;

#endif /* USER_I2C_H_ */
