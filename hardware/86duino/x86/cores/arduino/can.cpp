/*
  can.cpp - DM&P Vortex86EX CAN Bus library
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

#define __DMP_CAN_LIB

#include "can.h"
#define  USE_COMMON
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "io.h"
#include "irq.h"
#include "err.h"

#define CAN_DATA_SIZE       (13)

#define CAN_ERR_QUEUE_SIZE  (512)
#define CAN_RX_QUEUE_SIZE   (512)
#define CAN_TX_QUEUE_SIZE   (256)

/* Interrupt Flag */
#define CAN_ALL_INT         (0x07FFL)
#define CAN_XPI             (0x0400L)
#define CAN_RBOI            (0x0200L)
#define CAN_RBEI            (0x0100L)
#define CAN_ALI             (0x0080L)
#define CAN_BOI             (0x0040L)
#define CAN_EPI             (0x0020L)
#define CAN_ECI             (0x0010L)
#define CAN_TX2I            (0x0008L)
#define CAN_TX1I            (0x0004L)
#define CAN_TX0I            (0x0002L)
#define CAN_RXI             (0x0001L)

// TODO: bit time table
static CAN_BitTime bt_table[10] = {
	{true, false,   2, 1, 1, 4, 4}, // CAN_BPS_1000K
	{true, false,   2, 1, 1, 5, 5}, // CAN_BPS_833K
	{true, false,   4, 1, 1, 4, 4}, // CAN_BPS_500K
	{true, false,   8, 1, 1, 4, 4}, // CAN_BPS_250K
	{true, false,  16, 1, 1, 4, 4}, // CAN_BPS_125K
	{true, false,  20, 1, 1, 4, 4}, // CAN_BPS_100K
	{true, false,  24, 1, 1, 4, 4}, // CAN_BPS_83K3
	{true, false,  40, 1, 1, 4, 4}, // CAN_BPS_50K
	{true, false, 100, 1, 1, 4, 4}, // CAN_BPS_20K
	{true, false,  80, 1, 8, 8, 8}  // CAN_BPS_10K
};

static int round = 0;

DMP_INLINE(void) can_Reset(CAN_Bus *can) // does not reset ID filter table
{
	int i;
	unsigned long gcr;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return; }
	if (can->InUse != 1) { err_print((char*)"%s: CAN bus is not in use.\n", __FUNCTION__); return; }

	/* Software reset */
	io_Out32(can->handle, can->GCR, io_In32(can->handle, can->GCR) | 0x01L);    // reset CAN registers
	while (io_In32(can->handle, can->GCR) & 0x01L);
	
	can->state        = CAN_STAT_ACTIVE;
	can->error        = CAN_ERROR_NONE;
	
	can->ArbiLost_CNT = 0;
	can->Overrun_CNT  = 0;
	
	can->TEC       = 0;
	can->REC       = 0;
	
	for (i = 0; i < 3; i++)
	memset((void*)&can->TX[i], 0, sizeof(can->TX[i]));
	memset((void*)&can->RX   , 0, sizeof(can->RX)   );
	
	can_FlushRxQueue((void *)can);
	can_FlushTxQueue((void *)can);
	
	io_DisableINT();
	{
		io_Out32(can->handle, can->IER, CAN_ALL_INT);
	}
	io_RestoreINT();
	
	gcr = io_In32(can->handle, can->GCR);
	if (can->mode & CAN_ID_BYPASS)    gcr |= 0x0004000L;
	if (can->mode & CAN_POWER_SAVING) gcr |= 0x0100000L;
	io_Out32(can->handle, can->GCR, gcr |= 0x20382L); // Error retry, Arbitration lost retry, Round Robin
	                                                  // Idenfitier filter enable
	while (io_In32(can->handle, can->GCR) != gcr);
}

