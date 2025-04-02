/*
 * usb_com.c
 *
 *  Created on: 2024Äê10ÔÂ15ÈÕ
 *      Author: Reed
 */
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include "stdio.h"
#include "ns_com.h"
#include "ns_com_mux.h"
#include "global_api.h"
#include "rumble.h"
#include "conf.h"
#include "hd_rumble.h"
#include "imu.h"

NS_REPORT_MODE report_mode;
uint8_t is_connected,is_paired,auto_con,global_packet_timer;
uint32_t average_packet_gap,last_packet_send,max_packet_gap,tp_timer;

void (*ns_cmd_subcommand_cb_tb[NS_SUBCOMMAND_ID_MAX_VLAUE])(cmd_subcommand*,uint8_t);

#define NS_SUBCOMMAND_CB_PARAM cmd_subcommand* cmd,uint8_t len
#define DEFAULT_ACK NS_SUBCOMMAND_STATUS_ACK

#define SPI_DATA_MAX_SIZE (30)
static report_packet pkt;

#define SUBC_REPORT_BASIC_LENGTH (2)
void pkt_clr(){
    pkt.len=pkt.oob_len=0;
    pkt.oob_data=NULL;
}
void _ns_subcommand_set_ack(uint8_t id,uint8_t status){//default status 0x80
    pkt.data.subcommand_report.subcommand_status=status;
    //pkt.data.subcommand_report.subcommand_id=cmd->subcommand_id;
    pkt.data.subcommand_report.subcommand_id=id;
}

#define SUBC_ID_DEFAULT (0x00)
#define SUBC_REPORT_DEFAUT_LENGTH (3)
void ns_subcommand_default(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    _ns_subcommand_set_ack(cmd->subcommand_id,DEFAULT_ACK);
    pkt.data.subcommand_report.subcommand_data[0]=0x03;
    pkt.len=SUBC_REPORT_DEFAUT_LENGTH;
    ns_send_report(&pkt);
}

#define SUBC_ID_PAIR_BY_WIRE (0x01)
const uint8_t pro_controller_string[24] = {0x00, 0x25, 0x08, 0x50, 0x72, 0x6F, 0x20, 0x43, 0x6F,
                                             0x6E, 0x74, 0x72, 0x6F, 0x6C, 0x6C, 0x65, 0x72, 0x00,
                                             0x00, 0x00, 0x00, 0x00, 0x00, 0x68};
void ns_subcommand_pair_by_wire(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    uint8_t typ=cmd->subcommand_data[0];
    _ns_subcommand_set_ack(cmd->subcommand_id,0x81);
    pkt.data.subcommand_report.subcommand_data[0]=typ;
    printf("pair by wire subc id:%d \t typ:%d\r\n",cmd->subcommand_id,typ);
    switch(typ){
    case 0x01://request bd addr
        memcpy(connection_state.con_addr,cmd->subcommand_data+1,BD_ADDR_LEN);
        connection_state.con_addr_set=1;
        memcpy(pkt.data.subcommand_report.subcommand_data+1,connection_state.bd_addr,BD_ADDR_LEN);
        memcpy(pkt.data.subcommand_report.subcommand_data+1+BD_ADDR_LEN,pro_controller_string,sizeof(pro_controller_string));
        pkt.len=SUBC_REPORT_BASIC_LENGTH + sizeof(pro_controller_string)+BD_ADDR_LEN+1;
        break;
    case 0x02:
        memcpy(pkt.data.subcommand_report.subcommand_data+1,connection_state.bt_ltk,BT_LTK_LENGTH);
        pkt.len=SUBC_REPORT_BASIC_LENGTH+17;
        break;
    case 0x03:
        pkt.len=SUBC_REPORT_BASIC_LENGTH+1;
        break;
    default:
        pkt.len=SUBC_REPORT_BASIC_LENGTH+1;
        break;
    }
    ns_send_report(&pkt);
}


