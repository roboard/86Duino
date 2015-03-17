/*
  usb.cpp - DM&P Vortex86 USB Device library
  Copyright (c) 2013 DY Hung <Dyhung@dmp.com.tw>. All right reserved.

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
////////////////////////////////////////////////////////////////////////////////
//    note that most of functions in this lib assume no paging issue when 
//    using them in ISR; so to use this lib in ISR in DJGPP, it is suggested 
//    to employ PMODE/DJ or HDPMI instead of CWSDPMI.
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <conio.h>

#include "usb.h"
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
#define GPIO_CONFIG_ADDR  (0xf100)
#define GPIO_BASE_ADDR    (0xf200)
static unsigned short gpio_config_addr = 0x0000;
static unsigned short usb_detect_data  = 0x0000;
static unsigned short usb_detect_dir   = 0x0000;
static unsigned short usb_on_off_data  = 0x0000;
static unsigned short usb_on_off_dir   = 0x0000;
static char usb_detect_pin = 0;
static char usb_on_off_pin = 0;

static bool set_gpio_config_addr(unsigned short addr)
{
	if (io_Init() == false) {
		err_print((char*)"%s: Init IO lib error!!\n", __FUNCTION__);
		return false;
	}
	
	if ((gpio_config_addr = sb_Read16(0x62) & 0xfffe) == 0x0000) {
		sb_Write16(0x62, sb_Read16(0x62) | (gpio_config_addr = GPIO_CONFIG_ADDR) | 0x0001);
	}
	
	io_Close();
	
	return true;
}

static void set_pin_in(char port, char pin)
{
	io_outpdw(gpio_config_addr + 0x00, io_inpdw(gpio_config_addr + 0x00) | 0x01L << port);
	
	// set data address
	if ((usb_detect_data = io_inpw(gpio_config_addr + (4 + 4*port))) == 0x0000)
		io_outpw(gpio_config_addr + (4 + 4*port), (usb_detect_data = (GPIO_BASE_ADDR + port*4)));
	// set direction address
	if ((usb_detect_dir  = io_inpw(gpio_config_addr + (6 + 4*port))) == 0x0000)
		io_outpw(gpio_config_addr + (6 + 4*port), (usb_detect_dir = (GPIO_BASE_ADDR + port*4 + 2)));
	
	// set USB-Detect pin -> IN
	io_outpb(usb_detect_dir, io_inpb(usb_detect_dir) & ~(1 << pin));
	usb_detect_pin = pin;
}

static void set_pin_out(char port, char pin)
{
	io_outpdw(gpio_config_addr + 0x00, io_inpdw(gpio_config_addr + 0x00) | 0x01L << port);
	
	// set data address
	if ((usb_on_off_data = io_inpw(gpio_config_addr + (4 + 4*port))) == 0x0000)
	io_outpw(gpio_config_addr + (4 + 4*port), (usb_on_off_data = (GPIO_BASE_ADDR + port*4)));
	// set direction address
	if ((usb_on_off_dir  = io_inpw(gpio_config_addr + (6 + 4*port))) == 0x0000)
	io_outpw(gpio_config_addr + (6 + 4*port), (usb_on_off_dir = (GPIO_BASE_ADDR + port*4 + 2)));
	
	// set USB-ONOFF pin -> OUT
	io_outpb(usb_on_off_dir, io_inpb(usb_on_off_dir) | (1 << pin));
	io_outpb(usb_on_off_data, io_inpb(usb_on_off_data) | (1 << pin));
	usb_on_off_pin = pin;
}

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

//
// Get physical memory address
DMP_INLINE(unsigned long) GrabPhysicalMEM(void *memory)
{
	#if defined DMP_DOS_WATCOM
	{
		return (unsigned long)memory;
	}
	#elif defined DMP_DOS_BC
	{
		return ((unsigned long)FP_SEG(memory) << 4) + (unsigned long)FP_OFF(memory);
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
DMP_INLINE(void) SetEPnDLR(USB_Device *usb, int ep, int dir, unsigned long flag)
{
	switch (dir)
	{
		case IN   : io_outpdw(usb->EP[ep].InDLR   , flag); break;
		case OUT  : io_outpdw(usb->EP[ep].OutDLR  , flag); break;
		case SETUP: io_outpdw(usb->EP[ep].SetupDLR, flag); break;
		default: break;
	}
}

DMP_INLINE(void) EP0_InHandler(USB_Device *usb);
DMP_INLINE(void) EP0_OutHandler(USB_Device *usb);
DMP_INLINE(void) EP2_InHandler(USB_Device *usb);

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

DMP_INLINE(void) Get_Descriptor(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x80) return;
	
	switch (usb->Setup.wValue.Val[MSB])
	{
		case 0x01:
		{
			if (usb->Setup.wValue.Val[LSB] == 0 && usb->Setup.wIndex.Value == 0)
			{
				usb->InDataPtr = (unsigned char *)&desc_Device;
				usb->InDataSize = (unsigned short)usb->InDataPtr[0];
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
				usb->InDataPtr = (unsigned char *)&desc_Config_Set;
				usb->InDataSize = ((unsigned short)usb->InDataPtr[3] << 8) + (unsigned short)usb->InDataPtr[2];
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
				usb->InDataPtr = (unsigned char *)&StringDescTable[usb->Setup.wValue.Val[LSB]];
				usb->InDataSize = usb->InDataPtr[0];
				usb->setup_in_handled = true;
				if (usb->InDataSize > usb->Setup.wLength) usb->InDataSize = usb->Setup.wLength;
				if (usb->setup_in_handled == true) EP0_InHandler(usb);
			}
			else SetEPnDLR(usb, EP0, IN, ENABLE | STALL);
		}
		break;
		
		default: 
		{
			SetEPnDLR(usb, EP0, IN, ENABLE | STALL);
		}
		break;
	}
}

DMP_INLINE(void) Set_Descriptor(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x00) return;
	
	switch (usb->Setup.wValue.Val[MSB])
	{
		case 0x01:
		{
			if (usb->Setup.wValue.Val[LSB] == 0 && usb->Setup.wIndex.Value == 0)
			{
				usb->OutDataPtr = (unsigned char *)&desc_Device;
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
				usb->OutDataPtr = (unsigned char *)&desc_Config_Set;
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
				usb->OutDataPtr = (unsigned char *)&StringDescTable[usb->Setup.wValue.Val[LSB]];
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

DMP_INLINE(void) Get_Configuration(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x80) return;
	if (usb->Setup.wValue.Value != 0 || usb->Setup.wIndex.Value != 0 || usb->Setup.wLength != 1) return;
	
	usb->InDataPtr  = &desc_Config_Set.desc_CDC_config.bConfigurationValue;
	usb->InDataSize = 1;
	usb->setup_in_handled = true;
	
	if (usb->setup_in_handled == true) EP0_InHandler(usb);
}

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

DMP_INLINE(void) Get_Interface(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x80) return;
	if (usb->Setup.wValue.Value != 0 || usb->Setup.wLength != 1) return;
	
	switch (usb->Setup.wIndex.Value)
	{
		case 0:
		{
			if (usb->state == USB_DEV_ADDRESS)
			{
				SetEPnDLR(usb, EP0, IN, ENABLE | STALL);
			}
			else if (usb->state == USB_DEV_CONFIGURED)
			{
				usb->InDataPtr  = &desc_Config_Set.desc_CDC_comm.bInterfaceNumber;
				usb->InDataSize = 1;
				usb->setup_in_handled = true;
				if (usb->setup_in_handled == true) EP0_InHandler(usb);
			}
		}
		break;
		
		case 1:
		{
			if (usb->state == USB_DEV_ADDRESS)
			{
				SetEPnDLR(usb, EP0, IN, ENABLE | STALL);
			}
			else if (usb->state == USB_DEV_CONFIGURED)
			{
				usb->InDataPtr  = &desc_Config_Set.desc_CDC_data.bInterfaceNumber;
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

DMP_INLINE(void) Set_Interface(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x00) return;
	if (usb->Setup.wLength != 0) return;
	
	switch (usb->Setup.wIndex.Value)
	{
		case 0:
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
		
		case 1:
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

DMP_INLINE(void) Synch_Frame(USB_Device *usb)
{
	unsigned short FrameNumber;
	
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
			usb->InDataPtr  = (unsigned char *)&FrameNumber;
			usb->InDataSize = 2;
			usb->setup_in_handled = true;
			if (usb->setup_in_handled == true) EP0_InHandler(usb);
		}
		break;
		
		default: break;
	};
}

DMP_INLINE(void) USB_Standard_Request(USB_Device *usb)
{
	switch (usb->Setup.bRequest)
	{
		// case 0x00: Get_Status(usb);        break;
		// case 0x01: Clear_Feature(usb);     break;
		// case 0x02: /* Reserved */          break;
		// case 0x03: Set_Feature(usb);       break;
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

