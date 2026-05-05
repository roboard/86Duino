/*
  com.h - DM&P Vortex86 Universal Serial library
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

#ifndef __DMP_COM_H
#define __DMP_COM_H

#define COM_LIB_TIMEOUT_DEBUG    (0)

#include "dmpcfg.h"
#include "uart.h"
#include "usbcom.h"
#include "can.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int   com;
    void *func;

    DMPAPI(void) (*Close)(void *);
    DMPAPI(bool) (*SetBPS)(void *, unsigned long); // only for UART & CAN
    DMPAPI(void) (*SetTimeOut)(void *, unsigned long, unsigned long);

    DMPAPI(unsigned int) (*Read)(void *);
    DMPAPI(int)  (*Receive)(void *, unsigned char *, int);
    DMPAPI(int)  (*QueryRxQueue)(void *);
    DMPAPI(bool) (*RxQueueFull)(void *);
    DMPAPI(bool) (*RxQueueEmpty)(void *);
    DMPAPI(void) (*FlushRxQueue)(void *);

    DMPAPI(bool) (*Write)(void *, unsigned char);
    DMPAPI(int)  (*Send)(void *, unsigned char *, int);
    DMPAPI(int)  (*QueryTxQueue)(void *);
    DMPAPI(bool) (*TxQueueFull)(void *);
    DMPAPI(bool) (*TxQueueEmpty)(void *);
    DMPAPI(void) (*FlushTxQueue)(void *);
    DMPAPI(bool) (*TxReady)(void *);
    DMPAPI(void) (*FlushWFIFO)(void *);

    /* only for UART */
    DMPAPI(void) (*SetFormat)(void *, unsigned char);
    DMPAPI(void) (*SetFlowControl)(void *, int);
    DMPAPI(void) (*EnableFIFO)(void *, int);
    DMPAPI(bool) (*SetWFIFOSize)(void *, int);
    DMPAPI(void) (*ClearRFIFO)(void *);
    DMPAPI(void) (*ClearWFIFO)(void *);
    DMPAPI(void) (*SetLSRHandler)(void *, void (*)(SerialPort *));
    DMPAPI(void) (*SetMSRHandler)(void *, void (*)(SerialPort *));
    DMPAPI(unsigned char) (*GetLSR)(void *);
    DMPAPI(unsigned char) (*GetMSR)(void *);
    DMPAPI(void) (*EnableHalfDuplex)(void *);
    DMPAPI(void) (*EnableFullDuplex)(void *);
    DMPAPI(void) (*EnableDebugMode)(void *);
    DMPAPI(void) (*DisableDebugMode)(void *);
    DMPAPI(void) (*SetFastReadMode)(void *, int);
    DMPAPI(void) (*SetFastWriteMode)(void *, int);
    DMPAPI(void) (*CheckGlobalInterruptEnable)(void *);
    DMPAPI(void) (*CheckGlobalInterruptDisable)(void *);

    /* only for USB device */
    DMPAPI(bool) (*Ready)(void *);
    DMPAPI(int)  (*ReadBreak)(void *vusb);
    DMPAPI(unsigned long) (*GetBaud)(void *vusb);
    DMPAPI(unsigned char) (*GetStopBits)(void *vusb);
    DMPAPI(unsigned char) (*GetParityType)(void *vusb);
    DMPAPI(unsigned char) (*GetDataBits)(void *vusb);
    DMPAPI(bool) (*GetDTR)(void *vusb);
    DMPAPI(bool) (*GetRTS)(void *vusb);
    DMPAPI(bool) (*Connect)(void *vusb);
    DMPAPI(bool) (*Disconnect)(void *vusb);
    DMPAPI(bool) (*SendBreak)(void *vusb, int duration);
    DMPAPI(void) (*SetControlLineStateCallback)(void *vusb, void (*)(void));
    DMPAPI(void) (*SetDataInCallback)(void *vusb, void (*)(void));
    DMPAPI(void) (*SetDataOutCallback)(void *vusb, void (*)(void));
    DMPAPI(void) (*SetSOFCallback)(void *vusb, void (*)(void));
    DMPAPI(void) (*EnableSOFInterrupt)(void *vusb);
    DMPAPI(void) (*DisableSOFInterrupt)(void *vusb);

    /* only for CAN bus */
    DMPAPI(void) (*Reset)(void *);
    DMPAPI(bool) (*AddIDFilter)(void *, int, int, unsigned long, unsigned long);
    DMPAPI(bool) (*GetIDFilter)(void *, int, int *, unsigned long *, unsigned long *);
    DMPAPI(bool) (*DelIDFilter)(void *, int);
    DMPAPI(void) (*ClearIDList)(void *);
    DMPAPI(void) (*EnableBypass)(void *);
    DMPAPI(void) (*DisableBypass)(void *);
    DMPAPI(bool) (*SetEWLimit)(void *, int);
    DMPAPI(int)  (*GetEWLimit)(void *);
    DMPAPI(int)  (*GetTxErrorCount)(void *);
    DMPAPI(int)  (*GetRxErrorCount)(void *);
    DMPAPI(void) (*EnableStoreError)(void *);
    DMPAPI(void) (*DisableStoreError)(void *);
    DMPAPI(void) (*SetCANBusOffHandler)(void *, void (*)(CAN_Bus *));
    DMPAPI(unsigned char)  (*GetNowState)(void *);
    DMPAPI(unsigned char) (*PopError)(void *);
    DMPAPI(unsigned char) (*GetLastError)(void *);
    DMPAPI(bool) (*ReadCAN)(void *, CANFrame*);
    DMPAPI(bool) (*WriteCAN)(void *, CANFrame*);
    DMPAPI(bool) (*SetBitTiming)(void *, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);

} COMPort;

