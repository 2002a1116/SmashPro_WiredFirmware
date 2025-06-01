/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32v10x_it.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2020/04/30
 * Description        : Main Interrupt Service Routines.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "ch32v10x_it.h"
#include "conf.h"
#include "tick.h"

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*********************************************************************
 * @fn      NMI_Handler
 *
 * @brief   This function handles NMI exception.
 *
 * @return  none
 */
void NMI_Handler(void)
{
    flush_rgb(DISABLE);
    RCC->INTR|=RCC_CSSC;
    SetSysClock();//restart crystal
    PFIC->CFGR|=(1<<3);
    //while(1);
}

/*********************************************************************
 * @fn      HardFault_Handler
 *
 * @brief   This function handles Hard Fault exception.
 *
 * @return  none
 */
uint32_t EXC_CAUSE;
uint32_t EXC_PC;

void HardFault_Handler(void)
{
    flush_rgb(DISABLE);
    //csrr a0, mstatus
    __asm volatile ("csrr %0, mcause" : \
                "=r"(EXC_CAUSE): : "memory");
    __asm volatile ("csrr %0, mepc" : \
                    "=r"(EXC_PC): : "memory");
    user_calibration.mcause=EXC_CAUSE;
    user_calibration.mepc=EXC_PC;
    //flash_write(0, (uint8_t*)&user_calibration, sizeof(user_calibration));
    //HighPrecisionTimerDelayUs(2000);
    //Delay_Ms(2);
    //conf_write(addr, buf, size)
    if(!(EXC_CAUSE>>31)){//fault
        switch(EXC_CAUSE){
        case 0://instruction not aligned
            /*fall through*/
            //break;
        case 1://fetch-instruction memory access fail
            /*fall through*/
            //break;
        case 2://illegal instruction
            /*fall through*/
        case 4://load instruction address not aligned
            /*fall through*/
            //break;
        case 5://load instruction memory access fail
            /*fall through*/
            //break;
        case 6://Store/AMO instruction address not aligned
            /*fall through*/
            //break;
        case 7://Store/AMO instruction memory access fail
            __asm volatile ("csrw mepc, %0" : \
            : "r"(EXC_PC+4):);
            break;
        case 3://break point
            while(1);//do nothing
            break;
        case 8://user mode syscall
            //why this is a hardfault? or i just understand it wrong?
            /*fall through*/
        case 11://machine mode syscall
            break;
        default:
            break;
        }
    }
    PFIC->CFGR|=(1<<5);
    //while(1);
}
