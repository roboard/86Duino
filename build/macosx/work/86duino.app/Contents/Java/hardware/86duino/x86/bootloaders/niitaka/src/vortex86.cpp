/*
  vortex86.cpp - Part of DM&P Vortex86 Universal Serial library
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

#include <math.h>

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
    case CPU_VORTEX86EX2M:
    case CPU_VORTEX86EX2S:
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

    if ((cpuid == CPU_VORTEX86EX2M || cpuid == CPU_VORTEX86EX2S)) {
        void *pci, *io;
        unsigned long base;
        
        if (pci = pci_Alloc(0x00, 0x06, 0x00)) {
            base = pci_In32(pci, 0x44);
            if (base & 0x01UL) {
                
                base = base & 0x0000FE00UL;
                if (io = io_Alloc(IO_USE_PORTIO, base, 0xFF)) {
                    old_vx86_uart_config[com] = io_In32(io, 4 * com);
                    io_Free(io);
                }
                
            } else {
                
                base = base & 0xFFFFFF00UL;
                if (io = io_Alloc(IO_USE_MMIO, base, 0xFF)) {
                    old_vx86_uart_config[com] = io_In32(io, 4 * com);
                    io_Free(io);
                }
            }
            pci_Free(pci);
        }
    } else if (cpuid == CPU_VORTEX86EX || cpuid == CPU_VORTEX86DX2 || cpuid == CPU_VORTEX86DX3) {

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
	
	if ((cpuid == CPU_VORTEX86EX2M || cpuid == CPU_VORTEX86EX2S)) {
        void *pci, *io;
        unsigned long base;
        
        if (pci = pci_Alloc(0x00, 0x06, 0x00)) {
            base = pci_In32(pci, 0x44);
            if (base & 0x01UL) {
                
                base = base & 0x0000FE00UL;
                if (io = io_Alloc(IO_USE_PORTIO, base, 0xFF)) {
                    io_Out32(io, 4 * com, old_vx86_uart_config[com]);
                    io_Free(io);
                }
                
            } else {
                
                base = base & 0xFFFFFF00UL;
                if (io = io_Alloc(IO_USE_MMIO, base, 0xFF)) {
                    io_Out32(io, 4 * com, old_vx86_uart_config[com]);
                    io_Free(io);
                }
            }
            pci_Free(pci);
        }
    } else if (cpuid == CPU_VORTEX86EX) {
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
	unsigned short addr = 0;
	
	if ((cpuid == CPU_VORTEX86EX2M || cpuid == CPU_VORTEX86EX2S)) {
        void *pci, *io;
		unsigned long cfg_data = 0;
        unsigned long base;
        
        if (com < 0 || com > 9) {
            return 0x0000;
        }
        
        if (pci = pci_Alloc(0x00, 0x06, 0x00)) {
            base = pci_In32(pci, 0x44);
            if (base & 0x01UL) {
                
                base = base & 0x0000FE00UL;
                if (io = io_Alloc(IO_USE_PORTIO, base, 0xFF)) {
                    cfg_data = io_In32(io, 4 * com);
                    io_Free(io);
                }
                
            } else {
                
                base = base & 0xFFFFFF00UL;
                if (io = io_Alloc(IO_USE_MMIO, base, 0xFF)) {
                    cfg_data = io_In32(io, 4 * com);
                    io_Free(io);
                }
            }
            pci_Free(pci);
        }
        if (cfg_data == 0xFFFFFFFFUL)
            return 0x0000;
        if ((cfg_data & 0x00800000UL) == 0L)
            return 0x0000;
        
        addr = (unsigned short)(cfg_data & 0xFFF8UL);
	}
	else if (cpuid == CPU_VORTEX86EX) {
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

DMPAPI(void) vx86_uart_DisableTurboFIFO(int com)
{
    int cpuid = vx86_CpuID();
    
	if ((cpuid == CPU_VORTEX86EX2M || cpuid == CPU_VORTEX86EX2S) && com >= 0 && com <= 9) {
        void *pci, *io;
        unsigned long base;

        if (pci = pci_Alloc(0x00, 0x06, 0x00)) {
            base = pci_In32(pci, 0x44);
            if (base & 0x01UL) {
                
                base = base & 0x0000FE00UL;
                if (io = io_Alloc(IO_USE_PORTIO, base, 0xFF)) {
                    io_Out32(io, 4 * com, io_In32(io, 4 * com) & ~(1UL << 21));
                    io_Free(io);
                }
                
            } else {
                
                base = base & 0xFFFFFF00UL;
                if (io = io_Alloc(IO_USE_MMIO, base, 0xFF)) {
                    io_Out32(io, 4 * com, io_In32(io, 4 * com) & ~(1UL << 21));
                    io_Free(io);
                }
            }
            pci_Free(pci);
        }
	} else if (cpuid == CPU_VORTEX86EX && com >= 0 && com <= 9) {
        unsigned short uart_baseAddr;
        
        uart_baseAddr = sb_Read16(0x60) & 0xfffe;
        sb_Write16(0x60, sb_Read16(0x60) | 0x0001);

        io_outpdw(uart_baseAddr + com*4,
                  io_inpdw(uart_baseAddr + com*4) & 0xffdfffffL);
    }
}

DMPAPI(void) vx86_uart_EnableTurboFIFO(int com)
{
    int cpuid = vx86_CpuID();
	
    if ((cpuid == CPU_VORTEX86EX2M || cpuid == CPU_VORTEX86EX2S) && com >= 0 && com <= 9) {
        void *pci, *io;
        unsigned long base;

        if (pci = pci_Alloc(0x00, 0x06, 0x00)) {
            base = pci_In32(pci, 0x44);
            if (base & 0x01UL) {
                
                base = base & 0x0000FE00UL;
                if (io = io_Alloc(IO_USE_PORTIO, base, 0xFF)) {
                    io_Out32(io, 4 * com, io_In32(io, 4 * com) | (1UL << 21));
                    io_Free(io);
                }
                
            } else {
                
                base = base & 0xFFFFFF00UL;
                if (io = io_Alloc(IO_USE_MMIO, base, 0xFF)) {
                    io_Out32(io, 4 * com, io_In32(io, 4 * com) | (1UL << 21));
                    io_Free(io);
                }
            }
            pci_Free(pci);
        }
	} else if (cpuid == CPU_VORTEX86EX && com >= 0 && com <= 9) {
        unsigned short uart_baseAddr;
        uart_baseAddr = sb_Read16(0x60) & 0xfffe;
        sb_Write16(0x60, sb_Read16(0x60) | 0x0001);

        io_outpdw(uart_baseAddr + com*4,
                  io_inpdw(uart_baseAddr + com*4) | 0x00200000L);
    }
}

DMPAPI(unsigned int) vx86_uart_TruboFIFOSize(int com)
{
    unsigned int size = 16;
    int cpuid = vx86_CpuID();
	
    if (cpuid == CPU_VORTEX86EX2M || cpuid == CPU_VORTEX86EX2S) {
        switch (com)
        {
            case 0: size =  64; break;
            case 1: size =  64; break;
            case 2: size = 128; break;
            case 3: size = 128; break;
            case 4: size =  32; break;
            case 5: size =  32; break;
            case 6: size =  32; break;
            case 7: size =  32; break;
            case 8: size =  32; break;
            case 9: size =  32; break;
        }
	} else if (cpuid == CPU_VORTEX86EX) {
        switch (com)
        {
            case 0: size =  32; break;
            case 1: size =  32; break;
            case 2: size =  32; break;
            case 3: size =  32; break;
            case 4: size =  32; break;
            case 5: size =  32; break;
            case 6: size =  32; break;
            case 7: size =  32; break;
            case 8: size = 128; break;
            case 9: size = 128; break;
        }
    } else {
    }
    
    return size;
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
	
	if ((cpuid == CPU_VORTEX86EX2M || cpuid == CPU_VORTEX86EX2S)) {
        void *pci, *io;
        unsigned long base;
        
        if (com < 0 || com > 9) {
            return 0;
        }

        if (pci = pci_Alloc(0x00, 0x06, 0x00)) {
            base = pci_In32(pci, 0x44);
            if (base & 0x01UL) {
                
                base = base & 0x0000FE00UL;
                if (io = io_Alloc(IO_USE_PORTIO, base, 0xFF)) {
                    uirt = io_In32(io, 4 * com);
                    io_Free(io);
                }
                
            } else {
                
                base = base & 0xFFFFFF00UL;
                if (io = io_Alloc(IO_USE_MMIO, base, 0xFF)) {
                    uirt = io_In32(io, 4 * com);
                    io_Free(io);
                }
            }
            pci_Free(pci);
            if (uirt == 0xFFFFFFFFUL)
                uirt = 0;
            else
                uirt = (uirt >> 16) & 0x0FUL;
        }
	}
	else if (cpuid == CPU_VORTEX86EX) {
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

typedef struct baud_config {
    unsigned long baud;
    unsigned long info;
} BAUD_CONFIG;

BAUD_CONFIG v86bps[] = {
    {6000000UL, 0xC001FFFFUL},
    {3000000UL, 0xC002FFFFUL},
    {2000000UL, 0xC003FFFFUL},
    {1500000UL, 0xC004FFFFUL},
    {1000000UL, 0xC006FFFFUL},
    { 750000UL, 0xC0088002UL},
    { 500000UL, 0xC00C8003UL},
    { 461538UL, 0xC00DFFFFUL},
    { 333333UL, 0xC012FFFFUL},
    { 300000UL, 0xC0148005UL},
    { 250000UL, 0x800C8006UL},
    { 230400UL, 0x800DFFFFUL},
    { 200000UL, 0x800FFFFFUL},
    { 150000UL, 0x8014800AUL},
    { 125000UL, 0x8018800CUL},
    { 115200UL, 0x00010001UL},
    {  57600UL, 0x00020002UL},
    {  38400UL, 0x00030003UL},
    {  28800UL, 0x00040004UL},
    {  19200UL, 0x00060006UL},
    {  14400UL, 0x00080008UL},
    {   9600UL, 0x000C000CUL},
    {   4800UL, 0x00180018UL},
    {   2400UL, 0x00300030UL},
    {   1200UL, 0x00600060UL},
    {    800UL, 0x00900090UL},
    {    600UL, 0x00C000C0UL},
    {    300UL, 0x01800180UL},
    {    200UL, 0x02400240UL},
    {     80UL, 0x05A005A0UL},
    {     50UL, 0x09000900UL},
    {     40UL, 0x0B400B40UL}
};

BAUD_CONFIG ex2bps[] = {
    {20000000UL, 0x00040006UL},
    {15000000UL, 0x00040008UL},
    {12500000UL, 0x00050008UL},
    {12000000UL, 0x0004000AUL},
    {10000000UL, 0x0004000CUL},
    { 8000000UL, 0x0004000FUL},
    { 7500000UL, 0x00040010UL},
    { 6250000UL, 0x00050010UL},
    { 6000000UL, 0x00040014UL},
    { 5000000UL, 0x00050014UL},
    { 4000000UL, 0x00050019UL},
    { 3000000UL, 0x00040028UL},
    { 2500000UL, 0x00050028UL},
    { 2000000UL, 0x0004003CUL},
    { 1500000UL, 0x00040050UL},
    { 1000000UL, 0x00040078UL},
    {  750000UL, 0x000400A0UL},
    {  500000UL, 0x000400F0UL},
    {  461538UL, 0x00040104UL},
    {  333333UL, 0x00030064UL},
    {  300000UL, 0x00040190UL},
    {  250000UL, 0x000401E0UL},
    {  230400UL, 0x00020001UL},
    {  200000UL, 0x00040258UL},
    {  150000UL, 0x00040320UL},
    {  125000UL, 0x000403C0UL},
    {  115200UL, 0x00010001UL},
    {   57600UL, 0x00010002UL},
    {   38400UL, 0x00010003UL},
    {   28800UL, 0x00010004UL},
    {   19200UL, 0x00010006UL},
    {   14400UL, 0x00010008UL},
    {    9600UL, 0x0001000CUL},
    {    4800UL, 0x00010018UL},
    {    2400UL, 0x00010030UL},
    {    1200UL, 0x00010060UL},
    {     800UL, 0x00010090UL},
    {     600UL, 0x000100C0UL},
    {     300UL, 0x00010180UL},
    {     200UL, 0x00010240UL},
    {      80UL, 0x000105A0UL},
    {      50UL, 0x00010900UL},
    {      40UL, 0x00010B40UL}
};

DMPAPI(void) vx86_SetUartFormula(int com, int formula)
{
    if ((vx86_CpuID() == CPU_VORTEX86EX2M || vx86_CpuID() == CPU_VORTEX86EX2S)) {
        void *pci, *io;
        unsigned long base;
        
        if (pci = pci_Alloc(0x00, 0x06, 0x00)) {
            base = pci_In32(pci, 0x44);
            if (base & 0x01UL) {
                
                base = base & 0x0000FE00UL;
                if (io = io_Alloc(IO_USE_PORTIO, base, 0xFF)) {
                    io_Out32(io, 4 * com, io_In32(io, 4 * com) & 0x1FFFFFFFUL | ((unsigned long)formula << 29));
                    io_Free(io);
                }
                
            } else {
                
                base = base & 0xFFFFFF00UL;
                if (io = io_Alloc(IO_USE_MMIO, base, 0xFF)) {
                    io_Out32(io, 4 * com, io_In32(io, 4 * com) & 0x1FFFFFFFUL | ((unsigned long)formula << 29));
                    io_Free(io);
                }
            }
            pci_Free(pci);
        }
    }
}

DMPAPI(int) vx86_GetUartFormula(int com)
{
    int formula = -1;
    
    if ((vx86_CpuID() == CPU_VORTEX86EX2M || vx86_CpuID() == CPU_VORTEX86EX2S)) {
        void *pci, *io;
        unsigned long base;
        
        if (pci = pci_Alloc(0x00, 0x06, 0x00)) {
            base = pci_In32(pci, 0x44);
            if (base & 0x01UL) {
                
                base = base & 0x0000FE00UL;
                if (io = io_Alloc(IO_USE_PORTIO, base, 0xFF)) {
                    formula = (io_In32(io, 4 * com) >> 29) & 0x07;
                    io_Free(io);
                }
                
            } else {
                
                base = base & 0xFFFFFF00UL;
                if (io = io_Alloc(IO_USE_MMIO, base, 0xFF)) {
                    formula = (io_In32(io, 4 * com) >> 29) & 0x07;
                    io_Free(io);
                }
            }
            pci_Free(pci);
        }
    }
    
    return formula;
}

DMPAPI(unsigned long) vx86_GetBaudClock(int com)
{
    unsigned long maxbaud = 115384UL;
    
    switch (vx86_GetUartFormula(com)) {
    case VEX2_FORMULAR1: maxbaud =     115384UL; break;
    case VEX2_FORMULAR2: maxbaud =     230769UL; break;
    case VEX2_FORMULAR3: maxbaud =   33333333UL; break;
    case VEX2_FORMULAR4: maxbaud =  120000000UL; break;
    case VEX2_FORMULAR5: maxbaud =  100000000UL; break;
    }
    
    return maxbaud;
}

DMPAPI(unsigned long) vx86_GetUartDiv(int com, unsigned long baud)
{
	unsigned long divisor = 0;
    unsigned long info = 0xFFFFFFFFUL;
    int i, size;
	
    if ((vx86_CpuID() == CPU_VORTEX86EX2M || vx86_CpuID() == CPU_VORTEX86EX2S)) {
        size = sizeof(ex2bps)/sizeof(ex2bps[0]);
        
        for (i = 0; i < size; i++) {
            if (baud == ex2bps[i].baud) {
                info = ex2bps[i].info;
                break;
            }
        }
        
        if (info != 0xFFFFFFFFUL) {
            
            switch ((info >> 16) & 0x0F) {
                
            case 1: vx86_SetUartFormula(com, VEX2_FORMULAR1); break;
            case 2: vx86_SetUartFormula(com, VEX2_FORMULAR2); break;
            case 3: vx86_SetUartFormula(com, VEX2_FORMULAR3); break;
            case 4: vx86_SetUartFormula(com, VEX2_FORMULAR4); break;
            case 5: vx86_SetUartFormula(com, VEX2_FORMULAR5); break;
            
            }
            
            divisor = info & 0xFFFFUL;
        }
    } else {
        size = sizeof(v86bps)/sizeof(v86bps[0]);
        
        for (i = 0; i < size; i++) {
            if (baud == v86bps[i].baud) {
                info = v86bps[i].info;
                break;
            }
        }
        
        if (vx86_uart_GetSBCLK() != 0) {
            info = (info >> 16) & 0xFFFFUL;
        } else {
            info = info & 0xFFFFUL;
        }
        
        if (info != 0xFFFFUL) {
            vx86_uart_SetCS(com, (info & 0x8000) >> 15);
            vx86_uart_SetHCS(com, (info & 0x4000) >> 14);
            
            divisor = info & 0x3FFFUL;
        }
    }
    
    return divisor;
}

DMPAPI(unsigned long) vx86_GetUartDivEX(int com, unsigned long baud)
{
	unsigned long divisor = 0;
	
    if ((vx86_CpuID() == CPU_VORTEX86EX2M || vx86_CpuID() == CPU_VORTEX86EX2S)) {
        divisor = (unsigned long)round((double)vx86_GetBaudClock(com)/baud);
        switch (vx86_GetUartFormula(com)) {
        case VEX2_FORMULAR3:
        case VEX2_FORMULAR4:
        case VEX2_FORMULAR5:
            if (divisor < 6) {
                divisor = 0;
            }
            break;
        }
        if (divisor > 0xFFFFUL) {
            divisor = 0;
        }
    } else {
        int i, size = sizeof(v86bps)/sizeof(v86bps[0]);
        unsigned long info = 0xFFFFFFFFUL;
        
        for (i = 0; i < size; i++) {
            if (baud == v86bps[i].baud) {
                info = v86bps[i].info;
                break;
            }
        }
        
        if (vx86_uart_GetSBCLK() != 0) {
            info = (info >> 16) & 0xFFFFUL;
        } else {
            info = info & 0xFFFFUL;
        }
        
        if (info != 0xFFFFUL) {
            
            divisor = info & 0x3FFFUL;
        }
    }
    
    return divisor;
}

DMPAPI(void) vx86_uart_EnableHalfDuplex(int com)
{
	int cpuid = vx86_CpuID();
	
	if ((cpuid == CPU_VORTEX86EX2M || cpuid == CPU_VORTEX86EX2S) && com >= 0 && com <= 9) {
        void *pci, *io;
        unsigned long base;

        if (pci = pci_Alloc(0x00, 0x06, 0x00)) {
            base = pci_In32(pci, 0x44);
            if (base & 0x01UL) {
                
                base = base & 0x0000FE00UL;
                if (io = io_Alloc(IO_USE_PORTIO, base, 0xFF)) {
                    io_Out32(io, 4 * com, io_In32(io, 4 * com) | (1UL << 25));
                    io_Free(io);
                }
                
            } else {
                
                base = base & 0xFFFFFF00UL;
                if (io = io_Alloc(IO_USE_MMIO, base, 0xFF)) {
                    io_Out32(io, 4 * com, io_In32(io, 4 * com) | (1UL << 25));
                    io_Free(io);
                }
            }
            pci_Free(pci);
        }
	} else if (cpuid == CPU_VORTEX86EX && com >= 0 && com <= 9) {
		unsigned short uart_baseAddr = sb_Read16(0x60) & 0xfffe;
		sb_Write16(0x60, sb_Read16(0x60) | 0x0001);

		io_outpdw(uart_baseAddr + com*4, io_inpdw(uart_baseAddr + com*4) | 0x02000000L);
	}
}

DMPAPI(void) vx86_uart_EnableFullDuplex(int com)
{
	int cpuid = vx86_CpuID();
	
	if ((cpuid == CPU_VORTEX86EX2M || cpuid == CPU_VORTEX86EX2S) && com >= 0 && com <= 9) {
        void *pci, *io;
        unsigned long base;

        if (pci = pci_Alloc(0x00, 0x06, 0x00)) {
        
            base = pci_In32(pci, 0x44);
            if (base & 0x01UL) {
                
                base = base & 0x0000FE00UL;
                if (io = io_Alloc(IO_USE_PORTIO, base, 0xFF)) {
                    io_Out32(io, 4 * com, io_In32(io, 4 * com) & ~(1UL << 25));
                    io_Free(io);
                }
                
            } else {
                
                base = base & 0xFFFFFF00UL;
                if (io = io_Alloc(IO_USE_MMIO, base, 0xFF)) {
                    io_Out32(io, 4 * com, io_In32(io, 4 * com) & ~(1UL << 25));
                    io_Free(io);
                }
            }
            pci_Free(pci);
        }
	} else if (cpuid == CPU_VORTEX86EX && com >= 0 && com <= 9) {
		unsigned short uart_baseAddr = sb_Read16(0x60) & 0xfffe;
		sb_Write16(0x60, sb_Read16(0x60) | 0x0001);

		io_outpdw(uart_baseAddr + com*4, io_inpdw(uart_baseAddr + com*4) & 0xFDFFFFFFL);
	}
}

DMPAPI(void) vx86_SetUartMove2ShiftNormalMode(int com)
{
    int cpuid = vx86_CpuID();
    
    if ((cpuid == CPU_VORTEX86EX2M || cpuid == CPU_VORTEX86EX2S) && com >= 0 && com <= 9) {
        void *pci, *io;
        unsigned long base;

        if (pci = pci_Alloc(0x00, 0x06, 0x00)) {
        
            base = pci_In32(pci, 0x44);
            if (base & 0x01UL) {
                
                base = base & 0x0000FE00UL;
                if (io = io_Alloc(IO_USE_PORTIO, base, 0xFF)) {
                    io_Out32(io, 4 * com, io_In32(io, 4 * com) | (1UL << 28));
                    io_Free(io);
                }
                
            } else {
                
                base = base & 0xFFFFFF00UL;
                if (io = io_Alloc(IO_USE_MMIO, base, 0xFF)) {
                    io_Out32(io, 4 * com, io_In32(io, 4 * com) | (1UL << 28));
                    io_Free(io);
                }
            }
            pci_Free(pci);
        }
	}
}

DMPAPI(void) vx86_SetUartMove2ShiftFastMode(int com)
{
    int cpuid = vx86_CpuID();
    
    if ((cpuid == CPU_VORTEX86EX2M || cpuid == CPU_VORTEX86EX2S) && com >= 0 && com <= 9) {
        void *pci, *io;
        unsigned long base;

        if (pci = pci_Alloc(0x00, 0x06, 0x00)) {
        
            base = pci_In32(pci, 0x44);
            if (base & 0x01UL) {
                
                base = base & 0x0000FE00UL;
                if (io = io_Alloc(IO_USE_PORTIO, base, 0xFF)) {
                    io_Out32(io, 4 * com, io_In32(io, 4 * com) & ~(1UL << 28));
                    io_Free(io);
                }
                
            } else {
                
                base = base & 0xFFFFFF00UL;
                if (io = io_Alloc(IO_USE_MMIO, base, 0xFF)) {
                    io_Out32(io, 4 * com, io_In32(io, 4 * com) & ~(1UL << 28));
                    io_Free(io);
                }
            }
            pci_Free(pci);
        }
	}
}


DMPAPI(unsigned long) vx86_GetUartFastAccessBase()
{
    void *pci;
    int cpuid = vx86_CpuID();
    
    if (cpuid == CPU_VORTEX86EX2M || cpuid == CPU_VORTEX86EX2S) {

        if (pci = pci_Alloc(0x00, 0x06, 0x00)) {
            
            return pci_In32(pci, 0x44) & 0x0000FE00UL;
        }
        
        pci_Free(pci);
	}
    
    return 0xFFFFFFFFUL;
}

DMPAPI(unsigned int) vx86_GetUartTxFIFOFree(int com, unsigned long base)
{
    return io_inpdw(base + 0x3C + 0x10 * com) & 0xFF;
}

DMPAPI(unsigned int) vx86_GetUartRxFIFORcvd(int com, unsigned long base)
{
    return (io_inpdw(base + 0x3C + 0x10 * com) >> 16) & 0xFF;
}

DMPAPI(void) vx86_UartFastWrite(int com, unsigned long base, unsigned long value)
{
    io_outpdw(base + 0x30 + 0x10 * com, value);
}

DMPAPI(void) vx86_UartFastWrite16(int com, unsigned long base, unsigned short value)
{
    io_outpw(base + 0x30 + 0x10 * com, value);
}

DMPAPI(void) vx86_UartFastWrite8(int com, unsigned long base, unsigned char value)
{
    io_outpb(base + 0x30 + 0x10 * com, value);
}

DMPAPI(unsigned long) vx86_UartFastMode0Read(int com, unsigned long base)
{
    return io_inpdw(base + 0x34 + 0x10 * com);
}

DMPAPI(unsigned short) vx86_UartFastMode0Read16(int com, unsigned long base)
{
    return io_inpw(base + 0x34 + 0x10 * com);
}

DMPAPI(unsigned long) vx86_UartFastMode1Read(int com, unsigned long base)
{
    return io_inpdw(base + 0x38 + 0x10 * com);
}

DMPAPI(unsigned short) vx86_UartFastMode1Read16(int com, unsigned long base)
{
    return io_inpw(base + 0x38 + 0x10 * com);
}

DMPAPI(unsigned char) vx86_UartFastMode1Read8(int com, unsigned long base)
{
    return io_inpb(base + 0x38 + 0x10 * com);
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

DMPAPI(unsigned short) vx86_GetCANBusIOAddr(int can)
{
    int cpuid;
	unsigned short addr = 0x00;
	void *pciDev = NULL;

    cpuid = vx86_CpuID();
    switch (cpuid) {
    case CPU_VORTEX86EX:
        if (can == 0)
            pciDev = pci_Alloc(0x00, 0x11, 0x00);
        break;
    case CPU_VORTEX86EX2M:
    case CPU_VORTEX86EX2S:
        if (can == 0) {
            pciDev = pci_Alloc(0x00, 0x12, 0x00);
        } else if (can == 1) {
            pciDev = pci_Alloc(0x00, 0x12, 0x01);
        }
        break;
    default:
        break;
    }
	
    if (pciDev) {
        addr = pci_In16(pciDev, 0x14) & 0xFF80;
        pci_Out16(pciDev, 0x04, pci_In16(pciDev, 0x04) | 0x0003);
        pci_Free(pciDev);
    }

	return addr;
}

DMPAPI(unsigned long) vx86_GetCANBusMMIOAddr(int can)
{
    int cpuid;
	unsigned long addr = 0x00;
	void *pciDev = NULL;

    cpuid = vx86_CpuID();
    switch (cpuid) {
    case CPU_VORTEX86EX:
        pciDev = pci_Alloc(0x00, 0x11, 0x00);
        break;
    case CPU_VORTEX86EX2M:
    case CPU_VORTEX86EX2S:
        if (can == 0) {
            pciDev = pci_Alloc(0x00, 0x12, 0x00);
        } else if (can == 1) {
            pciDev = pci_Alloc(0x00, 0x12, 0x01);
        }
        break;
    default:
        break;
    }
	
    if (pciDev) {
        addr = pci_In32(pciDev, 0x10) & 0xFFFFFF80UL;
        pci_Out16(pciDev, 0x04, pci_In16(pciDev, 0x04) | 0x0003);
        pci_Free(pciDev);
    }

	return addr;
}

DMPAPI(int) vx86_GetCANBusIRQ(int can)
{
    int cpuid;
	int irq = 0;
	void *pciDev = NULL;

    cpuid = vx86_CpuID();
    switch (cpuid) {
    case CPU_VORTEX86EX:
        pciDev = pci_Alloc(0x00, 0x11, 0x00);
        break;
    case CPU_VORTEX86EX2M:
    case CPU_VORTEX86EX2S:
        if (can == 0) {
            pciDev = pci_Alloc(0x00, 0x12, 0x00);
        } else if (can == 1) {
            pciDev = pci_Alloc(0x00, 0x12, 0x01);
        }
        break;
    default:
        break;
    }
	
    if (pciDev) {
        irq = pci_In8(pciDev, 0x3C);
        pci_Free(pciDev);
    }

	return irq;
}

static unsigned long excan_bps_info[] =  { 0x80021144UL,    // 1M
                                           0x80021155UL,    // 833K
                                           0xFFFFFFFFUL,
                                           0xFFFFFFFFUL,
                                           0x80021388UL,    // 500K
                                           0x80041388UL,    // 250K
                                           0x80041888UL,    // 200K
                                           0x80081388UL,    // 125K
                                           0x800A1388UL,    // 100K
                                           0x800C1388UL,    // 83K3
                                           0xFFFFFFFFUL,
                                           0xFFFFFFFFUL,
                                           0x80141388UL,    // 50K
                                           0x80281388UL,    // 25K
                                           0x80321388UL,    // 20K
                                           0x80501888UL     // 10K
                                         };
static unsigned long ex2can_bps_info[] = { 0x80041155UL,    // 1M
                                           0xFFFFFFFFUL,
                                           0x80061144UL,    // 800K
                                           0x80081144UL,    // 600K
                                           0x80081155UL,    // 500K
                                           0x80101155UL,    // 250K
                                           0x80141155UL,    // 200K
                                           0x80201155UL,    // 125K
                                           0x80181577UL,    // 100K
                                           0xFFFFFFFFUL,
                                           0x801E1577UL,    // 80K
                                           0x80281388UL,    // 60K
                                           0x80301388UL,    // 50K
                                           0x80601388UL,    // 25K
                                           0x80601888UL,    // 20K
                                           0x80C01888UL     // 10K
                                         };
DMPAPI(unsigned long) vx86_GetCANBusBpsInfo(unsigned long bps)
{
    int cpuid;
	unsigned long info = 0xFFFFFFFFUL;
    unsigned long *can_bps_info = NULL;

    cpuid = vx86_CpuID();
    switch (cpuid) {
    case CPU_VORTEX86EX:
        can_bps_info = excan_bps_info;
        break;
    case CPU_VORTEX86EX2M:
    case CPU_VORTEX86EX2S:
        can_bps_info = ex2can_bps_info;
        break;
    }
    
    switch (bps) {
    case 1000000UL:
        if (can_bps_info) {
            info = can_bps_info[0];
        }
        break;
    case  833333UL:
        if (can_bps_info) {
            info = can_bps_info[1];
        }
        break;
    case  800000UL:
        if (can_bps_info) {
            info = can_bps_info[2];
        }
        break;
    case  600000UL:
        if (can_bps_info) {
            info = can_bps_info[3];
        }
        break;
    case  500000UL:
        if (can_bps_info) {
            info = can_bps_info[4];
        }
        break;
    case  250000UL:
        if (can_bps_info) {
            info = can_bps_info[5];
        }
        break;
    case  200000UL:
        if (can_bps_info) {
            info = can_bps_info[6];
        }
        break;
    case  125000UL:
        if (can_bps_info) {
            info = can_bps_info[7];
        }
        break;
    case  100000UL:
        if (can_bps_info) {
            info = can_bps_info[8];
        }
        break;
    case   83333UL:
        if (can_bps_info) {
            info = can_bps_info[9];
        }
        break;
    case   80000UL:
        if (can_bps_info) {
            info = can_bps_info[10];
        }
        break;
    case   60000UL:
        if (can_bps_info) {
            info = can_bps_info[11];
        }
        break;
    case   50000UL:
        if (can_bps_info) {
            info = can_bps_info[12];
        }
        break;
    case   25000UL:
        if (can_bps_info) {
            info = can_bps_info[13];
        }
        break;
    case   20000UL:
        if (can_bps_info) {
            info = can_bps_info[14];
        }
        break;
    case   10000UL:
        if (can_bps_info) {
            info = can_bps_info[15];
        }
        break;
    default:
        break;
    }
    
    return info;
}
