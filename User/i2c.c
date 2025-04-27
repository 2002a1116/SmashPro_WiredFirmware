/*
 * i2c.c
 *
 *  Created on: 2024Äê11ÔÂ22ÈÕ
 *      Author: Reed
 */

#include "debug.h"
#include "i2c.h"
#include "conf.h"
#include "watchdog.h"
void I2C2_ER_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
/* I2C Mode Definition */
uint8_t i2c_status;
uint8_t i2c_error_code;
uint8_t i2c_intr_error;
uint8_t i2c2_check_error(){
    //it seems like just timeout is faster
    if(I2C2->STAR1&0x1f00){
        i2c_intr_error=(I2C2->STAR1&0x1f00)>>8;
        I2C2->STAR1&=0xe0FF;
        return i2c_intr_error;
    }
    return 0;
}
uint8_t i2c2_check_flag(uint32_t flag){
    //return i2c2_check_error()||(I2C_GetFlagStatus( I2C2, flag ) == RESET);
    return !I2C_GetFlagStatus( I2C2, flag );
}
uint8_t i2c2_check_event(uint32_t event){
    //return i2c2_check_error()||I2C_CheckEvent(I2C2, event);
    return I2C_CheckEvent(I2C2, event);
}
/* Global define */
/*********************************************************************
 * @fn      IIC_Init
 *
 * @brief   Initializes the IIC peripheral.
 *
 * @return  none
 */
