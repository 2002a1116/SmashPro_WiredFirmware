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

factory_configuration_data factory_configuration;
user_calibration_data user_calibration;
user_config_data user_config;
uint32_t joystick_snapback_deadzone_sq[2];
static factory_configuration_flash_pack* fac;
#define JOYSTICK_RANGE_FACTOR_LEFT (0.6f)
#define JOYSTICK_RANGE_FACTOR_RIGHT (0.6f)
void conf_init()
{
    memset(&factory_configuration,-1,sizeof(factory_configuration));
    fac=get_raw_flash_buf();
    fac_conf_read();
    factory_configuration.IdentificationCode[0]=0x80;//no sn
    factory_configuration.DeviceType=0x03;
    factory_configuration.BoardRevision=0xA0;
    factory_configuration.FormatVersion=0x1;//use custom color/

    flash_read(0, (uint8_t*)&user_calibration, sizeof(user_calibration));
    if(user_calibration.nonexist){
        for(int i=0;i<4;++i)
            user_calibration.internal_center[i]=2048;
    }

/*    //printf("user cali magic:%d\r\n",user_calibration.UserJoystickCalibrationValue.AnalogStickLeftUserMagicNumber);
    for(int i=0;i<9;++i)
    {
        //printf("0x%02x ",*(((uint8_t*)&user_calibration.UserJoystickCalibrationValue.AnalogStickLeftUserCalibrationValue)+i));
    }
    //printf("\r\n");*/
    //0xb2 0xa1 0xf0 0x07 0x74 0x00 0x08 0x8b 0xf0 0x07 0x8a
    /*if(user_calibration.UserJoystickCalibrationValue.AnalogStickLeftUserMagicNumber!=0xa1b2)
    {
        memset((uint8_t*)&user_calibration, 0, sizeof(user_calibration));
        //user_calibration.UserJoystickCalibrationValue.AnalogStickLeftUserCalibrationValue=factory_configuration.Model.AnalogStickMainModelValue;
        //user_calibration.UserJoystickCalibrationValue.AnalogStickRightUserCalibrationValue=factory_configuration.Model.AnalogStickMainModelValue;
        user_calibration.UserJoystickCalibrationValue.AnalogStickLeftUserMagicNumber=0xa1b2;
        user_calibration.UserJoystickCalibrationValue.AnalogStickRightUserMagicNumber=0xa1b2;
        flash_write(0, (uint8_t*)&user_calibration, sizeof(user_calibration));
    }*/
    //xenoblade?
    memset(&user_config,-1,sizeof(user_config));
    custom_conf_read();
    if(user_config.nonexist)
    {
        memset(&user_config,0,sizeof(user_config));
        user_config.in_interval=8;
        user_config.out_interval=8;
        for(int i=0;i<4;++i){
            user_config.joystick_ratio[i]=(i<2?40:40);
            user_config.hd_rumble_amp_ratio[i]=(i<2?128:128);
            user_config.dead_zone[i]=96;
        }
        user_config.joystick_snapback_deadzone[0]=1400;
        user_config.joystick_snapback_deadzone[1]=1400;
        user_config.dead_zone_mode=1;
        user_config.rgb_cnt=27;
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
        custom_conf_write();
    }
    conf_flush();
    MyCfgDescr[33]=user_config.out_interval;
    MyCfgDescr[40]=user_config.in_interval;
    /*//printf("interval %d %d \r\n",user_config.out_interval,user_config.in_interval);
    //printf("jsratio %d %d %d %d\r\n",user_config.joystick_ratio[0],
            user_config.joystick_ratio[1],user_config.joystick_ratio[2],user_config.joystick_ratio[3]);
    //printf("ampratio %d %d %d %d\r\n",user_config.hd_rumble_amp_ratio[0],
            user_config.hd_rumble_amp_ratio[1],user_config.hd_rumble_amp_ratio[2],user_config.hd_rumble_amp_ratio[3]);
    */
    memcpy(connection_state.bd_addr,user_config.bd_addr,BD_ADDR_LEN);
}
void conf_read(uint32_t addr,uint8_t* buf,uint8_t size){
    switch(addr & 0xffff00)
    {
    case 0x5000:
        break;
    case 0x6000:
        memcpy(buf,((uint8_t*)&factory_configuration)+(addr&0xff),size);
        /*if(addr==0x6080)
        {
            buf[0]=size;
            //buf[0]=factory_configuration.Model.SixAxisSensorModelValue.SixAxisHorizontalOffsetX;
            buf[1]=factory_configuration.Model.SixAxisSensorModelValue.SixAxisHorizontalOffsetX;
            buf[2]=factory_configuration.Model.SixAxisSensorModelValue.SixAxisHorizontalOffsetX>>8;
        }*/
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
        break;
    case 0x8000:
        memcpy(((uint8_t*)&user_calibration)+(addr&0xff),(uint8_t*)buf,size);
        flash_res=flash_write(0, (uint8_t*)&user_calibration, sizeof(user_calibration));
        break;
    case 0xF000:
        memcpy(((uint8_t*)&user_config)+(addr&0xff),(uint8_t*)buf,size);
        break;
    default:
        break;
    }
    ////printf("conf write res: %d\r\n",flash_res);
    return flash_res;
}
void custom_conf_read()
{
    flash_read(1, (uint8_t*)&user_config, sizeof(user_config));
}
uint8_t custom_conf_write()
{
    return flash_write(1, (uint8_t*)&user_config, sizeof(user_config));
}
void unpack_fac_conf()
{
    if(!fac)return;
    //fac->nonexist=1;
    if(fac->nonexist){
        /*fac->DeviceType=0x03;
        fac->BoardRevision=0xA0;
        fac->FormatVersion=0x1;//use custom color/*/
        fac->JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue.AnalogStickCalXPositive=
                2048*JOYSTICK_RANGE_FACTOR_LEFT;
        fac->JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue.AnalogStickCalYPositive=
                2048*JOYSTICK_RANGE_FACTOR_LEFT;
        fac->JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue.AnalogStickCalXNegative=
                2048*JOYSTICK_RANGE_FACTOR_LEFT;
        fac->JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue.AnalogStickCalYNegative=
                2048*JOYSTICK_RANGE_FACTOR_LEFT;
        fac->JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue.AnalogStickCalX0=
                fac->JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue.AnalogStickCalY0=2048;
        /*fac->JoystickCalibrationValue.AnalogStickRightFactoryCalibrationValue=
                fac->JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue;*/
        memcpy(&fac->JoystickCalibrationValue.AnalogStickRightFactoryCalibrationValue,
               &fac->JoystickCalibrationValue.AnalogStickLeftFactoryCalibrationValue,
               sizeof(joystick_calibration_data));

        fac->Design.ControllerColor.MainColor.r=0x32;
        fac->Design.ControllerColor.MainColor.g=0x31;
        fac->Design.ControllerColor.MainColor.b=0x32;
        fac->Design.ControllerColor.SubColor.r=0xff;
        fac->Design.ControllerColor.SubColor.g=0xff;
        fac->Design.ControllerColor.SubColor.b=0xff;

        fac->Model.AnalogStickMainModelValue.AnalogStickModelNoise=0x00F;
        fac->Model.AnalogStickMainModelValue.AnalogStickModelTypicalStroke=0x613;
        fac->Model.AnalogStickMainModelValue.AnalogStickModelCenterDeadZoneSize=0x0AE;
        fac->Model.AnalogStickMainModelValue.AnalogStickModelCircuitDeadZoneScale=0xD99;
        fac->Model.AnalogStickMainModelValue.AnalogStickModelMinimumStrokeXPositive=0x4D4;
        fac->Model.AnalogStickMainModelValue.AnalogStickModelMinimumStrokeYPositive=0x541;
        fac->Model.AnalogStickMainModelValue.AnalogStickModelMinimumStrokeXNegative=0x541;
        fac->Model.AnalogStickMainModelValue.AnalogStickModelMinimumStrokeYNegative=0x541;
        fac->Model.AnalogStickMainModelValue.AnalogStickModelCenterRangeXPositive=0x9C7;
        fac->Model.AnalogStickMainModelValue.AnalogStickModelCenterRangeYPositive=0x9C7;
        fac->Model.AnalogStickMainModelValue.AnalogStickModelCenterRangeXNegative=0x633;
        fac->Model.AnalogStickMainModelValue.AnalogStickModelCenterRangeYNegative=0x633;
        fac->AnalogStickSubModelValue=fac->Model.AnalogStickMainModelValue;
        //5B008BFF96010040004000401D00BEFFEEFF3B343B343B34
        fac->SixAxisSensorCalibrationValue.Accelerometer0OffsetX=0x005B;
        fac->SixAxisSensorCalibrationValue.Accelerometer0OffsetY=0xFF8B;
        fac->SixAxisSensorCalibrationValue.Accelerometer0OffsetZ=0X0196;
        fac->SixAxisSensorCalibrationValue.Accelerometer1gScaleX=0X4000;
        fac->SixAxisSensorCalibrationValue.Accelerometer1gScaleY=0X4000;
        fac->SixAxisSensorCalibrationValue.Accelerometer1gScaleZ=0X4000;
        /*factory_configuration.SixAxisSensorCalibrationValue.Gyroscope0OffsetX=0X0001;
        factory_configuration.SixAxisSensorCalibrationValue.Gyroscope0OffsetY=0X0001;
        factory_configuration.SixAxisSensorCalibrationValue.Gyroscope0OffsetZ=0X0001;*/

        fac->SixAxisSensorCalibrationValue.Gyroscope0OffsetX=0X001D;
        fac->SixAxisSensorCalibrationValue.Gyroscope0OffsetY=0XFFBE;
        fac->SixAxisSensorCalibrationValue.Gyroscope0OffsetZ=0XFFEE;

        fac->SixAxisSensorCalibrationValue.Gyroscope78rpmScaleX=0X343B;
        fac->SixAxisSensorCalibrationValue.Gyroscope78rpmScaleY=0X343B;
        fac->SixAxisSensorCalibrationValue.Gyroscope78rpmScaleZ=0X343B;
        //50FD0000C60F0F30
        fac->Model.SixAxisSensorModelValue.SixAxisHorizontalOffsetX=0xFD50;
        fac->Model.SixAxisSensorModelValue.SixAxisHorizontalOffsetY=0X0000;
        fac->Model.SixAxisSensorModelValue.SixAxisHorizontalOffsetZ=0X0FC6;
        //fac->nonexist=0;
    }
    memcpy(&factory_configuration.SixAxisSensorCalibrationValue,
            &(fac->SixAxisSensorCalibrationValue),sizeof(imu_calibration_data));
    //factory_configuration.SixAxisSensorCalibrationValue.Gyroscope78rpmScaleX=0x343B;
    //factory_configuration.SixAxisSensorCalibrationValue.Gyroscope78rpmScaleY=0x343B;
    //factory_configuration.SixAxisSensorCalibrationValue.Gyroscope78rpmScaleZ=0x343B;
    memcpy(&factory_configuration.JoystickCalibrationValue,
            &(fac->JoystickCalibrationValue),sizeof(factory_joystick_calibration_data));
    memcpy(&factory_configuration.Design,&(fac->Design),sizeof(device_design_data));
    memcpy(&factory_configuration.Model,&(fac->Model),sizeof(model_data));
    memcpy(&factory_configuration.AnalogStickSubModelValue,
            &(fac->AnalogStickSubModelValue),sizeof(joystick_model_value));
    //factory_configuration.Model.SixAxisSensorModelValue.SixAxisHorizontalOffsetX=0xFD50;
    //factory_configuration.Model.SixAxisSensorModelValue.SixAxisHorizontalOffsetY=0X0000;
    //factory_configuration.Model.SixAxisSensorModelValue.SixAxisHorizontalOffsetZ=0X0FC6;
    if(fac->nonexist)
    {
        fac->nonexist=0;
        fac_conf_write();
    }
}
void pack_fac_conf()
{
    if(!fac)return;
    memset(fac,-1,128);
    memcpy(&(fac->SixAxisSensorCalibrationValue),
            &factory_configuration.SixAxisSensorCalibrationValue,sizeof(imu_calibration_data));
    memcpy(&(fac->JoystickCalibrationValue),
            &factory_configuration.JoystickCalibrationValue,sizeof(factory_joystick_calibration_data));
    memcpy(&(fac->Design),&factory_configuration.Design,sizeof(device_design_data));
    memcpy(&(fac->Model),&factory_configuration.Model,sizeof(model_data));
    memcpy(&(fac->AnalogStickSubModelValue),&factory_configuration.AnalogStickSubModelValue,sizeof(joystick_model_value));
    fac->nonexist=0;
    /*fac->DeviceType=factory_configuration.DeviceType;
    fac->BoardRevision=factory_configuration.BoardRevision;
    fac->FormatVersion=factory_configuration.FormatVersion;//use custom color/*/
}
void fac_conf_read(){
    flash_read(2, fac, sizeof(factory_configuration_flash_pack));
    unpack_fac_conf(fac);
}
uint8_t fac_conf_write(){
    pack_fac_conf(fac);
    return raw_flash_write(2);
    //flash_write(2, (uint8_t*)&factory_configuration_flash, sizeof(factory_configuration_flash));
}
void conf_flush(){
    flush_rgb(ENABLE);
    imu_ratio_xf=user_config.imu_ratio_x/127.0f;
    imu_ratio_yf=user_config.imu_ratio_y/127.0f;
    imu_ratio_zf=user_config.imu_ratio_z/127.0f;
    joystick_snapback_deadzone_sq[0]=((uint32_t)user_config.joystick_snapback_deadzone[0])*user_config.joystick_snapback_deadzone[0];
    joystick_snapback_deadzone_sq[1]=((uint32_t)user_config.joystick_snapback_deadzone[1])*user_config.joystick_snapback_deadzone[1];
    gpio_tb_init();
    if(user_config.a_b_swap){
        hid_num_to_gpio[NS_BUTTON_A]=GPIO_BUTTON_B;
        hid_num_to_gpio[NS_BUTTON_B]=GPIO_BUTTON_A;
    }
    if(user_config.x_y_swap){
        hid_num_to_gpio[NS_BUTTON_X]=GPIO_BUTTON_Y;
        hid_num_to_gpio[NS_BUTTON_Y]=GPIO_BUTTON_X;
    }
    if(user_config.cross_key_disabled){
        hid_num_to_gpio[NS_BUTTON_UP]=GPIO_BUTTON_NONE;
        hid_num_to_gpio[NS_BUTTON_DOWN]=GPIO_BUTTON_NONE;
        hid_num_to_gpio[NS_BUTTON_LEFT]=GPIO_BUTTON_NONE;
        hid_num_to_gpio[NS_BUTTON_RIGHT]=GPIO_BUTTON_NONE;
    }
    uart_update_config();
}
