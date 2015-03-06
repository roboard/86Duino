/*
  vortex86.cpp - Part of DM&P Vortex86 Universal Serial library
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

#define __DMP_VORTEX86_LIB

#include "vortex86.h"

#if defined(DMP_WIN32_MSVC)  // currently used in io.cpp & rcservo.cpp
#define USE_WINIO3       // use Yariv Kaplan's WinIO library 3.0 (allowing MMIO, http://www.internals.com/)
//#define USE_WINIO2     // use Yariv Kaplan's WinIO library 2.0 (has bug on MMIO, http://www.internals.com/)
//#define USE_PCIDEBUG   // use Kasiwano's PciDebug library      (http://www.otto.to/~kasiwano/)
//#define USE_PHYMEM     // use cyb70289's PhyMem library        (http://www.codeproject.com/KB/system/phymem.aspx)
#endif

#if defined DMP_DOS_DJGPP
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdarg.h>

    #include <go32.h>
    #include <dpmi.h>

    #include <sys/farptr.h>
    #include <pc.h>
    #include <dos.h>
    #include <time.h>
#elif defined DMP_DOS_WATCOM
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdarg.h>
    #include <string.h>

    #include <conio.h>
    #include <i86.h>
    #include <time.h>
#elif defined DMP_DOS_BC
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdarg.h>
    
    #include <conio.h>
    #include <dos.h>
#elif defined(USE_WINIO2) || defined(USE_WINIO3)
    #include <stdio.h>
    #include <windows.h>
    #include <winio.h>
#elif defined USE_PCIDEBUG
    #include <stdio.h>
    #include <windows.h>
    #include <pcifunc.h>
#elif defined USE_PHYMEM
    #include <stdio.h>
    #include <windows.h>
    #include <pmdll.h>
#elif defined DMP_LINUX
    #include <sys/io.h>
    #include <unistd.h>
    #include <sys/times.h>
#endif

#include "io.h"
#include "err.h"

DMPAPI(void) vx86_EnableTurboMode(int com)
{
	switch (com)
	{
	case 0:
		sb_Write8(0x53, sb_Read8(0x53) | (1 << 6));
		break;
	case 1:
		sb_Write(0xA0, sb_Read(0xA0) | (1L << 22));
		break;
	case 2:
		sb_Write(0xA4, sb_Read(0xA4) | (1L << 22));
		break;
	case 3:
		sb_Write(0xA8, sb_Read(0xA8) | (1L << 22));
		break;
	}
}

DMPAPI(void) vx86_DisableTurboMode(int com)
{
	switch (com)
	{
	case 0:
		sb_Write8(0x53, sb_Read8(0x53) & ~(1 << 6));
		break;
	case 1:
		sb_Write(0xA0, sb_Read(0xA0) & ~(1L << 22));
		break;
	case 2:
		sb_Write(0xA4, sb_Read(0xA4) & ~(1L << 22));
		break;
	case 3:
		sb_Write(0xA8, sb_Read(0xA8) & ~(1L << 22));
		break;
	}
}

DMP_INLINE(bool) vx86dx_IsTurboMode(int com)
{
	switch (com)
	{
	case 0:
		if ((sb_Read8(0x53) & (1 << 6)) == 0)
			return false;
		break;
	case 1:
		if ((sb_Read(0xA0) & (1L << 22)) == 0L)
			return false;
		break;
	case 2:
		if ((sb_Read(0xA4) & (1L << 22)) == 0L)
			return false;
		break;
	case 3:
		if ((sb_Read(0xA8) & (1L << 22)) == 0L)
			return false;
		break;
	}

    return true;
}

/**************************************************************
*             Vortex86 UART Clock Mode Config                 *
**************************************************************/
// CS
DMPAPI(int) vx86_uart_GetCS(int com)
{
	int cpuid = vx86_CpuID();
	int cs;
	
	switch (cpuid)
	{
	case CPU_VORTEX86EX:
		{
		unsigned short uart_baseAddr;
		
		uart_baseAddr = sb_Read16(0x60) & 0xfffe;
		sb_Write16(0x60, sb_Read16(0x60) | 0x0001);
		
		cs = (io_inpdw(uart_baseAddr + com*4) >> 22) & 0x01L;
		}
		break;
	case CPU_VORTEX86DX2:
	case CPU_VORTEX86DX3:
		{
		switch (com)
		{
		case 0:
			cs = (sb_Read8(0x53) >> 6) & 0x01;
			break;
		case 1:
			cs = (sb_Read(0xA0) >> 22) & 0x01L;
			break;
		case 2:
			cs = (sb_Read(0xA4) >> 22) & 0x01L;
			break;
		case 3:
			cs = (sb_Read(0xA8) >> 22) & 0x01L;
			break;
		case 4:
			cs = (sb1_Read(0xA0) >> 22) & 0x01L;
			break;
		case 5:
			cs = (sb1_Read(0xA4) >> 22) & 0x01L;
			break;
		case 6:
			cs = (sb1_Read(0xA8) >> 22) & 0x01L;
			break;
		case 7:
			cs = (sb1_Read(0xAC) >> 22) & 0x01L;
			break;
		case 8:
			cs = (sb_Read(0xAC) >> 22) & 0x01L;
			break;
		}
		}
		break;
	case CPU_VORTEX86DX_D:
		{
		switch (com)
		{
		case 0:
			cs = ((sb_Read8(0x53) & (1 << 6)) == 0) ? (0) : (1);
			break;
		case 1:
			cs = ((sb_Read(0xA0) & (1L << 22)) == 0L) ? (0) : (1);
			break;
		case 2:
			cs = ((sb_Read(0xA4) & (1L << 22)) == 0L) ? (0) : (1);
			break;
		case 3:
			cs = ((sb_Read(0xA8) & (1L << 22)) == 0L) ? (0) : (1);
			break;
		}
		}
		break;
	case CPU_VORTEX86MX:
	case CPU_VORTEX86MX_PLUS:
		{
		switch (com)
		{
		case 0:
			cs = ((sb_Read8(0x53) & (1 << 6)) == 0) ? (0) : (1);
			break;
		case 2:
			cs = ((sb_Read(0xA4) & (1L << 22)) == 0L) ? (0) : (1);
			break;
		case 3:
			cs = ((sb_Read(0xA8) & (1L << 22)) == 0L) ? (0) : (1);
			break;
		}
		}
		break;
	}
    
	return cs;
}

