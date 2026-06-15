#include "def.h"
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
#include "i2c.h"
#include "imu.h"
#include "pwr.h"
#include "spi.h"
#include "advance_coroutine.h"
#include "watchdog.h"
#include "joystick.h"
//
static uint8_t top_trigger=0;
void button_upd_all(){
    uint32_t tp=gpio_read_all();
    tp &= button_active_mask;//ignore disabled input.
    if(!top_trigger){
        if(config.btn.a_b_swap && ((tp&(1<<NS_BUTTON_A))^(tp&(1<<NS_BUTTON_B)))){
            tp ^= (1<<NS_BUTTON_A)|(1<<NS_BUTTON_B);
        }
        if(config.btn.x_y_swap && ((tp&(1<<NS_BUTTON_X))^(tp&(1<<NS_BUTTON_Y)))){
            tp ^= (1<<NS_BUTTON_X)|(1<<NS_BUTTON_Y);
        }
        sts_button=tp;
    }
    else{
        sts_button=0;
        if(NGC_ATLEAST(NGC_110)){
            if(sts_button_raw&(1<<NS_BUTTON_Y)){
                sts_button|=(1<<NS_BUTTON_MINUS);
            }
            if(sts_button_raw&(1<<NS_BUTTON_X)){
                sts_button|=(1<<NS_BUTTON_PLUS);
            }
            if(sts_button_raw&(1<<NS_BUTTON_B)){
                sts_button|=(1<<NS_BUTTON_CAP);
            }
            if(sts_button_raw&(1<<NS_BUTTON_A)){
                sts_button|=(1<<NS_BUTTON_HOME);
            }
            if(sts_button_raw&(1<<NS_BUTTON_UP)){
                sts_button|=(1<<NS_BUTTON_L);
            }
            if(sts_button_raw&(1<<NS_BUTTON_LEFT)){
                sts_button|=(1<<NS_BUTTON_LS);
            }
            if(sts_button_raw&(1<<NS_BUTTON_RIGHT)){
                sts_button|=(1<<NS_BUTTON_RS);
            }
        }
    }
}
uint8_t button_read(uint32_t num)
{
    return (sts_button&(1<<num))!=0;
}
//dif=0.8

