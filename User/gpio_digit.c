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
#include "tick.h"
GPIO_TypeDef* _gpio_mask_to_group[8]={NULL,GPIOA,GPIOB,NULL,GPIOC,NULL,NULL,/*GPIOD*/NULL};
GPIO_TypeDef* _gpio_mask_to_group_unsafe[8]={GPIOD,GPIOA,GPIOB,GPIOD,GPIOC,GPIOD,GPIOD,/*GPIOD*/GPIOD};
#define GPIO_SHIFTED_MASK(x) (((x)&GPIO_GROUP_MASK)>>16)
#define GPIO_PIN(x) ((x)&GPIO_PIN_MASK)
#define GPIO_SET_SAFE(x,v) (_gpio_mask_to_group[GPIO_SHIFTED_MASK(x)]?GPIO_WriteBit(_gpio_mask_to_group[GPIO_SHIFTED_MASK(x)],GPIO_PIN(x),(v)),1:-1)
#define GPIO_SET_UNSAFE(x,v) (GPIO_WriteBit(_gpio_mask_to_group_unsafe[GPIO_SHIFTED_MASK(x)],GPIO_PIN(x),(v)))
#define GPIO_READ_SAFE(x) (_gpio_mask_to_group[GPIO_SHIFTED_MASK(x)]?GPIO_ReadInputDataBit(_gpio_mask_to_group[GPIO_SHIFTED_MASK(x)],GPIO_PIN(x)):-1)
#define GPIO_READ_UNSAFE(x) (GPIO_ReadInputDataBit(_gpio_mask_to_group_unsafe[GPIO_SHIFTED_MASK(x)],GPIO_PIN(x)))

#define GPIO_SET(x,v) (GPIO_SET_UNSAFE((x),(v)))
#define GPIO_READ(x) ((uint32_t)GPIO_READ_UNSAFE(x))
//GPIO_ReadOutputDataBit
uint32_t sts_button;
uint32_t hid_num_to_gpio[GPIO_INPUT_CNT]={};
uint32_t kb_scan_to_hid[4][4]={
        {NS_BUTTON_X,NS_BUTTON_UP,NS_BUTTON_PLUS,NS_BUTTON_ZL},
        {NS_BUTTON_Y,NS_BUTTON_DOWN,NS_BUTTON_HOME,NS_BUTTON_R},
        {NS_BUTTON_A,NS_BUTTON_RIGHT,NS_BUTTON_MINUS,NS_BUTTON_ZR},
        {NS_BUTTON_B,NS_BUTTON_LEFT,NS_BUTTON_LEFT,NS_BUTTON_L}};//(pull_i,scan_i)=>button_hid
