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
////////////////////////////////////////////////////////////////////////////////
//    note that most of functions in this lib assume no paging issue when 
//    using them in ISR; so to use this lib in ISR in DJGPP, it is suggested 
//    to employ PMODE/DJ or HDPMI instead of CWSDPMI.
////////////////////////////////////////////////////////////////////////////////

#include "can.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "io.h"
#include "irq.h"
#include "err.h"

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

DMP_INLINE(void) can_RoundRobin(CAN_Bus *can)
{
	CANFrame temp;
		
	if (QueueEmpty(can->xmit))
		return;
	
	io_DisableINT();
	if (!(io_In32(can->ioHandle, can->REQ) & (0x01L << (can->round*2))))
	{
		PopBufQueue(can->xmit, (void*)&temp);
		
		io_Out32(can->ioHandle, can->TX[can->round].TYPE , (unsigned long)temp.type | ((unsigned long)temp.length << 4));
		io_Out32(can->ioHandle, can->TX[can->round].IDR  , temp.identifier);
		io_Out32(can->ioHandle, can->TX[can->round].DATAL, temp.Data.dword[0]);
		io_Out32(can->ioHandle, can->TX[can->round].DATAH, temp.Data.dword[1]);
		
		io_Out32(can->ioHandle, can->REQ, io_In32(can->ioHandle, can->REQ) | (0x01UL << (can->round*2)));
		
		can->round++;
		if (can->round == 3)
			can->round = 0;
	}
	io_RestoreINT();
}

static int CAN_ISR(int irq, void *data)
{
	unsigned long isr;
	CAN_Bus *can = (CAN_Bus *)data;
	
	if (((isr = io_In32(can->ioHandle, can->ISR)) & 0x07FFL) != 0x00L)
	{
		if (isr & CAN_RXI)
		{
			unsigned long temp;
			CANFrame rx_temp;
			
			io_DisableINT();
			{
				temp = io_In32(can->ioHandle, can->RX.TYPE);
				rx_temp.type          = (int)(temp & 0x03UL);
				rx_temp.length        = (int)((temp >> 4) & 0x0FUL);
				rx_temp.identifier    = io_In32(can->ioHandle, can->RX.IDR);
				rx_temp.Data.dword[0] = io_In32(can->ioHandle, can->RX.DATAL);
				rx_temp.Data.dword[1] = io_In32(can->ioHandle, can->RX.DATAH);
				PushBufQueue(can->rcvd, (void*)&rx_temp);
			}
			io_RestoreINT();
			
			io_Out32(can->ioHandle, can->REQ, 0x0100UL); // release the received message
			io_Out32(can->ioHandle, can->ISR, CAN_RXI);
		}
		
		if (isr & CAN_TX0I)
		{
			unsigned long stat;
			
			stat = io_In32(can->ioHandle, can->TX[0].STAT);
			if (stat & 0x20UL)
			{
				can->LastError = CAN_ERROR_TX0 + (unsigned char)((stat & 0x00070000UL) >> 16);
				if (can->StoreError)
					PushQueue(can->Error, can->LastError);
			}
			if ((stat & 0x07UL) == 0x05UL) can_RoundRobin(can);
			
			io_Out32(can->ioHandle, can->ISR, CAN_TX0I);
		}
		
		if (isr & CAN_TX1I)
		{
			unsigned long stat;
			
			stat = io_In32(can->ioHandle, can->TX[1].STAT);
			if (stat & 0x20UL)
			{
				can->LastError = CAN_ERROR_TX1 + (unsigned char)((stat & 0x00070000UL) >> 16);
				if (can->StoreError)
					PushQueue(can->Error, can->LastError);
			}
			if ((stat & 0x07UL) == 0x05UL) can_RoundRobin(can);
			
			io_Out32(can->ioHandle, can->ISR, CAN_TX1I);
		}
		
		if (isr & CAN_TX2I)
		{
			unsigned long stat;
			
			stat = io_In32(can->ioHandle, can->TX[2].STAT);
			if (stat & 0x20UL)
			{
				can->LastError = CAN_ERROR_TX2 + (unsigned char)((stat & 0x00070000UL) >> 16);
				if (can->StoreError)
					PushQueue(can->Error, can->LastError);
			}
			if ((stat & 0x07UL) == 0x05UL) can_RoundRobin(can);
			
			io_Out32(can->ioHandle, can->ISR, CAN_TX2I);
		}
		
		if (isr & CAN_ECI)
		{
			io_Out32(can->ioHandle, can->ISR, CAN_ECI);
		}
		
		if (isr & CAN_EPI)
		{
			io_Out32(can->ioHandle, can->ISR, CAN_EPI);
		}
		
		if (isr & CAN_BOI)
		{
			io_Out32(can->ioHandle, can->ISR, CAN_BOI);
			if (can->busoff_handler) (*can->busoff_handler)(can);
		}
		
		if (isr & CAN_ALI)
		{
			io_Out32(can->ioHandle, can->ISR, CAN_ALI);
		}
		
		if (isr & CAN_RBEI)
		{
			unsigned long stat;
			
			stat = io_In32(can->ioHandle, can->RX.STAT);
			can->LastError = CAN_ERROR_RX + (unsigned char)((stat & 0x70UL) >> 4);
			if (can->StoreError)
				PushQueue(can->Error, can->LastError);
			
			io_Out32(can->ioHandle, can->ISR, CAN_RBEI);
		}
		
		if (isr & CAN_RBOI)
		{
			io_Out32(can->ioHandle, can->ISR, CAN_RBOI);
		}
		
		if (isr & CAN_XPI)
		{
			io_Out32(can->ioHandle, can->ISR, CAN_XPI);
		}
	}
	else
		return ISR_NONE;

	return ISR_HANDLED;
}

