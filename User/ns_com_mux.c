/*
 * ns_com_mux.c
 *
 *  Created on: 2024Äê10ÔÂ15ÈÕ
 *      Author: Reed
 */
#include "ns_com_mux.h"
#include "ring_buffer.h"
#include "ch32v10x_usbfs_device.h"
#include "usbd_compatibility_hid.h"
#include "rumble.h"
#include "tick.h"
#include "hd_rumble2.h"
#include "conf.h"
#include "i2c.h"
#include "imu.h"
#include "gpio_adc.h"
#include "global_api.h"
#include "spi.h"
#include "watchdog.h"

struct __connection_state connection_state;
void (*ns_hid_packet_dispatch_tb[NS_PACKET_TYPE_MAX_VALUE])(cmd_packet*);
uint8_t is_esp32_enabled=0;
uint8_t bd_addr_default[BD_ADDR_LEN]={0x57, 0x30 ,0xea, 0x8a, 0xbb, 0x7c};
//hd_rumble_frame data;
uint8_t is_rumble_start;
uint32_t rts_cnt=0,rts_tcnt=1;
void ns_rumble_handler(cmd_packet* pkt){
    if(!is_rumble_start)return;
    decode_hd_rumble_multiformat_high_acc(&pkt->cmd->rumble_data_left,&pkt->cmd->rumble_data_right);
}
void ns_cmd_subcommand_dispatcher(cmd_packet* pkt){
        cmd_subcommand* cmd=(cmd_subcommand*)pkt->cmd;
        pkt->len-=NS_SUBCOMMAND_CMD_HEADER_LENGTH+1;
        //ESP_LOGI("SUBC ","dispatch subcmd:0x%02x",cmd->subcommand_id);
        ////printf("subc dispatch id:0x%02x,len:%d,tick:%d\r\n",cmd->subcommand_id,pkt->len,Get_Systick_MS());
        if(ns_cmd_subcommand_cb_tb[cmd->subcommand_id]){
            ////printf("dispatched %d\r\n",cmd->subcommand_id);
            ns_cmd_subcommand_cb_tb[cmd->subcommand_id](cmd,pkt->len);
        }
        else if(ns_cmd_subcommand_cb_tb[0]){
             //printf("error fall back to default ack");
             ns_cmd_subcommand_cb_tb[0](cmd,pkt->len);
        }
        else{
                //ESP_LOGE("","%s error subcommand callback not exist",__func__);
        }
        ns_rumble_handler(pkt);
}
static uint8_t usb_handshake_buf[RING_BUFFER_MAX_PKG_SIZE];
static uint8_t usb_hs01[]={0x81,0x01,0x00,0x03};
void ns_mux_usb_handshake_handler(cmd_packet* pkt){//0x80
    ////printf("usb handeshake len:%d\r\n",pkt->len);
    if(pkt->len<2||!pkt->data)return;
    uint8_t typ=pkt->data[1];
    //printf("usb handshake typ:%x\r\n",typ);
    switch(typ){
    case 0x01:
        memcpy(usb_handshake_buf,usb_hs01,4);
        memcpy(usb_handshake_buf+4,connection_state.bd_addr,BD_ADDR_LEN);
        ring_buffer_push(&ns_usb_send_rb, usb_handshake_buf, 10, 0x01);
        ////printf("rb pushed size:%d\r\n",ns_usb_send_rb.size);
        break;
    case 0x02:
        usb_handshake_buf[0]=0x81;
        usb_handshake_buf[1]=0x02;
        ring_buffer_push(&ns_usb_send_rb, usb_handshake_buf, 2, 0x01);
        break;
    case 0x03:
        usb_handshake_buf[0]=0x81;
        usb_handshake_buf[1]=0x03;
        ring_buffer_push(&ns_usb_send_rb, usb_handshake_buf, 2, 0x01);
        break;
    case 0x04:
        usb_handshake_buf[0]=0x81;
        usb_handshake_buf[1]=0x04;
        ring_buffer_push(&ns_usb_send_rb, usb_handshake_buf, 2, 0x01);
        break;
    case 0x05:
        usb_handshake_buf[0]=0x81;
        usb_handshake_buf[1]=0x05;
        ring_buffer_push(&ns_usb_send_rb, usb_handshake_buf, 2, 0x01);
        break;
    case 0x06:
        usb_handshake_buf[0]=0x81;
        usb_handshake_buf[1]=0x06;
        ring_buffer_push(&ns_usb_send_rb, usb_handshake_buf, 2, 0x01);
        break;
    case 0x91:
        break;
    case 0x92:
        break;
    default:
        break;
    }
}
static uint8_t fw_buf_raw[RING_BUFFER_MAX_PKG_SIZE];
static uint8_t* fw_buf=fw_buf_raw+2;
//todo:custom subcommand to config controller profile ,start with 0xf