// Note that set CAN bus space (IO/Memory) before init CAN
//      IO_PORT
//      IO_MEMORY
DMPAPI(void) com_SetCANSpace(int space);

#define SIZE_OF_COM			(13) // (COM1 ~ COM10) + USB_COM + CANBUS0 + CANBUS1

#define COM1			   	(0x00)
#define COM2			   	(0x01)
#define COM3			   	(0x02)
#define COM4			   	(0x03)
#define COM5			   	(0x04)
#define COM6			   	(0x05)
#define COM7			   	(0x06)
#define COM8			   	(0x07)
#define COM9			   	(0x08)
#define COM10				(0x09)
#define USB_COM			   	(0x0A)
#define CANBUS0				(0x0B + CAN0)    // CAN0 = 0
#define CANBUS1				(0x0B + CAN1)    // CAN1 = 1
#define CAN_BUS				(CANBUS0)        // Compatible to previous version library
DMPAPI(COMPort *) com_Init(int com);
DMPAPI(void) com_Close(COMPort *port);

DMPAPI(bool) com_SetBPS(COMPort *port, unsigned long bps);
// for UART
#define COM_UARTBAUD_6000000BPS         (UARTBAUD_6000000BPS)
#define COM_UARTBAUD_3000000BPS         (UARTBAUD_3000000BPS)
#define COM_UARTBAUD_2000000BPS         (UARTBAUD_2000000BPS)
#define COM_UARTBAUD_1500000BPS         (UARTBAUD_1500000BPS)
#define COM_UARTBAUD_1000000BPS         (UARTBAUD_1000000BPS)
#define COM_UARTBAUD_750000BPS          (UARTBAUD_750000BPS)
#define COM_UARTBAUD_500000BPS          (UARTBAUD_500000BPS)
#define COM_UARTBAUD_461538BPS          (UARTBAUD_461538BPS)
#define COM_UARTBAUD_333333BPS          (UARTBAUD_333333BPS)
#define COM_UARTBAUD_300000BPS          (UARTBAUD_300000BPS)
#define COM_UARTBAUD_250000BPS          (UARTBAUD_250000BPS)
#define COM_UARTBAUD_230400BPS          (UARTBAUD_230400BPS)
#define COM_UARTBAUD_200000BPS          (UARTBAUD_200000BPS)
#define COM_UARTBAUD_150000BPS          (UARTBAUD_150000BPS)
#define COM_UARTBAUD_125000BPS          (UARTBAUD_125000BPS)
#define COM_UARTBAUD_115200BPS          (UARTBAUD_115200BPS)
#define COM_UARTBAUD_57600BPS           (UARTBAUD_57600BPS)
#define COM_UARTBAUD_38400BPS           (UARTBAUD_38400BPS)
#define COM_UARTBAUD_28800BPS           (UARTBAUD_28800BPS)
#define COM_UARTBAUD_19200BPS           (UARTBAUD_19200BPS)
#define COM_UARTBAUD_14400BPS           (UARTBAUD_14400BPS)
#define COM_UARTBAUD_9600BPS            (UARTBAUD_9600BPS)
#define COM_UARTBAUD_4800BPS            (UARTBAUD_4800BPS)
#define COM_UARTBAUD_2400BPS            (UARTBAUD_2400BPS)
#define COM_UARTBAUD_1200BPS            (UARTBAUD_1200BPS)
#define COM_UARTBAUD_600BPS             (UARTBAUD_600BPS)
#define COM_UARTBAUD_300BPS             (UARTBAUD_300BPS)
#define COM_UARTBAUD_50BPS              (UARTBAUD_50BPS)
// for CAN bus
#define COM_CAN_BPS_1000K               (CAN_BPS_1000K)
#define COM_CAN_BPS_833K                (CAN_BPS_833K)
#define COM_CAN_BPS_800K                (CAN_BPS_800K)
#define COM_CAN_BPS_600K                (CAN_BPS_600K)
#define COM_CAN_BPS_500K                (CAN_BPS_500K)
#define COM_CAN_BPS_250K                (CAN_BPS_250K)
#define COM_CAN_BPS_200K                (CAN_BPS_200K)
#define COM_CAN_BPS_125K                (CAN_BPS_125K)
#define COM_CAN_BPS_100K                (CAN_BPS_100K)
#define COM_CAN_BPS_83K3                (CAN_BPS_83K3)
#define COM_CAN_BPS_80K                 (CAN_BPS_80K)
#define COM_CAN_BPS_60K                 (CAN_BPS_60K)
#define COM_CAN_BPS_50K                 (CAN_BPS_50K)
#define COM_CAN_BPS_25K                 (CAN_BPS_25K)
#define COM_CAN_BPS_20K                 (CAN_BPS_20K)
#define COM_CAN_BPS_10K                 (CAN_BPS_10K)

