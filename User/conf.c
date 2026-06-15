/*
 * conf.c
 *
 *  Created on: 2024Äê11ÔÂ26ÈÕ
 *      Author: Reed
 */

#include "debug.h"
#include "conf.h"
#include "flash.h"
#include "usb_desc.h"
#include "spi.h"
#include "imu.h"
#include "gpio_digit.h"
#include "hd_rumble2.h"
#include "hd_rumble_high_accuracy.h"
#include "affine.h"
#include <string.h>
const uint32_t FW_VERSION=(0x00020000);
const uint32_t FW_SUB_VERSION=0x0;//SmashProFw_V1.2.0.0.hex
//#pragma pack(push,4)
factory_configuration_data factory_configuration;
user_calibration_data user_calibration;
user_config_data config;
//#pragma pack(pop)
uint32_t joystick_snapback_deadzone_sq[2];
uint32_t button_active_mask;
//static factory_configuration_flash_pack* fac;
#define JOYSTICK_RANGE_FACTOR_LEFT (0.6f)
#define JOYSTICK_RANGE_FACTOR_RIGHT (0.6f)
void set_hd_rumble_range(){
    if(PRO_IS(PRO_200)){
        hd_rumble_cvr_range=650;
        hd_rumble_cvr_max_offset=720;
    }else{
        hd_rumble_cvr_range=540;
        hd_rumble_cvr_max_offset=720;
    }
    hd_rumble_cvr_max=HD_RUMBLE_TIM_PERIOD_MID+hd_rumble_cvr_max_offset;
    hd_rumble_cvr_min=HD_RUMBLE_TIM_PERIOD_MID-hd_rumble_cvr_max_offset;
}
void conf_init()
{
    memset(&factory_configuration,-1,sizeof(factory_configuration));
    fac_conf_read();
    if(factory_configuration.nonexist){
        factory_configuration.IdentificationCode[0]=0x80;//no sn
        factory_configuration.DeviceType=0x03;
        factory_configuration.BoardRevision=0xA0;
        factory_configuration.FormatVersion=0x1;//use custom color/
        factory_configuration.JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue.AnalogStickCalXPositive=
                2048*JOYSTICK_RANGE_FACTOR_LEFT;
        factory_configuration.JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue.AnalogStickCalYPositive=
                2048*JOYSTICK_RANGE_FACTOR_LEFT;
        factory_configuration.JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue.AnalogStickCalXNegative=
                2048*JOYSTICK_RANGE_FACTOR_LEFT;
        factory_configuration.JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue.AnalogStickCalYNegative=
                2048*JOYSTICK_RANGE_FACTOR_LEFT;
        factory_configuration.JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue.AnalogStickCalX0=
                factory_configuration.JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue.AnalogStickCalY0=2048;
        /*factory_configuration.JoystickCalibrationValue.AnalogStickRightFactoryCalibrationValue=
                factory_configuration.JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue;*/
        factory_configuration.JoystickCalibrationValue.AnalogStickRightFactoryCalibrationValue.AnalogStickCalXPositive=
                2048*JOYSTICK_RANGE_FACTOR_RIGHT;
        factory_configuration.JoystickCalibrationValue.AnalogStickRightFactoryCalibrationValue.AnalogStickCalYPositive=
                2048*JOYSTICK_RANGE_FACTOR_RIGHT;
        factory_configuration.JoystickCalibrationValue.AnalogStickRightFactoryCalibrationValue.AnalogStickCalXNegative=
                2048*JOYSTICK_RANGE_FACTOR_RIGHT;
        factory_configuration.JoystickCalibrationValue.AnalogStickRightFactoryCalibrationValue.AnalogStickCalYNegative=
                2048*JOYSTICK_RANGE_FACTOR_RIGHT;
        factory_configuration.JoystickCalibrationValue.AnalogStickRightFactoryCalibrationValue.AnalogStickCalX0=
                factory_configuration.JoystickCalibrationValue.AnalogStickRightFactoryCalibrationValue.AnalogStickCalY0=2048;

        /*factory_configuration.Design.ControllerColor.MainColor.r=0x32;
        factory_configuration.Design.ControllerColor.MainColor.g=0x31;
        factory_configuration.Design.ControllerColor.MainColor.b=0x32;*/
        factory_configuration.Design.ControllerColor.MainColor.r=0x00;
        factory_configuration.Design.ControllerColor.MainColor.g=0x00;
        factory_configuration.Design.ControllerColor.MainColor.b=0x00;
        factory_configuration.Design.ControllerColor.SubColor.r=0xff;
        factory_configuration.Design.ControllerColor.SubColor.g=0xff;
        factory_configuration.Design.ControllerColor.SubColor.b=0xff;
        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelNoise=0x00F;
        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelTypicalStroke=0x613;
        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelCenterDeadZoneSize=0x00F;//0x0AE;
        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelCircuitDeadZoneScale=0xD99;
        //factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelMinimumStrokeXPositive=0x4D4;
        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelMinimumStrokeXPositive=0x541;
        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelMinimumStrokeYPositive=0x541;
        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelMinimumStrokeXNegative=0x541;
        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelMinimumStrokeYNegative=0x541;
        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelCenterRangeXPositive=0x9C7;
        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelCenterRangeYPositive=0x9C7;
        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelCenterRangeXNegative=0x633;
        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelCenterRangeYNegative=0x633;
        factory_configuration.AnalogStickSubModelValue=factory_configuration.Model.AnalogStickMainModelValue;
        //5B008BFF96010040004000401D00BEFFEEFF3B343B343B34
        factory_configuration.SixAxisSensorCalibrationValue.Accelerometer0OffsetX=0x005B;
        factory_configuration.SixAxisSensorCalibrationValue.Accelerometer0OffsetY=0xFF8B;
        factory_configuration.SixAxisSensorCalibrationValue.Accelerometer0OffsetZ=0X0196;
        factory_configuration.SixAxisSensorCalibrationValue.Accelerometer1gScaleX=0X4000;
        factory_configuration.SixAxisSensorCalibrationValue.Accelerometer1gScaleY=0X4000;
        factory_configuration.SixAxisSensorCalibrationValue.Accelerometer1gScaleZ=0X4000;
        /*factory_configuration.SixAxisSensorCalibrationValue.Gyroscope0OffsetX=0X0001;
        factory_configuration.SixAxisSensorCalibrationValue.Gyroscope0OffsetY=0X0001;
        factory_configuration.SixAxisSensorCalibrationValue.Gyroscope0OffsetZ=0X0001;*/
        factory_configuration.SixAxisSensorCalibrationValue.Gyroscope0OffsetX=0X001D;
        factory_configuration.SixAxisSensorCalibrationValue.Gyroscope0OffsetY=0XFFBE;
        factory_configuration.SixAxisSensorCalibrationValue.Gyroscope0OffsetZ=0XFFEE;
        factory_configuration.SixAxisSensorCalibrationValue.Gyroscope78rpmScaleX=0X343B;
        factory_configuration.SixAxisSensorCalibrationValue.Gyroscope78rpmScaleY=0X343B;
        factory_configuration.SixAxisSensorCalibrationValue.Gyroscope78rpmScaleZ=0X343B;
        //50FD0000C60F0F30
        factory_configuration.Model.SixAxisSensorModelValue.SixAxisHorizontalOffsetX=0xFD50;
        factory_configuration.Model.SixAxisSensorModelValue.SixAxisHorizontalOffsetY=0X0000;
        factory_configuration.Model.SixAxisSensorModelValue.SixAxisHorizontalOffsetZ=0X0FC6;
        factory_configuration.nonexist=0;
        //fac_conf_write();
    }
    factory_configuration.IdentificationCode[0]=0x80;//no sn
    factory_configuration.DeviceType=0x03;
    factory_configuration.BoardRevision=0xA0;
    factory_configuration.FormatVersion=0x1;//use custom color/

    //flash_read(0, (uint8_t*)&user_calibration, sizeof(user_calibration));
    read_flash(FLASH_ADDR_USER_CALIBRATION, (uint8_t*)&user_calibration, sizeof(user_calibration));
    //xenoblade?
    memset(&config,0,sizeof(config));
    memset(&(config.rgb.data),-1,sizeof(config.rgb.data));
    //custom_conf_read();
    uint8_t magic = 0;
    read_flash(FLASH_ADDR_USER_CONFIG,&magic,1);
    //conf_read(0xF000, &magic, 1);
    if(!CONFIG_EXIST(magic)){
        //config.magic=CONFIG_MAGIC;

        config.hw.pcb_typ=PCB_TYP_PRO;
        config.hw.pcb_rev=PRO_300;
        config.hw.rgb_cnt=25;
        config.hw.indi_led_ofst=4;

        config.basic.ns_pkt_timer_mode=0;
        config.basic.pro_fw_version=2;

        config.usb.in_interval=8;
        config.usb.out_interval=8;
        config.usb.auto_recovery=1;
        for(int i=0;i<4;++i){
            config.js.ratio[i]=(i<2?32:32);
            config.rumble.hd.amp_ratio[i]=128;
            config.js.dz[i]=96;
            config.js.center[i]=2048;
        }
        //user_config.hd_rumble_mixer_ratio=64;//total amp = hi_amp + (-1/2)lo_amp
        config.snpbk.dz[0]=1200;
        config.snpbk.dz[1]=1200;
        config.js.dz_mode=3;
        config.js.l_mode=0;
        config.js.r_mode=0;

        config.imu.sample_gap=1750;
        for(int i=0;i<config.hw.rgb_cnt;++i){
            config.rgb.data[i].load=0xffffff;
        }
        config.imu.ratio_x=127;
        config.imu.ratio_y=127;
        config.imu.ratio_z=127;

        config.snpbk.filter_window=15000;
        config.rumble.hd.pattern=0;
        config.rumble.hd.legacy=0;

        config.imu.enable=0;//we disabled this in default as many people dont need this
        config.rumble.enable=1;
        config.rgb.enable=1;

        config.rgb.slow_start_period=100;
        config.rgb.indi_led_brightness=15;

    }else{
        custom_conf_read();
    }
    conf_flush();
}
void conf_read(uint32_t addr,uint8_t* buf,uint8_t size){
    switch(addr & 0xfff000)
    {
    case 0x5000:
        break;
    case 0x6000:
        memcpy(buf,((uint8_t*)&factory_configuration)+(addr&0xff),size);
        break;
    case 0x8000:
        memcpy(buf,((uint8_t*)&user_calibration)+(addr&0xff),size);
        break;
    case 0xF000://user config
        memcpy(buf,((uint8_t*)&config)+(addr&0xfff),size);
        break;
    default:
        break;
    }
}
uint8_t conf_write(uint32_t addr,uint8_t* buf,uint8_t size,uint8_t save){
    uint8_t flash_res=0;
    switch(addr & 0xfff000)
    {
    case 0x5000:
        break;
    case 0x6000:
        memcpy(((uint8_t*)&factory_configuration)+(addr&0xff),(uint8_t*)buf,size);
        if(save)
            flash_res=fac_conf_write();
        uart_conf_write(addr, ((uint8_t*)&factory_configuration)+(addr&0xff), size);
        break;
    case 0x8000:
        memcpy(((uint8_t*)&user_calibration)+(addr&0xff),(uint8_t*)buf,size);
        if(save)
            flash_res=write_flash(FLASH_ADDR_USER_CALIBRATION,(uint8_t*)&user_calibration,sizeof(user_calibration));
        uart_conf_write(addr, ((uint8_t*)&user_calibration)+(addr&0xff), size);
        break;
    case 0xF000:
        memcpy(((uint8_t*)&config)+(addr&0xfff),(uint8_t*)buf,size);
        if(save)
            flash_res=custom_conf_write(addr&0xfff,size);
        uart_conf_write(addr, ((uint8_t*)&config)+(addr&0xfFf), size);
        conf_flush();
        break;
    default:
        break;
    }
    ////printf("conf write res: %d\r\n",flash_res);
    return flash_res;
}
void custom_conf_read()
{
    read_flash(FLASH_ADDR_USER_CONFIG, (uint8_t*)&config, sizeof(config));
    //flash_read(1, (uint8_t*)&user_config, sizeof(user_config));
}
uint8_t custom_conf_write(uint32_t addr,uint32_t size)
{
    return write_flash(FLASH_ADDR_USER_CONFIG+addr,((uint8_t*)&config)+addr, size);
    //return flash_write(1, (uint8_t*)&user_config, sizeof(user_config));
}
void fac_conf_read(){
    read_flash(FLASH_ADDR_FACTORY_CONFIG,(uint8_t*)&factory_configuration,sizeof(factory_configuration_data));
}
uint8_t fac_conf_write(){
    return write_flash(FLASH_ADDR_FACTORY_CONFIG, (uint8_t*)&factory_configuration,sizeof(factory_configuration_data));
}
void conf_flush(){
    MyCfgDescr[33]=config.usb.out_interval;
    MyCfgDescr[40]=config.usb.in_interval;
    imu_ratio_xf=config.imu.ratio_x/127.0f;
    imu_ratio_yf=config.imu.ratio_y/127.0f;
    imu_ratio_zf=config.imu.ratio_z/127.0f;
    joystick_snapback_deadzone_sq[0]=((uint32_t)config.snpbk.dz[0])*config.snpbk.dz[0];
    joystick_snapback_deadzone_sq[1]=((uint32_t)config.snpbk.dz[1])*config.snpbk.dz[1];
    adc_init();
    gpio_init();
    hd_rumble_lookup_tb_init();
    hd_rumble_set_status(config.rumble.enable);
    button_active_mask=~config.btn.disable_mask;
    rgb_slow_start_div=config.rgb.slow_start_period*50.0f;
    if(rgb_slow_start_div<=0)rgb_slow_start_div=1.0f;
    set_hd_rumble_range();
    for(int j=0;j<2;++j){
        for(int i=0;i<config.affine[j].cnt;++i){
            affine_map[j][i].notch.x=config.affine[j].map[i].notch.ix;
            affine_map[j][i].notch.y=config.affine[j].map[i].notch.iy;
            affine_map[j][i].angle.x=config.affine[j].map[i].angle.ix;
            affine_map[j][i].angle.y=config.affine[j].map[i].angle.iy;
            affine_max[j].x=i32_max(affine_max[j].x, affine_map[j][i].angle.x);
            affine_max[j].y=i32_max(affine_max[j].y, affine_map[j][i].angle.y);
            affine_min[j].x=i32_min(affine_min[j].x, affine_map[j][i].angle.x);
            affine_min[j].y=i32_min(affine_min[j].y, affine_map[j][i].angle.y);
        }
    }
    affine_init();
    flush_rgb();
    //gpio_tb_init();
    //uart_update_config();
}