DMP_INLINE(void) Set_Line_Coding(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x00) return;
	
	usb->OutDataPtr  = (unsigned char *)&usb->ling_coding;
	usb->OutDataSize = usb->Setup.wLength;
	usb->setup_out_handled = true;
	
	SetEPnDLR(usb, EP0, OUT, ENABLE | EP0_MAX_PACKET_SIZE);
}

DMP_INLINE(void) Get_Line_Coding(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x80) return;
	
	usb->InDataPtr = (unsigned char *)&usb->ling_coding;
	usb->InDataSize = usb->Setup.wLength;
	usb->setup_in_handled = true;
	
	if (usb->setup_in_handled == true) EP0_InHandler(usb);
}

DMP_INLINE(void) Set_Control_Line_State(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x00) return;
	
	usb->control_line_state = usb->Setup.wValue.Value;
	
	if (usb->control_line_state == 0)
		usb->state = USB_DEV_CONFIGURED;
	else 
		usb->state = USB_DEV_CDC_CONNECT;
		
	SetEPnDLR(usb, EP0, IN, ENABLE | STSACK);
}

DMP_INLINE(void) Send_Break(USB_Device *usb)
{
	if ((usb->Setup.bmRequestType & 0x80) != 0x00) return;
	
	usb->break_time = usb->Setup.wValue.Value;
	SetEPnDLR(usb, EP0, IN, ENABLE | STSACK);
}

