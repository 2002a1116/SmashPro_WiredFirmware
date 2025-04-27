/*
 * tick.c
 *
 *  Created on: 2025年2月27日
 *      Author: Reed
 */
#include "debug.h"
#include "tick.h"
//void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

volatile uint32_t Systick_MS;
static uint16_t const * const Systick_CLK=&TIM2->CNT;
void _systick_init(void){
    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //使能定时器2时钟
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;//定义定时器2结构体
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;//设置时钟分频因子
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;//设置计数模式为向上计数
    TIM_TimeBaseStructure.TIM_Period=1000-1;//计数周期，计数到1000-1被重置为0，并且产生一个更新中断
    TIM_TimeBaseStructure.TIM_Prescaler=72-1;//设置预分频器72  PSC  即时钟频率为1MHZ
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //初始化定时器2
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); //使能定时器2更新中断
    TIM_Cmd(TIM2, ENABLE); //使能定时器2
    NVIC_InitTypeDef NVIC_InitStructure; //定义NVIC初始化结构体
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn; //设置NVIC通道为定时器2中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //设置NVIC通道抢占优先级为0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; //设置NVIC通道子优先级为0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能NVIC通道
    NVIC_Init(&NVIC_InitStructure); //初始化NVIC
    NVIC_SetFastIRQ(TIM2_IRQHandler,TIM2_IRQn,1);
}
void SysTick_Init(void)//定时器2初始化函数
{
    static inited=0;
    if(inited)return;
    inited=1;
    _systick_init();

    //printf("systick init comp\r\n");
}

void TIM2_IRQHandler(void)//定时器2中断服务函数，硬件自动调用，不需要手动调用
{
    if(TIM_GetFlagStatus(TIM2,TIM_FLAG_Update)==1)//判断定时器2更新标志位是否产生
    {
        Systick_MS++;//计数值加1 T=1000/1MHZ
        TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update); //清除定时器2更新标志位
    }
}
inline uint32_t Get_Systick_US(){//chip is not fast enough to maintain a us tick,so just bare with it。
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