#define DPAD_MASK ((1<<NS_BUTTON_LEFT)|(1<<NS_BUTTON_RIGHT)|(1<<NS_BUTTON_UP)|(1<<NS_BUTTON_DOWN))
#define DEFAULT_JOYSTICK_RANGE (1800);
uint32_t get_ljs_range_on_axis(uint8_t num){
    return DEFAULT_JOYSTICK_RANGE;
}
void get_peripheral_data_handler(peripheral_data* data){
    if(!data)return;
    //ananlog_read(adc_data,sizeof(adc_data)); //adc r now set with dma
    button_upd_all();
    joystick_upd();
    uint32_t sts_ljoy=sts_joy[0].data;//js_get_value(0);
    uint32_t sts_rjoy=sts_joy[1].data;//js_get_value(1);
    if(config.btn.dpad_mapping_js&&(sts_button&DPAD_MASK)){
        sts_ljoy=(2048<<12)+2048;
        if(sts_button&(1<<NS_BUTTON_LEFT)){
            sts_ljoy-=get_ljs_range_on_axis(0);
        }
        if(sts_button&(1<<NS_BUTTON_RIGHT)){
            sts_ljoy+=get_ljs_range_on_axis(1);
        }
        if(sts_button&(1<<NS_BUTTON_UP)){
            sts_ljoy+=get_ljs_range_on_axis(2)<<12;
        }
        if(sts_button&(1<<NS_BUTTON_DOWN)){
            sts_ljoy-=get_ljs_range_on_axis(3)<<12;
        }
        sts_button&=~DPAD_MASK;
    }
    data->button_status=sts_button;
    data->ljoy_status = sts_ljoy;
    data->rjoy_status = sts_rjoy;
    /*data->ljoy_status=sts_ljoy;
    data->rjoy_status=sts_rjoy;*/
}
static uart_packet pkt;
static uint32_t top_tick=0;
static uart_packet test_uart_pkt;
void func_switch_task(){
    static uint8_t f1=0,f2=0,f3=0,f4=0,f5=0,f6=0,fhf=0;
    static uint8_t save=0,upd=0;
    upd=save=0;
    if(top_trigger&&button_read_raw(NS_BUTTON_A)){//cause hard fault
        if(fhf){
            //trigger_hardfault();
            fhf=0;
        }
    }else   fhf=1;
    if(top_trigger&&button_read_raw(NS_BUTTON_MINUS)){//imu switch
        if(f1){
            config.imu.enable = !config.imu.enable;
            upd=save=1;
            f1=0;
        }
    }else   f1=1;
    if(top_trigger&&button_read_raw(NS_BUTTON_PLUS)){
        if(f2){
            config.rumble.enable = !config.rumble.enable;
            hd_rumble_set_status(!config.rumble.enable);
            upd=save=1;
            f2=0;
        }
    }else   f2=1;
    if(top_trigger&&button_read_raw(NS_BUTTON_CAP)){
        if(f3){
            config.rgb.enable = !config.rgb.enable;
            save=1;
            f3=0;
        }
    }else   f3=1;
    if(top_trigger&&button_read_raw(NS_BUTTON_RS)){
        if(f4){
            static uint32_t tk=0;
            if(Get_Systick_MS()-tk>100){
                pkt.typ=UART_PKG_PWR_CONTROL;
                pkt.id=0;
                pkt.load[0]=!force_esp32_active;
                send_uart_pkt(&pkt);
                tk=Get_Systick_MS();
            }
            //force_esp32_active=!force_esp32_active;
            f4=0;
        }
    }else   f4=1;
    if(top_trigger&&button_read_raw(NS_BUTTON_LS)){
        if(f5){
            pkt.typ=UART_PKG_PWR_CONTROL;
            pkt.id=0xf;//reboot esp32
            send_uart_pkt(&pkt);
            f5=0;
        }
    }else f5=1;
    if(top_trigger&&!connection_state.usb_paired&&button_read_raw(NS_BUTTON_HOME)){
        if(f6){
            send_bt_cmd(BT_CMD_LISTEN,0);
            f6=0;
        }
    }else f6=1;
    if(top_trigger&&button_read_raw(NS_BUTTON_UP)){
            NVIC_SystemReset();
    }
    /*if(upd)
        flush_rgb();*/
    if(save)//we save to flash
    {
        config.magic = CONFIG_MAGIC;
        custom_conf_write();
    }
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
                    for(int i=0;i<2;++i){
                        for(int j=0;j<config.affine[i].cnt;++j){
                            printf("notch: %d %d\r\n",config.affine[i].map[j].notch.x,config.affine[i].map[j].notch.y);
                            printf("angle: %d %d\r\n",config.affine[i].map[j].angle.x,config.affine[i].map[j].angle.y);
                            /*printf("%d %d\r\n%d %d\r\n",ceil(affine_trans[i][j].a*1000000),
                                    ceil(affine_trans[i][j].b*1000000),ceil(affine_trans[i][j].c*1000000),
                                    ceil(affine_trans[i][j].d*1000000));*/
                        }
                    }
                }
                top_trigger=1;
                set_indicate_led_mode(1);
            }
        }
        else {
            top_tick=Get_Systick_MS();
        }
    }
    else if(top_trigger){
        set_indicate_led_mode(0);
        top_trigger=0;
        top_tick=0;
    }
}
void pwr_detector(){
    static uint8_t thres_cnt=0;
    Voltage_ThresFlag = PWR_GetFlagStatus(PWR_FLAG_PVDO);
    if(Voltage_ThresFlag){
        thres_cnt++;
        if(thres_cnt>8){
            set_pwr_mode_stop();
            thres_cnt=0;
        }
    }else {
        thres_cnt=0;
    }
}
void performance_monitor(){
    if(!rts_cnt)
        last_pm_start=Get_Systick_MS();
    ++rts_cnt;
    if(Get_Systick_MS()<last_pm_start){
        last_pm_start=Get_Systick_MS();
        rts_cnt=1;
    }
}
void routine_service(void){
    performance_monitor();
    top_timer();
    push_waveform_into_buffer_task();
    func_switch_task();
    rgb_task();
    start_connect();
    imu_upd();
    pwr_detector();
}
void routine_service_init(void){
    top_init();
}
void set_peripherals_state(uint8_t state)
{
    DMA_Cmd(DMA1_Channel4, state);
    DMA_Cmd(DMA1_Channel5, state);
    DMA_Cmd(DMA1_Channel1, state);
    ADC_SoftwareStartConvCmd(ADC1, state);
    TIM_Cmd(TIM2, state);
    TIM_Cmd(TIM3, state);
    USART_Cmd(USART1, state);
    USART_DMACmd(USART1, USART_DMAReq_Rx, state);
}
void init_all()
{
    //SetSysClock();//this seems not reenterable?why?
    RCC->CTLR|=RCC_CSSON;
    RCC->CTLR=RCC->CTLR&(~RCC_SW)|0x01;
    RCC_LSICmd(ENABLE);
    SystemCoreClockUpdate();
    //RCC->CFGR0 &= ~RCC_HPRE;
    //RCC->CFGR0 &= ~RCC_PPRE2;
    //RCC->CFGR0 &= ~RCC_PPRE1;
    USART_printf_Init(115200);
    printf("\r\n\r\nPROCON PROJECT\r\n");
    memset(&connection_state,0,sizeof(connection_state));
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    //set_peripherals_state(ENABLE);
    HighPrecisionTimer_Init();
    SysTick_Init();
    //printf("systick\r\n");
    pwr_init();
    Delay_MS(20);
    //reliable_uart_init();
    //printf("conf\r\n");
    conf_init();
    /*RCC_ClocksTypeDef rcc_clock;
    RCC_GetClocksFreq(&rcc_clock);*/
    gpio_init();
    UART1_Init();
    UART1_DMA_Init();
    wake_esp32_init();
    gpio_set(GPIO_WAKE_BAND, 1);
    adc_init();
    routine_service_init();
    rgb_init();
    imu_spi_init();
    //i2c_init();
    imu_init();
    set_imu_awake();
    conf_flush();
    USBFS_RCC_Init();
    USBFS_Device_Init( ENABLE , pwr_vdd_voltage());
    ns_mux_init();
    ns_set_peripheral_data_getter(get_peripheral_data_handler);
    hid_init();
    hd_rumble_init();
    //hd_rumble_set_status(!user_config.rumble_disabled);
    set_indicate_led_mode(0);
    connection_state.usb_plugging=1;
}

