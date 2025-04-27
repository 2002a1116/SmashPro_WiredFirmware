/*
 * tick.c
 *
 *  Created on: 2025��2��27��
 *      Author: Reed
 */
#include "debug.h"
#include "tick.h"
//void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

volatile uint32_t Systick_MS;
static uint16_t const * const Systick_CLK=&TIM2->CNT;
void _systick_init(void){
    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //ʹ�ܶ�ʱ��2ʱ��
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;//���嶨ʱ��2�ṹ��
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;//����ʱ�ӷ�Ƶ����
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;//���ü���ģʽΪ���ϼ���
    TIM_TimeBaseStructure.TIM_Period=1000-1;//�������ڣ�������1000-1������Ϊ0�����Ҳ���һ�������ж�
    TIM_TimeBaseStructure.TIM_Prescaler=72-1;//����Ԥ��Ƶ��72  PSC  ��ʱ��Ƶ��Ϊ1MHZ
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //��ʼ����ʱ��2
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); //ʹ�ܶ�ʱ��2�����ж�
    TIM_Cmd(TIM2, ENABLE); //ʹ�ܶ�ʱ��2
    NVIC_InitTypeDef NVIC_InitStructure; //����NVIC��ʼ���ṹ��
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn; //����NVICͨ��Ϊ��ʱ��2�ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //����NVICͨ����ռ���ȼ�Ϊ0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; //����NVICͨ�������ȼ�Ϊ0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //ʹ��NVICͨ��
    NVIC_Init(&NVIC_InitStructure); //��ʼ��NVIC
    NVIC_SetFastIRQ(TIM2_IRQHandler,TIM2_IRQn,1);
}
void SysTick_Init(void)//��ʱ��2��ʼ������
{
    static inited=0;
    if(inited)return;
    inited=1;
    _systick_init();

    //printf("systick init comp\r\n");
}

void TIM2_IRQHandler(void)//��ʱ��2�жϷ�������Ӳ���Զ����ã�����Ҫ�ֶ�����
{
    if(TIM_GetFlagStatus(TIM2,TIM_FLAG_Update)==1)//�ж϶�ʱ��2���±�־λ�Ƿ����
    {
        Systick_MS++;//����ֵ��1 T=1000/1MHZ
        TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update); //�����ʱ��2���±�־λ
    }
}
inline uint32_t Get_Systick_US(){//chip is not fast enough to maintain a us tick,so just bare with it��
    return Systick_MS*1000+(*Systick_CLK);
}
inline uint64_t Get_Systick_US64(){
    return Systick_MS*1000ull+(*Systick_CLK);
}
inline uint32_t Get_Systick_MS(){
    return Systick_MS;
}
static uint8_t  p_us = 0;
static uint16_t p_ms = 0;
static uint64_t Systick_Trigger;
uint8_t Systick_Inited;
void _set_systick_cmp(uint64_t n);
inline void _set_systick_cmp(uint64_t n){
    SysTick->CMPLR0 = n;
    SysTick->CMPLR1 = n>>8;
    SysTick->CMPLR2 = n>>16;
    SysTick->CMPLR3 = n>>24;
    SysTick->CMPHR0 = n>>32;
    SysTick->CMPHR1 = n>>40;
    SysTick->CMPHR2 = n>>48;
    SysTick->CMPHR3 = n>>56;
    ////printf("set systick :%llu %llu",n,(((uint64_t)*(uint32_t*)(&SysTick->CMPLR0))<<32)+*(uint32_t*)(&SysTick->CMPHR0));
}
void _reset_systick_cnt();
inline void _reset_systick_cnt(){
    SysTick->CNTL0 = 0;
    SysTick->CNTL1 = 0;
    SysTick->CNTL2 = 0;
    SysTick->CNTL3 = 0;
    SysTick->CNTH0 = 0;
    SysTick->CNTH1 = 0;
    SysTick->CNTH2 = 0;
    SysTick->CNTH3 = 0;
}
void HighPrecisionTimer_Init(void)
{
    p_us = SystemCoreClock / 8000000;
    p_ms = (uint16_t)p_us * 1000;
}
void HighPrecisionTimerCmd(uint8_t status){
    SysTick->CTLR=status;
}
void HighPrecisionTimerStart(){
    SysTick->CTLR=0;
    _reset_systick_cnt();
    SysTick->CTLR=1;
}
uint64_t HighPrecisionTimerCnt(){
    return (((uint64_t)SysTick->CNTH)<<32)+SysTick->CNTL;
}
uint64_t HighPrecisionTimerUs(){
    return HighPrecisionTimerCnt()/p_us;
}
void HighPrecisionTimerDelayUs(uint32_t us){
    HighPrecisionTimerStart();
    us*=p_us;
    while(HighPrecisionTimerCnt()<us);
}
uint8_t wait_nonblocking_us(uint8_t (*func)(void*),void* param,uint32_t n)
{
    if(!func)return 0;
    volatile uint32_t tp=Get_Systick_US();
    //volatile uint32_t target=tp+n;
    ////printf("execute us now:%d tar:%d\r\n",tp,target);
    while(1){
        if(func(param))
            return 0;
        if(Get_Systick_US()-tp>n){
            return !func(param);
        }
    }
    return 1;
}
uint8_t wait_nonblocking_ms(uint8_t (*func)(void*),void* param,uint32_t n)
{
    if(!func)return 0;
    volatile uint32_t tp=Systick_MS;
    while(1){
        if(func(param))
            return 0;
        if(Systick_MS-tp>n)
            return !func(param);
    }
    return 1;
}
void Delay_Us(uint32_t n)
{
    volatile uint32_t tp=Get_Systick_US();
    while(Get_Systick_US()-tp<=n);
}
void Delay_Ms(uint32_t n)
{
    volatile uint32_t tp=Systick_MS;
    while(Systick_MS-tp<=n);
}
