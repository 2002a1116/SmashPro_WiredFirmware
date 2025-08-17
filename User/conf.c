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
#include <string.h>
//#pragma pack(push,4)
factory_configuration_data factory_configuration;
user_calibration_data user_calibration;
user_config_data user_config;
smashpro_factory_config_data smashpro_factory_config;
//#pragma pack(pop)
uint32_t joystick_snapback_deadzone_sq[2];
//static factory_configuration_flash_pack* fac;
#define JOYSTICK_RANGE_FACTOR_LEFT (0.6f)
#define JOYSTICK_RANGE_FACTOR_RIGHT (0.6f)
void set_hd_rumble_range(){
    switch(smashpro_factory_config.pcb_rev){
        case PCB_REV_200:
            hd_rumble_cvr_range=650;
            hd_rumble_cvr_max_offset=720;
            break;
        case PCB_REV_213:
            hd_rumble_cvr_range=540;
            hd_rumble_cvr_max_offset=720;
            break;
        //case PCB_REV_213_HIGH_VOLTAGE:
            //disused,use a 270k gain resistor instead
            /*fall through*/
        default://smallest the safest
            hd_rumble_cvr_range=500;
            hd_rumble_cvr_max_offset=680;
            break;
    }
    hd_rumble_cvr_max=HD_RUMBLE_TIM_PERIOD_MID+hd_rumble_cvr_max_offset;
    hd_rumble_cvr_min=HD_RUMBLE_TIM_PERIOD_MID-hd_rumble_cvr_max_offset;
}
void conf_init()
{
    memset(&factory_configuration,-1,sizeof(factory_configuration));
    memset(&smashpro_factory_config,-1,sizeof(smashpro_factory_config));
    read_flash(FLASH_ADDR_HARDWARE_INFO,(uint8_t*)&smashpro_factory_config,sizeof(smashpro_factory_config));
    if(smashpro_factory_config.nonexist)
    {
        memset(&smashpro_factory_config,0,sizeof(smashpro_factory_config));
        /*smashpro_factory_config.rev_high=2;//2.00.0
        smashpro_factory_config.rev_low=13;
        smashpro_factory_config.rev_suffix=0;*/
        smashpro_factory_config.pcb_rev=PCB_REV_213;
        smashpro_factory_config.rgb_cnt=31;
        uint8_t res = write_flash(FLASH_ADDR_HARDWARE_INFO,(uint8_t*)&smashpro_factory_config,
                sizeof(smashpro_factory_config));
    }
    //fac=get_raw_flash_buf();
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

        factory_configuration.Design.ControllerColor.MainColor.r=0x32;
        factory_configuration.Design.ControllerColor.MainColor.g=0x31;
        factory_configuration.Design.ControllerColor.MainColor.b=0x32;
        factory_configuration.Design.ControllerColor.SubColor.r=0xff;
        factory_configuration.Design.ControllerColor.SubColor.g=0xff;
        factory_configuration.Design.ControllerColor.SubColor.b=0xff;
        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelNoise=0x00F;
        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelTypicalStroke=0x613;
        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelCenterDeadZoneSize=0x0AE;
        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelCircuitDeadZoneScale=0xD99;
        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelMinimumStrokeXPositive=0x4D4;
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
        fac_conf_write();
    }
    factory_configuration.IdentificationCode[0]=0x80;//no sn
    factory_configuration.DeviceType=0x03;
    factory_configuration.BoardRevision=0xA0;
    factory_configuration.FormatVersion=0x1;//use custom color/

    //flash_read(0, (uint8_t*)&user_calibration, sizeof(user_calibration));
    read_flash(FLASH_ADDR_USER_CALIBRATION, (uint8_t*)&user_calibration, sizeof(user_calibration));
    if(user_calibration.nonexist){
        for(int i=0;i<4;++i)
            user_calibration.internal_center[i]=2048;
    }
    //xenoblade?
    memset(&user_config,-1,sizeof(user_config));
    custom_conf_read();
    if(user_config.nonexist)
    {
        memset(&user_config,0,sizeof(user_config));
        user_config.in_interval=8;
        user_config.out_interval=8;
        for(int i=0;i<4;++i){
            user_config.joystick_ratio[i]=(i<2?32:32);
            user_config.hd_rumble_amp_ratio[i]=128;
            //user_config.hd_rumble_amp_ratio[i]=(i<2?128:128);
            user_config.dead_zone[i]=64;
        }
        //user_config.hd_rumble_mixer_ratio=64;//total amp = hi_amp + (-1/2)lo_amp
        user_config.joystick_snapback_deadzone[0]=1400;
        user_config.joystick_snapback_deadzone[1]=1400;
        user_config.dead_zone_mode=3;
        //user_config.rgb_cnt=31;
        user_config.imu_sample_gap=1750;
        for(int i=0;i<smashpro_factory_config.rgb_cnt;++i){
            user_config.rgb_data[i].load=0xffffff;
        }
        user_config.imu_ratio_x=127;
        user_config.imu_ratio_y=127;
        user_config.imu_ratio_z=127;
        user_config.pro_fw_version=2;
        user_config.joystick_snapback_filter_max_delay=13500;
        user_config.rumble_pattern=0;
        user_config.legacy_rumble=0;
        //user_config.led_typ=CONF_PCB_TYPE_SMALL;
        //user_config.input_typ=0;
        user_config.imu_disabled=1;//we disabled this in default as many people dont need this
        custom_conf_write();
    }
    conf_flush();
    MyCfgDescr[33]=user_config.out_interval;
    MyCfgDescr[40]=user_config.in_interval;
    memcpy(connection_state.bd_addr,user_config.bd_addr,BD_ADDR_LEN);
}
void conf_read(uint32_t addr,uint8_t* buf,uint8_t size){
    switch(addr & 0xffff00)
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
        memcpy(buf,((uint8_t*)&user_config)+(addr&0xff),size);
        break;
    case 0x0000://persistent flash hack
        memcpy(buf,((uint8_t*)&smashpro_factory_config)+(addr&0xff),size);
        //memcpy(buf,((uint8_t*)FLASH_ADDR_USEROPTION_PERSISTENT_BYTE)+(addr&0xff),size);
        break;
    default:
        break;
    }
}
uint8_t conf_write(uint32_t addr,uint8_t* buf,uint8_t size,uint8_t save){
    uint8_t flash_res=0;
    switch(addr & 0xffff00)
    {
    case 0x5000:
        break;
    case 0x6000:
        memcpy(((uint8_t*)&factory_configuration)+(addr&0xff),(uint8_t*)buf,size);
        if(save)
            flash_res=fac_conf_write();
        //flash_res=write_flash(FLASH_ADDR_FACTORY_CONFIG, (uint8_t*)buf, size);
        uart_conf_write(addr, ((uint8_t*)&factory_configuration)+(addr&0xff), size);
        break;
    case 0x8000:
        memcpy(((uint8_t*)&user_calibration)+(addr&0xff),(uint8_t*)buf,size);
        //flash_res=flash_write(0, (uint8_t*)&user_calibration, sizeof(user_calibration));
        //flash_res=write_flash(FLASH_ADDR_USER_CALIBRATION, (uint8_t*)buf, size);
        if(save)
            flash_res=write_flash(FLASH_ADDR_USER_CALIBRATION,(uint8_t*)&user_calibration,sizeof(user_calibration));
        uart_conf_write(addr, ((uint8_t*)&user_calibration)+(addr&0xff), size);
        break;
    case 0xF000:
        memcpy(((uint8_t*)&user_config)+(addr&0xff),(uint8_t*)buf,size);
        if(save)
            flash_res=custom_conf_write();
        uart_conf_write(addr, ((uint8_t*)&user_config)+(addr&0xff), size);
        break;
    case 0x0000://smash pro factory hack
        memcpy(((uint8_t*)&smashpro_factory_config)+(addr&0xff),(uint8_t*)buf,size);
        if(save)
            flash_res=write_flash(FLASH_ADDR_HARDWARE_INFO, (uint8_t*)buf, size);
        uart_conf_write(addr, buf, size);
        hw_config_flush();
    default:
        break;
    }
    ////printf("conf write res: %d\r\n",flash_res);
    return flash_res;
}
void custom_conf_read()
{
    read_flash(FLASH_ADDR_USER_CONFIG, (uint8_t*)&user_config, sizeof(user_config));
    //flash_read(1, (uint8_t*)&user_config, sizeof(user_config));
}
uint8_t custom_conf_write()
{
    return write_flash(FLASH_ADDR_USER_CONFIG,(uint8_t*)&user_config, sizeof(user_config));
    //return flash_write(1, (uint8_t*)&user_config, sizeof(user_config));
}/*
void unpack_fac_conf()
{
    if(!fac)return;
    //factory_configuration.nonexist=1;
    if(factory_configuration.nonexist){
        //factory_configuration.DeviceType=0x03;
        //factory_configuration.BoardRevision=0xA0;
        //factory_configuration.FormatVersion=0x1;//use custom color/
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
                factory_configuration.JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue;/
        memcpy(&factory_configuration.JoystickCalibrationValue.AnalogStickRightFactoryCalibrationValue,
               &factory_configuration.JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue,
               sizeof(joystick_calibration_data_left));

        factory_configuration.Design.ControllerColor.MainColor.r=0x32;
        factory_configuration.Design.ControllerColor.MainColor.g=0x31;
        factory_configuration.Design.ControllerColor.MainColor.b=0x32;
        factory_configuration.Design.ControllerColor.SubColor.r=0xff;
        factory_configuration.Design.ControllerColor.SubColor.g=0xff;
        factory_configuration.Design.ControllerColor.SubColor.b=0xff;

        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelNoise=0x00F;
        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelTypicalStroke=0x613;
        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelCenterDeadZoneSize=0x0AE;
        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelCircuitDeadZoneScale=0xD99;
        factory_configuration.Model.AnalogStickMainModelValue.AnalogStickModelMinimumStrokeXPositive=0x4D4;
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
        factory_configuration.SixAxisSensorCalibrationValue.Gyroscope0OffsetZ=0X0001;/

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
        //factory_configuration.nonexist=0;
    }
    memcpy(&factory_configuration.SixAxisSensorCalibrationValue,
            &(factory_configuration.SixAxisSensorCalibrationValue),sizeof(imu_calibration_data));
    //factory_configuration.SixAxisSensorCalibrationValue.Gyroscope78rpmScaleX=0x343B;
    //factory_configuration.SixAxisSensorCalibrationValue.Gyroscope78rpmScaleY=0x343B;
    //factory_configuration.SixAxisSensorCalibrationValue.Gyroscope78rpmScaleZ=0x343B;
    memcpy(&factory_configuration.JoystickCalibrationValue,
            &(factory_configuration.JoystickCalibrationValue),sizeof(factory_joystick_calibration_data));
    memcpy(&factory_configuration.Design,&(factory_configuration.Design),sizeof(device_design_data));
    memcpy(&factory_configuration.Model,&(factory_configuration.Model),sizeof(model_data));
    memcpy(&factory_configuration.AnalogStickSubModelValue,
            &(factory_configuration.AnalogStickSubModelValue),sizeof(joystick_model_value));
    //factory_configuration.Model.SixAxisSensorModelValue.SixAxisHorizontalOffsetX=0xFD50;
    //factory_configuration.Model.SixAxisSensorModelValue.SixAxisHorizontalOffsetY=0X0000;
    //factory_configuration.Model.SixAxisSensorModelValue.SixAxisHorizontalOffsetZ=0X0FC6;
    if(factory_configuration.nonexist)
    {
        factory_configuration.nonexist=0;
        fac_conf_write();
    }
}*/
/*
void pack_fac_conf()
{
    if(!fac)return;
    memset(fac,-1,128);
    memcpy(&(factory_configuration.SixAxisSensorCalibrationValue),
            &factory_configuration.SixAxisSensorCalibrationValue,sizeof(imu_calibration_data));
    memcpy(&(factory_configuration.JoystickCalibrationValue),
            &factory_configuration.JoystickCalibrationValue,sizeof(factory_joystick_calibration_data));
    memcpy(&(factory_configuration.Design),&factory_configuration.Design,sizeof(device_design_data));
    memcpy(&(factory_configuration.Model),&factory_configuration.Model,sizeof(model_data));
    memcpy(&(factory_configuration.AnalogStickSubModelValue),&factory_configuration.AnalogStickSubModelValue,sizeof(joystick_model_value));
    factory_configuration.nonexist=0;
    /*factory_configuration.DeviceType=factory_configuration.DeviceType;
    factory_configuration.BoardRevision=factory_configuration.BoardRevision;
    factory_configuration.FormatVersion=factory_configuration.FormatVersion;//use custom color//
}*/
void fac_conf_read(){
    //flash_read(2, fac, sizeof(factory_configuration_flash_pack));
    //unpack_fac_conf(fac);
    read_flash(FLASH_ADDR_FACTORY_CONFIG,(uint8_t*)&factory_configuration,sizeof(factory_configuration_data));
}
uint8_t fac_conf_write(){
    //pack_fac_conf(fac);
    //return raw_flash_write(2);
    //flash_write(2, (uint8_t*)&factory_configuration_flash, sizeof(factory_configuration_flash));
    return write_flash(FLASH_ADDR_FACTORY_CONFIG, (uint8_t*)&factory_configuration,sizeof(factory_configuration_data));
}
void conf_flush(){
    imu_ratio_xf=user_config.imu_ratio_x/127.0f;
    imu_ratio_yf=user_config.imu_ratio_y/127.0f;
    imu_ratio_zf=user_config.imu_ratio_z/127.0f;
    joystick_snapback_deadzone_sq[0]=((uint32_t)user_config.joystick_snapback_deadzone[0])*user_config.joystick_snapback_deadzone[0];
    joystick_snapback_deadzone_sq[1]=((uint32_t)user_config.joystick_snapback_deadzone[1])*user_config.joystick_snapback_deadzone[1];
    gpio_tb_init();
    gpio_init();
    hd_rumble_lookup_tb_init();
    flush_rgb(ENABLE);
    //uart_update_config();
}
void hw_config_flush(){
    set_hd_rumble_range();
    flush_rgb(ENABLE);
}