DMPAPI(void *) CreateCANBus(int IO_Space)
{
	CAN_Bus *can;
	
	if (IO_Space != IO_PORT && IO_Space != IO_MEMORY)
		return NULL;
	
	if (io_Init() == false) {
		err_print((char*)"%s: Init IO lib error!!\n", __FUNCTION__);
		goto IO_INIT_FAIL;
	}
	
	if ((can = (CAN_Bus *)ker_Malloc(sizeof(CAN_Bus))) == NULL) {
		err_print((char*)"%s: CAN bus malloc error.\n", __FUNCTION__);
		goto CAN_MALLOC_FAIL;
	}
	
	can->IO_Space = IO_Space;
	
	/* Get CAN-BUS IO base address */
	can->IO_Addr = vx86_GetCANBusIOAddr();
	if (can->IO_Space == IO_PORT && can->IO_Addr == 0x0000)
		goto WITHOUT_CANBUS_FAIL;
		
	/* Get CAN-BUS MMIO base address */
	can->MMIO_Addr = vx86_GetCANBusMMIOAddr();
	if (can->IO_Space == IO_MEMORY && can->MMIO_Addr == 0x00UL)
		goto WITHOUT_CANBUS_FAIL;
		
	/* Get CAN-BUS IRQ number */
	can->nIRQ = vx86_GetCANBusIRQ();
	if (can->nIRQ == 0)
		goto WITHOUT_CANBUS_FAIL;
		
	can->InUse         = false;
	can->RxTimeOut     = CAN_NO_TIMEOUT;
	can->TxTimeOut     = CAN_NO_TIMEOUT;
	
	can->BitTime       = CAN_BPS_1000K;
	
	can->NowState      = CAN_STAT_ACTIVE;
	can->LastError     = CAN_ERROR_NONE;
	can->round         = 0;
	
	can->StoreError    = false;
	
	/* CAN-BUS registers offset */
	can->GCR           = 0x00;
	can->CPS           = 0x04;
	can->BTR           = 0x08;
	can->IER           = 0x0C;
	can->ISR           = 0x10;
	can->GSR           = 0x14;
	can->REQ           = 0x18;
	can->EWR           = 0x2C;
	can->ECR           = 0x30;
	can->IDI           = 0x34;
	can->IDF           = 0x38;
	can->IDM           = 0x3C;
	can->RX.STAT       = 0x28;
	can->RX.TYPE       = 0x70;
	can->RX.IDR        = 0x74;
	can->RX.DATAL      = 0x78;
	can->RX.DATAH      = 0x7C;
	can->TX[0].STAT    = 0x1C;
	can->TX[0].TYPE    = 0x40;
	can->TX[0].IDR     = 0x44;
	can->TX[0].DATAL   = 0x48;
	can->TX[0].DATAH   = 0x4C;
	can->TX[1].STAT    = 0x20;
	can->TX[1].TYPE    = 0x50;
	can->TX[1].IDR     = 0x54;
	can->TX[1].DATAL   = 0x58;
	can->TX[1].DATAH   = 0x5C;
	can->TX[2].STAT    = 0x24;
	can->TX[2].TYPE    = 0x60;
	can->TX[2].IDR     = 0x64;
	can->TX[2].DATAL   = 0x68;
	can->TX[2].DATAH   = 0x6C;
	
	can->busoff_handler = NULL;

	/* Alloc Tx, Rx, and Error queues */
	if ((can->Error = CreateQueue(CAN_ERR_QUEUE_SIZE)) == NULL)
		goto CAN_ERR_QUEUE_FAIL;
	if ((can->rcvd = CreateBufQueue(CAN_RX_QUEUE_SIZE, sizeof(CANFrame))) == NULL)
		goto CAN_RX_QUEUE_FAIL;
	if ((can->xmit = CreateBufQueue(CAN_TX_QUEUE_SIZE, sizeof(CANFrame))) == NULL)
		goto CAN_TX_QUEUE_FAIL;

	return (void*)can;
	
CAN_TX_QUEUE_FAIL:
	DestoryQueue(can->rcvd);
CAN_RX_QUEUE_FAIL:
	DestoryQueue(can->Error);
CAN_ERR_QUEUE_FAIL:
WITHOUT_CANBUS_FAIL:
	ker_Mfree((void*)can);
CAN_MALLOC_FAIL:
	io_Close();
IO_INIT_FAIL:
	return NULL;
}

