/*
 * joystick.c
 *
 *  Created on: 2026Äê6ÔÂ7ÈÕ
 *      Author: Reed
 */
#include "joystick.h"
#include "gpio_adc.h"
#include "conf.h"

static int16_t adc_debounce[4];
coord_compact sts_joy[2];
coord sts_joy_raw[2];
typedef struct{
    uint32_t sample_timestamp;
    int32_t x;
    int32_t y;
    int32_t len_sq;
}js_sample;
static js_sample js_snapback_samples[2];
uint32_t joystick_snapback_filter(int32_t x,int32_t y,uint8_t id){
    static uint8_t set=0;
    //deadzone is fixed
    //snapback deadzone is influenced by joystick ratio,as its value is set by output x & y
    if(config.snpbk.filter_window){
        set=0;
        int32_t len_sq=x*x+y*y;
        if(len_sq>=joystick_snapback_deadzone_sq[id])
            set=1;
        uint32_t timestamp=Get_Systick_US();
        if(!timestamp)timestamp=1;
        do{
            if(!js_snapback_samples[id].sample_timestamp)//no valid sample,skip
                break;
            if(timestamp-js_snapback_samples[id].sample_timestamp>=config.snpbk.filter_window){
                js_snapback_samples[id].sample_timestamp=0;//invalid sample
                break;
            }
            int64_t dp_sq=js_snapback_samples[id].x*x+js_snapback_samples[id].y*y;
            if(dp_sq>=0)
                break;
            if((dp_sq*dp_sq)<((((uint64_t)len_sq)*js_snapback_samples[id].len_sq)>>2))
                break;
            //prob snapback
            //force center
            //if prob snapback,dont upd sample as it was removed
            set=0;
            x=y=0;
        }while(0);
        //reset sample
        if(set){
            js_snapback_samples[id].x=x;
            js_snapback_samples[id].y=y;
            js_snapback_samples[id].len_sq=len_sq;
            js_snapback_samples[id].sample_timestamp=timestamp;
        }
    }
    x+=JOYSTICK_CENTER;
    y+=JOYSTICK_CENTER;
    //x=i32_clamp(x, 0, 4095);
    //y=i32_clamp(y, 0, 4095);
    if(config.js.normalization){
        //todo:this is broken,fix this
        //todo2:forget this,just use affine instead
        int32_t nx=0,ny=0,px=4095,py=4095;
        nx=factory_configuration.JoystickCalibrationValue.AnalogStickFactoryCalibrationValue[id].AnalogStickCalXNegative;
        px=factory_configuration.JoystickCalibrationValue.AnalogStickFactoryCalibrationValue[id].AnalogStickCalXPositive;
        ny=factory_configuration.JoystickCalibrationValue.AnalogStickFactoryCalibrationValue[id].AnalogStickCalYNegative;
        py=factory_configuration.JoystickCalibrationValue.AnalogStickFactoryCalibrationValue[id].AnalogStickCalYPositive;
        px=nx=i32_min(nx, px);
        py=ny=i32_min(ny, py);
        x=i32_clamp(x, 2048-nx, 2048+px);
        y=i32_clamp(y, 2048-ny, 2048+py);
    }
    x=i32_clamp(x, 0, 4095);
    y=i32_clamp(y, 0, 4095);
    return (y<<12)+x;
}
void joystick_set_dz(int32_t *x,int32_t *y,int id){
    switch(config.js.dz_mode)
    {
    case 1:
        if((abs(*x)<config.js.dz[id<<1])&&(abs(*y)<config.js.dz[1+(id<<1)]))
            *x=*y=0;
        break;
    case 2:
        if(abs(*x)<config.js.dz[id<<1]) *x=0;
        if(abs(*y)<config.js.dz[1+(id<<1)]) *y=0;
        break;
    case 3:
        if(abs(*x)<config.js.dz[id<<1]) *x=0;
        else *x+=(*x>0?-1:1)*config.js.dz[id<<1];
        if(abs(*y)<config.js.dz[1+(id<<1)]) *y=0;
        else *y+=(*y>0?-1:1)*config.js.dz[1+(id<<1)];
        break;
    default:
        break;
    }
}
void joystick_ratio_scale(int32_t* x,int32_t* y,int32_t id){
    *x=(*x*config.js.ratio[id<<1])>>5;
    *y=(*y*config.js.ratio[(id<<1)+1])>>5;
}
void joystick_upd(){
    memcpy(adc_debounce,adc_data,sizeof(adc_data));
    /*for(int i=0;i<4;++i){
        adc_debounce[i]=adc_data[i];
    }*/
    int32_t y=(adc_debounce[ADC_CHANNEL_LJOYS_VERT]-config.js.center[1]);
    int32_t x=(adc_debounce[ADC_CHANNEL_LJOYS_HORI]-config.js.center[0]);
    joystick_set_dz(&x,&y,0);
    sts_joy_raw[0].x = x;
    sts_joy_raw[0].y = y;
    //sts_joy_raw[0].x = i32_clamp(x, JOYSTICK_NRANGE, JOYSTICK_PRANGE);
    //sts_joy_raw[0].y = i32_clamp(y, JOYSTICK_NRANGE, JOYSTICK_PRANGE);
    switch(config.js.l_mode){
        case 0:
            joystick_ratio_scale(&x, &y, 0);
            break;
        case 1:
            joystick_affine_scale(&x, &y, 0);
            break;
        default:
            break;
    }
    x = i32_clamp(x, JOYSTICK_NRANGE, JOYSTICK_PRANGE);
    y = i32_clamp(y, JOYSTICK_NRANGE, JOYSTICK_PRANGE);
    sts_joy[0].data=joystick_snapback_filter(x,y,0);

    y=(adc_debounce[ADC_CHANNEL_RJOYS_VERT]-config.js.center[3]);
    x=(adc_debounce[ADC_CHANNEL_RJOYS_HORI]-config.js.center[2]);
    joystick_set_dz(&x,&y,0);
    sts_joy_raw[1].x = x;
    sts_joy_raw[1].y = y;
    //sts_joy_raw[0].x = i32_clamp(x, JOYSTICK_NRANGE, JOYSTICK_PRANGE);
    //sts_joy_raw[0].y = i32_clamp(y, JOYSTICK_NRANGE, JOYSTICK_PRANGE);
    switch(config.js.r_mode){
        case 0:
            joystick_ratio_scale(&x, &y, 1);
            break;
        case 1:
            joystick_affine_scale(&x, &y, 1);
            break;
        default:
            break;
    }
    x = i32_clamp(x, JOYSTICK_NRANGE, JOYSTICK_PRANGE);
    y = i32_clamp(y, JOYSTICK_NRANGE, JOYSTICK_PRANGE);
    sts_joy[1].data=joystick_snapback_filter(x,y,1);
}
