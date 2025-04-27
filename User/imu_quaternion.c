/*
 * imu_quaternion.c
 *
 *  Created on: 2025年2月28日
 *      Author: Reed
 */
/*
* imu_quaternion.c
*
*  Created on: 2025年2月28日
*      Author: Reed
*/
#include "debug.h"
#include "tick.h"
#include "ns_com.h"
#include "imu.h"
typedef struct
{
    union
    {
        float raw[4];
        struct {
            float x;
            float y;
            float z;
            float w;
        };
    };
    //uint32_t timestamp;
    int16_t ax;
    int16_t ay;
    int16_t az;
    uint32_t timestamp;
} quaternion_s;
quaternion_s _imu_quat_state = {.w = 1};
//here is some guessing.
//we still store 3 samples in mode 0x02,but we store gyro data by quaternion with delta and acc by raw.
//加速度传感器的静态稳定性更好,但在运动时其数据相对不可靠,而陀螺仪的动态稳定性更好,但在静止时数据相对不可靠
//because acc is more accuracy when stationary and gyro is more believable when moving.
//so we want acc and quat to have higher precision and gyro delta with lower precision as lower bit are not believable in the beginning
//and this make sense!
//so the problem left is what is delta,gyro data with bit shift or quaternion parts?
//i asume its the previous one.
//upd::gyro max dif less than 2^14,maybe they just droped high bit which isnt used
/*
float InvSqrt (float x)
{
    float xhalf = 0.5f*x;
    int i = *(int*)&x;
    i = 0x5f3759df - (i >> 1);        // 计算第一个近似根
    x = *(float*)&i;
    x = x*(1.5f - xhalf*x*x);       // 牛顿迭代法
    return x;
}*/
void _imu_quat_normalize(quaternion_s *data)
{
    float norm_inverse = 1.0f / sqrtf(data->x * data->x + data->y * data->y + data->z * data->z + data->w * data->w);
    data->x *= norm_inverse;
    data->y *= norm_inverse;
    data->z *= norm_inverse;
    data->w *= norm_inverse;
}
void _imu_rotate_quaternion(quaternion_s *first, quaternion_s *second) {
    float w = first->w * second->w - first->x * second->x - first->y * second->y - first->z * second->z;
    float x = first->w * second->x + first->x * second->w + first->y * second->z - first->z * second->y;
    float y = first->w * second->y - first->x * second->z + first->y * second->w + first->z * second->x;
    float z = first->w * second->z + first->x * second->y - first->y * second->x + first->z * second->w;

    first->w = w;
    first->x = x;
    first->y = y;
    first->z = z;
}