DMP_INLINE(void) can_RoundRobin(CAN_Bus *can)
{
	
	if (QueueEmpty(can->xmit)) return;
	
	io_DisableINT();
	{
		if (!(io_In32(can->handle, can->REQ) & (0x01L << (round*2))))
		{
			PopBufQueue(can->xmit, (void*)&can->tx_temp);
			
			io_Out32(can->handle, can->TX[round].TYPE , (unsigned long)can->tx_temp.type);
			io_Out32(can->handle, can->TX[round].IDR  , can->tx_temp.id);
			io_Out32(can->handle, can->TX[round].DATAL, can->tx_temp.ldata);
			io_Out32(can->handle, can->TX[round].DATAH, can->tx_temp.hdata);
			
			io_Out32(can->handle, can->REQ, io_In32(can->handle, can->REQ) | (0x01L << (round*2)));
			
			round++;
			if (round == 3) round = 0;
		}
	}
	io_RestoreINT();
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(can_RoundRobin)
#endif

static int CAN_ISR(int irq, void *data)
{
	unsigned long isr;
	CAN_Bus *can = (CAN_Bus *)data;
	
	if (((isr = io_In32(can->handle, can->ISR)) & 0x07FFL) != 0x00L)
	{
		if (isr & CAN_RXI)
		{
			io_DisableINT();
			{
				can->rx_temp.type  = (unsigned char)io_In32(can->handle, can->RX.TYPE);
				can->rx_temp.id    = io_In32(can->handle, can->RX.IDR);
				can->rx_temp.hdata = io_In32(can->handle, can->RX.DATAH);
				can->rx_temp.ldata = io_In32(can->handle, can->RX.DATAL);
				PushBufQueue(can->rcvd, (void*)&can->rx_temp);
			}
			io_RestoreINT();
			
			io_Out32(can->handle, can->REQ, 0x0100L); // release the received message
			io_Out32(can->handle, can->ISR, CAN_RXI);
		}
		
		if (isr & CAN_TX0I)
		{
			unsigned long stat;
			
			stat = io_In32(can->handle, can->TX[0].STAT);
			if (stat & 0x20L)
			{
				can->TX[0].Err_CNT++;
				can->error = CAN_ERROR_TX0 + (unsigned char)((stat & 0x00070000L) >> 16);
				if (can->ErrStoreEnable == true)
					PushQueue(can->err, can->error);
				if (can->error_handler != NULL) can->error_handler(can);
			}
			if ((stat & 0x07L) == 0x05L) can_RoundRobin(can);
			
			io_Out32(can->handle, can->ISR, CAN_TX0I);
		}
		
		if (isr & CAN_TX1I)
		{
			unsigned long stat;
			
			stat = io_In32(can->handle, can->TX[1].STAT);
			if (stat & 0x20L)
			{
				can->TX[1].Err_CNT++;
				can->error = CAN_ERROR_TX1 + (unsigned char)((stat & 0x00070000L) >> 16);
				if (can->ErrStoreEnable == true)
					PushQueue(can->err, can->error);
				if (can->error_handler != NULL) can->error_handler(can);
			}
			if ((stat & 0x07L) == 0x05L) can_RoundRobin(can);
			
			io_Out32(can->handle, can->ISR, CAN_TX1I);
		}
		
		if (isr & CAN_TX2I)
		{
			unsigned long stat;
			
			stat = io_In32(can->handle, can->TX[2].STAT);
			if (stat & 0x20L)
			{
				can->TX[2].Err_CNT++;
				can->error = CAN_ERROR_TX2 + (unsigned char)((stat & 0x00070000L) >> 16);
			if (can->ErrStoreEnable == true)
					PushQueue(can->err, can->error);
				if (can->error_handler != NULL) can->error_handler(can);
			}
			if ((stat & 0x07L) == 0x05L) can_RoundRobin(can);
			
			io_Out32(can->handle, can->ISR, CAN_TX2I);
		}
		
		if (isr & CAN_ECI)
		{
			can->state = CAN_STAT_WARNING;
			io_Out32(can->handle, can->ISR, CAN_ECI);
		}
		
		if (isr & CAN_EPI)
		{
			can->state = CAN_STAT_PASSIVE;
			io_Out32(can->handle, can->ISR, CAN_EPI);
		}
		
		if (isr & CAN_BOI)
		{
			can->state = CAN_STAT_BUS_OFF;
			io_Out32(can->handle, can->ISR, CAN_BOI);
		}
		
		if (isr & CAN_ALI)
		{
			can->ArbiLost_CNT++;
			io_Out32(can->handle, can->ISR, CAN_ALI);
		}
		
		if (isr & CAN_RBEI)
		{
			unsigned long stat;
			
			stat = io_In32(can->handle, can->RX.STAT);
			can->RX.Err_CNT++;
			can->error = CAN_ERROR_RX + (unsigned char)((stat & 0x70L) >> 4);
			if (can->ErrStoreEnable == true)
				PushQueue(can->err, can->error);
			if (can->error_handler != NULL) can->error_handler(can);
			
			io_Out32(can->handle, can->ISR, CAN_RBEI);
		}
		
		if (isr & CAN_RBOI)
		{
			can->Overrun_CNT++;
			io_Out32(can->handle, can->ISR, CAN_RBOI);
		}
		
		if (isr & CAN_XPI)
		{
			io_Out32(can->handle, can->ISR, CAN_XPI);
		}
	}
	else
		return ISR_NONE;

	return ISR_HANDLED;
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(CAN_ISR)
#endif

DMPAPI(void *) CreateCANBus(int space)
{
	int i;
	CAN_Bus *can;
	
	if (space != PORTIO_SPACE && space != MMIO_SPACE)
		return NULL;
	
	if ((can = (CAN_Bus *)ker_Malloc(sizeof(CAN_Bus))) == NULL)
	{
		err_print((char*)"%s: CAN bus malloc error.\n", __FUNCTION__);
		goto CAN_MALLOC_FAIL;
	}
	
	if (io_Init() == false)
	{
		err_print((char*)"%s: Init IO lib error!!\n", __FUNCTION__);
		goto IO_INIT_FAIL;
	}

	can->space        = space;
	can->io_addr      = vx86_GetCANBusIOAddr();
	can->mmio_addr    = vx86_GetCANBusMMIOAddr();
	can->nIRQ         = vx86_GetCANBusIRQ();
	if (can->nIRQ == 0)
		goto WITHOUT_CANBUS_FAIL;
	if (can->space == PORTIO_SPACE && can->io_addr == 0x0000)
		goto WITHOUT_CANBUS_FAIL;
	if (can->space == MMIO_SPACE && can->mmio_addr == 0x00UL)
		goto WITHOUT_CANBUS_FAIL;
		
	can->InUse        = 0;
	can->mode         = CAN_NONE;
	can->TimeOut      = CAN_NO_TIMEOUT;
	
	can->bt.cksel     = true;
	can->bt.sam       = false;
	can->bt.ckdiv     = 2;
	can->bt.sjw       = 1;
	can->bt.prog      = 1;
	can->bt.ps1       = 4;
	can->bt.ps2       = 4;
	can->nBPS         = CAN_BPS_1000K;
	
	can->state        = CAN_STAT_ACTIVE;
	can->error        = CAN_ERROR_NONE;
	
	can->ArbiLost_CNT = 0;
	can->Overrun_CNT  = 0;
	
	can->EWL          = 96;		
	can->TEC          = 0;
	can->REC          = 0;
	
	can->ErrStoreEnable = false;
	can->TxFormat       = CAN_STD_DATA_FRAME;
	can->RxFormat       = CAN_STD_DATA_FRAME;
	can->TxID           = 0x00L;
	can->RxID           = 0x00L;
	can->RxSize         = 0;
	
	for (i = 0; i < 3; i++)
	memset((void*)&can->TX[i], 0, sizeof(can->TX[i]));
	memset((void*)&can->RX   , 0, sizeof(can->RX)   );
	for (i = 0; i < 32; i++)
	memset((void*)&can->ID[i], 0, sizeof(can->ID[i]));

	if ((can->err  = CreateQueue(CAN_ERR_QUEUE_SIZE)) == NULL) goto CAN_ERR_QUEUE_FAIL;
	if ((can->rcvd = CreateBufQueue(CAN_RX_QUEUE_SIZE, sizeof(CAN_Data)) ) == NULL) goto CAN_RX_QUEUE_FAIL;
	if ((can->xmit = CreateBufQueue(CAN_TX_QUEUE_SIZE, sizeof(CAN_Data)) ) == NULL) goto CAN_TX_QUEUE_FAIL;
	
	can->error_handler = NULL;
	can->busof_handler = NULL;
	
	/* CAN registers offset */
	can->GCR         = 0x00;
	can->CPS         = 0x04;
	can->BTR         = 0x08;
	can->IER         = 0x0C;
	can->ISR         = 0x10;
	can->GSR         = 0x14;
	can->REQ         = 0x18;
	can->EWR         = 0x2C;
	can->ECR         = 0x30;
	can->IDI         = 0x34;
	can->IDF         = 0x38;
	can->IDM         = 0x3C;
	can->RX.STAT     = 0x28;
	can->RX.TYPE     = 0x70;
	can->RX.IDR      = 0x74;
	can->RX.DATAL    = 0x78;
	can->RX.DATAH    = 0x7C;
	can->TX[0].STAT  = 0x1C;
	can->TX[0].TYPE  = 0x40;
	can->TX[0].IDR   = 0x44;
	can->TX[0].DATAL = 0x48;
	can->TX[0].DATAH = 0x4C;
	can->TX[1].STAT  = 0x20;
	can->TX[1].TYPE  = 0x50;
	can->TX[1].IDR   = 0x54;
	can->TX[1].DATAL = 0x58;
	can->TX[1].DATAH = 0x5C;
	can->TX[2].STAT  = 0x24;
	can->TX[2].TYPE  = 0x60;
	can->TX[2].IDR   = 0x64;
	can->TX[2].DATAL = 0x68;
	can->TX[2].DATAH = 0x6C;
	
	return (void *)can;
	
CAN_TX_QUEUE_FAIL:
	DestoryQueue(can->rcvd);
CAN_RX_QUEUE_FAIL:
	DestoryQueue(can->err);
CAN_ERR_QUEUE_FAIL:
WITHOUT_CANBUS_FAIL:
	io_Close();
IO_INIT_FAIL:
	ker_Mfree((void*)can);
CAN_MALLOC_FAIL:
	return NULL;
}

DMPAPI(bool) can_Init(void *vcan, int mode)
{
	int io_space;
	unsigned long addr, gcr;
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return false; }
	
	#if defined DMP_DOS_DJGPP
	static bool locked = false;
	#endif
	
	if (can->InUse == 1) return true;
	
	if (irq_Init() == false) {
        err_print((char*)"%s: IRQ init fail.\n", __FUNCTION__);
        goto IRQ_INIT_FAIL;
    }
	
	#if defined DMP_DOS_DJGPP
	if (locked == false)
	{
		DPMI_LOCK_FUNC(CAN_ISR);
		DPMI_LOCK_FUNC(can_RoundRobin);
		DPMI_LOCK_VAR(round);
		locked = true;
	}
	#endif
	
	switch (can->space)
	{
		case PORTIO_SPACE:
		{
			io_space = IO_USE_PORTIO;
			addr = (unsigned long)can->io_addr;
		}
		break;
		
		case MMIO_SPACE:
		{
			io_space = IO_USE_MMIO;
			addr = can->mmio_addr;
		}
		break;
		
		default: 
		{
			err_print((char*)"%s: \'space\' error.\n", __FUNCTION__);
			goto SPACE_FAIL;
		}
	};
	
	if ((can->handle = io_Alloc(io_space, addr, 0xff)) == NULL)
	{
		err_print((char*)"%s: IO alloc error.\n", __FUNCTION__);
		goto IO_ALLOC_FAIL;
	}
	can->InUse = 1;
	
	/* Software reset */
	io_Out32(can->handle, can->GCR, io_In32(can->handle, can->GCR) | 0x01L);    // reset CAN registers
	while (io_In32(can->handle, can->GCR) & 0x01L);
	io_Out32(can->handle, can->GCR, io_In32(can->handle, can->GCR) | 0x10000L); // reset ID filter table
	while (io_In32(can->handle, can->GCR) & 0x10000L);
	
	/* Set Bit Time */
	can_SetBitTime(vcan, (CAN_BitTime*)&can->bt);
	
	/* Enable Interrupt */
	io_DisableINT();
	{
		irq_Setting(can->nIRQ, IRQ_LEVEL_TRIGGER);
		irq_InstallISR(can->nIRQ, CAN_ISR, (void *)can);
		io_Out32(can->handle, can->IER, CAN_ALL_INT);
	}
	io_RestoreINT();
	
	can->mode = mode;
	gcr = io_In32(can->handle, can->GCR);
	if (can->mode & CAN_ID_BYPASS)    gcr |= 0x0004000L;
	if (can->mode & CAN_POWER_SAVING) gcr |= 0x0100000L;
	io_Out32(can->handle, can->GCR, gcr |= 0x20382L); // Error retry, Arbitration lost retry, Round Robin
	                                                 // Idenfitier filter enable
	while (io_In32(can->handle, can->GCR) != gcr);
	
	can->state  = CAN_STAT_ACTIVE;
	
	return true;

IO_ALLOC_FAIL:	
SPACE_FAIL:
	irq_Close();
IRQ_INIT_FAIL:
	io_Close();
	return false;
}

DMPAPI(void) can_Close(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return; }
	
	if (can->InUse != 0)
	{
		io_DisableINT();
		{
			io_Out32(can->handle, can->GCR, io_In32(can->handle, can->GCR) | 0x01L);    // reset CAN registers
			while (io_In32(can->handle, can->GCR) & 0x01L);
			io_Out32(can->handle, can->GCR, io_In32(can->handle, can->GCR) | 0x10000L); // reset ID filter table
			while (io_In32(can->handle, can->GCR) & 0x10000L);
			irq_UninstallISR(can->nIRQ, (void *)can);
		}
		io_RestoreINT();
		
		io_Free(can->handle);
		irq_Close();
		if (io_Close() == false) err_print((char*)"%s: Close IO lib error!!\n", __FUNCTION__);

		can->state  = CAN_STAT_ACTIVE;
		can->error  = CAN_ERROR_NONE;
		can->TEC    = 0;
		can->REC    = 0;
		can->InUse  = 0;
	}
	DestoryQueue(can->err);
	DestoryQueue(can->rcvd);
	DestoryQueue(can->xmit);
	ker_Mfree((void *)can);
}