uint32_t kb_pull[]={GPIO_KB_PULL_1,GPIO_KB_PULL_2,GPIO_KB_PULL_3,GPIO_KB_PULL_4};
uint32_t kb_scan[]={GPIO_KB_SCAN_1,GPIO_KB_SCAN_2,GPIO_KB_SCAN_3,GPIO_KB_SCAN_4};
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
void _gpio_init(uint32_t* arr,uint8_t n,GPIOMode_TypeDef mode){
    GPIO_InitTypeDef  GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Mode=mode;//input pullup
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    for(int i=0;i<n;++i){
        if(arr[i]&GPIOA_GROUP_MASK){//gpio group A
            ////printf("set gpio a %d\r\n",(uint16_t)hid_num_to_gpio[i]);
            GPIO_InitStructure.GPIO_Pin|=arr[i];
        }
    }
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin=0;
    for(int i=0;i<n;++i){
        if(arr[i]&GPIOB_GROUP_MASK){//gpio group B
            ////printf("set gpio b %d\r\n",(uint16_t)hid_num_to_gpio[i]);
            GPIO_InitStructure.GPIO_Pin|=arr[i];
        }
    }
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin=0;
    for(int i=0;i<n;++i){
        if(arr[i]&GPIOC_GROUP_MASK){//gpio group C
            ////printf("set gpio c %d\r\n",(uint16_t)hid_num_to_gpio[i]);
            GPIO_InitStructure.GPIO_Pin|=arr[i];
        }
    }
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}
void gpio_kb_scan_init()
{
    _gpio_init(kb_scan,4,GPIO_Mode_IPD);
    _gpio_init(kb_pull,4,GPIO_Mode_Out_PP);
    for(int i=0;i<4;++i){
        for(int j=0;j<4;++j){
            hid_num_to_gpio[kb_scan_to_hid[i][j]]=GPIO_BUTTON_NONE;//remove kb scan part
        }
    }
}
void gpio_init(void){
    gpio_tb_init();
    if(user_config.input_typ){//scan
        gpio_kb_scan_init();
    }
    _gpio_init(hid_num_to_gpio,GPIO_INPUT_CNT,GPIO_Mode_IPU);
    /*GPIO_InitTypeDef  GPIO_InitStructure = {0};
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;//input pullup

    //GPIO_InitStructure.GPIO_Speed=;input dont need
    for(int i=0;i<GPIO_INPUT_CNT;++i){
        if(hid_num_to_gpio[i]&GPIOA_GROUP_MASK){//gpio group A
            ////printf("set gpio a %d\r\n",(uint16_t)hid_num_to_gpio[i]);
            GPIO_InitStructure.GPIO_Pin|=hid_num_to_gpio[i];
        }
    }
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin=0;
    for(int i=0;i<GPIO_INPUT_CNT;++i){
        if(hid_num_to_gpio[i]&GPIOB_GROUP_MASK){//gpio group B
            ////printf("set gpio b %d\r\n",(uint16_t)hid_num_to_gpio[i]);
            GPIO_InitStructure.GPIO_Pin|=hid_num_to_gpio[i];
        }
    }
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin=0;
    for(int i=0;i<GPIO_INPUT_CNT;++i){
        if(hid_num_to_gpio[i]&GPIOC_GROUP_MASK){//gpio group C
            ////printf("set gpio c %d\r\n",(uint16_t)hid_num_to_gpio[i]);
            GPIO_InitStructure.GPIO_Pin|=hid_num_to_gpio[i];
        }
    }
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    */
}
uint32_t gpio_kb_scan(){
    uint32_t res=0;
    for(uint8_t i=0;i<4;++i){
        GPIO_SET(kb_pull[i],1);
        Delay_Us(KB_SCAN_SETUP_TIME);
        for(uint8_t j=0;j<4;++j){
            res|=(GPIO_READ(kb_scan[j])<<kb_scan_to_hid[i][j]);
        }
        GPIO_SET(kb_pull[i],0);
        Delay_Us(KB_SCAN_SETUP_TIME);
    }
    return res;
}
uint32_t gpio_read_all(void){
    uint8_t ret=0;
    uint32_t res=0;
    if(user_config.input_typ){
        res=gpio_kb_scan();
    }
    for(int i=0;i<GPIO_INPUT_CNT;++i){
        switch(hid_num_to_gpio[i] & GPIO_GROUP_MASK){
        case GPIOA_GROUP_MASK:
            ret=GPIO_ReadInputDataBit(GPIOA,hid_num_to_gpio[i]);
            break;
        case GPIOB_GROUP_MASK:
            ret=GPIO_ReadInputDataBit(GPIOB,hid_num_to_gpio[i]);
            break;
        case GPIOC_GROUP_MASK:
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
        case GPIOA_GROUP_MASK:
            return GPIO_ReadInputDataBit(GPIOA,gpio_num&~GPIO_GROUP_MASK);
        case GPIOB_GROUP_MASK:
            return GPIO_ReadInputDataBit(GPIOB,gpio_num&~GPIO_GROUP_MASK);
        case GPIOC_GROUP_MASK:
            return GPIO_ReadInputDataBit(GPIOC,gpio_num&~GPIO_GROUP_MASK);
        default:
            return 1;
    }
}
