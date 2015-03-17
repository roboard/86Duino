/*
  com.cpp - DM&P Vortex86 Universal Serial library
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

#define __DMP_COM_LIB

#include "com.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "io.h"
#include "err.h"

static bool set_usb_pins = false;
static char usb_detect_port;
static char usb_detect_pin;
static char usb_on_off_port;
static char usb_on_off_pin;
DMPAPI(void) com_SetUSBPins(char port1, char pin1, char port2, char pin2)
{
	usb_detect_port = port1;
	usb_detect_pin  = pin1;
	usb_on_off_port = port2;
	usb_on_off_pin  = pin2;
	set_usb_pins = true;
}

static int InUse[SIZE_OF_COM] = {0};

DMPAPI(COMPort *) CreateCOMPort(int com)
{
	COMPort *port;
	
	if (com >= SIZE_OF_COM || com < 0) return NULL;
	if (InUse[com] != 0) return NULL;
	
	if ((port = (COMPort *)ker_Malloc(sizeof(COMPort))) == NULL) return NULL;
	
	port->com                = com;
	port->func               = NULL;
	port->Close              = NULL;
	port->SetTimeOut         = NULL;
	port->Read               = NULL;
	port->Receive            = NULL;
	port->QueryRxQueue       = NULL;
	port->RxQueueFull        = NULL;
	port->RxQueueEmpty       = NULL;
	port->FlushRxQueue       = NULL;
	port->Write              = NULL;
	port->Send               = NULL;
	port->QueryTxQueue       = NULL;
	port->TxQueueFull        = NULL;
	port->TxQueueEmpty       = NULL;
	port->FlushTxQueue       = NULL;
	port->TxReady            = NULL;
	port->FlushWFIFO         = NULL;
	port->SetBPS             = NULL;
	port->SetFormat          = NULL;
	port->SetFlowControl     = NULL;
	port->EnableFIFO         = NULL;
	port->SetWFIFOSize       = NULL;
	port->ClearRFIFO         = NULL;
	port->ClearWFIFO         = NULL;
	port->SetLSRHandler      = NULL;
	port->SetMSRHandler      = NULL;
	port->GetLSR             = NULL;
	port->GetMSR             = NULL;
	port->Ready              = NULL;
	port->GetLineCoding      = NULL;
	port->SetSerialState     = NULL;
	port->GetControlLineState= NULL;
	port->Reset              = NULL;
	port->AddIDFilter        = NULL;
	port->GetIDFilter        = NULL;
	port->DelIDFilter        = NULL;
	port->ClearIDList        = NULL;
	port->EnableBypass       = NULL;
	port->DisableBypass      = NULL;
	port->SetEWLimit         = NULL;
	port->GetEWLimit         = NULL;
	port->GetTxErrorCount    = NULL;
	port->GetRxErrorCount    = NULL;
	port->GetNowState        = NULL;
	port->EnableStoreError   = NULL;
	port->DisableStoreError  = NULL;
	port->SetCANBusOffHandler= NULL;
	port->PopError           = NULL;
	port->GetLastError       = NULL;
	port->ReadCAN            = NULL;
	port->WriteCAN           = NULL;
	port->EnableHalfDuplex   = NULL;
	port->EnableFullDuplex   = NULL;
	port->EnableDebugMode    = NULL;
	port->DisableDebugMode   = NULL;
	
	return port;
}

DMPAPI(COMPort *) com_Init(int com)
{
	COMPort *port;

	if ((port = (COMPort *)CreateCOMPort(com)) == NULL) return NULL;

	switch (com)
	{
		case COM1: case COM2: case COM3: case COM4: case COM5:
		case COM6: case COM7: case COM8: case COM9: case COM10:
		{
			if ((port->func = (void *)CreateUART(com)) == NULL) goto FAIL;
			if (uart_Init(port->func) == false)
			{
				uart_Close(port->func);
				goto FAIL;
			}
			
			port->Close              = uart_Close;
			port->SetTimeOut         = uart_SetTimeOut;
			port->Read               = uart_Read;
			port->Receive            = uart_Receive;
			port->QueryRxQueue       = uart_QueryRxQueue;
			port->RxQueueFull        = uart_RxQueueFull;
			port->RxQueueEmpty       = uart_RxQueueEmpty;
			port->FlushRxQueue       = uart_FlushRxQueue;
			port->Write              = uart_Write;
			port->Send               = uart_Send;
			port->QueryTxQueue       = uart_QueryTxQueue;
			port->TxQueueFull        = uart_TxQueueFull;
			port->TxQueueEmpty       = uart_TxQueueEmpty;
			port->FlushTxQueue       = uart_FlushTxQueue;
			port->TxReady            = uart_TxReady;
			port->FlushWFIFO         = uart_FlushWFIFO;
			port->SetBPS             = uart_SetBaud;
			port->SetFormat          = uart_SetFormat;
			port->SetFlowControl     = uart_SetFlowControl;
			port->EnableFIFO         = uart_SetHWFIFO;
			port->SetWFIFOSize       = uart_SetWFIFOSize;
			port->ClearRFIFO         = uart_ClearRFIFO;
			port->ClearWFIFO         = uart_ClearWFIFO;
			port->SetLSRHandler      = uart_SetLSRHandler;
			port->SetMSRHandler      = uart_SetMSRHandler;
			port->GetLSR             = uart_GetLSR;
			port->GetMSR             = uart_GetMSR;
			port->EnableHalfDuplex   = uart_EnableHalfDuplex;
			port->EnableFullDuplex   = uart_EnableFullDuplex;
			port->EnableDebugMode    = uart_EnableDebugMode;
			port->DisableDebugMode   = uart_DisableDebugMode;
			
			return port;
		}
		
		case USB_COM:
		{
			if ((port->func = (void *)CreateUSBDevice()) == NULL) goto FAIL;
			if (set_usb_pins == false)
			{
				err_print((char*)"%s: no set USB-Device pins.\n", __FUNCTION__);
				usb_Close(port->func);
				goto FAIL;
			}
			if (usb_SetUSBPins(port->func, usb_detect_port, usb_detect_pin, usb_on_off_port, usb_on_off_pin) == false)
			{
				usb_Close(port->func);
				goto FAIL;
			}
			if (usb_Init(port->func) == false)
			{
				usb_Close(port->func);
				goto FAIL;
			}
			
			port->Close              = usb_Close;
			port->SetTimeOut         = usb_SetTimeOut;
			port->Read               = usb_Read;
			port->Receive            = usb_Receive;
			port->QueryRxQueue       = usb_QueryRxQueue;
			port->RxQueueFull        = usb_RxQueueFull;
			port->RxQueueEmpty       = usb_RxQueueEmpty;
			port->FlushRxQueue       = usb_FlushRxQueue;
			port->Write              = usb_Write;
			port->Send               = usb_Send;
			port->QueryTxQueue       = usb_QueryTxQueue;
			port->TxQueueFull        = usb_TxQueueFull;
			port->TxQueueEmpty       = usb_TxQueueEmpty;
			port->FlushTxQueue       = usb_FlushTxQueue;
			port->TxReady            = usb_TxReady;
			port->FlushWFIFO         = usb_FlushWFIFO;
			port->Ready              = usb_Ready;
			port->GetLineCoding      = usb_GetLineCoding;
			port->SetSerialState     = usb_SetSerialState;
			port->GetControlLineState= usb_GetControlLineState;
			
			return port;
		}
		
		case CAN_BUS:
		{
			if ((port->func = (void *)CreateCANBus(IO_PORT)) == NULL) goto FAIL;
			if (can_Init(port->func) == false)
			{
				can_Close(port->func);
				goto FAIL;
			}
			
			port->Close              = can_Close;
			port->SetTimeOut         = can_SetTimeOut;
			port->QueryRxQueue       = can_QueryRxQueue;
			port->RxQueueFull        = can_RxQueueFull;
			port->RxQueueEmpty       = can_RxQueueEmpty;
			port->FlushRxQueue       = can_FlushRxQueue;
			port->QueryTxQueue       = can_QueryTxQueue;
			port->TxQueueFull        = can_TxQueueFull;
			port->TxQueueEmpty       = can_TxQueueEmpty;
			port->FlushTxQueue       = can_FlushTxQueue;
			port->TxReady            = can_TxReady;
			port->FlushWFIFO         = can_FlushWFIFO;
			port->SetBPS             = can_SetBPS;
			port->Reset              = can_Reset;
			port->AddIDFilter        = can_AddIDFilter;
			port->GetIDFilter        = can_GetIDFilter;
			port->DelIDFilter        = can_DelIDFilter;
			port->ClearIDList        = can_ClearIDList;
			port->EnableBypass       = can_EnableBypass;
			port->DisableBypass      = can_DisableBypass;
			port->SetEWLimit         = can_SetEWLimit;
			port->GetEWLimit         = can_GetEWLimit;
			port->GetTxErrorCount    = can_GetTxErrorCount;
			port->GetRxErrorCount    = can_GetRxErrorCount;
			port->GetNowState        = can_GetNowState;
			port->EnableStoreError   = can_EnableStoreError;
			port->DisableStoreError  = can_DisableStoreError;
			port->SetCANBusOffHandler= can_SetCANBusOffHandler;
			port->PopError           = can_PopError;
			port->GetLastError       = can_GetLastError;
			port->ReadCAN            = can_Read;
			port->WriteCAN           = can_Write;
			
			return port;
		}
		
		default: break;
	};
	
FAIL:
	ker_Mfree(port);
	return NULL;
}

DMPAPI(void) com_Close(COMPort *port)
{
	if (port == NULL)
		return;
		
	if (port->Close == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->Close(port->func);
	ker_Mfree(port);
}

DMPAPI(bool) com_SetBPS(COMPort *port, unsigned long bps)
{
	if (port->SetBPS == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return true;
	}
	return port->SetBPS(port->func, bps);
}

DMPAPI(void) com_SetTimeOut(COMPort *port, unsigned long rx_timeout, unsigned long tx_timeout)
{
	if (port->SetTimeOut == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->SetTimeOut(port->func, rx_timeout, tx_timeout);
}

DMPAPI(unsigned int) com_Read(COMPort *port)
{
	if (port->Read == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return 0xFFFF;
	}
	return port->Read(port->func);
}

DMPAPI(int) com_Receive(COMPort *port, unsigned char* buf, int bsize)
{
	if (port->Receive == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return 0;
	}
	return port->Receive(port->func, buf, bsize);
}

DMPAPI(int) com_QueryRxQueue(COMPort *port)
{
	if (port->QueryRxQueue == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return 0;
	}
	return port->QueryRxQueue(port->func);
}

DMPAPI(bool) com_RxQueueFull(COMPort *port)
{
	if (port->RxQueueFull == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return false;
	}
	return port->RxQueueFull(port->func);
}

DMPAPI(bool) com_RxQueueEmpty(COMPort *port)
{
	if (port->RxQueueEmpty == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return false;
	}
	return port->RxQueueEmpty(port->func);
}

DMPAPI(void) com_FlushRxQueue(COMPort *port)
{
	if (port->FlushRxQueue == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->FlushRxQueue(port->func);
}

DMPAPI(bool) com_Write(COMPort *port, unsigned char val)
{
	if (port->Write == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return false;
	}
	return port->Write(port->func, val);
}

DMPAPI(int) com_Send(COMPort *port, unsigned char* buf, int bsize)
{
	if (port->Send == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return 0;
	}
	return port->Send(port->func, buf, bsize);
}

DMPAPI(int) com_QueryTxQueue(COMPort *port)
{
	if (port->QueryTxQueue == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return 0;
	}
	return port->QueryTxQueue(port->func);
}

DMPAPI(bool) com_TxQueueFull(COMPort *port)
{
	if (port->TxQueueFull == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return false;
	}
	return port->TxQueueFull(port->func);
}

DMPAPI(bool) com_TxQueueEmpty(COMPort *port)
{
	if (port->TxQueueEmpty == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return false;
	}
	return port->TxQueueEmpty(port->func);
}

DMPAPI(void) com_FlushTxQueue(COMPort *port)
{
	if (port->FlushTxQueue == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->FlushTxQueue(port->func);
}

DMPAPI(bool) com_TxReady(COMPort *port)
{
	if (port->TxReady == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return false;
	}
	return port->TxReady(port->func);
}

DMPAPI(void) com_FlushWFIFO(COMPort *port)
{
	if (port->FlushWFIFO == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->FlushWFIFO(port->func);
}

DMPAPI(void) com_SetFormat(COMPort *port, unsigned char format)
{
	if (port->SetFormat == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->SetFormat(port->func, format);
}

DMPAPI(void) com_SetFlowControl(COMPort *port, int control)
{
	if (port->SetFlowControl == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->SetFlowControl(port->func, control);
}

DMPAPI(unsigned char) com_GetLSR(COMPort *port)
{
	if (port->GetLSR == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return 0x00;
	}
	return port->GetLSR(port->func);
}

DMPAPI(unsigned char) com_GetMSR(COMPort *port)
{
	if (port->GetMSR == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return 0x00;
	}
	return port->GetMSR(port->func);
}
DMPAPI(void) com_EnableFIFO(COMPort *port, int fifo)
{
	if (port->EnableFIFO == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->EnableFIFO(port->func, fifo);
}

DMPAPI(bool) com_SetWFIFOSize(COMPort *port, int size) // setup after enable/disable fifo
{
	if (port->SetWFIFOSize == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return true;
	}
	return port->SetWFIFOSize(port->func, size);
}

DMPAPI(void) com_ClearRFIFO(COMPort *port)
{
	if (port->ClearRFIFO == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->ClearRFIFO(port->func);
}

DMPAPI(void) com_ClearWFIFO(COMPort *port)
{
	if (port->ClearWFIFO == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->ClearWFIFO(port->func);
}

DMPAPI(void) com_SetLSRHandler(COMPort *port, void (*func)(SerialPort *port))
{
	if (port->SetLSRHandler == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->SetLSRHandler(port->func, func);
}

DMPAPI(void) com_SetMSRHandler(COMPort *port, void (*func)(SerialPort *port))
{
	if (port->SetMSRHandler == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->SetMSRHandler(port->func, func);
}

DMPAPI(void) com_EnableHalfDuplex(COMPort *port)
{
	if (port->EnableHalfDuplex == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->EnableHalfDuplex(port->func);
}

DMPAPI(void) com_EnableFullDuplex(COMPort *port)
{
	if (port->EnableFullDuplex == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->EnableFullDuplex(port->func);
}

DMPAPI(void) com_EnableDebugMode(COMPort *port)
{
	if (port->EnableDebugMode == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->EnableDebugMode(port->func);
}

DMPAPI(void) com_DisableDebugMode(COMPort *port)
{
	if (port->DisableDebugMode == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->DisableDebugMode(port->func);
}

// for USB_COM
DMPAPI(bool)  com_Ready(COMPort *port)
{
	if (port->Ready == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return true;
	}
	return port->Ready(port->func);
}

DMPAPI(Line_Coding *) com_GetLineCoding(COMPort *port)
{
	if (port->GetLineCoding == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return NULL;
	}
	return port->GetLineCoding(port->func);
}

DMPAPI(void) com_SetSerialState(COMPort *port, unsigned short state)
{
	if (port->SetSerialState == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->SetSerialState(port->func, state);
}

DMPAPI(unsigned short) com_GetControlLineState(COMPort *port)
{
	if (port->GetControlLineState == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return 0xffff;
	}
	return usb_GetControlLineState(port->func);
}

/* for CAN-BUS */
DMPAPI(void) com_Reset(COMPort *port)
{
	if (port->Reset == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->Reset(port->func);
}

DMPAPI(bool) com_AddIDFilter(COMPort *port, int index, int ext_id, unsigned long filter, unsigned long mask)
{
	if (port->AddIDFilter == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return false;
	}
	return port->AddIDFilter(port->func, index, ext_id, filter, mask);
}

DMPAPI(bool) com_GetIDFilter(COMPort *port, int index, int *ext_id, unsigned long *filter, unsigned long *mask)
{
	if (port->GetIDFilter == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return false;
	}
	return port->GetIDFilter(port->func, index, ext_id, filter, mask);
}

DMPAPI(bool) com_DelIDFilter(COMPort *port, int index)
{
	if (port->DelIDFilter == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return false;
	}
	return port->DelIDFilter(port->func, index);
}

DMPAPI(void) com_ClearIDList(COMPort *port)
{
	if (port->ClearIDList == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->ClearIDList(port->func);
}

DMPAPI(void) com_EnableBypass(COMPort *port)
{
	if (port->EnableBypass == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->EnableBypass(port->func);
}

DMPAPI(void) com_DisableBypass(COMPort *port)
{
	if (port->DisableBypass == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->DisableBypass(port->func);
}

DMPAPI(bool) com_SetEWLimit(COMPort *port, int ewl)
{
	if (port->SetEWLimit == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return false;
	}
	return port->SetEWLimit(port->func, ewl);
}

DMPAPI(int) com_GetEWLimit(COMPort *port)
{
	if (port->GetEWLimit == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return -1;
	}
	return port->GetEWLimit(port->func);
}

DMPAPI(int)  com_GetTxErrorCount(COMPort *port)
{
	if (port->GetTxErrorCount == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return -1;
	}
	return port->GetTxErrorCount(port->func);
}

DMPAPI(int) com_GetRxErrorCount(COMPort *port)
{
	if (port->GetRxErrorCount == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return -1;
	}
	return port->GetRxErrorCount(port->func);
}

DMPAPI(unsigned char) com_GetNowState(COMPort *port)
{
	if (port->GetNowState == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return 0xFF;
	}
	return port->GetNowState(port->func);
}

DMPAPI(void) com_EnableStoreError(COMPort *port)
{
	if (port->EnableStoreError == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->EnableStoreError(port->func);
}

DMPAPI(void) com_DisableStoreError(COMPort *port)
{
	if (port->DisableStoreError == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->DisableStoreError(port->func);
}

DMPAPI(void) com_SetCANBusOffHandler(COMPort *port, void (*func)(CAN_Bus *))
{
	if (port->SetCANBusOffHandler == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->SetCANBusOffHandler(port->func, func);
}

DMPAPI(unsigned char) com_PopError(COMPort *port)
{
	if (port->PopError == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return 0xFF;
	}
	return port->PopError(port->func);
}

DMPAPI(unsigned char) com_GetLastError(COMPort *port)
{
	if (port->GetLastError == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return 0xFF;
	}
	return port->GetLastError(port->func);
}

DMPAPI(bool) com_ReadCAN(COMPort *port, CANFrame *pack)
{
	if (port->ReadCAN == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return false;
	}
	return port->ReadCAN(port->func, pack);
}

DMPAPI(bool) com_WriteCAN(COMPort *port, CANFrame *pack)
{
	if (port->WriteCAN == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return false;
	}
	return port->WriteCAN(port->func, pack);
}
