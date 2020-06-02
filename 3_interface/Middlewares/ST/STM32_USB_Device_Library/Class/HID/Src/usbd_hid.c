/**
  ******************************************************************************
  * @file    usbd_hid.c
  * @author  MCD Application Team
  * @brief   This file provides the HID core functions.
  *
  * @verbatim
  *
  *          ===================================================================
  *                                HID Class  Description
  *          ===================================================================
  *           This module manages the HID class V1.11 following the "Device Class Definition
  *           for Human Interface Devices (HID) Version 1.11 Jun 27, 2001".
  *           This driver implements the following aspects of the specification:
  *             - The Boot Interface Subclass
  *             - The Mouse protocol
  *             - Usage Page : Generic Desktop
  *             - Usage : Joystick
  *             - Collection : Application
  *
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *
  *
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbd_hid.h"
#include "usbd_ctlreq.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_HID
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_HID_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_HID_Private_Defines
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_HID_Private_Macros
  * @{
  */
/**
  * @}
  */




/** @defgroup USBD_HID_Private_FunctionPrototypes
  * @{
  */


static uint8_t  USBD_HID_Init(USBD_HandleTypeDef *pdev,
                              uint8_t cfgidx);

static uint8_t  USBD_HID_DeInit(USBD_HandleTypeDef *pdev,
                                uint8_t cfgidx);

static uint8_t  USBD_HID_Setup(USBD_HandleTypeDef *pdev,
                               USBD_SetupReqTypedef *req);

static uint8_t  *USBD_HID_GetFSCfgDesc(uint16_t *length);
#if 0
static uint8_t  *USBD_HID_GetHSCfgDesc(uint16_t *length);

static uint8_t  *USBD_HID_GetOtherSpeedCfgDesc(uint16_t *length);

static uint8_t  *USBD_HID_GetDeviceQualifierDesc(uint16_t *length);
#endif
static uint8_t  USBD_HID_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
/**
  * @}
  */

/** @defgroup USBD_HID_Private_Variables
  * @{
  */

USBD_ClassTypeDef  USBD_HID =
{
  USBD_HID_Init,
  USBD_HID_DeInit,
  USBD_HID_Setup,
  NULL, /*EP0_TxSent*/
  NULL, /*EP0_RxReady*/
  USBD_HID_DataIn, /*DataIn*/
  NULL, /*DataOut*/
  NULL, /*SOF */
  NULL,
  NULL,
  NULL, //USBD_HID_GetHSCfgDesc,
  USBD_HID_GetFSCfgDesc,
  NULL, //USBD_HID_GetOtherSpeedCfgDesc,
  NULL, //USBD_HID_GetDeviceQualifierDesc,
};