void test_delay()
{
    Delay_MS(5);
}

/*simple coroutine service*/
#define COROUTINE_MAX_CNT (32)
#define COROUTINE_MAX_TIMESLICE (200)
void (*coroutine_list[COROUTINE_MAX_CNT])();
static uint8_t coroutine_cnt;
void coroutine_init(){
#ifdef COMPILE_WL
    coroutine_list[coroutine_cnt++]=UART1_Rx_Service;
    coroutine_list[coroutine_cnt++]=uart_com_task;
    //coroutine_list[coroutine_cnt++]=UART1_Tx_Service;
    coroutine_list[coroutine_cnt++]=start_connect;
    coroutine_list[coroutine_cnt++]=connection_state_handler;
#endif
    //coroutine_list[coroutine_cnt++]=performance_monitor;
    coroutine_list[coroutine_cnt++]=top_timer;
    coroutine_list[coroutine_cnt++]=push_waveform_into_buffer_task;
    coroutine_list[coroutine_cnt++]=func_switch_task;
    coroutine_list[coroutine_cnt++]=rgb_task;
    coroutine_list[coroutine_cnt++]=imu_upd;

    //coroutine_list[coroutine_cnt++]=test_delay;
}
void coroutine_scheduling(){
    static uint32_t stage=0;
    uint32_t start=Get_Systick_US();
    while(stage<coroutine_cnt){
        //printf("stage %d\r\n",stage);
        if(coroutine_list[stage]){
            coroutine_list[stage]();
        }
        ++stage;
        if(Get_Systick_US()-start>=COROUTINE_MAX_TIMESLICE)
            break;//stop here
    }
    if(stage>=coroutine_cnt)
        stage=0;
}
void main_loop(){
    pwr_detector();
    performance_monitor();
    //update input
    //joystick_debounce_task();
    get_peripheral_data_handler(&global_input_data);

    //hid task
    if(USBFS_DevEnumStatus){//usb
        connection_state.usb_plugging=1;
        hid_rx_service();
        hid_tx_service();
    }else{
        connection_state.usb_paired=0;
    }
    send_input_to_esp();
    UART1_Tx_Service();

    //other runs in simple coroutine
    //printf("pre sche\r\n");
    coroutine_scheduling();
}
void adv_cor_scheduler_init(){
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    TIM_OCInitTypeDef       TIM_OCInitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
    TIM_TimeBaseInitStructure.TIM_Period = 14400;//72m/250
    TIM_TimeBaseInitStructure.TIM_Prescaler = 0;//
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);
    TIM_SelectOnePulseMode(TIM4,TIM_OPMode_Single);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
    TIM_ClearITPendingBit(TIM4, TIM_FLAG_Update);

    NVIC_InitTypeDef NVIC_InitStructure; //定义NVIC初始化结构体
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn; //设置NVIC通道为定时器2中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //设置NVIC通道抢占优先级为0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; //设置NVIC通道子优先级为0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能NVIC通道
    NVIC_Init(&NVIC_InitStructure); //初始化NVIC
    //NVIC_SetFastIRQ(TIM4_IRQHandler,TIM4_IRQn,3);

    NVIC_ClearPendingIRQ(Software_IRQn);
    NVIC_SetPriority(Software_IRQn, 0x80);
    NVIC_EnableIRQ(Software_IRQn);
}
void adv_cor_scheduler_cmd(uint8_t state){
    TIM_Cmd(TIM4, state);
    TIM4->CNT=0;
    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
}
void adv_cor_schedule()
{
    uint32_t stage=0;
    while(stage<coroutine_cnt){
        if(coroutine_list[stage])
            coroutine_list[stage]();
        ++stage;
    }
}
void adv_cor_scheduling(){
    adv_cor_scheduler_cmd(ENABLE);
    if(adv_cor_intr_flag){
        NVIC_SetPendingIRQ(Software_IRQn);
    }else{
        adv_cor_schedule();
        adv_cor_scheduler_cmd(DISABLE);
    }
}
void adv_cor_main(){
    while(1){
        adv_cor_scheduler_cmd(DISABLE);
        pwr_detector();
        performance_monitor();
        //update input
        get_peripheral_data_handler(&global_input_data);

        //hid task
        if(USBFS_DevEnumStatus){//usb
            connection_state.usb_plugging=1;
            hid_rx_service();
            hid_tx_service();
        }else{
            connection_state.usb_paired=0;
        }
        send_input_to_esp();
        UART1_Tx_Service();
        adv_cor_scheduling();
    }
}
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    init_all();
    printf("SystemClk:%d\r\n",SystemCoreClock);
    printf("%08x\r\n",RCC->RSTSCKR);
#ifdef USE_COROUTINE
    coroutine_init();
    while(1){
        main_loop();
    }
#elif defined(USE_ADV_COR)
    printf("adv cor\r\n");
    NVIC_HaltPushCfg(DISABLE);
    coroutine_init();
    adv_cor_scheduler_init();
    adv_cor_main();
#else
    while(1)
    {
#ifdef COMPILE_WL
        UART1_Rx_Service();//check if uart recive anything
#endif
        get_peripheral_data_handler(&global_input_data);
        if(USBFS_DevEnumStatus){//usb
            connection_state.usb_plugging=1;
            hid_rx_service();
            hid_tx_service();
        }else{
            connection_state.usb_paired=0;
        }
#ifdef COMPILE_WL
        uart_com_task();
        send_input_to_esp();
        connection_state_handler();
        UART1_Tx_Service();
#endif
        routine_service();
    }
#endif
}

