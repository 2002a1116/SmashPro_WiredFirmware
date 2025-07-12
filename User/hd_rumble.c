/*
 * hd_rumble.c
 *
 *  Created on: 2024年12月3日
 *      Author: Reed
 */
#include "debug.h"
#include "ring_buffer.h"
#include "tick.h"
#include "hd_rumble.h"
#include "hd_rumble_high_accuracy.h"
#include "conf.h"
#include <math.h>
#include <string.h>
ring_buffer rumble_rb;
uint8_t rumble_state=1;
void hd_rumble_init(uint8_t force_disable){
    GPIO_InitTypeDef        GPIO_InitStructure = {0};
    TIM_OCInitTypeDef       TIM_OCInitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);

    if(force_disable){
        /*GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);*/
        return;
    }
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    /*
#if (PCB_TYPE==PCB_TYPE_1_0)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
#else
*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
//#endif
    TIM_TimeBaseInitStructure.TIM_Period = HD_RUMBLE_TIM_PERIOD;//72m/250
    TIM_TimeBaseInitStructure.TIM_Prescaler = HD_RUMBLE_TIM_PRESCALER-1;//
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

#if(PWM_MODE == PWM_MODE1)
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
#elif(PWM_MODE == PWM_MODE2)
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
#endif
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
/*#if (PCB_TYPE==PCB_TYPE_1_0)
    TIM_OC2Init(TIM3, &TIM_OCInitStructure);
#else*/
    TIM_OC4Init(TIM3, &TIM_OCInitStructure);
//#endif

    TIM_CtrlPWMOutputs(TIM3, ENABLE);
    TIM_OC1PreloadConfig(TIM3,TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM3,TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM3,TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM3,TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM3, DISABLE);
    /*TIM_SetCompare1(TIM3, HD_RUMBLE_AMP_FIXED_RATIO*1440/2);
    TIM_SetCompare2(TIM3, HD_RUMBLE_AMP_FIXED_RATIO*1440/2);
    TIM_SetCompare3(TIM3, HD_RUMBLE_AMP_FIXED_RATIO*1440/2);
    TIM_SetCompare4(TIM3, HD_RUMBLE_AMP_FIXED_RATIO*1440/2);*/

    //TIM_SetCompare1(TIM3, 590);
    //TIM_SetCompare2(TIM3, 590);
    TIM_SetCompare3(TIM3, HD_RUMBLE_TIM_PERIOD_MID);
    TIM_SetCompare4(TIM3, HD_RUMBLE_TIM_PERIOD_MID);

    hd_rumble_high_accurary_init();
    NVIC_InitTypeDef NVIC_InitStructure; //定义NVIC初始化结构体
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; //设置NVIC通道为定时器2中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //设置NVIC通道抢占优先级为0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6; //设置NVIC通道子优先级为0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能NVIC通道
    NVIC_Init(&NVIC_InitStructure); //初始化NVIC
    NVIC_SetFastIRQ(TIM3_IRQHandler,TIM3_IRQn,2);

    TIM_Cmd(TIM3, ENABLE);
    //ring_buffer_init(&rumble_rb, rumble_buf, HD_RUMBLE_RINGBUFFER_CAP, HD_RUMBLE_FRAME_SIZE);
}
void hd_rumble_set_status(uint8_t status)
{
    TIM_Cmd(TIM3, status);
}
