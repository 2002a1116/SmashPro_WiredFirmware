/*
 * watchdog.c
 *
 *  Created on: 2025年4月14日
 *      Author: Reed
 */
#include "debug.h"
#include "conf.h"
#ifndef USER_WATCHDOG_C_
#define USER_WATCHDOG_C_

void TIM4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
uint32_t last_feed_ms;
uint8_t watchdog_pos;
void sofw_watchdog_init()
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //使能定时器2时钟
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;//定义定时器2结构体
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;//设置时钟分频因子
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;//设置计数模式为向上计数
    TIM_TimeBaseStructure.TIM_Period=1000-1;//计数周期，计数到1000-1被重置为0，并且产生一个更新中断
    TIM_TimeBaseStructure.TIM_Prescaler=9-1;//设置预分频器72  PSC  即时钟频率为1MHZ
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //初始化定时器2
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE); //使能定时器2更新中断
    TIM_Cmd(TIM4, ENABLE); //使能定时器2
    NVIC_InitTypeDef NVIC_InitStructure; //定义NVIC初始化结构体
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn; //设置NVIC通道为定时器2中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //设置NVIC通道抢占优先级为0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //设置NVIC通道子优先级为0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能NVIC通道
    soft_watchdog_feed();
    NVIC_Init(&NVIC_InitStructure); //初始化NVIC
    //NVIC_SetFastIRQ(TIM4_IRQHandler,TIM4_IRQn,1);
}
void soft_watchdog_feed()
{
    last_feed_ms=Get_Systick_MS();
}
void TIM4_IRQHandler(void){
    if(TIM_GetFlagStatus(TIM4,TIM_FLAG_Update)==1)//判断定时器2更新标志位是否产生
    {
        if(Get_Systick_MS()-last_feed_ms>1000){//timeout
            if(watchdog_pos){
                user_calibration.tag=watchdog_pos;
                //flash_write(0, &user_calibration, 128);
            }
        }
        TIM_ClearITPendingBit(TIM4, TIM_FLAG_Update); //清除定时器2更新标志位
    }
}

#endif /* USER_WATCHDOG_C_ */