DMPAPI(void) vx86_uart_SetCS(int com, int cs)
{
	int cpuid = vx86_CpuID();
	
	switch (cpuid)
	{
	case CPU_VORTEX86EX:
		{
		unsigned short uart_baseAddr;
		
		uart_baseAddr = sb_Read16(0x60) & 0xfffe;
		sb_Write16(0x60, sb_Read16(0x60) | 0x0001);
		
		io_outpdw(uart_baseAddr + com*4, io_inpdw(uart_baseAddr + com*4) & 0xffbfffffL |
		                                 ((unsigned long)cs  << 22));
		}
		break;
	case CPU_VORTEX86DX2:
	case CPU_VORTEX86DX3:
		{
		switch (com)
		{
		case 0:
			sb_Write8(0x53, (sb_Read8(0x53) & 0xBF) | (cs << 6));
			break;
		case 1:
			sb_Write(0xA0, (sb_Read(0xA0) & 0xFFBFFFFFL) | ((unsigned long)cs << 22));
			break;
		case 2:
			sb_Write(0xA4, (sb_Read(0xA4) & 0xFFBFFFFFL) | ((unsigned long)cs << 22));
			break;
		case 3:
			sb_Write(0xA8, (sb_Read(0xA8) & 0xFFBFFFFFL) | ((unsigned long)cs << 22));
			break;
		case 4:
			sb1_Write(0xA0, (sb1_Read(0xA0) & 0xFFBFFFFFL) | ((unsigned long)cs << 22));
			break;
		case 5:
			sb1_Write(0xA4, (sb1_Read(0xA4) & 0xFFBFFFFFL) | ((unsigned long)cs << 22));
			break;
		case 6:
			sb1_Write(0xA8, (sb1_Read(0xA8) & 0xFFBFFFFFL) | ((unsigned long)cs << 22));
			break;
		case 7:
			sb1_Write(0xAC, (sb1_Read(0xAC) & 0xFFBFFFFFL) | ((unsigned long)cs << 22));
			break;
		case 8:
			sb_Write(0xAC, (sb_Read(0xAC) & 0xFFBFFFFFL) | ((unsigned long)cs << 22));
			break;
		}
		}
		break;
	case CPU_VORTEX86DX_D:
		{
		switch (com)
		{
		case 0:
			sb_Write8(0x53, (sb_Read8(0x53) & 0xBF) | (cs << 6));
			break;
		case 1:
			sb_Write(0xA0, (sb_Read(0xA0) & 0xFFBFFFFFL) | ((unsigned long)cs << 22));
			break;
		case 2:
			sb_Write(0xA4, (sb_Read(0xA4) & 0xFFBFFFFFL) | ((unsigned long)cs << 22));
			break;
		case 3:
			sb_Write(0xA8, (sb_Read(0xA8) & 0xFFBFFFFFL) | ((unsigned long)cs << 22));
			break;
		}
		}
		break;
	case CPU_VORTEX86MX:
	case CPU_VORTEX86MX_PLUS:
		{
		switch (com)
		{
		case 0:
			sb_Write8(0x53, (sb_Read8(0x53) & 0xBF) | (cs << 6));
			break;
		case 2:
			sb_Write(0xA4, (sb_Read(0xA4) & 0xFFBFFFFFL) | ((unsigned long)cs << 22));
			break;
		case 3:
			sb_Write(0xA8, (sb_Read(0xA8) & 0xFFBFFFFFL) | ((unsigned long)cs << 22));
			break;
		}
		}
		break;
	}
}

