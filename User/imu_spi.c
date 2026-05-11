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
#define IMU_SPI_START (GPIO_WriteBit(GPIOA,GPIO_Pin_8,0),SPI_Cmd(SPI2, ENABLE))
#define IMU_SPI_END (SPI_Cmd(SPI2, DISABLE),GPIO_WriteBit(GPIOA,GPIO_Pin_8,1))
void imu_spi_init(){

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    SPI_InitTypeDef  SPI_InitStructure = {0};
    //RCC_APB2PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    IMU_SPI_END;

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI2, &SPI_InitStructure);
    //RCC_AHBPeriphClockCmd(1<<6, DISABLE);//no crc
    //SPI_Cmd(SPI2, ENABLE);
}
uint8_t spi2_flag_reset(uint16_t flag){
    return !SPI_I2S_GetFlagStatus(SPI2, flag);
}
uint8_t spi2_flag_set(uint16_t flag){
    return SPI_I2S_GetFlagStatus(SPI2, flag);
}
uint8_t imu_spi_wait(){
    if(execute_timeout_us((EXECUTE_FUNC_TYP)spi2_flag_set,(void*)SPI_I2S_FLAG_TXE,SPI2_TX_TIMEOUT_US))
        return 1;
    return 0;
}
uint8_t _spi_write(uint8_t value){
    if(execute_timeout_us((EXECUTE_FUNC_TYP)spi2_flag_set,(void*)SPI_I2S_FLAG_TXE,SPI2_TX_TIMEOUT_US))
        return 1;
    SPI2->DATAR=value;
    //SPI_I2S_SendData(SPI2, value);
    return 0;
}
uint8_t _spi_read(uint8_t* value){
    if(execute_timeout_us((EXECUTE_FUNC_TYP)spi2_flag_set,(void*)SPI_I2S_FLAG_RXNE,SPI2_RX_TIMEOUT_US))
        return 1;
    //uint8_t tmp = SPI2->STATR;
    if(value)
        *value=SPI2->DATAR;
    else
        (volatile uint16_t)(SPI2->DATAR);
    //optimize notice.reed
    //*value=SPI_I2S_ReceiveData(SPI2);
    return 0;
}
uint8_t spi_write(uint8_t value){
    uint8_t ret = _spi_write(value);
    if(ret)return ret;
    ret = _spi_read(NULL);//read dummy
    return ret;
}
uint8_t spi_read(uint8_t *value){
    uint8_t ret = _spi_write(0);//write dummy
    if(ret)return ret;
    ret = _spi_read(value);
    return ret;
}
void spi_clear(){
    SPI2->DATAR=0;
}
uint8_t spi_get_reg(uint8_t addr,uint8_t* value){
    IMU_SPI_START;
    uint8_t ret=0;
    do{
        ret = spi_write(addr|0x80);//write addr
        if(ret)break;
        ret = spi_read(value);
    }while(0);
    ret = imu_spi_wait();
    IMU_SPI_END;
    return ret;
}
uint8_t spi_get_multi_reg(uint8_t addr,uint8_t* value,uint8_t size){
    IMU_SPI_START;
    uint8_t ret=0;
    ret = spi_write(addr|0x80);
    if(ret)goto spi_get_multi_reg_clr;
    while(size--){
        ret = spi_read(value++);
        if(ret)goto spi_get_multi_reg_clr;
    }
spi_get_multi_reg_clr:
    ret = imu_spi_wait();
    IMU_SPI_END;
    return ret;
}
uint8_t spi_set_reg(uint8_t addr,uint8_t value){
    IMU_SPI_START;
    uint8_t ret=0;
    do{
        ret = spi_write(addr);
        if(ret)break;
        ret = spi_write(value);
    }while(0);
    ret = imu_spi_wait();
    IMU_SPI_END;
    return ret;
}
uint8_t spi_set_multi_reg(uint8_t addr,uint8_t* value,uint8_t size){
    IMU_SPI_START;
    uint8_t ret=0;
    ret = spi_write(addr);
    if(ret)return ret;
    while(size--){
        ret = spi_write(*value);
        ++value;
        if(ret)break;
    }
    ret = imu_spi_wait();
    IMU_SPI_END;
    return ret;
}
void imu_set(uint32_t addr,uint32_t value){
}