#define SUBC_REPORT_GET_DEVICE_INFO_LENGTH (14)
#define SUBC_ID_GET_DEVICE_INFO (0x02)
#pragma pack(push,1)
typedef struct{
    uint8_t fw_version_major;
    uint8_t fw_version_minor;
    uint8_t device_typ;
    uint8_t reserved_0x02;
    uint8_t addr[BD_ADDR_LEN];
    uint8_t reserved_0x01;
    uint8_t use_spi_color;
}device_info;
#pragma pack(pop)
device_info default_device_info={0x04,0x33,0x03,0x02,{},0x00,0x01};
//device_info default_device_info={0x03,0x48,0x03,0x02,{},0x00,0x02};
void ns_subcommand_get_device_info(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    switch(user_config.pro_fw_version){
    case 0:
        default_device_info.fw_version_major=0x03;
        default_device_info.fw_version_minor=0x48;
        break;
    case 1:
    default:
        default_device_info.fw_version_major=0x04;
        default_device_info.fw_version_minor=0x33;
    }
    //pkt.report.subcommand_report.subcommand_status=NS_SUBCOMMAND_STATUS_ACK;
    //pkt.report.subcommand_report.subcommand_id=cmd->subcommand_id;
    //_ns_subcommand_set_ack(cmd->subcommand_id,DEFAULT_ACK);
    _ns_subcommand_set_ack(cmd->subcommand_id,0x82);
    //uint8_t* addr = esp_bt_dev_get_address();
    //todo
    for(int i=0;i<BD_ADDR_LEN;++i){
        default_device_info.addr[i]=connection_state.bd_addr[BD_ADDR_LEN-i-1];
        printf("0x%02x ",connection_state.bd_addr[BD_ADDR_LEN-i-1]);
    }
    printf("\r\n");
    //memcpy(default_device_info.addr,bd_addr,BD_ADDR_LEN);
    //ESP_LOGW("MAC ","0x%02x  0x%02x  0x%02x",default_device_info.addr[5],bt_addr[5],addr[5]);
    memcpy(pkt.data.subcommand_report.subcommand_data,&default_device_info,sizeof(device_info));
    pkt.len=SUBC_REPORT_GET_DEVICE_INFO_LENGTH;
    ns_send_report(&pkt);
}

uint8_t input_mode=0x30;
#define SUBC_ID_SET_INPUT_MODE (0x03)
void ns_subcommand_set_input_mode(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    printf("set input mode:0x%02x \r\n",cmd->subcommand_data[0]);
    if(cmd->subcommand_data[0]==0x30||cmd->subcommand_data[0]==0x31||cmd->subcommand_data[0]==0x32||cmd->subcommand_data[0]==0x33)
        input_mode=cmd->subcommand_data[0];
    _ns_subcommand_set_ack(cmd->subcommand_id,DEFAULT_ACK);
    pkt.len=SUBC_REPORT_BASIC_LENGTH;
    ns_send_report(&pkt);
    //ESP_LOGW("","%s set input mode %02x",__func__,cmd->subcommand_data[0]);
    //is_paired=true;
}

#define SUBC_REPORT_TRIGGER_BUTTONS_ELAPSED_TIME_LENGTH (16)
#define SUBC_ID_TRIGGER_BUTTONS_ELAPSED_TIME (0x04)
#pragma pack(push,1)
typedef struct{
    uint16_t L;
    uint16_t R;
    uint16_t ZL;
    uint16_t ZR;
    uint16_t SL;
    uint16_t SR;
    uint16_t HOME;
}buttons_elapsed_time;
#pragma pack(pop)
buttons_elapsed_time default_et={0x6a00,0xbb01,0x9301,0x9501,0x00};
void ns_subcommand_trigger_buttons_elapsed_time(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    //_ns_subcommand_set_ack(cmd->subcommand_id,DEFAULT_ACK);
    _ns_subcommand_set_ack(cmd->subcommand_id,0x83);
    buttons_elapsed_time* ptr=(buttons_elapsed_time*)pkt.data.subcommand_report.subcommand_data;
    *ptr=default_et;
    //todo
    pkt.len=SUBC_REPORT_TRIGGER_BUTTONS_ELAPSED_TIME_LENGTH;
    ns_send_report(&pkt);
}

