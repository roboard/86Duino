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


// CANFrame type
#define STD_DATA_FRAME     (0x00)
#define EXT_DATA_FRAME     (0x01)
#define STD_REMOTE_FRAME   (0x02) 
#define EXT_REMOTE_FRAME   (0x03)

typedef struct {
	int type;
	int length;
	unsigned long identifier;
	
	union {
		unsigned char byte[8];
		unsigned long dword[2];
	} Data;

} CANFrame;

/* CAN-BUS transceiver registers offset */
typedef struct {
	
	unsigned short STAT;                /* Status register         */
	unsigned short TYPE;                /* Package type register   */
	unsigned short IDR;                 /* Identifier register     */
	unsigned short DATAL;               /* Low-side data register  */
	unsigned short DATAH;               /* High-side data register */
	
} CAN_Transceiver;

typedef volatile struct can_bus
{
	void          *ioHandle;
	bool           InUse;

	int            nIRQ;
	int            IO_Space;
	unsigned short IO_Addr;             /* CAN bus IO space address */
	unsigned long  MMIO_Addr;           /* CAN bus memory space address */
	
	unsigned long  RxTimeOut;
	unsigned long  TxTimeOut;
	unsigned long  BitTime;             /* Bit-Time */
	
	int            NowState;            /* CAN bus now state */

	unsigned char  LastError;           /* last error type */

	bool           StoreError;
	Queue         *Error;               /* store errors have not been handled */
	
	Queue         *rcvd;                /* RX queue */
	Queue         *xmit;                /* TX queue */
	
	int            round;

	void (*busoff_handler)(volatile struct can_bus *can);
	
	/* -------------------------------- */
	/*    CAN bus registers's offset    */
	/* -------------------------------- */
	unsigned short  GCR;				/* Global Control Register      */
	unsigned short  CPS;				/* Clock Pre-Scaler             */
	unsigned short  BTR;			    /* Bus Timing                   */
	unsigned short  IER;				/* Interrupt Enable             */
	unsigned short  ISR;				/* Interrupr Status             */
	unsigned short  GSR;				/* Global Status                */
	unsigned short  REQ;				/* Request Register             */
	unsigned short  EWR;				/* Error Warning Limit Register */
	unsigned short  ECR;				/* TX/RX Error Counter          */
	unsigned short  IDI;				/* Identifier Index             */
	unsigned short  IDF;				/* Identifier Filter            */
	unsigned short  IDM;				/* Identifier Mask              */
	CAN_Transceiver TX[3];              /* Tx registers                 */
	CAN_Transceiver RX;                 /* Rx registers                 */
	 
} CAN_Bus;

DMPAPI(void*) CreateCANBus(int IO_Space);
#define IO_PORT       (0)
#define IO_MEMORY     (1)

DMPAPI(bool) can_Init(void *vcan);
DMPAPI(void) can_Close(void *vcan);
DMPAPI(void) can_Reset(void *vcan);

DMPAPI(bool) can_SetBPS(void *vcan, unsigned long bps);
#define CAN_BPS_1000K   (0x80021144UL)
#define CAN_BPS_833K    (0x80021155UL)
#define CAN_BPS_500K    (0x80041144UL)
#define CAN_BPS_250K    (0x80081144UL)
#define CAN_BPS_125K    (0x80101144UL)
#define CAN_BPS_100K    (0x80141144UL)
#define CAN_BPS_83K3    (0x80181144UL)
#define CAN_BPS_50K     (0x80281144UL)
#define CAN_BPS_20K     (0x80641144UL)
#define CAN_BPS_10K     (0x80501888UL)

DMPAPI(void) can_SetTimeOut(void *vcan, unsigned long rx_timeout, unsigned long tx_timeout);
#define CAN_NO_TIMEOUT  (-1)

/* CAN-BUS Identifier list has 32 filter. Note: index = 0 ~ 31. */
DMPAPI(bool) can_AddIDFilter(void *vcan, int index, int  ext_id, unsigned long  filter, unsigned long  mask);
DMPAPI(bool) can_GetIDFilter(void *vcan, int index, int *ext_id, unsigned long *filter, unsigned long *mask);
DMPAPI(bool) can_DelIDFilter(void *vcan, int index);
DMPAPI(void) can_ClearIDList(void *vcan);
DMPAPI(void) can_EnableBypass(void *vcan);
DMPAPI(void) can_DisableBypass(void *vcan);

DMPAPI(bool) can_SetEWLimit(void *vcan, int ewl);
DMPAPI(int)  can_GetEWLimit(void *vcan);

DMPAPI(int) can_GetTxErrorCount(void *vcan);
DMPAPI(int) can_GetRxErrorCount(void *vcan);

DMPAPI(unsigned char) can_GetNowState(void *vcan);
#define CAN_STAT_WARNING    (0x80)
#define CAN_STAT_ACTIVE     (0x00)
#define CAN_STAT_PASSIVE    (0x01)
#define CAN_STAT_BUS_OFF    (0x02)
DMPAPI(unsigned char) can_PopError(void *vcan);
DMPAPI(unsigned char) can_GetLastError(void *vcan);
#define CAN_ERROR_NONE      (0x00)
/* D7, D6: 00->RX,
           01->TX0,
		   10->TX1,
		   11->TX2  */
#define CAN_ERROR_RX        (0x00)
#define CAN_ERROR_TX0       (0x40)
#define CAN_ERROR_TX1       (0x80)
#define CAN_ERROR_TX2       (0xC0)
/* D2, D1, D0: 001 -> bit   error,
               010 -> stuff error,
               011 -> CRC   error,
			   100 -> form  error,
			   101 -> ACK   error  */
#define CAN_ERROR_BIT       (0x01)
#define CAN_ERROR_STUFF     (0x02)
#define CAN_ERROR_CRC       (0x03)
#define CAN_ERROR_FORM      (0x04)
#define CAN_ERROR_ACK       (0x05)

DMPAPI(void) can_EnableStoreError(void *vcan);
DMPAPI(void) can_DisableStoreError(void *vcan);

DMPAPI(bool) can_Read(void *vcan, CANFrame *pack);
DMPAPI(int)  can_QueryRxQueue(void *vcan);
DMPAPI(bool) can_RxQueueFull(void *vcan);
DMPAPI(bool) can_RxQueueEmpty(void *vcan);
DMPAPI(void) can_FlushRxQueue(void *vcan);

DMPAPI(bool) can_Write(void *vcan, CANFrame *pack);
DMPAPI(int)  can_QueryTxQueue(void *vcan);
DMPAPI(bool) can_TxQueueFull(void *vcan);
DMPAPI(bool) can_TxQueueEmpty(void *vcan);
DMPAPI(void) can_FlushTxQueue(void *vcan);
DMPAPI(bool) can_TxReady(void *vcan);
DMPAPI(void) can_FlushWFIFO(void *vcan);

DMPAPI(void) can_SetCANBusOffHandler(void *vcan, void (*func)(CAN_Bus *));

#ifdef __cplusplus
}
#endif

#endif
