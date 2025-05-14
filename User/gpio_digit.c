/*
 * gpio_digit.c
 *
 *  Created on: 2024Äê10ÔÂ10ÈÕ
 *      Author: Reed
 */

#include "debug.h"
#include "string.h"
#include "ch32v10x_rcc.h"
#include "usbd_compatibility_hid.h"
#include "gpio_digit.h"
#include "conf.h"
uint32_t sts_button;
uint32_t hid_num_to_gpio[GPIO_INPUT_CNT]={};
void gpio_tb_init(){
    hid_num_to_gpio[NS_BUTTON_LS]=GPIO_BUTTON_LS;
    hid_num_to_gpio[NS_BUTTON_RS]=GPIO_BUTTON_RS;
    hid_num_to_gpio[NS_BUTTON_X]=GPIO_BUTTON_X;
    hid_num_to_gpio[NS_BUTTON_Y]=GPIO_BUTTON_Y;
    hid_num_to_gpio[NS_BUTTON_A]=GPIO_BUTTON_A;
    hid_num_to_gpio[NS_BUTTON_B]=GPIO_BUTTON_B;
    hid_num_to_gpio[NS_BUTTON_UP]=GPIO_BUTTON_UP;
    hid_num_to_gpio[NS_BUTTON_DOWN]=GPIO_BUTTON_DOWN;
    hid_num_to_gpio[NS_BUTTON_LEFT]=GPIO_BUTTON_LEFT;
    hid_num_to_gpio[NS_BUTTON_RIGHT]=GPIO_BUTTON_RIGHT;
    hid_num_to_gpio[NS_BUTTON_L]=GPIO_BUTTON_L;
    hid_num_to_gpio[NS_BUTTON_R]=GPIO_BUTTON_R;
    hid_num_to_gpio[NS_BUTTON_ZL]=GPIO_BUTTON_ZL;
    hid_num_to_gpio[NS_BUTTON_ZR]=GPIO_BUTTON_ZR;
    hid_num_to_gpio[NS_BUTTON_MINUS]=GPIO_BUTTON_MINUS;
    hid_num_to_gpio[NS_BUTTON_PLUS]=GPIO_BUTTON_PLUS;
    hid_num_to_gpio[NS_BUTTON_HOME]=GPIO_BUTTON_HOME;
    hid_num_to_gpio[NS_BUTTON_CAP]=GPIO_BUTTON_CAP;
    if(user_config.a_b_swap){
        hid_num_to_gpio[NS_BUTTON_A]=GPIO_BUTTON_B;
        hid_num_to_gpio[NS_BUTTON_B]=GPIO_BUTTON_A;
    }
    if(user_config.x_y_swap){
        hid_num_to_gpio[NS_BUTTON_X]=GPIO_BUTTON_Y;
        hid_num_to_gpio[NS_BUTTON_Y]=GPIO_BUTTON_X;
    }
    if(user_config.cross_key_disabled){
        hid_num_to_gpio[NS_BUTTON_UP]=GPIO_BUTTON_NONE;
        hid_num_to_gpio[NS_BUTTON_DOWN]=GPIO_BUTTON_NONE;
        hid_num_to_gpio[NS_BUTTON_LEFT]=GPIO_BUTTON_NONE;
        hid_num_to_gpio[NS_BUTTON_RIGHT]=GPIO_BUTTON_NONE;
    }
    //printf("board_type %d\r\n",PCB_TYPE);
}
void gpio_init(void){
    gpio_tb_init();
    GPIO_InitTypeDef  GPIO_InitStructure = {0};
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);-
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;//input pullup

    //GPIO_InitStructure.GPIO_Speed=;input dont need
    for(int i=0;i<GPIO_INPUT_CNT;++i){
        if(hid_num_to_gpio[i]&GPIOA_PIN_MASK){//gpio group A
            ////printf("set gpio a %d\r\n",(uint16_t)hid_num_to_gpio[i]);
            GPIO_InitStructure.GPIO_Pin|=hid_num_to_gpio[i];
        }
    }
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin=0;
    for(int i=0;i<GPIO_INPUT_CNT;++i){
        if(hid_num_to_gpio[i]&GPIOB_PIN_MASK){//gpio group B
            ////printf("set gpio b %d\r\n",(uint16_t)hid_num_to_gpio[i]);
            GPIO_InitStructure.GPIO_Pin|=hid_num_to_gpio[i];
        }
    }
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin=0;
    for(int i=0;i<GPIO_INPUT_CNT;++i){
        if(hid_num_to_gpio[i]&GPIOC_PIN_MASK){//gpio group C
            ////printf("set gpio c %d\r\n",(uint16_t)hid_num_to_gpio[i]);
            GPIO_InitStructure.GPIO_Pin|=hid_num_to_gpio[i];
        }
    }
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}
uint32_t gpio_kb_scan(){
    return 0;
}
uint32_t gpio_read_all(void){
    uint8_t ret=0;
    uint32_t res=0;
    if(user_config.input_typ){
        //return gpio_kb_scan();
    }
    for(int i=0;i<GPIO_INPUT_CNT;++i){
        switch(hid_num_to_gpio[i] & GPIO_GROUP_MASK){
        case GPIOA_PIN_MASK:
            ret=GPIO_ReadInputDataBit(GPIOA,hid_num_to_gpio[i]);
            break;
        case GPIOB_PIN_MASK:
            ret=GPIO_ReadInputDataBit(GPIOB,hid_num_to_gpio[i]);
            break;
        case GPIOC_PIN_MASK:
            ret=GPIO_ReadInputDataBit(GPIOC,hid_num_to_gpio[i]);
            break;
        default:
            ret=1;
            break;
        }
        //if(!ret)
        //    //printf("set %d\r\n",i);
        res|=((!ret)<<i);
    }
    return res;
}

uint8_t gpio_read(uint32_t gpio_num)
{
    switch(gpio_num&GPIO_GROUP_MASK){
        case GPIOA_PIN_MASK:
            return GPIO_ReadInputDataBit(GPIOA,gpio_num&~GPIO_GROUP_MASK);
        case GPIOB_PIN_MASK:
            return GPIO_ReadInputDataBit(GPIOB,gpio_num&~GPIO_GROUP_MASK);
        case GPIOC_PIN_MASK:
            return GPIO_ReadInputDataBit(GPIOC,gpio_num&~GPIO_GROUP_MASK);
        default:
            return 1;
    }
}
