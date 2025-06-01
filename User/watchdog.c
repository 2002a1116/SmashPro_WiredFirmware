/*
 * watchdog.c
 *
 *  Created on: 2025��4��14��
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
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //ʹ�ܶ�ʱ��2ʱ��
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;//���嶨ʱ��2�ṹ��
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;//����ʱ�ӷ�Ƶ����
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;//���ü���ģʽΪ���ϼ���
    TIM_TimeBaseStructure.TIM_Period=1000-1;//�������ڣ�������1000-1������Ϊ0�����Ҳ���һ�������ж�
    TIM_TimeBaseStructure.TIM_Prescaler=9-1;//����Ԥ��Ƶ��72  PSC  ��ʱ��Ƶ��Ϊ1MHZ
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //��ʼ����ʱ��2
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE); //ʹ�ܶ�ʱ��2�����ж�
    TIM_Cmd(TIM4, ENABLE); //ʹ�ܶ�ʱ��2
    NVIC_InitTypeDef NVIC_InitStructure; //����NVIC��ʼ���ṹ��
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn; //����NVICͨ��Ϊ��ʱ��2�ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //����NVICͨ����ռ���ȼ�Ϊ0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //����NVICͨ�������ȼ�Ϊ0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //ʹ��NVICͨ��
    soft_watchdog_feed();
    NVIC_Init(&NVIC_InitStructure); //��ʼ��NVIC
    //NVIC_SetFastIRQ(TIM4_IRQHandler,TIM4_IRQn,1);
}
void soft_watchdog_feed()
{
    last_feed_ms=Get_Systick_MS();
}
void TIM4_IRQHandler(void){
    if(TIM_GetFlagStatus(TIM4,TIM_FLAG_Update)==1)//�ж϶�ʱ��2���±�־λ�Ƿ����
    {
        if(Get_Systick_MS()-last_feed_ms>1000){//timeout
            if(watchdog_pos){
                user_calibration.tag=watchdog_pos;
                //flash_write(0, &user_calibration, 128);
            }
        }
        TIM_ClearITPendingBit(TIM4, TIM_FLAG_Update); //�����ʱ��2���±�־λ
    }
}

#endif /* USER_WATCHDOG_C_ */
