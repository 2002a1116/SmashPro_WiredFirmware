/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2020/04/30
 * Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

/* @Note
 * Compatibility HID Example:
 * This program provides examples of the pass-through of USB-HID data and serial port
 *  data based on compatibility HID device. And the data returned by Get_Report request is
 *  the data sent by the last Set_Report request.Speed of UART1/2 is 115200bps.
 *
 * Interrupt Transfers:
 *   UART2_RX   ---> Endpoint2
 *   Endpoint1  ---> UART2_TX
 *
 *   Note that the first byte is the valid data length and the remaining bytes are
 *   the transmission data for interrupt Transfers.
 *
 * Control Transfers:
 *   Set_Report ---> UART1_TX
 *   Get_Report <--- last Set_Report packet
 *
 *  */
#include <stdlib.h>
#include <math.h>
#include "debug.h"
#include "string.h"
#include "ch32v10x_usbfs_device.h"
#include "usbd_compatibility_hid.h"
#include "gpio_adc.h"
#include "gpio_digit.h"
#include "ns_com_mux.h"
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

void Var_Init(void)
{
    uint16_t i;
    RingBuffer_Comm.LoadPtr = 0;
    RingBuffer_Comm.StopFlag = 0;
    RingBuffer_Comm.DealPtr = 0;
    RingBuffer_Comm.RemainPack = 0;
    for(i=0; i<DEF_Ring_Buffer_Max_Blks; i++)
    {
        RingBuffer_Comm.PackLen[i] = 0;
    }
}
enum ADC_CHANNEL_ID{
    ADC_CHANNEL_LJOYS_HORI=0,
    ADC_CHANNEL_LJOYS_VERT,
    ADC_CHANNEL_RJOYS_HORI,
    ADC_CHANNEL_RJOYS_VERT
};
static uint8_t top_trigger=0;
uint32_t sts_button;
void button_upd_all(){
    if(!top_trigger)
        sts_button=gpio_read_all();
    //printf("sts:%d\r\n",sts_button);
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
int32_t i32_clamp(int32_t v,int32_t min,int32_t max){
    if(v<min)return min;
    if(v>max)return max;
    return v;
}
#define SNAPBACK_DEADZONE_SQUARE (800*800)
uint32_t joystick_snapback_filter(int32_t x,int32_t y,uint8_t id){
    static uint8_t set=0;
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
#if (PCB_TYPE==PCB_TYPE_MICRO)
    x=-x;
    y=-y;
#endif
    if(user_config.joystick_snapback_filter_max_delay){
        set=0;
        int32_t len_sq=x*x+y*y;
        if(len_sq>=SNAPBACK_DEADZONE_SQUARE)
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
            int64_t a_sq_b_sq=len_sq*js_snapback_samples[id].len_sq;
            //float cos_sq=dp_sq/a_sq_b_sq;
            //cos_sq<0.8||cos_sq>(1/0.8)
            if(dp_sq<0.4f*a_sq_b_sq)
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
    x=(x*user_config.joystick_ratio[id*2])>>5;
    y=(y*user_config.joystick_ratio[id*2+1])>>5;
    //user offset wont be infulenced by ratio
    x+=user_config.joystick_offset[id].x;
    y+=user_config.joystick_offset[id].y;
    //x>>=5;
    //y>>=5;
    /*if(id){
        y+=factory_configuration.JoystickCalibrationValue.AnalogStickRightFactoryCalibrationValue.AnalogStickCalY0;
        x+=factory_configuration.JoystickCalibrationValue.AnalogStickRightFactoryCalibrationValue.AnalogStickCalX0;
    }
    else{
        y+=factory_configuration.JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue.AnalogStickCalY0;
        x+=factory_configuration.JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue.AnalogStickCalX0;
    }*/
    //(x-internal_Center)*ratio+offset-2048-->clamp(0,4095);
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
            //factory_configuration.JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue.AnalogStickCalY0);
    int32_t tmp2=(adc_debounce[ADC_CHANNEL_LJOYS_HORI]-user_calibration.internal_center[0]);
            //factory_configuration.JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue.AnalogStickCalX0);
    sts_ljoy=joystick_snapback_filter(tmp2, tmp1, 0);
    //data->ljoy_status=((uint16_t)(tmp1)<<12)+tmp2;
    tmp1=(adc_debounce[ADC_CHANNEL_RJOYS_VERT]-user_calibration.internal_center[3]);
            //factory_configuration.JoystickCalibrationValue.AnalogStickRightFactoryCalibrationValue.AnalogStickCalY0);
    tmp2=(adc_debounce[ADC_CHANNEL_RJOYS_HORI]-user_calibration.internal_center[2]);
            //factory_configuration.JoystickCalibrationValue.AnalogStickRightFactoryCalibrationValue.AnalogStickCalX0);
    sts_rjoy=joystick_snapback_filter(tmp2, tmp1, 1);
}
void get_peripheral_data_handler(peripheral_data* data){
    if(!data)return;
    //ananlog_read(adc_data,sizeof(adc_data)); //adc r now set with dma
    button_upd_all();
    //user_config.joystick_fitting_mode=0;
    data->button_status=sts_button;
    data->ljoy_status=sts_ljoy;
    data->rjoy_status=sts_rjoy;
}
/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
static uart_packet pkt;
static uint32_t input_update_tick;
#define UART_REPORT_GAP (2)
void send_input_with_uart(void)
{
    memset(&pkt,0,UART_PKG_SIZE);
    //printf("before siz:%d cap:%d full:%d\r\n",uart_tx_rb.size,uart_tx_rb.capcity,uart_tx_rb.full);
    pkt.typ=UART_PKG_INPUT_DATA;
    pkt.id=1;//button status
    pkt.load=global_input_data.button_status;
    //pkt.load=input_data.button_status;
    //printf("load:%d bts:%d arr:%d %d %d\r\n",pkt.load,input_data.button_status,pkt.arr[0],pkt.arr[1],pkt.arr[2]);
    send_uart_pkt(&pkt);
    pkt.id=2;//ljoy status
    pkt.load=global_input_data.ljoy_status;
    //pkt.load=input_data.ljoy_status;
    send_uart_pkt(&pkt);
    pkt.id=3;//rjoy status
    pkt.load=global_input_data.rjoy_status;
    //pkt.load=input_data.rjoy_status;
    send_uart_pkt(&pkt);

    //printf("after siz:%d cap:%d full:%d\r\n",uart_tx_rb.size,uart_tx_rb.capcity,uart_tx_rb.full);
}
static uint32_t top_tick=0;
//static hd_rumble_frame test_frame;
static uart_packet test_uart_pkt;
static uint32_t routine_tick,routine_cnt=0,last_routine_cnt;
void imu_read_test(){
    static uint32_t last_tick;
    static uint8_t id=0,res=0;
    if(last_tick+5>Get_Systick_MS())
        return;
    last_tick=Get_Systick_MS();
    printf("imu read start\r\n");
    i2c_write_byte(0x04, 0b00001000);
    id=0;
    res=i2c_read_byte(IMU_ID,&id);
    printf("imu id:0x%02x ,res:%d\r\n",id,res);
}
void func_switch_task(){
    static uint8_t f1=0,f2=0,f3=0,f4=0;
    static uint8_t save=0,upd=0;
    if(!top_trigger)return;
    upd=save=0;
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
            pkt.typ=UART_PKG_CONNECT_CONTROL;
            pkt.id=0;
            pkt.arr[0]=!force_esp32_active;
            send_uart_pkt(&pkt);
            upd=1;
        }
        f4=0;
    }else   f4=1;
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
                    printf("top triggered last cnt:%d\r\n",last_routine_cnt);
                    printf("esp32_connect:%d\r\n",connection_state.esp32_connected);
                    printf("UART1_Tx_Flag:%d\r\n",UART1_Tx_Flag);
                    test_uart_pkt.typ=UART_PKG_TEST_ECHO;
                    memcpy(test_uart_pkt.arr,"tst",3);
                    uint8_t res=send_uart_pkt(&test_uart_pkt);
                    printf("res:%d siz:%d cap:%d full:%d\r\n",res,uart_tx_rb.size,uart_tx_rb.capcity,uart_tx_rb.full);
                    imu_read_test();
                    //printf("imu data:%llu %llu\r\n",imu_rpt.acc_sample0.load,imu_rpt.gyo_sample0.load);
                    printf("i2c error code:%d\r\n",i2c_error_code);
                    printf("adc:%d %d\r\n",adc_data[ADC_CHANNEL_LJOYS_HORI]&0xfff,adc_data[ADC_CHANNEL_LJOYS_VERT]&0xfff);
                    printf("i2c status:%d\r\n",i2c_status);
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
        printf("ls down\r\n");
    }
}
void routine_service(void){
    //printf("routine service tick:%d %d %d\r\n",Get_Systick_MS(),SysTick->CNTL,SysTick->CMPLR);
    ++rts_cnt;
    if(!rts_cnt)
        rts_tcnt=1;
    if(routine_tick!=Get_Systick_MS())
    {
        //printf("tick:%d\r\n",routine_tick);
        //printf("cnt %d\r\n",routine_cnt);
        //if(routine_cnt<2){
        //    printf("performance warning! no routine task executed for %dms\r\n",Get_Systick_MS()-routine_tick);
        //}
        routine_tick=Get_Systick_MS();
        ++rts_tcnt;
        //last_routine_cnt=routine_cnt;
        //routine_cnt=1;
    }
    top_timer();
    //ls_test();
    push_waveform_into_buffer_task();
    imu_upd();
    func_switch_task();
    //hd_rumble_high_acc_sequence_gen_task();
    //next_rumble_frame();
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
void wake_esp32()
{
    static GPIO_InitTypeDef  GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_OD;//input pullup
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_WriteBit(GPIOB, GPIO_Pin_2, 0);
    //Delay_Us(10);
    top_init();
}
static uint8_t stop_flag;
void init_all()
{
    if(stop_flag){
        SetSysClock();
        //SystemInit();
    }
    //stop_flag=0;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    RCC->CFGR0 &= ~RCC_HPRE;
    RCC->CFGR0 &= ~RCC_PPRE2;
    RCC->CFGR0 &= ~RCC_PPRE1;
    USART_Printf_Init(115200);
    //printf("\r\n\r\nPROCON PROJECT\r\n");
    RCC_LSICmd(ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    printf("\r\n\r\nPROCON PROJECT\r\n");
    SysTick_Init();
    Delay_Ms(500);
    RCC_ClocksTypeDef rcc_clock;
    RCC_GetClocksFreq(&rcc_clock);
    printf("RCC_GetClocksFreq:%d %d %d %d\r\n",
            rcc_clock.SYSCLK_Frequency,rcc_clock.HCLK_Frequency,rcc_clock.PCLK1_Frequency,rcc_clock.PCLK2_Frequency);
    HighPrecisionTimer_Init();
    memset(&connection_state,0,sizeof(connection_state));
    Var_Init();
    gpio_init();
    conf_init();
    UART1_Init();
    UART1_DMA_Init();//why

    adc_init();
    USBFS_RCC_Init();
    wake_esp32();
    routine_service_init();
    spi_init();
    i2c_init();
    imu_init();
    set_imu_awake();
    uint8_t tmp=0;
    i2c_read_byte(0x12, &tmp);
    printf("init all %d\r\n",tmp);
}
void recv_esp32_connect_control()
{
    switch(pkt.id){
    case 0:
        memcpy(connection_state.bd_addr,pkt.arr,UART_PKG_ARR_LENGTH);
        break;
    case 1:
        memcpy(connection_state.bd_addr+UART_PKG_ARR_LENGTH,pkt.arr,UART_PKG_ARR_LENGTH);
        connection_state.bd_addr_set=1;
        for(int i=0;i<BD_ADDR_LEN;++i){
            if(connection_state.bd_addr[i]!=user_config.bd_addr[i]){
                memcpy(user_config.bd_addr,connection_state.bd_addr,BD_ADDR_LEN);
                custom_conf_write();
            }
        }
        break;
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        pkt.load^=0xAAAAAA;
        memcpy(connection_state.bt_ltk+((pkt.id-2)*UART_PKG_ARR_LENGTH),pkt.arr,UART_PKG_ARR_LENGTH);
        break;
    case 7:
        connection_state.bt_ltk[15]=pkt.arr[0]^0xAA;
        connection_state.bt_ltk_get=1;
        break;
    case 0xC:
        connection_state.esp32_paired=pkt.arr[0];
        //printf("esp32_paired:%d\r\n",connection_state.esp32_paired);
        break;
    /*case 0xD:
        connection_state.esp32_sleep=1;
        break;*/
    case 0xE:
        connection_state.con_addr_set=0;
        break;
    case 0xF:
        connection_state.esp32_bt_state=pkt.arr[0];
        break;
    default:
        break;
    }
}
//static hd_rumble_frame uart_frame;
static uint8_t uart_rumble_buf[8];
void recv_rumble_frame(){
    static uint8_t sts=0;
    switch(pkt.id)
    {
    case 0:
        sts=1;
        memcpy(uart_rumble_buf,pkt.arr,3);
        break;
    case 1:
        if(sts!=1){//drop
            sts=0;
            break;
        }
        sts=2;
        memcpy(uart_rumble_buf+3,pkt.arr,3);
        break;
    case 2:
        if(sts!=2){//drop
            sts=0;
            break;
        }
        sts=0;
        memcpy(uart_rumble_buf+6,pkt.arr,2);
        /*printf("rumble frame:0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\r\n",
                uart_rumble_buf[0],uart_rumble_buf[1],uart_rumble_buf[2],uart_rumble_buf[3],
                uart_rumble_buf[4],uart_rumble_buf[5],uart_rumble_buf[6],uart_rumble_buf[7]);*/
        decode_hd_rumble_multiformat_high_acc((hd_rumble_multiformat*)uart_rumble_buf, (hd_rumble_multiformat*)(uart_rumble_buf+4));
        break;
    default:
        break;
    }
}
void recv_pwr_control(){
    switch(pkt.id){
    case 0x00://force_esp32_active
        force_esp32_active=pkt.arr[0];
        flush_rgb(ENABLE);
        break;
    case 0x01:
        connection_state.esp32_sleep=1;
        break;
    case 0x02:
        flush_rgb(DISABLE);
        Delay_Ms(10);
        NVIC_SystemReset();
        break;
    default:
        break;
    }
}
void recv_esp32_pkg()
{
    switch(pkt.typ)
    {
    case UART_PKG_INPUT_REQ:
        //printf("recv uart hb\r\n");
        break;
    case UART_PKG_CONNECT_CONTROL:
        recv_esp32_connect_control();
        break;
    case UART_PKG_RUMBLE_FRAME:
        recv_rumble_frame();
    case UART_PKG_PWR_CONTROL:
        recv_pwr_control();
    default:
        break;
    }
}
void connection_state_handler()//decide if we go stop
{
    if(!connection_state.usb_enumed && connection_state.esp32_sleep)
    {
        set_peripherals_state(DISABLE);
        set_imu_sleep();
        stop_flag=set_pwr_mode_stop();
        /*if(stop_flag)//fk it,i have no idea what are needed to reset,so lets restart the mcu as its not that slow
            NVIC_SystemReset();*/
        //fail safe,try to reset everything
        init_all();
        set_peripherals_state(ENABLE);
        wake_esp32();
    }
    else if(connection_state.esp32_connected&&!connection_state.esp32_sleep&&(input_update_tick+UART_REPORT_GAP<=Get_Systick_MS()))
    {
        input_update_tick=Get_Systick_MS();
        if(connection_state.con_addr_set)//if esp32 recved,we set this flag to be zero
        {
            pkt.typ=UART_PKG_CONNECT_CONTROL;
            pkt.id=3;
            memcpy(pkt.arr,connection_state.con_addr,UART_PKG_ARR_LENGTH);
            send_uart_pkt(&pkt);
            pkt.id=4;
            memcpy(pkt.arr,connection_state.con_addr+UART_PKG_ARR_LENGTH,UART_PKG_ARR_LENGTH);
            send_uart_pkt(&pkt);
        }
        if(!connection_state.usb_paired)
        {
            send_input_with_uart();
            //printf("send bt start\r\n");
            if(!connection_state.bd_addr_set)
            {
                printf("send get bd addr cmd\r\n");
                pkt.typ=UART_PKG_CONNECT_CONTROL;
                pkt.id=1;
                //pkt.arr[0]=1;
                send_uart_pkt(&pkt);
            }
            if(!connection_state.bt_ltk_get)
            {
                printf("send get     bt ltk cmd\r\n");
                pkt.typ=UART_PKG_CONNECT_CONTROL;
                pkt.id=2;
                //pkt.arr[0]=1;
                send_uart_pkt(&pkt);
            }
            if(!(connection_state.esp32_bt_state&0x1))
            {
                //printf("send bt start cmd\r\n");
                pkt.typ=UART_PKG_CONNECT_CONTROL;
                pkt.id=0;
                pkt.arr[0]=1;
                send_uart_pkt(&pkt);
            }
        }
        else{//if usb paired,donot report to esp32 as esp32 doesnt need input data now
            if((connection_state.esp32_bt_state&0x1))
            {
                //printf("send bt stop cmd\r\n");
                pkt.typ=UART_PKG_CONNECT_CONTROL;
                pkt.id=0;
                pkt.arr[0]=0;
                send_uart_pkt(&pkt);
            }
        }

    }
}
//code size cant be larger than 57,255byte,then we can have 4*128byte to save config
//we really used up this chip dont we.
//no its just flash addr starter not correct value,it start at 57kb,so we actually still have 6kb
//reset start to 60k now.
int main(void)
{
    init_all();
    //printf("\r\n\r\nUSBFS Compatibility HID Example\r\n");
    printf("SystemClk:%d\r\n",SystemCoreClock);

    //flash_test();
    //conf_init();

    is_rumble_start=gpio_read(NS_BUTTON_HOME);
    //is_rumble_start=1;

    ns_mux_init();
    ns_set_peripheral_data_getter(get_peripheral_data_handler);


    /* Usb init */
    USBFS_RCC_Init();
    USBFS_Device_Init( ENABLE , PWR_VDD_SupplyVoltage());
    hid_init();
    
    /* Timer init */
    //TIM2_Init();
    hd_rumble_init(!is_rumble_start);
    //spi_init();
    //hd_rumble_init(1,0);
    //uint8_t send_input=0;
    printf("init fin\r\n");
    //while(1);
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
        //continue;
        //printf("enum %d\r\n",USBFS_DevEnumStatus);
        if(USBFS_DevEnumStatus){//usb
            connection_state.usb_enumed=1;
            //printf("enum %d\r\n",USBFS_DevEnumStatus);
            hid_rx_service();
            hid_tx_service();
        }
        else{
            connection_state.usb_enumed=connection_state.usb_paired=0;
        }
#ifdef COMPILE_WL
        while(uart_rx_rb.size)
        {
            connection_state.esp32_connected=0x01;//if revice uart pkt from esp32,means it exist
            memcpy(&pkt,&uart_rx_rb_buf[uart_rx_rb.top*UART_PKG_SIZE],UART_PKG_SIZE);
            ring_buffer_pop(&uart_rx_rb);
            recv_esp32_pkg();
        }
        connection_state_handler();
        UART1_Tx_Service();
#endif
    }
}