/* USB HID device FS Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_CfgFSDesc[USB_HID_CONFIG_DESC_SIZ]  __ALIGN_END =
{
  0x09, /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
  USB_HID_CONFIG_DESC_SIZ,
  /* wTotalLength: Bytes returned */
  0x00,
  0x03,         /*bNumInterfaces: 1 interface*/
  0x01,         /*bConfigurationValue: Configuration value*/
  0x00,         /*iConfiguration: Index of string descriptor describing
  the configuration*/
  0xA0,         /*bmAttributes: bus powered and Support Remote Wake-up */
  0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/

  /************** interface_0 ****************/
  /* 09 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  0x00,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x01,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: HID*/
  0x01,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x01,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0,            /*iInterface: Index of string descriptor*/
  /******************** HID_0 ********************/
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  HID_Keyboard_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
  /******************** endpoint_1 ********************/
  /* 27 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/
  HID_EPIN_1_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  0x08, /*wMaxPacketSize: 8 Byte max */
  0x00,
  HID_FS_BINTERVAL,          /*bInterval: Polling Interval */
  /* 34 */
	/************** interface_1 ****************/
  /* 09 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  0x01,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x01,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: HID*/
  0x00,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of Joystick Mouse HID ********************/
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  HID_Mouse_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
  /******************** Descriptor of Mouse endpoint ********************/
  /* 27 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/
  HID_EPIN_2_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  0x40, /*wMaxPacketSize: 64 Byte max */
  0x00,
  HID_FS_BINTERVAL,          /*bInterval: Polling Interval */
  /* 34 */
	/************** interface_2 ****************/
  /* 09 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  0x02,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x01,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: HID*/
  0x01,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x01,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0,            /*iInterface: Index of string descriptor*/
  /******************** HID_2 ********************/
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  HID_Dial_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
  /******************** Descriptor of Mouse endpoint ********************/
  /* 27 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/
  HID_EPIN_3_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  0x40, /*wMaxPacketSize: 64 Byte max */
  0x00,
  HID_FS_BINTERVAL,          /*bInterval: Polling Interval */
  /* 34 */
};
#if 0
/* USB HID device HS Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_CfgHSDesc[USB_HID_CONFIG_DESC_SIZ]  __ALIGN_END =
{
  0x09, /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
  USB_HID_CONFIG_DESC_SIZ,
  /* wTotalLength: Bytes returned */
  0x00,
  0x01,         /*bNumInterfaces: 1 interface*/
  0x01,         /*bConfigurationValue: Configuration value*/
  0x00,         /*iConfiguration: Index of string descriptor describing
  the configuration*/
  0xE0,         /*bmAttributes: bus powered and Support Remote Wake-up */
  0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/

  /************** Descriptor of Joystick Mouse interface ****************/
  /* 09 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  0x00,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x01,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: HID*/
  0x01,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x02,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of Joystick Mouse HID ********************/
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  HID_MOUSE_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
  /******************** Descriptor of Mouse endpoint ********************/
  /* 27 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

  HID_EPIN_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  HID_EPIN_SIZE, /*wMaxPacketSize: 4 Byte max */
  0x00,
  HID_HS_BINTERVAL,          /*bInterval: Polling Interval */
  /* 34 */
};

/* USB HID device Other Speed Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_OtherSpeedCfgDesc[USB_HID_CONFIG_DESC_SIZ]  __ALIGN_END =
{
  0x09, /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
  USB_HID_CONFIG_DESC_SIZ,
  /* wTotalLength: Bytes returned */
  0x00,
  0x01,         /*bNumInterfaces: 1 interface*/
  0x01,         /*bConfigurationValue: Configuration value*/
  0x00,         /*iConfiguration: Index of string descriptor describing
  the configuration*/
  0xE0,         /*bmAttributes: bus powered and Support Remote Wake-up */
  0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/

  /************** Descriptor of Joystick Mouse interface ****************/
  /* 09 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  0x00,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x01,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: HID*/
  0x01,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x02,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of Joystick Mouse HID ********************/
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  HID_MOUSE_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
  /******************** Descriptor of Mouse endpoint ********************/
  /* 27 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

  HID_EPIN_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  HID_EPIN_SIZE, /*wMaxPacketSize: 4 Byte max */
  0x00,
  HID_FS_BINTERVAL,          /*bInterval: Polling Interval */
  /* 34 */
};


/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_Desc[USB_HID_DESC_SIZ]  __ALIGN_END  =
{
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  HID_MOUSE_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC]  __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

#endif

__ALIGN_BEGIN static uint8_t HID_Keyboard_ReportDesc[HID_Keyboard_REPORT_DESC_SIZE]  __ALIGN_END =
{
	        0x05,0x01,                                     //   Usage Page      (Desktop)
          0x09,0x06,                                     //   Usage           (Keyboard)
          0xA1,0x01,                                     //   Collection      (Application)
//---------Modify ---------------------------------------//
          0x05,0x07,                                     //   Usage Page      (Keyboard)
          0x19,0xE0,                                     //   Usage Minimum   (Keyboard LeftControl)
          0x29,0xE7,                                     //   Usage Maximum   (Keyboard Right GUI)
          0x15,0x00,                                     //   Logical Minimum (0)
          0x25,0x01,                                     //   Logical Maximum (1)
          0x75,0x01,                                     //   Report Size     (1)
          0x95,0x08,                                     //   Report Count    (8)
          0x81,0x02,                                     //   Input           (Data,Var,Abs)
//---------Constant -------------------------------------//
          0x75,0x08,                                     //   Report Size     (8)
          0x95,0x01,                                     //   Report Count    (1)
          0x81,0x01,                                     //   Input           (Constant)
//---------Key Code -------------------------------------// 
          0x05,0x07,                                     //   Usage Page      (Keyboard)
          0x19,0x00,                                     //   Usage Minimum   (00)
          0x2A,0xFF,0x00,                                //   Usage Maximum   (FF)
          0x15,0x00,                                     //   Logical Minimum (0)
          0x26,0xFF,0x00,                                //   Logical Maximum (FF)
          0x95,0x06,                                     //   Report Count    (06)
          0x81,0x00,                                     //   Input           (Data,Array,Abs)
//---------Led ------------------------------------------//
          0x05,0x08,                                     //   Usage Page      (LED)
          0x25,0x01,                                     //   Logical Maximum (01)
          0x19,0x01,                                     //   Usage Minimum   (Num Lock)
          0x29,0x05,                                     //   Usage Maximum   (Kana)
          0x75,0x01,                                     //   Report Size     (01)
          0x95,0x05,                                     //   Report Count    (05)
          0x91,0x02,                                     //   Output          (Var)
          0x75,0x01,                                     //   Report Size     (01)
          0x95,0x03,                                     //   Report Count    (03)
          0x91,0x01,                                     //   Output          (Constant)
          0xC0,                                          //   End Collection		
};