//#define M_PI (3.14159265f)
#define GYO_DATA_MAX (6685)
#define GYO_DATA_MIN (-6685)
#define M_PI (3.14159265)
#define SCALE_FACTOR (61.0* M_PI/1000.0  / 180.0 / 1000000.0)
//#define SCALE_FACTOR (40.0* M_PI/1000.0  / 180.0 / 1000000.0) //good for raw
void _imu_update_quaternion(imu_pack *acc,imu_pack *gyo, uint32_t timestamp) {
    // Previous timestamp (in microseconds)
    static uint32_t prev_timestamp = 0;
    //float dt = fabsf((float)timestamp - (float)prev_timestamp);
    double dt=fabs(timestamp-prev_timestamp);
    prev_timestamp = timestamp;

    /*if(gyo->data[0]>GYO_DATA_MAX)gyo->data[0]=GYO_DATA_MAX;
    else if(gyo->data[0]<GYO_DATA_MIN)gyo->data[0]=GYO_DATA_MIN;
    if(gyo->data[1]>GYO_DATA_MAX)gyo->data[1]=GYO_DATA_MAX;
    else if(gyo->data[1]<GYO_DATA_MIN)gyo->data[1]=GYO_DATA_MIN;
    if(gyo->data[2]>GYO_DATA_MAX)gyo->data[2]=GYO_DATA_MAX;
    else if(gyo->data[2]<GYO_DATA_MIN)gyo->data[2]=GYO_DATA_MIN;*/
    double angle_x = gyo->data[0] * dt * SCALE_FACTOR ; // GX
    double angle_y = gyo->data[1] * dt * SCALE_FACTOR ; // GY
    double angle_z = gyo->data[2] * dt * SCALE_FACTOR ; // GZ
    // Euler to quaternion (in a custom Nintendo way)
    double norm_squared = angle_x * angle_x + angle_y * angle_y + angle_z * angle_z;
    double first_formula = norm_squared * norm_squared / 3840.0f - norm_squared / 48.0f + 0.5f;
    double second_formula = norm_squared * norm_squared / 384.0f - norm_squared / 8.0f + 1.0f;


    quaternion_s newstate = {
      .x = angle_x * first_formula,
      .y = angle_y * first_formula,
      .z = angle_z * first_formula,
      .w = second_formula
    };
    _imu_rotate_quaternion(&_imu_quat_state, &newstate);

    _imu_quat_normalize(&_imu_quat_state);
    ////printf("new quat %f %f %f %f\r\n",_imu_quat_state.x,_imu_quat_state.y,_imu_quat_state.z,_imu_quat_state.w);

    _imu_quat_state.ax = acc->data[0];
    _imu_quat_state.ay = acc->data[1];
    _imu_quat_state.az = acc->data[2];
}
#define QUAT_SHIFT (10)
void switch_motion_pack_quat(quaternion_s *in, imu_report_pack *out)
{
    static uint32_t timestamp = 0;
    static int16_t gyo_samples[3][3];
    static uint8_t pos=0;
    static uint8_t max_index = 0;
    static int32_t quaternion_30bit_components[3];
    out->mode = 2;
    if(imu_upd_cnt==3){//start clean
        timestamp=Get_Systick_MS();
        memset(gyo_samples,0,sizeof(gyo_samples));
        pos=0;
    }
    memcpy(gyo_samples[pos++],imu_res,IMU_GYO_SIZE);
    if(pos==3){
        out->max_index = 0;
        // Insert into the last sample components, do bit operations to account for split data
        for(uint8_t i = 1; i < 4; i++){
            if(fabsf(in->raw[i]) > fabsf(in->raw[out->max_index]))
                out->max_index = i;
        }
        for (int i = 0; i < 3; ++i) {
            quaternion_30bit_components[i] = in->raw[(max_index + i + 1) & 3] * 0x40000000 * (in->raw[max_index] < 0 ? -1 : 1);
        }
        out->last_sample_0 = quaternion_30bit_components[0] >> QUAT_SHIFT;
        out->last_sample_1 = (quaternion_30bit_components[1] >> QUAT_SHIFT);
        out->last_sample_2l = ((quaternion_30bit_components[2] >> QUAT_SHIFT) & 0x3);
        out->last_sample_2h = ((quaternion_30bit_components[2] >> QUAT_SHIFT) & 0x1FFFFC) >> 2;
        out->timestamp_start=timestamp;
        out->timestamp_count=3;
        //out->timestamp_count=15;
        //moded by reed base on guessing
        //todo:finish this
        out->delta_last_first_0 = (imu_res[0]-gyo_samples[0][0])>>3;
        out->delta_last_first_1 = (imu_res[1]-gyo_samples[0][1])>>3;
        out->delta_last_first_2l = ((imu_res[2]-gyo_samples[0][2])>>3)&0x7;
        out->delta_last_first_2h = (imu_res[2]-gyo_samples[0][2])>>6;
        out->delta_mid_avg_0 = (gyo_samples[1][0]-((imu_res[0]+gyo_samples[0][0]+gyo_samples[1][0])/3))>>9;
        out->delta_mid_avg_1 = (gyo_samples[1][1]-((imu_res[1]+gyo_samples[0][1]+gyo_samples[1][1])/3))>>9;
        out->delta_mid_avg_2 = (gyo_samples[1][2]-((imu_res[2]+gyo_samples[0][2]+gyo_samples[1][2])/3))>>9;
    }
}
void imu_upd2(){
    _imu_update_quaternion(imu_res+3,imu_res,Get_Systick_US());
    switch_motion_pack_quat(&_imu_quat_state, &imu_pack_buf[imu_buf_pos]);
    ////printf("imu ipd2 clk:%d\r\n",HighPrecisionTimerCnt());
}
