/*
 * conf.h
 *
 *  Created on: 2024Äê11ÔÂ26ÈÕ
 *      Author: Reed
 */

#ifndef USER_CONF_H_
#define USER_CONF_H_
#include <stdint.h>
#include "affine.h"
#include "ns_com_mux.h"

#define CONFIG_MAGIC (0x55)
#define CONFIG_EXIST(x) ((x)==CONFIG_MAGIC)

#define R32_ESIG_UNIID1 (0x1FFFF7E8)
#define R32_ESIG_UNIID2 (0x1FFFF7EC)
#define R32_ESIG_UNIID3 (0x1FFFF7F0)
#define ESIG_LENGTH (12)

#define NS_SPI_USER_JOYSTICK_CALIBRATION_ADDR (0x8010)
#define NS_SPI_USER_IMU_CALIBRATION_ADDR (0X8026)

#define RGB_MAX_CNT (85)

#define CONF_PCB_TYPE_LARGE (1)
#define CONF_PCB_TYPE_SMALL (0)

#define CONF_BTN_RGB_FULL (1)
#define CONF_BTN_RGB_PWR_ONLY (0)
#define CONF_BTN_LED (2)

/*
Firmware
The firmware is stored inside the flash in Broadcom's PatchRAM format as follows:

Offset  Size    Description
0x0 0x1000  StaticSection
0x1000  0x1000  FailsafeSection
0x2000  0x1000  VolatileSection
0x3000  0x1000  VolatileSectionBackup1
0x4000  0x1000  VolatileSectionBackup2
0x5000  0x1000  ShipmentInfo
0x6000  0x1000  FactoryConfiguration
0x7000  0x1000  Reserved
0x8000  0x1000  UserCalibration
0x9000  0x7000  Reserved
0x10000 0x18000 DynamicSection1
0x28000 0x18000 DynamicSection2
0x40000 0x40000 Reserved
*/