// HCS
DMPAPI(int) vx86_uart_GetHCS(int com)
{
	int cpuid = vx86_CpuID();
	int hcs;
	
	switch (cpuid)
	{
	case CPU_VORTEX86EX:
		{
		unsigned short uart_baseAddr = sb_Read16(0x60) & 0xfffe;
		sb_Write16(0x60, sb_Read16(0x60) | 0x0001);
		
		hcs = (io_inpdw(uart_baseAddr + com*4) >> 20) & 0x01L;
		}
		break;
	case CPU_VORTEX86DX2:
	case CPU_VORTEX86DX3:
		{
		switch (com)
		{
		case 0:
			hcs = (sb_Read(0x54) >> 16) & 0x01L;
			break;
		case 1:
			hcs = (sb_Read(0xA0) >> 20) & 0x01L;
			break;
		case 2:
			hcs = (sb_Read(0xA4) >> 20) & 0x01L;
			break;
		case 3:
			hcs = (sb_Read(0xA8) >> 20) & 0x01L;
			break;
		case 4:
			hcs = (sb1_Read(0xA0) >> 20) & 0x01L;
			break;
		case 5:
			hcs = (sb1_Read(0xA4) >> 20) & 0x01L;
			break;
		case 6:
			hcs = (sb1_Read(0xA8) >> 20) & 0x01L;
			break;
		case 7:
			hcs = (sb1_Read(0xAC) >> 20) & 0x01L;
			break;
		case 8:
			hcs = (sb_Read(0xAC) >> 20) & 0x01L;
			break;
		}
		}
		break;
	}
    
	return hcs;
}

DMPAPI(void) vx86_uart_SetHCS(int com, int hcs)
{
	int cpuid = vx86_CpuID();
	
	switch (cpuid)
	{
	case CPU_VORTEX86EX:
		{
		unsigned short uart_baseAddr = sb_Read16(0x60) & 0xfffe;
		sb_Write16(0x60, sb_Read16(0x60) | 0x0001);

		io_outpdw(uart_baseAddr + com*4, io_inpdw(uart_baseAddr + com*4) & 0xffefffffL |
		                                 ((unsigned long)hcs  << 20));
		}
		break;
	case CPU_VORTEX86DX2:
	case CPU_VORTEX86DX3:
		{
		switch (com)
		{
		case 0:
			sb_Write(0x54, (sb_Read(0x54) & 0xFFFEFFFFL) | ((unsigned long)hcs << 16));
			break;
		case 1:
			sb_Write(0xA0, (sb_Read(0xA0) & 0xFFEFFFFFL) | ((unsigned long)hcs << 20));
			break;
		case 2:
			sb_Write(0xA4, (sb_Read(0xA4) & 0xFFEFFFFFL) | ((unsigned long)hcs << 20));
			break;
		case 3:
			sb_Write(0xA8, (sb_Read(0xA8) & 0xFFEFFFFFL) | ((unsigned long)hcs << 20));
			break;
		case 4:
			sb1_Write(0xA0, (sb1_Read(0xA0) & 0xFFEFFFFFL) | ((unsigned long)hcs << 20));
			break;
		case 5:
			sb1_Write(0xA4, (sb1_Read(0xA4) & 0xFFEFFFFFL) | ((unsigned long)hcs << 20));
			break;
		case 6:
			sb1_Write(0xA8, (sb1_Read(0xA8) & 0xFFEFFFFFL) | ((unsigned long)hcs << 20));
			break;
		case 7:
			sb1_Write(0xAC, (sb1_Read(0xAC) & 0xFFEFFFFFL) | ((unsigned long)hcs << 20));
			break;
		case 8:
			sb_Write(0xAC, (sb_Read(0xAC) & 0xFFEFFFFFL) | ((unsigned long)hcs << 20));
			break;
		}
		}
		break;
	}
}

