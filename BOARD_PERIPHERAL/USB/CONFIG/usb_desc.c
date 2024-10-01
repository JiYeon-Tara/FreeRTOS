/**
  ******************************************************************************
  * @file    usb_desc.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Descriptors for Joystick Mouse Demo
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_desc.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/* USB Standard Device Descriptor */
const uint8_t Joystick_DeviceDescriptor[JOYSTICK_SIZ_DEVICE_DESC] =
  {
    0x12,                       /*bLength */
    USB_DEVICE_DESCRIPTOR_TYPE, /*bDescriptorType*/
    0x00,                       /*bcdUSB */
    0x02,
    0x00,                       /*bDeviceClass*/
    0x00,                       /*bDeviceSubClass*/
    0x00,                       /*bDeviceProtocol*/
    0x40,                       /*bMaxPacketSize 64*/
    0x83,                       /*idVendor (0x0483)*/
    0x04,
    0x10,                       /*idProduct = 0x5710, ALL LITTLE ENDIAN*/
    0x57,
    0x00,                       /*bcdDevice rel. 2.00*/
    0x02,
    1,                          /*Index of string descriptor describing
                                                  manufacturer */
    2,                          /*Index of string descriptor describing
                                                 product*/
    3,                          /*Index of string descriptor describing the
                                                 device serial number */
    0x01                        /*bNumConfigurations*/
  }
  ; /* Joystick_DeviceDescriptor */


/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
const uint8_t Joystick_ConfigDescriptor[JOYSTICK_SIZ_CONFIG_DESC] =
  {
    0x09, /* bLength: Configuration Descriptor size */
    USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType: Configuration */
    JOYSTICK_SIZ_CONFIG_DESC,
    /* wTotalLength: Bytes returned */
    0x00,
    0x01,         /*bNumInterfaces: 1 interface*/
    0x01,         /*bConfigurationValue: Configuration value*/
    0x00,         /*iConfiguration: Index of string descriptor describing
                                     the configuration*/
    0xE0,         /*bmAttributes: Self powered */
    0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/

    /************** Descriptor of Joystick Mouse interface ****************/
    /* array size:9 */
    0x09,         /*bLength: Interface Descriptor size*/
    USB_INTERFACE_DESCRIPTOR_TYPE,/*bDescriptorType: Interface descriptor type*/
    0x00,         /*bInterfaceNumber: Number of Interface*/
    0x00,         /*bAlternateSetting: Alternate setting*/
    0x01,         /*bNumEndpoints*/
    0x03,         /*bInterfaceClass: HID*/
    0x01,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
    0x02,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
    0,            /*iInterface: Index of string descriptor*/
    /******************** Descriptor of Joystick Mouse HID ********************/
    /* array size:18 */
    0x09,         /*bLength: HID Descriptor size*/
    HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
    0x00,         /*bcdHID: HID Class Spec release number*/
    0x01,
    0x00,         /*bCountryCode: Hardware target country*/
    0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
    0x22,         /*bDescriptorType*/
    JOYSTICK_SIZ_REPORT_DESC,/*wItemLength: Total length of Report descriptor*/
    0x00,
    /******************** Descriptor of Joystick Mouse endpoint ********************/
    /* array size:27 */
    0x07,          /*bLength: Endpoint Descriptor size*/
    USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType:*/

    0x81,          /*bEndpointAddress: Endpoint Address (IN)*/
    0x03,          /*bmAttributes: Interrupt endpoint*/
    0x04,          /*wMaxPacketSize: 4 Byte max */
    0x00,
    0x20,          /*bInterval: Polling Interval (32 ms)*/
    /*array size: 34 */
  }; /* MOUSE_ConfigDescriptor */

