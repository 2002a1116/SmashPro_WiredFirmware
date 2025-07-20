/*
 * hd_rumble2.c
 *
 *  Created on: 2024Äê12ÔÂ17ÈÕ
 *      Author: Reed
 */

#include "debug.h"
#include "hd_rumble2.h"
#include "hd_rumble.h"
#include "hd_rumble_high_accuracy.h"
#include "conf.h"
//0.03125==1 aka (x32) as 32 is the resolution factor
uint32_t exp2_lookup_tb[]={0,
        0,67,68,70,71,73,74,76,78,79,81,83,85,87,89,
        91,92,95,97,99,101,103,105,108,110,112,115,117,120,123,
        125,128,131,134,137,140,143,146,149,152,156,159,162,166,170,
        173,177,181,185,189,193,197,202,206,211,215,220,225,230,235,
        240,245,251,256,262,267,273,279,285,292,298,304,311,318,325,
        332,339,347,354,362,370,378,386,395,403,412,421,431,440,450,
        459,470,480,490,501,512,523,535,546,558,571,583,596,609,622,
        636,650,664,679,693,709,724,740,756,773,790,807,825,843,861,
        880,899,919,939,960,981,1002,1024,1046,1069,1093,1117,1141,1166,1192,
        1218,1244,1272,1300,1328,1357,1387,1417,1448,1480,1512,1545,1579,1614,1649,
        1685,1722,1760,1798,1838,1878,1919,1961,2004,2048,2093,2139,2186,2233,2282,
        2332,2383,2435,2489,2543,2599,2656,2714,2774,2834,2896,2960,3025,3091,3158,
        3228,3298,3371,3444,3520,3597,3676,3756,3838,3922,4008,4096,4186,4277,4371,
        4467,4565,4664,4767,4871,4978,5087,5198,5312,5428,5547,5668,5793,5919,6049,
        6182,6317,6455,6597,6741,6889,7039,7194,7351,7512,7677,7845,8016,8192,8371,
        8555,8742,8933,9129,9329,9533,9742,9955,10173,10396,10624,10856,11094,11337,11585,
        11839,12098,12363,12634,12910,13193,13482,13777,14079,14387,14702,15024,15353,15689,16033,
        16384,16743,17109,17484,17867,18258,18658,19066,19484,19911,20347,20792,21247,21713,22188,
        22674,23170,23678,24196,24726,25268,25821,26386,26964,27554,28158,28774,29405,30048,30706,
        31379,32066,32768,33486,34219,34968,35734,36516,37316,38133,38968,39821,40693,41584,42495,
        43425,44376,45348,46341,47356,48393,49452,50535,51642,52773,53928,55109,56316,57549,58809,
        60097,61413,62757,64132};
/*uint8_t amp_exp2_index_lookup_tb[]={0,
        16,24,32,40,48,56,64,72,80,88,96,104,112,120,128,
        130,132,134,136,138,140,142,144,146,148,150,152,154,156,158,
        160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,
        175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,
        190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,
        205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,
        220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,
        235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,
        250,251,252,253,254,255,255};
*/
uint16_t amp_exp2_index_lookup_tb_low_amp_rise[]={76,80,86,88,92,96,100,104};
uint16_t amp_exp2_index_lookup_tb_low_amp_raw[]={44,52,59,68,76,84,91,99};
uint16_t amp_exp2_index_lookup_tb[]={
        0,44,52,59,68,76,84,91,99,108,115,123,132,140,148,
        155,157,160,162,164,166,168,170,172,174,175,178,180,182,184,
        186,188,189,190,191,192,193,194,195,196,197,198,199,200,201,
        202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,
        217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,
        232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,
        247,248,249,250,251,252,253,254,255,256,257,258,259,260,261,
        262,263,264,265,266,267,268,269,270,271,272,273,274,275,276,
        277,278,279,280,281,282,283,284};