DMPAPI(bool) can_Init(void *vcan)
{
	int ioUse;
	unsigned long baseAddress;
	unsigned long temp;
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return false;
	}
	
	if (can->InUse == true)
		return true;

	switch (can->IO_Space)
	{
		case IO_PORT:
		{
			ioUse = IO_USE_PORTIO;
			baseAddress = (unsigned long)can->IO_Addr;
		}
		break;
		
		case IO_MEMORY:
		{
			ioUse = IO_USE_MMIO;
			baseAddress = can->MMIO_Addr;
		}
		break;
		
		default: 
		{
			err_print((char*)"%s: \'IO space\' error.\n", __FUNCTION__);
			goto SPACE_FAIL;
		}
	};
	
	if ((can->ioHandle = io_Alloc(ioUse, baseAddress, 0x100)) == NULL) {
		err_print((char*)"%s: IO alloc error.\n", __FUNCTION__);
		goto IO_ALLOC_FAIL;
	}
		
	if (irq_Init() == false) {
        err_print((char*)"%s: IRQ init fail.\n", __FUNCTION__);
        goto IRQ_INIT_FAIL;
    }
	
	/* Software reset */
	io_Out32(can->ioHandle, can->GCR, io_In32(can->ioHandle, can->GCR) | 0x01UL);    // reset CAN registers
	while (io_In32(can->ioHandle, can->GCR) & 0x01UL);
	io_Out32(can->ioHandle, can->GCR, io_In32(can->ioHandle, can->GCR) | 0x10000UL); // reset ID filter table
	while (io_In32(can->ioHandle, can->GCR) & 0x10000UL);
	
	/* Set Bit Time */
	can_SetBPS(vcan, can->BitTime);
	
	/* Enable Interrupt */
	io_DisableINT();
	irq_Setting(can->nIRQ, IRQ_LEVEL_TRIGGER);
	irq_InstallISR(can->nIRQ, CAN_ISR, (void *)can);
	io_Out32(can->ioHandle, can->IER, CAN_ALL_INT);
	io_RestoreINT();
	
	can->NowState  = CAN_STAT_ACTIVE;
	can->LastError = CAN_ERROR_NONE;
	can->round     = 0;
	
	/* Error retry, Arbitration lost retry, Round Robin, Idenfitier filter enable */
	temp = io_In32(can->ioHandle, can->GCR) | 0x20382UL;
	io_Out32(can->ioHandle, can->GCR, temp);
	while (io_In32(can->ioHandle, can->GCR) != temp);
	
	can->InUse = true;
	
	return true;
	
	
