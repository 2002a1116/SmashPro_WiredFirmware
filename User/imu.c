/*
 * imu.c
 *
 *  Created on: 2025Äê2ÔÂ25ÈÕ
 *      Author: Reed
 */
#include "debug.h"
#include "imu.h"
#include "i2c.h"
#include "tick.h"
#include "ns_com.h"
#include "imu_quaternion.h"
#include "conf.h"
#include "tick.h"
#include <string.h>
#pragma pack(push,1)
uint8_t imu_upd_cnt;
uint8_t imu_rdy;
uint8_t imu_mode;
static int32_t imu_sum[6];
int16_t imu_raw_buf[6];
int16_t imu_res[6];//x:1/2 x-1:1/4
float imu_ratio_xf,imu_ratio_yf,imu_ratio_zf;
#pragma pack(pop)
uint8_t imu_set_reg(uint8_t reg,uint8_t value,uint8_t mask)
{
    uint8_t buf,ret=0;
    if(ret=i2c_read_byte(reg, &buf))
        return ret;
    buf = (buf&(~mask))|value;
    //buf |= rate;
    ret=i2c_write_byte(reg, buf);
    return ret;
}

void imu_set_acc_bandwidth(uint8_t BW0XL, uint8_t ODR)
{
    imu_set_reg(LSM6DS3TRC_CTRL1_XL, BW0XL, 0);
    imu_set_reg(LSM6DS3TRC_CTRL8_XL, ODR, 0);
}
uint8_t imu_read(){
    static uint8_t status=0;
    uint8_t ret;
    if(ret=i2c_read_byte(LSM6DS3TRC_STATUS_REG, &status)){
        printf("imu status read fail,i2c error code:%d\r\n",i2c_error_code);
        return ret;
    }
    /*gyo_data=imu_raw_buf;
    acc_data=&(imu_raw_buf[3]);*/
    //memset(imu_raw_buf,0,12);
    //acc_available=gyo_available=1;
    if((status&LSM6DS3TRC_STATUS_ACCELEROMETER)&&(status&LSM6DS3TRC_STATUS_GYROSCOPE)){
        if(!i2c_read_continuous(LSM6DS3TRC_OUTX_L_G, imu_raw_buf, 12)){
            return 0;
        }
        else {
            return 1;
        }
    }
    /*
    if(status&LSM6DS3TRC_STATUS_ACCELEROMETER){
        if(!i2c_read_continuous(LSM6DS3TRC_OUTX_L_XL, imu_raw_buf+3, 6)){
            acc_available=1;
        }
    }
    if(status&LSM6DS3TRC_STATUS_GYROSCOPE){
        if(!i2c_read_continuous(LSM6DS3TRC_OUTX_L_G, imu_raw_buf, 6)){
            gyo_available=1;
        }
    }
    factory_configuration.Model.SixAxisSensorModelValue.SixAxisHorizontalOffsetY=(acc_available<<4)+gyo_available;
    if(acc_available&&gyo_available)
    {
        gyo_available=acc_available=0;
        return 0;
    }*/
    return 2;
}
void set_imu_awake()
{
    uint8_t ret=0;
    ret = imu_set_reg(0x12,0x01,0xff);
    printf("set_imu_awake 1\r\n");
    Delay_Ms(100);
    ret = imu_set_reg(0x13, 0x80, 0xFF);//if we set DRDY_MASK,imu seems crash?
    printf("set_imu_awake 7 %d\r\n",ret );
    ret = imu_set_reg(0x10,0x8e,0xff);
    printf("set_imu_awake 2 %d\r\n",ret );
    ret = imu_set_reg(0x11,0x5c,0xff);
    printf("set_imu_awake 3 %d\r\n",ret );
    //ret = imu_set_reg(LSM6DS3TRC_CTRL1_XL,0x0E,0x0f);//8g + 100hz aa
    //printf("set_imu_awake 4 %d\r\n",ret );
    //ret = imu_set_reg(LSM6DS3TRC_CTRL2_G,0x0C,0x0f);//2000dps
    //printf("set_imu_awake 5 %d\r\n",ret );
    //imu_set_acc_bandwidth(LSM6DS3TRC_ACC_BW0XL_400HZ, LSM6DS3TRC_ACC_LOW_PASS_ODR_100);
    //ret = imu_set_reg(0x0a, 0b00110110, 0xff);
    ret = imu_set_reg(0x12,0x44,0xff);//set Block Data Update
    printf("set_imu_awake 6 %d\r\n",ret );
    ret = imu_set_reg(0x14, 0x00, 0xFF);
    printf("set_imu_awake 8 %d\r\n",ret );
    ret = imu_set_reg(0x15, 0x00, 0xFF);
    printf("set_imu_awake 9 %d\r\n",ret );
    ret = imu_set_reg(0x16, 0x00, 0xff);
    printf("set_imu_awake 10 %d\r\n",ret );
    ret = imu_set_reg(0x17, 0x84, 0xFF);
    //ret = imu_set_reg(0x17, 0x00, 0xFF);
    printf("set_imu_awake 11 %d\r\n",ret );
    ret = imu_set_reg(0x18, 0x38, 0xFF);
    printf("set_imu_awake 12 %d\r\n",ret );
    ret = imu_set_reg(0x19, 0x3C, 0xFF);
    printf("set_imu_awake 13 %d\r\n",ret );
    /*ret = imu_set_reg(0x1B, 0x0F, 0xFF);
    printf("set_imu_awake 13 %d\r\n",ret );
    ret = imu_set_reg(0x1D, 0x02, 0xFF);
    printf("set_imu_awake 13 %d\r\n",ret );*/
    //todo:not sure about hpf freq.
    imu_rdy=1;
}
void set_imu_sleep()
{
    imu_rdy=0;
    imu_set_reg(LSM6DS3TRC_CTRL1_XL,LSM6DS3TRC_ACC_RATE_0,0xf0);
    imu_set_reg(LSM6DS3TRC_CTRL2_G,LSM6DS3TRC_GYR_RATE_0,0xf0);
}
void imu_init(){

}
static uint32_t last_imu_upd=0;
static uint8_t imu_sample_cnt=0;
imu_report_pack imu_pack_buf[2];
uint8_t imu_buf_pos;
static imu_report_pack* rep;
static uint8_t acc_exist,gyo_exist;
uint16_t test_cnt=0;
//78rpm in 5ms to milldgree
#define GYO_DATA_MAX (25000)
#define GYO_DATA_MIN (-25000)
void imu_upd()
{
    static uint32_t last_upd=0;
    rep=&imu_pack_buf[imu_buf_pos];
    if(user_config.imu_disabled)
        return;//disabled mask
    if(!i2c_status)return;
    if(!imu_mode)return;
    if(!imu_upd_cnt)
        imu_upd_cnt=3;
    uint32_t tick=Get_Systick_US();
    if(tick<last_imu_upd+IMU_UPD_GAP)
        return;
    //HighPrecisionTimerStart();
    if(imu_read())
    {
        printf("error on %d\r\n",Get_Systick_US());
        return;
    }else {
        for(int i=0;i<6;++i)
            imu_sum[i]+=imu_raw_buf[i];
        ++imu_sample_cnt;
        last_imu_upd=tick;
    }
    if(tick>=last_upd+IMU_FLASH_GAP)
    {
        imu_sum[0]*=imu_ratio_xf;
        imu_sum[1]*=imu_ratio_yf;
        imu_sum[2]*=imu_ratio_zf;
        for(int i=0;i<6;++i){
            if(imu_sample_cnt)
                imu_res[i]=imu_sum[i]/imu_sample_cnt;
            else
                imu_res[i]=0;
        }
        imu_sample_cnt=0;
        memset(imu_sum,0,sizeof(imu_sum));
        last_upd=tick;
        //upd acc
        memcpy(&rep->acc0,&rep->acc1,IMU_ACC_SIZE);
        memcpy(&rep->acc1,&rep->acc2,IMU_ACC_SIZE);
        memcpy(&rep->acc2,imu_raw_buf+3,IMU_ACC_SIZE);
        //memcpy(&rep->acc0,imu_res+3,IMU_ACC_SIZE);
        if(imu_mode==0x01){
            for(int i=0;i<3;++i){
                if(imu_res[i]>GYO_DATA_MAX)imu_res[i]=GYO_DATA_MAX;
                if(imu_res[i]<GYO_DATA_MIN)imu_res[i]=GYO_DATA_MIN;
            }
            memcpy(&rep->gyo0,&rep->gyo1,IMU_GYO_SIZE);
            memcpy(&rep->gyo1,&rep->gyo2,IMU_GYO_SIZE);
            memcpy(&rep->gyo2,imu_res,IMU_GYO_SIZE);
        }else if(imu_mode==0x02){
            imu_upd2(rep);
        }else{//it seems theres a mode 0x03,but we dont know whats it is. and i cant be bother to reverse engineer it
            //printf("unknowed imu mode:%d\r\n",imu_mode);
        }
        if(!--imu_upd_cnt)
        {
            set_imu_available(rep);
            imu_buf_pos=!imu_buf_pos;
            memset(&imu_pack_buf[imu_buf_pos],0,sizeof(imu_report_pack));
            imu_upd_cnt=3;
        }
    }
}
