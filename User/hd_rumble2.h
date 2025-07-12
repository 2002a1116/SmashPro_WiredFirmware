/*
 * hd_rumble2.h
 *
 *  Created on: 2024Äê12ÔÂ17ÈÕ
 *      Author: Reed
 */

#ifndef USER_HD_RUMBLE2_H_
#define USER_HD_RUMBLE2_H_
#include <stdint.h>
#pragma pack(push,1)
//ALL RESERVER FILLED WITH 0
    typedef struct _VibrationAmFmPackFormatZero
    {
        uint32_t Reserved:30;
        uint8_t PackFormat:2;//equal to 0
    }VibrationAmFmPackFormatZero;
    typedef struct _VibrationAmFmPackFormatOne
    {
        uint32_t Reserved:20;
        uint8_t ChannelCodesHigh:5;
        uint8_t ChannelCodesLow:5;
        uint8_t PackFormat:2;//equal to 0
    }VibrationAmFmPackFormatOne;//whats this?
    typedef struct _VibrationAmFmPackFormatOne28bit
    {
        uint8_t Reserved:2;
        uint8_t ChannelFrequencyHigh:7;
        uint8_t ChannelAmplitudeHigh:7;
        uint8_t ChannelFrequencyLow:7;
        uint8_t ChannelAmplitudeLow:7;
        uint8_t PackFormat:2;//equal to 1
    }VibrationAmFmPackFormatOne28bit;
    typedef struct _VibrationAmFmPackFormatTwo
    {
        uint16_t Reserved:10;
        uint8_t VibrationCodesHigh:5;
        uint8_t VibrationCodesLow:5;
        uint8_t ChannelCodesHigh:5;
        uint8_t ChannelCodesLow:5;
        uint8_t PackFormat:2;//equal to 2
    }VibrationAmFmPackFormatTwo;
    typedef struct _VibrationAmFmPackFormatTwo14bit
    {
        uint8_t IsChannelHigh:1;
        uint8_t ChannelFrequency:7;//IsChannelHigh ?high/low;
        uint8_t VibrationCodesHigh1:5;
        uint8_t VibrationCodesLow1:5;
        uint8_t VibrationCodes0:5;//IsChannelHigh?VibrationCodesLow0:VibrationCodesHigh0
        uint8_t ChannelAmplitude:7;
        uint8_t PackFormat:2;//equal to 2
    }VibrationAmFmPackFormatTwo14bit;
    typedef struct _VibrationAmFmPackFormatThree
    {
        uint8_t VibrationCodesHigh1:5;
        uint8_t VibrationCodesLow1:5;
        uint8_t VibrationCodesHigh0:5;
        uint8_t VibrationCodesLow0:5;
        uint8_t ChannelCodesHigh:5;
        uint8_t ChannelCodesLow:5;
        uint8_t PackFormat:2;//equal to 3
    }VibrationAmFmPackFormatThree;
    typedef struct _VibrationAmFmPackFormatThree7bit
    {
        uint8_t IsChannelHigh:1;
        uint8_t IsThree7bit:1;
        uint8_t IsFm:1;
        uint8_t VibrationCodesHigh1:5;
        uint8_t VibrationCodesLow1:5;
        uint8_t VibrationCodesHigh0:5;
        uint8_t VibrationCodesLow0:5;
        uint8_t ChannelAmFm:7;
        uint8_t PackFormat:2;//equal to 1!! not 3
    }VibrationAmFmPackFormatThree7bit;
typedef union _hd_rumble_multiformat
{
    //uint8_t _raw[4];
    uint32_t Raw;
    VibrationAmFmPackFormatZero FromatZero;
    VibrationAmFmPackFormatOne FormatOne;
    VibrationAmFmPackFormatOne28bit FormatOne28bit;
    VibrationAmFmPackFormatTwo FormatTwo;
    VibrationAmFmPackFormatTwo14bit FormatTwo14bit;
    VibrationAmFmPackFormatThree FormatThree;
    VibrationAmFmPackFormatThree7bit FormatThree7bit;
}hd_rumble_multiformat;
#pragma pack(pop)
enum Switch5BitAction{
    Switch5BitAction_Ignore     = 0x0,
    Switch5BitAction_Default    = 0x1,
    Switch5BitAction_Substitute = 0x2,
    Switch5BitAction_Sum        = 0x3
};
struct Switch5BitCommand{
    uint8_t am_action;
    uint8_t fm_action;
    int16_t am_offset;
    int16_t fm_offset;
};
typedef struct _hd_rumble_pack
{
    uint16_t high_amp;
    uint16_t low_amp;
    uint32_t high_freq;
    uint32_t low_freq;
}hd_rumble_pack;
#define MinAmplitude      (0)
#define MaxAmplitude       (256)
#define DefaultAmplitude   (MinAmplitude)
#define MinFrequency      (192)
#define MaxFrequency       (320)
#define DefaultFrequency   (256)
#define CenterFreqHigh  (320)
#define CenterFreqLow   (160)
#define SAMPLE_CHANNEL_L (0)
#define SAMPLE_CHANNEL_R (1)

#define EXP2_FACTOR (16384) //2^14 aka 1<<14
#define EXP2_FACTOR_SHIFT (14)
#define HD_RUMBLE_HIGH_ACC_AMP_SHIFT (11)
//#define HD_RUMBLE_HIGH_ACC_AMP_SHIFT (10)

extern int32_t hliner,ftmp;

int16_t ApplyCommand(uint8_t action, int16_t offset, int16_t current_val, int16_t default_val, int16_t min, int16_t max);
int16_t ApplyAmCommand(u8 amfm_code, int16_t current_val);
int16_t ApplyFmCommand(u8 amfm_code, int16_t current_val);
void get_hd_rumble_pack(uint8_t id);
void deocde_hd_rumble_format1(VibrationAmFmPackFormatOne* pkg);
void decode_hd_rumble_format1long(VibrationAmFmPackFormatOne28bit* pkg);
void decode_hd_rumble_format1full(VibrationAmFmPackFormatThree7bit* pkg);
void decode_hd_rumble_format2(VibrationAmFmPackFormatTwo* pkg);
void decode_hd_rumble_format2long(VibrationAmFmPackFormatTwo14bit* pkg);
void decode_hd_rumble_format3(VibrationAmFmPackFormatThree* pkg);
void decode_hd_rumble_multiformat_high_acc(hd_rumble_multiformat* pkt,hd_rumble_multiformat* pkt_r);
void hd_rumble_lookup_tb_init();
#endif /* USER_HD_RUMBLE2_H_ */