IRQ_INIT_FAIL:
	io_Free(can->ioHandle);
	can->ioHandle = NULL;
IO_ALLOC_FAIL:
SPACE_FAIL:
	return false;
}

DMPAPI(void) can_Close(void *vcan)
{	
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return;
	}
	
	if (can->InUse != 0) {
		io_DisableINT();
		{
			io_Out32(can->ioHandle, can->GCR, io_In32(can->ioHandle, can->GCR) | 0x01UL);    // reset CAN registers
			while (io_In32(can->ioHandle, can->GCR) & 0x01UL);
			io_Out32(can->ioHandle, can->GCR, io_In32(can->ioHandle, can->GCR) | 0x10000UL); // reset ID filter table
			while (io_In32(can->ioHandle, can->GCR) & 0x10000UL);
			irq_UninstallISR(can->nIRQ, (void *)can);
		}
		io_RestoreINT();
		
		irq_Close();
		io_Free(can->ioHandle);
		io_Close();
	}
	DestoryQueue(can->Error);
	DestoryQueue(can->rcvd);
	DestoryQueue(can->xmit);
	ker_Mfree((void *)can);
}

DMPAPI(void) can_Reset(void *vcan)
{
	unsigned long gcr;
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL || can->InUse == false) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return;
	}
	
	/* Store Global Control Register */
	gcr = io_In32(can->ioHandle, can->GCR);
	
	/* Software reset */
	io_Out32(can->ioHandle, can->GCR, io_In32(can->ioHandle, can->GCR) | 0x01UL);    // reset CAN registers
	while (io_In32(can->ioHandle, can->GCR) & 0x01UL);
	
	can->NowState  = CAN_STAT_ACTIVE;
	can->LastError = CAN_ERROR_NONE;
	can->round     = 0;
	
	/* Clear buffers */
	ClearQueue(can->xmit);
	ClearQueue(can->rcvd);
	ClearQueue(can->Error);
	
	/* Enable Interrupt */
	io_Out32(can->ioHandle, can->IER, CAN_ALL_INT);
	
	/* Restore Global Control Register */
	io_Out32(can->ioHandle, can->GCR, gcr);
	while (io_In32(can->ioHandle, can->GCR) != gcr);
}