uint16_t freq_exp2_index_lookup_tb[]={192,
        193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
        208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,
        223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,
        238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,
        253,254,255,256,257,258,259,260,261,262,263,264,265,266,267,
        268,269,270,271,272,273,274,275,276,277,278,279,280,281,282,
        283,284,285,286,287,288,289,290,291,292,293,294,295,296,297,
        298,299,300,301,302,303,304,305,306,307,308,309,310,311,312,
        313,314,315,316,317,318,319};
struct Switch5BitCommand CommandTable[] = {//offset*=32
        { .am_action = Switch5BitAction_Default,    .fm_action = Switch5BitAction_Default,    .am_offset =  0,     .fm_offset =  0 },
        { .am_action = Switch5BitAction_Substitute, .fm_action = Switch5BitAction_Ignore,     .am_offset =  0,     .fm_offset =  0 },
        { .am_action = Switch5BitAction_Substitute, .fm_action = Switch5BitAction_Ignore,     .am_offset = -16,    .fm_offset =  0 },
        { .am_action = Switch5BitAction_Substitute, .fm_action = Switch5BitAction_Ignore,     .am_offset = -32,    .fm_offset =  0 },
        { .am_action = Switch5BitAction_Substitute, .fm_action = Switch5BitAction_Ignore,     .am_offset = -48,    .fm_offset =  0 },
        { .am_action = Switch5BitAction_Substitute, .fm_action = Switch5BitAction_Ignore,     .am_offset = -64,    .fm_offset =  0 },
        { .am_action = Switch5BitAction_Substitute, .fm_action = Switch5BitAction_Ignore,     .am_offset = -80,    .fm_offset =  0 },
        { .am_action = Switch5BitAction_Substitute, .fm_action = Switch5BitAction_Ignore,     .am_offset = -96,    .fm_offset =  0 },
        { .am_action = Switch5BitAction_Substitute, .fm_action = Switch5BitAction_Ignore,     .am_offset = -112,   .fm_offset =  0 },
        { .am_action = Switch5BitAction_Substitute, .fm_action = Switch5BitAction_Ignore,     .am_offset = -128,   .fm_offset =  0 },
        { .am_action = Switch5BitAction_Substitute, .fm_action = Switch5BitAction_Ignore,     .am_offset = -144,   .fm_offset =  0 },
        { .am_action = Switch5BitAction_Substitute, .fm_action = Switch5BitAction_Ignore,     .am_offset = -160,   .fm_offset =  0 },
        { .am_action = Switch5BitAction_Ignore,     .fm_action = Switch5BitAction_Substitute, .am_offset =  0,     .fm_offset = -12},
        { .am_action = Switch5BitAction_Ignore,     .fm_action = Switch5BitAction_Substitute, .am_offset =  0,     .fm_offset = -6 },
        { .am_action = Switch5BitAction_Ignore,     .fm_action = Switch5BitAction_Substitute, .am_offset =  0,     .fm_offset =  0 },
        { .am_action = Switch5BitAction_Ignore,     .fm_action = Switch5BitAction_Substitute, .am_offset =  0,     .fm_offset =  6 },
        { .am_action = Switch5BitAction_Ignore,     .fm_action = Switch5BitAction_Substitute, .am_offset =  0,     .fm_offset =  12},
        { .am_action = Switch5BitAction_Sum,        .fm_action = Switch5BitAction_Sum,        .am_offset =  4,   .fm_offset =  1 },
        { .am_action = Switch5BitAction_Sum,        .fm_action = Switch5BitAction_Ignore,     .am_offset =  4,   .fm_offset =  0   },
        { .am_action = Switch5BitAction_Sum,        .fm_action = Switch5BitAction_Sum,        .am_offset =  4,   .fm_offset = -1 },
        { .am_action = Switch5BitAction_Sum,        .fm_action = Switch5BitAction_Sum,        .am_offset =  1, .fm_offset =  1 },
        { .am_action = Switch5BitAction_Sum,        .fm_action = Switch5BitAction_Ignore,     .am_offset =  1, .fm_offset =  0     },
        { .am_action = Switch5BitAction_Sum,        .fm_action = Switch5BitAction_Sum,        .am_offset =  1, .fm_offset = -1 },
        { .am_action = Switch5BitAction_Ignore,     .fm_action = Switch5BitAction_Sum,        .am_offset =  0,     .fm_offset =  1 },
        { .am_action = Switch5BitAction_Ignore,     .fm_action = Switch5BitAction_Ignore,     .am_offset =  0,     .fm_offset =  0 },
        { .am_action = Switch5BitAction_Ignore,     .fm_action = Switch5BitAction_Sum,        .am_offset =  0,     .fm_offset = -1 },
        { .am_action = Switch5BitAction_Sum,        .fm_action = Switch5BitAction_Sum,        .am_offset = -1, .fm_offset =  1 },
        { .am_action = Switch5BitAction_Sum,        .fm_action = Switch5BitAction_Ignore,     .am_offset = -1, .fm_offset =  0     },
        { .am_action = Switch5BitAction_Sum,        .fm_action = Switch5BitAction_Sum,        .am_offset = -1, .fm_offset = -1 },
        { .am_action = Switch5BitAction_Sum,        .fm_action = Switch5BitAction_Sum,        .am_offset = -4,   .fm_offset =  1 },
        { .am_action = Switch5BitAction_Sum,        .fm_action = Switch5BitAction_Ignore,     .am_offset = -4,   .fm_offset =  0 },
        { .am_action = Switch5BitAction_Sum,        .fm_action = Switch5BitAction_Sum,        .am_offset = -4,   .fm_offset = -1 }
};//thk to mission control guys
uint8_t decoded_cnt=0;
uint8_t global_sample_channel;
//int32_t hliner=320;
typedef struct _rumble_data_linear{
    int16_t lo_amp_linear;
    int16_t lo_freq_linear;
    int16_t hi_amp_linear;
    int16_t hi_freq_linear;
}rumble_data_linear;
rumble_data_linear m_state;
rumble_data_linear linear_samples[3];
int16_t ApplyCommand(uint8_t action, int16_t offset, int16_t current_val, int16_t default_val, int16_t min, int16_t max) {
    switch (action) {
        case Switch5BitAction_Ignore:     return current_val;
        case Switch5BitAction_Substitute: //return offset;
            /*/current_val = offset;
            if(current_val<min)current_val=min;
            else if(current_val>max)current_val=max;
            return current_val;*/
            return (int16_t)i32_clamp(offset, min, max);
        case Switch5BitAction_Sum:
            /*current_val += offset;
            if(current_val<min)current_val=min;
            else if(current_val>max)current_val=max;
            return current_val;*/
            return (int16_t)i32_clamp((int32_t)current_val+offset,min,max);
        default:                          return default_val;
    }
}