// SBCLK
DMPAPI(int) vx86_uart_GetSBCLK(void)
{
	int sbclk;
	int cpuid = vx86_CpuID();
	
	switch (cpuid)
	{
	case CPU_VORTEX86EX:
	case CPU_VORTEX86DX2:
	case CPU_VORTEX86DX3:
		sbclk = (sb_Read(0xc0) & 0x80000000L) ? (1) : (0);
		break;
	default:
		sbclk = 0;
		break;
	}
	
    return sbclk;
}

DMPAPI(void) vx86_uart_SetSBCLK(int sbclk)
{
	int cpuid = vx86_CpuID();
	
	switch (cpuid)
	{
	case CPU_VORTEX86EX:
	case CPU_VORTEX86DX2:
	case CPU_VORTEX86DX3:
		sb_Write(0xc0, sb_Read(0xc0) & 0x7fffffffL | ((unsigned long)sbclk << 31));
		break;
	default:
		break;
	}
}

DMP_INLINE(void) vx86_uart_SetCLKMode(int com, int sbclk, int cs, int hcs)
{
    vx86_uart_SetSBCLK(sbclk);
    vx86_uart_SetCS(com, cs);
    vx86_uart_SetHCS(com, hcs);
}

/**************************************************************
*                  Vortex86 UART Initial                      *
**************************************************************/
#define VORTEX86EX_UART_SETS      (10)
#define VORTEX86DX_UART_SETS      (4)
#define VORTEX86DX2_UART_SETS     (9)
#define VORTEX86DX3_UART_SETS     (9)
DMP_INLINE(int) vx86_uart_UARTsets(void)
{
	int cpuid = vx86_CpuID();
	int sets;
	
	switch (cpuid)
	{
	case CPU_VORTEX86EX:
		sets = VORTEX86EX_UART_SETS;
		break;
	case CPU_VORTEX86DX2:
		sets = VORTEX86DX2_UART_SETS;
		break;
	case CPU_VORTEX86DX3:
		sets = VORTEX86DX3_UART_SETS;
		break;
	case CPU_VORTEX86DX_A:
	case CPU_VORTEX86DX_C:
	case CPU_VORTEX86DX_D:
	case CPU_VORTEX86MX:
	case CPU_VORTEX86MX_PLUS:
		sets = VORTEX86DX_UART_SETS;
		break;
	default:
		sets = -1;
		break;
	}
	
    return sets;
}

static int vx86_uart_use[10];
static unsigned long old_vx86_uart_config[10];
static unsigned char old_dx2_ucfg[1];
static bool old_dx_turbo[4];

