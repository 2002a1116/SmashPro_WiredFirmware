/*
 * timer.c
 *
 *  Created on: 2024年11月14日
 *      Author: Reed
 */

#include "ch32v10x.h"                 // Device header
/*
void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

static uint32_t global_tick = 0; //定义一个变量count

void TIM2_Init(void)//定时器2初始化函数
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //使能定时器2时钟

    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;//定义定时器2结构体
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;//设置时钟分频因子
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;//设置计数模式为向上计数
    TIM_TimeBaseStructure.TIM_Period=1000-1;//计数周期，计数到1000-1被重置为0，并且产生一个更新中断
    TIM_TimeBaseStructure.TIM_Prescaler=72-1;//设置预分频器72  PSC  即时钟频率为1MHZ
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //初始化定时器2

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); //使能定时器2更新中断

    TIM_Cmd(TIM2, ENABLE); //使能定时器2

    //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC优先级分组2



    NVIC_InitTypeDef NVIC_InitStructure; //定义NVIC初始化结构体
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn; //设置NVIC通道为定时器2中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //设置NVIC通道抢占优先级为0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; //设置NVIC通道子优先级为0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能NVIC通道
    NVIC_Init(&NVIC_InitStructure); //初始化NVIC
}

void TIM2_IRQHandler(void)//定时器2中断服务函数，硬件自动调用，不需要手动调用
{
    if(TIM_GetFlagStatus(TIM2,TIM_FLAG_Update)==1)//判断定时器2更新标志位是否产生
    {
        global_tick++;//计数值加1 T=1000/1MHZ
        TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update); //清除定时器2更新标志位
    }
}
uint32_t timer_get_tick()//返回计数值
{
    return global_tick;
}*/