DMPAPI(bool) can_SetBPS(void *vcan, unsigned long bps)
{
	int temp;
	unsigned long gcr, cps, btr;
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return false;
	}
	
	/* Check CAN bus clock select source */
	cps = bps & 0x80000000UL;
	
	/* Check CAN bus clock divider */
	temp = (int)((bps >> 16) & 0xFFUL);
	if (temp > 128 || temp < 2 || (temp%2) != 0) {
		err_print((char*)"%s: 'ckdiv' error.\n", __FUNCTION__);
		return false;
	}
	cps |= (unsigned long)((temp >> 1) - 1);
	
	/* Check CAN bus bit-time sampling */
	btr = (bps & 0x40000000UL) ? (0x8000UL) : (0x0000UL);
	
	/* Check CAN bus synchronization jump width */
	temp = (int)((bps >> 12) & 0x0FUL);
	if (temp > 4 || temp < 0) {
		err_print((char*)"%s: 'sjw' error.\n", __FUNCTION__);
		return false;
	}
	btr |= (unsigned long)(temp - 1) << 12;
	
	/* Check CAN bus propagation segment */
	temp = (int)((bps >> 8) & 0x0FUL);
	if (temp > 8 || temp < 0) {
		err_print((char*)"%s: 'prog' error.\n", __FUNCTION__);
		return false;
	}
	btr |= (unsigned long)(temp - 1);
	
	/* Check CAN bus phase segment 1 */
	temp = (int)((bps >> 4) & 0x0FUL);
	if (temp > 8 || temp < 0) {
		err_print((char*)"%s: 'ps1' error.\n", __FUNCTION__);
		return false;
	}
	btr |= (unsigned long)(temp - 1) << 4;
	
	/* Check CAN bus phase segment 2 */
	temp = (int)(bps & 0x0FUL);
	if (temp > 8 || temp < 0) {
		err_print((char*)"%s: 'ps2' error.\n", __FUNCTION__);
		return false;
	}
	btr |= (unsigned long)(temp - 1) << 8;
	
	/* Disable CAN-BUS */
	gcr = io_In32(can->ioHandle, can->GCR);
	io_Out32(can->ioHandle, can->GCR, gcr & 0xFFFFFFFDUL);
	while (io_In32(can->ioHandle, can->GCR) & 0x00000002UL);
	
	/* Set Clock Pre-Scaler */
	io_Out32(can->ioHandle, can->CPS, cps);
	
	/* Set Bus Timing */
	io_Out32(can->ioHandle, can->BTR, btr);
	
	/* Enable CAN-BUS */
	io_Out32(can->ioHandle, can->GCR, gcr);
	while (io_In32(can->ioHandle, can->GCR) != gcr);
	
	return true;
}

DMPAPI(void) can_SetTimeOut(void *vcan, unsigned long rx_timeout, unsigned long tx_timeout)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return; }
	
	can->RxTimeOut = rx_timeout;
	can->TxTimeOut = tx_timeout;
}

DMPAPI(bool) can_AddIDFilter(void *vcan, int index, int ext_id, unsigned long filter, unsigned long mask)
{
	unsigned long idf, idm;
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return false;
	}
	
	/* Check Identifier filter index */
	if (index > 31 || index < 0) {
		err_print((char*)"%s: 'index' error.\n", __FUNCTION__);
		return false;
	}
	
	/* Check Identifier filter format */
	idf = (ext_id != 0) ? (0xC0000000UL) : (0x80000000UL);
	
	/* Check Identifier filter */
	if ((ext_id != 0 && (filter & 0xE0000000UL)) || (ext_id == 0 && (filter & 0xFFFFF800UL))) {
		err_print((char*)"%s: 'filter' error.\n", __FUNCTION__);
		return false;
	}
	idf |= filter;
	
	/* Check Identifier mask */
	if ((ext_id != 0 && (mask & 0xE0000000UL)) || (ext_id == 0 && (mask & 0xFFFFF800UL))) {
		err_print((char*)"%s: 'mask' error.\n", __FUNCTION__);
		return false;
	}
	idm = mask;
	
	io_DisableINT();
	io_Out32(can->ioHandle, can->IDF, idf);
	io_Out32(can->ioHandle, can->IDM, idm);
	io_Out32(can->ioHandle, can->IDI, (unsigned long)index | 0x00000100UL);
	io_RestoreINT();
	
	return true;
}

