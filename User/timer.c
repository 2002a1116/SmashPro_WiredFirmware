/*
 * timer.c
 *
 *  Created on: 2024��11��14��
 *      Author: Reed
 */

#include "ch32v10x.h"                 // Device header
/*
void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

static uint32_t global_tick = 0; //����һ������count

void TIM2_Init(void)//��ʱ��2��ʼ������
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //ʹ�ܶ�ʱ��2ʱ��

    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;//���嶨ʱ��2�ṹ��
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;//����ʱ�ӷ�Ƶ����
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;//���ü���ģʽΪ���ϼ���
    TIM_TimeBaseStructure.TIM_Period=1000-1;//�������ڣ�������1000-1������Ϊ0�����Ҳ���һ�������ж�
    TIM_TimeBaseStructure.TIM_Prescaler=72-1;//����Ԥ��Ƶ��72  PSC  ��ʱ��Ƶ��Ϊ1MHZ
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //��ʼ����ʱ��2

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); //ʹ�ܶ�ʱ��2�����ж�

    TIM_Cmd(TIM2, ENABLE); //ʹ�ܶ�ʱ��2

    //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC���ȼ�����2



    NVIC_InitTypeDef NVIC_InitStructure; //����NVIC��ʼ���ṹ��
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn; //����NVICͨ��Ϊ��ʱ��2�ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //����NVICͨ����ռ���ȼ�Ϊ0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; //����NVICͨ�������ȼ�Ϊ0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //ʹ��NVICͨ��
    NVIC_Init(&NVIC_InitStructure); //��ʼ��NVIC
}

void TIM2_IRQHandler(void)//��ʱ��2�жϷ�������Ӳ���Զ����ã�����Ҫ�ֶ�����
{
    if(TIM_GetFlagStatus(TIM2,TIM_FLAG_Update)==1)//�ж϶�ʱ��2���±�־λ�Ƿ����
    {
        global_tick++;//����ֵ��1 T=1000/1MHZ
        TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update); //�����ʱ��2���±�־λ
    }
}
uint32_t timer_get_tick()//���ؼ���ֵ
{
    return global_tick;
}*/

