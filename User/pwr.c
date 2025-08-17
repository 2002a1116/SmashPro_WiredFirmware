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
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    /* GPIOB.1 ----> EXTI_Line1 *///HOME
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource15);
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init(&EXTI_InitStructure);

    /* GPIOC.15 ----> EXTI_Line15 *///PLUS
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource15);
    EXTI_InitStructure.EXTI_Line = EXTI_Line15;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init(&EXTI_InitStructure);

    /* GPIOB.6 ----> EXTI_Line6 *///MINUS
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource6);
    EXTI_InitStructure.EXTI_Line = EXTI_Line6;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init(&EXTI_InitStructure);

    /* GPIOB.7 ----> EXTI_Line7 *///CAP
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource7);
    EXTI_InitStructure.EXTI_Line = EXTI_Line7;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init(&EXTI_InitStructure);

    /* GPIOB.14 ----> EXTI_Line14 *///TOP
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);
    EXTI_InitStructure.EXTI_Line = EXTI_Line14;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init(&EXTI_InitStructure);
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
    if(((RCC->CFGR0 & RCC_SWS)==0x04)||((RCC->CFGR0 & RCC_PLLSRC) == RCC_PLLSRC))
        RCC_HSICmd(DISABLE);
    __WFE();
}
uint32_t* PFIC_SCTLR=(uint32_t*)0xD10;
uint32_t* PWR_CSR=(uint32_t*)0x004;
uint8_t set_pwr_mode_stop(void){
    //user_config.led_disabled=1;
    flush_rgb(DISABLE);
    Delay_Ms(10);
    printf("sleep\r\n");
    set_peripherals_state(DISABLE);
    set_imu_sleep();
    setup_exti(ENABLE);
    RCC_LSICmd(ENABLE);
    if(!RCC_GetFlagStatus(RCC_FLAG_LSIRDY))
        return 1;
    {
        //printf("PUT TO SLEEP\r\n");
        //Delay_Ms(1);
        set_pwr_mode_sleep();
        //PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFE);
        //init_all();
        //Delay_Ms(1);
    }
    //Delay_Ms(100);
    //todo:wake esp32
    setup_exti(DISABLE);
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
