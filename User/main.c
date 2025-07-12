#include <stdlib.h>
#include <math.h>
#include "debug.h"
#include "string.h"
#include "global_api.h"
#include "ch32v10x_usbfs_device.h"
#include "usbd_compatibility_hid.h"
#include "gpio_adc.h"
#include "gpio_digit.h"
#include "ns_com_mux.h"
#include "uart.h"
#include "uart_com.h"
#include "tick.h"
#include "conf.h"
#include "flash.h"
#include "hd_rumble.h"
#include "hd_rumble2.h"
#include "hd_rumble_high_accuracy.h"
#include "board_type.h"
#include "i2c.h"
#include "imu.h"
#include "pwr.h"
#include "spi.h"
#include "watchdog.h"

void Var_Init(void)
{

}
enum ADC_CHANNEL_ID{
    ADC_CHANNEL_LJOYS_HORI=0,
    ADC_CHANNEL_LJOYS_VERT,
    ADC_CHANNEL_RJOYS_HORI,
    ADC_CHANNEL_RJOYS_VERT
};
static uint8_t top_trigger=0;
void button_upd_all(){
    if(!top_trigger)
        sts_button=gpio_read_all();
    else
        sts_button=0;
}
int32_t adc_debounce[4];
typedef struct{
    uint32_t sample_timestamp;
    int32_t x;
    int32_t y;
    int64_t len_sq;
}js_sample;
js_sample js_snapback_samples[2];
//dif=0.8
uint32_t joystick_snapback_filter(int32_t x,int32_t y,uint8_t id){
    static uint8_t set=0;
    //deadzone is fixed
    switch(user_config.dead_zone_mode)
    {
    case 1:
        if((abs(x)<user_config.dead_zone[id<<1])&&(abs(y)<user_config.dead_zone[1+(id<<1)]))
            x=y=0;
        break;
    case 2:
        if(abs(x)<user_config.dead_zone[id<<1]) x=0;
        if(abs(y)<user_config.dead_zone[1+(id<<1)]) y=0;
        break;
    case 3:
        if(abs(x)<user_config.dead_zone[id<<1]) x=0;
        else x+=(x>0?-1:1)*user_config.dead_zone[id<<1];
        if(abs(y)<user_config.dead_zone[1+(id<<1)]) y=0;
        else y+=(y>0?-1:1)*user_config.dead_zone[1+(id<<1)];
        break;
    default:
        break;
    }
    //snapback deadzone is influenced by joystick ratio,as its value is set by output x & y
    x=(x*user_config.joystick_ratio[id*2])>>5;
    y=(y*user_config.joystick_ratio[id*2+1])>>5;
/*#if (PCB_TYPE==PCB_TYPE_MICRO)
    x=-x;
    y=-y;
#endif*/
    if(user_config.joystick_snapback_filter_max_delay){
        set=0;
        int32_t len_sq=x*x+y*y;
        if(len_sq>=joystick_snapback_deadzone_sq[id])
            set=1;
        uint32_t timestamp=Get_Systick_US();
        if(!timestamp)++timestamp;
        do{
            //if(len_sq<160000)
            //if(js_snapback_samples[id].len_sq<160000)
            //    break;
            if(!js_snapback_samples[id].sample_timestamp)//no valid sample,skip
                break;
            if(timestamp-js_snapback_samples[id].sample_timestamp>=user_config.joystick_snapback_filter_max_delay){
                js_snapback_samples[id].sample_timestamp=0;//invalid sample
                break;
            }
            int64_t dp_sq=js_snapback_samples[id].x*x+js_snapback_samples[id].y*y;
            if(dp_sq>=0)
                break;
            dp_sq*=dp_sq;
            //int64_t a_sq_b_sq=len_sq*js_snapback_samples[id].len_sq;
            //float cos_sq=dp_sq/a_sq_b_sq;
            //cos_sq<0.8||cos_sq>(1/0.8)
            //if(dp_sq<0.375f*a_sq_b_sq)
            uint64_t a_sq_b_sq=(len_sq*js_snapback_samples[id].len_sq*3)>>3;
            if(dp_sq<a_sq_b_sq)
                break;
            //prob snapback
            //force center
            //if prob snapback,dont upd sample as it was removed
            set=0;
            x=y=0;
            //return (user_config.joystick_center[id+1]<<12)+user_config.joystick_center[id];
        }while(0);
        //reset sample
        if(set){
            js_snapback_samples[id].x=x;
            js_snapback_samples[id].y=y;
            js_snapback_samples[id].len_sq=len_sq;
            js_snapback_samples[id].sample_timestamp=timestamp;
        }
    }
    x+=2048;
    y+=2048;
    x=i32_clamp(x, 0, 4095);
    y=i32_clamp(y, 0, 4095);
    return (y<<12)+x;
}
static uint32_t sts_ljoy,sts_rjoy;
void joystick_debounce_task(){
    for(int i=0;i<4;++i){
        adc_debounce[i]=adc_data[i];
    }
    int32_t tmp1=(adc_debounce[ADC_CHANNEL_LJOYS_VERT]-user_calibration.internal_center[1]);
    int32_t tmp2=(adc_debounce[ADC_CHANNEL_LJOYS_HORI]-user_calibration.internal_center[0]);
    sts_ljoy=joystick_snapback_filter(tmp2, tmp1, 0);
    tmp1=(adc_debounce[ADC_CHANNEL_RJOYS_VERT]-user_calibration.internal_center[3]);
    tmp2=(adc_debounce[ADC_CHANNEL_RJOYS_HORI]-user_calibration.internal_center[2]);
    sts_rjoy=joystick_snapback_filter(tmp2, tmp1, 1);
}
void get_peripheral_data_handler(peripheral_data* data){
    if(!data)return;
    //ananlog_read(adc_data,sizeof(adc_data)); //adc r now set with dma
    button_upd_all();
    data->button_status=sts_button;
    data->ljoy_status=sts_ljoy;
    data->rjoy_status=sts_rjoy;
}
static uart_packet pkt;
static uint32_t top_tick=0;
static uart_packet test_uart_pkt;
static uint32_t routine_tick,routine_cnt=0,last_routine_cnt;
void imu_read_test(){
    static uint32_t last_tick;
    static uint8_t id=0,res=0;
    if(Get_Systick_MS()-last_tick>5)
        return;
    last_tick=Get_Systick_MS();
    //printf("imu read start\r\n");
    i2c_write_byte(0x04, 0b00001000);
    id=0;
    res=i2c_read_byte(IMU_ID_REG,&id);
    //printf("imu id:0x%02x ,res:%d\r\n",id,res);
}
void func_switch_task(){
    static uint8_t f1=0,f2=0,f3=0,f4=0,f5=0,f6=0,fhf=0;
    static uint8_t save=0,upd=0;
    if(!top_trigger)return;
    upd=save=0;
    if(!gpio_read(GPIO_BUTTON_A)){//cause hard fault
        if(fhf){
            //trigger_hardfault();
        }
        fhf=0;
    }else   fhf=1;
    if(!gpio_read(GPIO_BUTTON_MINUS)){//imu switch
        if(f1){
            user_config.imu_disabled=!user_config.imu_disabled;
            upd=save=1;
        }
        f1=0;
    }else   f1=1;
    if(!gpio_read(GPIO_BUTTON_PLUS)){
        if(f2){
            user_config.rumble_disabled=!user_config.rumble_disabled;
            hd_rumble_set_status(!user_config.rumble_disabled);
            upd=save=1;
        }
        f2=0;
    }else   f2=1;
    if(!gpio_read(GPIO_BUTTON_CAP)){
        if(f3){
            user_config.led_disabled=!user_config.led_disabled;
            upd=save=1;
        }
        f3=0;
    }else   f3=1;
    if(!gpio_read(GPIO_BUTTON_HOME)){
        if(f4){
            //force_esp32_active=!force_esp32_active;
            pkt.typ=UART_PKG_PWR_CONTROL;
            pkt.id=0;
            pkt.load[0]=!force_esp32_active;
            send_uart_pkt(&pkt);
            upd=1;
        }
        f4=0;
    }else   f4=1;
    if(!gpio_read(GPIO_BUTTON_LS)){
        if(f5){
            for(int i=5;i>=0&&(user_config.bd_addr[i]++)==255;--i);
            memcpy(connection_state.bd_addr,user_config.bd_addr,BD_ADDR_LEN);
            custom_conf_write();
        }
        f5=0;
    }else f5=1;
    if(!connection_state.usb_paired&&!gpio_read(GPIO_BUTTON_RS)){
        if(f6){
            pkt.typ=UART_PKG_CONNECT_CONTROL;
            pkt.id=5;
            pkt.load[0]=1;
            send_uart_pkt(&pkt);
            upd=1;
        }f6=0;
    }else f6=1;
    if(upd)
        flush_rgb(ENABLE);
    if(save)//we save to flash
        custom_conf_write();
}
void top_timer(void){
    uint8_t s=!gpio_read(GPIO_BUTTON_TOP);
    if(s)//if top pressed
    {
        if(top_tick)
        {
            if(Get_Systick_MS()-top_tick>100){
                if(!top_trigger){
                    printf("top pressed\r\n");
                }
                top_trigger=1;
            }
        }
        else {
            top_tick=Get_Systick_MS();
        }
    }
    else {
        top_trigger=0;
        top_tick=0;
    }
}
void ls_test()
{
    uint8_t res=!gpio_read(NS_BUTTON_LS);
    if(res)
    {
        //printf("ls down\r\n");
    }
}
void routine_service(void){
    ////printf("routine service tick:%d %d %d\r\n",Get_Systick_MS(),SysTick->CNTL,SysTick->CMPLR);
    ++rts_cnt;
    if(!rts_cnt)
        rts_tcnt=1;
    if(routine_tick!=Get_Systick_MS()){
        rts_tcnt+=Get_Systick_MS()-routine_tick;
        routine_tick=Get_Systick_MS();
    }
    top_timer();
    //push_waveform_into_buffer_task();
    imu_upd();
    func_switch_task();
}
void routine_service_init(void){
    top_init();
}
void set_peripherals_state(uint8_t state)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, state);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, state);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, state);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, state);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, state);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, state);
    DMA_Cmd(DMA1_Channel4, state);
    DMA_Cmd(DMA1_Channel5, state);
    DMA_Cmd(DMA1_Channel1, state);
    TIM_Cmd(TIM2, state);
    TIM_Cmd(TIM3, state);
    USART_Cmd(USART1, state);
    USART_DMACmd(USART1, USART_DMAReq_Rx, state);
}
static uint8_t stop_flag;
void init_all()
{
    //SetSysClock();//this seems not reenterable?why?
    RCC->CTLR|=RCC_CSSON;
    RCC_LSICmd(ENABLE);
    //RCC->CFGR0 &= ~RCC_HPRE;
    //RCC->CFGR0 &= ~RCC_PPRE2;
    //RCC->CFGR0 &= ~RCC_PPRE1;
    USART_printf_Init(115200);
    memset(&connection_state,0,sizeof(connection_state));
    printf("\r\n\r\nPROCON PROJECT\r\n");
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    HighPrecisionTimer_Init();
    SysTick_Init();
    Delay_Ms(10);
    conf_init();
    RCC_ClocksTypeDef rcc_clock;
    RCC_GetClocksFreq(&rcc_clock);
    Var_Init();
    gpio_init();
    UART1_Init();
    UART1_DMA_Init();
    adc_init();
    wake_esp32();
    routine_service_init();
    spi_init();
    i2c_init();
    imu_init();
    set_imu_awake();
    USBFS_RCC_Init();
    USBFS_Device_Init( ENABLE , PWR_VDD_SupplyVoltage());
    //sofw_watchdog_init();
}
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    //NVIC_HaltPushCfg(ENABLE);
    //seems every pfic(nvic) operation will influnce sysclock,and increase imu fail rate.
    //dont know why.
    //todo: figure it out.
    init_all();
    printf("SystemClk:%d\r\n",SystemCoreClock);
    is_rumble_start=gpio_read(NS_BUTTON_HOME);
    ns_mux_init();
    ns_set_peripheral_data_getter(get_peripheral_data_handler);
    hid_init();
    hd_rumble_init(!is_rumble_start);
    routine_service_init();
#define COMPILE_WL
    while(1)
    {
#ifdef COMPILE_WL
        UART1_Rx_Service();//check if uart recive anything
#endif
        joystick_debounce_task();
        get_peripheral_data_handler(&global_input_data);
        routine_service();
        if(USBFS_DevEnumStatus){//usb
            connection_state.usb_enumed=1;
            hid_rx_service();
            hid_tx_service();
        }
        else{
            connection_state.usb_enumed=connection_state.usb_paired=0;
        }
        push_waveform_into_buffer_task();
#ifdef COMPILE_WL
        uart_com_task();
        UART1_Tx_Service();
#endif
        //soft_watchdog_feed();
    }
}