__ALIGN_BEGIN static uint8_t HID_Mouse_ReportDesc[HID_Mouse_REPORT_DESC_SIZE]  __ALIGN_END =
{
   //-----------------Consumer Control--------------------------;;
							0x05,0x0C,                                     //   Usage Page          (Consumer Device)
							0x09,0x01,                                     //   Usage               (Consumer Control)
							0xA1,0x01,                                     //   Collection          (Application)
							0x85,0x04,                                     //   Report ID         (04)
							0x19,0x00,                                     //   Usage Minimum     (00)
							0x2A,0x9C,0x02,                                //   Usage Maximum     (029C)
							0x15,0x00,                                     //   Logical Minimum   (00)
							0x26,0x9C,0x02,                                //   Logical Maximum (AC Distribute Vertically/029C)
							0x95,0x01,                                     //   Report Count      (01)
							0x75,0x10,                                     //   Report Size       (1 Word)
							0x81,0x00,                                     //   Input             (Dat,Array)
							0xC0,                                          //   End Collection
//------------------System Control --------------------------;;
							0x05,0x01,                                     //    Usage Page          (Desktop)
							0x09,0x80,                                     //    Usage               (System Control)
							0xA1,0x01,                                     //    Collection          (Application)
							0x85,0x05,                                     //    Report ID         (05)
							0x19,0x81,                                     //    Usage Minimum     ( Power Down )
							0x29,0x83,                                     //    Usage Maximum     ( Wake up )
							0x15,0x00,                                     //    Logical Minimum   (00)
							0x25,0x01,                                     //    Logical Maximum   (01)
							0x75,0x01,                                     //    Report Size       (01)
							0x95,0x03,                                     //    Report Count      (03)
							0x81,0x02,                                     //    Input             (Dat,Var)
							0x95,0x05,                                     //    Report Count      (05)
							0x81,0x01,                                     //    Input             (Cnt)
							0xC0,                                          //    End Collection		
//-------------------- Mouse With AC Pan --------------------;;
							0x05, 0x01,                                    //     USAGE PAGE (Generic Desktop)
							0x09, 0x02,                                    //     USAGE (Mouse)
							0xA1, 0x01,                                    //     COLLECTION (Application)
							0x05, 0x01,                                    //     USAGE PAGE (Generic Desktop)
							0x09, 0x02,                                    //     USAGE (Mouse)
							0xA1, 0x02,                                    //     COLLECTION (Logical)
							0x85, 0x01,                                    //     REPORT_ID (0x01)
							0x09, 0x01,                                    //     USAGE (Pointer) 
							0xA1, 0x00,                                    //   	COLLECTION (Physical)
							0x05, 0x09,                                    //     USAGE_PAGE (Button)
							0x19, 0x01,                                    //     USAGE_MINIMUM (Button 1)
							0x29, 0x08,                                    //     USAGE_MAXIMUM (Button 8)
							0x95, 0x08,                                    //     REPORT_COUNT (8)
							0x75, 0x01,                                    //     REPORT_SIZE (1)
							0x15, 0x00,                                    //     LOGICAL_MINIMUM (0)
							0x25, 0x01,                                    //     LOGICAL_MAXIMUM (1)
							0x81, 0x02,                                    //     INPUT (Data,Var,Abs)
							0x05, 0x01,                                    //     USAGE_PAGE (Generic Desktop)
							0x09, 0x30,                                    //     USAGE (X)
							0x09, 0x31,                                    //     USAGE (Y) 
//----------------------- Wheel -----------------------------;;
							0x09, 0x38,                                    //       Usage      (Wheel) 
//---------------------------                                //
							0x95, 0x03,                                    //       REPORT_COUNT (3)
							0x75, 0x08,                                    //       REPORT_SIZE (8)   
							0x15, 0x81,                                    //       Logical Minimum(-127)
							0x25, 0x7F,                                    //       Logical Maximum(+127)
							0x81, 0x06,                                    //       INPUT (Var, Rel)
//-----  AC PAN ---------------------------------------------;; 
							0x05, 0x0c,           						             //       USAGE_PAGE (Consumer Devices)		
							0x0A, 0x38,0x02,                               //       Usage      (AC Pan)																						     
							0x95, 0x01,                                    //       REPORT_COUNT (1)
							0x75, 0x08,                                    //       REPORT_SIZE (8)   
							0x15, 0x81,                                    //       Logical Minimum(-127)
							0x25, 0x7F,                                    //       Logical Maximum(+127)
							0x81, 0x06,                                    //       INPUT (Var, Rel)
							0xC0,                                          //       END COLLECTION (Physical)   			
							0xC0,                                          //       END COLLECTION (Logical)
							0xC0,                                          //       END COLLECTION (Application) 

              0x06, 0x01,0xFF,                               //    USAGE_PAGE (Vendor Define)
              0x09, 0x00,                                    //    USAGE (Undefine)
              0xA1, 0x01,                                    //    COLLECTION (Application)
              0x85, 0x0A,                                    //      Report ID         (0A)
              0x15, 0x00,                                    //      LOGICAL_MINIMUM (0)
              0x26, 0xFF, 0x00,                              //      LOGICAL_MAXIMUM (255)
              0x09, 0x00,                                    //      USAGE (Undefine)
              0x75, 0x08,                                    //      REPORT_SIZE (8)
              0x95, 0x24,                                    //      REPORT_COUNT (36)
              0xB1, 0x02,                                    //      Feature
              0xC0,                                          //    END_COLLECTION 
};