#define SUBC_REPORT_GET_PAGE_LIST_STATE_LENGTH (3)
#define SUBC_ID_GET_PAGE_LIST_STATE (0x05)
void ns_subcommand_get_page_list_state(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    _ns_subcommand_set_ack(cmd->subcommand_id,DEFAULT_ACK);
    pkt.data.subcommand_report.subcommand_data[0]=0x1;
    pkt.len=SUBC_REPORT_GET_PAGE_LIST_STATE_LENGTH;
    ns_send_report(&pkt);
}

#define SUBC_ID_SET_HCI_STATE (0x06)
void ns_subcommand_set_hci_state(NS_SUBCOMMAND_CB_PARAM){
    uint8_t state=cmd->subcommand_data[0];
    //todo
    switch(state){
        case 0b0://break connection,go page scan
            break;
        case 0b1://reboot,go page mode
            break;
        case 0b10://reboot,go inquiry mode
            break;
        case 0b100://reboot,go reconnect
            break;
    }
}

#define SUBC_ID_RESET_PAIRING_INFO (0x07)
void ns_subcommand_reset_pairing_info(NS_SUBCOMMAND_CB_PARAM){
    //todo remove con_mac
}

#define SUBC_ID_SET_SHIPPING_MODE (0x08)
void ns_subcommand_set_shipping_mode(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    _ns_subcommand_set_ack(cmd->subcommand_id,DEFAULT_ACK);
    pkt.data.subcommand_report.subcommand_data[0]=0x00;
    pkt.len=SUBC_REPORT_BASIC_LENGTH+1;
    ns_send_report(&pkt);
    //this subcommand will be send by switch on every connection,so do nothing for now
    //UPD 2024/10/20 this suncommand must reply 0x00 apart from stand ack header,otherwise usb hid pairing wont continue.
}
/*
static uint8_t reply1060[] = {0x00, 0x60, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                              , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static uint8_t reply1050[] = {  0x50, 0x60, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                               , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static uint8_t reply1080[] = { 0x80, 0x60, 0x00, 0x00, 0x18, 0x5e, 0x01, 0x00, 0x00, 0xf1, 0x0f,
                              0x19, 0xd0, 0x4c, 0xae, 0x40, 0xe1,
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0x00, 0x00};
static uint8_t reply1098[] = { 0x98, 0x60, 0x00, 0x00, 0x12, 0x19, 0xd0, 0x4c, 0xae, 0x40, 0xe1,
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0x00, 0x00};
//User analog stick calib
static uint8_t reply1010[] = { 0x10, 0x80, 0x00, 0x00, 0x18, 0x00, 0x00};
static uint8_t reply103D[] = {0x3D, 0x60, 0x00, 0x00, 0x19, 0xF0, 0x07, 0x7f, 0xF0, 0x07, 0x7f, 0xF0, 0x07, 0x7f, 0xF0, 0x07, 0x7f, 0xF0, 0x07, 0x7f, 0xF0, 0x07, 0x7f, 0xF0, 0x07, 0x7f, 0xF0, 0x07, 0x7f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00};
static uint8_t reply1020[] = {0x20, 0x60, 0x00, 0x00, 0x18, 0x00, 0x00};
static uint8_t reply3001[] = {0x21, 0x04, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x80, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t reply3333[] = {0x21, 0x03, 0x8E, 0x84, 0x00, 0x12, 0x01, 0x18, 0x80, 0x01, 0x18, 0x80, 0x80, 0x80, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                              , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
*/
#define SUBC_ID_SPI_READ (0x10)
#define SUBC_REPORT_SPI_READ_LENGTH (7)
#define SUBC_INPUT_SPI_READ_DATA_LENGTH (5)

