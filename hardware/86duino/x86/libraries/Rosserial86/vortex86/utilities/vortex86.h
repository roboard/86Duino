/*
  vortex86.h - Part of DM&P Vortex86 Universal Serial library
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

#ifndef __DMP_VORTEX86_H
#define __DMP_VORTEX86_H

#include "dmpcfg.h"

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------
//            Vortex86 UART functions
// ---------------------------------------------------
//
// vx86_uart_Init(com, mode)
//     [com] : 0 ~ (Maximum size of UART - 1)
//     [mode]: Maximum baud mode
DMPAPI(bool) vx86_uart_Init(int com);
DMPAPI(void) vx86_uart_Close(int com);

DMPAPI(unsigned short) vx86_uart_GetBaseAddr(int com);
DMPAPI(void) vx86_uart_SetFIFO16(int com);
DMPAPI(void) vx86_uart_SetFIFO32(int com);

DMPAPI(int) vx86_uart_GetIRQ(int com);

DMPAPI(unsigned long) vx86_uart_MaxBPS(int com);
DMPAPI(void) vx86_uart_EnableHalfDuplex(int com);
DMPAPI(void) vx86_uart_EnableFullDuplex(int com);

DMPAPI(int) vx86_uart_GetSBCLK(void);
DMPAPI(int) vx86_uart_GetCS(int com);
DMPAPI(int) vx86_uart_GetHCS(int com);
DMPAPI(void) vx86_uart_SetSBCLK(int sbclk);
DMPAPI(void) vx86_uart_SetCS(int com, int cs);
DMPAPI(void) vx86_uart_SetHCS(int com, int hcs);

DMPAPI(void) vx86_EnableTurboMode(int com);
DMPAPI(void) vx86_DisableTurboMode(int com);


// ---------------------------------------------------
//       Vortex86 USB Device functions
// ---------------------------------------------------
DMPAPI(unsigned short) vx86_GetUSBDevAddr(void);
DMPAPI(int) vx86_GetUSBDevIRQ(void);

// ---------------------------------------------------
//       Vortex86 CAN bus functions
// ---------------------------------------------------
DMPAPI(unsigned short) vx86_GetCANBusIOAddr(void);
DMPAPI(unsigned long) vx86_GetCANBusMMIOAddr(void);
DMPAPI(int) vx86_GetCANBusIRQ(void);

#ifdef __cplusplus
}
#endif

#endif