DMPAPI(bool) can_SetBitTime(void *vcan, CAN_BitTime *bt)
{
	unsigned long val, gcr;
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return false; }
	if (can->InUse != 1) { err_print((char*)"%s: CAN bus is not in use.\n", __FUNCTION__); return false; }
	
	if (bt->ckdiv > 128 || bt->ckdiv < 2 || (bt->ckdiv%2) != 0) { err_print((char*)"%s: 'ckdiv' error.\n", __FUNCTION__); return false;}
	if (bt->sjw   > 4   || bt->sjw   < 0) { err_print((char*)"%s: 'sjw' error.\n", __FUNCTION__); return false;}
	if (bt->prog  > 8   || bt->prog  < 0) { err_print((char*)"%s: 'prog' error.\n", __FUNCTION__); return false;}
	if (bt->ps1   > 8   || bt->ps1   < 0) { err_print((char*)"%s: 'ps1' error.\n", __FUNCTION__); return false;}
	if (bt->ps2   > 8   || bt->ps2   < 0) { err_print((char*)"%s: 'ps2' error.\n", __FUNCTION__); return false;}
	
	can->bt.cksel = bt->cksel;
	can->bt.sam   = bt->sam;
	can->bt.ckdiv = bt->ckdiv;
	can->bt.sjw   = bt->sjw;
	can->bt.prog  = bt->prog;
	can->bt.ps1   = bt->ps1;
	can->bt.ps2   = bt->ps2;
	
	gcr = io_In32(can->handle, can->GCR);
	io_Out32(can->handle, can->GCR, gcr & 0xFFFFFFFDL);
	while (io_In32(can->handle, can->GCR) & 0x00000002L);
	{
		/* Set Clock Pre-Scaler */
		val  = (can->bt.cksel == true) ? (0x80000000L) : (0x00000000L);
		val |= (unsigned long)(((can->bt.ckdiv >> 1) - 1) & 0x3F);
		io_Out32(can->handle, can->CPS, val);
		
		/* Set Bus Timing */
		val  = (can->bt.sam == true) ? (0x00008000L) : (0x00000000L);
		val |= (unsigned long)((can->bt.sjw  - 1) & 0x03) << 12;
		val |= (unsigned long)((can->bt.ps2  - 1) & 0x07) << 8;
		val |= (unsigned long)((can->bt.ps1  - 1) & 0x07) << 4;
		val |= (unsigned long)((can->bt.prog - 1) & 0x07);
		io_Out32(can->handle, can->BTR, val); while (io_In32(can->handle, can->BTR) != val);
	}
	io_Out32(can->handle, can->GCR, gcr);//io_In32(can->handle, can->GCR) | 0x00000002L);
	while (io_In32(can->handle, can->GCR) != gcr);
	
	return true;
}