void ns_subcommand_spi_read(NS_SUBCOMMAND_CB_PARAM){
    printf("spi read\r\n");
    pkt_clr();
    //spi_addr=*(uint32_t*)cmd->subcommand_data;
    uint32_t spi_addr=fetch_uint32(cmd->subcommand_data);
    uint8_t size=cmd->subcommand_data[4];
    printf("spi read addr 0x%08x\r\n", (uint16_t)spi_addr);
    if(size>0x1D){
        //ESP_LOGW("","%s warning spi read too long addr:%"PRIu32"  ,size:%"PRIu8,__func__,addr,size);
    }else if(!size)
        size=0x1D;
    //spi_addr=*(uint32_t*)(&(cmd->subcommand_data[4]));
    //printf("spi addr %d\r\n",spi_addr);
    //printf("spi read 0x%04x\r\n",spi_addr);
    printf("SPI_READ :0x%02x,0x%02x,0x%02x,0x%02x, size:0x%02x\r\n",cmd->subcommand_data[0],cmd->subcommand_data[1],cmd->subcommand_data[2],cmd->subcommand_data[3],size);
    _ns_subcommand_set_ack(cmd->subcommand_id,0x90);
    memcpy(pkt.data.subcommand_report.subcommand_data,cmd->subcommand_data,SUBC_INPUT_SPI_READ_DATA_LENGTH);
    if(size>sizeof(pkt.data.subcommand_report.subcommand_data)-SUBC_INPUT_SPI_READ_DATA_LENGTH)
        size=sizeof(pkt.data.subcommand_report.subcommand_data)-SUBC_INPUT_SPI_READ_DATA_LENGTH;
    conf_read(spi_addr,pkt.data.subcommand_report.subcommand_data+SUBC_INPUT_SPI_READ_DATA_LENGTH,size);
    pkt.len=SUBC_REPORT_BASIC_LENGTH+SUBC_INPUT_SPI_READ_DATA_LENGTH+size;
    ns_send_report(&pkt);
}

#define SUBC_REPORT_SPI_WRITE_LENGTH (3)
#define SUBC_ID_SPI_WRITE (0x11)
void ns_subcommand_spi_write(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    //uint32_t addr=*(uint32_t*)cmd->subcommand_data;
    uint32_t addr=fetch_uint32(cmd->subcommand_data);
    uint8_t size=cmd->subcommand_data[4];
    uint8_t *data=cmd->subcommand_data+5;
    printf("spi write addr:%d size:%d\r\n",addr,size);
    conf_write(addr, data, size);
    //todo spi write,Replies with x8011 ack and a uint8 status. x00 = success, x01 = write protected.
    _ns_subcommand_set_ack(cmd->subcommand_id,DEFAULT_ACK);
    pkt.data.subcommand_report.subcommand_data[0]=0x00;
    pkt.len=SUBC_REPORT_SPI_WRITE_LENGTH;
    ns_send_report(&pkt);
}

#define SUBC_REPORT_SPI_ERASE_LENGTH (3)
#define SUBC_ID_SPI_ERASE (0x12)
void ns_subcommand_spi_erase(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    uint32_t addr=*(uint32_t*)cmd->subcommand_data;
    uint16_t size=4096;
    //todo spi erase
    //Erases the whole 4KB in the specified address to 0xFF.
    //Replies with x8012 ack and a uint8 status. x00 = success, x01 = write protected.
    _ns_subcommand_set_ack(cmd->subcommand_id,DEFAULT_ACK);
    pkt.data.subcommand_report.subcommand_data[0]=0x00;
    pkt.len=SUBC_REPORT_SPI_ERASE_LENGTH;
    ns_send_report(&pkt);
}

#define SUBC_ID_RESET_MCU (0x20)
void ns_subcommand_reset_mcu(NS_SUBCOMMAND_CB_PARAM){
    //do nothing for now
}