DMPAPI(bool) vx86_uart_Init(int com)
{
	int cpuid = vx86_CpuID();
	
	if (com < 0 || com >= vx86_uart_UARTsets()) {
		err_print((char*)"%s: There is no COM%d.\n", __FUNCTION__,com + 1);
		return false;
	}
	if ((cpuid == CPU_VORTEX86MX || cpuid == CPU_VORTEX86MX_PLUS) && com == 1) {
		err_print((char*)"%s: There is no COM%d.\n", __FUNCTION__,com + 1);
		return false;
	}

    if (vx86_uart_use[com] != 0) {
        err_print((char*)"%s: COM%d was already opened.\n", __FUNCTION__,com + 1);
        return false;
    }

	if (cpuid == CPU_VORTEX86EX || cpuid == CPU_VORTEX86DX2 || cpuid == CPU_VORTEX86DX3) {

		if (cpuid == CPU_VORTEX86EX) {
			unsigned short uart_baseAddr;
			
			uart_baseAddr = sb_Read16(0x60) & 0xfffe;
			sb_Write16(0x60, sb_Read16(0x60) | 0x0001);
			old_vx86_uart_config[com] = io_inpdw(uart_baseAddr + com*4);
		}
		else {
			switch (com)
			{
			case 0:
				old_dx2_ucfg[com] = sb_Read8(0x53);
				old_vx86_uart_config[com] = sb_Read(0x54);
				break;
			case 1:
				old_vx86_uart_config[com] = sb_Read(0xA0);
				break;
			case 2:
				old_vx86_uart_config[com] = sb_Read(0xA4);
				break;
			case 3:
				old_vx86_uart_config[com] = sb_Read(0xA8);
				break;
			case 4:
				old_vx86_uart_config[com] = sb1_Read(0xA0);
				break;
			case 5:
				old_vx86_uart_config[com] = sb1_Read(0xA4);
				break;
			case 6:
				old_vx86_uart_config[com] = sb1_Read(0xA8);
				break;
			case 7:
				old_vx86_uart_config[com] = sb1_Read(0xAC);
				break;
			case 8:
				old_vx86_uart_config[com] = sb_Read(0xAC);
				break;
			}
		}
	}
	else if (cpuid == CPU_VORTEX86DX_D || cpuid == CPU_VORTEX86MX || cpuid == CPU_VORTEX86MX_PLUS) {
		old_dx_turbo[com] = vx86dx_IsTurboMode(com);
	}
	else if (cpuid == CPU_VORTEX86DX_A || cpuid == CPU_VORTEX86DX_C)
		;
	else
		return false;
	
    vx86_uart_use[com] = 1;
	
    return true;
}

DMPAPI(void) vx86_uart_Close(int com)
{
	int cpuid = vx86_CpuID();
	
	if (com < 0 || com >= vx86_uart_UARTsets())
		return;
	if ((cpuid == CPU_VORTEX86MX || cpuid == CPU_VORTEX86MX_PLUS) && com == 1)
		return;
	if (vx86_uart_use[com] == 0)
		return;
	
	if (cpuid == CPU_VORTEX86EX) {
		unsigned short uart_baseAddr;

		uart_baseAddr = sb_Read16(0x60) & 0xfffe;
		sb_Write16(0x60, sb_Read16(0x60) | 0x0001);
		
		io_outpdw(uart_baseAddr + com*4, old_vx86_uart_config[com]);
	}
	else if (cpuid == CPU_VORTEX86DX2 || cpuid == CPU_VORTEX86DX3) {
		switch (com)
		{
		case 0:
			sb_Write8(0x53, old_dx2_ucfg[com]);
			sb_Write(0x54, old_vx86_uart_config[com]);
			break;
		case 1:
			sb_Write(0xA0, old_vx86_uart_config[com]);
			break;
		case 2:
			sb_Write(0xA4, old_vx86_uart_config[com]);
			break;
		case 3:
			sb_Write(0xA8, old_vx86_uart_config[com]);
			break;
		case 4:
			sb1_Write(0xA0, old_vx86_uart_config[com]);
			break;
		case 5:
			sb1_Write(0xA4, old_vx86_uart_config[com]);
			break;
		case 6:
			sb1_Write(0xA8, old_vx86_uart_config[com]);
			break;
		case 7:
			sb1_Write(0xAC, old_vx86_uart_config[com]);
			break;
		case 8:
			sb_Write(0xAC, old_vx86_uart_config[com]);
			break;
		}
	}
	else if (cpuid == CPU_VORTEX86DX_D || cpuid == CPU_VORTEX86MX || cpuid == CPU_VORTEX86MX_PLUS) {
		if (old_dx_turbo[com] == true)
			vx86_EnableTurboMode(com);
		else
			vx86_DisableTurboMode(com);
	}
	else if (cpuid == CPU_VORTEX86DX_A || cpuid == CPU_VORTEX86DX_C)
		;
	else
		return;
	
    vx86_uart_use[com] = 0;
}