DMPAPI(bool) can_SetBPS(void *vcan, unsigned long bps)
{
	switch (bps)
	{
		case CAN_BPS_1000K: return can_SetBitTime(vcan, &bt_table[0]);
		case CAN_BPS_833K:  return can_SetBitTime(vcan, &bt_table[1]);
		case CAN_BPS_500K:  return can_SetBitTime(vcan, &bt_table[2]);
		case CAN_BPS_250K:  return can_SetBitTime(vcan, &bt_table[3]);
		case CAN_BPS_125K:  return can_SetBitTime(vcan, &bt_table[4]);
		case CAN_BPS_100K:  return can_SetBitTime(vcan, &bt_table[5]);
		case CAN_BPS_83K3:  return can_SetBitTime(vcan, &bt_table[6]);
		case CAN_BPS_50K:   return can_SetBitTime(vcan, &bt_table[7]);
		case CAN_BPS_20K:   return can_SetBitTime(vcan, &bt_table[8]);
		case CAN_BPS_10K:   return can_SetBitTime(vcan, &bt_table[9]);
		default: return false;
	};
}

DMPAPI(void) can_SetTimeOut(void *vcan, unsigned long timeout)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return; }
	
	can->TimeOut = timeout;
}

DMPAPI(bool) can_SetEWLimit(void *vcan, int ewl)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return false; }
	if (can->InUse != 1) { err_print((char*)"%s: CAN bus is not in use.\n", __FUNCTION__); return false; }
	
	if (ewl > 255 || ewl < 0) { err_print((char*)"%s: 'ewl' error.\n", __FUNCTION__); return false;}
	
	can->EWL = ewl;
	
	io_Out32(can->handle, can->GCR, io_In32(can->handle, can->GCR) & 0xFFFFFFFDL);
	{
		io_Out32(can->handle, can->EWR, (unsigned long)can->EWL);
	}
	io_Out32(can->handle, can->GCR, io_In32(can->handle, can->GCR) | 0x00000002L);
	
	return true;
}