DMPAPI(void) com_SetTimeOut(COMPort *port, unsigned long rx_timeout, unsigned long tx_timeout);
#define NO_TIMEOUT         (-1)

DMPAPI(unsigned int) com_Read(COMPort *port); // not for CAN bus
DMPAPI(int)  com_Receive(COMPort *port, unsigned char* buf, int bsize);
DMPAPI(int)  com_QueryRxQueue(COMPort *port);
DMPAPI(bool) com_RxQueueFull(COMPort *port);
DMPAPI(bool) com_RxQueueEmpty(COMPort *port);
DMPAPI(void) com_FlushRxQueue(COMPort *port);

DMPAPI(bool) com_Write(COMPort *port, unsigned char val); // not for CAN bus
DMPAPI(int)  com_Send(COMPort *port, unsigned char* buf, int bsize);
DMPAPI(int)  com_QueryTxQueue(COMPort *port);
DMPAPI(bool) com_TxQueueFull(COMPort *port);
DMPAPI(bool) com_TxQueueEmpty(COMPort *port);
DMPAPI(void) com_FlushTxQueue(COMPort *port);
DMPAPI(bool) com_TxReady(COMPort *port);
DMPAPI(void) com_FlushWFIFO(COMPort *port);


/* only for UART */
DMPAPI(void) com_SetFormat(COMPort *port, unsigned char format);
#define BYTESIZE5          (0x00)
#define BYTESIZE6          (0x01)
#define BYTESIZE7          (0x02)
#define BYTESIZE8          (0x03)
#define STOPBIT1           (0x00)
#define STOPBIT2           (0x04)
#define NOPARITY           (0x00)
#define ODDPARITY          (0x08)
#define EVENPARITY         (0x18)
DMPAPI(void) com_SetFlowControl(COMPort *port, int control);
#define NO_CONTROL         (0)
#define RTS_CTS            (1)
#define XON_XOFF           (2)
DMPAPI(void) com_EnableFIFO(COMPort *port, int fifo);
#define FIFO_001		   (DISABLE_HW_FIFO)
#define FIFO_016		   (ENABLE_HW_NORMAL_FIFO)
#define FIFO_032		   (ENABLE_HW_TURBO_FIFO)
#define FIFO_TURBO		   (FIFO_032)
DMPAPI(bool) com_SetWFIFOSize(COMPort *port, int size); // setup after enable/disable fifo
DMPAPI(void) com_ClearRFIFO(COMPort *port);
DMPAPI(void) com_ClearWFIFO(COMPort *port);
DMPAPI(void) com_SetLSRHandler(COMPort *port, void (*func)(SerialPort *port));
DMPAPI(void) com_SetMSRHandler(COMPort *port, void (*func)(SerialPort *port));
DMPAPI(unsigned char) com_GetLSR(COMPort *port);
#define LSR_EB             (0x80)
#define LSR_TEMT           (0x40)
#define LSR_THRE           (0x20)
#define LSR_BI             (0x10)
#define LSR_FE             (0x08)
#define LSR_PE             (0x04)
#define LSR_OE             (0x02)
#define LSR_DR             (0x01)
DMPAPI(unsigned char) com_GetMSR(COMPort *port);
#define MSR_DCD            (0x80)
#define MSR_RI             (0x40)
#define MSR_DSR            (0x20)
#define MSR_CTS            (0x10)
#define MSR_DDCD           (0x08)
#define MSR_TERI           (0x04)
#define MSR_DDSR           (0x02)
#define MSR_DCTS           (0x01)
DMPAPI(void) com_EnableHalfDuplex(COMPort *port);
DMPAPI(void) com_EnableFullDuplex(COMPort *port);
DMPAPI(void) com_EnableDebugMode(COMPort *port);
DMPAPI(void) com_DisableDebugMode(COMPort *port);
DMPAPI(void) com_SetFastReadMode(COMPort *port, int mode);
#define UART_FAST_READ_AUTO     (FAST_READ_AUTO)
#define UART_NATIVE_READ        (NATIVE_READ)
#define UART_FAST_READ_MODE0W   (FAST_READ_MODE0W)
#define UART_FAST_READ_MODE0D   (FAST_READ_MODE0D)
#define UART_FAST_READ_MODE1B   (FAST_READ_MODE1B)
#define UART_FAST_READ_MODE1W   (FAST_READ_MODE1W)
#define UART_FAST_READ_MODE1D   (FAST_READ_MODE1D)
DMPAPI(void) com_SetFastWriteMode(COMPort *port, int mode);
#define UART_FAST_WRITE_AUTO    (FAST_WRITE_AUTO)
#define UART_NATIVE_WRITE       (NATIVE_WRITE)
#define UART_FAST_WRITE_B       (FAST_WRITE_B)
#define UART_FAST_WRITE_W       (FAST_WRITE_W)
#define UART_FAST_WRITE_D       (FAST_WRITE_D)
DMPAPI(void) com_CheckGlobalInterruptEnable(COMPort *port);
DMPAPI(void) com_CheckGlobalInterruptDisable(COMPort *port);

