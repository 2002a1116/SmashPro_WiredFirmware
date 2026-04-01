/*
 * imu_spi.c
 *
 *  Created on: 2026Äê3ÔÂ11ÈÕ
 *      Author: Reed
 */

#include "debug.h"
#include "conf.h"
#include "imu_spi.h"
#include "tick.h"

void imu_spi_init(){

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    SPI_InitTypeDef  SPI_InitStructure = {0};
    //RCC_APB2PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI2, &SPI_InitStructure);
    SPI_Cmd(SPI2, ENABLE);
}
uint8_t spi2_check_flag(uint16_t flag){
    return !SPI_I2S_GetFlagStatus(SPI2, flag);
}
uint8_t spi_write(uint8_t value){
    if(execute_timeout_us((EXECUTE_FUNC_TYP)spi2_check_flag,(void*)SPI_I2S_FLAG_TXE,SPI2_TX_TIMEOUT_US))
        return 1;
    SPI2->DATAR=value;
    //SPI_I2S_SendData(SPI2, value);
    return 0;
}
uint8_t spi_read(uint8_t* value){
    if(execute_timeout_us((EXECUTE_FUNC_TYP)spi2_check_flag,(void*)SPI_I2S_FLAG_RXNE,SPI2_RX_TIMEOUT_US))
        return 1;
    *value=SPI2->DATAR;
    //SPI_I2S_ReceiveData(SPI2, value);
    return 0;
}
void spi_clear(){
    SPI2->DATAR=0;
}
uint8_t spi_get_reg(uint8_t addr,uint8_t* value){
    uint8_t ret=0;
    ret = spi_write(addr);
    if(ret)return ret;
    ret = spi_read(value);
    return ret;
}
uint8_t spi_get_multi_reg(uint8_t addr,uint8_t* value,uint8_t size){
    uint8_t ret=0;
    ret = spi_write(addr);
    if(ret)return ret;
    while(size--){
        ret = spi_read(value++);
        if(ret)break;
    }
    return ret;
}
uint8_t spi_set_reg(uint8_t addr,uint8_t value){
    uint8_t ret=0;
    ret = spi_write(addr);
    if(ret)return ret;
    ret = spi_write(value);
    return ret;
}
uint8_t spi_set_multi_reg(uint8_t addr,uint8_t* value,uint8_t size){
    uint8_t ret=0;
    ret = spi_write(addr);
    if(ret)return ret;
    while(size--){
        ret = spi_write(*value);
        ++value;
        if(ret)break;
    }
    return ret;
}
void imu_set(uint32_t addr,uint32_t value){
}
