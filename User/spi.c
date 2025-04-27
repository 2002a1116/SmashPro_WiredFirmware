/*
 * spi.c
 *
 *  Created on: 2025年1月29日
 *      Author: Reed
 */


 /********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/01/05
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *SPI uses DMA, master / slave mode transceiver routine:
 *Master:SPI1_SCK(PA5)\SPI1_MISO(PA6)\SPI1_MOSI(PA7).
 *Slave:SPI1_SCK(PA5)\SPI1_MISO(PA6)\SPI1_MOSI(PA7).
 *
 *This example demonstrates that Master and Slave use DAM full-duplex transmission
 *and reception at the same time.
 *Note: The two boards download the Master and Slave programs respectively, and
 *power on at the same time.
 *     Hardware connection:
 *               PA5 -- PA5
 *               PA6 -- PA6
 *               PA7 -- PA7
 *
 */

#include "debug.h"
#include "string.h"
#include "conf.h"
#include "pwr.h"
#include "spi.h"
#include "board_type.h"

void DMA1_Channel3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/* Global define */

/* Global Variable */
#pragma pack(push,1)
typedef struct _rgb_spi_pkg{
    union{
        struct{
            uint32_t load:24;
        };
        uint8_t buf[3];
    };
}rgb_spi_pkg;
#pragma pack(pop)
#define SPI_RESET_OFFSET (5)
rgb_spi_pkg spi_tx_buf[RGB_MAX_CNT*3+SPI_RESET_OFFSET*2];//+reset
//uint16_t spi_length;

/*********************************************************************
 * @fn      SPI_FullDuplex_Init
 *
 * @brief   Configuring the SPI for full-duplex communication.
 *
 * @return  none
 */
void SPI_FullDuplex_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    SPI_InitTypeDef  SPI_InitStructure = {0};
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1, ENABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    //GPIO_PinRemapConfig(AFIO_PCFR1_USART1_REMAP, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
}

void spi_DMA_Tx_Init(DMA_Channel_TypeDef *DMA_CHx, u32 ppadr, u32 memadr, u16 bufsize)
{
    DMA_InitTypeDef DMA_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_DeInit(DMA_CHx);

    DMA_InitStructure.DMA_PeripheralBaseAddr = ppadr;
    DMA_InitStructure.DMA_MemoryBaseAddr = memadr;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = bufsize;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    //DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA_CHx, &DMA_InitStructure);
    //DMA_ITConfig(DMA1_Channel3, DMA_IT_TC, ENABLE);
    /* DMA发送中断源 */
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;
    /* 抢断优先级 */
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    /* 响应优先级 */
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    /* 使能外部中断通道 */
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    /* 配置NVIC */
    //NVIC_Init(&NVIC_InitStructure);
}
uint8_t spi_rgb_trans[2]={0b100,0b110};
rgb_spi_pkg rgb_spi_pkg_gen(uint8_t v)
{
    static rgb_spi_pkg p;
    static uint8_t tmp;
    p.load=0;
    for(int i=0;i<8;++i)
    {
        p.load|=(spi_rgb_trans[(v&0x1)]<<(i*3));
        v>>=1;
    }
    tmp=p.buf[2];
    p.buf[2]=p.buf[0];
    p.buf[0]=tmp;
    return p;
}
void set_led_rgb(uint8_t i,uint8_t r,uint8_t g,uint8_t b){
    spi_tx_buf[i*3+0+SPI_RESET_OFFSET]=rgb_spi_pkg_gen(g);
    spi_tx_buf[i*3+1+SPI_RESET_OFFSET]=rgb_spi_pkg_gen(r);
    spi_tx_buf[i*3+2+SPI_RESET_OFFSET]=rgb_spi_pkg_gen(b);
}
void flush_spi_tx_seq(uint8_t status)
{
    //spi_tx_buf[0].load=0;
    //spi_tx_buf[1].load=0;
    //printf("flush rgb sequence.\r\n");
    memset(spi_tx_buf,0,sizeof(spi_tx_buf));
    for(int i=0;i<user_config.rgb_cnt;++i){
        if(user_config.led_disabled||!status)
            set_led_rgb(i, 0, 0, 0);
        else
            set_led_rgb(i, user_config.rgb_data[i].r, user_config.rgb_data[i].g, user_config.rgb_data[i].b);
        //printf("rgb %d 0x%02x 0x%02x 0x%02x\r\n",i,user_config.rgb_data[i].r,user_config.rgb_data[i].g,user_config.rgb_data[i].b);
    }
    if(!status)
        return;
#if (PCB_TYPE==PCB_TYPE_MICRO)
    set_led_rgb(0, connection_state.esp32_paired*200, connection_state.esp32_paired*200, connection_state.esp32_paired*200);
    set_led_rgb(1, (!user_config.imu_disabled)*200, (!user_config.imu_disabled)*200, (!user_config.imu_disabled)*200);
    set_led_rgb(2, (!user_config.rumble_disabled)*200, (!user_config.rumble_disabled)*200, (!user_config.rumble_disabled)*200);
    set_led_rgb(3, (force_esp32_active)*200, (force_esp32_active)*200, (force_esp32_active)*200);
#else
    set_led_rgb(4, connection_state.esp32_paired*200, connection_state.esp32_paired*200, connection_state.esp32_paired*200);
    set_led_rgb(5, (!user_config.imu_disabled)*200, (!user_config.imu_disabled)*200, (!user_config.imu_disabled)*200);
    set_led_rgb(6, (!user_config.rumble_disabled)*200, (!user_config.rumble_disabled)*200, (!user_config.rumble_disabled)*200);
    set_led_rgb(7, (force_esp32_active)*200, (force_esp32_active)*200, (force_esp32_active)*200);
#endif
    //spi_tx_buf[user_config.rgb_cnt*3].load=0;
    //spi_tx_buf[user_config.rgb_cnt*3+1].load=0;
}
void flush_rgb(uint8_t status)
{
    //SPI_Cmd(SPI1, ENABLE);
    flush_spi_tx_seq(status);
    DMA1_Channel3->MADDR = (uint32_t)spi_tx_buf;
    DMA1_Channel3->CNTR = (uint32_t)(user_config.rgb_cnt*3+SPI_RESET_OFFSET*2)*sizeof(rgb_spi_pkg);
    DMA_Cmd(DMA1_Channel3, ENABLE);
    SPI_Cmd(SPI1, ENABLE);
}
int spi_init(void)
{
    SPI_FullDuplex_Init();
    spi_DMA_Tx_Init(DMA1_Channel3, (u32)&SPI1->DATAR, (u32)(uint8_t*)spi_tx_buf, (user_config.rgb_cnt*3+SPI_RESET_OFFSET*2)*sizeof(rgb_spi_pkg));
    //printf("SPI INIT:%d size:%d\r\n",sizeof(rgb_spi_pkg),(user_config.rgb_cnt*3+SPI_RESET_OFFSET*2)*sizeof(rgb_spi_pkg));
    flush_rgb(ENABLE);
    return 0;
}
void DMA1_Channel3_IRQHandler(){
    if(DMA_GetITStatus(DMA1_IT_TC3))
    {
        DMA_ClearITPendingBit(DMA1_IT_GL3); //清除全部中断标志
        DMA_Cmd(DMA1_Channel3, DISABLE);
        SPI_Cmd(SPI1, DISABLE);
        ////printf("spi dma complete\r\n");
    }
}