/**************************************************************
*              Vortex86 UART Config Register                  *
**************************************************************/
DMPAPI(unsigned short) vx86_uart_GetBaseAddr(int com)
{
	int cpuid = vx86_CpuID();
	unsigned short addr;
	
	if (cpuid == CPU_VORTEX86EX) {
		unsigned long cfg_data;
		unsigned short uart_baseAddr;
		
		if (com < 0 || com > 9)
			return 0x0000;
		
		uart_baseAddr = sb_Read16(0x60) & 0xfffe;
		sb_Write16(0x60, sb_Read16(0x60) | 0x0001);
		
		cfg_data = io_inpdw(uart_baseAddr + com*4);
		if ((cfg_data & 0x00800000L) == 0L)
			return 0x0000;
	
		addr = (unsigned short)(cfg_data & 0x0000FFFFL);
	}
	else if (cpuid == CPU_VORTEX86DX2 || cpuid == CPU_VORTEX86DX3) {
		if (com < 0 || com > 8)
			return 0;
			
		switch (com)
		{
		case 0:
			addr = sb_Read(0x54) & 0x0000FFFFL;
			break;
		case 1:
			addr = sb_Read(0xA0) & 0x0000FFFFL;
			break;
		case 2:
			addr = sb_Read(0xA4) & 0x0000FFFFL;
			break;
		case 3:
			addr = sb_Read(0xA8) & 0x0000FFFFL;
			break;
		case 4:
			addr = sb1_Read(0xA0) & 0x0000FFFFL;
			break;
		case 5:
			addr = sb1_Read(0xA4) & 0x0000FFFFL;
			break;
		case 6:
			addr = sb1_Read(0xA8) & 0x0000FFFFL;
			break;
		case 7:
			addr = sb1_Read(0xAC) & 0x0000FFFFL;
			break;
		case 8:
			addr = sb_Read(0xAC) & 0x0000FFFFL;
			break;
		}
	}
	else if (cpuid == CPU_VORTEX86DX_A || cpuid == CPU_VORTEX86DX_C || cpuid == CPU_VORTEX86DX_D ||
	         cpuid == CPU_VORTEX86MX   || cpuid == CPU_VORTEX86MX_PLUS) {
		if (com < 0 || com > 3)
			return 0;
		
		if ((cpuid == CPU_VORTEX86MX || cpuid == CPU_VORTEX86MX_PLUS) && com == 1)
			return 0;
			
		switch (com)
		{
		case 0:
			addr = sb_Read(0x54) & 0x0000FFFFL;
			break;
		case 1:
			addr = sb_Read(0xA0) & 0x0000FFFFL;
			break;
		case 2:
			addr = sb_Read(0xA4) & 0x0000FFFFL;
			break;
		case 3:
			addr = sb_Read(0xA8) & 0x0000FFFFL;
			break;
		}
	}
	
	return addr;
}

DMPAPI(void) vx86_uart_SetFIFO16(int com)
{
	unsigned short uart_baseAddr;
	
	if (vx86_CpuID() != CPU_VORTEX86EX)
		return;
    
	uart_baseAddr = sb_Read16(0x60) & 0xfffe;
    sb_Write16(0x60, sb_Read16(0x60) | 0x0001);

    io_outpdw(uart_baseAddr + com*4,
              io_inpdw(uart_baseAddr + com*4) & 0xffdfffffL);
}

DMPAPI(void) vx86_uart_SetFIFO32(int com)
{
	unsigned short uart_baseAddr;
	
	if (vx86_CpuID() != CPU_VORTEX86EX)
		return;
    
	uart_baseAddr = sb_Read16(0x60) & 0xfffe;
    sb_Write16(0x60, sb_Read16(0x60) | 0x0001);

    io_outpdw(uart_baseAddr + com*4,
              io_inpdw(uart_baseAddr + com*4) | 0x00200000L);
}

/**************************************************************
*               Vortex86 IRQ Config Register                  *
**************************************************************/
static int IRQ_route[16]   = {0x00, 0x09, 0x03, 0x0A, 0x04, 0x05, 0x07, 0x06,
                              0x01, 0x0B, 0x00, 0x0C, 0x00, 0x0E, 0x00, 0x0F};