DMPAPI(bool) can_AddIDFilter(void *vcan, bool ext_id, unsigned long filter, unsigned long mask)
{
	int i, size;
	unsigned long val;
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return false; }
	if (can->InUse != 1) { err_print((char*)"%s: CAN bus is not in use.\n", __FUNCTION__); return false; }
	
	if ((ext_id == true && (filter & 0xE0000000L)) || (ext_id == false && (filter & 0xFFFFF800L)))
	{
		err_print((char*)"%s: 'filter' error.\n", __FUNCTION__);
		return false;
	}
	if ((ext_id == true && (mask & 0xE0000000L)) || (ext_id == false && (mask & 0xFFFFF800L)))
	{
		err_print((char*)"%s: 'mask' error.\n", __FUNCTION__);
		return false;
	}
	
	size = sizeof(can->ID)/sizeof(can->ID[0]);
	
	for (i = 0; i < size; i++)
	{
		if (can->ID[i].InUse == false)
		{
			can->ID[i].ext_id = ext_id;
			can->ID[i].filter = filter;
			can->ID[i].mask   = mask;
			can->ID[i].enable = true;
			
			val = (can->ID[i].enable == true) ? (0x80000000L) : (0x00000000L);
			if (can->ID[i].ext_id == true) val |= 0x40000000L;
			val |= can->ID[i].filter;
			
			io_DisableINT();
			{
				/* Set ID filter */
				io_Out32(can->handle, can->IDF, val);
				
				/* Set ID mask */
				io_Out32(can->handle, can->IDM, can->ID[i].mask);
				
				/* Set ID index */
				io_Out32(can->handle, can->IDI, (unsigned long)i | 0x00000100L);
				
				can->ID[i].InUse = true;
			}
			io_RestoreINT();
			
			return true;
		}
	}
	
	return false;
}