#define SUBC_ID_SET_MCU_CONF (0x21)
#define SUBC_REPORT_SET_MCU_CONF_LENGTH (36)
void ns_subcommand_set_mcu_conf(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    //Takes 38 or 37 bytes long argument data.
    //ESP_LOGI("","%s set mcu conf length:%d",__func__,len);
    //reply ack with 0xA0 0x20??? why not 0x21?
    //todo : check if its correct
    pkt.data.subcommand_report.subcommand_id=0x21;
    pkt.data.subcommand_report.subcommand_status=0xA0;
    memcpy(pkt.data.subcommand_report.subcommand_data,cmd->subcommand_data,NS_SUBCOMMAND_REPORT_DATA_MAX_LENGTH);
    pkt.len=SUBC_REPORT_SET_MCU_CONF_LENGTH;
    ns_send_report(&pkt);
}

#define SUBC_ID_SET_MCU_STATE (0x22)
//todo : no reply? check if its correct
void ns_subcommand_set_mcu_state(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    uint8_t state=cmd->subcommand_data[0];
    _ns_subcommand_set_ack(cmd->subcommand_id,DEFAULT_ACK);
    pkt.len=SUBC_REPORT_BASIC_LENGTH;
    ns_send_report(&pkt);
    //0:suspend 1:resume 2:resume for update
}

#define SUBC_ID_ENABLE_MCU_POLLING (0x24)
#define SUBC_REPORT_ENABLE_MCU_POLLING_LENGTH (3)
void ns_subcommand_enable_mcu_polling(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    //todo
    //ans with ack always for now;
    _ns_subcommand_set_ack(cmd->subcommand_id,DEFAULT_ACK);
    pkt.data.subcommand_report.subcommand_data[0]=0x00;
    pkt.len=SUBC_REPORT_ENABLE_MCU_POLLING_LENGTH;
    ns_send_report(&pkt);
}

#define SUBC_ID_DISABLE_MCU_POLLING (0x25)
#define SUBC_REPORT_DISABLE_MCU_POLLING_LENGTH (3)
void ns_subcommand_disable_mcu_polling(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    //todo
    //ans with ack always for now;
    _ns_subcommand_set_ack(cmd->subcommand_id,DEFAULT_ACK);
    pkt.data.subcommand_report.subcommand_data[0]=0x00;
    pkt.len=SUBC_REPORT_DISABLE_MCU_POLLING_LENGTH;
    ns_send_report(&pkt);
}

#define SUBC_ID_ATTACHMENT_WRITE (0x28)
//#define SUBC_REPORT_ATTACHMENT_WRITE_LENGTH (3)
void ns_subcommand_attachmend_write(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    //todo
    //ans with ack always for now;
    //Does the same job with OUTPUT report 0x12.???
    _ns_subcommand_set_ack(cmd->subcommand_id,DEFAULT_ACK);
    pkt.len=SUBC_REPORT_BASIC_LENGTH;
    ns_send_report(&pkt);
}

#define SUBC_ID_ATTACHMENT_READ (0x29)
#define SUBC_REPORT_ATTACHMENT_READ_LENGTH (36)
void ns_subcommand_attachmend_read(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    //todo
    //ans with ack always for now;
    //where is the data come from??
    _ns_subcommand_set_ack(cmd->subcommand_id,DEFAULT_ACK);
    pkt.len=SUBC_REPORT_ATTACHMENT_READ_LENGTH;
    ns_send_report(&pkt);
}

#define SUBC_ID_SET_ATTACHMENT_STATE (0x2A)
//Replies always with ACK x00 x2A.
//x00 as an ACK here is a bug. Devs forgot to add an ACK reply.
//still this now??? todo : check if its correct
void ns_subcommand_attachment_state(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    uint8_t state=cmd->subcommand_data[0];//gpio pin value
    _ns_subcommand_set_ack(cmd->subcommand_id,0x00);
    pkt.len=SUBC_REPORT_BASIC_LENGTH;
    ns_send_report(&pkt);
}