DMPAPI(int) vx86_uart_GetIRQ(int com)
{
	int cpuid = vx86_CpuID();
	int nIRQ;
	unsigned long uirt;
	
	if (cpuid == CPU_VORTEX86EX) {
		unsigned short uart_baseAddr;
		
		if (com < 0 || com > 9)
			return 0;
		
		uart_baseAddr = sb_Read16(0x60) & 0xfffe;
		sb_Write16(0x60, sb_Read16(0x60) | 0x0001);
		
		uirt = ((io_inpdw(uart_baseAddr + com*4) >> 16) & 0x0FL);
	}
	else if (cpuid == CPU_VORTEX86DX2 || cpuid == CPU_VORTEX86DX3) {
		if (com < 0 || com > 8)
			return 0;
			
		switch (com)
		{
		case 0:
			uirt = sb_Read8(0x53) & 0x0F;
			break;
		case 1:
			uirt = (sb_Read(0xA0) >> 16) & 0x0FL;
			break;
		case 2:
			uirt = (sb_Read(0xA4) >> 16) & 0x0FL;
			break;
		case 3:
			uirt = (sb_Read(0xA8) >> 16) & 0x0FL;
			break;
		case 4:
			uirt = (sb1_Read(0xA0) >> 16) & 0x0FL;
			break;
		case 5:
			uirt = (sb1_Read(0xA4) >> 16) & 0x0FL;
			break;
		case 6:
			uirt = (sb1_Read(0xA8) >> 16) & 0x0FL;
			break;
		case 7:
			uirt = (sb1_Read(0xAC) >> 16) & 0x0FL;
			break;
		case 8:
			uirt = (sb_Read(0xAC) >> 16) & 0x0FL;
			break;
		}
	}
	else if (cpuid == CPU_VORTEX86DX_A || cpuid == CPU_VORTEX86DX_C || cpuid == CPU_VORTEX86DX_D ||
	         cpuid == CPU_VORTEX86MX   || cpuid == CPU_VORTEX86MX_PLUS) {
		if (com < 0 || com > 3)
			return 0;
		
		if ((cpuid == CPU_VORTEX86MX || cpuid == CPU_VORTEX86MX_PLUS) && com == 1)
			return 0;
			
		switch (com)
		{
		case 0:
			uirt = sb_Read8(0x53) & 0x0F;
			break;
		case 1:
			uirt = (sb_Read(0xA0) >> 16) & 0x0FL;
			break;
		case 2:
			uirt = (sb_Read(0xA4) >> 16) & 0x0FL;
			break;
		case 3:
			uirt = (sb_Read(0xA8) >> 16) & 0x0FL;
			break;
		}
	}
	else
		return 0;
	
	nIRQ = IRQ_route[uirt];
		
	return nIRQ;
}

DMPAPI(unsigned long) vx86_uart_MaxBPS(int com)
{
	int cpuid = vx86_CpuID();
	
	if (cpuid == CPU_VORTEX86DX2 || cpuid == CPU_VORTEX86DX3 || cpuid == CPU_VORTEX86EX) {
	
		if (cpuid != CPU_VORTEX86EX && (com < 0 || com > 8))
			return 0L;
		if (cpuid == CPU_VORTEX86EX && (com < 0 || com > 9))
			return 0L;
			
			 if (vx86_uart_GetSBCLK() == 1 && vx86_uart_GetCS(com) == 1 && vx86_uart_GetHCS(com) == 1)
				return 6000000L;
		else if (vx86_uart_GetSBCLK() == 1 && vx86_uart_GetCS(com) == 1 && vx86_uart_GetHCS(com) == 0)
				return 3000000L;
		else if (vx86_uart_GetSBCLK() == 0 && vx86_uart_GetCS(com) == 1 && vx86_uart_GetHCS(com) == 0)
				return 1500000L;
		else if (vx86_uart_GetSBCLK() == 1 && vx86_uart_GetCS(com) == 0 && vx86_uart_GetHCS(com) == 0)
				return 115200L;
		else if (vx86_uart_GetSBCLK() == 0 && vx86_uart_GetCS(com) == 0 && vx86_uart_GetHCS(com) == 0)
				return 115200L;
		else return 0L;
	}
	else if (cpuid == CPU_VORTEX86DX_D || cpuid == CPU_VORTEX86MX || cpuid == CPU_VORTEX86MX_PLUS) {
		if (com < 0 || com > 3)
			return 0L;
		
		if ((cpuid == CPU_VORTEX86MX || cpuid == CPU_VORTEX86MX_PLUS) && com == 1)
			return 0;
		
		if (vx86dx_IsTurboMode(com) == true)
			return 1500000L;
		else
			return 115200L;
	}
	else if (cpuid == CPU_VORTEX86DX_A || cpuid == CPU_VORTEX86DX_C) {
		if (com < 0 || com > 3)
			return 0L;
			
		return 115200L;
	}
	else
		return 0L;
}