#define FW_MAX_PAYLOAD_LENGTH (61)
#define FW_PKG_SIZE (62)
uint8_t fw_snd_pkt(uint8_t id,uint8_t len){
    fw_buf_raw[0]=0xFE;
    fw_buf_raw[1]=id;
    return hid_send_full64byte_report(fw_buf_raw,len+2);
}
#define FW_SUBC_ID_READ_SETTING (0x01)
void fw_subcommand_read_setting(cmd_packet* pkt){
    //printf("fw read %d\r\n",pkt->data[0]);
    //fw_buf[0]=FW_SUBC_ID_READ_SETTING;
    uint8_t i=0;
    for(;FW_MAX_PAYLOAD_LENGTH*(i+1)<sizeof(user_config);i++){
        fw_buf[0]=i;//offset
        memcpy(fw_buf+1,((uint8_t*)&user_config)+i*FW_MAX_PAYLOAD_LENGTH,FW_MAX_PAYLOAD_LENGTH);//payload
        //hid_send_full64byte_report(fw_buf, FW_PKG_SIZE);
        fw_snd_pkt(FW_SUBC_ID_READ_SETTING,FW_PKG_SIZE);
    }
    if(FW_MAX_PAYLOAD_LENGTH*i<sizeof(user_config)){
        memset(fw_buf+1,0,FW_MAX_PAYLOAD_LENGTH);
        fw_buf[0]=i;//offset
        memcpy(fw_buf+1,((uint8_t*)&user_config)+i*FW_MAX_PAYLOAD_LENGTH,sizeof(user_config)-FW_MAX_PAYLOAD_LENGTH*i);
        //hid_send_full64byte_report(fw_buf, sizeof(user_config)-FW_MAX_PAYLOAD_LENGTH*i+2);
        fw_snd_pkt(FW_SUBC_ID_READ_SETTING,sizeof(user_config)-FW_MAX_PAYLOAD_LENGTH*i+1);
        ++i;
    }
    fw_buf[0]=0xff;
    fw_buf[1]=i;
    fw_snd_pkt(FW_SUBC_ID_READ_SETTING,2);
    //hid_send_full64byte_report(fw_buf, 3);
}

