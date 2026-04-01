/*
 * pwr.c
 *
 *  Created on: 2024Äê11ÔÂ12ÈÕ
 *      Author: Reed
 */
#include "pwr.h"
#include "debug.h"
#include "gpio_digit.h"
#include "conf.h"
/*uint32_t light_sleep_exti_gpio[]={GPIO_BUTTON_LS,GPIO_BUTTON_RS,GPIO_BUTTON_X,GPIO_BUTTON_Y,GPIO_BUTTON_A,
        GPIO_BUTTON_B,GPIO_BUTTON_UP,GPIO_BUTTON_DOWN,GPIO_BUTTON_LEFT,GPIO_BUTTON_RIGHT,GPIO_BUTTON_L,GPIO_BUTTON_R,
        GPIO_BUTTON_ZL,GPIO_BUTTON_ZR,GPIO_BUTTON_MINUS,GPIO_BUTTON_PLUS,GPIO_BUTTON_HOME,GPIO_BUTTON_CAP,
        GPIO_Pin_9|GPIOA_GROUP_MASK,GPIO_Pin_10|GPIOA_GROUP_MASK};//uart1*/
//well there will be no such thing as for one pin number,channel a/b/c can have only one related to its exti
/*
void EXTI0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));*/

uint8_t force_esp32_active;
uint32_t deep_sleep_exti_gpio[]={GPIO_BUTTON_HOME,GPIO_BUTTON_PLUS,GPIO_BUTTON_MINUS,GPIO_BUTTON_CAP,GPIO_BUTTON_TOP};
void setup_exti(uint8_t state){
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    /* GPIOB.1 ----> EXTI_Line1 *///HOME
    /*GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource15);
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init(&EXTI_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource15);
    EXTI_InitStructure.EXTI_Line = EXTI_Line15;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init(&EXTI_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource6);
    EXTI_InitStructure.EXTI_Line = EXTI_Line6;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init(&EXTI_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource7);
    EXTI_InitStructure.EXTI_Line = EXTI_Line7;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init(&EXTI_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);
    EXTI_InitStructure.EXTI_Line = EXTI_Line14;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init(&EXTI_InitStructure);
*/
    uint32_t t = GPIO_KB_SCAN_3;
    _gpio_init(&t,1,GPIO_Mode_IPD);
    _gpio_init(kb_pull,4,GPIO_Mode_Out_PP);
    for(int i=0;i<4;++i){
        gpio_set(kb_pull[i], 1);
    }
    //Delay_Us(10);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource3);
    EXTI_InitStructure.EXTI_Line = EXTI_Line3;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    //EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init(&EXTI_InitStructure);
return;

    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = state;
    NVIC_Init(&NVIC_InitStructure);
    EXTI_ClearITPendingBit(EXTI_Line3);
}
void top_init(){
    GPIO_InitTypeDef  GPIO_InitStructure = {0};
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;//input pullup
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}
void set_pwr_mode_sleep(void){
    //sleepdeep set 1,
    _force_rgb(DISABLE);
    Delay_Ms(10);
    if(((RCC->CFGR0 & RCC_SWS)==0x04)||((RCC->CFGR0 & RCC_PLLSRC) == RCC_PLLSRC))
        RCC_HSICmd(DISABLE);
    __WFE();
}
uint32_t* PFIC_SCTLR=(uint32_t*)0xD10;
uint32_t* PWR_CSR=(uint32_t*)0x004;
uint8_t set_pwr_mode_stop(void){
    _force_rgb(DISABLE);
    Delay_Ms(10);
    printf("sleep\r\n");
    set_imu_sleep();
    set_peripherals_state(DISABLE);
    setup_exti(ENABLE);
    for(int i=0;i<1e7;++i);//NO DELAY HERE,SYSTICK NOT INITED.
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFE);

    setup_exti(DISABLE);
    SetSysClock();
    SystemCoreClockUpdate();
    USART_printf_Init(115200);
    set_peripherals_state(ENABLE);
    _force_rgb(ENABLE);
    flush_rgb();
    return 0;
}
/*
void wakeup_irq_handler(void)
{
    EXTI_ClearITPendingBit(EXTI_Line0);
    EXTI_ClearITPendingBit(EXTI_Line1);
    EXTI_ClearITPendingBit(EXTI_Line2);
    EXTI_ClearITPendingBit(EXTI_Line3);
    //todo:wake esp32
    esp32_bt_status=0;
}
void EXTI0_IRQHandler(void){wakeup_irq_handler();}
void EXTI1_IRQHandler(void){wakeup_irq_handler();}
void EXTI2_IRQHandler(void){wakeup_irq_handler();}
void EXTI3_IRQHandler(void){wakeup_irq_handler();}
*/
