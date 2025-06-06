/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32v10x_usbfs_device.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2020/04/30
 * Description        : header file for ch32v10x_usbfs_device.c
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

#ifndef USER_CH32V10X_USBFS_DEVICE_H_
#define USER_CH32V10X_USBFS_DEVICE_H_
#ifdef __cplusplus
 extern "C" {
#endif 

#include "debug.h"
#include "string.h"
#include "ch32v10x_usb.h"
#include "usb_desc.h"
#include "ring_buffer.h"

/******************************************************************************/
/* Global Define */
#ifndef __PACKED
  #define __PACKED   __attribute__((packed))
#endif

/* end-point number */
#define DEF_UEP_IN                    0x80
#define DEF_UEP_OUT                   0x00
#define DEF_UEP0                      0x00
#define DEF_UEP1                      0x01
#define DEF_UEP2                      0x02
#define DEF_UEP3                      0x03
#define DEF_UEP4                      0x04
#define DEF_UEP5                      0x05
#define DEF_UEP6                      0x06
#define DEF_UEP7                      0x07
#define DEF_UEP_NUM                   8

#define USBFS_UEP_MOD_BASE            0x4002340C
#define USBFS_UEP_DMA_BASE            0x40023410
#define USBFS_UEP_LEN_BASE            0x40023430
#define USBFS_UEP_CTL_BASE            0x40023432
#define USBFS_UEP_RX_EN               0x08
#define USBFS_UEP_TX_EN               0x04
#define USBFS_UEP_BUF_MOD             0x01
#define DEF_UEP_DMA_LOAD              0 /* Direct the DMA address to the data to be processed */
#define DEF_UEP_CPY_LOAD              1 /* Use memcpy to move data to a buffer */
#define USBFS_UEP_MOD(n)              (*((volatile uint8_t *)(USBFS_UEP_MOD_BASE+n)))
#define USBFS_UEP_CTRL(n)             (*((volatile uint8_t *)(USBFS_UEP_CTL_BASE+n*0x04)))
#define USBFS_UEP_DMA(n)              (*((volatile uint32_t *)(USBFS_UEP_DMA_BASE+n*0x04)))
#define USBFS_UEP_BUF(n)              ((uint8_t *)(*((volatile uint32_t *)(USBFS_UEP_DMA_BASE+n*0x04)))+0x20000000)
#define USBFS_UEP_TLEN(n)             (*((volatile uint16_t *)(USBFS_UEP_LEN_BASE+n*0x04)))

/* Ringbuffer define  */
#define DEF_Ring_Buffer_Max_Blks      24
#define DEF_RING_BUFFER_SIZE          (DEF_Ring_Buffer_Max_Blks*DEF_USBD_FS_PACK_SIZE)
#define DEF_RING_BUFFER_REMINE        4
#define DEF_RING_BUFFER_RESTART       8

#define NS_USB_RINGBUFFER_PKG_CAP (24+1)
#define NS_USB_RINGBUFFER_PKG_SIZE (64)

/* Ring Buffer typedef *//*
typedef struct __PACKED _RING_BUFF_COMM
{
    volatile uint8_t LoadPtr;
    volatile uint8_t DealPtr;
    volatile uint8_t RemainPack;
    volatile uint8_t PackLen[DEF_Ring_Buffer_Max_Blks];
    volatile uint8_t StopFlag;
} RING_BUFF_COMM, pRING_BUFF_COMM;*/

/* Setup Request Packets */
#define pUSBFS_SetupReqPak                 ((PUSB_SETUP_REQ)USBFS_EP0_Buf)

/*******************************************************************************/
/* Variable Definition */
/* Global */
extern const    uint8_t  *pUSBFS_Descr;

/* Setup Request */
extern volatile uint8_t  USBFS_SetupReqCode;
extern volatile uint8_t  USBFS_SetupReqType;
extern volatile uint16_t USBFS_SetupReqValue;
extern volatile uint16_t USBFS_SetupReqIndex;
extern volatile uint16_t USBFS_SetupReqLen;

/* USB Device Status */
extern volatile uint8_t  USBFS_DevConfig;
extern volatile uint8_t  USBFS_DevAddr;
extern volatile uint8_t  USBFS_DevSleepStatus;
extern volatile uint8_t  USBFS_DevEnumStatus;

/* Endpoint Buffer */
extern __attribute__ ((aligned(4))) uint8_t USBFS_EP0_Buf[];
extern __attribute__ ((aligned(4))) uint8_t USBFS_EP1_Buf[];


/* USB IN Endpoint Busy Flag */
extern volatile uint8_t  USBFS_Endp_Busy[ ];
extern ring_buffer ns_usb_send_rb;
extern ring_buffer ns_usb_recv_rb;
extern uint8_t ns_usb_send_buf[NS_USB_RINGBUFFER_PKG_CAP*NS_USB_RINGBUFFER_PKG_SIZE];
extern uint8_t ns_usb_recv_buf[NS_USB_RINGBUFFER_PKG_CAP*NS_USB_RINGBUFFER_PKG_SIZE];

/* Ringbuffer variables */
//extern RING_BUFF_COMM  RingBuffer_Comm;
//extern __attribute__ ((aligned(4))) uint8_t  Data_Buffer[];

/******************************************************************************/
/* external functions */
extern void USBFS_Device_Init( FunctionalState sta , PWR_VDD VDD_Voltage);
extern void USBFS_Device_Endp_Init(void);
extern void USBFS_RCC_Init(void);
extern void USBFS_Send_Resume(void);
extern void USBFS_Sleep_Wakeup_CFG( void );

#ifdef __cplusplus
}
#endif


#endif /* USER_CH32V10X_USB_DEVICE_H_ */
