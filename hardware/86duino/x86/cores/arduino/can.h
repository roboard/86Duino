/*
  can.h - DM&P Vortex86EX CAN Bus library
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

#ifndef __DMP_CAN_H
#define __DMP_CAN_H

#include "dmpcfg.h"
#include "vortex86.h"
#include "queue.h"

#if COM_LIB_TIMEOUT_DEBUG != 0
#define CAN_TIMEOUT_DEBUG  (1)
#else
#define CAN_TIMEOUT_DEBUG  (0)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	bool cksel;     // clock select: 0-> PCI clock, 1-> external input(20MHz)
	bool sam;       // sampling: 0-> once at sample point, 1-> three times prior to sample point
                    // ('0' = 'false', '1' = 'true')
					
	int ckdiv;      // clock divisor: 2, 4, 6, ..., 128
	int sjw;        // synchronization jump width: 1-4 Tq
	int prog;       // propagation segment: 1-4 Tq
	int ps1;        // phase segment 1: 1-4 Tq
	int ps2;        // phase segment 2: 1-4 Tq
	
} CAN_BitTime;

typedef struct
{
	bool          InUse;  // not for users
	
	bool          enable;
	bool          ext_id;
	unsigned long filter;
	unsigned long mask;
	
} CAN_Identifier;

typedef struct
{
	unsigned char type; 
	unsigned long id;
	unsigned long ldata;
	unsigned long hdata;

} CAN_Data;

typedef struct
{
	unsigned long  Err_CNT; // if this transceiver is happened, 'error_count' plus one.

	// offset
	unsigned short STAT;
	unsigned short TYPE;
	unsigned short IDR;
	unsigned short DATAL;
	unsigned short DATAH;
	
} CAN_Transceiver;

typedef volatile struct can_bus
{
	void             *handle;

	unsigned short    io_addr;          // CAN bus IO space address
	unsigned long     mmio_addr;        // CAN bus memory space address
	int               space;
	int               nIRQ;
	int               InUse;
	
	int               mode;             // 
	
	unsigned long     TimeOut;
	
	/* Bit-Time setting */
	CAN_BitTime       bt;
	unsigned long     nBPS;             // Bit-Time
	
	int               state;            // CAN bus status:
										//     Error warning
	                                    //     Active Error
										//     Passive Error
										//     Bus off
										
	unsigned char     error;            // last error type
	
	unsigned long     ArbiLost_CNT;
	unsigned long     Overrun_CNT;
										
	int               EWL;              // Error warning limit
	int               TEC;              // TX error count
	int               REC;              // RX error count
	
	int               TxFormat;
	int               RxFormat;
	unsigned long     TxID;
	unsigned long     RxID;
	
	unsigned char     RxBuffer[8];
	int               RxSize;
	int               RxCount;
	
	CAN_Transceiver   TX[3];
	CAN_Transceiver   RX;
	CAN_Identifier    ID[32];
	
	CAN_Data          rx_temp;
	CAN_Data          tx_temp;

	bool              ErrStoreEnable;
	Queue            *err;              // store errors have not been handled
	
	Queue            *rcvd;				// RX queue
	Queue            *xmit;				// TX queue
	
	void (*error_handler)(volatile struct can_bus *can);
	void (*busof_handler)(volatile struct can_bus *can);
	
	// -------------------------------- //
	//    CAN bus registers's offset    //
	// -------------------------------- //
	unsigned short GCR;				    // Global Control Register
	unsigned short CPS;				    // Clock Pre-Scaler
	unsigned short BTR;				    // Bus Timing
	unsigned short IER;				    // Interrupt Enable
	unsigned short ISR;					// Interrupr Status
	unsigned short GSR;					// Global Status
	unsigned short REQ;					// Request Register
	unsigned short EWR;					// Error Warning Limit Register
	unsigned short ECR;					// TX/RX Error Counter
	unsigned short IDI;					// Identifier Index
	unsigned short IDF;					// Identifier Filter
	unsigned short IDM;					// Identifier Mask
	 
} CAN_Bus;

DMPAPI(void *) CreateCANBus(int space);
DMPAPI(bool) can_Init(void *vcan, int mode);
DMPAPI(void) can_Close(void *vcan);
// space
#define PORTIO_SPACE       (0)
#define MMIO_SPACE         (1)
// mode
#define CAN_NONE           (0x00)
#define CAN_ID_BYPASS      (0x01)
#define CAN_POWER_SAVING   (0x02)