#pragma pack(push,1)
typedef struct _joystick_calibration_data_left{
    uint16_t AnalogStickCalXPositive:12;
    uint16_t AnalogStickCalYPositive:12;
    uint16_t AnalogStickCalX0:12;
    uint16_t AnalogStickCalY0:12;
    uint16_t AnalogStickCalXNegative:12;
    uint16_t AnalogStickCalYNegative:12;
}joystick_calibration_data_left;
typedef struct _joystick_calibration_data_right{
    uint16_t AnalogStickCalX0:12;
    uint16_t AnalogStickCalY0:12;
    uint16_t AnalogStickCalXNegative:12;
    uint16_t AnalogStickCalYNegative:12;
    uint16_t AnalogStickCalXPositive:12;
    uint16_t AnalogStickCalYPositive:12;
}joystick_calibration_data_right;
typedef struct _user_joystick_calibration_data{
    uint16_t AnalogStickLeftUserMagicNumber;
    joystick_calibration_data_left AnalogStickLeftUserCalibrationValue;
    uint16_t AnalogStickRightUserMagicNumber;
    joystick_calibration_data_right AnalogStickRightUserCalibrationValue;
}user_joystick_calibration_data;
typedef struct _factory_joystick_calibration_data{
    union{
        struct{
            joystick_calibration_data_left AnalogStickLeftFactoryCalibrationValue;
            joystick_calibration_data_right AnalogStickRightFactoryCalibrationValue;
        };
        joystick_calibration_data_left AnalogStickFactoryCalibrationValue[2];
    };
}factory_joystick_calibration_data;
typedef struct _imu_calibration_data{
    int16_t Accelerometer0OffsetX;
    int16_t Accelerometer0OffsetY;
    int16_t Accelerometer0OffsetZ;
    uint16_t Accelerometer1gScaleX;
    uint16_t Accelerometer1gScaleY;
    uint16_t Accelerometer1gScaleZ;
    int16_t Gyroscope0OffsetX;
    int16_t Gyroscope0OffsetY;
    int16_t Gyroscope0OffsetZ;
    uint16_t Gyroscope78rpmScaleX;
    uint16_t Gyroscope78rpmScaleY;
    uint16_t Gyroscope78rpmScaleZ;
}imu_calibration_data;
typedef struct _user_imu_calibration_data{
    uint16_t SixAxisUserCalibrationMagicNumber;
    imu_calibration_data SixAxisSensorCalibrationValue;
}user_imu_calibration_data;
typedef struct _user_calibration_data{
    uint8_t Reserved1[16];
    user_joystick_calibration_data UserJoystickCalibrationValue;
    user_imu_calibration_data UserSixAxisSensorCalibrationValue;
    uint8_t Reserved2[16];
}user_calibration_data;
typedef struct _rgb_data_complete{
    union{
        struct{
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };
        uint32_t load:24;
    };
}rgb_data_complete;
typedef struct _controller_color_data{
    rgb_data_complete MainColor;
    rgb_data_complete SubColor;
    rgb_data_complete ExtraColor1;
    rgb_data_complete ExtraColor2;
}controller_color_data;
typedef struct _device_design_data{
    controller_color_data ControllerColor;
    uint8_t DesignVariation;//0,1,2
}device_design_data;
typedef struct _imu_model_data{
    uint16_t SixAxisHorizontalOffsetX;
    uint16_t SixAxisHorizontalOffsetY;
    uint16_t SixAxisHorizontalOffsetZ;
}imu_model_data;
typedef struct _joystick_model_value{
    uint16_t AnalogStickModelNoise:12;
    uint16_t AnalogStickModelTypicalStroke:12;
    uint16_t AnalogStickModelCenterDeadZoneSize:12;
    uint16_t AnalogStickModelCircuitDeadZoneScale:12;
    uint16_t AnalogStickModelMinimumStrokeXPositive:12;
    uint16_t AnalogStickModelMinimumStrokeYPositive:12;
    uint16_t AnalogStickModelMinimumStrokeXNegative:12;
    uint16_t AnalogStickModelMinimumStrokeYNegative:12;
    uint16_t AnalogStickModelCenterRangeXPositive:12;
    uint16_t AnalogStickModelCenterRangeYPositive:12;
    uint16_t AnalogStickModelCenterRangeXNegative:12;
    uint16_t AnalogStickModelCenterRangeYNegative:12;
}joystick_model_value;
typedef struct _model_data{
    imu_model_data SixAxisSensorModelValue;
    joystick_model_value AnalogStickMainModelValue;
}model_data;
typedef struct _factory_configuration_data{
    uint8_t IdentificationCode[16];//sn code
    union{
        uint8_t nonexist;
        uint8_t Reserved1[2];
    };
    uint8_t DeviceType;//0x03 to be a pro controller
    uint8_t BoardRevision;
    uint8_t Reserved2[7];
    uint8_t FormatVersion;
    uint8_t Reserved3[4];
    imu_calibration_data SixAxisSensorCalibrationValue;
    uint8_t Reserved4[5];
    factory_joystick_calibration_data JoystickCalibrationValue;
    uint8_t Reserved5;
    device_design_data Design;
    uint8_t Reserved6[35];
    model_data Model;
    joystick_model_value AnalogStickSubModelValue;
    uint8_t Reserved7;
    uint8_t AccelerometerAxisAssignment;
    uint8_t GyroscopeAxisAssignment;
    uint8_t AnalogStickMainAxisAssignment;
    uint8_t AnalogStickSubAxisAssignment;
    //uint8_t Reserved8[337];
    uint16_t BatteryVoltage;
    /*
    0x202   0xB7E   Reserved
    0xD80   0x2 TarragonVid
    0xD82   0x2 TarragonPid
    0xD84   0x7C    Reserved
    0xE00   0x100   InspectionLog
    0xF00   0x100   Reserved
    */
}factory_configuration_data;
typedef struct _factory_configuration_flash_pack{
    uint8_t nonexist;//available when 0
    uint8_t DeviceType;//0x03 to be a pro controller
    uint8_t BoardRevision;
    uint8_t FormatVersion;
    imu_calibration_data SixAxisSensorCalibrationValue;
    factory_joystick_calibration_data JoystickCalibrationValue;
    device_design_data Design;
    model_data Model;
    joystick_model_value AnalogStickSubModelValue;
}factory_configuration_flash_pack;
typedef struct _rgb_data_simple{
    uint8_t b:4;
    uint8_t g:4;
    uint8_t r:4;
}rgb_data_simple;
//used for wired model to save space
typedef struct{
    union{
        uint8_t affine_bitmap0;
        struct{
            uint8_t cnt:4;
            uint8_t x_rev:1;
            uint8_t y_rev:1;
            uint8_t:2;
        };
    };
    struct{
        coord notch;
        coord angle;
    }map[16];
}affine_pack;
typedef struct _user_config_data{
    uint8_t magic;
    struct{
        uint8_t pcb_typ;
        uint8_t pcb_rev;
        uint8_t indi_led_ofst;
        uint8_t rgb_cnt;
    }hw;
    struct{
        uint8_t bd_addr[BD_ADDR_LEN];
        uint8_t pro_fw_version;
        uint8_t ns_pkt_timer_mode;//0:stock(timestamp_div_5) 1:timestamp 2:pkt cnt
    }basic;
    struct{
        union{
            uint8_t usb_bitmap0;
            struct{
                uint8_t auto_recovery:1;
                uint8_t:7;//padding
            };
        };
        uint8_t in_interval;
        uint8_t out_interval;
    }usb;
    struct{
        union{
            uint8_t btn_bitmap0;
            struct{
                uint8_t x_y_swap:1;
                uint8_t a_b_swap:1;
                uint8_t dpad_mapping_js:1;
                uint8_t:5;
            };
        };
        uint32_t disable_mask:24;
    }btn;
    struct{
        union{
            uint8_t rumble_bitmap0;
            union{
                uint8_t enable:1;
                uint8_t mode:3;
                uint8_t:4;
            };
        };
        struct{
            union{
                uint8_t hd_bitmap0;
                struct{
                    uint8_t pattern:1;
                    uint8_t high_amp_drop:1;
                    uint8_t low_amp_rise:1;
                    uint8_t legacy:1;
                    uint8_t:4;
                };
            };
            uint8_t amp_ratio[4];
        }hd;
        struct{

        }erm;//todo?really someone will like this?
    }rumble;
    struct{
        union{
            uint8_t js_bitmap0;
            struct{
                uint8_t normalization:1;
                uint8_t l_mode:2;//0:ratio 1:affine
                uint8_t r_mode:2;
                //when sat affine will auto active after pairing and can be turned off by driver through cmd
                uint8_t:3;
            };
        };
        uint16_t center[4];
        int8_t ratio[4];
        uint8_t dz[4];
        uint8_t dz_mode;
    }js;
    struct{
        union{
            uint8_t snpbk_bitmap0;
            struct{
                uint8_t only_depress_x:1;
                uint8_t:7;
            };
        };
        uint16_t dz[2];
        uint16_t filter_window;
    }snpbk;
    struct{
        union{
            uint8_t imu_bitmap0;
            struct{
                uint8_t enable:1;
                uint8_t:7;
            };
        };
        uint16_t sample_gap;
        uint8_t ratio_x;//div 127
        uint8_t ratio_y;//div 127
        uint8_t ratio_z;//div 127
    }imu;
    struct{
        union{
            uint8_t affine_bitmap0;
            struct{
                uint8_t cnt:4;
                uint8_t x_rev:1;
                uint8_t y_rev:1;
                uint8_t:2;
            };
        };
        struct{
            coord_compact notch;
            coord_compact angle;
        }map[16];
    }affine[2];
    struct{
        union{
            uint8_t rgb_bitmap0;
            struct{
                uint8_t enable:1;
                uint8_t allow_rgb_on_bat:1;
            };
        };
        uint8_t slow_start_period;
        uint8_t indi_led_brightness;
        rgb_data_complete data[RGB_MAX_CNT];
    }rgb;
}user_config_data;
enum PCB_REV_PRO{
    PRO_200,
    PRO_213,
    PRO_300,
};
enum PCB_REV_NGC{
    NGC_110,
};
enum PCB_TYP{
    PCB_TYP_PRO,
    PCB_TYP_NGC
};
#define PRO_ATLEAST(x) (config.hw.pcb_typ==PCB_TYP_PRO && config.hw.pcb_rev >= (x))
#define PRO_IS(x) (config.hw.pcb_typ==PCB_TYP_PRO && config.hw.pcb_rev == (x))
#define NGC_ATLEAST(x) (config.hw.pcb_typ==PCB_TYP_NGC && config.hw.pcb_rev >= (x))
#define NGC_IS(x) (config.hw.pcb_typ==PCB_TYP_NGC && config.hw.pcb_rev == (x))