#define SUBC_ID_GET_ATTACHMENT_INFO (0x2B)
#define SUBC_REPORT_GET_ATTACHMENT_INFO_LENGTH (22)
void ns_subcommand_get_attachment_info(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    //todo : get attachment info. how to get?
    _ns_subcommand_set_ack(cmd->subcommand_id,DEFAULT_ACK);
    pkt.len=SUBC_REPORT_GET_ATTACHMENT_INFO_LENGTH;
    ns_send_report(&pkt);
}


uint8_t indicate_led_status;
#define SUBC_ID_SET_INDICATE_LED (0x30)
void ns_subcommand_set_indicate_led(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    indicate_led_status=cmd->subcommand_data[0];
    //0x00 disable 0x01 enable
    _ns_subcommand_set_ack(cmd->subcommand_id,DEFAULT_ACK);
    pkt.len=SUBC_REPORT_BASIC_LENGTH;
    ns_send_report(&pkt);

    //todo write a FSM for pairing sequence
    //is_paired=1;
    connection_state.usb_paired=1;
}

#define SUBC_ID_GET_INDICATE_LED (0x31)
#define SUBC_REPORT_GET_INDICATE_LED_LENGTH (3)
void ns_subcommand_get_indicate_led(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    _ns_subcommand_set_ack(cmd->subcommand_id,0xB0);
    pkt.data.subcommand_report.subcommand_data[0]=indicate_led_status;
    pkt.len=SUBC_REPORT_GET_INDICATE_LED_LENGTH;
    ns_send_report(&pkt);
}

#define SUBC_ID_SET_HOME_LED (0x38)
void ns_subcommand_set_home_led(NS_SUBCOMMAND_CB_PARAM){
    //really complecate,do nothing as we dont really have such thing;
    //no reply needed seems;
}


typedef struct{
    uint8_t state;
    struct{
        uint8_t gyro_sensitivity;
        uint8_t acc_sensitivity;
        uint8_t gyro_rate;
        uint8_t acc_rate;
    }imu_sensor_conf;
}_imu_conf;
_imu_conf imu_conf;
#define SUBC_ID_SET_IMU_STATE (0x40)
void ns_subcommand_set_imu_state(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    uint8_t state=cmd->subcommand_data[0];
    printf("set imu state:%d\r\n",state);
    imu_conf.state=state;
    imu_mode=state;
    //0x00 disable 0x01 enable
    _ns_subcommand_set_ack(cmd->subcommand_id,NS_SUBCOMMAND_STATUS_ACK);
    //_ns_subcommand_set_ack(cmd->subcommand_id, 0x90);
    //pkt.data.subcommand_report.subcommand_data[0]=state;
    pkt.len=SUBC_REPORT_BASIC_LENGTH;
    ns_send_report(&pkt);
}

#define SUBC_ID_SET_IMU_CONF (0x41)
#define SUBC_REPORT_SET_IMU_CONF_LENGTH (3)
void ns_subcommand_set_imu_conf(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    imu_mode=cmd->subcommand_data[0];
    printf("set imu mode to:%d\r\n",imu_mode);
    if(!imu_conf.state){
        imu_conf.state=0x01;
        //todo : set imu conf to default
        _ns_subcommand_set_ack(cmd->subcommand_id,DEFAULT_ACK);
        pkt.data.subcommand_report.subcommand_data[0]=0x40;
        pkt.data.subcommand_report.subcommand_data[1]=0x01;
        pkt.len=SUBC_REPORT_SET_IMU_CONF_LENGTH;
        ns_send_report(&pkt);
        return;
    }
    memcpy(&imu_conf.imu_sensor_conf,cmd->subcommand_data,sizeof(imu_conf.imu_sensor_conf));
    _ns_subcommand_set_ack(cmd->subcommand_id,NS_SUBCOMMAND_STATUS_ACK);
    pkt.len=SUBC_REPORT_BASIC_LENGTH;
    ns_send_report(&pkt);
}