__ALIGN_BEGIN static uint8_t HID_Dial_ReportDesc[HID_Dial_REPORT_DESC_SIZE]  __ALIGN_END =
{
		       0x05, 0x01,                                    //    Usage Page (Generic Desktop Ctrls)
           0x09, 0x0E,                                    //    Usage (0x0E)
           0xA1, 0x01,                                    //    Collection (Application)
           0x85, 0x10,                                    //    Report ID (0x10)
           0x05, 0x0D,                                    //    Usage Page (Digitizer)
           0x09, 0x21,                                    //    Usage (Puck)
           0xA1, 0x02,                                    //    Collection (Logical)
           0x15, 0x00,                                    //    Logical Minimum (0)
           0x25, 0x01,                                    //    Logical Maximum (1)
           0x75, 0x01,                                    //    Report Size (1)
           0x95, 0x01,                                    //    Report Count (1)
           0xA1, 0x00,                                    //    Collection (Physical)
           0x05, 0x09,                                    //    Usage Page (Button)
           0x09, 0x01,                                    //    Usage (Button 1)
           0x81, 0x02,                                    //    Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
           0x05, 0x0D,                                    //    Usage Page (Digitizer)
           0x09, 0x33,                                    //    Usage (Touch)
           0x81, 0x02,                                    //    Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
           0x95, 0x06,                                    //    Report Count (6)
           0x81, 0x03,                                    //    Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
           0xA1, 0x02,                                    //    Collection (Logical)
           0x05, 0x01,                                    //    Usage Page (Generic Desktop Ctrls)
           0x09, 0x37,                                    //    Usage (Dial)
           0x16, 0x00,0x80,                               //    Logical Minimum (-32768)
           0x26, 0xFF,0x7F,                               //    Logical Maximum (+32767)
           0x75, 0x10,                                    //    Report Size (16)
           0x95, 0x01,                                    //    Report Count (1)
           0x81, 0x06,                                    //    Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
           0x35, 0x00,                                    //    Physical Minimum (0)
           0x46, 0x10,0x0E,                               //    Physical Maximum (3600)
           0x15, 0x00,                                    //    Logical Minimum (0)
           0x26, 0x10,0x0E,                               //    Logical Maximum (3600)
           0x09, 0x48,                                    //    Usage (0x48)
           0xB1, 0x02,                                    //    Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
           0x45, 0x00,                                    //    Physical Maximum (0)
           0xC0,                                          //    End Collection
           0x55, 0x0E,                                    //    Unit Exponent (-2)
           0x65, 0x11,                                    //    Unit (System: SI Linear, Length: Centimeter)
									                                        //
           0x46, 0x00,0x00,                               //       Physical Maximum (0)
           0x26, 0x00,0x00,                               //       Logical Maximum (0)
           0x09, 0x30,                                    //       Usage (X)
           0x81, 0x42,                                    //       Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
           0x09, 0x31,                                    //       Usage (Y)
           0x46, 0x00,0x00,                               //       Physical Maximum (0)
           0x26, 0x00,0x00,                               //       Logical Maximum (0)
           0x81, 0x42,                                    //       Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
									                                        //
           0x05, 0x0D,                                    //       Usage Page (Digitizer)
           0x09, 0x48,                                    //       Usage (0x48)
           0x15, 0x3A,                                    //       Logical Minimum (58)
           0x25, 0x3A,                                    //       Logical Maximum (58)
           0x75, 0x08,                                    //       Report Size (8)
           0x55, 0x0F,                                    //       Unit Exponent (-1)
           0x35, 0x3A,                                    //       Physical Minimum (58)
           0x45, 0x3A,                                    //       Physical Maximum (58)
           0x81, 0x03,                                    //       Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
           0x55, 0x00,                                    //       Unit Exponent (0)
           0x65, 0x00,                                    //       Unit (None)
           0x35, 0x00,                                    //       Physical Minimum (0)
           0x45, 0x00,                                    //       Physical Maximum (0)
           0x05, 0x0E,                                    //       Usage Page (Reserved 0x0E)
           0x09, 0x01,                                    //       Usage (0x01)
           0xA1, 0x02,                                    //       Collection (Logical)
           0x15, 0x00,                                    //         Logical Minimum (0)
           0x26, 0xFF,0x00,                               //         Logical Maximum (255)
           0x09, 0x24,                                    //         Usage (0x24)
           0xB1, 0x42,                                    //         Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State,Non-volatile)
           0x09, 0x24,                                    //         Usage (0x24)
           0x91, 0x42,                                    //         Output (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State,Non-volatile)
           0x15, 0x01,                                    //         Logical Minimum (1)
           0x25, 0x07,                                    //         Logical Maximum (7)
           0x09, 0x20,                                    //         Usage (0x20)
           0xB1, 0x42,                                    //         Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State,Non-volatile)
           0x09, 0x21,                                    //         Usage (0x21)
           0x91, 0x42,                                    //         Output (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State,Non-volatile)
           0x25, 0x0A,                                    //         Logical Maximum (10)
           0x09, 0x28,                                    //         Usage (0x28)
           0xB1, 0x42,                                    //         Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State,Non-volatile)
           0x75, 0x10,                                    //         Report Size (16)
           0x26, 0xD0,0x07,                               //         Logical Maximum (2000)
           0x09, 0x25,                                    //         Usage (0x25)
           0xB1, 0x42,                                    //         Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State,Non-volatile)
           0x09, 0x25,                                    //         Usage (0x25)
           0x91, 0x42,                                    //         Output (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State,Non-volatile)
           0xC0,                                          //      End Collection
           0xC0,                                          //      End Collection
           0xC0,                                          //      End Collection
           0xC0,                                          //      End Collection*/			
};