DMPAPI(bool) can_DelIDFilter(void *vcan, bool ext_id, unsigned long filter, unsigned long mask)
{
	int i, size;
	bool del = false;
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return false; }
	if (can->InUse != 1) { err_print((char*)"%s: CAN bus is not in use.\n", __FUNCTION__); return false; }
	
	if ((ext_id == true && (filter & 0xE0000000L)) || (ext_id == false && (filter & 0xFFFFF800L)))
	{
		err_print((char*)"%s: 'filter' error.\n", __FUNCTION__);
		return false;
	}
	if ((ext_id == true && (mask & 0xE0000000L)) || (ext_id == false && (mask & 0xFFFFF800L)))
	{
		err_print((char*)"%s: 'mask' error.\n", __FUNCTION__);
		return false;
	}
	
	size = sizeof(can->ID)/sizeof(can->ID[0]);
	
	for (i = 0; i < size; i++)
	{
		if (can->ID[i].InUse  == true   && can->ID[i].ext_id == ext_id
		&&  can->ID[i].filter == filter && can->ID[i].mask   == mask)
		{
			io_DisableINT();
			{
				io_Out32(can->handle, can->IDI, (unsigned long)i);
				io_Out32(can->handle, can->IDF, io_In32(can->handle, can->IDF) & 0x7FFFFFFFL);
				io_Out32(can->handle, can->IDI, (unsigned long)i | 0x00000100L);
				can->ID[i].enable = false;
				can->ID[i].InUse = false;
			}
			io_RestoreINT();
			del = true;
		}
	}
	
	return del;
}