/**********************************************************
    Choose one function to set bit-rate:
	    1. 'can_SetBitTime': Users calculate Bit-Rate by
		                     themselves.
		2. 'can_SetBPS': Using Bit-Rate Table.
**********************************************************/
DMPAPI(bool) can_SetBitTime(void *vcan, CAN_BitTime *bt);
DMPAPI(bool) can_SetBPS(void *vcan, unsigned long bps);
// bps(for external clock)
#define CAN_BPS_1000K   (1000000L)
#define CAN_BPS_833K    (833333L)
#define CAN_BPS_500K    (500000L)
#define CAN_BPS_250K    (250000L)
#define CAN_BPS_125K    (125000L)
#define CAN_BPS_100K    (100000L)
#define CAN_BPS_83K3    (83333L)
#define CAN_BPS_50K     (50000L)
#define CAN_BPS_20K     (20000L)
#define CAN_BPS_10K     (10000L)
DMPAPI(void) can_SetTimeOut(void *vcan, unsigned long timeout);
#define CAN_NO_TIMEOUT         (-1)
DMPAPI(bool) can_SetEWLimit(void *vcan, int ewl);
DMPAPI(bool) can_AddIDFilter(void *vcan, bool ext_id, unsigned long filter, unsigned long mask);
DMPAPI(bool) can_DelIDFilter(void *vcan, bool ext_id, unsigned long filter, unsigned long mask);
DMPAPI(void) can_ClearIDTable(void *vcan);
DMPAPI(void) can_EnableBypass(void *vcan);
DMPAPI(void) can_DisableBypass(void *vcan);

DMPAPI(unsigned char) can_GetSTAT(void *vcan);
#define CAN_STAT_ACTIVE     (0)
#define CAN_STAT_WARNING    (1)
#define CAN_STAT_PASSIVE    (2)
#define CAN_STAT_BUS_OFF    (3)
DMPAPI(void) can_EnableStoreERROR(void *vcan);
DMPAPI(void) can_DisableStoreERROR(void *vcan);
DMPAPI(unsigned char) can_GetERROR(void *vcan);
#define CAN_ERROR_NONE      (0x00)
// D7, D6: 00->RX, 01->TX0, 10->TX1, 11->TX2
#define CAN_ERROR_RX        (0x00)
#define CAN_ERROR_TX0       (0x40)
#define CAN_ERROR_TX1       (0x80)
#define CAN_ERROR_TX2       (0xC0)
// D2, D1, D0: 001->bit error,  010->stuff error, 011->CRC error
//             100->form error, 101->ACK error
#define CAN_ERROR_BIT       (0x01)
#define CAN_ERROR_STUFF     (0x02)
#define CAN_ERROR_CRC       (0x03)
#define CAN_ERROR_FORM      (0x04)
#define CAN_ERROR_ACK       (0x05)

/**********************************************************
   'buf' must be 13 bytes, and 'bsize' should be 13

   'buf' format:
       [Type] [ID] [Data 0] [Data 1] ... [Data 7]
 ---------------------------------------------------------	   
	   
	   [Type]: (1-byte)
	   
	           bit0: 0 -> Standard format(11-bit ID)
			         1 -> Extended format(29-bit ID)
					 
			   bit1: 0 -> data frame
			         1 -> remote frame
			   
			   bit4-7: 0000-0111 = 0-7 bytes
			           1xxx = 8 bytes
					   
	   [ID]: (4-byte)
	   
	         if [Type].bit0 = 0, [ID].bit11-28 will be ignored
			 
	   [Data 0]: (1-byte)
	   ...
	   ...
	   
	Note: If [Type].bit4-7 = x, [Data x] - [Data 7] will
	
	      be ignored.
		  
**********************************************************/
DMPAPI(void) can_SetTxControl(void *vcan, int format, unsigned long id);
// format
#define CAN_STD_DATA_FRAME    (0x00)
#define CAN_EXT_DATA_FRAME    (0x01)
#define CAN_STD_REMOTE_FRAME  (0x02)
#define CAN_EXT_REMOTE_FRAME  (0x03)
DMPAPI(bool) can_GetRxControl(void *vcan, int *format, unsigned long *id, int *len);

DMPAPI(unsigned int) can_Read(void *vcan);
DMPAPI(int)  can_Receive(void *vcan, unsigned char* buf, int bsize); // read one frame
DMPAPI(int)  can_QueryRxQueue(void *vcan);
DMPAPI(bool) can_RxQueueFull(void *vcan);
DMPAPI(bool) can_RxQueueEmpty(void *vcan);
DMPAPI(void) can_FlushRxQueue(void *vcan);

DMPAPI(bool) can_Write(void *vcan, unsigned char val);
DMPAPI(int)  can_Send(void *vcan, unsigned char* buf, int bsize); // write one frame
DMPAPI(int)  can_QueryTxQueue(void *vcan);
DMPAPI(bool) can_TxQueueFull(void *vcan);
DMPAPI(bool) can_TxQueueEmpty(void *vcan);
DMPAPI(void) can_FlushTxQueue(void *vcan);
DMPAPI(bool) can_TxReady(void *vcan);
DMPAPI(void) can_FlushWFIFO(void *vcan);

DMPAPI(void) can_SetCANErrorHandler(void *vcan, void (*func)(CAN_Bus *));
DMPAPI(void) can_SetCANBusOfHandler(void *vcan, void (*func)(CAN_Bus *));

#ifdef __cplusplus
}
#endif

#endif
