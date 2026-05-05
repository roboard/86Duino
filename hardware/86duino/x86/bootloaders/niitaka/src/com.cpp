/*
  com.cpp - DM&P Vortex86 Universal Serial library
  Copyright (c) 2018 Johnson Hung <Dyhung@dmp.com.tw>. All right reserved.

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

bool (*usb_reinitialize)(struct usb_dev *) = NULL;
void (*usb_uninitialize)(struct usb_dev *) = NULL;
void (*usb_suspend)(void) = NULL;

DMPAPI(void) com_SetUSBCallback(bool (*reinitialize)(struct usb_dev *), void (*uninitialize)(struct usb_dev *), void (*suspend)(void))
{
    usb_reinitialize = reinitialize;
    usb_uninitialize = uninitialize;
    usb_suspend = suspend;
}

static int canSpace = IO_PORT;
DMPAPI(void) com_SetCANSpace(int space)
{
    if (space != IO_PORT && space != IO_MEMORY) {
        return;
    }
    canSpace = space;
}

static int InUse[SIZE_OF_COM] = {0};

DMPAPI(COMPort *) CreateCOMPort(int com)
{
	COMPort *port;
	
	if (com >= SIZE_OF_COM || com < 0) return NULL;
	if (InUse[com] != 0) return NULL;
	
	if ((port = (COMPort *)ker_Malloc(sizeof(COMPort))) == NULL) return NULL;
	
	port->com                 = com;
	port->func                = NULL;
	port->Close               = NULL;
	port->SetTimeOut          = NULL;
	port->Read                = NULL;
	port->Receive             = NULL;
	port->QueryRxQueue        = NULL;
	port->RxQueueFull         = NULL;
	port->RxQueueEmpty        = NULL;
	port->FlushRxQueue        = NULL;
	port->Write               = NULL;
	port->Send                = NULL;
	port->QueryTxQueue        = NULL;
	port->TxQueueFull         = NULL;
	port->TxQueueEmpty        = NULL;
	port->FlushTxQueue        = NULL;
	port->TxReady             = NULL;
	port->FlushWFIFO          = NULL;
	port->SetBPS              = NULL;
	port->SetFormat           = NULL;
	port->SetFlowControl      = NULL;
	port->EnableFIFO          = NULL;
	port->SetWFIFOSize        = NULL;
	port->ClearRFIFO          = NULL;
	port->ClearWFIFO          = NULL;
	port->SetLSRHandler       = NULL;
	port->SetMSRHandler       = NULL;
	port->GetLSR              = NULL;
	port->GetMSR              = NULL;
	port->Ready               = NULL;
    port->ReadBreak           = NULL;
    port->GetBaud             = NULL;
    port->GetStopBits         = NULL;
    port->GetParityType       = NULL;
    port->GetDataBits         = NULL;
    port->GetDTR              = NULL;
    port->GetRTS              = NULL;
    port->Connect             = NULL;
    port->Disconnect          = NULL;
    port->SendBreak           = NULL;
	port->Reset               = NULL;
	port->AddIDFilter         = NULL;
	port->GetIDFilter         = NULL;
	port->DelIDFilter         = NULL;
	port->ClearIDList         = NULL;
	port->EnableBypass        = NULL;
	port->DisableBypass       = NULL;
	port->SetEWLimit          = NULL;
	port->GetEWLimit          = NULL;
	port->GetTxErrorCount     = NULL;
	port->GetRxErrorCount     = NULL;
	port->GetNowState         = NULL;
	port->EnableStoreError    = NULL;
	port->DisableStoreError   = NULL;
	port->SetCANBusOffHandler = NULL;
	port->PopError            = NULL;
	port->GetLastError        = NULL;
	port->ReadCAN             = NULL;
	port->WriteCAN            = NULL;
	port->EnableHalfDuplex    = NULL;
	port->EnableFullDuplex    = NULL;
	port->EnableDebugMode     = NULL;
	port->DisableDebugMode    = NULL;
    port->SetFastReadMode     = NULL;
    port->SetFastWriteMode    = NULL;
    port->CheckGlobalInterruptEnable = NULL;
    port->CheckGlobalInterruptDisable = NULL;
    port->SetBitTiming = NULL;
    port->SetControlLineStateCallback = NULL;
    port->SetDataInCallback = NULL;
    port->SetDataOutCallback = NULL;
    port->SetSOFCallback = NULL;
    port->EnableSOFInterrupt  = NULL;
    port->DisableSOFInterrupt = NULL;
	
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
            port->SetFastReadMode    = uart_SetFastReadMode;
            port->SetFastWriteMode   = uart_SetFastWriteMode;
            port->CheckGlobalInterruptEnable  = uart_CheckGlobalInterruptEnable;
            port->CheckGlobalInterruptDisable = uart_CheckGlobalInterruptDisable;
			
			return port;
		}
		
		case USB_COM:
		{
			if ((port->func = (void *)CreateUSBDevice(usb_reinitialize, usb_uninitialize, usb_suspend)) == NULL)
                goto FAIL;
			if (usb_Init(port->func) == false)
			{
				DestoryUSBDevice(port->func);
				goto FAIL;
			}
			
			port->Close               = usb_Close;
			port->SetTimeOut          = usb_SetTimeOut;
			port->Read                = usb_Read;
			port->Receive             = usb_Receive;
			port->QueryRxQueue        = usb_QueryRxQueue;
			port->RxQueueFull         = usb_RxQueueFull;
			port->RxQueueEmpty        = usb_RxQueueEmpty;
			port->FlushRxQueue        = usb_FlushRxQueue;
			port->Write               = usb_Write;
			port->Send                = usb_Send;
			port->QueryTxQueue        = usb_QueryTxQueue;
			port->TxQueueFull         = usb_TxQueueFull;
			port->TxQueueEmpty        = usb_TxQueueEmpty;
			port->FlushTxQueue        = usb_FlushTxQueue;
			port->TxReady             = usb_TxReady;
			port->FlushWFIFO          = usb_FlushWFIFO;
			port->Ready               = usb_Ready;
            port->ReadBreak           = usb_ReadBreak;
            port->GetBaud             = usb_GetBaud;
            port->GetStopBits         = usb_GetStopBits;
            port->GetParityType       = usb_GetParityType;
            port->GetDataBits         = usb_GetDataBits;
            port->GetDTR              = usb_GetDTR;
            port->GetRTS              = usb_GetRTS;
            port->Connect             = usb_Connect;
            port->Disconnect          = usb_Disconnect;
            port->SendBreak           = usb_SendBreak;
            port->SetControlLineStateCallback = usb_SetControlLineStateCallback;
            port->SetDataInCallback = usb_SetDataInCallback;
            port->SetDataOutCallback = usb_SetDataOutCallback;
            port->SetSOFCallback = usb_SetSOFCallback;
            port->EnableSOFInterrupt  = usb_EnableSOFInterrupt;
            port->DisableSOFInterrupt = usb_DisableSOFInterrupt;
			
			return port;
		}
		
		case CANBUS0: case CANBUS1:
		{
            int can;
            switch (com) {
                case CANBUS0: can = CAN0; break;
                case CANBUS1: can = CAN1; break;
            }
            
			if ((port->func = (void *)CreateCANBus(can, canSpace)) == NULL) goto FAIL;
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
            port->SetBitTiming       = can_SetBitTiming;
			
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

DMPAPI(void) com_SetFastReadMode(COMPort *port, int mode)
{
    if (port->SetFastReadMode == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->SetFastReadMode(port->func, mode);
}

DMPAPI(void) com_SetFastWriteMode(COMPort *port, int mode)
{
    if (port->SetFastWriteMode == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->SetFastWriteMode(port->func, mode);
}

DMPAPI(void) com_CheckGlobalInterruptEnable(COMPort *port)
{
	if (port->CheckGlobalInterruptEnable == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->CheckGlobalInterruptEnable(port->func);
}

DMPAPI(void) com_CheckGlobalInterruptDisable(COMPort *port)
{
	if (port->CheckGlobalInterruptDisable == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->CheckGlobalInterruptDisable(port->func);
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

DMPAPI(int) com_ReadBreak(COMPort *port)
{
	if (port->ReadBreak == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return true;
	}
	return port->ReadBreak(port->func);
}

DMPAPI(unsigned long) com_GetBaud(COMPort *port)
{
	if (port->GetBaud == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return true;
	}
	return port->GetBaud(port->func);
}

DMPAPI(unsigned char) com_GetStopBits(COMPort *port)
{
	if (port->GetStopBits == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return true;
	}
	return port->GetStopBits(port->func);
}

DMPAPI(unsigned char) com_GetParityType(COMPort *port)
{
	if (port->GetParityType == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return true;
	}
	return port->GetParityType(port->func);
}

DMPAPI(unsigned char) com_GetDataBits(COMPort *port)
{
	if (port->GetDataBits == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return true;
	}
	return port->GetDataBits(port->func);
}

DMPAPI(bool) com_GetDTR(COMPort *port)
{
	if (port->GetDTR == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return true;
	}
	return port->GetDTR(port->func);
}

DMPAPI(bool) com_GetRTS(COMPort *port)
{
	if (port->GetRTS == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return true;
	}
	return port->GetRTS(port->func);
}

DMPAPI(bool) com_Connect(COMPort *port)
{
	if (port->Connect == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return true;
	}
	return port->Connect(port->func);
}

DMPAPI(bool) com_Disconnect(COMPort *port)
{
	if (port->Disconnect == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return true;
	}
	return port->Disconnect(port->func);
}

DMPAPI(bool) com_SendBreak(COMPort *port, int duration)
{
	if (port->SendBreak == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return true;
	}
	return port->SendBreak(port->func, duration);
}

DMPAPI(void) com_SetControlLineStateCallback(COMPort *port, void (*func)(void))
{
	if (port->SetControlLineStateCallback == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->SetControlLineStateCallback(port->func, func);
}

DMPAPI(void) com_SetDataInCallback(COMPort *port, void (*func)(void))
{
	if (port->SetDataInCallback == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->SetDataInCallback(port->func, func);
}

DMPAPI(void) com_SetDataOutCallback(COMPort *port, void (*func)(void))
{
	if (port->SetDataOutCallback == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->SetDataOutCallback(port->func, func);
}

DMPAPI(void) com_SetSOFCallback(COMPort *port, void (*func)(void))
{
	if (port->SetSOFCallback == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->SetSOFCallback(port->func, func);
}

DMPAPI(void) com_EnableSOFInterrupt(COMPort *port)
{
	if (port->EnableSOFInterrupt == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->EnableSOFInterrupt(port->func);
}

DMPAPI(void) com_DisableSOFInterrupt(COMPort *port)
{
	if (port->DisableSOFInterrupt == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return;
	}
	port->DisableSOFInterrupt(port->func);
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

DMPAPI(bool) com_SetBitTiming(COMPort *port, unsigned int ckdiv, unsigned int sjw
                                                               , unsigned int prog
                                                               , unsigned int ps1
                                                               , unsigned int ps2)
{
	if (port->SetBitTiming == NULL)
	{
		err_print((char*)"%s: function pointer is null.\n", __FUNCTION__);
		return false;
	}
	return port->SetBitTiming(port->func, ckdiv, sjw, prog, ps1, ps2);
}