DMPAPI(void) can_ClearIDTable(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return; }
	if (can->InUse != 1) { err_print((char*)"%s: CAN bus is not in use.\n", __FUNCTION__); return; }
	
	io_Out32(can->handle, can->GCR, io_In32(can->handle, can->GCR) | 0x10000L); // reset ID filter table
	while (io_In32(can->handle, can->GCR) & 0x10000L);
}

DMPAPI(void) can_EnableBypass(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return; }
	if (can->InUse != 1) { err_print((char*)"%s: CAN bus is not in use.\n", __FUNCTION__); return; }
	io_Out32(can->handle, can->GCR, io_In32(can->handle, can->GCR) | 0x00040000L);
}

DMPAPI(void) can_DisableBypass(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return; }
	if (can->InUse != 1) { err_print((char*)"%s: CAN bus is not in use.\n", __FUNCTION__); return; }
	io_Out32(can->handle, can->GCR, io_In32(can->handle, can->GCR) & 0xFFFBFFFFL);
}

DMPAPI(unsigned char) can_GetSTAT(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return 0x00; }
	return can->state;
}

DMPAPI(void) can_EnableStoreERROR(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return; }
	
	ClearQueue(can->err);
	can->ErrStoreEnable = true;
}

DMPAPI(void) can_DisableStoreERROR(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return; }
	
	can->ErrStoreEnable = false;
	ClearQueue(can->err);
}

DMPAPI(unsigned char) can_GetERROR(void *vcan)
{
	unsigned char error;
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return 0x00; }
	
	if (can->ErrStoreEnable == false) {
		io_DisableINT();
		error = can->error;
		can->error = CAN_ERROR_NONE;
		io_RestoreINT();
	}
	else {
		if (!QueueEmpty(can->err))
			error = PopQueue(can->err);
		else
			error = CAN_ERROR_NONE;
	}
		
	return error;
}

DMPAPI(void) can_SetTxControl(void *vcan, int format, unsigned long id)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return; }
	
	io_DisableINT();
	switch (format)
	{
	case CAN_STD_DATA_FRAME:
	case CAN_EXT_DATA_FRAME :
	case CAN_STD_REMOTE_FRAME:
	case CAN_EXT_REMOTE_FRAME:
		can->TxFormat = format;
		break;
	default:
		break;
	}
	can->TxID = id;
	io_RestoreINT();
}

DMPAPI(bool) can_GetRxControl(void *vcan, int *format, unsigned long *id, int *len)
{
	bool Status = false;
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return false; }
	
	if (can->RxSize - can->RxCount > 0) {
		*format = can->RxFormat;
		*id = can->RxID;
		*len = can->RxSize - can->RxCount;
		Status = true;
	}
	else if (!QueueEmpty(can->rcvd)) {
		CAN_Data temp;
		
		PeekBufQueue(can->rcvd, (void*)&temp);
		*format = temp.type & 0x03;
		*id = temp.id;
		*len = (temp.type >> 4) & 0x0F;
		
		Status = true;
	}
	else {
		*format = -1;
		*id = 0L;
		*len = 0;
		
		Status = false;
	}
	
	return Status;
}

DMPAPI(int) can_Receive(void *vcan, unsigned char* buf, int bsize)
{
	int i;
	unsigned long pretime;
	CAN_Data temp;
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return 0; }
	
	i = 0;
	do {
		if (can->RxCount >= can->RxSize ||
		   (can->RxFormat == CAN_STD_REMOTE_FRAME || can->RxFormat == CAN_EXT_REMOTE_FRAME))
		{
			if (can->TimeOut != CAN_NO_TIMEOUT)
			{
				pretime = timer_nowtime();
				while (QueueEmpty(can->rcvd) == true && (timer_nowtime() - pretime) < can->TimeOut); 
				
				if (QueueEmpty(can->rcvd) == true) {
					if (CAN_TIMEOUT_DEBUG)
						err_print((char*)"%s: CAN bus receive timeout.\n", __FUNCTION__);
					return 0;
				}
			}
			else while (QueueEmpty(can->rcvd) == true);
			
			PopBufQueue(can->rcvd, (void *)&temp);
			can->RxFormat = temp.type & 0x03;
			can->RxID = temp.id;
			can->RxSize = (temp.type >> 4) & 0x0F;
			can->RxCount = 0;
			
			if (can->RxFormat == CAN_STD_DATA_FRAME || can->RxFormat == CAN_EXT_DATA_FRAME)
			{
				memcpy((void*)&can->RxBuffer[0], &temp.ldata, 4);
				memcpy((void*)&can->RxBuffer[4], &temp.hdata, 4);
			}
			else {
				can->RxCount = can->RxSize;
				return 0;
			}
		}
		
		while (can->RxCount < can->RxSize && i < bsize)
			buf[i++] = can->RxBuffer[can->RxCount++];
	} while (i < bsize);

    return i;
}