/**
  * @}
  */

/** @defgroup USBD_HID_Private_Functions
  * @{
  */

/**
  * @brief  USBD_HID_Init
  *         Initialize the HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  /* Open EP IN */
  USBD_LL_OpenEP(pdev, HID_EPIN_1_ADDR, USBD_EP_TYPE_INTR, EP1_PACKET_SIZE);
  pdev->ep_in[HID_EPIN_1_ADDR & 0xFU].is_used = 1U;

	USBD_LL_OpenEP(pdev, HID_EPIN_2_ADDR, USBD_EP_TYPE_INTR, EP2_PACKET_SIZE);
  pdev->ep_in[HID_EPIN_2_ADDR & 0xFU].is_used = 1U;
    
  USBD_LL_OpenEP(pdev, HID_EPIN_3_ADDR, USBD_EP_TYPE_INTR, EP3_PACKET_SIZE);
  pdev->ep_in[HID_EPIN_3_ADDR & 0xFU].is_used = 1U;
	
  pdev->pClassData = USBD_malloc(sizeof(USBD_HID_HandleTypeDef));

  if (pdev->pClassData == NULL)
  {
    return USBD_FAIL;
  }

  ((USBD_HID_HandleTypeDef *)pdev->pClassData)->state = HID_IDLE;

  return USBD_OK;
}

