 /*
 * spi.c
 *
 *  Created on: 2025年1月29日
 *      Author: Reed
 */

#include "debug.h"
#include "string.h"
#include "conf.h"
#include "pwr.h"
#include "spi.h"
#include "hd_rumble_high_accuracy.h"
#include "imu.h"
#include "gpio_digit.h"
#include "def.h"

void DMA1_Channel3_IRQHandler(void) __attribute__((portINTR));

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
#define SPI_RESET_OFFSET (16)
rgb_spi_pkg spi_tx_buf[RGB_MAX_CNT*3+SPI_RESET_OFFSET*2];//+reset
uint8_t indi_status=0;
static uint8_t indicate_led_updated=1,indicate_led_override=0;
uint32_t rgb_slow_start_timestamp_ms;
uint8_t rgb_in_slow_start,rgb_update,rgb_reset,rgb_reset_slow;
float rgb_slow_start_div=1.0f;
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
    SPI_InitStructure.SPI_CRCPolynomial = 0;//7;
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
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    //DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA_CHx, &DMA_InitStructure);
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
void flush_spi_tx_seq(uint8_t status,uint8_t use_factor,float factor)
{
    if(factor>1.0f)
        factor=1.0f;
    if(!status){
        for(int i=0;i<config.hw.rgb_cnt;++i){
            set_led_rgb(i, 0, 0, 0);
        }
        return;
    }
    for(int i=0,j=0;j<config.hw.rgb_cnt;++i){
        if(i>=config.hw.indi_led_ofst && i<config.hw.indi_led_ofst+4)
            continue;//skip indicate
        if(!config.rgb.enable)
            set_led_rgb(i, 0, 0, 0),++j;
        else{
            if(use_factor)
                set_led_rgb(i, config.rgb.data[j].r*factor, config.rgb.data[j].g*factor, config.rgb.data[j].b*factor);
            else
                set_led_rgb(i, config.rgb.data[j].r, config.rgb.data[j].g, config.rgb.data[j].b);
            ++j;
        }
    }
}
void _force_rgb(uint8_t status){
    flush_spi_tx_seq(status,0,1);
    _flush_rgb();
}
void _flush_rgb()
{
    SPI_Cmd(SPI1, DISABLE);
    DMA_Cmd(DMA1_Channel3, DISABLE);
    DMA1_Channel3->MADDR = (uint32_t)spi_tx_buf;
    DMA1_Channel3->CNTR = (uint32_t)(config.hw.rgb_cnt*3+SPI_RESET_OFFSET*2)*sizeof(rgb_spi_pkg);
    DMA_Cmd(DMA1_Channel3, ENABLE);
    SPI_Cmd(SPI1, ENABLE);
}
void flush_rgb(){
    //flush_spi_tx_seq(status);
    //_flush_rgb(status);
    rgb_reset=1;
    indicate_led_updated=1;
}
void flush_rgb_slow(){
    rgb_reset_slow=1;
    indicate_led_updated=1;
}
void update_rgb(){
    rgb_update=1;
}
void led_pwr_ctrl(uint8_t state){
    if(state!=DISABLE)
        gpio_set(GPIO_LED_PWR, DISABLE);
    else
        gpio_set(GPIO_LED_PWR, ENABLE);
}
int rgb_init(void)
{
    uint32_t t=GPIO_LED_PWR;
    _gpio_init(&t, 1, GPIO_Mode_Out_OD);
    led_pwr_ctrl(ENABLE);
    //gpio_set(t, 1);
    memset(spi_tx_buf,0,sizeof(spi_tx_buf));
    SPI_FullDuplex_Init();
    //Delay_MS(2);
    spi_DMA_Tx_Init(DMA1_Channel3, (u32)&SPI1->DATAR, (u32)(uint8_t*)spi_tx_buf, (config.hw.rgb_cnt*3+SPI_RESET_OFFSET*2)*sizeof(rgb_spi_pkg));
    //printf("SPI INIT:%d size:%d\r\n",sizeof(rgb_spi_pkg),(user_config.rgb_cnt*3+SPI_RESET_OFFSET*2)*sizeof(rgb_spi_pkg));
    //flush_rgb();
    flush_rgb_slow();
    return 0;
}
void set_indicate_led_function(){
    uint8_t ofst=config.hw.indi_led_ofst;//indi_led_brightness
    set_led_rgb(ofst+0, connection_state.esp32_paired*config.rgb.indi_led_brightness, connection_state.esp32_paired*config.rgb.indi_led_brightness, connection_state.esp32_paired*config.rgb.indi_led_brightness);
    set_led_rgb(ofst+1, (config.imu.enable)*config.rgb.indi_led_brightness, (config.imu.enable)*config.rgb.indi_led_brightness, (config.imu.enable)*config.rgb.indi_led_brightness);
    //set_led_rgb(ofst+2, (!user_config.rumble_disabled)*config.rgb.indi_led_brightness, (!user_config.rumble_disabled)*config.rgb.indi_led_brightness, (!user_config.rumble_disabled)*config.rgb.indi_led_brightness);
    set_led_rgb(ofst+3, (force_esp32_active)*config.rgb.indi_led_brightness, (force_esp32_active)*config.rgb.indi_led_brightness, (force_esp32_active)*config.rgb.indi_led_brightness);
    if(!config.rumble.enable){
        set_led_rgb(ofst+2,0,0,0);
    }else{
        if(config.rumble.hd.legacy)
            set_led_rgb(ofst+2,0,0,config.rgb.indi_led_brightness);
        else
            set_led_rgb(ofst+2,0,config.rgb.indi_led_brightness,0);
    }
}
uint8_t bat_warn;
void set_indicate_led_bat_warn()
{
    uint8_t ofst=config.hw.indi_led_ofst;
    if(bat_warn){
        set_led_rgb(ofst+0, config.rgb.indi_led_brightness, 0, 0);
        set_led_rgb(ofst+1, config.rgb.indi_led_brightness, 0, 0);
        set_led_rgb(ofst+2, config.rgb.indi_led_brightness, 0, 0);
        set_led_rgb(ofst+3, config.rgb.indi_led_brightness, 0, 0);
    }else{
        set_led_rgb(ofst+0, 0, 0, 0);
        set_led_rgb(ofst+1, 0, 0, 0);
        set_led_rgb(ofst+2, 0, 0, 0);
        set_led_rgb(ofst+3, 0, 0, 0);
    }
}
void set_indicate_led_player(uint8_t is_on_usb){
    uint8_t ofst=config.hw.indi_led_ofst;
    uint8_t res=indi_status;
    if(is_on_usb)
        res=res|(res>>4);
    if(res&0xf){
        for(int i=0;i<4;++i){
            if((1<<i)&res){
                set_led_rgb(ofst+i,config.rgb.indi_led_brightness,0,0);
            }else {
                set_led_rgb(ofst+i,0,0,0);
            }
        }
    }else {
        set_led_rgb(ofst+0,config.rgb.indi_led_brightness,0,0);
        set_led_rgb(ofst+1,config.rgb.indi_led_brightness,0,0);
        set_led_rgb(ofst+2,config.rgb.indi_led_brightness,0,0);
        set_led_rgb(ofst+3,config.rgb.indi_led_brightness,0,0);
    }
}
void set_indicate_led_status(uint8_t status)
{
    if(indi_status==status)
        return;
    indi_status=status;
    indicate_led_updated=1;
}
void set_indicate_led_mode(uint8_t mode){
    if(mode!=indicate_led_override){
        indicate_led_override=mode;
        indicate_led_updated=1;
    }
}
void set_indicate_led(uint8_t is_on_usb){
    switch(indicate_led_override){
        case 1:
            set_indicate_led_function();
            break;
        case 2:
            set_indicate_led_bat_warn();
            break;
        case 0:
            /*fall through*/
        default:
            if(indicate_led_updated){
                set_indicate_led_player(is_on_usb);
                indicate_led_updated=0;
            }
            break;
    }
}
void set_rgb(uint8_t use_factor,float fac){
    uint8_t ofst=config.hw.indi_led_ofst;
    if(connection_state.usb_plugging)
        flush_spi_tx_seq(ENABLE,use_factor,fac);
    else if(config.rgb.allow_rgb_on_bat)
        flush_spi_tx_seq(ENABLE,use_factor,fac);
    else{
        flush_spi_tx_seq(ENABLE,1,0);
        if(config.hw.pcb_typ==PCB_TYP_PRO && config.hw.pcb_rev>=PRO_300){
            set_led_rgb(ofst+8, config.rgb.data[8].r*fac, config.rgb.data[8].g*fac, config.rgb.data[8].b*fac);
            set_led_rgb(ofst+9, config.rgb.data[9].r*fac, config.rgb.data[9].g*fac, config.rgb.data[9].b*fac);
        }
    }
}
//void rgb_task(uint8_t is_on_usb){
void rgb_task(){
    static uint32_t rgb_task_t=0;
    static uint8_t pre_led_disable=1;
    static uint8_t pre_is_pluged=1;

    uint32_t t=Get_Systick_MS();
    if(t-rgb_task_t<50)//50 ms gap
        return;
    //SPI_Cmd(SPI1, DISABLE);
    //DMA_Cmd(DMA1_Channel3, DISABLE);

    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, DISABLE);
    rgb_task_t=t;
    if(connection_state.usb_paired){
        set_indicate_led_status(connection_state.usb_indicate_led);
    }else if(connection_state.esp32_paired){
        set_indicate_led_status(connection_state.esp32_indicate_led);
    }else {
        set_indicate_led_status(0xff);
    }
    if(connection_state.usb_plugging!=pre_is_pluged){
        pre_is_pluged=connection_state.usb_plugging;
        rgb_reset_slow=1;
    }
    if(config.rgb.enable!=pre_led_disable){
        pre_led_disable=config.rgb.enable;
        rgb_reset_slow=1;
    }
    if(rgb_reset_slow){
        if(config.rgb.slow_start_period){
            rgb_in_slow_start=1;
            rgb_slow_start_timestamp_ms=t;
        }else{
            rgb_reset=1;
        }
        rgb_reset_slow=0;
    }
    if(rgb_reset||rgb_reset_slow)
        indicate_led_updated=1;
    rgb_update |= rgb_reset|indicate_led_override|rgb_in_slow_start|indicate_led_updated;
    if(rgb_update){
        if(rgb_in_slow_start){
            float f=(t-rgb_slow_start_timestamp_ms)/rgb_slow_start_div;
            //f=f*f;
            if(f>=1.0f){
                set_rgb(0,1.0f);
                rgb_in_slow_start=0;
            }else {
                set_rgb(1,f*f);
            }
        }else if(rgb_reset){
            set_rgb(0,1.0f);
        }
        set_indicate_led(connection_state.usb_paired);
        _flush_rgb();
    }
    rgb_reset=0;
    rgb_update=0;
}
void DMA1_Channel3_IRQHandler(){
    if(DMA_GetITStatus(DMA1_IT_TC3))
    {
        DMA_ClearITPendingBit(DMA1_IT_GL3); //清除全部中断标志
        DMA_Cmd(DMA1_Channel3, DISABLE);
        SPI_Cmd(SPI1, DISABLE);
    }
}