DMPAPI(void) vx86_uart_EnableHalfDuplex(int com)
{
	int cpuid = vx86_CpuID();
	
	if (cpuid == CPU_VORTEX86EX && com >= 0 && com <= 9) {
		unsigned short uart_baseAddr = sb_Read16(0x60) & 0xfffe;
		sb_Write16(0x60, sb_Read16(0x60) | 0x0001);

		io_outpdw(uart_baseAddr + com*4, io_inpdw(uart_baseAddr + com*4) | 0x02000000L);
	}
}

DMPAPI(void) vx86_uart_EnableFullDuplex(int com)
{
	int cpuid = vx86_CpuID();
	
	if (cpuid == CPU_VORTEX86EX && com >= 0 && com <= 9) {
		unsigned short uart_baseAddr = sb_Read16(0x60) & 0xfffe;
		sb_Write16(0x60, sb_Read16(0x60) | 0x0001);

		io_outpdw(uart_baseAddr + com*4, io_inpdw(uart_baseAddr + com*4) & 0xFDFFFFFFL);
	}
}

DMPAPI(unsigned short) vx86_GetUSBDevAddr(void)
{
	unsigned short addr = 0x00;
	void *pciDev = NULL;

	if (vx86_CpuID() != CPU_VORTEX86EX)
		return 0x0000;
	
	pciDev = pci_Alloc(0x00, 0x0F, 0x00);
	{
		if (pciDev != NULL) addr = (unsigned short)(pci_In32(pciDev, 0x10) & 0xFFFFFF80L);
	}
	pci_Out8(pciDev, 0x04, 0x05);
	pci_Out8(pciDev, 0x05, 0x01);
	pci_Free(pciDev);
	
	return addr;
}

DMPAPI(int) vx86_GetUSBDevIRQ()
{
	int irq = 0;
	void *pciDev = NULL;

	if (vx86_CpuID() != CPU_VORTEX86EX)
		return 0;
	
	pciDev = pci_Alloc(0x00, 0x0F, 0x00);
	{
		if (pciDev != NULL) irq = (int)(pci_In16(pciDev, 0x3C) & 0x00FF);
	}
	pci_Free(pciDev);
	
	return irq;
}

DMPAPI(unsigned short) vx86_GetCANBusIOAddr(void)
{
	unsigned short addr = 0x00;
	void *pciDev = NULL;

	if (vx86_CpuID() != CPU_VORTEX86EX)
		return 0x0000;
	
	pciDev = pci_Alloc(0x00, 0x11, 0x00); // for EX Real CPU
	{
		if (pciDev != NULL)
		{
			addr = (unsigned short)(pci_In32(pciDev, 0x14) & 0x0000FF80L);
			pci_Out16(pciDev, 0x04, pci_In16(pciDev, 0x04) | 0x0003);
		}
	}
	pci_Free(pciDev);

	return addr;
}

DMPAPI(unsigned long) vx86_GetCANBusMMIOAddr(void)
{
	unsigned long addr = 0x00;
	void *pciDev = NULL;
	
	if (vx86_CpuID() != CPU_VORTEX86EX)
		return 0x00UL;
	
	pciDev = pci_Alloc(0x00, 0x11, 0x00); // for EX Real CPU
	{
		if (pciDev != NULL)
		{
			addr = pci_In32(pciDev, 0x10) & 0xFFFFFF80L;
			pci_Out16(pciDev, 0x04, pci_In16(pciDev, 0x04) | 0x0003);
		}
	}
	pci_Free(pciDev);

	return addr;
}

DMPAPI(int) vx86_GetCANBusIRQ(void)
{
	int irq = 0;
	void *pciDev = NULL;
	
	if (vx86_CpuID() != CPU_VORTEX86EX)
		return 0;
	
	pciDev = pci_Alloc(0x00, 0x11, 0x00); // for EX Real CPU
	{
		if (pciDev != NULL) irq = (int)pci_In8(pciDev, 0x3C);
	}
	pci_Free(pciDev);

	return irq;
}