/**
  * @brief  USBD_HID_Init
  *         DeInitialize the HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_DeInit(USBD_HandleTypeDef *pdev,
                                uint8_t cfgidx)
{
  /* Close HID EPs */
  USBD_LL_CloseEP(pdev, HID_EPIN_1_ADDR);
  pdev->ep_in[HID_EPIN_1_ADDR & 0xFU].is_used = 0U;

	 USBD_LL_CloseEP(pdev, HID_EPIN_2_ADDR);
  pdev->ep_in[HID_EPIN_2_ADDR & 0xFU].is_used = 0U;
    
  USBD_LL_CloseEP(pdev, HID_EPIN_3_ADDR);
  pdev->ep_in[HID_EPIN_3_ADDR & 0xFU].is_used = 0U;
  /* FRee allocated memory */
  if (pdev->pClassData != NULL)
  {
    USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }

  return USBD_OK;
}

/**
  * @brief  USBD_HID_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_HID_Setup(USBD_HandleTypeDef *pdev,
                               USBD_SetupReqTypedef *req)
{
  USBD_HID_HandleTypeDef *hhid = (USBD_HID_HandleTypeDef *) pdev->pClassData;
  uint16_t len = 0U;
  uint8_t *pbuf = NULL;
  uint16_t status_info = 0U;
  USBD_StatusTypeDef ret = USBD_OK;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS :
      switch (req->bRequest)
      {
        case HID_REQ_SET_PROTOCOL:
          hhid->Protocol = (uint8_t)(req->wValue);
          break;

        case HID_REQ_GET_PROTOCOL:
          USBD_CtlSendData(pdev, (uint8_t *)(void *)&hhid->Protocol, 1U);
          break;

        case HID_REQ_SET_IDLE:
          hhid->IdleState = (uint8_t)(req->wValue >> 8);
          break;

        case HID_REQ_GET_IDLE:
          USBD_CtlSendData(pdev, (uint8_t *)(void *)&hhid->IdleState, 1U);
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;
    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            USBD_CtlSendData(pdev, (uint8_t *)(void *)&status_info, 2U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_GET_DESCRIPTOR:
          if (req->wValue >> 8 == HID_REPORT_DESC)
          {
           switch ((uint8_t)(req->wIndex & 0xFF))
						{
							case 0:
								len = MIN(HID_Keyboard_REPORT_DESC_SIZE, req->wLength);
                pbuf = HID_Keyboard_ReportDesc;
								break;
							case 1:
								len = MIN(HID_Mouse_REPORT_DESC_SIZE, req->wLength);
                pbuf = HID_Mouse_ReportDesc;
							
								break;
							case 2:
								len = MIN(HID_Dial_REPORT_DESC_SIZE, req->wLength);
                pbuf = HID_Dial_ReportDesc;
								break;
							case 3:
							case 4:
							case 5:
              default:
								USBD_CtlError(pdev, req);
                ret = USBD_FAIL;
                break;								
						}
          }
          else if (req->wValue >> 8 == HID_DESCRIPTOR_TYPE)
          {
           switch ((uint8_t)(req->wIndex & 0xFF))
						{
							case 0:
								len =9;//MIN(USB_HID_DESC_SIZ, req->wLength);
                pbuf = (uint8_t *)(USBD_HID_CfgFSDesc + 18);
								break;
							case 1:
								len = 9;//MIN(USB_HID_DESC_SIZ, req->wLength);
                pbuf = (uint8_t *)(USBD_HID_CfgFSDesc + 43);
								break;
							case 2:
								len = 9;//MIN(USB_HID_DESC_SIZ, req->wLength);
                pbuf = (uint8_t *)(USBD_HID_CfgFSDesc + 68);
								break;
							case 3:
							case 4:
							case 5:
              default:
								USBD_CtlError(pdev, req);
                ret = USBD_FAIL;
                break;		
						}							
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
            break;
          }
          USBD_CtlSendData(pdev, pbuf, len);
          break;

        case USB_REQ_GET_INTERFACE :
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            USBD_CtlSendData(pdev, (uint8_t *)(void *)&hhid->AltSetting, 1U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_SET_INTERFACE :
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            hhid->AltSetting = (uint8_t)(req->wValue);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
  }

  return ret;
}

/**
  * @brief  USBD_HID_SendReport
  *         Send HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @retval status
  */