DMPAPI(unsigned int) can_Read(void *vcan)
{
	unsigned char val;
	
	if (can_Receive(vcan, &val, 1) != 1)
		return 0xFFFF;
		
	return (unsigned int)val;
}

DMPAPI(int) can_QueryRxQueue(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return 0; }
	return can->rcvd->count;
}

DMPAPI(bool) can_RxQueueFull(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return false; }
	return QueueFull(can->rcvd);
}

DMPAPI(bool) can_RxQueueEmpty(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return false; }
	return QueueEmpty(can->rcvd);
}

DMPAPI(void) can_FlushRxQueue(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return; }
	ClearQueue(can->rcvd);
}

DMPAPI(int) can_Send(void *vcan, unsigned char* buf, int bsize)
{
	int i, len;
	unsigned long pretime;
	CAN_Data temp;
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return 0; }
	
	if (bsize < 0)
		return 0;
	
	i = 0;
	do {
		if (can->TimeOut != CAN_NO_TIMEOUT)
		{
			pretime = timer_nowtime();
			while (QueueFull(can->xmit) == true && (timer_nowtime() - pretime) < can->TimeOut); 
			
			if (QueueFull(can->xmit) == true) {
				if (CAN_TIMEOUT_DEBUG)
					err_print((char*)"%s: CAN bus transmit timeout.\n", __FUNCTION__);
				return i;
			}
		}
		else while (QueueFull(can->xmit) == true);
		
		len = ((bsize - i) > 8) ? (8) : (bsize - i);
			
		temp.type  = can->TxFormat | (len << 4);
		temp.id    = can->TxID;
		temp.ldata = 0L;
		temp.hdata = 0L;
		
		if (can->TxFormat == CAN_STD_DATA_FRAME || can->TxFormat == CAN_EXT_DATA_FRAME) {
		
			if (len > 4) {
				memcpy(&temp.ldata, &buf[i], 4);
				i += 4;
				len -= 4;
				memcpy(&temp.hdata, &buf[i], len);
				i += len;
			}
			else {
				memcpy(&temp.ldata, &buf[i], len);
				i += len;
			}
		}
		else
			i += len;
		PushBufQueue(can->xmit, (void *)&temp);
		
		can_RoundRobin(can);
		
	} while (i < bsize);
	
	if (can->TxFormat == CAN_STD_REMOTE_FRAME || can->TxFormat == CAN_EXT_REMOTE_FRAME)
		return 0;
	
	return i;
}

DMPAPI(bool) can_Write(void *vcan, unsigned char val)
{
	if (can_Send(vcan, &val, 1) != 1)
		return false;
		
	return true;
}

DMPAPI(int) can_QueryTxQueue(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return 0; }
	return can->xmit->count;
}

DMPAPI(bool) can_TxQueueFull(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return false; }
	return QueueFull(can->xmit);
}

DMPAPI(bool) can_TxQueueEmpty(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return false; }
	return QueueEmpty(can->xmit);
}

DMPAPI(void) can_FlushTxQueue(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return; }
	ClearQueue(can->xmit);
}

DMPAPI(bool) can_TxReady(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return false; }
	return (QueueFull(can->xmit) == true) ? (false) : (true);
}

DMPAPI(void) can_FlushWFIFO(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return; }
	if (can->InUse != 1) { err_print((char*)"%s: CAN bus is not in use.\n", __FUNCTION__); return; }
	
	while (!QueueEmpty(can->xmit));
	while (io_In32(can->handle, can->REQ) & 0x15L);
}

DMPAPI(void) can_SetCANErrorHandler(void *vcan, void (*func)(CAN_Bus *))
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return; }
	
	can->error_handler = func;
}

DMPAPI(void) can_SetCANBusOfHandler(void *vcan, void (*func)(CAN_Bus *))
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return; }
	
	can->busof_handler = func;
}
