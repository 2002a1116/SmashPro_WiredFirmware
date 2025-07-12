/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb_desc.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2020/04/30
 * Description        : usb device descriptor,configuration descriptor,
 *                      string descriptors and other descriptors.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

#include "usb_desc.h"

/* Device Descriptor */
const uint8_t  MyDevDescr[] =
{
    0x12,                                               // bLength
    0x01,                                               // bDescriptorType (Device)
    0x02, 0x00,                                         // bcdUSB 2.00
    0x00,                                               // bDeviceClass (Use class information in the Interface Descriptors)
    0x00,                                               // bDeviceSubClass
    0x00,                                               // bDeviceProtocol
    DEF_USBD_UEP0_SIZE,                                 // bMaxPacketSize0
    0x7E,0x05,
    0x09,0x20,
    //(uint8_t)DEF_USB_VID, (uint8_t)(DEF_USB_VID >> 8),  // idVendor 0x1A86
    //(uint8_t)DEF_USB_PID, (uint8_t)(DEF_USB_PID >> 8),  // idProduct 0xE6E1
    0x00, DEF_IC_PRG_VER,                               // bcdDevice 1.00
    0x01,                                               // iManufacturer (String Index)
    0x02,                                               // iProduct (String Index)
    0x03,                                               // iSerialNumber (String Index)
    0x01,                                               // bNumConfigurations 1
};

