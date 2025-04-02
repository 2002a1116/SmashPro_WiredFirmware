/*
 * ns_com.h
 *
 *  Created on: 2024Äê10ÔÂ15ÈÕ
 *      Author: Reed
 */

#ifndef USER_NS_COM_H_
#define USER_NS_COM_H_

#define BD_ADDR_LEN (6)

#define MAX_ADC_CHANNEL_CNT (10)

#define NS_SUBCOMMAND_REPORT_DATA_MAX_LENGTH (34)
#define NS_MCU_FW_UPDATE_REPORT_DATA_MAX_LENGTH (35)
#define NS_BASIC_REPORT_RESERVED_MAX_LENGTH (35)
#define NS_SUBCOMMAND_CMD_DATA_MAX_LENGTH (38)
#define NS_MCU_FW_UPDATE_CMD_DATA_MAX_LENGTH (308)
#define NS_MCU_CALL_CMD_DATA_MAX_LENGTH (38)
#define NS_ATTACHMENT_CMD_DATA_MAX_LENGTH (38)
#define NS_PACKET_TYPE_MAX_VALUE (255)
#define NS_SUBCOMMAND_ID_MAX_VLAUE (255)
//#define NS_STD_REPORT_BASIC_LENGTH (12)//(13)
#define NS_SUBCOMMAND_CMD_HEADER_LENGTH (11)

#define NS_SUBCOMMAND_STATUS_ACK (0x80)
#define NS_SUBCOMMAND_STATUS_NACK (0x00)

typedef enum{
    NS_REPORT_MODE_STD,
    NS_REPORT_MODE_NFC_IR,
    NS_REPORT_MODE_SIMPLE_HID
}NS_REPORT_MODE;

#pragma pack(push,1)
typedef struct{
    union{
        int16_t data[3];
        //uint64_t load:48;
    };
}imu_pack;
typedef struct{
    imu_pack acc0;
    union{
        struct{
            uint32_t mode : 2;
            uint32_t max_index : 2;
            uint32_t last_sample_0 : 21;
            /*uint32_t last_sample_1l : 7;
            uint16_t last_sample_1h : 14;*/
            uint32_t last_sample_1:21;
            uint16_t last_sample_2l : 2;
        };
        imu_pack gyo0;
    };
    imu_pack acc1;
    union{
        struct{
            uint32_t last_sample_2h : 19;
            uint32_t delta_last_first_0 : 13;
            uint16_t delta_last_first_1 : 13;
            uint16_t delta_last_first_2l : 3;
        };
        imu_pack gyo1;
    };
    imu_pack acc2;
    union{
        struct{
            uint32_t delta_last_first_2h : 10;
            uint32_t delta_mid_avg_0 : 7;
            uint32_t delta_mid_avg_1 : 7;
            uint32_t delta_mid_avg_2 : 7;
            /*uint32_t timestamp_start_l : 1;
            uint16_t timestamp_start_h : 10;*/
            uint16_t timestamp_start;
            uint16_t timestamp_count : 6;
        };
        imu_pack gyo2;
    };
}imu_report_pack;
//thk to HandHeldLegend and mission control guys
//we got a quaternion,which is normalized so only transfer 3 smaller component,
//which is translate into a 31bit digit include the sign bit.
//i dont know why dont just drop the first component(x) or the last component(w)
typedef struct{
    union{
        struct{
            uint8_t subcommand_status;//ack or nack
            uint8_t subcommand_id;
            uint8_t subcommand_data[NS_SUBCOMMAND_REPORT_DATA_MAX_LENGTH];
        }subcommand_report;
        uint8_t mcu_fw_update_report[NS_MCU_FW_UPDATE_REPORT_DATA_MAX_LENGTH];
        uint8_t raw_report_reserved[NS_BASIC_REPORT_RESERVED_MAX_LENGTH];
        imu_report_pack imu_report;
    };
}std_report_data;
typedef struct{
    uint32_t button_status:24;
    uint32_t ljoy_status:24;
    uint32_t rjoy_status:24;
}peripheral_data;
typedef struct{
    uint8_t typ;
    uint16_t button_status;
    uint8_t stick_hat_data;
    uint16_t ljoy_status_hori;
    uint16_t ljoy_status_vert;
    uint16_t rjoy_status_hori;
    uint16_t rjoy_status_vert;
}simple_report;
typedef struct{
    uint8_t typ;
    uint8_t timer;
    uint8_t con_info:4;
    uint8_t battery_status:4;
    peripheral_data input_data;
    uint8_t rumble_status;
    std_report_data data;
}std_report;
typedef struct{
    uint8_t typ;
    uint8_t timer;
    union{
        struct{
            uint32_t rumble_data_left;
            uint32_t rumble_data_right;
        };
        uint8_t rumble_data[8];
    };
}cmd_std;
typedef struct{
    cmd_std cmd_header;
    uint8_t subcommand_id;
    uint8_t subcommand_data[NS_SUBCOMMAND_CMD_DATA_MAX_LENGTH];
}cmd_subcommand;
typedef struct{
    cmd_std cmd_header;
    uint8_t mcu_fw_update_data[NS_MCU_FW_UPDATE_CMD_DATA_MAX_LENGTH];
}cmd_mcu_fw_update;
typedef struct{
    cmd_std cmd_header;
    uint8_t mcu_call_data[NS_MCU_CALL_CMD_DATA_MAX_LENGTH];
}cmd_mcu_call;
typedef struct{
    cmd_std cmd_header;
    uint8_t attachment_data[NS_ATTACHMENT_CMD_DATA_MAX_LENGTH];
}cmd_attachment;
#pragma pack(pop)

typedef struct{
    std_report_data data;
    void* oob_data;
    uint16_t len;
    uint32_t oob_len;
}report_packet;
typedef struct{
    union{
        cmd_std* cmd;
        uint8_t* data;
    };
    uint8_t len;
}cmd_packet;

extern NS_REPORT_MODE report_mode;
extern uint8_t is_connected,is_paired,auto_con,global_packet_timer;
extern uint32_t average_packet_gap,last_packet_send,max_packet_gap,tp_timer;
extern uint8_t input_mode;

extern void (*ns_cmd_subcommand_cb_tb[NS_SUBCOMMAND_ID_MAX_VLAUE])(cmd_subcommand*,uint8_t);

void ns_register_subcommand_cb(int,void (*)(cmd_subcommand*,uint8_t));//SET CB NULL TO UNREG
void ns_set_peripheral_data_getter(void (*)(peripheral_data*));
void ns_subcommand_callback_init();

#endif /* USER_NS_COM_H_ */