/* only for USB device */
DMPAPI(void) com_SetUSBCallback(bool (*reinitialize)(struct usb_dev *), void (*uninitialize)(struct usb_dev *), void (*suspend)(void));
DMPAPI(bool) com_Ready(COMPort *port);
DMPAPI(int) com_ReadBreak(COMPort *port);
DMPAPI(unsigned long) com_GetBaud(COMPort *port);
DMPAPI(unsigned char) com_GetStopBits(COMPort *port);
DMPAPI(unsigned char) com_GetParityType(COMPort *port);
DMPAPI(unsigned char) com_GetDataBits(COMPort *port);
DMPAPI(bool) com_GetDTR(COMPort *port);
DMPAPI(bool) com_GetRTS(COMPort *port);
DMPAPI(bool) com_Connect(COMPort *port);
DMPAPI(bool) com_Disconnect(COMPort *port);
DMPAPI(bool) com_SendBreak(COMPort *port, int duration);
DMPAPI(void) com_SetControlLineStateCallback(COMPort *port, void (*func)(void));
DMPAPI(void) com_SetDataInCallback(COMPort *port, void (*func)(void));
DMPAPI(void) com_SetDataOutCallback(COMPort *port, void (*func)(void));
DMPAPI(void) com_SetSOFCallback(COMPort *port, void (*func)(void));
DMPAPI(void) com_EnableSOFInterrupt(COMPort *port);
DMPAPI(void) com_DisableSOFInterrupt(COMPort *port);

/* only for CAN bus */
DMPAPI(void) com_Reset(COMPort *port);
DMPAPI(bool) com_AddIDFilter(COMPort *port, int index, int ext_id, unsigned long filter, unsigned long mask);
DMPAPI(bool) com_GetIDFilter(COMPort *port, int index, int *ext_id, unsigned long *filter, unsigned long *mask);
DMPAPI(bool) com_DelIDFilter(COMPort *port, int index);
DMPAPI(void) com_ClearIDList(COMPort *port);
DMPAPI(void) com_EnableBypass(COMPort *port);
DMPAPI(void) com_DisableBypass(COMPort *port);
DMPAPI(bool) com_SetEWLimit(COMPort *port, int ewl);
DMPAPI(int)  com_GetEWLimit(COMPort *port);
DMPAPI(int)  com_GetTxErrorCount(COMPort *port);
DMPAPI(int)  com_GetRxErrorCount(COMPort *port);
DMPAPI(void) com_EnableStoreError(COMPort *port);
DMPAPI(void) com_DisableStoreError(COMPort *port);
DMPAPI(void) com_SetCANBusOffHandler(COMPort *port, void (*func)(CAN_Bus *));
DMPAPI(unsigned char)  com_GetNowState(COMPort *port);
DMPAPI(unsigned char) com_PopError(COMPort *port);
DMPAPI(unsigned char) com_GetLastError(COMPort *port);
DMPAPI(bool) com_ReadCAN(COMPort *port, CANFrame*);
DMPAPI(bool) com_WriteCAN(COMPort *port, CANFrame*);
DMPAPI(bool) com_SetBitTiming(COMPort *port, unsigned int ckdiv, unsigned int sjw
                                                               , unsigned int prog
                                                               , unsigned int ps1
                                                               , unsigned int ps2);

#ifdef __cplusplus
}
#endif

#endif