const uint8_t Joystick_ReportDescriptor[JOYSTICK_SIZ_REPORT_DESC] =
  {
    0x05, 0x01,         /*Usage Page(Generic Desktop)*/
    0x09, 0x02,         /*Usage(Mouse)*/
    0xA1, 0x01,         /*主条目:开集合, 应用集合Collection(Application)*/
    
    0x09, 0x01,         /*Usage(Pointer)*/
    0xA1, 0x00,         /*主条目:开集合 Collection(Physical)*/
    0x05, 0x09,         /*Usage Page(Buttons)*/
    0x19, 0x01,         /*Usage Minimum(1)*/
    0x29, 0x03,         /*Usage Maximum(3)*/
    0x15, 0x00,         /*Logical Minimum(0)*/
    0x25, 0x01,         /*Logical Maximum(1)*/
    0x95, 0x03,         /*Report Count(3)*/
    0x75, 0x01,         /*Report Size(1)*/
    0x81, 0x02,         /*主条目:输入Input(Variable), */
    
    0x95, 0x01,         /*Report Count(1)*/
    0x75, 0x05,         /*Report Size(5)*/
    0x81, 0x01,         /*主条目:输入 Input(Constant,Array), 为了与上面的三个 bit 合在一起构成一个 byte*/
    
    0x05, 0x01,         /*Usage Page(Generic Desktop)*/
    0x09, 0x30,         /*Usage(X axis)*/
    0x09, 0x31,         /*Usage(Y axis)*/
    0x09, 0x38,         /*Usage(Wheel)*/
    0x15, 0x81,         /*Logical Minimum(-127)*/
    0x25, 0x7F,         /*Logical Maximum(127)*/
    0x75, 0x08,         /*Report Size(8)*/
    0x95, 0x03,         /*Report Count(3)*/
    0x81, 0x06,         /*主条目:输入 Input(Variable, Relative)*/
    
    0xC0,               /*End Collection*/
  
    /* TODO: */
    0x09, 0x3c,
    0x05, 0xff,
    0x09, 0x01,
    0x15, 0x00,
    0x25, 0x01,
    0x75, 0x01,
    0x95, 0x02,
    0xb1, 0x22,
    0x75, 0x06,
    0x95, 0x01,
    0xb1, 0x01,
    /* 72 */
    0xc0                /*End Collection*/
  }; /* Joystick_ReportDescriptor */

/* USB String Descriptors (optional) */
const uint8_t Joystick_StringLangID[JOYSTICK_SIZ_STRING_LANGID] =
  {
    JOYSTICK_SIZ_STRING_LANGID,
    USB_STRING_DESCRIPTOR_TYPE,
    0x09, /* LangID = 0x0409: U.S. English */
    0x04
  }; 

const u8 Joystick_StringVendor[JOYSTICK_SIZ_STRING_VENDOR] =	    
{
	JOYSTICK_SIZ_STRING_VENDOR, /* Size of manufaturer string */
	0x03,           /* bDescriptorType = String descriptor */
	/* Manufacturer: "广州星翼电子科技有限公司" */
	0X7F,0X5E,//广
	0XDE,0X5D,//州
	0X1F,0X66,//星
	0XFC,0X7F,//翼
	0X35,0X75,//电
	0X50,0X5B,//子
	0XD1,0X79,//科
	0X62,0X80,//技
	0X09,0X67,//有
	0X50,0X96,//限
	0X6C,0X51,//公
	0XF8,0X53,//司	  
};

//使用UNICODE编码
const u8 Joystick_StringProduct[JOYSTICK_SIZ_STRING_PRODUCT] =
{
	JOYSTICK_SIZ_STRING_PRODUCT,          /* bLength */
	USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
	'A', 0, 'L', 0, 'I', 0, 'E', 0, 'N', 0, 'T', 0, 'E', 0, 'K', 0,//ALIENTEK
	' ', 0, 
	0XE6,0X89,//触
	0XA7,0X63,//控
	'U', 0,
	'S', 0, 
	'B', 0,
	0X20,0X9F,//鼠
	0X07,0X68,//标 
};
uint8_t Joystick_StringSerial[JOYSTICK_SIZ_STRING_SERIAL] =
  {
    JOYSTICK_SIZ_STRING_SERIAL,           /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
    'S', 0, 'T', 0, 'M', 0, '3', 0, '2', 0
  };

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

