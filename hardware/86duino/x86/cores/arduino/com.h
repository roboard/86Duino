#ifndef __DMP_COM_H
#define __DMP_COM_H

#define COM_LIB_TIMEOUT_DEBUG    (0)

#include "dmpcfg.h"
#include "uart.h"
#include "usb.h"
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
	DMPAPI(void) (*SetTimeOut)(void *, unsigned long);
	
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
	
	/* only for USB device */
	DMPAPI(bool) (*Ready)(void *);
	DMPAPI(void) (*SetSerialState)(void *, unsigned short);
	DMPAPI(Line_Coding *) (*GetLineCoding)(void *);
	DMPAPI(unsigned short) (*GetControlLineState)(void *);
	
	/* only for CAN bus */
	DMPAPI(bool) (*SetBitTime)(void *, CAN_BitTime *);
	DMPAPI(bool) (*SetEWLimit)(void *, int);
	DMPAPI(bool) (*AddIDFilter)(void *, bool, unsigned long, unsigned long);
	DMPAPI(bool) (*DelIDFilter)(void *, bool, unsigned long, unsigned long);
	DMPAPI(void) (*ClearIDTable)(void *);
	DMPAPI(void) (*SetCANErrorHandler)(void *, void (*)(CAN_Bus *));
	DMPAPI(void) (*SetCANBusOfHandler)(void *, void (*)(CAN_Bus *));
	DMPAPI(unsigned char) (*GetSTAT)(void *);
	DMPAPI(unsigned char) (*GetERROR)(void *);
	DMPAPI(void) (*EnableBypass)(void *);
	DMPAPI(void) (*DisableBypass)(void *);
	DMPAPI(void) (*EnableStoreERROR)(void *);
	DMPAPI(void) (*DisableStoreERROR)(void *);
	DMPAPI(void) (*SetTxControl)(void *, int, unsigned long);
	DMPAPI(bool) (*GetRxControl)(void *, int *, unsigned long *, int *);
	
} COMPort;

// Note that set USB-Device DETECT & ONOFF pins before init USB_COM
DMPAPI(void) com_SetUSBPins(char port1, char pin1, char port2, char pin2);

#define SIZE_OF_COM			(12) // (COM1 ~ COM10) + USB_COM + CAN_BUS

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
#define CAN_BUS				(0x0B)
DMPAPI(COMPort *) com_Init(int com);
DMPAPI(void) com_Close(COMPort *port);

DMPAPI(bool) com_SetBPS(COMPort *port, unsigned long bps);
	#define COM_CAN_BPS_1000K   (1000000L)
	#define COM_CAN_BPS_833K    (833333L)
	#define COM_CAN_BPS_500K    (500000L)
	#define COM_CAN_BPS_250K    (250000L)
	#define COM_CAN_BPS_125K    (125000L)
	#define COM_CAN_BPS_100K    (100000L)
	#define COM_CAN_BPS_83K3    (83333L)
	#define COM_CAN_BPS_50K     (50000L)
	#define COM_CAN_BPS_20K     (20000L)
	#define COM_CAN_BPS_10K     (10000L)
DMPAPI(void) com_SetTimeOut(COMPort *port, unsigned long timeout);
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
#define FIFO_001		   (1)
#define FIFO_016		   (16)
#define FIFO_032		   (32)
#define FIFO_128		   (128)
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

/* only for USB device */
DMPAPI(bool) com_Ready(COMPort *port);
DMPAPI(Line_Coding *) com_GetLineCoding(COMPort *port);
	// typedef struct {
	//	   DWORD dwDTERate;
	//	   BYTE  bCharFormat;
	//	   BYTE  bParityType;
	//	   BYTE  bDataBits;
	// } Line_Coding;
DMPAPI(void) com_SetSerialState(COMPort *port, unsigned short state);
	//  Serial State Bitmap
	//    7 - 15: reserved
	//    6:  bOverRun    overrun error
	//    5:  bParity     parity error
	//    4:  bFraming    framing error
	//    3:  bRingSignal RI
	//    2:  bBreak      break reception
	//    1:  bTxCarrier  DSR
	//    0:  bRxCarrier  DCD
DMPAPI(unsigned short) com_GetControlLineState(COMPort *port);

/* only for CAN bus */
DMPAPI(bool) com_SetBitTime(COMPort *port, CAN_BitTime *bt);
DMPAPI(bool) com_SetEWLimit(COMPort *port, int ewl);
DMPAPI(bool) com_AddIDFilter(COMPort *port, bool ext_id, unsigned long filter, unsigned long mask);
DMPAPI(bool) com_DelIDFilter(COMPort *port, bool ext_id, unsigned long filter, unsigned long mask);
DMPAPI(void) com_ClearIDTable(COMPort *port);
DMPAPI(void) com_SetCANErrorHandler(COMPort *port, void (*func)(CAN_Bus *));
DMPAPI(void) com_SetCANBusOfHandler(COMPort *port, void (*func)(CAN_Bus *));
DMPAPI(unsigned char) com_GetSTAT(COMPort *port);
DMPAPI(unsigned char) com_GetERROR(COMPort *port);
DMPAPI(void) com_EnableBypass(COMPort *port);
DMPAPI(void) com_DisableBypass(COMPort *port);
DMPAPI(void) com_EnableStoreERROR(COMPort *port);
DMPAPI(void) com_DisableStoreERROR(COMPort *port);
DMPAPI(void) com_SetTxControl(COMPort *port, int format, unsigned long id);
DMPAPI(bool) com_GetRxControl(COMPort *port, int *format, unsigned long *id, int *len);

#ifdef __cplusplus
}
#endif

#endif
