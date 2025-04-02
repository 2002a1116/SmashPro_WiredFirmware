/*
 * pwr.c
 *
 *  Created on: 2024Äê11ÔÂ12ÈÕ
 *      Author: Reed
 */

//if bt disconnected for too long or lack of input for too long or other reason,
//esp32 actively put ch32 into sleep before it sleep on it own

//when usb intr or gpio exti wake ch32,ch32 will try to wake esp32 through uart as theres no more ch32 pins for inter connect
//todo:is one uart pkt will be enough?
//may be we can use led pin to do so? as when controller awake,top led should be light as an indication of controlller running.
//todo:pcb dont have such wire,consider mod pcb to support it
//as led pin will be set as drain,esp32 should pullup and monitor a falling edge

#include "pwr.h"
#include "debug.h"
#include "gpio_digit.h"
#include "uart_com.h"
/*uint32_t light_sleep_exti_gpio[]={GPIO_BUTTON_LS,GPIO_BUTTON_RS,GPIO_BUTTON_X,GPIO_BUTTON_Y,GPIO_BUTTON_A,
        GPIO_BUTTON_B,GPIO_BUTTON_UP,GPIO_BUTTON_DOWN,GPIO_BUTTON_LEFT,GPIO_BUTTON_RIGHT,GPIO_BUTTON_L,GPIO_BUTTON_R,
        GPIO_BUTTON_ZL,GPIO_BUTTON_ZR,GPIO_BUTTON_MINUS,GPIO_BUTTON_PLUS,GPIO_BUTTON_HOME,GPIO_BUTTON_CAP,
        GPIO_Pin_9|GPIOA_PIN_MASK,GPIO_Pin_10|GPIOA_PIN_MASK};//uart1*/
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

    /* GPIOB.1 ----> EXTI_Line1 *///HOME
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init(&EXTI_InitStructure);

    /* GPIOC.15 ----> EXTI_Line15 *///PLUS
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource15);
    EXTI_InitStructure.EXTI_Line = EXTI_Line15;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init(&EXTI_InitStructure);

    /* GPIOB.6 ----> EXTI_Line6 *///MINUS
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource6);
    EXTI_InitStructure.EXTI_Line = EXTI_Line6;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init(&EXTI_InitStructure);

    /* GPIOB.7 ----> EXTI_Line7 *///CAP
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource7);
    EXTI_InitStructure.EXTI_Line = EXTI_Line7;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init(&EXTI_InitStructure);

    /* GPIOB.14 ----> EXTI_Line14 *///TOP
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);
    EXTI_InitStructure.EXTI_Line = EXTI_Line14;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
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
    setup_exti(ENABLE);
    RCC_LSICmd(ENABLE);
    flush_rgb(DISABLE);
    Delay_Us(100);
    if(!RCC_GetFlagStatus(RCC_FLAG_LSIRDY))
        return 1;
    {
        printf("PUT TO SLEEP\r\n");
        Delay_Ms(1);
        //set_pwr_mode_sleep();
        PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFE);
        //init_all();
        Delay_Ms(1);
    }
    //Delay_Ms(100);
    //todo:wake esp32
    setup_exti(DISABLE);
    //Delay_Init();
    //USART_Printf_Init(115200);
    //wake esp32 through top button.
    //this is some clumsy fk but at least better than use the backup plan(wkup pin) as its also used as right joystick button.
    //fk espressif for write uart2 cant wake esp32 not in the sleep mode document
    //but in the fking api description for esp_sleep_enable_uart_wakeup()
    //and cant wake through uart0/1 unless signal came straight out io mux and bypass gpio matrix
    //so actually we cant really wakeup esp32 through uart as uart1 stock pin is used by modules for psram
    //and uart0 is used for debug and firmware flash,
    //if we connect it with ch32,ch32 may be interfered during flash as they are put to download in the same time by top button
    //esp32 will print some hints when download,i dont want to risk messing up ch32 flashing as it handles all the core functions.
    //fk espressif again as they could have put all these rather critical things in the beginning;
    //fk fk fk.i just dont want to read 1000 pages of chip datasheet before i start all the job,otherwise i just dont want to start at all.

    //i dont konw.i just dont have enough gpio.should i use keyboard scan like stock or progcc(i dont want to do that)
    //or shall i just switch to a 64pin package to have more gpio and redesign the pcb?
    //and should i switch to ch32v203 with 144mhz and better core architecture for better performance?

    //cant be bothered by now,this will do,and i all stick with it for now.
    //Delay_Us(1000);
    //NVIC_SystemReset();
    //connection_state.esp32_bt_status=0;
    //connection_state.esp32_connected=0;
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