DMPAPI(bool) can_GetIDFilter(void *vcan, int index, int *ext_id, unsigned long *filter, unsigned long *mask)
{
	unsigned long idf, idm;
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return false;
	}
	
	/* Check Identifier filter index */
	if (index > 31 || index < 0) {
		err_print((char*)"%s: 'index' error.\n", __FUNCTION__);
		return false;
	}
	
	io_DisableINT();
	io_Out32(can->ioHandle, can->IDI, (unsigned long)index);
	idf = io_In32(can->ioHandle, can->IDF);
	idm = io_In32(can->ioHandle, can->IDM);
	io_RestoreINT();
		
	*ext_id = (int)((idf & 0x40000000UL) >> 30);
	*filter = idf & 0x1FFFFFFFUL;
	*mask   = idm & 0x1FFFFFFFUL;
	
	return (idf & 0x80000000UL) ? true : false;
}

DMPAPI(bool) can_DelIDFilter(void *vcan, int index)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return false;
	}
	
	/* Check Identifier filter index */
	if (index > 31 || index < 0) {
		err_print((char*)"%s: 'index' error.\n", __FUNCTION__);
		return false;
	}
	
	io_DisableINT();
	io_Out32(can->ioHandle, can->IDI, (unsigned long)index);
	io_Out32(can->ioHandle, can->IDF, io_In32(can->ioHandle, can->IDF) & 0x7FFFFFFFUL);
	io_Out32(can->ioHandle, can->IDI, (unsigned long)index | 0x00000100UL);
	io_RestoreINT();
	
	return true;
}

DMPAPI(void) can_ClearIDList(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return;
	}
	
	io_Out32(can->ioHandle, can->GCR, io_In32(can->ioHandle, can->GCR) | 0x10000UL); // reset ID filter list
	while (io_In32(can->ioHandle, can->GCR) & 0x10000UL);
}

DMPAPI(void) can_EnableBypass(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL && can->InUse == false) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return;
	}
	
	io_Out32(can->ioHandle, can->GCR, io_In32(can->ioHandle, can->GCR) | 0x00060000UL);
}

DMPAPI(void) can_DisableBypass(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL && can->InUse == false) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return;
	}
	
	io_Out32(can->ioHandle, can->GCR, io_In32(can->ioHandle, can->GCR) & 0xFFFBFFFFUL);
}

DMPAPI(bool) can_SetEWLimit(void *vcan, int ewl)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return false;
	}
	
	if (ewl > 255 || ewl < 0) {
		err_print((char*)"%s: 'ewl' error.\n", __FUNCTION__);
		return false;
	}
	
	io_Out32(can->ioHandle, can->EWR, (unsigned long)ewl);
	
	return true;
}

DMPAPI(int) can_GetEWLimit(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return -1;
	}
	
	return (int)(io_In32(can->ioHandle, can->EWR) & 0xFFUL);
}

DMPAPI(int) can_GetTxErrorCount(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return -1;
	}
	
	return (int)(io_In32(can->ioHandle, can->ECR) & 0x1FFUL);
}

DMPAPI(int) can_GetRxErrorCount(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return -1;
	}
	
	return (int)((io_In32(can->ioHandle, can->ECR) >> 16) & 0xFFUL);
}

DMPAPI(unsigned char) can_GetNowState(void *vcan)
{
	unsigned char state;
	unsigned long gsr;
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL && can->InUse == false) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return 0xFF;
	}	
	
	gsr = io_In32(can->ioHandle, can->GSR) & 0x34UL;
	
	if (gsr & 0x04UL)
		state = CAN_STAT_BUS_OFF;
	else if (gsr & 0x10UL)
		state = CAN_STAT_PASSIVE;
	else
		state = CAN_STAT_ACTIVE;
		
	if (gsr & 0x20UL)
		state |= CAN_STAT_WARNING;
	
	return state;
}

DMPAPI(void) can_EnableStoreError(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return;
	}
	
	can->StoreError = true;
}

DMPAPI(void) can_DisableStoreError(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return;
	}
	
	can->StoreError = false;
	ClearQueue(can->Error);
}

DMPAPI(unsigned char) can_PopError(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return 0xFF;
	}
	
	if (QueueEmpty(can->Error))
		return CAN_ERROR_NONE;
		
	return PopQueue(can->Error);
}

