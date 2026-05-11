/*
 * pwr.c
 *
 *  Created on: 2024年11月12日
 *      Author: Reed
 */
#include "pwr.h"
#include "spi.h"
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
volatile uint8_t Voltage_ThresFlag=0;
PWR_VDD pwr_vdd_voltage()
{
    PWR_VDD tmp = PWR_VDD_SupplyVoltage();
    pwr_init();
    //Delay_Ms(2);
    //Voltage_ThresFlag = PWR_GetFlagStatus(PWR_FLAG_PVDO);
    return tmp;
}
void pwr_init(){
    /*NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    EXTI_ClearITPendingBit(EXTI_Line18);
    EXTI_InitStructure.EXTI_Line = EXTI_Line18;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    EXTI_Init(&EXTI_InitStructure);*/

    PWR_PVDLevelConfig(PWR_PVDLevel_MODE2); // VDD电压低于3.07V时触发中断,高于2.89V触发中断
    PWR_PVDCmd(ENABLE);
    Delay_Ms(1);
    Voltage_ThresFlag = PWR_GetFlagStatus(PWR_FLAG_PVDO);
}
void setup_exti(uint8_t state){
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);

    uint32_t t = GPIO_KB_SCAN_3;
    _gpio_init(&t,1,GPIO_Mode_IPD);
    _gpio_init(kb_pull,4,GPIO_Mode_Out_PP);
    for(int i=0;i<4;++i){
        gpio_set(kb_pull[i], 1);
    }
    //for(int i=0;i<1e7;++i);//NO DELAY HERE,SYSTICK NOT INITED.
    /*if(state)
        Delay_Ms(5);
    _gpio_init(&t,1,GPIO_Mode_IN_FLOATING);*/
    if(state)
        Delay_Ms(5);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource3);
    EXTI_InitStructure.EXTI_Line = EXTI_Line3;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    //EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init(&EXTI_InitStructure);
    EXTI_InitStructure.EXTI_Line = EXTI_Line18;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    //EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init(&EXTI_InitStructure);

return;
    t=GPIO_Pin_6|GPIOB_GROUP_MASK;
    _gpio_init(&t, 1, GPIO_Mode_IPD);
    t=GPIO_Pin_7|GPIOB_GROUP_MASK;
    _gpio_init(&t, 1, GPIO_Mode_IPD);
    EXTI_InitStructure.EXTI_Line = EXTI_Line7;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init( &EXTI_InitStructure );
    EXTI_InitStructure.EXTI_Line = EXTI_Line6;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init( &EXTI_InitStructure );
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource7);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource6);


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
uint8_t set_pwr_mode_stop(void){
    led_pwr_ctrl(DISABLE);
    hd_rumble_set_status(DISABLE);
    _force_rgb(DISABLE);
    Delay_Ms(10);
    printf("sleep\r\n");
    set_imu_sleep();
    setup_exti(ENABLE);
    R8_UDEV_CTRL = 0;
    //set_peripherals_state(DISABLE);
    //EXTEN->EXTEN_CTR &= ~EXTEN_USBFS_IO_EN;
    //__disable_irq( );
    gpio_set(GPIO_WAKE_BAND, DISABLE);
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFE);
    //while(1);
    //for(int i=0;i<5e7;++i);//NO DELAY HERE,SYSTICK NOT INITED.
    //__enable_irq();
    SetSysClock();
    SystemCoreClockUpdate();
    USART_printf_Init(115200);
    setup_exti(DISABLE);
    init_all();
    return 0;
}
