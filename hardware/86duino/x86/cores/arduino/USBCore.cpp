/*
  usb.cpp - DM&P Vortex86 USB Device library
  Copyright (c) 2013 DY Hung <Dyhung@dmp.com.tw>. All right reserved.
  2014/06 Modified by Android Lin <acen@dmp.com.tw>.
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)
*/

#define __DMP_USB_LIB

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <conio.h>

#include "USBCore.h"
#include "vortex86.h"

#if defined DMP_DOS_DJGPP
    #include <go32.h>
    #include <dpmi.h>

    #include <sys/farptr.h>
    #include <pc.h>
    #include <dos.h>
    #include <time.h>
#elif defined DMP_DOS_WATCOM
    #include <i86.h>
    #include <time.h>
#elif defined DMP_DOS_BC
    #include <dos.h>
#endif

#include "io.h"
#include "err.h"
#include "irq.h"

#define USB_RECONNECT_DELAY_MS (300UL)
static USB_Device *usbdev = NULL;

#if defined DMP_DOS_DJGPP

typedef int DMA_HANDLE_t;
#define DMA_FAIL (-1)

static DMA_HANDLE_t  dma_handle;
static unsigned long dma_addr;

static DMA_HANDLE_t dma_Alloc(unsigned long size, unsigned long* phyaddr)
{
     DMA_HANDLE_t dma_handle;

	 int tmp = __dpmi_allocate_dos_memory((size+15)>>4, &dma_handle);
	 if (tmp == -1) return DMA_FAIL; else *phyaddr = (unsigned long)tmp << 4;

     return dma_handle;
}

static bool dma_Free(DMA_HANDLE_t dma_handle)
{
	if (__dpmi_free_dos_memory(dma_handle) == -1) return false; else return true;
}

#endif

// ------------------------------------------
//   USB Device Pins setting: Detect & ON/OFF
// ------------------------------------------
#define GPIO_CONFIG_ADDR  (0x0d00)
#define GPIO_BASE_ADDR    (0xf100)
static unsigned short gpio_config_addr = 0x0000;
static unsigned short usb_detect_data  = 0x0000;
static unsigned short usb_detect_dir   = 0x0000;
static unsigned short usb_on_off_data  = 0x0000;
static unsigned short usb_on_off_dir   = 0x0000;
static char usb_detect_pin = 0;
static char usb_on_off_pin = 0;

#ifdef DMP_86DUINO_MODE
static unsigned short tx_led_data  = 0x0000;
static unsigned short tx_led_dir   = 0x0000;
static unsigned short rx_led_data  = 0x0000;
static unsigned short rx_led_dir   = 0x0000;
static char tx_led_pin = 0;
static char rx_led_pin = 0;
#endif

static bool set_gpio_config_addr(unsigned short addr)
{
	if (io_Init() == false) {
		err_print((char*)"%s: Init IO lib error!!\n", __FUNCTION__);
		return false;
	}
	
	if ((gpio_config_addr = sb_Read16(0x62) & 0xfffe) == 0x0000)
	sb_Write16(0x62, sb_Read16(0x62) | (gpio_config_addr = GPIO_CONFIG_ADDR));
	sb_Write16(0x62, sb_Read16(0x62) | 0x0001);
	
	// io_outpdw(GPIO_CONFIG_ADDR + 0x00, io_inpdw(GPIO_CONFIG_ADDR + 0x00) | 0x00000004L);
	
	if (io_Close() == false) err_print((char*)"%s: Close IO lib error!!\n", __FUNCTION__);
	
	return true;
}

static void set_pin_in(char port, char pin)
{
	io_outpdw(gpio_config_addr + 0x00, io_inpdw(gpio_config_addr + 0x00) | 0x01L << port);
	
	// set data address
	if ((usb_detect_data = io_inpw(gpio_config_addr + (4 + 4*port))) == 0x0000)
	io_outpw(gpio_config_addr + (4 + 4*port), (usb_detect_data = (GPIO_BASE_ADDR + port)));
	// set direction address
	if ((usb_detect_dir  = io_inpw(gpio_config_addr + (6 + 4*port))) == 0x0000)
	io_outpw(gpio_config_addr + (6 + 4*port), (usb_detect_dir = (GPIO_BASE_ADDR + port + 10)));
	
	// set USB-Detect pin -> IN
	io_outpb(usb_detect_dir, io_inpb(usb_detect_dir) & ~(1 << pin));
	usb_detect_pin = pin;
}

static void set_pin_out(char port, char pin)
{
	io_outpdw(gpio_config_addr + 0x00, io_inpdw(gpio_config_addr + 0x00) | 0x01L << port);
	
	// set data address
	if ((usb_on_off_data = io_inpw(gpio_config_addr + (4 + 4*port))) == 0x0000)
	io_outpw(gpio_config_addr + (4 + 4*port), (usb_on_off_data = (GPIO_BASE_ADDR + port)));
	// set direction address
	if ((usb_on_off_dir  = io_inpw(gpio_config_addr + (6 + 4*port))) == 0x0000)
	io_outpw(gpio_config_addr + (6 + 4*port), (usb_on_off_dir = (GPIO_BASE_ADDR + port + 10)));
	
	// set USB-ONOFF pin -> OUT
	io_outpb(usb_on_off_dir, io_inpb(usb_on_off_dir) | (1 << pin));
	io_outpb(usb_on_off_data, io_inpb(usb_on_off_data) | (1 << pin));
	usb_on_off_pin = pin;
}

#ifdef DMP_86DUINO_MODE
static void set_tx_led(char port, char pin)
{
	io_outpdw(gpio_config_addr + 0x00, io_inpdw(gpio_config_addr + 0x00) | 0x01L << port);
	
	// set data address
	if ((tx_led_data = io_inpw(gpio_config_addr + (4 + 4*port))) == 0x0000)
	io_outpw(gpio_config_addr + (4 + 4*port), (tx_led_data = (GPIO_BASE_ADDR + port)));
	// set direction address
	if ((tx_led_dir  = io_inpw(gpio_config_addr + (6 + 4*port))) == 0x0000)
	io_outpw(gpio_config_addr + (6 + 4*port), (tx_led_dir = (GPIO_BASE_ADDR + port + 10)));
	
	// set TX LED pin -> OUT
	io_outpb(tx_led_dir, io_inpb(tx_led_dir) | (1 << pin));
	tx_led_pin = pin;
}

static void set_rx_led(char port, char pin)
{
	io_outpdw(gpio_config_addr + 0x00, io_inpdw(gpio_config_addr + 0x00) | 0x01L << port);
	
	// set data address
	if ((rx_led_data = io_inpw(gpio_config_addr + (4 + 4*port))) == 0x0000)
	io_outpw(gpio_config_addr + (4 + 4*port), (rx_led_data = (GPIO_BASE_ADDR + port)));
	// set direction address
	if ((rx_led_dir  = io_inpw(gpio_config_addr + (6 + 4*port))) == 0x0000)
	io_outpw(gpio_config_addr + (6 + 4*port), (rx_led_dir = (GPIO_BASE_ADDR + port + 10)));
	
	// set RX LED pin -> OUT
	io_outpb(rx_led_dir, io_inpb(rx_led_dir) | (1 << pin));
	rx_led_pin = pin;
}
#endif

static bool USB_IsAttached(void)
{
	return (io_inpb(usb_detect_data) & (1 << usb_detect_pin)) ? (true) : (false);
}
static void USB_Connect(void)
{
	io_outpb(usb_on_off_data, io_inpb(usb_on_off_data) & ~(1 << usb_on_off_pin));
}
static void USB_Disconnect(void)
{
	io_outpb(usb_on_off_data, io_inpb(usb_on_off_data) | (1 << usb_on_off_pin));
}
static void USB_Reconnect(void)
{
    USB_Disconnect();
    timer_Delay(USB_RECONNECT_DELAY_MS);
    USB_Connect();
}

#ifdef DMP_86DUINO_MODE
#define LED_MAX_COUNT    (100)
static int tx_led_count = 0;
static int rx_led_count = 0;

static void TX_LED_ON(void)
{
	io_outpb(tx_led_data, io_inpb(tx_led_data) & ~(1 << tx_led_pin));
	tx_led_count = LED_MAX_COUNT;
}

static void RX_LED_ON(void)
{
	io_outpb(rx_led_data, io_inpb(rx_led_data) & ~(1 << rx_led_pin));
	rx_led_count = LED_MAX_COUNT;
}

static void TX_LED_OFF(void)
{
	io_outpb(tx_led_data, io_inpb(tx_led_data) | (1 << tx_led_pin));
}

static void RX_LED_OFF(void)
{
	io_outpb(rx_led_data, io_inpb(rx_led_data) | (1 << rx_led_pin));
}
#endif

//
// Get physical memory address
DMP_INLINE(DWORD) GrabPhysicalMEM(void *memory)
{
	#if defined DMP_DOS_WATCOM
	{
		return (DWORD)memory;
	}
	#elif defined DMP_DOS_BC
	{
		return ((DWORD)FP_SEG(memory) << 4) + (DWORD)FP_OFF(memory);
	}
	#endif
}

