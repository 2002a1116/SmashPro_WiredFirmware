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
#include "watchdog.h"
#include "uart.h"
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
    if(ret=i2c_read_byte(reg, &buf)){
        return ret;
    }
    buf = (buf&(~mask))|value;
    ret=i2c_write_byte(reg, buf);
    return ret;
}

void imu_set_acc_bandwidth(uint8_t BW0XL, uint8_t ODR)
{
    imu_set_reg(LSM6DS3TRC_CTRL1_XL, BW0XL, 0);
    imu_set_reg(LSM6DS3TRC_CTRL8_XL, ODR, 0);
}
uint32_t imu_read_cnt,imu_read_fail_cnt;
uint32_t imu_id_read_fail_cnt;
uint8_t imu_error=0;
uint8_t imu_read(){
    uint8_t status=0;
    uint8_t ret;
    if((ret=i2c_read_byte(IMU_ID_REG, &status))||(status!=IMU_ID_LSM6DS3&&status!=IMU_ID_LSM6DS3TRC)){
        imu_id_read_fail_cnt++;
        if((!imu_error)&&(imu_id_read_fail_cnt>=10)){
            imu_error=1;
            flush_rgb(ENABLE);
        }
    }else {
        imu_id_read_fail_cnt=0;
        if(imu_error)
            flush_rgb(ENABLE);
        imu_error=0;
        //flush_rgb(ENABLE);
    }
    if(imu_error)return 3;
    if(ret=i2c_read_byte(LSM6DS3TRC_STATUS_REG, &status)){
        ++imu_read_cnt;
        ++imu_read_fail_cnt;
        return ret;
    }
    if((status&LSM6DS3TRC_STATUS_ACCELEROMETER)&&(status&LSM6DS3TRC_STATUS_GYROSCOPE)){
        ++imu_read_cnt;
        if(i2c_read_continuous(LSM6DS3TRC_OUTX_L_G, imu_raw_buf, 12)){
            ++imu_read_fail_cnt;
            return 1;
        }
        else {
            return 0;
        }
    }
    return 2;
}
void set_imu_awake()
{
    uint8_t ret=0,retry=5;
    retry=5;
    do{
        ret = imu_set_reg(0x12,0x01,0xff);
    }while(ret&&retry--);
    Delay_Ms(10);
    retry=5;
    do{
        ret = imu_set_reg(0x13, 0x80, 0xFF);//if we set DRDY_MASK,imu seems crash?
    }while(ret&&retry--);
    retry=5;
    do{
        ret = imu_set_reg(0x10,0x8e,0xff);
    }while(ret&&retry--);
    retry=5;
    do{
        ret = imu_set_reg(0x11,0x5c,0xff);
    }while(ret&&retry--);
    retry=5;
    do{
        ret = imu_set_reg(0x12,0x44,0xff);//set Block Data Update
    }while(ret&&retry--);
    retry=5;
    /*do{
        ret = imu_set_reg(0x14, 0x00, 0xFF);
    }while(ret&&retry--);
    retry=5;
    do{
        ret = imu_set_reg(0x15, 0x00, 0xFF);
    }while(ret&&retry--);
    retry=5;
    do{
        ret = imu_set_reg(0x16, 0x00, 0xff);
    }while(ret&&retry--);
    retry=5;
    do{
        ret = imu_set_reg(0x17, 0x84, 0xFF);
    }while(ret&&retry--);
    retry=5;
    do{
        ret = imu_set_reg(0x18, 0x38, 0xFF);
    }while(ret&&retry--);
    retry=5;
    do{
        ret = imu_set_reg(0x19, 0x3C, 0xFF);
    }while(ret&&retry--);
    */
    //todo:not sure about hpf freq.
    imu_rdy=!ret;
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
    if(user_config.imu_disabled){
        memset(rep,0,IMU_GYO_SIZE*6);
        set_imu_available(rep);
        return;//disabled mask
    }
    if(!i2c_status)return;
    if(!imu_mode)return;
    if(!imu_upd_cnt)
        imu_upd_cnt=3;
    uint32_t tick=Get_Systick_US();
    if(tick-last_imu_upd<user_config.imu_sample_gap)
        return;
    if(imu_read())
    {
        //printf("error on %d\r\n",Get_Systick_MS());
        return;
    }else {
        for(int i=0;i<6;++i)
            imu_sum[i]+=imu_raw_buf[i];
        ++imu_sample_cnt;
        last_imu_upd=tick;
    }
    if(tick-last_upd>=IMU_FLASH_GAP)
    {
        imu_sum[0]*=imu_ratio_xf;
        imu_sum[1]*=imu_ratio_yf;
        imu_sum[2]*=imu_ratio_zf;
        /*imu_sum[3]*=0.95f;
        imu_sum[4]*=0.95f;
        imu_sum[5]*=0.95f;*/
        for(int i=0;i<6;++i){
            if(imu_sample_cnt)
                imu_res[i]=imu_sum[i]/imu_sample_cnt;
            else
                imu_res[i]=0;
            //imu_res[i]&=0xffB0;
        }
        imu_sample_cnt=0;
        memset(imu_sum,0,sizeof(imu_sum));
        last_upd=tick;
        //upd acc

        memcpy(&rep->acc0,&rep->acc1,IMU_ACC_SIZE);
        memcpy(&rep->acc1,&rep->acc2,IMU_ACC_SIZE);
        memcpy(&rep->acc2,imu_res+3,IMU_ACC_SIZE);
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
            ////printf("unknowed imu mode:%d\r\n",imu_mode);
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