DMPAPI(unsigned char) can_GetLastError(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return 0xFF;
	}
	
	return can->LastError;
}

DMPAPI(bool) can_Read(void *vcan, CANFrame *pack)
{
	unsigned long pretime;
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return false;
	}
	
	if (pack == NULL) {
		err_print((char*)"%s: CAN-BUS package is null.\n", __FUNCTION__);
		return false;
	}
	
	if (can->RxTimeOut != CAN_NO_TIMEOUT)
	{
		pretime = timer_NowTime();
		while (QueueEmpty(can->rcvd) && (timer_NowTime() - pretime) < can->RxTimeOut); 
		
		if (QueueEmpty(can->rcvd)) {
			if (CAN_TIMEOUT_DEBUG)
				err_print((char*)"%s: CAN bus receive timeout.\n", __FUNCTION__);
			return false;
		}
	}
	else while (QueueEmpty(can->rcvd));
		
	PopBufQueue(can->rcvd, (void*)pack);
	
	return true;
}

DMPAPI(int) can_QueryRxQueue(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	if (can == NULL) { err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__); return 0; }
	return QueueSize(can->rcvd);
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

DMPAPI(bool) can_Write(void *vcan, CANFrame *pack)
{
	unsigned long pretime;
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return false;
	}
	
	if (pack == NULL) {
		err_print((char*)"%s: CAN-BUS package is null.\n", __FUNCTION__);
		return false;
	}
	/* Check CAN-BUS packet type */
	if (pack->type > 0x03 || pack->type < 0x00) {
		err_print((char*)"%s: CAN-BUS \'packet type\' error.\n", __FUNCTION__);
		return false;
	}
	
	/* Check CAN-BUS data length */
	if (pack->length > 8 || pack->length < 0) {
		err_print((char*)"%s: CAN-BUS \'data length\' error.\n", __FUNCTION__);
		return false;
	}
	
	if (can->TxTimeOut != CAN_NO_TIMEOUT)
	{
		pretime = timer_NowTime();
		while (QueueFull(can->xmit) && (timer_NowTime() - pretime) < can->TxTimeOut); 
		
		if (QueueFull(can->xmit)) {
			if (CAN_TIMEOUT_DEBUG)
				err_print((char*)"%s: CAN bus transmit timeout.\n", __FUNCTION__);
			return false;
		}
	}
	else while (QueueFull(can->xmit));
	
	PushBufQueue(can->xmit, (void*)pack);
	can_RoundRobin(can);
	
	return true;
}

DMPAPI(int) can_QueryTxQueue(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return 0;
	}
	
	return QueueSize(can->xmit);
}

DMPAPI(bool) can_TxQueueFull(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return false;
	}
	
	return QueueFull(can->xmit);
}

DMPAPI(bool) can_TxQueueEmpty(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return false;
	}
	
	return QueueEmpty(can->xmit);
}

DMPAPI(void) can_FlushTxQueue(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return;
	}
	
	ClearQueue(can->xmit);
}

DMPAPI(bool) can_TxReady(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return false;
	}
	
	return (QueueFull(can->xmit)) ? (false) : (true);
}

DMPAPI(void) can_FlushWFIFO(void *vcan)
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL || can->InUse == false) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return;
	}
	
	while (!QueueEmpty(can->xmit)) {
		if (can_GetNowState(vcan) == CAN_STAT_BUS_OFF)
			break;
	}
	while (io_In32(can->ioHandle, can->REQ) & 0x15L);
}

DMPAPI(void) can_SetCANBusOffHandler(void *vcan, void (*func)(CAN_Bus *))
{
	CAN_Bus *can = (CAN_Bus *)vcan;
	
	if (can == NULL) {
		err_print((char*)"%s: CAN bus is null.\n", __FUNCTION__);
		return;
	}
	
	can->busoff_handler = func;
}