/* Configuration Descriptor */
uint8_t  MyCfgDescr[] =
{
    /* Configuration Descriptor */
    0x09,                           // bLength
    0x02,                           // bDescriptorType
    0x29, 0x00,                     // wTotalLength
    0x01,                           // bNumInterfaces
    0x01,                           // bConfigurationValue
    0x03,                           // iConfiguration (String Index)
    0x80,                           // bmAttributes Remote Wakeup
    0x23,                           // bMaxPower 70mA

    /* Interface Descriptor */
    0x09,                           // bLength
    0x04,                           // bDescriptorType (Interface)
    0x00,                           // bInterfaceNumber 0
    0x00,                           // bAlternateSetting
    0x02,                           // bNumEndpoints 2
    0x03,                           // bInterfaceClass
    0x00,                           // bInterfaceSubClass
    0x00,                           // bInterfaceProtocol
    0x00,                           // iInterface (String Index)

    /* HID Descriptor */
    0x09,                           // bLength
    0x21,                           // bDescriptorType
    0x11, 0x01,                     // bcdHID
    0x00,                           // bCountryCode
    0x01,                           // bNumDescriptors
    0x22,                           // bDescriptorType
    DEF_USBD_REPORT_DESC_LEN & 0xFF, DEF_USBD_REPORT_DESC_LEN >> 8, // wDescriptorLength

    /* Endpoint Descriptor */
    0x07,                           // bLength
    0x05,                           // bDescriptorType
    0x01,                           // bEndpointAddress: OUT Endpoint 1
    0x03,                           // bmAttributes
    0x40, 0x00,                     // wMaxPacketSize
    0x08,                           // bInterval: 8mS index 33

    /* Endpoint Descriptor */
    0x07,                           // bLength
    0x05,                           // bDescriptorType
    0x81,                           // bEndpointAddress: IN Endpoint 1
    0x03,                           // bmAttributes
    0x40, 0x00,                     // wMaxPacketSize
    0x08,                           // bInterval: 8mS index 40
};
/* HID Report Descriptor */
/*
const uint8_t  MyHIDReportDesc[] =
{
    0x06, 0x00, 0xFF,               // Usage Page (Vendor Defined 0xFF00)
    0x09, 0x01,                     // Usage (0x01)
    0xA1, 0x01,                     // Collection (Application)
    0x09, 0x02,                     //   Usage (0x02)
    0x26, 0xFF, 0x00,               //   Logical Maximum (255)
    0x75, 0x08,                     //   Report Size (8)
    0x15, 0x00,                     //   Logical Minimum (0)
    0x95, 0x40,                     //   Report Count (64)
    0x81, 0x06,                     //   Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0x02,                     //   Usage (0x02)
    0x15, 0x00,                     //   Logical Minimum (0)
    0x26, 0xFF, 0x00,               //   Logical Maximum (255)
    0x75, 0x08,                     //   Report Size (8)
    0x95, 0x40,                     //   Report Count (64)
    0x91, 0x06,                     //   Output (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,                           // End Collection
};
*/
const uint8_t MyHIDReportDesc[] =
{
        0x05, 0x01, // Usage Page (Generic Desktop Ctrls)
        //0x05,0x05,
            0x15, 0x00, // Logical Minimum (0)

            0x09, 0x04, // Usage (Joystick)
            //0x09,0x05,
            0xA1, 0x01, // Collection (Application)

            0x85, 0x30, //   Report ID (48)
            0x05, 0x01, //   Usage Page (Generic Desktop Ctrls)
            0x05, 0x09, //   Usage Page (Button)
            0x19, 0x01, //   Usage Minimum (0x01)
            0x29, 0x0A, //   Usage Maximum (0x0A)
            0x15, 0x00, //   Logical Minimum (0)
            0x25, 0x01, //   Logical Maximum (1)
            0x75, 0x01, //   Report Size (1)
            0x95, 0x0A, //   Report Count (10)
            0x55, 0x00, //   Unit Exponent (0)
            0x65, 0x00, //   Unit (None)
            0x81, 0x02, //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x05, 0x09, //   Usage Page (Button)
            0x19, 0x0B, //   Usage Minimum (0x0B)
            0x29, 0x0E, //   Usage Maximum (0x0E)
            0x15, 0x00, //   Logical Minimum (0)
            0x25, 0x01, //   Logical Maximum (1)
            0x75, 0x01, //   Report Size (1)
            0x95, 0x04, //   Report Count (4)
            0x81, 0x02, //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x75, 0x01, //   Report Size (1)
            0x95, 0x02, //   Report Count (2)
            0x81, 0x03, //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

            0x0B, 0x01, 0x00, 0x01, 0x00, //   Usage (0x010001)
            0xA1, 0x00,                   //   Collection (Physical)
            0x0B, 0x30, 0x00, 0x01, 0x00, //     Usage (0x010030)
            0x0B, 0x31, 0x00, 0x01, 0x00, //     Usage (0x010031)
            0x0B, 0x32, 0x00, 0x01, 0x00, //     Usage (0x010032)
            0x0B, 0x35, 0x00, 0x01, 0x00, //     Usage (0x010035)
            0x15, 0x00,                   //     Logical Minimum (0)
            0x27, 0xFF, 0xFF, 0x00, 0x00, //     Logical Maximum (65534)
            0x75, 0x10,                   //     Report Size (16)
            0x95, 0x04,                   //     Report Count (4)
            0x81, 0x02,                   //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0xC0,                         //   End Collection

            0x0B, 0x39, 0x00, 0x01, 0x00, //   Usage (0x010039)
            0x15, 0x00,                   //   Logical Minimum (0)
            0x25, 0x07,                   //   Logical Maximum (7)
            0x35, 0x00,                   //   Physical Minimum (0)
            0x46, 0x3B, 0x01,             //   Physical Maximum (315)
            0x65, 0x14,                   //   Unit (System: English Rotation, Length: Centimeter)
            0x75, 0x04,                   //   Report Size (4)
            0x95, 0x01,                   //   Report Count (1)
            0x81, 0x02,                   //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x05, 0x09,                   //   Usage Page (Button)
            0x19, 0x0F,                   //   Usage Minimum (0x0F)
            0x29, 0x12,                   //   Usage Maximum (0x12)
            0x15, 0x00,                   //   Logical Minimum (0)
            0x25, 0x01,                   //   Logical Maximum (1)
            0x75, 0x01,                   //   Report Size (1)
            0x95, 0x04,                   //   Report Count (4)
            0x81, 0x02,                   //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x75, 0x08,                   //   Report Size (8)
            0x95, 0x34,                   //   Report Count (52)
            0x81, 0x03,                   //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

            0x06, 0x00, 0xFF, //   Usage Page (Vendor Defined 0xFF00)
            0x85, 0x21,       //   Report ID (33)
            0x09, 0x01,       //   Usage (0x01)
            0x75, 0x08,       //   Report Size (8)
            0x95, 0x3F,       //   Report Count (63)
            0x81, 0x03,       //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

            0x85, 0x81, //   Report ID (-127)
            0x09, 0x02, //   Usage (0x02)
            0x75, 0x08, //   Report Size (8)
            0x95, 0x3F, //   Report Count (63)
            0x81, 0x03, //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

            0x85, 0x01, //   Report ID (1)
            0x09, 0x03, //   Usage (0x03)
            0x75, 0x08, //   Report Size (8)
            0x95, 0x3F, //   Report Count (63)
            0x91, 0x83, //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)

            0x85, 0x10, //   Report ID (16)
            0x09, 0x04, //   Usage (0x04)
            0x75, 0x08, //   Report Size (8)
            0x95, 0x3F, //   Report Count (63)
            0x91, 0x83, //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)

            0x85, 0x80, //   Report ID (-128)
            0x09, 0x05, //   Usage (0x05)
            0x75, 0x08, //   Report Size (8)
            0x95, 0x3F, //   Report Count (63)
            0x91, 0x83, //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)

            0x85, 0x82, //   Report ID (-126)
            0x09, 0x06, //   Usage (0x06)
            0x75, 0x08, //   Report Size (8)
            0x95, 0x3F, //   Report Count (63)
            0x91, 0x83, //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)

            //custom report implement with 0x21 subcommand
            //upd 2025.2.24 move fw report to hid layer

            0x85, 0xFE,
            0x09, 0x07,
            0x75, 0x08,
            0x95, 0x3F,
            0x81, 0x02,

            0X85, 0XFE,
            0X09, 0X07,
            0X75, 0X08,
            0X95, 0X3F,
            0X91, 0X82,

            0xC0, // End Collection
};
/* Language Descriptor */
const uint8_t  MyLangDescr[] =
{
    0x04, 0x03, 0x09, 0x04
};

/* Manufacturer Descriptor */
//const uint8_t  MyManuInfo[] ={0x0E, 0x03, 'w', 0, 'c', 0, 'h', 0, '.', 0, 'c', 0, 'n', 0};
//Reed
const uint8_t  MyManuInfo[] ={0x0A, 0x03, 'R', 0, 'e', 0, 'e', 0, 'd', 0};

/* Product Information */
//const uint8_t  MyProdInfo[] ={0x12, 0x03, 'C', 0, 'H', 0, '3', 0, '2', 0, 'V', 0, '1', 0, '0', 0, 'x', 0};
//Smash Pro Controller
const uint8_t MyProdInfo[]={0x2A,0x03,'S',0,'m',0,'a',0,'s',0,'h',0,' ',0,'P',0,'r',0,'o',0,
                           ' ',0,'C',0,'o',0,'n',0,'t',0,'r',0,'o',0,'l',0,'l',0,'e',0,'r',0};

/* Serial Number Information */
//const uint8_t  MySerNumInfo[] ={0x16, 0x03, '0', 0, '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, '8', 0, '9', 0};
//1145141919
const uint8_t  MySerNumInfo[] ={0x16, 0x03, '0', 0, '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, '8', 0, '9', 0};