DMP_INLINE(void) USB_CDC_Request(USB_Device *usb)
{
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

DMP_INLINE(void) EP0_SetupHandler(USB_Device *usb)
{
	#if defined DMP_DOS_DJGPP
	dosmemget(usb->EP[0].SetupPhysical, EP0_MAX_PACKET_SIZE, usb->EP[0].SetupBuf);
	#endif
	usb->Setup.bmRequestType = usb->EP[0].SetupBuf[0];
	usb->Setup.bRequest      = usb->EP[0].SetupBuf[1];
	usb->Setup.wValue.Value  = ((unsigned short)usb->EP[0].SetupBuf[3] << 8) + (unsigned short)usb->EP[0].SetupBuf[2];
	usb->Setup.wIndex.Value  = ((unsigned short)usb->EP[0].SetupBuf[5] << 8) + (unsigned short)usb->EP[0].SetupBuf[4];
	usb->Setup.wLength       = ((unsigned short)usb->EP[0].SetupBuf[7] << 8) + (unsigned short)usb->EP[0].SetupBuf[6];
	
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
			USB_CDC_Request(usb); break;
				
		default: break;
	};
	
	SetEPnDLR(usb, EP0, SETUP, ENABLE);
}

DMP_INLINE(void) EP0_InHandler(USB_Device *usb)
{
	int len;
	static int j = 0;
	
	if (usb->setup_in_handled == false) return;

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

DMP_INLINE(void) EP0_OutHandler(USB_Device *usb)
{
	int i;
	unsigned short size;
	static int j = 0;
	
	if (usb->setup_out_handled == false) return; 

	size = (unsigned short)(io_inpdw(usb->EP[0].OutDLR) & 0x00001FFFFL);

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

DMP_INLINE(void) EP1_InHandler(USB_Device *usb)
{
	static unsigned char notification[8] = {0xA1, 0x20, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00};
	static bool Completed = true;

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
		usb->EP[1].InBuf[0] = (unsigned char)(usb->serial_state & 0x00FF);
		usb->EP[1].InBuf[1] = (unsigned char)((usb->serial_state & 0xFF00) >> 8);
		#if defined DMP_DOS_DJGPP
		dosmemput(usb->EP[1].InBuf, EP1_MAX_PACKET_SIZE_IN, usb->EP[1].InPhysical);
		#endif
		SetEPnDLR(usb, EP1, IN, ENABLE | 0x02);
		Completed = true;
	}
}

DMP_INLINE(void) EP2_InHandler(USB_Device *usb)
{
	static int ep2_in_len = 0;
	
	if (usb->bulk_in_transmitting == true) return;
	if (QueueEmpty(usb->xmit)) {
		if (ep2_in_len == EP2_MAX_PACKET_SIZE_IN) {
			SetEPnDLR(usb, EP2, IN, ENABLE);
			ep2_in_len = 0;
		}
		return;
	}
	
	ep2_in_len = 0;
	do {
		usb->EP[2].InBuf[ep2_in_len++] =(unsigned char)PopQueue(usb->xmit);
	} while(QueueEmpty(usb->xmit) == false && ep2_in_len < EP2_MAX_PACKET_SIZE_IN);
	
	#if defined DMP_DOS_DJGPP
	dosmemput(usb->EP[2].InBuf, EP2_MAX_PACKET_SIZE_IN, usb->EP[2].InPhysical);
	#endif
	
	SetEPnDLR(usb, EP2, IN, ENABLE | ep2_in_len);
	usb->bulk_in_transmitting = true;
}

DMP_INLINE(void) EP2_OutHandler(USB_Device *usb)
{
	int i;
	unsigned short size;

	size = (unsigned short)(io_inpdw(usb->EP[2].OutDLR) & 0x00001FFFFL);
	#if defined DMP_DOS_DJGPP
	dosmemget(usb->EP[2].OutPhysical, EP2_MAX_PACKET_SIZE_OUT, usb->EP[2].OutBuf);
	#endif
	for (i = 0; i < size; i++) PushQueue(usb->rcvd, usb->EP[2].OutBuf[i]);
	
	if (QueueSize(usb->rcvd) < (RX_QUEUE_SIZE - NEAR_FULL_SIZE))
		SetEPnDLR(usb, EP2, OUT, ENABLE | EP2_MAX_PACKET_SIZE_OUT);
}

DMP_INLINE(bool) usb_Reset(USB_Device *usb)
{
	if (usb->InUse == 0) return false;
	
	usb->DevAddr = 0x00;
	usb->ReadySetAddr = false;
	io_outpb(usb->DAR, 0x80); // enable USB device
	
	SetEPnDLR(usb, EP0, SETUP, ENABLE);
	SetEPnDLR(usb, EP0, IN, 0L);
	SetEPnDLR(usb, EP0, OUT, 0L);
	SetEPnDLR(usb, EP1, IN, 0L);
	SetEPnDLR(usb, EP2, OUT, 0L);
	SetEPnDLR(usb, EP2, IN, 0L);
	
	ClearQueue(usb->rcvd);
	ClearQueue(usb->xmit);
	
	usb->state = USB_DEV_DEFAULT;
	
	return true;
}

static int USB_ISR(int irq, void* data)
{
	unsigned long isr;
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
		}
		
		// Suspend interrupt
		if (isr & ISUSP)
		{
			io_outpdw(usb->ISR, ISUSP);
			usb->IsSet = 0;
			usb->state = USB_DEV_POWERED;
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
		}
	}
	else
		return ISR_NONE;


	return ISR_HANDLED;
}

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
	
	usb->DevAddr                 = 0x00;
	usb->ReadySetAddr            = false;
	
	usb->state					 = USB_DEV_NOT_ATTACHED;
	usb->stall					 = false;
	
	usb->InUse                   = 0;
	usb->IsSet                   = 0;
	usb->setup_in_handled        = false;
	usb->setup_out_handled       = false;
	usb->bulk_in_transmitting 	 = false;
	
	usb->RxTimeOut		         = USB_NO_TIMEOUT;
	usb->TxTimeOut		         = USB_NO_TIMEOUT;
	
	usb->InDataPtr               = NULL;
	usb->OutDataPtr              = NULL;
	usb->InDataSize              = 0;
	usb->OutDataSize             = 0;

	if ((usb->rcvd = CreateQueue(RX_QUEUE_SIZE)) == NULL) goto CREATE_RX_QUEUE_FAIL;
	if ((usb->xmit = CreateQueue(TX_QUEUE_SIZE)) == NULL) goto CREATE_TX_QUEUE_FAIL;
	
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

	return (void *)usb;
	
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
	int size_temp;
	#endif
	USB_Device *usb = (USB_Device *)vusb;
	
	if (usb->InUse == 1) return true;
	
	if (irq_Init() == false) {
        err_print((char*)"%s: IRQ init fail.\n", __FUNCTION__);
		io_Close();
        return false;
    }

	io_outpb(usb->CFR, 0x02); // Soft reset
	while (io_inpb(usb->CFR) & 0x02);
	
	if ((usb->EP[0].SetupBuf = (unsigned char *)ker_Malloc(sizeof(unsigned char)*EP0_MAX_PACKET_SIZE)) == NULL) 	  goto EP0_SETUP_FAIL;
	if ((usb->EP[0].InBuf    = (unsigned char *)ker_Malloc(sizeof(unsigned char)*EP0_MAX_PACKET_SIZE)) == NULL) 	  goto EP0_IN_FAIL;
	if ((usb->EP[0].OutBuf   = (unsigned char *)ker_Malloc(sizeof(unsigned char)*EP0_MAX_PACKET_SIZE)) == NULL) 	  goto EP0_OUT_FAIL;
	if ((usb->EP[1].InBuf    = (unsigned char *)ker_Malloc(sizeof(unsigned char)*EP1_MAX_PACKET_SIZE_IN)) == NULL)  goto EP1_IN_FAIL;
	if ((usb->EP[2].InBuf    = (unsigned char *)ker_Malloc(sizeof(unsigned char)*EP2_MAX_PACKET_SIZE_IN)) == NULL)  goto EP2_IN_FAIL;
	if ((usb->EP[2].OutBuf   = (unsigned char *)ker_Malloc(sizeof(unsigned char)*EP2_MAX_PACKET_SIZE_OUT)) == NULL) goto EP2_OUT_FAIL;
	
	#if defined DMP_DOS_DJGPP
	if ((dma_handle = dma_Alloc(EP0_MAX_PACKET_SIZE    +
	                            EP0_MAX_PACKET_SIZE    +
						        EP0_MAX_PACKET_SIZE    +
						        EP1_MAX_PACKET_SIZE_IN +
						        EP2_MAX_PACKET_SIZE_IN +
						        EP2_MAX_PACKET_SIZE_OUT, &dma_addr)) == DMA_FAIL) goto EP2_OUT_FAIL;
	size_temp = 0;
	usb->EP[0].SetupPhysical = dma_addr;
	usb->EP[0].InPhysical    = dma_addr + (size_temp += EP0_MAX_PACKET_SIZE);
	usb->EP[0].OutPhysical   = dma_addr + (size_temp += EP0_MAX_PACKET_SIZE);
	usb->EP[1].InPhysical    = dma_addr + (size_temp += EP0_MAX_PACKET_SIZE);
	usb->EP[2].InPhysical    = dma_addr + (size_temp += EP1_MAX_PACKET_SIZE_IN);
	usb->EP[2].OutPhysical   = dma_addr + (size_temp += EP2_MAX_PACKET_SIZE_IN);
	
	#else
	usb->EP[0].SetupPhysical = GrabPhysicalMEM((void *)usb->EP[0].SetupBuf);
	usb->EP[0].InPhysical    = GrabPhysicalMEM((void *)usb->EP[0].InBuf);
	usb->EP[0].OutPhysical   = GrabPhysicalMEM((void *)usb->EP[0].OutBuf);
	usb->EP[1].InPhysical    = GrabPhysicalMEM((void *)usb->EP[1].InBuf);
	usb->EP[2].InPhysical    = GrabPhysicalMEM((void *)usb->EP[2].InBuf);
	usb->EP[2].OutPhysical   = GrabPhysicalMEM((void *)usb->EP[2].OutBuf);
	#endif
	
	
	io_outpdw(usb->EP[0].SetupDSR, usb->EP[0].SetupPhysical);
	io_outpdw(usb->EP[0].InDSR   , usb->EP[0].InPhysical);
	io_outpdw(usb->EP[0].OutDSR  , usb->EP[0].OutPhysical);
	io_outpdw(usb->EP[1].InDSR   , usb->EP[1].InPhysical);
	io_outpdw(usb->EP[2].InDSR   , usb->EP[2].InPhysical);
	io_outpdw(usb->EP[2].OutDSR  , usb->EP[2].OutPhysical);
	
	io_outpw(usb->EP[0].CtrlTR   , 0x2000 | EP0_MAX_PACKET_SIZE);
	io_outpw(usb->EP[1].InTR     , 0x3800 | EP1_MAX_PACKET_SIZE_IN);
	io_outpw(usb->EP[2].InTR     , 0x3000 | EP2_MAX_PACKET_SIZE_IN);
	io_outpw(usb->EP[2].OutTR    , 0x3000 | EP2_MAX_PACKET_SIZE_OUT);
	
	SetEPnDLR(usb, EP0, SETUP, ENABLE);
	
	io_outpb(usb->DAR, 0x80); // enable USB device
	while (!(io_inpb(usb->DAR) & 0x80));
	
	io_outpdw(usb->ISR, 0xFFFFFFFFL);

	io_DisableINT();
	{
		io_outpb(usb->CFR, io_inpb(usb->CFR) & 0xFE);
		irq_Setting(usb->nIRQ, IRQ_LEVEL_TRIGGER);
		irq_InstallISR(usb->nIRQ, USB_ISR, (void *)usb);
		io_outpdw(usb->IER, ISOF + IBRST + ISUSP + IRESM + SYSERR + 
		                    IEP0SETUP + IEP0RX + IEP0TX + IEP1TX + IEP2RX + IEP2TX);
		io_outpb(usb->CFR, io_inpb(usb->CFR) | 0x01);
	}
	io_RestoreINT();
	
	usb->state  = USB_DEV_POWERED;
	usb->InUse = 1;
	USB_Connect();
	
	return true;
	
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
		
		if (io_Close() == false) err_print((char*)"%s: Close IO lib error!!\n", __FUNCTION__);
		
		usb->state  = USB_DEV_POWERED;
		usb->InUse = 0;
		
		USB_Disconnect();
	}
	DestoryQueue(usb->rcvd);
	DestoryQueue(usb->xmit);
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

DMPAPI(void) usb_SetSerialState(void *vusb, unsigned short serial)
{
	USB_Device *usb = (USB_Device *)vusb;
	io_DisableINT();
	usb->serial_state = serial;
	EP1_InHandler(usb);
	io_RestoreINT();
}

DMPAPI(void) usb_SetTimeOut(void *vusb, unsigned long rx_timeout, unsigned long tx_timeout)
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

DMPAPI(int) usb_Receive(void *vusb, unsigned char* buf, int bsize)
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
			buf[i] = (unsigned char)PopQueue(usb->rcvd);
		
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

DMPAPI(int) usb_Send(void *vusb, unsigned char *buf, int bsize)
{
	int i;
	unsigned long pretime;
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

DMPAPI(bool) usb_Write(void *vusb, unsigned char val)
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

DMPAPI(unsigned short) usb_GetControlLineState(void *vusb)
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
	
	return true;
}