typedef struct _affine_notch_data{
    uint8_t cnt;
    coord notch[16];
    coord target[16];
}affine_notch_data;//49 byte
typedef struct _calibrate_data{
    uint8_t magic;
    uint16_t center[4];
    affine_notch_data affine[2];
}calibrate_data;

#pragma pack(pop)

#define JOYSITCK_FITTING_PARAM_RATIO (255)
#define JOYSITCK_FITTING_PARAM_RATIO_SHIFT (8)

#define FLASH_ADDR_USER_CALIBRATION (0x0)
#define FLASH_ADDR_FACTORY_CONFIG (0x100)
#define FLASH_ADDR_USER_CONFIG (0x200)

extern const uint32_t FW_VERSION;
extern const uint32_t FW_SUB_VERSION;

extern factory_configuration_data factory_configuration;
extern user_calibration_data user_calibration;
extern user_config_data config;
extern uint32_t joystick_snapback_deadzone_sq[2];
extern uint32_t button_active_mask;

void conf_init();
void conf_read(uint32_t addr,uint8_t* buf,uint8_t size);
uint8_t conf_write(uint32_t addr,uint8_t* buf,uint8_t size,uint8_t save);
void custom_conf_read();
uint8_t custom_conf_write();
void conf_flush();
void fac_conf_read();
uint8_t fac_conf_write();

#endif /* USER_CONF_H_ */