#define FW_SUBC_ID_WRITE_SETTING (0x02)
static uint8_t fw_red_cnt=0;
void fw_subcommand_write_setting(cmd_packet* pkt){
    //printf("fw write\r\n");
    //fw_buf[0]=FW_SUBC_ID_WRITE_SETTING;
    uint8_t offset=pkt->data[1];
    if(offset==0xff){//confirm
        if(fw_red_cnt==pkt->data[2])
        {
            //printf("fw write success");
            if(pkt->data[3]==1)
                custom_conf_write();
            conf_flush();
            fw_buf[0]=1;
        }
        else {
            //printf("fw write fail,data incomplate");
            fw_buf[0]=0;
        }
        fw_buf[1]=pkt->data[3];
        fw_red_cnt=0;
        fw_snd_pkt(FW_SUBC_ID_WRITE_SETTING,2);
        //hid_send_full64byte_report(fw_buf,3);
        //hid_send_full64byte_report(fw_buf,4);
    }
    else {//dataset
        memcpy(((uint8_t*)&user_config)+offset*FW_MAX_PAYLOAD_LENGTH,pkt->data+2,
                ((FW_MAX_PAYLOAD_LENGTH<=(sizeof(user_config)-offset*FW_MAX_PAYLOAD_LENGTH))?
                FW_MAX_PAYLOAD_LENGTH:(sizeof(user_config)-offset*FW_MAX_PAYLOAD_LENGTH)));
        ++fw_red_cnt;
    }
    //memcpy(&user_config,cmd->subcommand_data,sizeof(user_config));
}
#define FW_SUBC_ID_READ_EMULATE_ROM (0x03)
void fw_subcommand_read_emulate_rom(cmd_packet* pkt){
    uint32_t addr=fetch_uint32(&pkt->data[1]);
    memcpy(fw_buf,pkt->data+1,5);
    conf_read(addr, fw_buf+5, pkt->data[5]);
    //hid_send_full64byte_report(fw_buf, pkt->data[5]+2);
    fw_snd_pkt(FW_SUBC_ID_READ_EMULATE_ROM, pkt->data[5]+5);
}
#define FW_SUBC_ID_WRITE_EMULATE_ROM (0x04)
void fw_subcommand_write_emulate_rom(cmd_packet* pkt){
    uint32_t addr=fetch_uint32(&pkt->data[1]);
    fw_buf[0]=conf_write(addr, &pkt->data[6], pkt->data[5]);
    //hid_send_full64byte_report(fw_buf, 2);
    fw_snd_pkt(FW_SUBC_ID_WRITE_EMULATE_ROM, 1);
}
#define CALIBRATE_MAX_RETRY (5)
#define CALIBRATE_SAMPLE_CNT (20)
static int32_t calibrate_imu_raw_buf[6];
uint8_t _calibrate_imu_raw(uint32_t sample_cnt){
    uint8_t res=0,retry=CALIBRATE_MAX_RETRY;
    memset(calibrate_imu_raw_buf,0,sizeof(calibrate_imu_raw_buf));
    for(int i=0;i<sample_cnt;++i){
        retry=CALIBRATE_MAX_RETRY;
        Delay_Ms(5);
        do{
            res=imu_read();
            Delay_Ms(5);
        }while(res&&retry--);
        if(res)
            break;
        for(int j=0;j<6;++j){
            calibrate_imu_raw_buf[j]+=imu_raw_buf[j];
        }
    }
    calibrate_imu_raw_buf[0]*=imu_ratio_xf;
    calibrate_imu_raw_buf[1]*=imu_ratio_yf;
    calibrate_imu_raw_buf[2]*=imu_ratio_zf;
    return res;
}
uint8_t calibrate_imu(uint8_t cnt){
    uint8_t res=_calibrate_imu_raw(cnt);
    //calibrate_imu_raw_buf[5]-=CALIBRATE_SAMPLE_CNT*1000/0.244f;//but left 1g to ground
    if(!res){
        factory_configuration.SixAxisSensorCalibrationValue.Accelerometer0OffsetX=-
                factory_configuration.Model.SixAxisSensorModelValue.SixAxisHorizontalOffsetX+(1.0f*calibrate_imu_raw_buf[3]/CALIBRATE_SAMPLE_CNT+0.5f);//remove x shift
        factory_configuration.SixAxisSensorCalibrationValue.Accelerometer0OffsetY=-
                factory_configuration.Model.SixAxisSensorModelValue.SixAxisHorizontalOffsetY+(1.0f*calibrate_imu_raw_buf[4]/CALIBRATE_SAMPLE_CNT+0.5f);//remove y shift
        factory_configuration.SixAxisSensorCalibrationValue.Accelerometer0OffsetZ=-
                factory_configuration.Model.SixAxisSensorModelValue.SixAxisHorizontalOffsetZ+(1.0f*calibrate_imu_raw_buf[5]/CALIBRATE_SAMPLE_CNT+0.5f);//remove z shift
        factory_configuration.SixAxisSensorCalibrationValue.Gyroscope0OffsetX=+(1.0f*calibrate_imu_raw_buf[0]/CALIBRATE_SAMPLE_CNT+0.5f);
        factory_configuration.SixAxisSensorCalibrationValue.Gyroscope0OffsetY=+(1.0f*calibrate_imu_raw_buf[1]/CALIBRATE_SAMPLE_CNT+0.5f);
        factory_configuration.SixAxisSensorCalibrationValue.Gyroscope0OffsetZ=+(1.0f*calibrate_imu_raw_buf[2]/CALIBRATE_SAMPLE_CNT+0.5f);
        fac_conf_write();
    }
    return res;
}
#define FW_SUBC_ID_CALIBRATE_IMU (0x05)
void fw_subcommand_calibrate_imu(cmd_packet* pkt)
{
    //fw_buf[0]=FW_SUBC_ID_CALIBRATE_IMU;
    fw_buf[0]=calibrate_imu(CALIBRATE_SAMPLE_CNT);
    fw_buf[1]=i2c_error_code;
    fw_snd_pkt(FW_SUBC_ID_CALIBRATE_IMU, 2);
    //hid_send_full64byte_report(fw_buf,2);
}