#define SUBC_ID_SET_IMU_REGISTER (0x42)
void ns_subcommand_set_imu_register(NS_SUBCOMMAND_CB_PARAM){
    uint8_t addr=cmd->subcommand_data[0];
    uint8_t value=cmd->subcommand_data[2];
    imu_set_reg(addr,value,0xff);
    printf("imu set reg addr:%d value:%d\r\n",addr,value);
    //exit(0);
}

#define SUBC_ID_READ_IMU_REGISTER (0x43)
#define SUBC_REPORT_READ_IMU_REGISTER_BASIC_LENGTH (4)
void ns_subcommand_read_imu_register(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    uint8_t addr=cmd->subcommand_data[0];
    uint8_t cnt=cmd->subcommand_data[1];
    if(cnt>0x20){
        //ESP_LOGE("","%s error read imu reg too many cnt:%d",__func__,cnt);
        cnt=0x20;
    }
    _ns_subcommand_set_ack(cmd->subcommand_id,0xC0);
    pkt.data.subcommand_report.subcommand_data[0]=addr;
    pkt.data.subcommand_report.subcommand_data[1]=cnt;
    printf("imu read reg addr:%d cnt:%d\r\n",addr,cnt);
    i2c_read_continuous(addr, pkt.data.subcommand_report.subcommand_data+2, cnt);
    pkt.len=SUBC_REPORT_READ_IMU_REGISTER_BASIC_LENGTH+cnt;
    ns_send_report(&pkt);
}

#define SUBC_ID_SET_RUMBLE_STATE (0x48)
void ns_subcommand_set_rumble_state(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    rumble_state=cmd->subcommand_data[0];
    //hd_rumble_set_status(rumble_state);
    //printf("set rumble state %d\r\n",rumble_state);
    //0x00 disable 0x01 enable
    _ns_subcommand_set_ack(cmd->subcommand_id,DEFAULT_ACK);
    pkt.len=SUBC_REPORT_BASIC_LENGTH;
    ns_send_report(&pkt);
}

#define SUBC_ID_GET_BATTERY_VOLTAGE (0x50)
#define SUBC_REPORT_GET_BATTERY_VOLTAGE (4)
void ns_subcommand_get_battery_voltage(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    _ns_subcommand_set_ack(cmd->subcommand_id,0xD0);
    *(uint16_t*)pkt.data.subcommand_report.subcommand_data=0x0618;
    pkt.len=SUBC_REPORT_GET_BATTERY_VOLTAGE;
    ns_send_report(&pkt);
}

#define SUBC_ID_WRITE_CHARGE_SETTING (0x51)
void ns_subcommand_write_charge_setting(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    _ns_subcommand_set_ack(cmd->subcommand_id,DEFAULT_ACK);
    pkt.len=SUBC_REPORT_BASIC_LENGTH;
    ns_send_report(&pkt);
}

#define SUBC_ID_READ_CHARGE_SETTING (0x52)
#define SUBC_REPORT_READ_CHARGE_SETTING_LENGTH (3)
void ns_subcommand_read_charge_setting(NS_SUBCOMMAND_CB_PARAM){
    pkt_clr();
    _ns_subcommand_set_ack(cmd->subcommand_id,0xD1);
    pkt.data.subcommand_report.subcommand_data[0]=0x14;
    pkt.len=SUBC_REPORT_READ_CHARGE_SETTING_LENGTH;
    ns_send_report(&pkt);
}

//todo:more subcommand to implement,but doesnt seems to be necessary