void I2C2_ER_IRQHandler(void){
    i2c_intr_error=(I2C2->STAR1)>>8;
    I2C2->STAR1&=~0x00FF;//we clear it bit
}
void i2c_enable_error_interupt(){
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    i2c_intr_error=0;
    I2C2->CTLR2|=0x100;//ITERREN
    NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init( &NVIC_InitStructure );
}
void i2c_hardware_init(u32 baudrate, u16 address, u8 allow_reset)
{
    //printf("i2c init\r\n");
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    I2C_InitTypeDef  I2C_InitTSturcture = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    //GPIO_PinRemapConfig(GPIO_Remap_I2C2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

    //printf("i2c init rate:%d\r\n",baudrate);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);


    I2C_InitTSturcture.I2C_ClockSpeed = baudrate;
    I2C_InitTSturcture.I2C_Mode = I2C_Mode_I2C;
    I2C_InitTSturcture.I2C_DutyCycle = I2C_DutyCycle_16_9;
    I2C_InitTSturcture.I2C_OwnAddress1 = address;
    I2C_InitTSturcture.I2C_Ack = I2C_Ack_Enable;
    I2C_InitTSturcture.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C2, &I2C_InitTSturcture);

    //i2c_enable_error_interupt();//we check and clear internal error register in the await task

    I2C_Cmd(I2C2, ENABLE);
    //I2C_GenerateSTOP( I2C2, ENABLE );
    //Delay_Us(10);
    //printf("i2c busy flag:%d\r\n",I2C_GetFlagStatus(I2C2, I2C_FLAG_BUSY));
    if(wait_nonblocking_us(IMU_I2C_EXECUTE_FUNC(i2c2_check_flag),(void*)I2C_FLAG_BUSY,IMU_I2C_TIMEOUT_US)){
        //printf("i2c_init fail %d\r\n",allow_reset);
        if(allow_reset){
            I2C_SoftwareResetCmd(I2C2, ENABLE);
            i2c2_reset();
        }
        else{
            //printf("i2c2 init fail,retry reset not allowed\r\n");
            i2c_status=0;
            return;
        }
        //I2C_SoftwareResetCmd(I2C2, ENABLE);
        //I2C_Cmd(I2C2, ENABLE);
    }
    //I2C_GenerateSTOP( I2C2, ENABLE );
    i2c_status=1;
}
#define SCL_HIGH (GPIO_SetBits(GPIOB, GPIO_Pin_10))
#define SCL_LOW (GPIO_ResetBits(GPIOB, GPIO_Pin_10))
#define SDA_HIGH (GPIO_SetBits(GPIOB, GPIO_Pin_11))
#define SDA_LOW (GPIO_ResetBits(GPIOB, GPIO_Pin_11))
#define I2C_DELAY (Delay_Us(1))
uint8_t i2c_hardware_recovery()
{
    I2C2->CTLR1&=~0x1;
    I2C2->CTLR1|=0x1;
    I2C_SoftwareResetCmd(I2C2, ENABLE);
}
uint8_t i2c_software_recovery()
{
    I2C_Cmd(I2C2, DISABLE);
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /*SCL_LOW;
    I2C_DELAY;
    SCL_HIGH;
    I2C_DELAY;
    SDA_HIGH;//END
    I2C_DELAY;*/
    SCL_LOW;
    SDA_HIGH;
    I2C_DELAY;

    SCL_HIGH;
    I2C_DELAY;
    SDA_LOW;//START
    I2C_DELAY;

    for(int i=0;i<10;++i){
        SCL_LOW;
        SDA_HIGH;
        I2C_DELAY;
        SCL_HIGH;
        I2C_DELAY;
    }//9 cycle with sda high
    SCL_LOW;
    SDA_LOW;
    I2C_DELAY;

    SCL_HIGH;
    I2C_DELAY;
    SDA_HIGH;//END
    I2C_DELAY;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    return GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11);
}
void i2c2_reset()
{
    static uint32_t last_reset=0;
    //printf("reset gap:%d\r\n",Get_Systick_US()-last_reset);

    //i2c_hardware_recovery();
    //return;

    i2c_status=0;
    I2C_Cmd(I2C2, DISABLE);
    I2C_DeInit(I2C2);
    //printf("i2c reset start busy flag:%d ecode:%d\r\n",I2C_GetFlagStatus(I2C2, I2C_FLAG_BUSY),i2c_error_code);
    uint8_t res;
    res=i2c_software_recovery();
    //I2C_Cmd(I2C2, ENABLE);
    //res&=!wait_nonblocking_us(IMU_I2C_EXECUTE_FUNC(i2c2_check_flag),(void*)I2C_FLAG_BUSY,IMU_I2C_TIMEOUT_US);
    if(res|i2c_intr_error){
        //printf("i2c reset success\r\n");
        //if(i2c_error_code==1||i2c_error_code==10)
        //    I2C_SoftwareResetCmd(I2C2, ENABLE);
        i2c_hardware_init(IMU_I2C_FREQ, CH32V_I2C_ADDR,0);
        //printf("i2c reset busy flag:%d\r\n",I2C_GetFlagStatus(I2C2, I2C_FLAG_BUSY));
        //Delay_Us(10);
        //printf("i2c sleep %d\r\n",wait_nonblocking_us(IMU_I2C_EXECUTE_FUNC(i2c2_check_flag),(void*)I2C_FLAG_BUSY,IMU_I2C_TIMEOUT_US));
    }else {
        //printf("i2c restart fail,disable i2c\r\n");
    }
    last_reset=Get_Systick_US();
}
uint8_t i2c_read_start(uint8_t addr,uint8_t ack_mode)
{
    ////printf("i2c_read_start status:%d\r\n",i2c_status);
    if(!i2c_status)return 1;
    uint8_t res=0;
    do{
        //I2C_GenerateSTOP( I2C2, ENABLE );
        ////printf("i2c_read_byte 1\r\n");
        I2C_AcknowledgeConfig(I2C2, ENABLE);
        res=wait_nonblocking_us(IMU_I2C_EXECUTE_FUNC(i2c2_check_flag),(void*)I2C_FLAG_BUSY,IMU_I2C_TIMEOUT_US);
        if(res|i2c_intr_error){
            i2c_error_code=1;
            break;
        }
        //while( I2C_GetFlagStatus( I2C2, I2C_FLAG_BUSY ) != RESET );
        I2C_GenerateSTART( I2C2, ENABLE );
        //while( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_MODE_SELECT ));
        res=wait_nonblocking_us(IMU_I2C_EXECUTE_FUNC(i2c2_check_event),(void*)I2C_EVENT_MASTER_MODE_SELECT,IMU_I2C_TIMEOUT_US);
        if(res|i2c_intr_error){
            i2c_error_code=2;
            break;
        }
        ////printf("HighPrecisionTimerStart\r\n");
        I2C_Send7bitAddress(I2C2, IMU_ADDR, I2C_Direction_Transmitter);
        //HighPrecisionTimerStart();
        //while( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ));
        res=wait_nonblocking_us(IMU_I2C_EXECUTE_FUNC(i2c2_check_event),(void*)I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED,IMU_I2C_TIMEOUT_US);
        ////printf("time spend:%d\r\n",HighPrecisionTimerUs());
        if(res|i2c_intr_error){
            i2c_error_code=3;
            break;
        }
        I2C_SendData(I2C2, addr);
        //while( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
        res=wait_nonblocking_us(IMU_I2C_EXECUTE_FUNC(i2c2_check_event),(void*)I2C_EVENT_MASTER_BYTE_TRANSMITTED,IMU_I2C_TIMEOUT_US);
        if(res|i2c_intr_error){
            i2c_error_code=4;
            break;
        }
        I2C_AcknowledgeConfig(I2C2, ack_mode);
        I2C_GenerateSTART( I2C2, ENABLE );
        res=wait_nonblocking_us(IMU_I2C_EXECUTE_FUNC(i2c2_check_event),(void*)I2C_EVENT_MASTER_MODE_SELECT,IMU_I2C_TIMEOUT_US);
        if(res|i2c_intr_error){
            i2c_error_code=5;
            break;
        }
        I2C_Send7bitAddress(I2C2, IMU_ADDR, I2C_Direction_Receiver);
        res=wait_nonblocking_us(IMU_I2C_EXECUTE_FUNC(i2c2_check_event),(void*)I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED,IMU_I2C_TIMEOUT_US);
        if(res|i2c_intr_error){
            i2c_error_code=6;
            break;
        }
    }while(0);
    return res;
}
uint8_t i2c_read_continuous(uint8_t addr,uint8_t* buf,uint8_t len)
{
    if(len==1)return i2c_read_byte(addr, buf);
    //if(len==1)return -1;
    uint8_t res=0,tl=len;
    res=i2c_read_start(addr,ENABLE);
    if(res){
        i2c2_reset();
        return res;
    }
    while(len--){
        //res=wait_nonblocking_us(IMU_I2C_EXECUTE_FUNC(i2c2_check_event),(void*)I2C_FLAG_RXNE,IMU_I2C_TIMEOUT_US);
        res=wait_nonblocking_us(IMU_I2C_EXECUTE_FUNC(i2c2_check_event),(void*)I2C_EVENT_MASTER_BYTE_RECEIVED,IMU_I2C_TIMEOUT_US);
        if(res|i2c_intr_error){
            //printf("errorr len multi byte tot:%d len:%d\r\n",tl,len);
            i2c_error_code=7;
            break;
        }
        if(len==1){
            I2C_NACKPositionConfig(I2C2, I2C_NACKPosition_Next);
            //I2C_AcknowledgeConfig(I2C2, DISABLE);
        }
        *buf=I2C_ReceiveData(I2C2);
        ++buf;
    };
    I2C_GenerateSTOP( I2C2, ENABLE );
    if(res|i2c_intr_error)
        i2c2_reset();
    return res;
}
uint8_t i2c_read_byte(uint32_t addr,uint8_t *ret)
{
    static uint8_t res=0;
    do{
        res=i2c_read_start(addr,DISABLE);
        if(res|i2c_intr_error){
            break;
        }
        res=wait_nonblocking_us(IMU_I2C_EXECUTE_FUNC(i2c2_check_event),(void*)I2C_EVENT_MASTER_BYTE_RECEIVED,IMU_I2C_TIMEOUT_US);
        if(res|i2c_intr_error){
            i2c_error_code=9;
            break;
        }
        *ret=I2C_ReceiveData(I2C2);
        I2C_GenerateSTOP( I2C2, ENABLE );
    }while(0);
    if(res|i2c_intr_error)
        i2c2_reset();
    user_calibration.tag2=res;
    return res;
}
uint8_t i2c_write_byte(uint8_t reg,uint8_t v)
{
    if(!i2c_status)return;
    uint8_t res=0;
    do{
        res=wait_nonblocking_us(IMU_I2C_EXECUTE_FUNC(i2c2_check_flag),(void*)I2C_FLAG_BUSY,IMU_I2C_TIMEOUT_US);
        if(res|i2c_intr_error){
            i2c_error_code=10;
            break;
        }
        I2C_GenerateSTART( I2C2, ENABLE );
        res=wait_nonblocking_us(IMU_I2C_EXECUTE_FUNC(i2c2_check_event),(void*)I2C_EVENT_MASTER_MODE_SELECT,IMU_I2C_TIMEOUT_US);
        if(res|i2c_intr_error){
            i2c_error_code=11;
            break;
        }
        I2C_Send7bitAddress(I2C2, IMU_ADDR, I2C_Direction_Transmitter);
        res=wait_nonblocking_us(IMU_I2C_EXECUTE_FUNC(i2c2_check_event),(void*)I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED,IMU_I2C_TIMEOUT_US);
        if(res|i2c_intr_error){
            i2c_error_code=12;
            break;
        }
        I2C_SendData(I2C2, reg);
        res=wait_nonblocking_us(IMU_I2C_EXECUTE_FUNC(i2c2_check_event),(void*)I2C_EVENT_MASTER_BYTE_TRANSMITTED,IMU_I2C_TIMEOUT_US);
        if(res|i2c_intr_error){
            i2c_error_code=13;
            break;
        }
        I2C_SendData(I2C2, v);
        res=wait_nonblocking_us(IMU_I2C_EXECUTE_FUNC(i2c2_check_event),(void*)I2C_EVENT_MASTER_BYTE_TRANSMITTED,IMU_I2C_TIMEOUT_US);
        if(res|i2c_intr_error){
            i2c_error_code=14;
            break;
        }
        I2C_GenerateSTOP( I2C2, ENABLE );
    }while(0);
    if(res|i2c_intr_error)
        i2c2_reset();
    return res;
}

void i2c_init()
{
    i2c_hardware_init(IMU_I2C_FREQ, CH32V_I2C_ADDR,1);
}