int16_t ApplyAmCommand(u8 amfm_code, int16_t current_val) {
    return ApplyCommand(CommandTable[amfm_code].am_action, CommandTable[amfm_code].am_offset, current_val, DefaultAmplitude, MinAmplitude, MaxAmplitude);
}

int16_t ApplyFmCommand(u8 amfm_code, int16_t current_val) {
    return ApplyCommand(CommandTable[amfm_code].fm_action, CommandTable[amfm_code].fm_offset, current_val, DefaultFrequency, MinFrequency, MaxFrequency);
}
void get_hd_rumble_pack(uint8_t id)
{
    linear_samples[id]=m_state;
    //if(!pkg)return;
    /*
    static int32_t ftmp=1e9;
    hd_rumble_pack* pkg=&rumble_samples[global_sample_channel][id];
    pkg->high_amp=exp2_lookup_tb[m_state.hi_amp_linear];
    pkg->low_amp=exp2_lookup_tb[m_state.lo_amp_linear];
    pkg->high_freq=exp2_lookup_tb[m_state.hi_freq_linear] * CenterFreqHigh;
    pkg->low_freq=exp2_lookup_tb[m_state.lo_freq_linear] * CenterFreqLow;
    if(pkg->high_freq&&ftmp>pkg->high_freq)
    {
        ftmp=pkg->high_freq;
    }
    if(pkg->low_freq&&ftmp>pkg->low_freq){
        ftmp=pkg->low_freq;
    }*/
    ////printf("lowf %f highf %f\r\n",pkg->low_freq/16384.0,pkg->high_freq/16384.0);
}
void deocde_hd_rumble_format1(VibrationAmFmPackFormatOne* pkg)
{
    m_state.lo_amp_linear  = ApplyAmCommand(pkg->ChannelCodesLow, m_state.lo_amp_linear);
    m_state.lo_freq_linear = ApplyFmCommand(pkg->ChannelCodesLow, m_state.lo_freq_linear);
    m_state.hi_amp_linear  = ApplyAmCommand(pkg->ChannelCodesHigh, m_state.hi_amp_linear);
    m_state.hi_freq_linear = ApplyFmCommand(pkg->ChannelCodesHigh, m_state.hi_freq_linear);
    //this->GetCurrentOutputValue(&decoded->samples[0]);
    get_hd_rumble_pack(0);
    decoded_cnt=1;
}
void decode_hd_rumble_format1long(VibrationAmFmPackFormatOne28bit* pkg)
{
    /*m_state.lo_amp_linear  = Am7BitLookup[pkg->ChannelAmplitudeLow];
    m_state.lo_freq_linear = Fm7BitLookup[pkg->ChannelFrequencyLow];
    m_state.hi_amp_linear  = Am7BitLookup[pkg->ChannelAmplitudeHigh];
    m_state.hi_freq_linear = Fm7BitLookup[pkg->ChannelFrequencyHigh];*/
    m_state.lo_amp_linear  = amp_exp2_index_lookup_tb[pkg->ChannelAmplitudeLow];
    //if(m_state.lo_amp_linear)
    //    hliner=pkg->ChannelAmplitudeLow;
    m_state.lo_freq_linear = freq_exp2_index_lookup_tb[pkg->ChannelFrequencyLow];
    m_state.hi_amp_linear  = amp_exp2_index_lookup_tb[pkg->ChannelAmplitudeHigh];
    m_state.hi_freq_linear = freq_exp2_index_lookup_tb[pkg->ChannelFrequencyHigh];
    //this->GetCurrentOutputValue(&decoded->samples[0]);
    get_hd_rumble_pack(0);
    decoded_cnt=1;
}
void decode_hd_rumble_format1full(VibrationAmFmPackFormatThree7bit* pkg)
{
    /*if (encoded->three7bit.high_select) {
        if (encoded->three7bit.freq_select) {
            m_state.hi_freq_linear = Fm7BitLookup[encoded->three7bit.xx_7bit_xx];
        } else {
            m_state.hi_amp_linear  = Am7BitLookup[encoded->three7bit.xx_7bit_xx];
        }
    } else {
        if (encoded->three7bit.freq_select) {
            m_state.lo_freq_linear = Fm7BitLookup[encoded->three7bit.xx_7bit_xx];
        } else {
            m_state.lo_amp_linear  = Am7BitLookup[encoded->three7bit.xx_7bit_xx];
        }
    }
    this->GetCurrentOutputValue(&decoded->samples[0]);*/
    if(pkg->IsChannelHigh){
        if(pkg->IsFm){
            m_state.hi_freq_linear = freq_exp2_index_lookup_tb[pkg->ChannelAmFm];
        }else{
            m_state.hi_amp_linear = amp_exp2_index_lookup_tb[pkg->ChannelAmFm];
        }
    }else{
        if(pkg->IsFm){
            m_state.lo_freq_linear = freq_exp2_index_lookup_tb[pkg->ChannelAmFm];
        }else{
            m_state.lo_amp_linear = amp_exp2_index_lookup_tb[pkg->ChannelAmFm];
        }
    }
    get_hd_rumble_pack(0);
    m_state.lo_amp_linear  = ApplyAmCommand(pkg->VibrationCodesLow0, m_state.lo_amp_linear);
    m_state.lo_freq_linear = ApplyFmCommand(pkg->VibrationCodesLow0, m_state.lo_freq_linear);
    m_state.hi_amp_linear  = ApplyAmCommand(pkg->VibrationCodesHigh0, m_state.hi_amp_linear);
    m_state.hi_freq_linear = ApplyFmCommand(pkg->VibrationCodesHigh0, m_state.hi_freq_linear);
    get_hd_rumble_pack(1);

    m_state.lo_amp_linear  = ApplyAmCommand(pkg->VibrationCodesLow1, m_state.lo_amp_linear);
    m_state.lo_freq_linear = ApplyFmCommand(pkg->VibrationCodesLow1, m_state.lo_freq_linear);
    m_state.hi_amp_linear  = ApplyAmCommand(pkg->VibrationCodesHigh1, m_state.hi_amp_linear);
    m_state.hi_freq_linear = ApplyFmCommand(pkg->VibrationCodesHigh1, m_state.hi_freq_linear);
    get_hd_rumble_pack(2);
    decoded_cnt=3;
    /*m_state.lo_amp_linear  = ApplyAmCommand(encoded->three7bit.amfm_5bit_lo_1, m_state.lo_amp_linear);
    m_state.lo_freq_linear = ApplyFmCommand(encoded->three7bit.amfm_5bit_lo_1, m_state.lo_freq_linear);
    m_state.hi_amp_linear  = ApplyAmCommand(encoded->three7bit.amfm_5bit_hi_1, m_state.hi_amp_linear);
    m_state.hi_freq_linear = ApplyFmCommand(encoded->three7bit.amfm_5bit_hi_1, m_state.hi_freq_linear);
    this->GetCurrentOutputValue(&decoded->samples[1]);

    m_state.lo_amp_linear  = ApplyAmCommand(encoded->three7bit.amfm_5bit_lo_2, m_state.lo_amp_linear);
    m_state.lo_freq_linear = ApplyFmCommand(encoded->three7bit.amfm_5bit_lo_2, m_state.lo_freq_linear);
    m_state.hi_amp_linear  = ApplyAmCommand(encoded->three7bit.amfm_5bit_hi_2, m_state.hi_amp_linear);
    m_state.hi_freq_linear = ApplyFmCommand(encoded->three7bit.amfm_5bit_hi_2, m_state.hi_freq_linear);
    this->GetCurrentOutputValue(&decoded->samples[2]);

    decoded->count = 3;*/
}
void decode_hd_rumble_format2(VibrationAmFmPackFormatTwo* pkg)
{
    /*m_state.lo_amp_linear  = ApplyAmCommand(encoded->two5bit.amfm_5bit_lo_0, m_state.lo_amp_linear);
    m_state.lo_freq_linear = ApplyFmCommand(encoded->two5bit.amfm_5bit_lo_0, m_state.lo_freq_linear);
    m_state.hi_amp_linear  = ApplyAmCommand(encoded->two5bit.amfm_5bit_hi_0, m_state.hi_amp_linear);
    m_state.hi_freq_linear = ApplyFmCommand(encoded->two5bit.amfm_5bit_hi_0, m_state.hi_freq_linear);
    this->GetCurrentOutputValue(&decoded->samples[0]);

    m_state.lo_amp_linear  = ApplyAmCommand(encoded->two5bit.amfm_5bit_lo_1, m_state.lo_amp_linear);
    m_state.lo_freq_linear = ApplyFmCommand(encoded->two5bit.amfm_5bit_lo_1, m_state.lo_freq_linear);
    m_state.hi_amp_linear  = ApplyAmCommand(encoded->two5bit.amfm_5bit_hi_1, m_state.hi_amp_linear);
    m_state.hi_freq_linear = ApplyFmCommand(encoded->two5bit.amfm_5bit_hi_1, m_state.hi_freq_linear);
    this->GetCurrentOutputValue(&decoded->samples[1]);*/
    m_state.lo_amp_linear  = ApplyAmCommand(pkg->ChannelCodesLow, m_state.lo_amp_linear);
    m_state.lo_freq_linear = ApplyFmCommand(pkg->ChannelCodesLow, m_state.lo_freq_linear);
    m_state.hi_amp_linear  = ApplyAmCommand(pkg->ChannelCodesHigh, m_state.hi_amp_linear);
    m_state.hi_freq_linear = ApplyFmCommand(pkg->ChannelCodesHigh, m_state.hi_freq_linear);
    get_hd_rumble_pack(0);

    m_state.lo_amp_linear  = ApplyAmCommand(pkg->VibrationCodesLow, m_state.lo_amp_linear);
    m_state.lo_freq_linear = ApplyFmCommand(pkg->VibrationCodesLow, m_state.lo_freq_linear);
    m_state.hi_amp_linear  = ApplyAmCommand(pkg->VibrationCodesHigh, m_state.hi_amp_linear);
    m_state.hi_freq_linear = ApplyFmCommand(pkg->VibrationCodesHigh, m_state.hi_freq_linear);
    get_hd_rumble_pack(1);
    decoded_cnt=2;
}
void decode_hd_rumble_format2long(VibrationAmFmPackFormatTwo14bit* pkg)
{
    if (pkg->IsChannelHigh) {
        m_state.hi_amp_linear  = amp_exp2_index_lookup_tb[pkg->ChannelAmplitude];
        m_state.hi_freq_linear = freq_exp2_index_lookup_tb[pkg->ChannelFrequency];
        m_state.lo_amp_linear  = ApplyAmCommand(pkg->VibrationCodes0, m_state.lo_amp_linear);
        m_state.lo_freq_linear = ApplyFmCommand(pkg->VibrationCodes0, m_state.lo_freq_linear);
    } else {
        m_state.lo_amp_linear  = amp_exp2_index_lookup_tb[pkg->ChannelAmplitude];
        m_state.lo_freq_linear = freq_exp2_index_lookup_tb[pkg->ChannelFrequency];
        m_state.hi_amp_linear  = ApplyAmCommand(pkg->VibrationCodes0, m_state.hi_amp_linear);
        m_state.hi_freq_linear = ApplyFmCommand(pkg->VibrationCodes0, m_state.hi_freq_linear);
    }
    get_hd_rumble_pack(0);

    m_state.lo_amp_linear  = ApplyAmCommand(pkg->VibrationCodesLow1, m_state.lo_amp_linear);
    m_state.lo_freq_linear = ApplyFmCommand(pkg->VibrationCodesLow1, m_state.lo_freq_linear);
    m_state.hi_amp_linear  = ApplyAmCommand(pkg->VibrationCodesHigh1, m_state.hi_amp_linear);
    m_state.hi_freq_linear = ApplyFmCommand(pkg->VibrationCodesHigh1, m_state.hi_freq_linear);
    get_hd_rumble_pack(1);

    decoded_cnt=2;
}
void decode_hd_rumble_format3(VibrationAmFmPackFormatThree* pkg)
{
    m_state.lo_amp_linear  = ApplyAmCommand(pkg->ChannelCodesLow, m_state.lo_amp_linear);
    m_state.lo_freq_linear = ApplyFmCommand(pkg->ChannelCodesLow, m_state.lo_freq_linear);
    m_state.hi_amp_linear  = ApplyAmCommand(pkg->ChannelCodesHigh, m_state.hi_amp_linear);
    m_state.hi_freq_linear = ApplyFmCommand(pkg->ChannelCodesHigh, m_state.hi_freq_linear);
    get_hd_rumble_pack(0);

    m_state.lo_amp_linear  = ApplyAmCommand(pkg->VibrationCodesLow0, m_state.lo_amp_linear);
    m_state.lo_freq_linear = ApplyFmCommand(pkg->VibrationCodesLow0, m_state.lo_freq_linear);
    m_state.hi_amp_linear  = ApplyAmCommand(pkg->VibrationCodesHigh0, m_state.hi_amp_linear);
    m_state.hi_freq_linear = ApplyFmCommand(pkg->VibrationCodesHigh0, m_state.hi_freq_linear);
    get_hd_rumble_pack(1);

    m_state.lo_amp_linear  = ApplyAmCommand(pkg->VibrationCodesLow1, m_state.lo_amp_linear);
    m_state.lo_freq_linear = ApplyFmCommand(pkg->VibrationCodesLow1, m_state.lo_freq_linear);
    m_state.hi_amp_linear  = ApplyAmCommand(pkg->VibrationCodesHigh1, m_state.hi_amp_linear);
    m_state.hi_freq_linear = ApplyFmCommand(pkg->VibrationCodesHigh1, m_state.hi_freq_linear);
    get_hd_rumble_pack(2);

    decoded_cnt=3;
}
hd_rumble_high_accurary_pack sample;
static uint64_t FULLSTEP=1LL*(HD_RUMBLE_CLK<<EXP2_FACTOR_SHIFT)*HD_RUMBLE_STEP>>HD_RUMBLE_SAMPLERATE_SHIFT;
//static uint32_t FULLSTEP=(1ULL*(HD_RUMBLE_CLK<<EXP2_FACTOR_SHIFT)*HD_RUMBLE_STEP)>>HD_RUMBLE_SAMPLERATE_SHIFT;
void decode_hd_rumble_multiformat_high_acc(hd_rumble_multiformat* pkt,hd_rumble_multiformat* pkt_r)
{
    global_sample_channel=SAMPLE_CHANNEL_L;
    rumble_data_linear* ptr=NULL;
    //memset(sample,0,HD_RUMBLE_HIGH_ACC_PACK_SIZE);
    decoded_cnt=0;
    switch(pkt->FromatZero.PackFormat)
    {
    case 0:
        //decoded_cnt=0;
        decoded_cnt=1;
        linear_samples[0].hi_amp_linear=0;
        linear_samples[0].lo_amp_linear=0;
        break;
    case 1:
        if(pkt->FormatOne.Reserved==0) {
            deocde_hd_rumble_format1(&pkt->FormatOne);
        }else if(pkt->FormatOne28bit.Reserved==0) {
            decode_hd_rumble_format1long(&pkt->FormatOne28bit);
        }else {
            decode_hd_rumble_format1full(&pkt->FormatThree7bit);
        }
        break;
    case 2:
        if(pkt->FormatTwo.Reserved==0) {
            decode_hd_rumble_format2(&pkt->FormatTwo);
        }else{
            decode_hd_rumble_format2long(&pkt->FormatTwo14bit);
        }
        break;
    case 3:
        decode_hd_rumble_format3(&pkt->FormatThree);
        break;
    default:
        break;
    }
    ptr=linear_samples;
    decoded_cnt=i32_clamp(decoded_cnt, 0, 3);
    //decoded_cnt=i32_clamp(decoded_cnt, 0, 1);
    for(int i=0;i<decoded_cnt;++i,++ptr)
    {
        sample.amp=(exp2_lookup_tb[ptr->hi_amp_linear]*user_config.hd_rumble_amp_ratio[0])>>HD_RUMBLE_HIGH_ACC_AMP_SHIFT;
        sample.step=(exp2_lookup_tb[ptr->hi_freq_linear]?FULLSTEP/(exp2_lookup_tb[ptr->hi_freq_linear]*CenterFreqHigh):0);
        //sample.tick=1LL*(HD_RUMBLE_CLK<<EXP2_FACTOR_SHIFT)/(exp2_lookup_tb[ptr->hi_freq_linear]*CenterFreqHigh);
        ////printf("wave left low amp:%d step:%d amp linear:%d\r\n",sample.amp,sample.step,ptr->hi_amp_linear);
        push_waveform(0,&sample);
        //ring_buffer_push(&left_high_rb, (uint8_t*)&sample, HD_RUMBLE_HIGH_ACC_PACK_SIZE, 0);
        sample.amp=(exp2_lookup_tb[ptr->lo_amp_linear]*user_config.hd_rumble_amp_ratio[2])>>HD_RUMBLE_HIGH_ACC_AMP_SHIFT;
        sample.amp=(-sample.amp)*user_config.hd_rumble_mixer_ratio>>HD_RUMBLE_MIXER_SHIFT;
        sample.step=(exp2_lookup_tb[ptr->lo_freq_linear]?(FULLSTEP/(exp2_lookup_tb[ptr->lo_freq_linear]*CenterFreqLow)):0);
        //sample.tick=1LL*(HD_RUMBLE_CLK<<EXP2_FACTOR_SHIFT)/(exp2_lookup_tb[ptr->lo_freq_linear]*CenterFreqLow);
        push_waveform(2,&sample);
        //ring_buffer_push(&left_low_rb, (uint8_t*)&sample, HD_RUMBLE_HIGH_ACC_PACK_SIZE, 0);
    }
    pkt=pkt_r;
    global_sample_channel=SAMPLE_CHANNEL_R;
    decoded_cnt=0;
    switch(pkt->FromatZero.PackFormat)
    {
    case 0:
        decoded_cnt=1;
        linear_samples[0].hi_amp_linear=0;
        linear_samples[0].lo_amp_linear=0;
        break;
    case 1:
        if(pkt->FormatOne.Reserved==0) {
            deocde_hd_rumble_format1(&pkt->FormatOne);
        }else if(pkt->FormatOne28bit.Reserved==0) {
            decode_hd_rumble_format1long(&pkt->FormatOne28bit);
        }else {
            decode_hd_rumble_format1full(&pkt->FormatThree7bit);
        }
        break;
    case 2:
        if(pkt->FormatTwo.Reserved==0) {
            decode_hd_rumble_format2(&pkt->FormatTwo);
        }else{
            decode_hd_rumble_format2long(&pkt->FormatTwo14bit);
        }
        break;
    case 3:
        decode_hd_rumble_format3(&pkt->FormatThree);
        break;
    default:
        break;
    }
    decoded_cnt=i32_clamp(decoded_cnt, 0, 3);
    //decoded_cnt=i32_clamp(decoded_cnt, 0, 1);
    ptr=linear_samples;
    for(int i=0;i<decoded_cnt;++i,++ptr)
    {
        sample.amp=(exp2_lookup_tb[ptr->hi_amp_linear]*user_config.hd_rumble_amp_ratio[1])>>HD_RUMBLE_HIGH_ACC_AMP_SHIFT;
        sample.step=(exp2_lookup_tb[ptr->hi_freq_linear]?FULLSTEP/(exp2_lookup_tb[ptr->hi_freq_linear]*CenterFreqHigh):0);
        //sample.tick=1LL*(HD_RUMBLE_CLK<<EXP2_FACTOR_SHIFT)/(exp2_lookup_tb[ptr->hi_freq_linear]*CenterFreqHigh);
        push_waveform(1,&sample);
        //ring_buffer_push(&right_high_rb, (uint8_t*)&sample, HD_RUMBLE_HIGH_ACC_PACK_SIZE, 0);
        sample.amp=(exp2_lookup_tb[ptr->lo_amp_linear]*user_config.hd_rumble_amp_ratio[3])>>HD_RUMBLE_HIGH_ACC_AMP_SHIFT;
        sample.amp=(-sample.amp)*user_config.hd_rumble_mixer_ratio>>HD_RUMBLE_MIXER_SHIFT;
        sample.step=(exp2_lookup_tb[ptr->lo_freq_linear]?(FULLSTEP/(exp2_lookup_tb[ptr->lo_freq_linear]*CenterFreqLow)):0);
        //sample.tick=1LL*(HD_RUMBLE_CLK<<EXP2_FACTOR_SHIFT)/(exp2_lookup_tb[ptr->lo_freq_linear]*CenterFreqLow);
        push_waveform(3,&sample);
        //ring_buffer_push(&right_low_rb, (uint8_t*)&sample, HD_RUMBLE_HIGH_ACC_PACK_SIZE, 0);
    }
    ////printf("decoded")
}
void hd_rumble_lookup_tb_init(){
    if(user_config.rumble_low_amp_rise)
        memcpy(amp_exp2_index_lookup_tb+1,amp_exp2_index_lookup_tb_low_amp_rise,sizeof(amp_exp2_index_lookup_tb_low_amp_rise));
    else
        memcpy(amp_exp2_index_lookup_tb+1,amp_exp2_index_lookup_tb_low_amp_raw,sizeof(amp_exp2_index_lookup_tb_low_amp_raw));
}