// Extern Variable
extern Device_Descriptor desc_Device;
extern Configuration_Desc_Set desc_Config_Set;
extern String_Descriptor StringDescTable[];

#define LSB (0)
#define MSB (1)

// macro
#define RX_QUEUE_SIZE			   	(4096)
#define TX_QUEUE_SIZE				(4096)
#define NEAR_FULL_SIZE				(1024)

#define IALL        (0x0001FF1FL)
#define IBRST       (0x00000001L)
#define ISOF        (0x00000002L)
#define ISUSP       (0x00000004L)
#define IRESM       (0x00000008L)
#define SYSERR      (0x00000010L)
#define IEP0SETUP   (0x00000100L)
#define IEP0RX      (0x00000200L)
#define IEP0TX      (0x00000400L)
#define IEP1RX      (0x00000800L)
#define IEP1TX      (0x00001000L)
#define IEP2RX      (0x00002000L)
#define IEP2TX      (0x00004000L)
#define IEP3RX      (0x00008000L)
#define IEP3TX      (0x00010000L)

#define EP0    (0)
#define EP1    (1)
#define EP2    (2)
#define EP3    (3)
#define IN     (0)
#define OUT    (1)
#define SETUP  (2)
#define NONE   (0x00000000L)
#define ENABLE (0x80000000L)
#define STALL  (0x40000000L)
#define STSACK (0x20000000L)
DMP_INLINE(void) SetEPnDLR(USB_Device *usb, int ep, int dir, DWORD flag)
{
	switch (dir)
	{
		case IN   : io_outpdw(usb->EP[ep].InDLR   , flag); break;
		case OUT  : io_outpdw(usb->EP[ep].OutDLR  , flag); break;
		case SETUP: io_outpdw(usb->EP[ep].SetupDLR, flag); break;
		default: break;
	}
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(SetEPnDLR)
#endif

DMP_INLINE(void) EP0_InHandler(USB_Device *usb);
DMP_INLINE(void) EP0_OutHandler(USB_Device *usb);
DMP_INLINE(void) EP2_InHandler(USB_Device *usb);

DMP_INLINE(void) Get_Status(USB_Device *usb) 
{
	BYTE data[2] = {0, 0};
	if ((usb->Setup.bmRequestType & 0x80) != 0x80) return;
	//if (usb->Setup.wLength != 2) return;
	usb->InDataPtr = data;
	usb->InDataSize = (WORD)(sizeof(data));
	usb->setup_in_handled = true;
	if (usb->setup_in_handled == true) EP0_InHandler(usb);
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(Get_Status)
#endif

DMP_INLINE(void) Clear_Feature(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x00) return;

	SetEPnDLR(usb, EP0, IN, ENABLE | STSACK);
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(Clear_Feature)
#endif

DMP_INLINE(void) Set_Feature(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x00) return;

	SetEPnDLR(usb, EP0, IN, ENABLE | STSACK);
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(Set_Feature)
#endif
				
DMP_INLINE(void) Set_Address(USB_Device *usb)
{
	if (usb->state == USB_DEV_CONFIGURED) return;
	if ((usb->Setup.bmRequestType & 0x80) != 0x00) return;
	if (usb->Setup.wValue.Value > 127 || usb->Setup.wIndex.Value != 0 || usb->Setup.wLength != 0) return;
	
	usb->DevAddr = usb->Setup.wValue.Val[LSB];

	SetEPnDLR(usb, EP0, IN, ENABLE | STSACK);
	usb->ReadySetAddr = true;
	// while (io_inpdw(usb->EP[0].InDLR) & 0x20000000L);
	
	// io_outpb(usb->DAR, usb->DevAddr | 0x80);
	usb->state = (usb->DevAddr == 0) ? USB_DEV_DEFAULT : USB_DEV_ADDRESS;
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(Set_Address)
#endif

DMP_INLINE(void) Get_Descriptor(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x80) return;
	
	switch (usb->Setup.wValue.Val[MSB])
	{
		case 0x01:
		{
			if (usb->Setup.wValue.Val[LSB] == 0 && usb->Setup.wIndex.Value == 0)
			{
				usb->InDataPtr = (BYTE *)&desc_Device;
				usb->InDataSize = (WORD)usb->InDataPtr[0];
				usb->setup_in_handled = true;
				if (usb->InDataSize > usb->Setup.wLength) usb->InDataSize = usb->Setup.wLength;
				if (usb->IsSet == 0)
				{
					usb->InDataSize = EP0_MAX_PACKET_SIZE;
					usb->IsSet = 1;
				}
				if (usb->setup_in_handled == true) EP0_InHandler(usb);
			}
		}
		break;
			
		case 0x02:
		{
			if (usb->Setup.wValue.Val[LSB] == 0 && usb->Setup.wIndex.Value == 0)
			{
				usb->InDataPtr = (BYTE *)&desc_Config_Set;
				usb->InDataSize = ((WORD)usb->InDataPtr[3] << 8) + (WORD)usb->InDataPtr[2];
				usb->setup_in_handled = true;
				if (usb->InDataSize > usb->Setup.wLength) usb->InDataSize = usb->Setup.wLength;
				if (usb->setup_in_handled == true) EP0_InHandler(usb);
			}
		}
		break;
		
		case 0x03:
		{
			if (usb->Setup.wValue.Val[LSB] <= 6) // string index 0 -> 6
			{
				usb->InDataPtr = (BYTE *)&StringDescTable[usb->Setup.wValue.Val[LSB]];
				usb->InDataSize = usb->InDataPtr[0];
				usb->setup_in_handled = true;
				if (usb->InDataSize > usb->Setup.wLength) usb->InDataSize = usb->Setup.wLength;
				if (usb->setup_in_handled == true) EP0_InHandler(usb);
			}
			else SetEPnDLR(usb, EP0, IN, ENABLE | STALL);
		}
		break;
		
		case 0x21: 
		{
			// HID
		}
		break;
		
		case 0x22: 
		{
			if (usb->Setup.wValue.Val[LSB] == 0 && usb->Setup.wIndex.Value == 2)
			{
				usb->InDataPtr = (BYTE *)hidReportDescriptor;
				usb->InDataSize = desc_Config_Set.desc_HID.bDescriptorLength;
				usb->setup_in_handled = true;
				if (usb->InDataSize > usb->Setup.wLength) usb->InDataSize = usb->Setup.wLength;
				if (usb->setup_in_handled == true) EP0_InHandler(usb);
			}
		}
		break;
		
		case 0x23: 
		{
			// Physical descriptor
		}
		break;
		
		default: 
		{
			SetEPnDLR(usb, EP0, IN, ENABLE | STALL);
		}
		break;
	}
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(Get_Descriptor)
#endif

DMP_INLINE(void) Set_Descriptor(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x00) return;
	
	switch (usb->Setup.wValue.Val[MSB])
	{
		case 0x01:
		{
			if (usb->Setup.wValue.Val[LSB] == 0 && usb->Setup.wIndex.Value == 0)
			{
				usb->OutDataPtr = (BYTE *)&desc_Device;
				usb->OutDataSize = usb->Setup.wLength;
				usb->setup_out_handled = true;
				SetEPnDLR(usb, EP0, OUT, ENABLE | EP0_MAX_PACKET_SIZE);
			}
		}
		break;
			
		case 0x02:
		{
			if (usb->Setup.wValue.Val[LSB] == 0 && usb->Setup.wIndex.Value == 0)
			{
				usb->OutDataPtr = (BYTE *)&desc_Config_Set;
				usb->OutDataSize = usb->Setup.wLength;
				usb->setup_out_handled = true;
				SetEPnDLR(usb, EP0, OUT, ENABLE | EP0_MAX_PACKET_SIZE);
			}
		}
		break;
		
		case 0x03:
		{
			if (usb->Setup.wValue.Val[LSB] <= 6) // string index 0 -> 6
			{
				usb->OutDataPtr = (BYTE *)&StringDescTable[usb->Setup.wValue.Val[LSB]];
				usb->OutDataSize = usb->Setup.wLength;
				usb->setup_out_handled = true;
				SetEPnDLR(usb, EP0, OUT, ENABLE | EP0_MAX_PACKET_SIZE);
			}
			else SetEPnDLR(usb, EP0, OUT, ENABLE | STALL);
		}
		break;
		
		default: 
		{
			SetEPnDLR(usb, EP0, OUT, ENABLE | STALL);
		}
		break;
	}
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(Set_Descriptor)
#endif

DMP_INLINE(void) Get_Configuration(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x80) return;
	if (usb->Setup.wValue.Value != 0 || usb->Setup.wIndex.Value != 0 || usb->Setup.wLength != 1) return;
	
	usb->InDataPtr  = &desc_Config_Set.desc_CDC_config.bConfigurationValue;
	usb->InDataSize = 1;
	usb->setup_in_handled = true;
	
	if (usb->setup_in_handled == true) EP0_InHandler(usb);
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(Get_Configuration)
#endif

DMP_INLINE(void) Set_Configuration(USB_Device *usb)
{
	if (usb->state == USB_DEV_DEFAULT) return;
	if ((usb->Setup.bmRequestType & 0x80) != 0x00) return;
	if (usb->Setup.wValue.Val[MSB] != 0 || usb->Setup.wIndex.Value != 0 || usb->Setup.wLength != 0) return;
	
	switch (usb->state)
	{
		case USB_DEV_ADDRESS:
		{
			if (usb->Setup.wValue.Val[LSB] != 0)
			{
				if (usb->Setup.wValue.Val[LSB] == 1)
				{
					usb->state = USB_DEV_CONFIGURED;
					SetEPnDLR(usb, EP2, OUT, ENABLE | EP2_MAX_PACKET_SIZE_OUT);
					SetEPnDLR(usb, EP0, IN, ENABLE | STSACK);
				}
				else 
					SetEPnDLR(usb, EP0, IN, ENABLE | STALL);
			}
			else
				SetEPnDLR(usb, EP0, IN, ENABLE | STSACK);
		}
		break;
		
		case USB_DEV_CONFIGURED:
		{
			if (usb->Setup.wValue.Val[LSB] == 0)
			{
				usb->state = USB_DEV_ADDRESS;
				SetEPnDLR(usb, EP2, OUT, NONE);
				SetEPnDLR(usb, EP0, IN, ENABLE | STSACK);
			}
			else if (usb->Setup.wValue.Val[LSB] != 1)
				SetEPnDLR(usb, EP0, IN, ENABLE | STALL);
			else
				SetEPnDLR(usb, EP0, IN, ENABLE | STSACK);
		}
		break;
		
		default: break;
	};
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(Set_Configuration)
#endif

DMP_INLINE(void) Get_Interface(USB_Device *usb)
{
	BYTE tmp = 0;
	if ((usb->Setup.bmRequestType & 0x80) != 0x80) return;
	if (usb->Setup.wValue.Value != 0 || usb->Setup.wLength != 1) return;
	
	switch (usb->Setup.wIndex.Value)
	{
		case 0: case 1: case 2:
		{
			if (usb->state == USB_DEV_ADDRESS)
			{
				SetEPnDLR(usb, EP0, IN, ENABLE | STALL);
			}
			else if (usb->state == USB_DEV_CONFIGURED)
			{
				usb->InDataPtr  = &tmp;
				usb->InDataSize = 1;
				usb->setup_in_handled = true;
				if (usb->setup_in_handled == true) EP0_InHandler(usb);
			}
		}
		break;
		
		default:
		{
			SetEPnDLR(usb, EP0, IN, ENABLE | STALL);
		}
		break;
	};
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(Get_Interface)
#endif

DMP_INLINE(void) Set_Interface(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x00) return;
	if (usb->Setup.wLength != 0) return;
	
	switch (usb->Setup.wIndex.Value)
	{
		case 0: case 1: case 2:
		{   
			if (usb->state == USB_DEV_ADDRESS)
			{
				SetEPnDLR(usb, EP0, IN, ENABLE | STALL);
			}
			else if (usb->state == USB_DEV_CONFIGURED)
			{
				SetEPnDLR(usb, EP0, IN, ENABLE | STSACK);
			}
		}
		break;
		
		default:
		{
			SetEPnDLR(usb, EP0, IN, ENABLE | STALL);
		}
		break;
	};
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(Set_Interface)
#endif

DMP_INLINE(void) Synch_Frame(USB_Device *usb)
{
	WORD FrameNumber;
	
	if ((usb->Setup.bmRequestType & 0x80) != 0x80) return;
	if (usb->Setup.wValue.Value != 0 || usb->Setup.wLength != 2) return;
	
	switch (usb->state)
	{
		case USB_DEV_ADDRESS:
		{
			SetEPnDLR(usb, EP0, IN, ENABLE | STSACK);
		}
		break;
		
		case USB_DEV_CONFIGURED:
		{
			FrameNumber = io_inpw(usb->FNR) & 0x0FFF;
			usb->InDataPtr  = (BYTE *)&FrameNumber;
			usb->InDataSize = 2;
			usb->setup_in_handled = true;
			if (usb->setup_in_handled == true) EP0_InHandler(usb);
		}
		break;
		
		default: break;
	};
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(Synch_Frame)
#endif

DMP_INLINE(void) USB_Standard_Request(USB_Device *usb)
{
	switch (usb->Setup.bRequest)
	{
		case 0x00: Get_Status(usb);        break;
		case 0x01: Clear_Feature(usb);     break; 
		// case 0x02: /* Reserved */          break;
		case 0x03: Set_Feature(usb);       break; 
		// case 0x04: /* Reserved */          break;
		case 0x05: Set_Address(usb);       break;
		case 0x06: Get_Descriptor(usb);    break;
		case 0x07: Set_Descriptor(usb);    break;
		case 0x08: Get_Configuration(usb); break;
		case 0x09: Set_Configuration(usb); break;
		case 0x0A: Get_Interface(usb);     break;
		case 0x0B: Set_Interface(usb);     break;
		case 0x0C: Synch_Frame(usb);       break;
		default: break;
	};
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(USB_Standard_Request)
#endif

DMP_INLINE(void) Set_Line_Coding(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x00) return;
	
	usb->OutDataPtr  = (BYTE *)&usb->ling_coding;
	usb->OutDataSize = usb->Setup.wLength;
	usb->setup_out_handled = true;
	
	SetEPnDLR(usb, EP0, OUT, ENABLE | EP0_MAX_PACKET_SIZE);
	//usb->state = USB_DEV_CDC_CONNECT;
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(Set_Line_Coding)
#endif

DMP_INLINE(void) Get_Line_Coding(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x80) return;
	
	usb->InDataPtr = (BYTE *)&usb->ling_coding;
	usb->InDataSize = usb->Setup.wLength;
	usb->setup_in_handled = true;
	
	if (usb->setup_in_handled == true) EP0_InHandler(usb);
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(Get_Line_Coding)
#endif

DMP_INLINE(void) Set_Control_Line_State(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x00) return;
	
	usb->control_line_state = usb->Setup.wValue.Value;
	
	if (usb->control_line_state == 0)
		usb->state = USB_DEV_CONFIGURED;
	else
		usb->state = USB_DEV_CDC_CONNECT;
	//printf("%d %d\n", usb->control_line_state, usb->state);	
	SetEPnDLR(usb, EP0, IN, ENABLE | STSACK);
	
#ifdef DMP_86DUINO_MODE	
	// the below behavier is only for compatible of Arduino Leonado (windows)
	if(usb->ling_coding.dwDTERate == 1200 && (usb->control_line_state & 0x01) == 0)
	{   
		io_DisableINT();
		io_outpb(usb_on_off_data, io_inpb(usb_on_off_data) | (1 << usb_on_off_pin));
		io_outpb(0xf21A, 0x5a); // write soft reset key
		io_outpb(0x64, 0xfe); // reboot
	}
#endif
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(Set_Control_Line_State)
#endif

DMP_INLINE(void) Send_Break(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x00) return;
	
	usb->break_time = usb->Setup.wValue.Value;
	SetEPnDLR(usb, EP0, IN, ENABLE | STSACK);
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(Send_Break)
#endif

DMP_INLINE(void) USB_CDC_Request(USB_Device *usb)
{
	//printf("CDC: %d\n", usb->Setup.bRequest);
	switch (usb->Setup.bRequest)
	{
		// For USB CDC ACM
		// case 0x00: Send_Encapsulated_Command(usb); break;
		// case 0x01: Get_Encapsulated_Command(usb);  break;
		// case 0x02: Set_Comm_Feature(usb);          break;
		// case 0x03: Get_Comm_Feature(usb);          break;
		// case 0x04: Clear_Comm_Feature(usb);        break;
		case 0x20: Set_Line_Coding(usb);           break;
		case 0x21: Get_Line_Coding(usb);           break;
		case 0x22: Set_Control_Line_State(usb);    break;
		case 0x23: Send_Break(usb);                break;
		default: break;
	};
}

#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(USB_CDC_Request)
#endif


DMP_INLINE(void) Set_Idle(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x00) return;
	
	SetEPnDLR(usb, EP0, IN, ENABLE | STSACK);
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(Set_Idle)
#endif


DMP_INLINE(void) Get_Report(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x80) return;
	
	usb->InDataPtr = (BYTE *)&usb->ling_coding;
	usb->InDataSize = 0; // zero package
	usb->setup_in_handled = true;
	
	if (usb->setup_in_handled == true) EP0_InHandler(usb);
	//SetEPnDLR(usb, EP0, IN, ENABLE | STSACK);
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(Get_Report)
#endif


DMP_INLINE(void) Get_Protocol(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x80) return;
	
	usb->InDataPtr = (BYTE *)&usb->ling_coding;
	usb->InDataSize = 0; // zero package
	usb->setup_in_handled = true;
	
	if (usb->setup_in_handled == true) EP0_InHandler(usb);
	//SetEPnDLR(usb, EP0, IN, ENABLE | STSACK);
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(Get_Protocol)
#endif


DMP_INLINE(void) Set_Protocol(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x00) return;
	
	SetEPnDLR(usb, EP0, IN, ENABLE | STSACK);
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(Set_Protocol)
#endif


DMP_INLINE(void) USB_HID_Request(USB_Device *usb)
{   
	switch (usb->Setup.bRequest)
	{
		case 0x01: Get_Report(usb);             break;
		//case 0x02: Get_Idle(usb);               break;
		case 0x03: Get_Protocol(usb);           break;
		//case 0x09: Set_Report(usb);             break;
		case 0x0A: Set_Idle(usb);               break;
		case 0x0B: Set_Protocol(usb);           break;
		default: break;
	};
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(USB_HID_Request)
#endif

DMP_INLINE(void) EP0_SetupHandler(USB_Device *usb)
{
	#if defined DMP_DOS_DJGPP
	dosmemget(usb->EP[0].SetupPhysical, EP0_MAX_PACKET_SIZE, usb->EP[0].SetupBuf);
	#endif
	usb->Setup.bmRequestType = usb->EP[0].SetupBuf[0];
	usb->Setup.bRequest      = usb->EP[0].SetupBuf[1];
	usb->Setup.wValue.Value  = ((WORD)usb->EP[0].SetupBuf[3] << 8) + (WORD)usb->EP[0].SetupBuf[2];
	usb->Setup.wIndex.Value  = ((WORD)usb->EP[0].SetupBuf[5] << 8) + (WORD)usb->EP[0].SetupBuf[4];
	usb->Setup.wLength       = ((WORD)usb->EP[0].SetupBuf[7] << 8) + (WORD)usb->EP[0].SetupBuf[6];

	if (usb->stall == true)
	{
		io_outpdw(usb->EP[0].CtrlTR, io_inpdw(usb->EP[0].CtrlTR) & 0xDFFF);
		io_outpdw(usb->EP[0].CtrlTR, io_inpdw(usb->EP[0].CtrlTR) | 0x2000);
		usb->stall = false;
	}
	
	switch (usb->Setup.bmRequestType)
	{
		case 0x00: case 0x01: case 0x02:
		case 0x80: case 0x81: case 0x82:
			USB_Standard_Request(usb); break;
		
		case 0x21: case 0xA1:
			if(usb->Setup.wIndex.Value == 0)
			{
				USB_CDC_Request(usb);
			}
			else if(usb->Setup.wIndex.Value == 2)
			{
				USB_HID_Request(usb);
			}
			break;	
		default: break;
	};
	
	SetEPnDLR(usb, EP0, SETUP, ENABLE);
	/*
	printf("%02X %02X %02X %02X %02X %02X %02X %02X\n", usb->EP[0].SetupBuf[0],
		   usb->EP[0].SetupBuf[1], usb->EP[0].SetupBuf[2], usb->EP[0].SetupBuf[3],
		   usb->EP[0].SetupBuf[4], usb->EP[0].SetupBuf[5], usb->EP[0].SetupBuf[6],
		   usb->EP[0].SetupBuf[7]);
	*/
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(EP0_SetupHandler)
#endif

DMP_INLINE(void) EP0_InHandler(USB_Device *usb)
{
	int len;
	static int j = 0;
	
	if (usb->setup_in_handled == false) return;
#ifdef DMP_86DUINO_MODE
	TX_LED_ON();
#endif
	if (j < usb->InDataSize)
	{
		for (len = 0; len < EP0_MAX_PACKET_SIZE && j < usb->InDataSize; len++, j++)
		{
			usb->EP[0].InBuf[len] = *usb->InDataPtr++;
		}
		#if defined DMP_DOS_DJGPP
		dosmemput(usb->EP[0].InBuf, EP0_MAX_PACKET_SIZE, usb->EP[0].InPhysical);
		#endif
		SetEPnDLR(usb, EP0, IN, ENABLE | len);
	}
	else
	{
		if (j < usb->Setup.wLength && (j % EP0_MAX_PACKET_SIZE) == 0)
		{
			SetEPnDLR(usb, EP0, IN, ENABLE);
		}
		j = 0;
		usb->InDataSize = 0;
		usb->setup_in_handled = false;
		SetEPnDLR(usb, EP0, OUT, ENABLE | STSACK);
	}
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(EP0_InHandler)
#endif

DMP_INLINE(void) EP0_OutHandler(USB_Device *usb)
{
	int i;
	WORD size;
	static int j = 0;
	
	if (usb->setup_out_handled == false) return; 
#ifdef DMP_86DUINO_MODE
	RX_LED_ON();
#endif
	size = (WORD)(io_inpdw(usb->EP[0].OutDLR) & 0x00001FFFFL);

	if (j < usb->OutDataSize)
	{
		#if defined DMP_DOS_DJGPP
		dosmemget(usb->EP[0].OutPhysical, EP0_MAX_PACKET_SIZE, usb->EP[0].OutBuf);
		#endif
		for (i = 0; i < size && j < usb->OutDataSize; i++, j++)
		{
			*usb->OutDataPtr++ = usb->EP[0].OutBuf[i];
		}
	}
	
	if (j < usb->OutDataSize)
	{
		SetEPnDLR(usb, EP0, OUT, ENABLE | EP0_MAX_PACKET_SIZE);
	}
	else
	{
		j = 0;
		usb->OutDataSize = 0;
		usb->setup_out_handled = false;
		SetEPnDLR(usb, EP0, IN, ENABLE | STSACK);
	}
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(EP0_OutHandler)
#endif

DMP_INLINE(void) EP1_InHandler(USB_Device *usb)
{
	static BYTE notification[8] = {0xA1, 0x20, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00};
	static bool Completed = true;
#ifdef DMP_86DUINO_MODE
	TX_LED_ON();
#endif	
	if (Completed == true)
	{
		usb->EP[1].InBuf = notification;
		#if defined DMP_DOS_DJGPP
		dosmemput(usb->EP[1].InBuf, EP1_MAX_PACKET_SIZE_IN, usb->EP[1].InPhysical);
		#endif
		SetEPnDLR(usb, EP1, IN, ENABLE | 0x08);
		Completed = false;
	}
	else
	{
		usb->EP[1].InBuf[0] = (BYTE)(usb->serial_state & 0x00FF);
		usb->EP[1].InBuf[1] = (BYTE)((usb->serial_state & 0xFF00) >> 8);
		#if defined DMP_DOS_DJGPP
		dosmemput(usb->EP[1].InBuf, EP1_MAX_PACKET_SIZE_IN, usb->EP[1].InPhysical);
		#endif
		SetEPnDLR(usb, EP1, IN, ENABLE | 0x02);
		Completed = true;
	}
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(EP1_InHandler)
#endif

DMP_INLINE(void) EP2_InHandler(USB_Device *usb)
{
	static int ep2_in_len = 0;
	
	if (usb->bulk_in_transmitting == true) return;
	if (QueueEmpty(usb->xmit)) {
		if (ep2_in_len == EP2_MAX_PACKET_SIZE_IN) {
			usb->bulk_in_transmitting = true;
			SetEPnDLR(usb, EP2, IN, ENABLE);
			ep2_in_len = 0;
		}
		return;
	}
	
#ifdef DMP_86DUINO_MODE
	TX_LED_ON();
#endif	
	
	ep2_in_len = 0;
	do {
		usb->EP[2].InBuf[ep2_in_len++] =(BYTE)PopQueue(usb->xmit);
	} while(QueueEmpty(usb->xmit) == false && ep2_in_len < EP2_MAX_PACKET_SIZE_IN);
	
	#if defined DMP_DOS_DJGPP
	dosmemput(usb->EP[2].InBuf, EP2_MAX_PACKET_SIZE_IN, usb->EP[2].InPhysical);
	#endif
	
	usb->bulk_in_transmitting = true;
	SetEPnDLR(usb, EP2, IN, ENABLE | ep2_in_len);
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(EP2_InHandler)
#endif

DMP_INLINE(void) EP2_OutHandler(USB_Device *usb)
{
	int i;
	WORD size;
	
#ifdef DMP_86DUINO_MODE
	RX_LED_ON();
#endif	
	size = (WORD)(io_inpdw(usb->EP[2].OutDLR) & 0x00001FFFFL);
	#if defined DMP_DOS_DJGPP
	dosmemget(usb->EP[2].OutPhysical, EP2_MAX_PACKET_SIZE_OUT, usb->EP[2].OutBuf);
	#endif
	for (i = 0; i < size; i++) PushQueue(usb->rcvd, usb->EP[2].OutBuf[i]);
	
	if (QueueSize(usb->rcvd) < (RX_QUEUE_SIZE - NEAR_FULL_SIZE))
	SetEPnDLR(usb, EP2, OUT, ENABLE | EP2_MAX_PACKET_SIZE_OUT);
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(EP2_OutHandler)
#endif


DMP_INLINE(void) EP3_InHandler(USB_Device *usb)
{
	static int ep3_in_len = 0;
	
	if (usb->interrupt_in_transmitting == true) return;
	if (QueueEmpty(usb->hidxmit)) {
		if (ep3_in_len == EP3_MAX_PACKET_SIZE_IN) {
			SetEPnDLR(usb, EP3, IN, ENABLE);
			ep3_in_len = 0;
		}
		return;
	}
	
#ifdef DMP_86DUINO_MODE
	TX_LED_ON();
#endif	
	
	ep3_in_len = 0;
	do {
		usb->EP[3].InBuf[ep3_in_len++] =(BYTE)PopQueue(usb->hidxmit);
	} while(QueueEmpty(usb->hidxmit) == false && ep3_in_len < EP3_MAX_PACKET_SIZE_IN);
	
	#if defined DMP_DOS_DJGPP
	dosmemput(usb->EP[3].InBuf, EP3_MAX_PACKET_SIZE_IN, usb->EP[3].InPhysical);
	#endif
	
	SetEPnDLR(usb, EP3, IN, ENABLE | ep3_in_len);
	usb->interrupt_in_transmitting = true;
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(EP3_InHandler)
#endif

DMP_INLINE(bool) usb_Reset(USB_Device *usb)
{
	//if (usb->InUse == 0) return false;
	
    io_outpb(usb->CFR, io_inpb(usb->CFR) & 0xFE);
	io_outpb(usb->CFR, 0x02); // Soft reset
	while (io_inpb(usb->CFR) & 0x02);
	usb->DevAddr = 0x00;
	usb->ReadySetAddr = false;
	//io_outpb(usb->DAR, 0x80); // enable USB device
	
	// io_outpdw(usb->EP[0].SetupDSR, usb->EP[0].SetupPhysical);
	// io_outpdw(usb->EP[0].InDSR   , usb->EP[0].InPhysical);
	// io_outpdw(usb->EP[0].OutDSR  , usb->EP[0].OutPhysical);
	// io_outpdw(usb->EP[1].InDSR   , usb->EP[1].InPhysical);
	// io_outpdw(usb->EP[2].InDSR   , usb->EP[2].InPhysical);
	// io_outpdw(usb->EP[2].OutDSR  , usb->EP[2].OutPhysical);
	
	// io_outpw(usb->EP[0].CtrlTR   , 0x2000 | EP0_MAX_PACKET_SIZE);
	// io_outpw(usb->EP[1].InTR     , 0x3800 | EP1_MAX_PACKET_SIZE_IN);
	// io_outpw(usb->EP[2].InTR     , 0x3000 | EP2_MAX_PACKET_SIZE_IN);
	// io_outpw(usb->EP[2].OutTR    , 0x3000 | EP2_MAX_PACKET_SIZE_OUT);
	
	//SetEPnDLR(usb, EP0, SETUP, ENABLE);
	//SetEPnDLR(usb, EP0, IN, 0L);
	//SetEPnDLR(usb, EP0, OUT, 0L);
	//SetEPnDLR(usb, EP1, IN, 0L);
	//SetEPnDLR(usb, EP2, OUT, 0L);
	//SetEPnDLR(usb, EP2, IN, 0L);
	//SetEPnDLR(usb, EP3, IN, 0L);
	
	// io_outpb(usb->DAR, 0x80); // enable USB device
	// while (!(io_inpb(usb->DAR) & 0x80));
	
	ClearQueue(usb->rcvd);
	ClearQueue(usb->xmit);
	ClearQueue(usb->hidxmit); 
	
    io_outpdw(usb->EP[0].SetupDSR, usb->EP[0].SetupPhysical);
	io_outpdw(usb->EP[0].InDSR   , usb->EP[0].InPhysical);
	io_outpdw(usb->EP[0].OutDSR  , usb->EP[0].OutPhysical);
	io_outpdw(usb->EP[1].InDSR   , usb->EP[1].InPhysical);
	io_outpdw(usb->EP[2].InDSR   , usb->EP[2].InPhysical);
	io_outpdw(usb->EP[2].OutDSR  , usb->EP[2].OutPhysical);
	io_outpdw(usb->EP[3].InDSR   , usb->EP[3].InPhysical);

	io_outpw(usb->EP[0].CtrlTR   , 0x2000 | EP0_MAX_PACKET_SIZE);
	io_outpw(usb->EP[1].InTR     , 0x3800 | EP1_MAX_PACKET_SIZE_IN);
	io_outpw(usb->EP[2].InTR     , 0x3000 | EP2_MAX_PACKET_SIZE_IN);
	io_outpw(usb->EP[2].OutTR    , 0x3000 | EP2_MAX_PACKET_SIZE_OUT);
	io_outpw(usb->EP[3].InTR     , 0x3800 | EP3_MAX_PACKET_SIZE_IN);

	SetEPnDLR(usb, EP0, SETUP, ENABLE);

	io_outpb(usb->DAR, 0x80); // enable USB device
	while (!(io_inpb(usb->DAR) & 0x80));

	io_outpdw(usb->ISR, 0xFFFFFFFFL);

	io_DisableINT();
	{
		io_outpdw(usb->IER, ISOF + IBRST + ISUSP + IRESM + SYSERR +
		                    IEP0SETUP + IEP0RX + IEP0TX + IEP1TX +
							IEP2RX + IEP2TX + IEP3RX + IEP3TX);
		io_outpb(usb->CFR, io_inpb(usb->CFR) | 0x01);
	}
	io_RestoreINT();
    usb->state = USB_DEV_POWERED;

	// io_DisableINT();
	// {
		// io_outpb(usb->CFR, io_inpb(usb->CFR) & 0xFE);
		// io_outpdw(usb->IER, ISOF + IBRST + ISUSP + IRESM + SYSERR + 
		                    // IEP0SETUP + IEP0RX + IEP0TX + IEP1TX + IEP2RX + IEP2TX);
		// io_outpb(usb->CFR, io_inpb(usb->CFR) | 0x01);
	// }
	// io_RestoreINT();

	return true;
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(usb_Reset)
#endif

static int USB_ISR(int irq, void* data)
{
	DWORD isr;
	USB_Device *usb;
	
	usb = (USB_Device *)data;

	if ((isr = io_inpdw(usb->ISR)) != 0x00L)
	{
		// Bus reset interrupt
		if (isr & IBRST)
		{
			io_outpdw(usb->ISR, IALL);
			usb_Reset(usb);
		}
		
		// SOF interrupt
		if (isr & ISOF)
		{
			io_outpdw(usb->ISR, ISOF);
			if (usb->ReadySetAddr == true)
			{
				io_outpb(usb->DAR, usb->DevAddr | 0x80);
				usb->ReadySetAddr = false;
			}
#ifdef DMP_86DUINO_MODE
			if(tx_led_count && !(--tx_led_count)) TX_LED_OFF();
			if(rx_led_count && !(--rx_led_count)) RX_LED_OFF();
#endif
		}
		
		// Suspend interrupt
		if (isr & ISUSP)
		{
			io_outpdw(usb->ISR, ISUSP);
			usb->IsSet = 0;
			usb->state = USB_DEV_POWERED;
#ifdef DMP_86DUINO_MODE
			TX_LED_OFF();
			RX_LED_OFF();
#endif
		}
		
		// Resume interrupt
		if (isr & IRESM)
		{
			io_outpdw(usb->ISR, IRESM);
		}
		
		// System error interrupt
		if (isr & SYSERR)
		{
			io_outpdw(usb->ISR, SYSERR);
		}
		
		// EP0 Setup interrupt
		if (isr & IEP0SETUP)
		{
			io_outpdw(usb->ISR, IEP0SETUP);
			EP0_SetupHandler(usb);
		}
		
		// EP0 RX interrupt
		if (isr & IEP0RX)
		{
			io_outpdw(usb->ISR, IEP0RX);
			EP0_OutHandler(usb);
		}
		
		// EP0 TX interrupt
		if (isr & IEP0TX)
		{
			io_outpdw(usb->ISR, IEP0TX);
			EP0_InHandler(usb);
		}
		
		// EP1 RX interrupt
		if (isr & IEP1RX)
		{
			io_outpdw(usb->ISR, IEP1RX);
		}
		
		// EP1 TX interrupt
		if (isr & IEP1TX)
		{
			io_outpdw(usb->ISR, IEP1TX);
			EP1_InHandler(usb);
		}
		
		// EP2 RX interrupt
		if (isr & IEP2RX)
		{
			io_outpdw(usb->ISR, IEP2RX);
			EP2_OutHandler(usb);
		}
		
		// EP2 TX interrupt
		if (isr & IEP2TX)
		{
			io_outpdw(usb->ISR, IEP2TX);
			usb->bulk_in_transmitting = false;
			EP2_InHandler(usb);
		}
		
		// EP3 RX interrupt
		if (isr & IEP3RX)
		{
			io_outpdw(usb->ISR, IEP3RX);
		}
		
		// EP3 TX interrupt
		if (isr & IEP3TX)
		{
			io_outpdw(usb->ISR, IEP3TX);
			usb->interrupt_in_transmitting = false;
			EP3_InHandler(usb); 
		}
	}
	else
		return ISR_NONE;


	return ISR_HANDLED;
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(USB_ISR)
#endif

DMPAPI(void *) CreateUSBDevice(void)
{
	USB_Device *usb = NULL;
	
	if ((usb = (USB_Device *)ker_Malloc(sizeof(USB_Device))) == NULL) return NULL;
	
	if (io_Init() == false) {
		err_print((char*)"%s: Init IO lib error!!\n", __FUNCTION__);
		ker_Mfree((void *)usb);
		return NULL;
	}

	usb->addr = vx86_GetUSBDevAddr();
	usb->nIRQ = vx86_GetUSBDevIRQ();
	if (usb->addr == 0x0000 || usb->nIRQ == 0)
	{ 
		io_Close();
		ker_Mfree((void *)usb);
		return NULL;
	}
	
	usb->DevAddr                   = 0x00;
	usb->ReadySetAddr              = false;
	
	usb->state					   = USB_DEV_NOT_ATTACHED;
	usb->stall					   = false;
	
	usb->InUse                     = 0;
	usb->IsSet                     = 0;
	usb->setup_in_handled          = false;
	usb->setup_out_handled         = false;
	usb->bulk_in_transmitting 	   = false;
	usb->interrupt_in_transmitting = false;
	
	usb->RxTimeOut		           = USB_NO_TIMEOUT;
	usb->TxTimeOut		           = USB_NO_TIMEOUT;
	
	usb->InDataPtr                 = NULL;
	usb->OutDataPtr                = NULL;
	usb->InDataSize                = 0;
	usb->OutDataSize               = 0;

	if ((usb->rcvd    = CreateQueue(RX_QUEUE_SIZE)) == NULL) goto CREATE_RX_QUEUE_FAIL;
	if ((usb->xmit    = CreateQueue(TX_QUEUE_SIZE)) == NULL) goto CREATE_TX_QUEUE_FAIL;
	if ((usb->hidxmit = CreateQueue(TX_QUEUE_SIZE)) == NULL) goto CREATE_HIDTX_QUEUE_FAIL;
	
	usb->Setup.bmRequestType     = 0;
	usb->Setup.bRequest          = 0;
	usb->Setup.wValue.Value      = 0;
	usb->Setup.wIndex.Value      = 0;
	usb->Setup.wLength           = 0;
	
	usb->ling_coding.dwDTERate   = 0;
	usb->ling_coding.bCharFormat = 0;
	usb->ling_coding.bParityType = 0;
	usb->ling_coding.bDataBits   = 0;
	
	usb->control_line_state      = 0;
	usb->serial_state            = 0;
	
	usb->DAR = usb->addr + 0x00;
	usb->CFR = usb->addr + 0x02;
	usb->FNR = usb->addr + 0x06;
	usb->IER = usb->addr + 0x08;
	usb->ISR = usb->addr + 0x0C;
	usb->TMR = usb->addr + 0x68;
	
	memset((Endpoint *)usb->EP, 0, sizeof(usb->EP));
	usb->EP[0].CtrlTR   = usb->addr + 0x10;
	usb->EP[1].OutTR    = usb->addr + 0x12;
	usb->EP[1].InTR     = usb->addr + 0x14;
	usb->EP[2].OutTR    = usb->addr + 0x16;
	usb->EP[2].InTR     = usb->addr + 0x18;
	usb->EP[3].OutTR    = usb->addr + 0x1A;
	usb->EP[3].InTR     = usb->addr + 0x1C;
	usb->EP[0].SetupDLR = usb->addr + 0x20;
	usb->EP[0].OutDLR   = usb->addr + 0x24;
	usb->EP[0].InDLR    = usb->addr + 0x28;
	usb->EP[1].OutDLR   = usb->addr + 0x2C;
	usb->EP[1].InDLR    = usb->addr + 0x30;
	usb->EP[2].OutDLR   = usb->addr + 0x34;
	usb->EP[2].InDLR    = usb->addr + 0x38;
	usb->EP[3].OutDLR   = usb->addr + 0x3C;
	usb->EP[3].InDLR    = usb->addr + 0x40;
	usb->EP[0].SetupDSR = usb->addr + 0x44;
	usb->EP[0].OutDSR   = usb->addr + 0x48;
	usb->EP[0].InDSR    = usb->addr + 0x4C;
	usb->EP[1].OutDSR   = usb->addr + 0x50;
	usb->EP[1].InDSR    = usb->addr + 0x54;
	usb->EP[2].OutDSR   = usb->addr + 0x58;
	usb->EP[2].InDSR    = usb->addr + 0x5C;
	usb->EP[3].OutDSR   = usb->addr + 0x60;
	usb->EP[3].InDSR    = usb->addr + 0x64;	

#ifdef DMP_86DUINO_MODE
	set_gpio_config_addr(GPIO_CONFIG_ADDR); // for 86duino
	set_tx_led(7, 2);
	set_rx_led(7, 3);
	TX_LED_OFF();
	RX_LED_OFF();
#endif
	return (void *)usb;

CREATE_HIDTX_QUEUE_FAIL:
	DestoryQueue(usb->xmit);	
CREATE_TX_QUEUE_FAIL:
	DestoryQueue(usb->rcvd);
CREATE_RX_QUEUE_FAIL:
	io_Close();
	ker_Mfree((void *)usb);
	return NULL;
}

DMPAPI(bool) usb_Init(void *vusb)
{
	#if defined DMP_DOS_DJGPP
	static bool locked = false;
	int size_temp;
	#endif
	USB_Device *usb = (USB_Device *)vusb;
	
    if (usb == NULL) return false;
    if (usbdev && usb != usbdev) return false;
	if (usb->InUse == 1) return true;
	
	// if (USB_IsAttached() == false) return false;
	
	if (irq_Init() == false) {
        err_print((char*)"%s: IRQ init fail.\n", __FUNCTION__);
		io_Close();
        return false;
    }
	
	#if defined DMP_DOS_DJGPP
	if (locked == false)
	{
		int i, str_size;
		DPMI_LOCK_FUNC(SetEPnDLR);
		DPMI_LOCK_FUNC(Get_Status); 
		DPMI_LOCK_FUNC(Clear_Feature);  
		DPMI_LOCK_FUNC(Set_Feature); 
		DPMI_LOCK_FUNC(Set_Address);
		DPMI_LOCK_FUNC(Get_Descriptor);
		DPMI_LOCK_FUNC(Set_Descriptor);
		DPMI_LOCK_FUNC(Get_Configuration);
		DPMI_LOCK_FUNC(Set_Configuration);
		DPMI_LOCK_FUNC(Get_Interface);
		DPMI_LOCK_FUNC(Set_Interface);
		DPMI_LOCK_FUNC(Synch_Frame);
		DPMI_LOCK_FUNC(USB_Standard_Request);
		DPMI_LOCK_FUNC(Set_Line_Coding);
		DPMI_LOCK_FUNC(Get_Line_Coding);
		DPMI_LOCK_FUNC(Set_Control_Line_State);
		DPMI_LOCK_FUNC(Send_Break);
		DPMI_LOCK_FUNC(USB_CDC_Request);
		DPMI_LOCK_FUNC(Set_Idle); 
		DPMI_LOCK_FUNC(Get_Report); 
		DPMI_LOCK_FUNC(Get_Protocol); 
		DPMI_LOCK_FUNC(Set_Protocol); 
		DPMI_LOCK_FUNC(USB_HID_Request); 
		DPMI_LOCK_FUNC(EP0_SetupHandler);
		DPMI_LOCK_FUNC(EP0_InHandler);
		DPMI_LOCK_FUNC(EP0_OutHandler);
		DPMI_LOCK_FUNC(EP1_InHandler);
		DPMI_LOCK_FUNC(EP2_InHandler);
		DPMI_LOCK_FUNC(EP2_OutHandler);
		DPMI_LOCK_FUNC(EP3_InHandler);
		DPMI_LOCK_FUNC(usb_Reset);
		DPMI_LOCK_FUNC(USB_ISR);
		DPMI_LOCK_VAR(desc_Device);
		DPMI_LOCK_VAR(desc_Config_Set);
		DPMI_LOCK_VAR(StringDescTable[0]);
		DPMI_LOCK_VAR(StringDescTable[1]);
		DPMI_LOCK_VAR(StringDescTable[2]);
		DPMI_LOCK_VAR(StringDescTable[3]);
		DPMI_LOCK_VAR(StringDescTable[4]);
		DPMI_LOCK_VAR(StringDescTable[5]);
		DPMI_LOCK_VAR(StringDescTable[6]);
		DPMI_LOCK_VAR(hidReportDescriptor); 
		locked = true;
	}
	#endif

	io_outpb(usb->CFR, 0x02); // Soft reset
	while (io_inpb(usb->CFR) & 0x02);
	
	if ((usb->EP[0].SetupBuf = (BYTE *)ker_Malloc(sizeof(BYTE)*EP0_MAX_PACKET_SIZE)) == NULL) 	  goto EP0_SETUP_FAIL;
	if ((usb->EP[0].InBuf    = (BYTE *)ker_Malloc(sizeof(BYTE)*EP0_MAX_PACKET_SIZE)) == NULL) 	  goto EP0_IN_FAIL;
	if ((usb->EP[0].OutBuf   = (BYTE *)ker_Malloc(sizeof(BYTE)*EP0_MAX_PACKET_SIZE)) == NULL) 	  goto EP0_OUT_FAIL;
	if ((usb->EP[1].InBuf    = (BYTE *)ker_Malloc(sizeof(BYTE)*EP1_MAX_PACKET_SIZE_IN)) == NULL)  goto EP1_IN_FAIL;
	if ((usb->EP[2].InBuf    = (BYTE *)ker_Malloc(sizeof(BYTE)*EP2_MAX_PACKET_SIZE_IN)) == NULL)  goto EP2_IN_FAIL;
	if ((usb->EP[2].OutBuf   = (BYTE *)ker_Malloc(sizeof(BYTE)*EP2_MAX_PACKET_SIZE_OUT)) == NULL) goto EP2_OUT_FAIL;
	if ((usb->EP[3].InBuf    = (BYTE *)ker_Malloc(sizeof(BYTE)*EP3_MAX_PACKET_SIZE_IN)) == NULL)  goto EP3_IN_FAIL; 
	
	#if defined DMP_DOS_DJGPP
	if ((dma_handle = dma_Alloc(EP0_MAX_PACKET_SIZE    +
	                            EP0_MAX_PACKET_SIZE    +
						        EP0_MAX_PACKET_SIZE    +
						        EP1_MAX_PACKET_SIZE_IN +
						        EP2_MAX_PACKET_SIZE_IN +
						        EP2_MAX_PACKET_SIZE_OUT+
								EP3_MAX_PACKET_SIZE_IN, &dma_addr)) == DMA_FAIL) goto EP2_OUT_FAIL;  
	size_temp = 0;
	usb->EP[0].SetupPhysical = dma_addr;
	usb->EP[0].InPhysical    = dma_addr + (size_temp += EP0_MAX_PACKET_SIZE);
	usb->EP[0].OutPhysical   = dma_addr + (size_temp += EP0_MAX_PACKET_SIZE);
	usb->EP[1].InPhysical    = dma_addr + (size_temp += EP1_MAX_PACKET_SIZE_IN);
	usb->EP[2].InPhysical    = dma_addr + (size_temp += EP2_MAX_PACKET_SIZE_IN);
	usb->EP[2].OutPhysical   = dma_addr + (size_temp += EP2_MAX_PACKET_SIZE_OUT);
	usb->EP[3].InPhysical    = dma_addr + (size_temp += EP3_MAX_PACKET_SIZE_IN);
	
	#else
	usb->EP[0].SetupPhysical = GrabPhysicalMEM((void *)usb->EP[0].SetupBuf);
	usb->EP[0].InPhysical    = GrabPhysicalMEM((void *)usb->EP[0].InBuf);
	usb->EP[0].OutPhysical   = GrabPhysicalMEM((void *)usb->EP[0].OutBuf);
	usb->EP[1].InPhysical    = GrabPhysicalMEM((void *)usb->EP[1].InBuf);
	usb->EP[2].InPhysical    = GrabPhysicalMEM((void *)usb->EP[2].InBuf);
	usb->EP[2].OutPhysical   = GrabPhysicalMEM((void *)usb->EP[2].OutBuf);
	usb->EP[3].InPhysical    = GrabPhysicalMEM((void *)usb->EP[3].InBuf); 
	#endif
	
	// usb->DevAddr = 0x00;
	// usb->ReadySetAddr = false;
	// io_outpb(usb->DAR, 0x00); // enable USB device

	/*
	io_outpdw(usb->EP[0].SetupDSR, usb->EP[0].SetupPhysical);
	io_outpdw(usb->EP[0].InDSR   , usb->EP[0].InPhysical);
	io_outpdw(usb->EP[0].OutDSR  , usb->EP[0].OutPhysical);
	io_outpdw(usb->EP[1].InDSR   , usb->EP[1].InPhysical);
	io_outpdw(usb->EP[2].InDSR   , usb->EP[2].InPhysical);
	io_outpdw(usb->EP[2].OutDSR  , usb->EP[2].OutPhysical);
	io_outpdw(usb->EP[3].InDSR   , usb->EP[3].InPhysical); 
	
	io_outpw(usb->EP[0].CtrlTR   , 0x2000 | EP0_MAX_PACKET_SIZE);
	io_outpw(usb->EP[1].InTR     , 0x3800 | EP1_MAX_PACKET_SIZE_IN);
	io_outpw(usb->EP[2].InTR     , 0x3000 | EP2_MAX_PACKET_SIZE_IN);
	io_outpw(usb->EP[2].OutTR    , 0x3000 | EP2_MAX_PACKET_SIZE_OUT);
	io_outpw(usb->EP[3].InTR     , 0x3800 | EP3_MAX_PACKET_SIZE_IN); 

	SetEPnDLR(usb, EP0, SETUP, ENABLE);
	*/

	// io_outpb(usb->DAR, 0x80); // enable USB device
	// while (!(io_inpb(usb->DAR) & 0x80));
	
	// ClearQueue(usb->rcvd);
	// ClearQueue(usb->xmit);
	
	// io_DisableINT();
	// {
		// io_outpb(usb->CFR, io_inpb(usb->CFR) & 0xFE);
		// io_outpdw(usb->IER, ISOF + IBRST + ISUSP + IRESM + SYSERR + 
		                    // IEP0SETUP + IEP0RX + IEP0TX + IEP1TX + IEP2RX + IEP2TX);
		// io_outpb(usb->CFR, io_inpb(usb->CFR) | 0x01);
	// }
	// io_RestoreINT();
	
	// usb->state = USB_DEV_DEFAULT;

	/*
	io_outpb(usb->DAR, 0x80); // enable USB device
	while (!(io_inpb(usb->DAR) & 0x80));
	
	io_outpdw(usb->ISR, 0xFFFFFFFFL);

	io_DisableINT();
	{
		io_outpb(usb->CFR, io_inpb(usb->CFR) & 0xFE);

		io_outpdw(usb->IER, ISOF + IBRST + ISUSP + IRESM + SYSERR + 
		                    IEP0SETUP + IEP0RX + IEP0TX + IEP1TX + 
							IEP2RX + IEP2TX + IEP3RX + IEP3TX);
		io_outpb(usb->CFR, io_inpb(usb->CFR) | 0x01);
	}
	io_RestoreINT();
	*/

    irq_Setting(usb->nIRQ, IRQ_LEVEL_TRIGGER);
	irq_InstallISR(usb->nIRQ, USB_ISR, (void *)usb);
    usb_Reset(usb);
    USB_Reconnect();
	usb->InUse = 1;
    usbdev = usb;

	return true;

EP3_IN_FAIL:
	ker_Mfree(usb->EP[2].OutBuf);  	
EP2_OUT_FAIL:
	ker_Mfree(usb->EP[2].InBuf);
EP2_IN_FAIL:
	ker_Mfree(usb->EP[1].InBuf);
EP1_IN_FAIL:	
	ker_Mfree(usb->EP[0].OutBuf);
EP0_OUT_FAIL:
	ker_Mfree(usb->EP[0].InBuf);
EP0_IN_FAIL:
	ker_Mfree(usb->EP[0].SetupBuf);
EP0_SETUP_FAIL:
	err_print((char*)"%s: Alloc endpoint buffers error!!\n", __FUNCTION__);
	irq_Close();
	io_Close();
	return false;
}

DMPAPI(void) usb_Close(void *vusb)
{
	USB_Device *usb = (USB_Device *)vusb;
	if (usb == NULL) { err_print((char*)"%s: USB device is null.\n", __FUNCTION__); return; }
	
	if (usb->InUse != 0)
	{
		io_DisableINT();
		{
			io_outpb(usb->CFR, io_inpb(usb->CFR) & 0xFE);
			irq_UninstallISR(usb->nIRQ, (void *)usb);
		}
		io_RestoreINT();
		irq_Close();
		
		io_outpb(usb->CFR, 0x02); // Soft reset
		while (io_inpb(usb->CFR) & 0x02);
		
		#if defined DMP_DOS_DJGPP
		if (dma_Free(dma_handle) == false) err_print((char*)"%s: Free DMA buffer fail!!\n", __FUNCTION__);
		#endif
		ker_Mfree(usb->EP[0].SetupBuf);
		ker_Mfree(usb->EP[0].InBuf);
		ker_Mfree(usb->EP[0].OutBuf);
		ker_Mfree(usb->EP[1].InBuf);
		ker_Mfree(usb->EP[2].InBuf);
		ker_Mfree(usb->EP[2].OutBuf);
		ker_Mfree(usb->EP[3].InBuf);
		
		if (io_Close() == false) err_print((char*)"%s: Close IO lib error!!\n", __FUNCTION__);
		
		usb->state  = USB_DEV_POWERED;
		usb->InUse = 0;
		
		USB_Disconnect();
	}
	DestoryQueue(usb->rcvd);
	DestoryQueue(usb->xmit);
	DestoryQueue(usb->hidxmit);
	ker_Mfree((void *)usb);
}

DMPAPI(int) usb_State(void *vusb)
{
	int state;
	USB_Device *usb = (USB_Device *)vusb;
	if (usb == NULL) { err_print((char*)"%s: USB device is null.\n", __FUNCTION__); return 0; }
	io_DisableINT();
	state = usb->state;
	io_RestoreINT();
	return state;
}

DMPAPI(bool) usb_Ready(void *vusb)
{
	if (vusb == NULL) { err_print((char*)"%s: USB device is null.\n", __FUNCTION__); return false; }
	return (usb_State(vusb) == USB_DEV_CDC_CONNECT) ? (true) : (false);
}

DMPAPI(void) usb_SetSerialState(void *vusb, WORD serial)
{
	USB_Device *usb = (USB_Device *)vusb;
	io_DisableINT();
	usb->serial_state = serial;
	EP1_InHandler(usb);
	io_RestoreINT();
}

DMPAPI(void) usb_SetTimeOut(void *vusb, DWORD rx_timeout, DWORD tx_timeout)
{
	USB_Device *usb = (USB_Device *)vusb;
	if (usb == NULL) { err_print((char*)"%s: USB device is null.\n", __FUNCTION__); return; }
	usb->RxTimeOut = rx_timeout;
	usb->TxTimeOut = tx_timeout;
}

DMPAPI(Line_Coding *) usb_GetLineCoding(void *vusb)
{
	USB_Device *usb = (USB_Device *)vusb;
	if (usb == NULL) { err_print((char*)"%s: USB device is null.\n", __FUNCTION__); return NULL; }
	
	return (Line_Coding*)&(usb->ling_coding);
}

DMPAPI(int) usb_QueryRxQueue(void *vusb)
{
	USB_Device *usb = (USB_Device *)vusb;
	if (usb == NULL) { err_print((char*)"%s: USB device is null.\n", __FUNCTION__); return 0; }
	return QueueSize(usb->rcvd);
}

DMPAPI(bool) usb_RxQueueFull(void *vusb)
{
	USB_Device *usb = (USB_Device *)vusb;
	if (usb == NULL) { err_print((char*)"%s: USB device is null.\n", __FUNCTION__); return false; }
	return QueueFull(usb->rcvd);
}

DMPAPI(bool) usb_RxQueueEmpty(void *vusb)
{
	USB_Device *usb = (USB_Device *)vusb;
	if (usb == NULL) { err_print((char*)"%s: USB device is null.\n", __FUNCTION__); return false; }
	return QueueEmpty(usb->rcvd);
}
                     
DMPAPI(unsigned int) usb_Read(void *vusb)
{
	unsigned char val;

	if (usb_Receive(vusb, &val, 1) <= 0)
		return 0xffff;
		
	return val;
}

DMPAPI(int) usb_Receive(void *vusb, BYTE* buf, int bsize)
{
	int i;
	unsigned long pretime;
	USB_Device *usb = (USB_Device *)vusb;
	
	if (usb == NULL) { err_print((char*)"%s: USB device is null.\n", __FUNCTION__); return 0; }
	
	for (i = 0; i < bsize; i++) {
		if (usb->RxTimeOut != USB_NO_TIMEOUT) {
			pretime = timer_NowTime();
			while (QueueEmpty(usb->rcvd) && (timer_NowTime() - pretime) < usb->RxTimeOut); 
			
			if (QueueEmpty(usb->rcvd)) {
				if (USB_TIMEOUT_DEBUG)
					err_print((char*)"%s: USB device receive timeout.\n", __FUNCTION__);
				break;
			}
		}
		else while (QueueEmpty(usb->rcvd));

		io_DisableINT();
		{
			buf[i] = (BYTE)PopQueue(usb->rcvd);
			if (QueueSize(usb->rcvd) < (RX_QUEUE_SIZE - NEAR_FULL_SIZE) && !(io_inpdw(usb->EP[2].OutDLR) & 0x80000000L)) 
			SetEPnDLR(usb, EP2, OUT, ENABLE | EP2_MAX_PACKET_SIZE_OUT);
		}
		io_RestoreINT();
	}

    return i;	
}

DMPAPI(void) usb_FlushRxQueue(void *vusb)
{
	USB_Device *usb = (USB_Device *)vusb;
	if (usb == NULL) { err_print((char*)"%s: USB device is null.\n", __FUNCTION__); return; }
	ClearQueue(usb->rcvd);
}

DMPAPI(int) usb_QueryTxQueue(void *vusb)
{
	USB_Device *usb = (USB_Device *)vusb;
	if (usb == NULL) { err_print((char*)"%s: USB device is null.\n", __FUNCTION__); return 0; }
	return QueueSize(usb->xmit);
}

DMPAPI(bool) usb_TxQueueFull(void *vusb)
{
	USB_Device *usb = (USB_Device *)vusb;
	if (usb == NULL) { err_print((char*)"%s: USB device is null.\n", __FUNCTION__); return false; }
	return QueueFull(usb->xmit);
}

DMPAPI(bool) usb_TxQueueEmpty(void *vusb)
{
	USB_Device *usb = (USB_Device *)vusb;
	if (usb == NULL) { err_print((char*)"%s: USB device is null.\n", __FUNCTION__); return false; }
	return QueueEmpty(usb->xmit);
}

DMPAPI(bool) usb_TxReady(void *vusb)
{
	USB_Device *usb = (USB_Device *)vusb;
	if (usb == NULL) { err_print((char*)"%s: USB device is null.\n", __FUNCTION__); return false; }
	return (!QueueFull(usb->xmit)) ? (true) : (false);
}

DMPAPI(int) usb_Send(void *vusb, BYTE *buf, int bsize)
{
	int i;
	DWORD pretime;
	USB_Device *usb = (USB_Device *)vusb;
	
	if (usb == NULL) { err_print((char*)"%s: USB device is null.\n", __FUNCTION__); return 0; }
	
	if (usb->state != USB_DEV_CDC_CONNECT)
		return 0;
	
	for (i = 0; i < bsize; i++)
	{
		if (usb->TxTimeOut != USB_NO_TIMEOUT) {
			pretime = timer_NowTime();
			while (QueueFull(usb->xmit) && (timer_NowTime() - pretime) < usb->TxTimeOut); 
			
			if (QueueFull(usb->xmit)) {
				if (USB_TIMEOUT_DEBUG)
					err_print((char*)"%s: USB device transmit timeout.\n", __FUNCTION__);
				io_DisableINT();
				EP2_InHandler(usb);
				io_RestoreINT();
				return i;
			}
		}
		else while (QueueFull(usb->xmit));
		
		io_DisableINT();
		{
			PushQueue(usb->xmit, buf[i]);
			
			if (i == (bsize - 1) || QueueFull(usb->xmit))
			{
				EP2_InHandler(usb);
			}
		}
		io_RestoreINT();
	}
	
	return i;
}


DMPAPI(int) hid_Send(void *vusb, BYTE *buf, int bsize)
{
	int i;
	DWORD pretime;
	USB_Device *usb = (USB_Device *)vusb;

	if (usb == NULL) { err_print((char*)"%s: USB device is null.\n", __FUNCTION__); return 0; }
	
	if (usb->state != USB_DEV_CONFIGURED && usb->state != USB_DEV_CDC_CONNECT)
		return 0;

	if (usb->TxTimeOut != USB_NO_TIMEOUT) {
		pretime = timer_NowTime();
		while (QueueEmpty(usb->hidxmit) == false && (timer_NowTime() - pretime) < usb->TxTimeOut); 
		
		if (QueueEmpty(usb->hidxmit) == false) {
			if (USB_TIMEOUT_DEBUG)
				err_print((char*)"%s: USB device transmit timeout.\n", __FUNCTION__);
			return 0;
		}
	}
	else while(QueueEmpty(usb->hidxmit) == false);
	
	io_DisableINT();
	for (i = 0; i < bsize; i++)
		PushQueue(usb->hidxmit, buf[i]);
	EP3_InHandler(usb);
	io_RestoreINT();
	
	return i;
}

DMPAPI(bool) usb_Write(void *vusb, BYTE val)
{
	if (usb_Send(vusb, &val, 1) != 1) return false;
	return true;
}

DMPAPI(void) usb_FlushTxQueue(void *vusb)
{
	USB_Device *usb = (USB_Device *)vusb;
	if (usb == NULL) { err_print((char*)"%s: USB device is null.\n", __FUNCTION__); return; }
	
	ClearQueue(usb->xmit);
}

DMPAPI(void) usb_FlushWFIFO(void *vusb)
{
	USB_Device *usb = (USB_Device *)vusb;
	if (usb == NULL) { err_print((char*)"%s: USB device is null.\n", __FUNCTION__); return; }
	while (!QueueEmpty(usb->xmit));
	while (io_inpdw(usb->EP[2].InDLR) & 0x80000000L);
}

DMPAPI(WORD) usb_GetControlLineState(void *vusb)
{
	USB_Device *usb = (USB_Device *)vusb;
	if (usb == NULL) { err_print((char*)"%s: USB device is null.\n", __FUNCTION__); return 0xffff; }
	return usb->control_line_state;
}

DMPAPI(bool) usb_SetUSBPins(void *vusb, char port1, char pin1, char port2, char pin2)
{
	USB_Device *usb = (USB_Device *)vusb;
	if (usb == NULL) { err_print((char*)"%s: USB device is null.\n", __FUNCTION__); return false; }
	
	if (set_gpio_config_addr(GPIO_CONFIG_ADDR) == false)
	{
		err_print((char*)"%s: init USB-DEV DETECT and ONOFF pins fail.\n", __FUNCTION__);
		return false;
	}
	set_pin_in(port1, pin1);
	set_pin_out(port2, pin2);
	
	//USB_Connect();
	return true;
}