uint8_t USBD_HID_SendReport(USBD_HandleTypeDef  *pdev,
                            uint8_t *report,
                            uint16_t len)
{
  USBD_HID_HandleTypeDef     *hhid = (USBD_HID_HandleTypeDef *)pdev->pClassData;

  if (pdev->dev_state == USBD_STATE_CONFIGURED)
  {
    if (hhid->state == HID_IDLE)
    {
      hhid->state = HID_BUSY;
      USBD_LL_Transmit(pdev,
                       HID_EPIN_ADDR,
                       report,
                       len);
    }
  }
  return USBD_OK;
}

/**
  * @brief  USBD_HID_GetPollingInterval
  *         return polling interval from endpoint descriptor
  * @param  pdev: device instance
  * @retval polling interval
  */
uint32_t USBD_HID_GetPollingInterval(USBD_HandleTypeDef *pdev)
{
  uint32_t polling_interval = 0U;

  /* HIGH-speed endpoints */
  if (pdev->dev_speed == USBD_SPEED_HIGH)
  {
    /* Sets the data transfer polling interval for high speed transfers.
     Values between 1..16 are allowed. Values correspond to interval
     of 2 ^ (bInterval-1). This option (8 ms, corresponds to HID_HS_BINTERVAL */
    polling_interval = (((1U << (HID_HS_BINTERVAL - 1U))) / 8U);
  }
  else   /* LOW and FULL-speed endpoints */
  {
    /* Sets the data transfer polling interval for low and full
    speed transfers */
    polling_interval =  HID_FS_BINTERVAL;
  }

  return ((uint32_t)(polling_interval));
}

/**
  * @brief  USBD_HID_GetCfgFSDesc
  *         return FS configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_HID_GetFSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_HID_CfgFSDesc);
  return USBD_HID_CfgFSDesc;
}

/**
  * @brief  USBD_HID_GetCfgHSDesc
  *         return HS configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
#if 0
static uint8_t  *USBD_HID_GetHSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_HID_CfgHSDesc);
  return USBD_HID_CfgHSDesc;
}

/**
  * @brief  USBD_HID_GetOtherSpeedCfgDesc
  *         return other speed configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_HID_GetOtherSpeedCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_HID_OtherSpeedCfgDesc);
  return USBD_HID_OtherSpeedCfgDesc;
}

/**
  * @brief  USBD_HID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
#endif	
static uint8_t  USBD_HID_DataIn(USBD_HandleTypeDef *pdev,
                                uint8_t epnum)
{

  /* Ensure that the FIFO is empty before a new transfer, this condition could
  be caused by  a new transfer before the end of the previous transfer */
  ((USBD_HID_HandleTypeDef *)pdev->pClassData)->state = HID_IDLE;
  return USBD_OK;
}

#if 0
/**
* @brief  DeviceQualifierDescriptor
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
static uint8_t  *USBD_HID_GetDeviceQualifierDesc(uint16_t *length)
{
  *length = sizeof(USBD_HID_DeviceQualifierDesc);
  return USBD_HID_DeviceQualifierDesc;
}
#endif
/**
  * @}
  */


/**
  * @}
  */


/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
