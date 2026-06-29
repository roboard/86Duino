/*
  vortex86.h - Part of DM&P Vortex86 Universal Serial library
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

#ifndef __DMP_VORTEX86_H
#define __DMP_VORTEX86_H

#include "dmpcfg.h"
#include "io.h"

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
DMPAPI(void) vx86_uart_DisableTurboFIFO(int com);
DMPAPI(void) vx86_uart_EnableTurboFIFO(int com);
DMPAPI(unsigned int) vx86_uart_TruboFIFOSize(int com);

DMPAPI(int) vx86_uart_GetIRQ(int com);
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

#define VX86_UART_FORMULA1  (0x00)
#define VX86_UART_FORMULA2  (0x01)
#define VX86_UART_FORMULA3  (0x04)
#define VX86_UART_FORMULA4  (0x05)
#define VX86_UART_FORMULA5  (0x06)
#define VX86_UART_FORMULA6  (0x08)
#define VX86_UART_FORMULA7  (0x09)
DMPAPI(int) vx86_GetUartFormula(int com);
DMPAPI(void) vx86_SetUartFormula(int com, int formula);
DMPAPI(void) vx86_SetFixedUartFormula(int com, bool enable);
DMPAPI(unsigned long) vx86_GetBaudClock(int com);
DMPAPI(unsigned long) vx86_GetUartDiv(int com, unsigned long baud);
DMPAPI(void) vx86_SetUartExternalClocks(unsigned long clocks);
DMPAPI(unsigned long) vx86_GetUartFormulaClocks(int formula);
DMPAPI(unsigned short) vx86_GetUartFormulaDivisor(int formula, unsigned long baud);

/* UART Fast Access Register */
DMPAPI(unsigned long) vx86_GetUartFastAccessBase();

DMP_INLINE(unsigned int) vx86_GetUartTxFIFOFree(int com, unsigned long base)
{
    return io_inpdw(base + 0x3C + 0x10 * com) & 0xFF;
}

DMP_INLINE(unsigned int) vx86_GetUartRxFIFORcvd(int com, unsigned long base)
{
    return (io_inpdw(base + 0x3C + 0x10 * com) >> 16) & 0xFF;
}

DMP_INLINE(void) vx86_UartFastWrite(int com, unsigned long base, unsigned long value)
{
    io_outpdw(base + 0x30 + 0x10 * com, value);
}

DMP_INLINE(void) vx86_UartFastWrite16(int com, unsigned long base, unsigned short value)
{
    io_outpw(base + 0x30 + 0x10 * com, value);
}

DMP_INLINE(void) vx86_UartFastWrite8(int com, unsigned long base, unsigned char value)
{
    io_outpb(base + 0x30 + 0x10 * com, value);
}

DMP_INLINE(unsigned long) vx86_UartFastMode0Read(int com, unsigned long base)
{
    return io_inpdw(base + 0x34 + 0x10 * com);
}

DMP_INLINE(unsigned short) vx86_UartFastMode0Read16(int com, unsigned long base)
{
    return io_inpw(base + 0x34 + 0x10 * com);
}

DMP_INLINE(unsigned long) vx86_UartFastMode1Read(int com, unsigned long base)
{
    return io_inpdw(base + 0x38 + 0x10 * com);
}

DMP_INLINE(unsigned short) vx86_UartFastMode1Read16(int com, unsigned long base)
{
    return io_inpw(base + 0x38 + 0x10 * com);
}

DMP_INLINE(unsigned char) vx86_UartFastMode1Read8(int com, unsigned long base)
{
    return io_inpb(base + 0x38 + 0x10 * com);
}

DMPAPI(void) vx86_SetUartMove2ShiftNormalMode(int com);
DMPAPI(void) vx86_SetUartMove2ShiftFastMode(int com);

// ---------------------------------------------------
//       Vortex86 USB Device functions
// ---------------------------------------------------
DMPAPI(unsigned short) vx86_GetUSBDevAddr(void);
DMPAPI(int) vx86_GetUSBDevIRQ(void);

// ---------------------------------------------------
//       Vortex86 CAN bus functions
// ---------------------------------------------------
DMPAPI(unsigned short) vx86_GetCANBusIOAddr(int can);
DMPAPI(unsigned long) vx86_GetCANBusMMIOAddr(int can);
DMPAPI(int) vx86_GetCANBusIRQ(int can);
DMPAPI(unsigned long) vx86_GetCANBusBpsInfo(unsigned long bps);

DMPAPI(bool) vx86_IsPCIeTarget();
DMPAPI(void) vx86_SetSerialMode();
DMPAPI(void) vx86_SetParallelMode();

#ifdef __cplusplus
}
#endif

#endif
