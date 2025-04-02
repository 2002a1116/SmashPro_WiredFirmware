/*
 * rumble.c
 *
 *  Created on: 2024年11月24日
 *      Author: Reed
 */


#include "debug.h"
#include "ring_buffer.h"
#include "tick.h"
#include "rumble.h"
#include <math.h>
#include "hd_rumble.h"
#ifndef HD_RUMBLE
void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

#define RUMBLE_STEP (512)
#define RUMBLE_MOVE_FACTOR (100000)
#define CCR_TABLE_FACTOR (4)

ring_buffer rumble_rb;
uint8_t rumble_state;
uint8_t rumble_enabled;
static uint32_t last_rumble_tick=0;
static uint32_t rumble_high_tick=0,rumble_low_tick=0;
static uint32_t rumble_high_step=0,rumble_low_step=0;
static uint8_t rumble_reset=0;
static uint8_t rumble_buf[RUMBLE_FRAME_SIZE*RUMBLE_RINGBUFFER_CAP];
static rumble_frame current_rumble_frame;
static uint16_t arr_lookup_table[]={0,6097,5952,5813,5681,5555,5434,5319,5208,5102,5000,4901,4807,4716,4629,4545,4385,4310,4237,4166,4032,3968,3906,3787,3731,3623,3571,3472,3424,3333,3246,3205,3125,3048,2976,2941,2873,2808,2747,2688,2631,2577,2525,2450,2403,2358,2314,2252,2212,2155,2118,2066,2032,1984,1937,1893,1851,1824,1773,1736,1700,1666,1633,1592,1562,1524,1497,1461,1436,1404,1373,1344,1315,1288,1256,1231,1207,1179,1152,1131,1106,1082,1059,1037,1012,992,968,950,929,909,889,871,853,833,816,798,781,764,748,733,716,700,686,672,656,642,629,615,602,589,577,564,551,541,528,517,506,496,485,474,464,454,444,435,425,416,407,399,390,382,374,366,358,350,342,335,328,321,314,307,301,294,288,282,276,270,264,258,253,247,242,237,232,227,222,217,212,208,203,199};
static float_t amp_lookup_table[]={0.000,0.010,0.012,0.014,0.017,0.020,0.024,0.028,0.033,0.040,0.047,0.056,0.067,0.080,0.095,0.112,0.117,0.123,0.128,0.134,0.140,0.146,0.152,0.159,0.166,0.173,0.181,0.189,0.198,0.206,0.215,0.225,0.230,0.235,0.240,0.245,0.251,0.256,0.262,0.268,0.273,0.279,0.286,0.292,0.298,0.305,0.311,0.318,0.325,0.332,0.340,0.347,0.355,0.362,0.370,0.378,0.387,0.395,0.404,0.413,0.422,0.431,0.440,0.450,0.460,0.470,0.480,0.491,0.501,0.512,0.524,0.535,0.547,0.559,0.571,0.584,0.596,0.609,0.623,0.636,0.650,0.665,0.679,0.694,0.709,0.725,0.741,0.757,0.773,0.790,0.808,0.825,0.843,0.862,0.881,0.900,0.920,0.940,0.960,0.981,0.999};
static float_t ccr_lookup_table[RUMBLE_STEP]={0.000903588,0.00632507,0.0171678,0.0334315,0.0551154,0.0822187,0.11474,0.152679,0.196034,0.244803,
        0.298984,0.358576,0.423575,0.49398,0.569788,0.650996,0.737601,0.8296,0.926989,1.02976,1.13792,1.25146,1.37037,1.49465,1.6243,
        1.75931,1.89967,2.04538,2.19644,2.35283,2.51456,2.68162,2.85399,3.03168,3.21468,3.40298,3.59657,3.79545,3.9996,4.20903,4.42372,
        4.64367,4.86886,5.09929,5.33495,5.57583,5.82192,6.07321,6.3297,6.59137,6.85821,7.13022,7.40738,7.68968,7.97712,8.26967,8.56734,
        8.8701,9.17796,9.49088,9.80888,10.1319,10.46,10.7931,11.1312,11.4744,11.8225,12.1756,12.5337,12.8967,13.2646,13.6375,14.0152,
        14.3979,14.7855,15.1779,15.5751,15.9772,16.3842,16.7959,17.2124,17.6337,18.0597,18.4905,18.926,19.3662,19.8111,20.2607,20.715,
        21.1739,21.6374,22.1055,22.5782,23.0555,23.5373,24.0237,24.5146,25.01,25.5098,26.0141,26.5229,27.0361,27.5537,28.0757,28.602,
        29.1327,29.6677,30.207,30.7506,31.2985,31.8506,32.4069,32.9675,33.5322,34.101,34.674,35.2512,35.8324,36.4177,37.007,37.6004,
        38.1978,38.7991,39.4044,40.0137,40.6269,41.2439,41.8649,42.4897,43.1183,43.7507,44.3868,45.0267,45.6704,46.3177,46.9688,47.6235,
        48.2818,48.9437,49.6092,50.2782,50.9508,51.6269,52.3064,52.9894,53.6759,54.3657,55.0589,55.7555,56.4554,57.1585,57.865,58.5747,
        59.2876,60.0037,60.7229,61.4453,62.1708,62.8994,63.6311,64.3657,65.1034,65.844,66.5876,67.3341,68.0834,68.8357,69.5907,70.3486,
        71.1092,71.8726,72.6387,73.4075,74.1789,74.953,75.7297,76.5089,77.2907,78.075,78.8618,79.651,80.4426,81.2367,82.0331,82.8318,
        83.6329,84.4362,85.2417,86.0495,86.8595,87.6716,88.4858,89.3021,90.1205,90.9409,91.7633,92.5876,93.4139,94.2421,95.0722,95.9041,
        96.7379,97.5734,98.4106,99.2496,100.09,100.933,101.777,102.622,103.469,104.318,105.168,106.019,106.872,107.727,108.583,109.44,
        110.298,111.158,112.019,112.881,113.744,114.608,115.474,116.34,117.208,118.077,118.946,119.817,120.688,121.561,122.434,123.308,
        124.183,125.058,125.935,126.811,127.689,128.567,129.446,130.325,131.205,132.085,132.966,133.847,134.729,135.611,136.493,137.376,
        138.258,139.141,140.024,140.908,141.791,142.675,143.558,144.442,145.325,146.209,147.092,147.976,148.859,149.742,150.624,151.507,
        152.389,153.271,154.153,155.034,155.915,156.795,157.675,158.554,159.433,160.311,161.189,162.065,162.942,163.817,164.692,165.566,
        166.439,167.312,168.183,169.054,169.923,170.792,171.66,172.526,173.392,174.256,175.119,175.981,176.842,177.702,178.56,179.417,
        180.273,181.128,181.981,182.832,183.682,184.531,185.378,186.223,187.067,187.91,188.75,189.589,190.427,191.262,192.096,192.928,
        193.758,194.586,195.412,196.237,197.059,197.88,198.698,199.514,200.328,201.141,201.95,202.758,203.564,204.367,205.168,205.967,
        206.763,207.557,208.349,209.138,209.925,210.709,211.491,212.27,213.047,213.821,214.593,215.361,216.127,216.891,217.651,218.409,
        219.164,219.917,220.666,221.412,222.156,222.897,223.634,224.369,225.101,225.829,226.555,227.277,227.996,228.712,229.425,230.135,
        230.841,231.545,232.245,232.941,233.634,234.324,235.011,235.694,236.373,237.049,237.722,238.391,239.056,239.718,240.377,241.031,
        241.682,242.33,242.973,243.613,244.249,244.882,245.51,246.135,246.756,247.373,247.986,248.596,249.201,249.802,250.4,250.993,
        251.582,252.168,252.749,253.326,253.899,254.468,255.033,255.593,256.149,256.702,257.249,257.793,258.332,258.867,259.398,259.924,
        260.446,260.964,261.477,261.986,262.49,262.99,263.485,263.976,264.463,264.945,265.422,265.895,266.363,266.826,267.285,267.739,
        268.189,268.634,269.074,269.509,269.94,270.366,270.788,271.204,271.616,272.023,272.425,272.822,273.215,273.602,273.985,274.363,
        274.735,275.103,275.466,275.824,276.178,276.526,276.869,277.207,277.54,277.868,278.191,278.509,278.822,279.13,279.433,279.73,
        280.023,280.31,280.593,280.87,281.142,281.409,281.67,281.927,282.178,282.424,282.665,282.901,283.131,283.356,283.576,283.791,
        284,284.205,284.403,284.597,284.785,284.968,285.146,285.318,285.485,285.647,285.804,285.955,286.1,286.241,286.376,286.505,286.63,
        286.749,286.862,286.97,287.073,287.17,287.262,287.349,287.43,287.506,287.576,287.641,287.701,287.755,287.804,287.847,287.885,
        287.918,287.945,287.967,287.983,287.994,287.999};