#define FW_SUBC_ID_CALIBRATE_JS_CENTER (0x06)
//abandon
void fw_subcommand_calibrate_js_center(cmd_packet* pkt){
    /*uint8_t id=pkt->data[1];
    if(id>=2){
        fw_buf[0]=1;
        fw_snd_pkt(FW_SUBC_ID_CALIBRATE_JS_CENTER, 1);
        return;
    }
    if(id){
        factory_configuration.JoystickCalibrationValue.AnalogStickRightFactoryCalibrationValue.AnalogStickCalX0=adc_data[2];
        factory_configuration.JoystickCalibrationValue.AnalogStickRightFactoryCalibrationValue.AnalogStickCalY0=adc_data[3];
    }else{
        factory_configuration.JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue.AnalogStickCalX0=adc_data[0];
        factory_configuration.JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue.AnalogStickCalY0=adc_data[1];
    }
    //fac_conf_write();
    fw_buf[0]=0;
    fw_snd_pkt(FW_SUBC_ID_CALIBRATE_JS_CENTER, 1);*/
}
#define FW_SUBC_ID_CALIBRATE_JS_OFFSET (0x07)
void fw_subcommand_calibrate_js_offset(cmd_packet* pkt){
    /*uint8_t id=pkt->data[1];
    if(id>3){
        fw_buf[0]=1;
        fw_snd_pkt(FW_SUBC_ID_CALIBRATE_JS_OFFSET, 1);
        return;
    }
    user_config.joystick_offset[id].value=2048-(int32_t)adc_data[id];
    fw_buf[1]=0;
    fw_buf[2]=id;
    fw_buf[3]=user_config.joystick_offset[id].value&0xff;
    fw_buf[4]=user_config.joystick_offset[id].value>>8;
    fw_snd_pkt(FW_SUBC_ID_CALIBRATE_JS_OFFSET, 1);*/
    for(int i=0;i<4;++i)
        user_calibration.internal_center[i]=adc_data[i];
    user_calibration.nonexist=0;
    fw_buf[0]=conf_write(0x8000, 0, 0);
    fw_snd_pkt(FW_SUBC_ID_CALIBRATE_JS_OFFSET, 1);
}
#define FW_SUBC_ID_GET_STATUS (0xFD)
void fw_subcommand_get_status(cmd_packet* pkt){
    fw_buf[0]=i2c_read_byte(IMU_ID,fw_buf+1);
    if(!rts_tcnt)rts_tcnt=1;
    fw_buf[2]=rts_cnt/rts_tcnt;
    memcpy(fw_buf+3,&imu_read_cnt,4);
    memcpy(fw_buf+7,&imu_read_fail_cnt,4);
    fw_snd_pkt(FW_SUBC_ID_GET_STATUS, 11);
}
#define FW_SUBC_ID_REBOOT (0xFE)
void fw_subcommand_reboot(cmd_packet* pkt){
    flush_rgb(DISABLE);
    Delay_Ms(10);
    NVIC_SystemReset();
}
#define FW_SUBC_ID_GET_VERSION (0xFF)
void fw_subcommand_get_version(cmd_packet* pkt){
    //fw_buf[0]=FW_SUBC_ID_GET_VERSION;
    fw_buf[0]=FW_VERSION_HIGH;
    fw_buf[1]=FW_VERSION_LOW;
    fw_snd_pkt(FW_SUBC_ID_GET_VERSION, 2);
    //hid_send_full64byte_report(fw_buf,3);
}
void fw_subcommand_dispatcher(cmd_packet* pkt){
    pkt->data++;
    if(pkt->len<=1)return;//no payload.so drop it incase something goes wrong.
    pkt->len-=1;
    //hid header already dropped
    switch(pkt->data[0])
    {
    case FW_SUBC_ID_READ_SETTING:
        fw_subcommand_read_setting(pkt);
        break;
    case FW_SUBC_ID_WRITE_SETTING:
        fw_subcommand_write_setting(pkt);
        break;
    case FW_SUBC_ID_READ_EMULATE_ROM:
        fw_subcommand_read_emulate_rom(pkt);
        break;
    case FW_SUBC_ID_WRITE_EMULATE_ROM:
        fw_subcommand_write_emulate_rom(pkt);
        break;
    case FW_SUBC_ID_CALIBRATE_IMU:
        fw_subcommand_calibrate_imu(pkt);
        break;
    case FW_SUBC_ID_GET_VERSION:
        fw_subcommand_get_version(pkt);
        break;
    case FW_SUBC_ID_CALIBRATE_JS_CENTER:
        fw_subcommand_calibrate_js_center(pkt);
        break;
    case FW_SUBC_ID_REBOOT:
        fw_subcommand_reboot(pkt);
        break;
    case FW_SUBC_ID_GET_STATUS:
        fw_subcommand_get_status(pkt);
        break;
    case FW_SUBC_ID_CALIBRATE_JS_OFFSET:
        fw_subcommand_calibrate_js_offset(pkt);
        break;
    default:
        break;
    }
}
void ns_mux_init(){
    /*ns_hid_register_packet_dispatch(0x01,ns_cmd_subcommand_dispatcher);
    ns_hid_register_packet_dispatch(0x80,ns_mux_usb_handshake_handler);
    ns_hid_register_packet_dispatch(0x10, ns_rumble_handler);
    ns_hid_register_packet_dispatch(FW_SUBC_ID_READ_SETTING,fw_subcommand_read_setting);
    ns_hid_register_packet_dispatch(FW_SUBC_ID_WRITE_SETTING,fw_subcommand_write_setting);
    ns_hid_register_packet_dispatch(FW_SUBC_ID_GET_VERSION,fw_subcommand_get_version);
    ns_hid_register_packet_dispatch(FW_SUBC_ID_CALIBRATE_IMU, fw_subcommand_calibrate_imu);*/
    ns_subcommand_callback_init();
}
void hid_dispatch(cmd_packet* pkt)
{
    switch(*(pkt->data)){
    case 0x01:
        ns_cmd_subcommand_dispatcher(pkt);
        break;
    case 0x10:
        ns_rumble_handler(pkt);
        break;
    case 0x80:
        ns_mux_usb_handshake_handler(pkt);
        break;
    case 0xFE://fw subcommand
        fw_subcommand_dispatcher(pkt);
        break;
    default:
        //printf("dispatch fail typ:0x%02x\r\n",*(pkt->data));
        break;
    }
}
uint8_t ns_send_report(report_packet* rpt){
   return ring_buffer_push(&ns_usb_send_rb, (uint8_t*)&(rpt->data), rpt->len,0x00);
}
uint8_t ns_send_full64byte_report(report_packet* rpt){
    return ring_buffer_push(&ns_usb_send_rb, (uint8_t*)&(rpt->data), rpt->len,0x02);
}
uint8_t hid_send_full64byte_report(uint8_t* buf,uint8_t len){
    return ring_buffer_push(&ns_usb_send_rb, buf, len,0x03);
}

void ns_hid_register_packet_dispatch(int typ,void (*handler)(cmd_packet*)){
        if(typ>=0&&typ<NS_PACKET_TYPE_MAX_VALUE)
                ns_hid_packet_dispatch_tb[typ]=handler;
}

