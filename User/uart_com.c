/*
 * uart_com.c
 *
 *  Created on: 2024Äê10ÔÂ16ÈÕ
 *      Author: Reed
 */

#include "debug.h"
#include "string.h"
#include "ch32v10x_usbfs_device.h"
#include "usbd_compatibility_hid.h"
#include "ring_buffer.h"
#include "gpio_digit.h"
#include "ns_com.h"
#include "ns_com_mux.h"
#include "uart_com.h"
#include "uart.h"
#include "global_api.h"
#include "spi.h"
#include "conf.h"
#include "pwr.h"
#include "imu.h"
static uart_packet pkt;
static uint32_t input_update_tick;
void uart_conf_write(uint32_t addr,uint8_t* ptr,uint8_t size)
{
   pkt.typ=UART_PKG_CH32_FLASH_READ;
   pkt.id=addr>>12;
   addr&=0xff;
   for(int i=0;i<size;i+=8){
       pkt.load[0]=i+addr;
       memcpy(pkt.load+1,ptr,8);
       ptr+=8;
       send_uart_pkt(&pkt);
   }
}
void uart_update_config()
{
    pkt.typ=UART_PKG_CH32_FLASH_WRITE;
    pkt.id=0xF;
    for(int i=0;i<sizeof(user_config);i+=8){
        pkt.load[0]=i;
        memcpy(pkt.load+1,((uint8_t*)&user_config)+i*8,8);
        send_uart_pkt(&pkt);
        send_uart_pkt(&pkt);
    }
    /*pkt.id=0x6;
    for(int i=0;i<sizeof(factory_configuration);i+=8){
        pkt.load[0]=i;
        memcpy(pkt.load+1,((uint8_t*)&factory_configuration)+i*8,8);
        send_uart_pkt(&pkt);
        send_uart_pkt(&pkt);
    }
    pkt.id=0x8;
    for(int i=0;i<sizeof(user_calibration);i+=8){
        pkt.load[0]=i;
        memcpy(pkt.load+1,((uint8_t*)&user_calibration)+i*8,8);
        send_uart_pkt(&pkt);
        send_uart_pkt(&pkt);
    }*/
}
static uint8_t imu_report_buffer_ptr_reset_flag;
void imu_buffer_reset_notifier()
{
    imu_report_buffer_ptr_reset_flag=1;
}
void send_input_with_uart(void)
{
    memset(&pkt,0,UART_PKG_SIZE);
    pkt.typ=UART_PKG_INPUT_DATA;
    pkt.id=1;//button status
    /*pkt.load=global_input_data.button_status;
    send_uart_pkt(&pkt);
    pkt.id=2;//ljoy status
    pkt.load=global_input_data.ljoy_status;
    send_uart_pkt(&pkt);
    pkt.id=3;//rjoy status
    pkt.load=global_input_data.rjoy_status;
    send_uart_pkt(&pkt);*/
    memcpy(pkt.load,&global_input_data,9);
    send_uart_pkt(&pkt);
    //if(connection_state.esp32_connected&&!connection_state.usb_paired&&!connection_state.esp32_sleep)
    if(imu_mode&&imu_report_buffer_ptr_reset_flag){
        send_uart_large_pkt(imu_report_buffer_ptr,sizeof(imu_report_pack),UART_PKG_IMU_REPORT_DATA);
        imu_report_buffer_ptr_reset_flag=0;
    }
}
void start_connect(){
    static uint32_t tick;
    if((!sts_button)||connection_state.usb_paired)//if no button pressed or paired by usb,
        return;
    if((Get_Systick_MS()-tick>START_CONNECTION_GAP)&&(
    connection_state.esp32_connected&&!connection_state.esp32_bt_state&&!connection_state.esp32_sleep||!connection_state.esp32_paired))
    {
        pkt.typ=UART_PKG_CONNECT_CONTROL;
        pkt.id=0;
        pkt.load[0]=1;
        send_uart_pkt(&pkt);
    }
}
void wake_esp32()
{
    //we wake up with uart
    pkt.typ=UART_PKG_PWR_CONTROL;
    pkt.id=1;
    memset(pkt.load,-1,9);
    send_uart_pkt(&pkt);
}
void recv_esp32_connect_control()
{
    switch(pkt.id){
    case 0:
        memcpy(connection_state.bd_addr,pkt.load,BD_ADDR_LEN);
        connection_state.bd_addr_set=1;
        //we reset this every time we start
        break;
    case 1:
        for(int i=0;i<9;++i)
            pkt.load[i]^=0xAA;
        memcpy(connection_state.bt_ltk,pkt.load,9);
        break;
    case 2:
        for(int i=0;i<7;++i)
            pkt.load[i]^=0xAA;
        memcpy(connection_state.bt_ltk+9,pkt.load,7);
        connection_state.bt_ltk_set=1;
        break;
    case 0xC:
        connection_state.esp32_paired=pkt.load[0];
        flush_rgb(!connection_state.esp32_sleep);
        ////printf("esp32_paired:%d\r\n",connection_state.esp32_paired);
        break;
    /*case 0xD:
        connection_state.esp32_sleep=1;
        break;*/
    case 0xE:
        connection_state.con_addr_set=0;
        break;
    case 0xF:
        connection_state.esp32_bt_state=pkt.load[0];
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
        decode_hd_rumble_multiformat_high_acc(pkt.load,pkt.load+4);
        break;
    default:
        break;
    }
}
void recv_pwr_control(){
    switch(pkt.id){
    case 0x00://force_esp32_active
        force_esp32_active=pkt.load[0];
        flush_rgb(ENABLE);
        break;
    case 0x01:
        connection_state.esp32_sleep=1;
        break;
    case 0x02://for wireless update,we use this to restart ch32 mcu.
        flush_rgb(DISABLE);
        Delay_Ms(10);
        NVIC_SystemReset();
        break;
    default:
        break;
    }
}
void recv_flash_operation(){
    if(pkt.typ!=UART_PKG_CH32_FLASH_READ&&pkt.typ!=UART_PKG_CH32_FLASH_WRITE)return;
    if(pkt.typ==UART_PKG_CH32_FLASH_READ){
        //pkt.typ=UART_PKG_CH32_FLASH_READ;
        switch(pkt.id_short){
            case 0xF://user config
                conf_read(0xF000|(pkt.load[0]), pkt.load+1, 8);
                break;
            case 0x6:
                conf_read(0x6000|(pkt.load[0]), pkt.load+1, 8);
                break;
            case 0x8:
                conf_read(0x8000|(pkt.load[0]), pkt.load+1, 8);
                break;
            default:
                break;
        }
    }else{//write
        //pkt.typ=UART_PKG_CH32_FLASH_WRITE;
        switch(pkt.id_short){
            case 0xF://user config
                conf_write(0xF000|(pkt.load[0]),pkt.load+1,i32_min(8,sizeof(user_config)-pkt.load[0]),pkt.flag);
                break;
            case 0x6:
                conf_write(0x6000|(pkt.load[0]),pkt.load+1,i32_min(8,sizeof(factory_configuration)-pkt.load[0]),pkt.flag);
                break;
            case 0x8:
                conf_write(0x8000|(pkt.load[0]),pkt.load+1,i32_min(8,sizeof(user_calibration)-pkt.load[0]),pkt.flag);
                break;
            default:
                break;
        }
    }
    send_uart_pkt(&pkt);
    //send_uart_pkt(&pkt);
}
void recv_esp32_pkg()
{
    switch(pkt.typ)
    {
    case UART_PKG_INPUT_REQ:
        ////printf("recv uart hb\r\n");
        break;
    case UART_PKG_CONNECT_CONTROL:
        recv_esp32_connect_control();
        break;
    case UART_PKG_RUMBLE_FRAME:
        recv_rumble_frame();
        break;
    case UART_PKG_PWR_CONTROL:
        recv_pwr_control();
        break;
    case UART_PKG_CH32_FLASH_READ:
        /*fall through*/
    case UART_PKG_CH32_FLASH_WRITE:
        recv_flash_operation();
        break;
    default:
        break;
    }
}
void connection_state_handler()//decide if we go stop
{
    start_connect();
    if(!connection_state.usb_enumed && connection_state.esp32_sleep)
    {
        set_peripherals_state(DISABLE);
        set_imu_sleep();
        uint8_t stop_flag=set_pwr_mode_stop();
        if(stop_flag)//fk it,i have no idea what are needed to reset,so lets restart the mcu as its not that slow
            NVIC_SystemReset();
        //fail safe,try to reset everything
        init_all();
        //set_peripherals_state(ENABLE);
        wake_esp32();
    }
    else if(connection_state.esp32_connected&&(!connection_state.esp32_sleep)&&(Get_Systick_MS()-input_update_tick>UART_REPORT_GAP))
    {
        input_update_tick=Get_Systick_MS();
        if(connection_state.con_addr_set)//if esp32 recved,we set this flag to be zero
        {
            pkt.typ=UART_PKG_CONNECT_CONTROL;
            pkt.id=3;
            memset(pkt.data,0,11);
            memcpy(pkt.load,connection_state.con_addr,6);
            send_uart_pkt(&pkt);
        }
        if(!connection_state.usb_paired)
        {
            send_input_with_uart();
            ////printf("send bt start\r\n");
            if(!connection_state.bd_addr_set)
            {
                //printf("send get bd addr cmd\r\n");
                pkt.typ=UART_PKG_CONNECT_CONTROL;
                pkt.id=1;
                //pkt.arr[0]=1;
                send_uart_pkt(&pkt);
            }
            if(!connection_state.bt_ltk_set)
            {
                //printf("send get     bt ltk cmd\r\n");
                pkt.typ=UART_PKG_CONNECT_CONTROL;
                pkt.id=2;
                //pkt.arr[0]=1;
                send_uart_pkt(&pkt);
            }
            /*if(!(connection_state.esp32_bt_state&0x1))
            {
                ////printf("send bt start cmd\r\n");
                pkt.typ=UART_PKG_CONNECT_CONTROL;
                pkt.id=0;
                pkt.arr[0]=1;
                send_uart_pkt(&pkt);
            }*/
            //we active esp32 connect when user request
        }
        else{//if usb paired,donot report to esp32 as esp32 doesnt need input data now
            if((connection_state.esp32_bt_state&0x1))
            {
                ////printf("send bt stop cmd\r\n");
                pkt.typ=UART_PKG_CONNECT_CONTROL;
                pkt.id=0;
                pkt.load[0]=0;
                send_uart_pkt(&pkt);
                //actively close bt connection
            }
        }

    }
}
void uart_com_task()
{
    uint8_t max_uart_handled=50;
    while(uart_rx_rb.size&&max_uart_handled--)
    {
        connection_state.esp32_connected=0x01;//if revice uart pkt from esp32,means it exist
        memcpy(&pkt,&uart_rx_rb_buf[uart_rx_rb.top*UART_PKG_SIZE],UART_PKG_SIZE);
        ring_buffer_pop(&uart_rx_rb);
        recv_esp32_pkg();
    }
    connection_state_handler();
}
