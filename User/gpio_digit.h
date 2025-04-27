/*
 * gpio_digit.h
 *
 *  Created on: 2024Äê10ÔÂ10ÈÕ
 *      Author: Reed
 */

#ifndef USER_GPIO_DIGIT_H_
#define USER_GPIO_DIGIT_H_

#include "board_type.h"

#define GPIOA_PIN_MASK (0x10000)
#define GPIOB_PIN_MASK (0x20000)
#define GPIOC_PIN_MASK (0x40000)
#define GPIO_GROUP_MASK (0x70000)

#define NS_BUTTON_SL_L (5)
#define NS_BUTTON_SR_L (4)
#define NS_BUTTON_SL_R (20)
#define NS_BUTTON_SR_R (21)
#define NS_BUTTON_X (1)
#define NS_BUTTON_Y (0)
#define NS_BUTTON_A (3)
#define NS_BUTTON_B (2)
#define NS_BUTTON_UP (17)
#define NS_BUTTON_DOWN (16)
#define NS_BUTTON_LEFT (19)
#define NS_BUTTON_RIGHT (18)
#define NS_BUTTON_L (22)
#define NS_BUTTON_R (6)
#define NS_BUTTON_ZL (23)
#define NS_BUTTON_ZR (7)
#define NS_BUTTON_MINUS (8)
#define NS_BUTTON_PLUS (9)
#define NS_BUTTON_HOME (12)
#define NS_BUTTON_CAP (13)
#define NS_BUTTON_RS (10)
#define NS_BUTTON_LS (11)

#define GPIO_BUTTON_LS (GPIO_Pin_8|GPIOA_PIN_MASK)
#define GPIO_BUTTON_RS (GPIO_Pin_0|GPIOA_PIN_MASK)
#define GPIO_BUTTON_LEFT (GPIO_Pin_15|GPIOA_PIN_MASK)
#define GPIO_BUTTON_DOWN (GPIO_Pin_5|GPIOB_PIN_MASK)
#define GPIO_BUTTON_A (GPIO_Pin_13|GPIOB_PIN_MASK)
#define GPIO_BUTTON_B (GPIO_Pin_14|GPIOB_PIN_MASK)
#define GPIO_BUTTON_R (GPIO_Pin_13|GPIOC_PIN_MASK)
#define GPIO_BUTTON_ZL (GPIO_Pin_8|GPIOB_PIN_MASK)
#define GPIO_BUTTON_ZR (GPIO_Pin_14|GPIOC_PIN_MASK)
#define GPIO_BUTTON_L (GPIO_Pin_9|GPIOB_PIN_MASK)
#define GPIO_BUTTON_HOME (GPIO_Pin_15|GPIOB_PIN_MASK)
#define GPIO_BUTTON_MINUS (GPIO_Pin_6|GPIOB_PIN_MASK)
#define GPIO_BUTTON_PLUS (GPIO_Pin_15|GPIOC_PIN_MASK)
#define GPIO_BUTTON_CAP (GPIO_Pin_7|GPIOB_PIN_MASK)
#define GPIO_BUTTON_X (GPIO_Pin_6|GPIOA_PIN_MASK)
#define GPIO_BUTTON_UP (GPIO_Pin_3|GPIOB_PIN_MASK)
#define GPIO_BUTTON_RIGHT (GPIO_Pin_4|GPIOB_PIN_MASK)
#define GPIO_BUTTON_Y (GPIO_Pin_12|GPIOB_PIN_MASK)
#define GPIO_BUTTON_TOP (GPIO_Pin_2|GPIOB_PIN_MASK)

#define GPIO_PIN_SPI (GPIO_Pin_15|GPIOB_PIN_MASK)
#define GPIO_PIN_SDA (GPIO_Pin_11|GPIOB_PIN_MASK)
#define GPIO_PIN_SCL (GPIO_Pin_10|GPIOB_PIN_MASK)

#define GPIO_INPUT_CNT (24)
extern uint32_t hid_num_to_gpio[GPIO_INPUT_CNT];
extern uint32_t sts_button;

void gpio_init(void);
uint32_t gpio_read_all(void);
uint8_t gpio_read(uint32_t);

#endif /* USER_GPIO_DIGIT_H_ */
