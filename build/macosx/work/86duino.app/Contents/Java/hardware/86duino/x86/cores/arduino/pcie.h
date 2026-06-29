/*
  pcie.h - DM&P Vortex86 PCIE Target library
  Copyright (c) 2019 Johnson Hung <Dyhung@dmp.com.tw>. All right reserved.

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

#ifndef __DMP_PCIE_H
#define __DMP_PCIE_H

#include "dmpcfg.h"

#ifdef __cplusplus
extern "C" {
#endif

DMPAPI(void*) pcie_Init();
DMPAPI(void) pcie_Close(void *pcie);
DMPAPI(void) pcie_SetFormat(void *pcie, unsigned char format);
#define BYTESIZE5          (0x00)
#define BYTESIZE6          (0x01)
#define BYTESIZE7          (0x02)
#define BYTESIZE8          (0x03)
#define STOPBIT1           (0x00)
#define STOPBIT2           (0x04)
#define NOPARITY           (0x00)
#define ODDPARITY          (0x08)
#define EVENPARITY         (0x18)
DMPAPI(void) pcie_SetTimeOut(void *pcie, unsigned long rx_timeout, unsigned long tx_timeout);
#define PCIE_NO_TIMEOUT (~0UL)

DMPAPI(int)  pcie_QueryRxQueue(void *pcie);
DMPAPI(bool) pcie_RxQueueFull(void *pcie);
DMPAPI(bool) pcie_RxQueueEmpty(void *pcie);
DMPAPI(unsigned int) pcie_Read(void *pcie);
DMPAPI(int)  pcie_Receive(void *pcie, unsigned char *buf, int len);
DMPAPI(void) pcie_FlushRxQueue(void *pcie);

DMPAPI(int)  pcie_QueryTxQueue(void *pcie);
DMPAPI(bool) pcie_TxQueueFull(void *pcie);
DMPAPI(bool) pcie_TxQueueEmpty(void *pcie);
DMPAPI(bool) pcie_TxReady(void *pcie);
DMPAPI(int)  pcie_Send(void *pcie, unsigned char *buf, int len);
DMPAPI(bool) pcie_Write(void *pcie, unsigned char val);
DMPAPI(void) pcie_FlushTxQueue(void *pcie);
DMPAPI(void) pcie_FlushWFIFO(void *pcie);

DMPAPI(void) pcie_SetSerialMode(void *vport);
DMPAPI(void) pcie_SetParallelMode(void *vport);

DMPAPI(void) pcie_SetSemaphoreToHost(void *pcie, int sema);
DMPAPI(int)  pcie_GetSemaphoreFromHost(void *pcie);
DMPAPI(void) pcie_SemaphoreFromHostIntEnable(void *pcie, void (*callback)(void));
DMPAPI(void) pcie_SemaphoreFromHostIntDisable(void *pcie);
DMPAPI(void) pcie_SetSemaphoreToEX2(void *pcie, int sema);
DMPAPI(int)  pcie_GetSemaphoreFromEX2(void *pcie);
DMPAPI(void) pcie_SemaphoreFromEX2IntEnable(void *pcie, void (*callback)(void));
DMPAPI(void) pcie_SemaphoreFromEX2IntDisable(void *pcie);

#ifdef __cplusplus
}
#endif

#endif
