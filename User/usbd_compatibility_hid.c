#include "debug.h"
#include "string.h"
#include "ch32v10x_usbfs_device.h"
#include "usbd_compatibility_hid.h"
#include "ring_buffer.h"
#include "ns_com.h"
#include "ns_com_mux.h"
#include "uart_com.h"
#include "uart.h"
#include "rumble.h"
#include "imu.h"
#include "tick.h"
#include "conf.h"

__attribute__ ((aligned(4))) uint8_t  HID_Report_Buffer[64];              // HID Report Buffer
volatile uint8_t HID_Set_Report_Flag = SET_REPORT_DEAL_OVER;               // HID SetReport flag


void (*get_peripheral_data)(peripheral_data*);
peripheral_data global_input_data;


void ns_set_peripheral_data_getter(void (*getter)(peripheral_data*)){
        get_peripheral_data=getter;
}
void update_peripheral_data(){
        if(!get_peripheral_data)
        {
            //printf("error no peripheral data getter set\r\n");
               //ESP_LOGE("","%s error no peripheral data getter set",__func__);
               return;
        }
        get_peripheral_data(&global_input_data);
}
static uint8_t ns_hid_pkt_cnt;
uint8_t ns_hid_get_packet_timer(){
    switch(user_config.ns_pkt_timer_mode)
    {
    case 1:
        return Get_Systick_MS();
    case 2:
        return ns_hid_pkt_cnt++;
    case 0:
    default:
        return Get_Systick_MS()/5;
    }
}
void rpt_warpper(std_report* rpt,std_report_data* data,uint16_t len){
    ////printf("rpt\r\n");
        //todo : support more report typ someday
        memset(rpt,0,sizeof(std_report));
        rpt->typ=input_mode;
        rpt->typ=0x30;
        if(!rpt){
                //printf("error rpt addr not set\r\n");
                //ESP_LOGE("","%s error rpt not exist",__func__);
                return;
        }
        if(data&&len){
                memcpy(&(rpt->data),data,len);
                rpt->typ=0x21;
        }
        //update_peripheral_data();
        rpt->input_data=global_input_data;
        //rpt->timer=global_packet_timer;
        rpt->timer=ns_hid_get_packet_timer();
        rpt->battery_status=0x09;
        //todo : support battery quantity and charge state(oops,hardware doesnt support this one,so forget it);
        rpt->con_info=0x01;//todo : expect set this to 0x00 to be pro controller on battery,check if its correct;
        rpt->rumble_status=0x00;//todo : value includes(0x70,0xC0,0xB0,0x80,0xA0),what does these mean?
}
uint8_t empty_report[]={0xa1,0x00};
uint8_t* pkg_ptr;
uint8_t pkg_len,pkg_typ;
std_report *rpt;
imu_report_pack* imu_report_buffer_ptr;
void set_imu_available(imu_report_pack* p){
    imu_report_buffer_ptr=p;
    imu_buffer_reset_notifier();
}
void hid_init()
{
    rpt=(std_report*)pEP1_IN_DataBuf;
}
void hid_tx_service( void )
{
    if(USBFS_Endp_Busy[DEF_UEP1]){//busy, so skip
        ////printf("busy %d\r\n",Get_Systick_MS());
        return;
    }
    ////printf("not busy\r\n");
    if(ns_usb_send_rb.size)//special packet to send
    {
        pkg_ptr=&ns_usb_send_rb.buf[ns_usb_send_rb.top*NS_USB_RINGBUFFER_PKG_SIZE+1];
        pkg_len=ns_usb_send_rb.len[ns_usb_send_rb.top];
        //pkg_typ=ns_usb_send_rb.typ[ns_usb_send_rb.top];
        pkg_typ=ns_usb_send_rb.buf[ns_usb_send_rb.top*NS_USB_RINGBUFFER_PKG_SIZE];
        ring_buffer_pop(&ns_usb_send_rb);
        switch(pkg_typ){
        case 0x00://std report
            rpt_warpper(rpt,(std_report_data*)pkg_ptr,pkg_len);
            pkg_len+=NS_STD_REPORT_BASIC_LENGTH;
            break;
        case 0x01://hid report
            //if hid report,just send
            ////printf("hid raw\r\n");
            memcpy(rpt,pkg_ptr,pkg_len);
            break;
        /*case 0x02:
            rpt_warpper(rpt,(std_report_data*)pkg_ptr,pkg_len);
            pkg_ptr=(uint8_t*)rpt;
            pkg_len=64;
            break;
        case 0x03:
            memset(rpt,0,64);
            memcpy(rpt,pkg_ptr,pkg_len);
            pkg_len=64;
        */
        default:
            break;
        }
    }
    else{//std report or empty report
        rpt_warpper(rpt,NULL,0);
        pkg_len=NS_STD_REPORT_BASIC_LENGTH;
        if((!user_config.imu_disabled)&&imu_mode&&i2c_status){
            if(imu_report_buffer_ptr){
                memcpy(&rpt->data.imu_report,imu_report_buffer_ptr,sizeof(imu_report_pack));
                pkg_len+=sizeof(imu_report_pack);
                //rep=NULL;
            }
        }
        pkg_ptr=(uint8_t*)rpt;
        //pkg_len=NS_STD_REPORT_BASIC_LENGTH;
    }
    pkg_len=64;
    R16_UEP1_T_LEN = pkg_len;
    USBFS_Endp_Busy[DEF_UEP1] = 1;
    //R8_UEP1_CTRL being 0x40023436,we need to handle 4 byte at once.
    __AMOAND_W(0x40023434,0xFFFCFFFF);
    //here is a critical zone which isnt taken care of;is this the reason why usb will disconnect randomly?
    //i hope so,this shit is starting to be irritating.
    //R8_UEP1_CTRL = (R8_UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
}
static uint8_t usb_rx_buf[NS_USB_RINGBUFFER_PKG_SIZE];
void hid_rx_service(){
    static cmd_packet pkt;
    uint8_t pkg_len=0;
    uint8_t* pbuf=usb_rx_buf;
    if(ns_usb_recv_rb.size){
        pkg_len=ns_usb_recv_rb.len[ns_usb_recv_rb.top];
        memcpy(pbuf,&ns_usb_recv_buf[ns_usb_recv_rb.top*NS_USB_RINGBUFFER_PKG_SIZE],pkg_len);
        //we minimize critical section with atomic instruction,maybe unexpected disconnecting will be solved.
        ring_buffer_pop(&ns_usb_recv_rb);
        pkt.data=pbuf;
        pkt.len=pkg_len;
        hid_dispatch(&pkt);
    }
}


/*********************************************************************
 * @fn      HID_Set_Report_Deal
 *
 * @brief   print hid set report data
 *
 * @return  none
 */
void HID_Set_Report_Deal()
{
    uint16_t i;
    if (HID_Set_Report_Flag == SET_REPORT_WAIT_DEAL)
    {
        //printf("Set Report:\n");
        for (i = 0; i < 64; ++i)
        {
            //printf("%02x ",HID_Report_Buffer[i]);
        }
        //printf("\n");
        HID_Set_Report_Flag = SET_REPORT_DEAL_OVER;
        R8_UEP0_T_LEN = 0;
        R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_T_RES_ACK;
    }
}