static uint16_t high_ccr_tb[RUMBLE_STEP],low_ccr_tb[RUMBLE_STEP];
#define FREQ_MAX_RAW (sizeof(arr_lookup_table))
#define AMP_MAX_RAW (sizeof(amp_lookup_table))
/*
double_t decode_ccr(uint32_t v)
{
    if(v>=100)
        return 1;
    if(v>=32)    {
        return pow(2,v/32.0)/8.7;
    }
    if(v>=16)    {
        return pow(2,v/16.0)/17.0;
    }
    return pow(2,0.547*v)/1000;
}*/
uint32_t decode_time;
void cal_ccr_tb(uint16_t* tb,float_t amp)
{
    for(int i=0;i<RUMBLE_STEP;++i){
        tb[i]=(uint16_t)(ccr_lookup_table[i]*amp)<<CCR_TABLE_FACTOR;
    }
}
void decode_rumble(uint8_t* ptr,rumble_frame* rumble_data)
{
    if(!rumble_enabled)return;
    if(!(ptr[0]|ptr[1]|ptr[2]|ptr[3]))
    {
        rumble_data->high_ccr=rumble_data->low_ccr=0;
        return;
    }
    decode_time=Get_Systick_MS();
    uint8_t hf=((ptr[0]>>2)&0x3f)+0x20+((ptr[1]&1)<<6);
    uint8_t lf=(ptr[2]&0x7f);
    uint8_t ha=(ptr[1]>>1);
    uint8_t la=((ptr[3]>=0x40?(ptr[3]-0x40):0)<<1)|((ptr[2]&0x80)>>7);
    if(hf>=FREQ_MAX_RAW)
        ha=0;
    if(lf>=FREQ_MAX_RAW)
        lf=0;
    rumble_data->high_arr=arr_lookup_table[hf];
    rumble_data->low_arr=arr_lookup_table[lf];
    rumble_data->high_amp=amp_lookup_table[ha]*0.5;
    rumble_data->low_amp=amp_lookup_table[la]*0.5;
    //rumble_data->high_amp=amp_lookup_table[ha];
    //rumble_data->low_amp=amp_lookup_table[la];
    rumble_data->high_ccr=rumble_data->high_amp*rumble_data->high_arr;
    rumble_data->low_ccr=rumble_data->low_amp*rumble_data->low_arr;
    rumble_data->high_arr>>=1;
    rumble_data->low_arr>>=1;
#if (RUMBLE_MODE==RUMBLE_MODE_SIN)
    rumble_high_step=RUMBLE_MOVE_FACTOR*rumble_data->high_arr/RUMBLE_STEP;
    rumble_low_step=RUMBLE_MOVE_FACTOR*rumble_data->low_arr/RUMBLE_STEP;
    if(rumble_data->high_amp)
        cal_ccr_tb(high_ccr_tb,rumble_data->high_amp);
    if(rumble_data->low_amp)
        cal_ccr_tb(low_ccr_tb,rumble_data->low_amp);
    //rumble_data->low_amp=0;
    //rumble_data->low_amp=0;
#else
    rumble_high_step=1.237*rumble_data->high_arr/(rumble_data->high_amp*36);
    rumble_low_step=1.237*rumble_data->low_arr/(rumble_data->low_amp*36);
    rumble_data->high_ccr*=0.637;
    rumble_data->low_ccr*=0.637;
#endif
    //printf("decode gap %d\r\n",Get_Systick_MS()-decode_time);
    //rumble_data->low_ccr=0;
    //printf("hs:%f ls:%f\r\n",rumble_data->high_amp,rumble_data->low_amp);
    //printf("rumble 0x%02x 0x%02x 0x%02x 0x%02x\r\n",ptr[0],ptr[1],ptr[2],ptr[3]);
    //printf("f1:0x%02x 0x%02x  f2:0x%02x\r\n",((0x3f&(ptr[0]>>2))+0x60),((ptr[1]&1)?0x40:0),((ptr[2]&0x7f)+0x40));
    /*if(ptr[0]|(ptr[1]&1))//high byte exist
    {
        rumble_data->high_arr=(0x3f&(ptr[0]>>2));
        if(ptr[1]&1)rumble_data->high_arr+=0x40;
        //0x60~127+0x60   range:127
        rumble_data->high_arr+=0x60;
        rumble_data->high_arr=25000/pow(2,rumble_data->high_arr/32.0);
    }
    if(ptr[2]){
        rumble_data->low_arr=(ptr[2]&0x7f);
        rumble_data->low_arr+=0x40;
        rumble_data->low_arr=25000/pow(2,rumble_data->low_arr/32.0);
        //0x40 ~ 127+0x40 range:127
    }
    /*{
        rumble_data->high_arr=0;
        rumble_data->low_arr=0;
    }*/
    /*rumble_data->high_ccr=ptr[1]>>1;//0~c8/2=64 aka 100
    rumble_data->low_ccr=((ptr[3]-0x40)<<1)+(1&(ptr[2]>>7));//0~32*2=64 aka 100
    //printf("raw ccr :%d %d\r\n",rumble_data->high_ccr,rumble_data->low_ccr);
    rumble_data->high_amp=decode_ccr(rumble_data->high_ccr);
    rumble_data->low_amp=decode_ccr(rumble_data->low_ccr);
    rumble_data->high_ccr=rumble_data->high_amp*rumble_data->high_arr;
    rumble_data->low_ccr=rumble_data->low_amp*rumble_data->low_arr;
    rumble_data->high_arr>>=1;
    rumble_data->low_arr>>=1;
    rumble_high_step=rumble_data->high_arr/(rumble_data->high_amp*36);
    rumble_low_step=rumble_data->low_arr/(rumble_data->low_amp*36);
    rumble_data->high_ccr*=0.637;
    rumble_data->low_ccr*=0.637;
    rumble_data->low_ccr=0;
    printf("decode time:%d ha:%d la:%d,hc:%d,lc:%d\r\n",Get_Systick_MS()-decode_time,rumble_data->high_arr,rumble_data->low_arr,rumble_data->high_ccr,rumble_data->low_ccr);
    /*
    rumble_data->high_sin=-sin(2*M_PI/rumble_data->high_arr)*rumble_data->high_arr/(4*M_PI);
    rumble_data->low_sin=-sin(2*M_PI/rumble_data->low_arr)*rumble_data->low_arr/(4*M_PI);
    rumble_data->high_ccr=rumble_data->high_arr*decode_ccr(rumble_data->high_ccr); //ccr = arr*dutycycle%
    rumble_data->low_ccr=rumble_data->low_arr*decode_ccr(rumble_data->low_ccr);
    */
    //tmp=rumble_data->freq/32;
    //rumble_data->freq=round(pow(2.0,rumble_data->freq/32.0)*10);
    /*if(rumble_data->amp>=32){
        rumble_data->amp=pow(2.0,rumble_data->amp/32.0)/8.7;
    }
    else if(rumble_data->amp>=16){
        rumble_data->amp=pow(2.0,rumble_data->amp/16.0)/17.0;
    }
    else {
        //todo :figure out an algorithm to cal correct amp
        //rumble_data->amp=0.05;
        rumble_data->amp=pow(2,0.547*rumble_data->amp)/1000;
        //0.547*15
    }
    if(rumble_data->amp>=1)
        rumble_data->amp=0.99;
    */
}
void set_rumble_status(rumble_frame* rumble_data)// freq = 1m/period-->period=1m/freq, amp=duty_cycle
{
    /*TIM_Cmd(TIM3,DISABLE);
    if((!rumble_state)||(!rumble_data))return;
    arr=rumble_data->freq?1000000/rumble_data->freq:0;
    if(arr==0)return;
    TIM_ARRPreloadConfig(TIM3,DISABLE);
    TIM_SetAutoreload(TIM3, arr);
    TIM_SetCompare1(TIM3, arr*rumble_data->amp);
    TIM_ARRPreloadConfig(TIM3,ENABLE);
    TIM_Cmd(TIM3,ENABLE);*/
    if(!rumble_data){
        memset(&current_rumble_frame,0,sizeof(current_rumble_frame));
        //current_rumble_frame.high_ccr=current_rumble_frame.low_ccr=0;
        //printf("rumble reset\r\n");
    }
    else
        current_rumble_frame=*rumble_data;
    rumble_reset=1;
}
uint8_t push_rumble_frame(rumble_frame* ptr)
{
    //if(!ptr)return -1;
    return ring_buffer_push(&rumble_rb,(uint8_t*)ptr,RUMBLE_FRAME_SIZE,0);
}
static uint8_t is_rumbling=0;
void next_rumble_frame(){
    if(!rumble_enabled)return;
    if(Get_Systick_MS()-last_rumble_tick<RUMBLE_FRAME_TIME_MS)
        return;
    //last_rumble_tick=Get_Systick_MS();
    if(rumble_rb.size)
    {
        rumble_frame* ptr=(rumble_frame*)&rumble_rb.buf[rumble_rb.top*RUMBLE_FRAME_SIZE];
        ring_buffer_pop(&rumble_rb);
        set_rumble_status(ptr);
        is_rumbling=1;
        last_rumble_tick=Get_Systick_MS();
        //printf("start rumble\r\n");
    }
    else if(is_rumbling&&((Get_Systick_MS()-last_rumble_tick)>RUMBLE_FRAME_TIMEOUT_MS)){
        set_rumble_status(NULL);
        is_rumbling=0;
        last_rumble_tick=Get_Systick_MS();
        printf("no more frame,stop rumble\r\n");
    }
    /*
    if(Get_Systick_MS()-last_rumble_tick<RUMBLE_FRAME_TIME_MS)
        return;
    last_rumble_tick=Get_Systick_MS();
    if(rumble_rb.size)
    {
        rumble_frame* ptr=(rumble_frame*)&rumble_rb.buf[rumble_rb.top*RUMBLE_FRAME_SIZE];
        ring_buffer_pop(&rumble_rb);
        set_rumble_status(ptr);
        is_rumbling=1;
        //printf("start rumble\r\n");
    }
    else if(is_rumbling){
        set_rumble_status(NULL);
        is_rumbling=0;
        printf("no more frame,stop rumble\r\n");
    }
    */
}
void rumble_init(uint8_t enable){
    GPIO_InitTypeDef        GPIO_InitStructure = {0};
    TIM_OCInitTypeDef       TIM_OCInitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    if(!enable){
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        return;
    }

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    TIM_TimeBaseInitStructure.TIM_Period = 288;//72m/288
    TIM_TimeBaseInitStructure.TIM_Prescaler = 0;//
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

#if(PWM_MODE == PWM_MODE1)
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
#elif(PWM_MODE == PWM_MODE2)
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
#endif
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);

    TIM_CtrlPWMOutputs(TIM3, ENABLE);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Disable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_SetCompare1(TIM3, 0);
    TIM_Cmd(TIM3, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure; //定义NVIC初始化结构体
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; //设置NVIC通道为定时器2中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //设置NVIC通道抢占优先级为0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; //设置NVIC通道子优先级为0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能NVIC通道
    NVIC_Init(&NVIC_InitStructure); //初始化NVIC

    ring_buffer_init(&rumble_rb, rumble_buf, RUMBLE_RINGBUFFER_CAP, RUMBLE_FRAME_SIZE);
    rumble_enabled=1;
}
static uint16_t rumble_ccr;
#define RUMBLE_CRR_RESET (0X8000)
static uint8_t rumble_phase,rumble_set=0;
static uint32_t rumble_high_cnt=0,rumble_low_cnt=0;
static uint16_t rumble_high_pos=0,rumble_low_pos=0;
void TIM3_IRQHandler(void)//定时器3中断服务函数，硬件自动调用，不需要手动调用
{
    if(TIM_GetFlagStatus(TIM3,TIM_FLAG_Update)==1)//判断定时器2更新标志位是否产生
    {
        //printf("tim3 irq\r\n");
        //rumble_ccr=0;
        if(rumble_reset)
        {
            rumble_phase=0;
            rumble_ccr=RUMBLE_CRR_RESET;
            rumble_high_cnt=rumble_low_cnt=rumble_high_tick=rumble_low_tick=0;
            rumble_high_pos=rumble_low_pos=0;
            rumble_reset=0;
        }
        if(rumble_ccr&&!(current_rumble_frame.high_amp||current_rumble_frame.low_amp))
            rumble_ccr=RUMBLE_CRR_RESET;

#if(RUMBLE_MODE == RUMBLE_MODE_SIN)
        if(current_rumble_frame.high_amp)
        {
            rumble_high_tick++;
            rumble_high_cnt+=RUMBLE_MOVE_FACTOR;
            if(rumble_high_tick>=current_rumble_frame.high_arr){
                rumble_high_tick=0;
                rumble_phase^=0x1;
            }
            if(rumble_high_cnt>=rumble_high_step)
            {
                while(rumble_high_cnt>=rumble_high_step)
                {
                    rumble_high_cnt-=rumble_high_step;
                    if(rumble_phase&0x1){//down
                        rumble_high_pos--;
                    }
                    else {
                        rumble_high_pos++;
                    }
                }
                rumble_set=1;
            }
        }
        if(current_rumble_frame.low_amp)
        {
            rumble_low_tick++;
            rumble_low_cnt+=RUMBLE_MOVE_FACTOR;
            if(rumble_low_tick>=current_rumble_frame.low_arr){
                rumble_low_tick=0;
                rumble_phase^=0x2;
            }
            if(rumble_low_cnt>=rumble_low_step)
            {
                while(rumble_low_cnt>=rumble_low_step)
                {
                    rumble_low_cnt-=rumble_low_step;
                    if(rumble_phase&0x2){//down
                        rumble_low_pos--;
                    }
                    else {
                        rumble_low_pos++;
                    }
                }
                rumble_set=1;
            }
        }
        if(rumble_set)
        {
            rumble_set=0;
            rumble_ccr=RUMBLE_CRR_RESET|(((uint16_t)(high_ccr_tb[rumble_high_pos]+low_ccr_tb[rumble_low_pos])>>CCR_TABLE_FACTOR)/2);
            //printf("high ccr:%d low_ccr:%d hp:%d lp:%d\r\n",high_ccr_tb[rumble_high_pos]>>CCR_TABLE_FACTOR,low_ccr_tb[rumble_low_pos],
            //        rumble_high_pos,rumble_low_pos);
        }
#elif(RUMBLE_MODE == RUMBLE_MODE_LINEAR)
        if(current_rumble_frame.high_ccr)
        {
            rumble_high_tick++;
            rumble_high_cnt++;
            if(rumble_high_tick>=current_rumble_frame.high_arr){
                rumble_high_tick=0;
                if(rumble_phase&0x1){
                    rumble_phase^=0x4;
                }
                rumble_phase^=0x1;
            }
            //if(rumble_phase&0x4)
            {
                if(rumble_high_cnt>=rumble_high_step)
                {
                    rumble_high_cnt-=rumble_high_step;
                    if(rumble_phase&0x1)
                    {
                        ++rumble_ccr;
                    }
                    else {
                        --rumble_ccr;
                    }
                    rumble_ccr|=RUMBLE_CRR_RESET;
                }
            }

        }
        if(current_rumble_frame.low_ccr)
        {
            rumble_low_tick++;
            rumble_low_cnt++;
            if(rumble_low_tick>=current_rumble_frame.low_arr){
                rumble_low_tick=0;
                if(rumble_phase&0x2)
                {
                    rumble_phase^=0x8;
                }
                rumble_phase^=0x2;
            }
            //if(rumble_phase&0x8)
            {
                if(rumble_low_cnt>=rumble_low_step)
                {
                    rumble_low_cnt=0;
                    if(rumble_phase&0x2)
                    {
                        ++rumble_ccr;
                    }
                    else {
                        --rumble_ccr;
                    }
                    rumble_ccr|=RUMBLE_CRR_RESET;
                }
            }

        }
#else
        if(current_rumble_frame.high_ccr){
            rumble_high_tick++;
            if(rumble_high_tick>=current_rumble_frame.high_arr){
                rumble_high_tick=0;
                rumble_phase^=0x1;
                if(rumble_phase&0x1)
                {
                    rumble_ccr+=current_rumble_frame.high_ccr;
                }
                else {
                    rumble_ccr-=current_rumble_frame.high_ccr;
                }
                rumble_ccr|=RUMBLE_CRR_RESET;
            }
        }
        if(current_rumble_frame.low_ccr){
            rumble_low_tick++;
            if(rumble_low_tick>=current_rumble_frame.low_arr){
                rumble_low_tick=0;
                rumble_phase^=0x2;
                if(rumble_phase&0x2)
                {
                    rumble_ccr+=current_rumble_frame.low_ccr;
                }
                else {
                    rumble_ccr-=current_rumble_frame.low_ccr;
                }
                rumble_ccr|=RUMBLE_CRR_RESET;
            }
        }
#endif
        if(RUMBLE_CRR_RESET&rumble_ccr)
        {
            rumble_ccr^=RUMBLE_CRR_RESET;
            if(rumble_ccr<288)
                TIM_SetCompare1(TIM3, rumble_ccr);
            else {
                TIM_SetCompare1(TIM3, 288);
            }
        }
        //rumble_ccr=0x80;
        //if(!(current_rumble_frame.high_ccr|current_rumble_frame.low_ccr)&&rumble_ccr)
        /*if(current_rumble_frame.high_amp){
            rumble_high_tick++;
            if(rumble_high_tick>=current_rumble_frame.high_arr){
                rumble_high_tick=0;
                //rumble_ccr+=30;
                //rumble_ccr|=0x80;
                //printf("high freq clock reset\r\n");
            }else if(rumble_high_tick==current_rumble_frame.high_ccr){
                //rumble_ccr-=30;
                //rumble_ccr|=0x80;
            }
            rumble_ccr+=72*current_rumble_frame.high_amp*(0.5+current_rumble_frame.high_sin*cos(2*M_PI*(2*rumble_high_tick+1)/current_rumble_frame.high_arr));
            //printf("rumble_ccr %d %f %f %f\r\n",rumble_ccr,current_rumble_frame.high_amp,current_rumble_frame.high_sin,cos(2*M_PI*(2*rumble_high_tick+1)/current_rumble_frame.high_arr));
        }
        if(current_rumble_frame.low_amp){
            rumble_low_tick++;
            if(rumble_low_tick>=current_rumble_frame.low_arr){
                rumble_low_tick=0;
                //rumble_ccr+=30;
                //rumble_ccr|=0x80;
                //printf("low freq clock reset\r\n");
            }else if(rumble_low_tick==current_rumble_frame.low_ccr){
                //rumble_ccr-=30;
                //rumble_ccr|=0x80;
            }
            rumble_ccr+=72*current_rumble_frame.low_amp*(0.5+current_rumble_frame.low_sin*cos(2*M_PI*(2*rumble_low_tick+1)/current_rumble_frame.low_arr));
        }
        //printf("rumble_ccr %d\r\n",rumble_ccr);
        TIM_SetCompare1(TIM3, rumble_ccr);
        /*if(rumble_ccr&0x80)//set
        {
            rumble_ccr^=0x80;
            //printf("set c:%d %d %d %d %d\r\n",rumble_ccr,current_rumble_frame.high_arr,current_rumble_frame.low_arr
            //        ,current_rumble_frame.high_ccr,current_rumble_frame.low_ccr);
            TIM_SetCompare1(TIM3, rumble_ccr);
        }*/
        TIM_ClearITPendingBit(TIM3, TIM_FLAG_Update); //清除定时器2更新标志位
    }
}
#endif
