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
#include <string.h>
//#pragma pack(push,4)
factory_configuration_data factory_configuration;
user_calibration_data user_calibration;
user_config_data user_config;
//#pragma pack(pop)
//we need these structure 4-byte align as flash api requiring an u32*
uint32_t joystick_snapback_deadzone_sq[2];
//static factory_configuration_flash_pack* fac;
#define JOYSTICK_RANGE_FACTOR_LEFT (0.6f)
#define JOYSTICK_RANGE_FACTOR_RIGHT (0.6f)
void conf_init()
{
    memset(&factory_configuration,-1,sizeof(factory_configuration));
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
        memcpy(&factory_configuration.JoystickCalibrationValue.AnalogStickRightFactoryCalibrationValue,
               &factory_configuration.JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue,
               sizeof(joystick_calibration_data));
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
            user_config.joystick_ratio[i]=(i<2?56:56);
            user_config.hd_rumble_amp_ratio[i]=(i<2?128:128);
            user_config.dead_zone[i]=96;
        }
        user_config.joystick_snapback_deadzone[0]=1400;
        user_config.joystick_snapback_deadzone[1]=1400;
        user_config.dead_zone_mode=1;
        user_config.rgb_cnt=31;
        user_config.imu_sample_gap=1750;
        for(int i=0;i<user_config.rgb_cnt;++i){
            user_config.rgb_data[i].load=0xffffff;
        }
        user_config.imu_ratio_x=127;
        user_config.imu_ratio_y=127;
        user_config.imu_ratio_z=127;
        user_config.pro_fw_version=2;
        user_config.joystick_snapback_filter_max_delay=12500;
        user_config.rumble_pattern=0;
        user_config.pcb_typ=CONF_PCB_TYPE_LARGE;
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
    default:
        break;
    }
}
uint8_t conf_write(uint32_t addr,uint8_t* buf,uint8_t size){
    uint8_t flash_res=0;
    switch(addr & 0xffff00)
    {
    case 0x5000:
        break;
    case 0x6000:
        memcpy(((uint8_t*)&factory_configuration)+(addr&0xff),(uint8_t*)buf,size);
        flash_res=fac_conf_write();
        //flash_res=write_flash(FLASH_ADDR_FACTORY_CONFIG, (uint8_t*)buf, size);
        uart_conf_write(addr, ((uint8_t*)&factory_configuration)+(addr&0xff), size);
        break;
    case 0x8000:
        memcpy(((uint8_t*)&user_calibration)+(addr&0xff),(uint8_t*)buf,size);
        //flash_res=flash_write(0, (uint8_t*)&user_calibration, sizeof(user_calibration));
        //flash_res=write_flash(FLASH_ADDR_USER_CALIBRATION, (uint8_t*)buf, size);
        flash_res=write_flash(FLASH_ADDR_USER_CALIBRATION,(uint8_t*)&user_calibration,sizeof(user_calibration));
        uart_conf_write(addr, ((uint8_t*)&user_calibration)+(addr&0xff), size);
        break;
    case 0xF000:
        memcpy(((uint8_t*)&user_config)+(addr&0xff),(uint8_t*)buf,size);
        uart_conf_write(addr, ((uint8_t*)&user_config)+(addr&0xff), size);
        break;
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
               sizeof(joystick_calibration_data));

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
    flush_rgb(ENABLE);
    imu_ratio_xf=user_config.imu_ratio_x/127.0f;
    imu_ratio_yf=user_config.imu_ratio_y/127.0f;
    imu_ratio_zf=user_config.imu_ratio_z/127.0f;
    joystick_snapback_deadzone_sq[0]=((uint32_t)user_config.joystick_snapback_deadzone[0])*user_config.joystick_snapback_deadzone[0];
    joystick_snapback_deadzone_sq[1]=((uint32_t)user_config.joystick_snapback_deadzone[1])*user_config.joystick_snapback_deadzone[1];
    gpio_tb_init();
    //uart_update_config();
}