void ns_subcommand_callback_init(){
    ns_register_subcommand_cb(SUBC_ID_DEFAULT,ns_subcommand_default);
    ns_register_subcommand_cb(SUBC_ID_PAIR_BY_WIRE,ns_subcommand_pair_by_wire);
    ns_register_subcommand_cb(SUBC_ID_GET_DEVICE_INFO,ns_subcommand_get_device_info);
    ns_register_subcommand_cb(SUBC_ID_SET_INPUT_MODE,ns_subcommand_set_input_mode);
    ns_register_subcommand_cb(SUBC_ID_TRIGGER_BUTTONS_ELAPSED_TIME,ns_subcommand_trigger_buttons_elapsed_time);
    ns_register_subcommand_cb(SUBC_ID_GET_PAGE_LIST_STATE,ns_subcommand_get_page_list_state);
    ns_register_subcommand_cb(SUBC_ID_SET_HCI_STATE,ns_subcommand_set_hci_state);
    ns_register_subcommand_cb(SUBC_ID_RESET_PAIRING_INFO,ns_subcommand_reset_pairing_info);
    ns_register_subcommand_cb(SUBC_ID_SET_SHIPPING_MODE,ns_subcommand_set_shipping_mode);
    ns_register_subcommand_cb(SUBC_ID_SPI_READ,ns_subcommand_spi_read);
    ns_register_subcommand_cb(SUBC_ID_SPI_WRITE,ns_subcommand_spi_write);
    ns_register_subcommand_cb(SUBC_ID_SPI_ERASE,ns_subcommand_spi_erase);
    ns_register_subcommand_cb(SUBC_ID_RESET_MCU,ns_subcommand_reset_mcu);
    ns_register_subcommand_cb(SUBC_ID_SET_MCU_CONF,ns_subcommand_set_mcu_conf);
    ns_register_subcommand_cb(SUBC_ID_SET_MCU_STATE,ns_subcommand_set_mcu_state);
    ns_register_subcommand_cb(SUBC_ID_ENABLE_MCU_POLLING,ns_subcommand_enable_mcu_polling);
    ns_register_subcommand_cb(SUBC_ID_DISABLE_MCU_POLLING,ns_subcommand_disable_mcu_polling);
    ns_register_subcommand_cb(SUBC_ID_ATTACHMENT_WRITE,ns_subcommand_attachmend_write);
    ns_register_subcommand_cb(SUBC_ID_ATTACHMENT_READ,ns_subcommand_attachmend_read);
    ns_register_subcommand_cb(SUBC_ID_SET_ATTACHMENT_STATE,ns_subcommand_attachment_state);
    ns_register_subcommand_cb(SUBC_ID_GET_ATTACHMENT_INFO,ns_subcommand_get_attachment_info);
    ns_register_subcommand_cb(SUBC_ID_SET_INDICATE_LED,ns_subcommand_set_indicate_led);
    ns_register_subcommand_cb(SUBC_ID_GET_INDICATE_LED,ns_subcommand_get_indicate_led);
    ns_register_subcommand_cb(SUBC_ID_SET_HOME_LED,ns_subcommand_set_home_led);
    ns_register_subcommand_cb(SUBC_ID_SET_IMU_STATE,ns_subcommand_set_imu_state);
    ns_register_subcommand_cb(SUBC_ID_SET_IMU_CONF,ns_subcommand_set_imu_conf);
    ns_register_subcommand_cb(SUBC_ID_SET_IMU_REGISTER,ns_subcommand_set_imu_register);
    ns_register_subcommand_cb(SUBC_ID_READ_IMU_REGISTER,ns_subcommand_read_imu_register);
    ns_register_subcommand_cb(SUBC_ID_SET_RUMBLE_STATE,ns_subcommand_set_rumble_state);
    ns_register_subcommand_cb(SUBC_ID_GET_BATTERY_VOLTAGE,ns_subcommand_get_battery_voltage);
    ns_register_subcommand_cb(SUBC_ID_WRITE_CHARGE_SETTING,ns_subcommand_write_charge_setting);
    ns_register_subcommand_cb(SUBC_ID_READ_CHARGE_SETTING,ns_subcommand_read_charge_setting);
}

void ns_register_subcommand_cb(int id,void (*cb)(cmd_subcommand*,uint8_t)){//SET CB NULL TO UNREG
        if(id>=0&&id<NS_SUBCOMMAND_ID_MAX_VLAUE)
                ns_cmd_subcommand_cb_tb[id]=cb;
}
