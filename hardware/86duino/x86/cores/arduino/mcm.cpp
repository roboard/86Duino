/*
  mcm.cpp - DM&P Vortex86DX2/EX MCM library
  Copyright (c) 2013 AAA <aaa@dmp.com.tw>. All right reserved.

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

  Modified 01 November 2013 by Android Lin, DY Hung, and Hellion Chuang
  Modified 01 January 2015 by Android Lin
  
  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)
*/


#include <stdio.h>
#include <conio.h>
#include <dos.h>

#include "io.h"
#include "mcm.h"

#include <go32.h>
#include <dpmi.h>
#include <sys/farptr.h>
#include <pc.h>
#include <dos.h>


static bool MC_useMMIO = false;

DMPAPI(void) set_MMIO(void) {
#if defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM)
    MC_useMMIO = true;
#endif
}

DMPAPI(void) set_PortIO(void) {
#if defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM)
    MC_useMMIO = false;
#endif
}


static unsigned PWM_baseAddr = 0;
static unsigned long MMIO_baseAddr = 0L;
static void* io_handle = NULL;
static void* mcm_p = NULL;
DMPAPI(int) mcmInit(void) {
	if(mcm_p != NULL) return 0;
    mcm_p = pci_Alloc(0, 0x10, 0);
	if(mcm_p == NULL)
	{
		printf("Alloc PCI config fail.\n");
		return -1;
	}

	#if defined (DMP_LINUX) || defined (DMP_WIN32_MSVC)
        MMIO_baseAddr = pci_In32(mcm_p, 0x14);
	    io_handle = io_Alloc(IO_USE_MMIO, MMIO_baseAddr, 0x380);
		if(io_handle == NULL)
		{
			printf("Alloc MCM MMIO space fail.");
			return -1;
		}
        MC_useMMIO = true;
	#elif defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM)
		if(MC_useMMIO == true)
		{
			MMIO_baseAddr = pci_In32(mcm_p, 0x14);
			io_handle = io_Alloc(IO_USE_MMIO, MMIO_baseAddr, 0x380);
			if(io_handle == NULL)
			{
				printf("Alloc MCM MMIO space fail.");
				return -1;
			}
		}
		else
		{
			PWM_baseAddr = (unsigned)(pci_In32(mcm_p, 0x10) & 0x0000fff0L);
			io_handle = io_Alloc(IO_USE_PORTIO, PWM_baseAddr, 0xD4);
			if(io_handle == NULL)
			{
				printf("Alloc MCM IO space fail.");
				return -1;
			}
		}
	#else
		PWM_baseAddr = (unsigned)(pci_In32(mcm_p, 0x10) & 0x0000fff0L);
		io_handle = io_Alloc(IO_USE_PORTIO, PWM_baseAddr, 0xD4);
		if(io_handle == NULL)
		{
			printf("Alloc MCM IO space fail.");
			return -1;
		}
        MC_useMMIO = false;
	#endif
	return 0;
}

DMPAPI(void) mcmClose(void) {
    PWM_baseAddr = 0L;
    MMIO_baseAddr = 0L;
	pci_Free(mcm_p);
	io_Free(io_handle);
}

DMPAPI(unsigned long) mc_getbaseaddr(void) {
    if(MC_useMMIO == true)
		return MMIO_baseAddr;
	else
		return PWM_baseAddr;
}

DMPAPI(int) mc_getIRQ(void) {
	return (int)(pci_In32(mcm_p, 0x3c) & 0xFF);
}

DMPAPI(void) mc_outp(int mc, unsigned long idx, unsigned long val) {
    // if (USE_8051B == true) { mc8051_outp(mc, idx, val); return; }
	if(io_handle == NULL) return;

    if (MC_useMMIO == true)
    {
		if (mc == MC_GENERAL)
            io_Out32(io_handle, idx, val);
        else
            io_Out32(io_handle, 0x40L + (unsigned long)mc * 0xd0L + idx, val);
    }
    else
    {   
        io_DisableINT();
        io_Out32(io_handle, 0xd0, 1UL<<(mc+1));  // paging to corresponding reg window
        io_Out32(io_handle, (unsigned)idx, val);
        io_RestoreINT();
    }
}

DMPAPI(unsigned long) mc_inp(int mc, unsigned long idx) {
    unsigned long tmp;
    // if (USE_8051B == true) return mc8051_inp(mc, idx);
    if(io_handle == NULL) return 0L;

    if (MC_useMMIO == true)
    {
		if (mc == MC_GENERAL)
            return io_In32(io_handle, idx);
        else
            return io_In32(io_handle, 0x40L + (unsigned long)mc * 0xd0L + idx);
    }
    else
    {   
        io_DisableINT();
        io_Out32(io_handle, 0xd0, 1UL<<(mc+1));  // paging to corresponding reg window
        tmp = io_In32(io_handle, (unsigned)idx);
        io_RestoreINT();
        return tmp;
    }
}

DMPAPI(void) mc_outpb(int mc, unsigned long idx, unsigned char val) {
    // if (USE_8051B == true) { mc8051_outpb(mc, idx, val); return; }
    if(io_handle == NULL) return;

    if (MC_useMMIO == true)
    {
        if (mc == MC_GENERAL)
            io_Out8(io_handle, idx, val);
        else
            io_Out8(io_handle, 0x40L + (unsigned long)mc * 0xd0L + idx, val);
    }
    else
    {   
        io_DisableINT();
        io_Out32(io_handle, 0xd0, 1UL<<(mc+1));  // paging to corresponding reg window
        io_Out8(io_handle, (unsigned)idx, val);
        io_RestoreINT();
    }
}

DMPAPI(unsigned char) mc_inpb(int mc, unsigned long idx) {
    unsigned char tmp;
    // // if (USE_8051B == true) return mc8051_inpb(mc, idx);
    if(io_handle == NULL) return 0L;

    if (MC_useMMIO == true)
    {
        if (mc == MC_GENERAL)
            return io_In8(io_handle, idx);
        else
            return io_In8(io_handle, 0x40L + (unsigned long)mc * 0xd0L + idx);
        return 0;
    }
    else
    {   
        io_DisableINT();
        io_Out32(io_handle, 0xd0, 1UL<<(mc+1));  // paging to corresponding reg window
        tmp = io_In8(io_handle, (unsigned)idx);
        io_RestoreINT();
        return tmp;
    }
}

/*----------------------------------------------------------------------------*/



/***************************  General Settings  *******************************/

// MC General Registers
#define MCG_MODEREG1        (0x00L)
#define MCG_MODEREG2        (0x04L)
#define MCG_ENABLEREG1      (0x08L)
#define MCG_ENABLEREG2      (0x0cL)
#define MCG_PROTECTREG1     (0x10L)
#define MCG_PROTECTREG2     (0x14L)
#define MCG_LDRDYSYNCREG1   (0x18L)
#define MCG_LDRDYSYNCREG2   (0x1cL)
#define MCG_G1ROUTEREG1     (0x20L)
#define MCG_G1ROUTEREG2     (0x24L)
#define MCG_G2ROUTEREG1     (0x28L)
#define MCG_G2ROUTEREG2     (0x2cL)
#define MCG_G3ROUTEREG1     (0x30L)
#define MCG_G3ROUTEREG2     (0x34L)
#define MCG_INTMASKREG      (0x38L)     // 16-bit
#define MCG_INTSTATREG      (0x3aL)     // 16-bit
#define MCG_UNLOCKREG1      (0x3cL)     // 16-bit
#define MCG_UNLOCKREG2      (0x3eL)     // 16-bit

DMPAPI(bool) mc_IsEnabled(int mc) {
    unsigned long reg1 = mc_inp(MC_GENERAL, MCG_ENABLEREG1);
    unsigned long reg2 = mc_inp(MC_GENERAL, MCG_ENABLEREG2);
    int i;

    if ((reg2 & (1L << (mc+4))) != 0L) return true;     // check SIFBEN

    for (i=0; i<3; i++)  // check PWMAEN/SERVOAEN/SIFAEN, PWMBEN/SERVOBEN, PWMCEN/SERVOCEN
    {
        int tmp = mc*3 + i;
        if (tmp <  32 && ((reg1 & (0x01L << tmp)) != 0L)) return true;
        if (tmp >= 32 && ((reg2 & (0x01L << (tmp-32))) != 0L)) return true;
    }
    
    return false;
}

DMPAPI(bool) mc_SetMode(int mc, unsigned long mode) {
    unsigned long mask = ~(0x0fL << ((mc & 0x07) * 4));
    unsigned long val  = mode << ((mc & 0x07) * 4);
    unsigned long reg  = (mc < 8)? MCG_MODEREG1 : MCG_MODEREG2;

    if (mc_IsEnabled(mc) == true)
    {
        //printf("ERROR: can't set MC mode when some module is enabled!\n");
        return false;
    }

    mc_outp(MC_GENERAL, reg, (mc_inp(MC_GENERAL, reg) & mask) | val);
    return true;
}


DMPAPI(void) mc_LockLDRDYs(unsigned long group1, unsigned long group2, unsigned long group3) {
    unsigned long reg1 = group1 | (group2 << 12) | (group3 << 24);
    unsigned long reg2 = group3 >> 8;

    mc_outp(MC_GENERAL, MCG_LDRDYSYNCREG1, mc_inp(MC_GENERAL, MCG_LDRDYSYNCREG1) & ~reg1);
    mc_outp(MC_GENERAL, MCG_LDRDYSYNCREG2, mc_inp(MC_GENERAL, MCG_LDRDYSYNCREG2) & ~reg2);
}

DMPAPI(void) mc_UnlockLDRDYs(unsigned long group1, unsigned long group2, unsigned long group3) {
    unsigned long reg1 = group1 | (group2 << 12) | (group3 << 24);
    unsigned long reg2 = group3 >> 8;

    mc_outp(MC_GENERAL, MCG_LDRDYSYNCREG1, mc_inp(MC_GENERAL, MCG_LDRDYSYNCREG1) | reg1);
    mc_outp(MC_GENERAL, MCG_LDRDYSYNCREG2, mc_inp(MC_GENERAL, MCG_LDRDYSYNCREG2) | reg2);
}

DMPAPI(void) mc_LockLDRDY(int mc, int module) {
    unsigned long group[3] = {0L};
    
    group[mc/4] = 1L << ((mc & 0x03) * 3 + module);
    mc_LockLDRDYs(group[0], group[1], group[2]);
}

DMPAPI(void) mc_UnlockLDRDY(int mc, int module) {
    unsigned long group[3] = {0L};
    
    group[mc/4] = 1L << ((mc & 0x03) * 3 + module);
    mc_UnlockLDRDYs(group[0], group[1], group[2]);
}



DMPAPI(void) mc_EnableMulti(unsigned long group1, unsigned long group2, unsigned long group3) {
    unsigned long val1 = (group1 & 0x00000fffL) + ((group2 & 0x00000fffL) << 12) + ((group3 & 0x00000ffL) << 24);
    unsigned long val2 = ((group3 & 0x00000f00L) >> 8) + ((group1 & 0x000f0000L) >> 12) + ((group2 & 0x000f0000L) >> 8) + ((group3 & 0x000f0000L) >> 4);

    // 注意這裡的 enable 順序是反的
    mc_outp(MC_GENERAL, MCG_ENABLEREG2, mc_inp(MC_GENERAL, MCG_ENABLEREG2) | val2);
    mc_outp(MC_GENERAL, MCG_ENABLEREG1, mc_inp(MC_GENERAL, MCG_ENABLEREG1) | val1);
}

DMPAPI(void) mc_DisableMulti(unsigned long group1, unsigned long group2, unsigned long group3) {
    unsigned long val1 = (group1 & 0x00000fffL) + ((group2 & 0x00000fffL) << 12) + ((group3 & 0x00000ffL) << 24);
    unsigned long val2 = ((group3 & 0x00000f00L) >> 8) + ((group1 & 0x000f0000L) >> 12) + ((group2 & 0x000f0000L) >> 8) + ((group3 & 0x000f0000L) >> 4);

    mc_outp(MC_GENERAL, MCG_ENABLEREG1, mc_inp(MC_GENERAL, MCG_ENABLEREG1) & (~val1));
    mc_outp(MC_GENERAL, MCG_ENABLEREG2, mc_inp(MC_GENERAL, MCG_ENABLEREG2) & (~val2));
}

DMPAPI(unsigned long) mc_ReadEnableREG1(void) {
    return mc_inp(MC_GENERAL, MCG_ENABLEREG1);
}
DMPAPI(unsigned long) mc_ReadEnableREG2(void) {
    return mc_inp(MC_GENERAL, MCG_ENABLEREG2);
}



DMPAPI(void) mc_SetRouting_MC2SIF(unsigned long group1, unsigned long group2, unsigned long group3) {
    mc_outp(MC_GENERAL, MCG_G1ROUTEREG1, mc_inp(MC_GENERAL, MCG_G1ROUTEREG1) | group1);
    mc_outp(MC_GENERAL, MCG_G2ROUTEREG1, mc_inp(MC_GENERAL, MCG_G2ROUTEREG1) | group2);
    mc_outp(MC_GENERAL, MCG_G3ROUTEREG1, mc_inp(MC_GENERAL, MCG_G3ROUTEREG1) | group3);
}

DMPAPI(void) mc_RemoveRouting_MC2SIF(unsigned long group1, unsigned long group2, unsigned long group3) {
    mc_outp(MC_GENERAL, MCG_G1ROUTEREG1, mc_inp(MC_GENERAL, MCG_G1ROUTEREG1) & (~group1));
    mc_outp(MC_GENERAL, MCG_G2ROUTEREG1, mc_inp(MC_GENERAL, MCG_G2ROUTEREG1) & (~group2));
    mc_outp(MC_GENERAL, MCG_G3ROUTEREG1, mc_inp(MC_GENERAL, MCG_G3ROUTEREG1) & (~group3));
}

DMPAPI(void) mc_SetRouting_SIF2MC(unsigned long group1, unsigned long group2, unsigned long group3) {
    mc_outp(MC_GENERAL, MCG_G1ROUTEREG2, mc_inp(MC_GENERAL, MCG_G1ROUTEREG2) | group1);
    mc_outp(MC_GENERAL, MCG_G2ROUTEREG2, mc_inp(MC_GENERAL, MCG_G2ROUTEREG2) | group2);
    mc_outp(MC_GENERAL, MCG_G3ROUTEREG2, mc_inp(MC_GENERAL, MCG_G3ROUTEREG2) | group3);
}

DMPAPI(void) mc_RemoveRouting_SIF2MC(unsigned long group1, unsigned long group2, unsigned long group3) {
    mc_outp(MC_GENERAL, MCG_G1ROUTEREG2, mc_inp(MC_GENERAL, MCG_G1ROUTEREG2) & (~group1));
    mc_outp(MC_GENERAL, MCG_G2ROUTEREG2, mc_inp(MC_GENERAL, MCG_G2ROUTEREG2) & (~group2));
    mc_outp(MC_GENERAL, MCG_G3ROUTEREG2, mc_inp(MC_GENERAL, MCG_G3ROUTEREG2) & (~group3));
}

/*-----------------------  end of General Settings  --------------------------*/



/*****************************  PWM Settings  *********************************/

static unsigned long MCPWM_modOffset[3] = {0x08L, 0x08L + 0x2cL, 0x08L + 2L*0x2cL};

// MC PWM Registers
#define MCPWM_PERREG        (0x00L)
#define MCPWM_P0REG         (0x04L)
#define MCPWM_CTRLREG       (0x08L)
#define MCPWM_STATREG1      (0x0cL)
#define MCPWM_STATREG2      (0x10L)
#define MCPWM_EVTREG1       (0x14L)
#define MCPWM_EVTREG2       (0x18L)
#define MCPWM_OUTCTRLREG    (0x1cL)
#define MCPWM_FOUTREG       (0x24L)
#define MCPWM_LDREG         (0x28L)

DMPAPI(void) mcpwm_Enable(int mc, int module) {
    unsigned long val  = 1L << ((mc*3 + module) & 31);
    unsigned long reg  = ((mc*3 + module) < 32)? MCG_ENABLEREG1 : MCG_ENABLEREG2;
    
    mc_outp(MC_GENERAL, reg, mc_inp(MC_GENERAL, reg) | val);
}

DMPAPI(void) mcpwm_Disable(int mc, int module) {
    unsigned long val  = 1L << ((mc*3 + module) & 31);
    unsigned long reg  = ((mc*3 + module) < 32)? MCG_ENABLEREG1 : MCG_ENABLEREG2;

    mc_outp(MC_GENERAL, reg, mc_inp(MC_GENERAL, reg) & ~val);
}

DMPAPI(void) mcpwm_DisableProtect(int mc, int module) {
    unsigned long val = ~(1L << ((mc*3 + module) & 31));
    unsigned long reg = ((mc*3 + module) < 32)? MCG_PROTECTREG1 : MCG_PROTECTREG2;
    
    val = mc_inp(MC_GENERAL, reg) & val;
    if ((mc*3 + module) < 32)  // unlock the write protection register
    {
        mc_outp(MC_GENERAL, MCG_UNLOCKREG1, 0x000055aaL);
        mc_outp(MC_GENERAL, MCG_UNLOCKREG1, 0x0000aa55L);
    }
    else
    {
        mc_outp(MC_GENERAL, MCG_UNLOCKREG1, 0x55aa0000L);
        mc_outp(MC_GENERAL, MCG_UNLOCKREG1, 0xaa550000L);
    }
    mc_outp(MC_GENERAL, reg, val);
}



DMPAPI(void) mcpwm_SetWaveform(int mc, int module, unsigned long mode) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x3fffffffL) | mode);
}

DMPAPI(void) mcpwm_SetWidth(int mc, int module, unsigned long period, unsigned long phase0) {
    unsigned long reg = MCPWM_modOffset[module];

    mc_outp(mc, reg + MCPWM_PERREG, period);
    mc_outp(mc, reg + MCPWM_P0REG,  phase0);
}

DMPAPI(void) mcpwm_SetSamplCycle(int mc, int module, unsigned long sc) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xe0000000L) | (sc & 0x1fffffffL));
}

DMPAPI(void) mcpwm_SetSCRESET(int mc, int module) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_LDREG;

    mc_outpb(mc, reg, mc_inpb(mc, reg) | 0x02);
}

DMPAPI(void) mcpwm_ClearSCRESET(int mc, int module) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_LDREG;

    mc_outpb(mc, reg, mc_inpb(mc, reg) & 0xfd);
}

DMPAPI(void) mcpwm_SetSCLAST(int mc, int module) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_LDREG;

    mc_outpb(mc, reg, mc_inpb(mc, reg) | 0x01);
}

DMPAPI(void) mcpwm_ClearSCLAST(int mc, int module) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_LDREG;

    mc_outpb(mc, reg, mc_inpb(mc, reg) & 0xfe);
}

DMPAPI(unsigned long) mcpwm_ReadSCLAST(int mc, int module) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_LDREG;

    return mc_inp(mc, reg) & 1L;
}

DMPAPI(unsigned long) mcpwm_ReadSCREG(int mc, int module) {
    return mc_inp(mc, MCPWM_modOffset[module] + MCPWM_STATREG1) & 0x1fffffffL;
}



DMPAPI(unsigned long) mcpwm_ReadSTATREG1(int mc, int module) {
    return mc_inp(mc, MCPWM_modOffset[module] + MCPWM_STATREG1);
}
DMPAPI(unsigned long) mcpwm_ReadSTATREG2(int mc, int module) {
    return mc_inp(mc, MCPWM_modOffset[module] + MCPWM_STATREG2);
}



DMPAPI(void) mcpwm_SetDeadband(int mc, int module, unsigned long db) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_OUTCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xff000000L) | (db & 0x00ffffffL));
}

DMPAPI(void) mcpwm_SetOutMask(int mc, int module, unsigned long mask) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_OUTCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xf0ffffffL) | (mask & 0x0f000000L));
}

DMPAPI(void) mcpwm_SetOutPolarity(int mc, int module, unsigned long pol) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_OUTCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xcfffffffL) | (pol & 0x30000000L));
}

DMPAPI(void) mcpwm_SetMPWM(int mc, int module, unsigned long mpwm) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_OUTCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x3fffffffL) | (mpwm & 0xc0000000L));
}



DMPAPI(void) mcpwm_SetEVTMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_EVTREG2;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x3fffffffL) | (mode << 30));
}

DMPAPI(void) mcpwm_SetEVT(int mc, int module, unsigned long pwm, unsigned long pwmstat, unsigned long sc) {
    unsigned long reg1 = MCPWM_modOffset[module] + MCPWM_EVTREG1;
    unsigned long reg2 = MCPWM_modOffset[module] + MCPWM_EVTREG2;

    mc_outp(mc, reg1, pwm);
    mc_outp(mc, reg2, (mc_inp(mc, reg2) & 0xc0000000L) | (pwmstat << 29) | sc);
}



DMPAPI(void) mcpwm_SetFAU1Mode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffc3fL) | (mode << 6));
}
DMPAPI(void) mcpwm_SetFAU2Mode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfff0ffffL) | (mode << 16));
}
DMPAPI(void) mcpwm_SetFAU3Mode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xc3ffffffL) | (mode << 26));
}

DMPAPI(void) mcpwm_SetFAU1Output(int mc, int module, unsigned long mode) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffffffc0L) | mode);
}
DMPAPI(void) mcpwm_SetFAU2Output(int mc, int module, unsigned long mode) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff03ffL) | (mode << 10));
}
DMPAPI(void) mcpwm_SetFAU3Output(int mc, int module, unsigned long mode) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfc0fffffL) | (mode << 20));
}



DMPAPI(void) mcpwm_ReloadPWM(int mc, int module, unsigned char mode) {
    mc_outpb(mc, MCPWM_modOffset[module] + MCPWM_LDREG + 3L, mode);
}

DMPAPI(void) mcpwm_ReloadEVT(int mc, int module, unsigned char mode) {
    mc_outpb(mc, MCPWM_modOffset[module] + MCPWM_LDREG + 1L, mode);
}

DMPAPI(void) mcpwm_ReloadOUT(int mc, int module, unsigned char mode) {
    mc_outpb(mc, MCPWM_modOffset[module] + MCPWM_LDREG + 2L, mode);
}

DMPAPI(void) mcpwm_ReloadOUT_Unsafe(int mc, int module, unsigned char mode) {
    mcpwm_DisableProtect(mc, module);
    mc_outpb(mc, MCPWM_modOffset[module] + MCPWM_LDREG + 2L, mode);
}

DMPAPI(void) mcspwm_ReloadOUT_Unsafe(int mc, unsigned char mode) {
    mcpwm_DisableProtect(mc, MCPWM_MODULEA);
    mcpwm_DisableProtect(mc, MCPWM_MODULEB);
    mcpwm_DisableProtect(mc, MCPWM_MODULEC);
    mc_outpb(mc, MCPWM_modOffset[MCPWM_MODULEA] + MCPWM_LDREG + 2L, mode);
}


DMPAPI(unsigned char) mcpwm_ReadReloadPWM(int mc, int module) {
    return mc_inpb(mc, MCPWM_modOffset[module] + MCPWM_LDREG + 3L) & 0xf0;
}

DMPAPI(unsigned char) mcpwm_ReadReloadEVT(int mc, int module) {
    return mc_inpb(mc, MCPWM_modOffset[module] + MCPWM_LDREG + 1L) & 0xf0;
}

DMPAPI(unsigned char) mcpwm_ReadReloadOUT(int mc, int module) {
    return mc_inpb(mc, MCPWM_modOffset[module] + MCPWM_LDREG + 2L) & 0xf0;
}
/*------------------------  end of PWM Settings  -----------------------------*/



/****************************  Servo Settings  ********************************/

static unsigned long MCSV_modOffset[3] = {0x08L, 0x08L + 0x2cL, 0x08L + 2L*0x2cL};

// MC Servo Registers
#define MCSV_PERREG         (0x00L)
#define MCSV_P0REG          (0x04L)
#define MCSV_CTRLREG        (0x08L)
#define MCSV_STATREG1       (0x0cL)
#define MCSV_STATREG2       (0x10L)
#define MCSV_EVTREG1        (0x14L)
#define MCSV_EVTREG2        (0x18L)
#define MCSV_OUTCTRLREG     (0x1cL)
#define MCSV_DDAREG         (0x20L)
#define MCSV_FOUTREG        (0x24L)
#define MCSV_LDREG          (0x28L)
#define MCSV_DDAPOSREG      (0xc0L)
#define MCSV_EXTCTRLREG     (0xccL)

DMPAPI(void) mcsv_Enable(int mc, int module) {
    mcpwm_Enable(mc, module);
}

DMPAPI(void) mcsv_Disable(int mc, int module) {
    mcpwm_Disable(mc, module);
}

DMPAPI(void) mcsv_SetMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x1fffffffL) | mode);
}


DMPAPI(void) mcsv_SetWidth(int mc, int module, unsigned long period, unsigned long phase0) {
    unsigned long reg = MCSV_modOffset[module];

    mc_outp(mc, reg + MCSV_PERREG, period);
    mc_outp(mc, reg + MCSV_P0REG,  phase0);
}

DMPAPI(void) mcsv_SetSamplCycle(int mc, int module, unsigned long sc) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xe0000000L) | (sc & 0x1fffffffL));
}



/************************** 2012-04-26 Modify for EX **************************/
DMPAPI(void) mcsv_SetDDA(int mc, int module, unsigned long dir, unsigned long dda) {
    mc_outp(mc, MCSV_modOffset[module] + MCSV_DDAREG, (dir << 31) | (dda & 0x7fffffffL));
}

DMPAPI(unsigned long) mcsv_ReadDDA(int mc, int module) {
	return mc_inp(mc, MCSV_modOffset[module] + MCSV_DDAREG);
}

DMPAPI(void) mcsv_SetStatPulse(int mc, int module) {
    int offset = 9 + module*10;
    mc_outp(mc, MCSV_EXTCTRLREG, mc_inp(mc, MCSV_EXTCTRLREG) | (1L << offset));
}

DMPAPI(void) mcsv_ClearStatPulse(int mc, int module) {
    int offset = 9 + module*10;
    mc_outp(mc, MCSV_EXTCTRLREG, mc_inp(mc, MCSV_EXTCTRLREG) & ~(1L << offset));
}

DMPAPI(void) mcsv_SetDirOwner(int mc, int module) {
    int offset = 8 + module*10;
    mc_outp(mc, MCSV_EXTCTRLREG, mc_inp(mc, MCSV_EXTCTRLREG) | (1L << offset));
}

DMPAPI(void) mcsv_ClearDirOwner(int mc, int module) {
    int offset = 8 + module*10;
    mc_outp(mc, MCSV_EXTCTRLREG, mc_inp(mc, MCSV_EXTCTRLREG) & ~(1L << offset));
}

DMPAPI(void) mcsv_SetDDAInt(int mc, int module, unsigned long ddaint) {
   int offset = 4 + module*10;
   unsigned long mask = ~(0x0fL << offset); // 新增
   mc_outp(mc, MCSV_EXTCTRLREG, (mc_inp(mc, MCSV_EXTCTRLREG) & mask) | (ddaint << offset)); // 改變
}

DMPAPI(void) mcsv_SetDDAMode(int mc, int module, unsigned long mode) {
   int offset = 2 + module*10;
   unsigned long mask = ~(0x03L << offset); // 新增
   mc_outp(mc, MCSV_EXTCTRLREG, (mc_inp(mc, MCSV_EXTCTRLREG) & mask) | (mode << offset)); // 改變
}

DMPAPI(unsigned long) mcsv_ReadDDASTAT(int mc, int module) {
    int offset = module*10;
    return ((mc_inp(mc, MCSV_EXTCTRLREG) >> offset) & 0x00000003L);
}

DMPAPI(unsigned long) mcsv_ReadDDAMode(int mc, int module) {
    int offset = 2 + module*10;
    return ((mc_inp(mc, MCSV_EXTCTRLREG) >> offset) & 0x00000003L);
}

DMPAPI(unsigned long) mcsv_ReadDDAInt(int mc, int module) {
    int offset = 4 + module*10;
    return ((mc_inp(mc, MCSV_EXTCTRLREG) >> offset) & 0x0000000fL);
}

DMPAPI(unsigned long) mcsv_ReadDirOwner(int mc, int module) {
    int offset = 8 + module*10;
    return ((mc_inp(mc, MCSV_EXTCTRLREG) >> offset) & 1L);
}

DMPAPI(unsigned long) mcsv_ReadStatPulse(int mc, int module) {
    int offset = 9 + module*10;
    return ((mc_inp(mc, MCSV_EXTCTRLREG) >> offset) & 1L);
}
/******************************** End for EX **********************************/


// may cause racing condition if SCLAST <> 0
DMPAPI(void) mcsv_SetSCRESET(int mc, int module) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_LDREG;

    mc_outpb(mc, reg, mc_inpb(mc, reg) | 0x02);
}

// may cause racing condition if SCLAST <> 0
DMPAPI(void) mcsv_ClearSCRESET(int mc, int module) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_LDREG;

    mc_outpb(mc, reg, mc_inpb(mc, reg) & 0xfd);
}


DMPAPI(void) mcsv_SetSCLAST(int mc, int module) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_LDREG;

    mc_outpb(mc, reg, mc_inpb(mc, reg) | 0x01);
}

DMPAPI(void) mcsv_ClearSCLAST(int mc, int module) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_LDREG;

    mc_outpb(mc, reg, mc_inpb(mc, reg) & 0xfe);
}

DMPAPI(unsigned long) mcsv_ReadSCLAST(int mc, int module) {
    return mc_inp(mc, MCSV_modOffset[module] + MCSV_LDREG) & 1L;
}



DMPAPI(unsigned long) mcsv_ReadSCREG(int mc, int module) {
    return mc_inp(mc, MCSV_modOffset[module] + MCSV_STATREG1) & 0x1fffffffL;
}

DMPAPI(unsigned long) mcsv_ReadFBSTAT(int mc, int module) {
    return (mc_inp(mc, MCSV_modOffset[module] + MCSV_STATREG1) >> 30) & 0x03L;
}



DMPAPI(unsigned long) mcsv_ReadSTATREG1(int mc, int module) {
    return mc_inp(mc, MCSV_modOffset[module] + MCSV_STATREG1);
}
DMPAPI(unsigned long) mcsv_ReadSTATREG2(int mc, int module) {
    return mc_inp(mc, MCSV_modOffset[module] + MCSV_STATREG2);
}



DMPAPI(void) mcsv_SetEVTMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_EVTREG2;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x3fffffffL) | (mode << 30));
}

DMPAPI(void) mcsv_SetEVT(int mc, int module, unsigned long pcnt, unsigned long sc) {
    unsigned long reg1 = MCSV_modOffset[module] + MCSV_EVTREG1;
    unsigned long reg2 = MCSV_modOffset[module] + MCSV_EVTREG2;

    mc_outp(mc, reg1, pcnt);
    mc_outp(mc, reg2, (mc_inp(mc, reg2) & 0xc0000000L) | (sc & 0x1fffffffL));
}


DMPAPI(void) mcsv_SetFAU1Mode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffc3fL) | (mode << 6));
}
DMPAPI(void) mcsv_SetFAU2Mode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfff0ffffL) | (mode << 16));
}
DMPAPI(void) mcsv_SetFAU3Mode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xc3ffffffL) | (mode << 26));
}

DMPAPI(void) mcsv_SetFAU1Output(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffffffc0L) | mode);
}
DMPAPI(void) mcsv_SetFAU2Output(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff03ffL) | (mode << 10));
}
DMPAPI(void) mcsv_SetFAU3Output(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfc0fffffL) | (mode << 20));
}

DMPAPI(void) mcsv_SetDDAPOS(int mc, int module, unsigned long ddapos){
    mc_outp(mc, module * 0x04 + MCSV_DDAPOSREG, ddapos);
}

DMPAPI(unsigned long) mcsv_ReadDDAPOS(int mc, int module){
    return mc_inp(mc, module * 0x04 + MCSV_DDAPOSREG);
}

DMPAPI(void) mcsv_SetSWPIN(int mc, int module){
    unsigned long reg = MCSV_modOffset[module] + MCSV_OUTCTRLREG;
    mc_outp(mc, reg, mc_inp(mc, reg) | 0x80000000L);   
}

DMPAPI(void) mcsv_ClearSWPIN(int mc, int module){
    unsigned long reg = MCSV_modOffset[module] + MCSV_OUTCTRLREG;
    mc_outp(mc, reg, mc_inp(mc, reg) & 0x7fffffffL);
}



// may cause racing condition if SCLAST <> 0
DMPAPI(void) mcsv_SetPABInitOut(int mc, int module, unsigned char mode) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_LDREG;

    mc_outpb(mc, reg, (mc_inpb(mc, reg) & 0xf3) | mode);
}

DMPAPI(void) mcsv_SetOutMask(int mc, int module, unsigned long mask) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_OUTCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xf0ffffffL) | (mask & 0x0f000000L));
}

DMPAPI(void) mcsv_SetOutPolarity(int mc, int module, unsigned long pol) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_OUTCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xdfffffffL) | (pol & 0x20000000L));
}

DMPAPI(void) mcsv_SetDIR(int mc, int module, unsigned long dir) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_OUTCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xbfffffffL) | (dir & 0x40000000L));
}

DMPAPI(void) mcsv_SetDIRDeadband(int mc, int module, unsigned long db) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_OUTCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xff000000L) | (db & 0x00ffffffL));
}



DMPAPI(void) mcsv_ReloadSV(int mc, int module, unsigned char mode) {
    mc_outpb(mc, MCSV_modOffset[module] + MCSV_LDREG + 3L, mode);
}

DMPAPI(void) mcsv_ReloadEVT(int mc, int module, unsigned char mode) {
    mc_outpb(mc, MCSV_modOffset[module] + MCSV_LDREG + 1L, mode);
}

DMPAPI(void) mcsv_ReloadOUT(int mc, int module, unsigned char mode) {
    mc_outpb(mc, MCSV_modOffset[module] + MCSV_LDREG + 2L, mode);
}

DMPAPI(unsigned char) mcsv_ReadReloadSV(int mc, int module) {
    return mc_inpb(mc, MCSV_modOffset[module] + MCSV_LDREG + 3L) & 0xf0;
}

DMPAPI(unsigned char) mcsv_ReadReloadEVT(int mc, int module) {
    return mc_inpb(mc, MCSV_modOffset[module] + MCSV_LDREG + 1L) & 0xf0;
}

DMPAPI(unsigned char) mcsv_ReadReloadOUT(int mc, int module) {
    return mc_inpb(mc, MCSV_modOffset[module] + MCSV_LDREG + 2L) & 0xf0;
}

/*-----------------------  end of Servo Settings  ----------------------------*/



/*****************************  SIF Settings  *********************************/

static unsigned long MCSIF_modOffset[2] = {0x08L, 0x8cL};
static int           MCSIF_intOffset[2] = {0, 24};

// MC Sensor Interface Registers
#define MCSIF_CTRLREG           (0x00L)
#define MCSIF_DBREG             (0x04L)
#define MCSIF_FILREG            (0x08L)

#define MCSIF_ENC_PCNTREG       (0x0cL)
#define MCSIF_ENC_LIMREG1       (0x10L)
#define MCSIF_ENC_LIMREG2       (0x14L)
#define MCSIF_ENC_IDXREG        (0x18L)
#define MCSIF_ENC_EVTREG        (0x1cL)
#define MCSIF_ENC_CTRLREG       (0x20L)
#define MCSIF_ENC_IDXCONDREG    (0x24L)
#define MCSIF_ENC_STATREG       (0x28L)
#define MCSIF_ENC_CAPCTRLREG    (0x2cL)
#define MCSIF_ENC_CAPFIFOREG    (0x30L)

#define MCSIF_HALL_PCNTREG      (0x0cL)
#define MCSIF_HALL_LIMREG1      (0x10L)
#define MCSIF_HALL_LIMREG2      (0x14L)
#define MCSIF_HALL_EVTREG       (0x1cL)
#define MCSIF_HALL_CTRLREG      (0x20L)
#define MCSIF_HALL_STATREG      (0x28L)
#define MCSIF_HALL_CAPCTRLREG   (0x2cL)
#define MCSIF_HALL_CAPFIFOREG   (0x30L)

#define MCSIF_PFAU_CAPCTRLREG3  (0x14L)
#define MCSIF_PFAU_CAPFIFOREG3  (0x18L)
#define MCSIF_PFAU_CAPCTRLREG2  (0x20L)
#define MCSIF_PFAU_CAPFIFOREG2  (0x24L)
#define MCSIF_PFAU_STATREG      (0x28L)
#define MCSIF_PFAU_CAPCTRLREG1  (0x2cL)
#define MCSIF_PFAU_CAPFIFOREG1  (0x30L)

#define MCSIF_SSI_PCNTREG       (0x0cL)
#define MCSIF_SSI_CLKREG        (0x10L)
#define MCSIF_SSI_WAITREG       (0x14L)
#define MCSIF_SSI_DATAREG       (0x18L)
#define MCSIF_SSI_TIMEREG       (0x1cL)
#define MCSIF_SSI_CTRLREG       (0x20L)
#define MCSIF_SSI_STATREG       (0x28L)
#define MCSIF_SSI_CAPCTRLREG    (0x2cL)
#define MCSIF_SSI_CAPFIFOREG    (0x30L)

//

DMPAPI(void) mcsif_Enable(int mc, int module) {
    if (module == MCSIF_MODULEA)
        mcpwm_Enable(mc, module);
    else
        mc_outp(MC_GENERAL, MCG_ENABLEREG2, mc_inp(MC_GENERAL, MCG_ENABLEREG2) | (1L << (mc + 4)));
}

DMPAPI(void) mcsif_Disable(int mc, int module) {
    if (module == MCSIF_MODULEA)
        mcpwm_Disable(mc, module);
    else
        mc_outp(MC_GENERAL, MCG_ENABLEREG2, mc_inp(MC_GENERAL, MCG_ENABLEREG2) & ~(1L << (mc + 4)));
}

DMPAPI(void) mcsif_SetMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff1fffL) | mode);
}



DMPAPI(void) mcsif_SetSamplWin(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffffff00L) | mode);
}

DMPAPI(void) mcsif_SetSWPOL(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffcffL) | mode);
}

DMPAPI(void) mcsif_SetSWDeadband(int mc, int module, unsigned long db) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_DBREG, db & 0x00ffffffL);
}

DMPAPI(unsigned long) mcsif_ReadSWSTAT(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_CTRLREG) & 0x00000400L) >> 10;
}

DMPAPI(void) mcsif_SetInputFilter(int mc, int module, unsigned long filtime) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_FILREG, filtime & 0xc000ffffL);
}



DMPAPI(void) mcsif_EnableINT(int mc, int module, unsigned long intsort) {
    intsort = intsort << MCSIF_intOffset[module];
    mc_outp(mc, 0x00L, mc_inp(mc, 0x00L) | intsort);
}

DMPAPI(void) mcsif_DisableINT(int mc, int module, unsigned long intsort) {
    intsort = intsort << MCSIF_intOffset[module];
    mc_outp(mc, 0x00L, mc_inp(mc, 0x00L) & (~intsort));
}

DMPAPI(bool) mcsif_CheckINT(int mc, int module, unsigned long intsort) {
    intsort = intsort << MCSIF_intOffset[module];

    if ((mc_inp(mc, 0x04L) & intsort) == 0L) return false;
    return true;
}

DMPAPI(void) mcsif_ClearINT(int mc, int module, unsigned long intsort) {
    intsort = intsort << MCSIF_intOffset[module];
    mc_outp(mc, 0x04L, intsort);
}



// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^   Encoder Modes  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

DMPAPI(void) mcenc_SetCntMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_ENC_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff00ffL) | mode);
}

DMPAPI(void) mcenc_SetResetMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_ENC_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffffff00L) | mode);
}

DMPAPI(void) mcenc_SetTrigResetMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_ENC_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x1fffffffL) | mode);
}



DMPAPI(void) mcenc_SetPulCnt(int mc, int module, unsigned long pcnt) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_ENC_PCNTREG, pcnt & 0xffffffffL);
}

DMPAPI(unsigned long) mcenc_ReadPulCnt(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_PCNTREG) & 0xffffffffL;
}

DMPAPI(unsigned long) mcenc_ReadDIR(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_STATREG) >> 4) & 0x01L;
}

DMPAPI(unsigned long) mcenc_ReadPINS(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_STATREG) >> 5) & 0x07L;
}

DMPAPI(void) mcenc_SetCntMin(int mc, int module, unsigned long cmin) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_ENC_LIMREG1, cmin & 0xffffffffL);
}

DMPAPI(void) mcenc_SetCntMax(int mc, int module, unsigned long cmax) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_ENC_LIMREG2, cmax & 0xffffffffL);
}

DMPAPI(unsigned long) mcenc_ReadCntMax(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_LIMREG2) & 0xffffffffL;
}

DMPAPI(void) mcenc_SetCntIdx(int mc, int module, unsigned long cidx) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_ENC_IDXREG, cidx & 0xffffffffL);
}

DMPAPI(void) mcenc_SetCntEvt(int mc, int module, unsigned long cevt) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_ENC_EVTREG, cevt & 0xffffffffL);
}



DMPAPI(void) mcenc_SetIdxCond(int mc, int module, unsigned long mode, unsigned long abz) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_ENC_IDXCONDREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffffff18L) | (mode | abz));
}

DMPAPI(bool) mcenc_CheckIDXERR(int mc, int module) {
    if ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_STATREG) & 0x08L) != 0L) return true;
    return false;
}

DMPAPI(void) mcenc_ClearIDXERR(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_ENC_STATREG;

    mc_outp(mc, reg, mc_inp(mc, reg) | 0x08L);
}

DMPAPI(bool) mcenc_CheckIDXSTAT(int mc, int module) {
    if ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_STATREG) & 0x04L) != 0L) return true;
    return false;
}



DMPAPI(void) mcenc_SetCapMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_ENC_CAPCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfff0ffffL) | mode);
}

DMPAPI(void) mcenc_SetCapInterval(int mc, int module, unsigned long capivl) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_ENC_CAPCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffe00L) | (capivl & 0x1ffL));
}

DMPAPI(unsigned long) mcenc_ReadCAPSTAT(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_STATREG) & 0x03L;
}

DMPAPI(unsigned long) mcenc_ReadCAPFIFO(int mc, int module, unsigned long* data) {
    unsigned long tmp = mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_CAPFIFOREG);
    
    if (data != NULL) *data = tmp & 0x0fffffffL;
    return tmp & 0xf0000000L;
}

DMPAPI(void) mcenc_SetRLDTRIG(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_ENC_CAPCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffcfffffL) | mode);
}

DMPAPI(unsigned long) mcenc_ReadRLDTRIG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_CAPCTRLREG) & 0x00300000L;
}

DMPAPI(unsigned long) mcenc_ReadPINAInput(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_STATREG) >> 7) & 0x01L);
}

DMPAPI(unsigned long) mcenc_ReadPINBInput(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_STATREG) >> 6) & 0x01L);
}

DMPAPI(unsigned long) mcenc_ReadPINZInput(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_STATREG) >> 5) & 0x01L);
}



DMPAPI(void) mcenc_SetCtrlREG(int mc, int module, unsigned long val) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_ENC_CTRLREG, val);
}

DMPAPI(unsigned long) mcenc_ReadCtrlREG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_CTRLREG);
}

DMPAPI(void) mcenc_SetIdxCondREG(int mc, int module, unsigned long val) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_ENC_IDXCONDREG, val);
}

DMPAPI(unsigned long) mcenc_ReadIdxCondREG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_IDXCONDREG);
}

DMPAPI(void) mcenc_SetCapCtrlREG(int mc, int module, unsigned long val) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_ENC_CAPCTRLREG, val);
}

DMPAPI(unsigned long) mcenc_ReadCapCtrlREG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_CAPCTRLREG);
}



// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^   Hall Mode  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
DMPAPI(void) mchall_SetCntMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_HALL_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff00ffL) | mode);
}

DMPAPI(void) mchall_SetResetMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_HALL_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffffffccL) | mode);
}

DMPAPI(void) mchall_SetDirPattern(int mc, int module, unsigned long pattern) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_HALL_CTRLREG;

    pattern = ((pattern & 0x0003fffcL) << 14) | ((pattern & 0x00000003L) << 6);
    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x0000ff3fL) | pattern);
}



DMPAPI(void) mchall_SetHallCnt(int mc, int module, unsigned long pcnt) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_HALL_PCNTREG, pcnt & 0xffffffffL);
}

DMPAPI(unsigned long) mchall_ReadHallCnt(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_PCNTREG) & 0xffffffffL;
}

DMPAPI(unsigned long) mchall_ReadDIR(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_STATREG) >> 4) & 0x01L;
}

DMPAPI(unsigned long) mchall_ReadPINS(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_STATREG) >> 5) & 0x07L;
}

DMPAPI(void) mchall_SetCntMin(int mc, int module, unsigned long cmin) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_HALL_LIMREG1, cmin & 0xffffffffL);
}

DMPAPI(void) mchall_SetCntMax(int mc, int module, unsigned long cmax) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_HALL_LIMREG2, cmax & 0xffffffffL);
}

DMPAPI(void) mchall_SetCntEvt(int mc, int module, unsigned long cevt) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_HALL_EVTREG, cevt & 0xffffffffL);
}



DMPAPI(bool) mchall_CheckSTERR(int mc, int module) {
    if ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_STATREG) & 0x08L) != 0L) return true;
    return false;
}

DMPAPI(void) mchall_ClearSTERR(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_HALL_STATREG;

    mc_outp(mc, reg, mc_inp(mc, reg) | 0x08L);
}



DMPAPI(void) mchall_SetCapMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_HALL_CAPCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfff8ffffL) | mode);
}

DMPAPI(void) mchall_SetCapInterval(int mc, int module, unsigned long capivl) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_HALL_CAPCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffe00L) | (capivl & 0x1ffL));
}

DMPAPI(unsigned long) mchall_ReadCAPSTAT(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_STATREG) & 0x03L;
}

DMPAPI(unsigned long) mchall_ReadCAPFIFO(int mc, int module, unsigned long* data) {
    unsigned long tmp = mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_CAPFIFOREG);
    
    if (data != NULL) *data = tmp & 0x0fffffffL;
    return tmp & 0xf0000000L;
}

DMPAPI(unsigned long) mchall_ReadPINAInput(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_STATREG) >> 7) & 0x01L);
}

DMPAPI(unsigned long) mchall_ReadPINBInput(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_STATREG) >> 6) & 0x01L);
}

DMPAPI(unsigned long) mchall_ReadPINZInput(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_STATREG) >> 5) & 0x01L);
}



DMPAPI(void) mchall_SetCtrlREG(int mc, int module, unsigned long val) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_HALL_CTRLREG, val);
}

DMPAPI(unsigned long) mchall_ReadCtrlREG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_CTRLREG);
}

DMPAPI(void) mchall_SetCapCtrlREG(int mc, int module, unsigned long val) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_HALL_CAPCTRLREG, val);
}

DMPAPI(unsigned long) mchall_ReadCapCtrlREG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_CAPCTRLREG);
}



// ^^^^^^^^^^^^^^^^^^^^^^^   Pulse & Fault Capture Mode  ^^^^^^^^^^^^^^^^^^^^^^^

DMPAPI(void) mcpfau_SetCapMode1(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfff0ffffL) | mode);
}
DMPAPI(void) mcpfau_SetCapMode2(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfff0ffffL) | mode);
}
DMPAPI(void) mcpfau_SetCapMode3(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfff0ffffL) | mode);
}

DMPAPI(void) mcpfau_SetCapInterval1(int mc, int module, unsigned long capivl) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffe00L) | (capivl & 0x1ffL));
}
DMPAPI(void) mcpfau_SetCapInterval2(int mc, int module, unsigned long capivl) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffe00L) | (capivl & 0x1ffL));
}
DMPAPI(void) mcpfau_SetCapInterval3(int mc, int module, unsigned long capivl) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffe00L) | (capivl & 0x1ffL));
}

DMPAPI(unsigned long) mcpfau_ReadPIN1Input(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_STATREG) >> 7) & 0x01L);
}

DMPAPI(unsigned long) mcpfau_ReadPIN2Input(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_STATREG) >> 6) & 0x01L);
}

DMPAPI(unsigned long) mcpfau_ReadPIN3Input(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_STATREG) >> 5) & 0x01L);
}

DMPAPI(unsigned long) mcpfau_ReadCAPSTAT1(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_STATREG) & 0x03L;
}
DMPAPI(unsigned long) mcpfau_ReadCAPSTAT2(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_STATREG) & 0x0300L) >> 8;
}
DMPAPI(unsigned long) mcpfau_ReadCAPSTAT3(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_STATREG) & 0x0c00L) >> 10;
}

DMPAPI(unsigned long) mcpfau_ReadCAPFIFO1(int mc, int module, unsigned long* data) {
    unsigned long tmp = mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPFIFOREG1);
    
    if (data != NULL) *data = tmp & 0x0fffffffL;
    return tmp & 0xf0000000L;
}
DMPAPI(unsigned long) mcpfau_ReadCAPFIFO2(int mc, int module, unsigned long* data) {
    unsigned long tmp = mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPFIFOREG2);
    
    if (data != NULL) *data = tmp & 0x0fffffffL;
    return tmp & 0xf0000000L;
}
DMPAPI(unsigned long) mcpfau_ReadCAPFIFO3(int mc, int module, unsigned long* data) {
    unsigned long tmp = mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPFIFOREG3);
    
    if (data != NULL) *data = tmp & 0x0fffffffL;
    return tmp & 0xf0000000L;
}



DMPAPI(void) mcpfau_SetPolarity1(int mc, int module, unsigned long pol) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x7fffffffL) | pol);
}
DMPAPI(void) mcpfau_SetPolarity2(int mc, int module, unsigned long pol) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x7fffffffL) | pol);
}
DMPAPI(void) mcpfau_SetPolarity3(int mc, int module, unsigned long pol) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x7fffffffL) | pol);
}

DMPAPI(void) mcpfau_SetMask1(int mc, int module, unsigned long mask) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1;

	mc_outp(mc, reg, (mc_inp(mc, reg) & 0x8fffffffL) | mask);
}
DMPAPI(void) mcpfau_SetMask2(int mc, int module, unsigned long mask) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2;
    
    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x8fffffffL) | mask);
}
DMPAPI(void) mcpfau_SetMask3(int mc, int module, unsigned long mask) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x8fffffffL) | mask);
}



DMPAPI(void) mcpfau_SetRLDTRIG1(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffcfffffL) | mode);
}
DMPAPI(void) mcpfau_SetRLDTRIG2(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffcfffffL) | mode);
}
DMPAPI(void) mcpfau_SetRLDTRIG3(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffcfffffL) | mode);
}

DMPAPI(unsigned long) mcpfau_ReadRLDTRIG1(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1) & 0x00300000L;
}
DMPAPI(unsigned long) mcpfau_ReadRLDTRIG2(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2) & 0x00300000L;
}
DMPAPI(unsigned long) mcpfau_ReadRLDTRIG3(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3) & 0x00300000L;
}



DMPAPI(void) mcpfau_SetEXTRIG1(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xff3fffffL) | mode);
}
DMPAPI(void) mcpfau_SetEXTRIG2(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xff3fffffL) | mode);
}
DMPAPI(void) mcpfau_SetEXTRIG3(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xff3fffffL) | mode);
}

DMPAPI(unsigned long) mcpfau_ReadEXTRIG1(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1) & 0x00c00000L;
}
DMPAPI(unsigned long) mcpfau_ReadEXTRIG2(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2) & 0x00c00000L;
}
DMPAPI(unsigned long) mcpfau_ReadEXTRIG3(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3) & 0x00c00000L;
}



DMPAPI(unsigned long) mcpfau_ReadFAU1STAT(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1) >> 28) & 0x01L;
}
DMPAPI(unsigned long) mcpfau_ReadFAU2STAT(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2) >> 28) & 0x01L;
}
DMPAPI(unsigned long) mcpfau_ReadFAU3STAT(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3) >> 28) & 0x01L;
}

DMPAPI(void) mcpfau_ClearFAU1STAT(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1;

    mc_outp(mc, reg, mc_inp(mc, reg) | (1L << 28));
}
DMPAPI(void) mcpfau_ClearFAU2STAT(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2;

    mc_outp(mc, reg, mc_inp(mc, reg) | (1L << 28));
}
DMPAPI(void) mcpfau_ClearFAU3STAT(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3;

    mc_outp(mc, reg, mc_inp(mc, reg) | (1L << 28));
}

DMPAPI(void) mcpfau_SetFAU1RELS(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xf3ffffffL) | mode);
}
DMPAPI(void) mcpfau_SetFAU2RELS(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xf3ffffffL) | mode);
}
DMPAPI(void) mcpfau_SetFAU3RELS(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xf3ffffffL) | mode);
}

DMPAPI(void) mcpfau_SetFAU1TRIG(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfcffffffL) | mode);
}
DMPAPI(void) mcpfau_SetFAU2TRIG(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfcffffffL) | mode);
}
DMPAPI(void) mcpfau_SetFAU3TRIG(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfcffffffL) | mode);
}



DMPAPI(unsigned long) mcpfau_ReadCap1CtrlREG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1);
}
DMPAPI(unsigned long) mcpfau_ReadCap2CtrlREG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2);
}
DMPAPI(unsigned long) mcpfau_ReadCap3CtrlREG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3);
}

DMPAPI(unsigned long) mcpfau_ReadCapStatREG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_STATREG);
}

#define MCSIF_PFAU_CAPCTRLREG3  (0x14L)
#define MCSIF_PFAU_CAPCTRLREG2  (0x20L)
#define MCSIF_PFAU_CAPCTRLREG1  (0x2cL)
DMPAPI(void) mcpfau_SetCap1INT(int mc, int module, unsigned long interval)
{
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1;
	interval = (interval << 11L) & 0x0000f800L;
    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff07ffL) + interval);
}

DMPAPI(void) mcpfau_SetCap2INT(int mc, int module, unsigned long interval)
{
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2;
	interval = (interval << 11L) & 0x0000f800L;
    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff07ffL) + interval);
}

DMPAPI(void) mcpfau_SetCap3INT(int mc, int module, unsigned long interval)
{
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3;
	interval = (interval << 11L) & 0x0000f800L;
    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff07ffL) + interval);
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^   SSI Mode  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
DMPAPI(void) mcssi_SetPulCnt(int mc, int module, unsigned long pcnt) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_SSI_PCNTREG, pcnt & 0x0fffffffL);
}

DMPAPI(unsigned long) mcssi_ReadPulCnt(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_SSI_PCNTREG) & 0x0fffffffL;
}

DMPAPI(unsigned long) mcssi_ReadData(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_SSI_DATAREG) & 0xffffffffL;
}

DMPAPI(void) mcssi_SetClock(int mc, int module, unsigned long clk) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_SSI_CLKREG, clk & 0x0fffffffL);
}

DMPAPI(unsigned long) mcssi_ReadClock(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_SSI_CLKREG) & 0x0fffffffL;
}

DMPAPI(void) mcssi_SetWaitTime(int mc, int module, unsigned long waittime) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_SSI_WAITREG, waittime & 0x0fffffffL);
}

DMPAPI(unsigned long) mcssi_ReadWaitTime(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_SSI_WAITREG) & 0x0fffffffL;
}

DMPAPI(void) mcssi_SetLatchPhase(int mc, int module, unsigned long phase) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_TIMEREG;
	mc_outp(mc, reg, (mc_inp(mc, reg) & 0x7fffffffL) | phase);
}

DMPAPI(unsigned long) mcssi_ReadLatchPhase(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_TIMEREG;
	return (mc_inp(mc, reg) & ~(0x7fffffffL)) >> 31;
}

DMPAPI(void) mcssi_SetLatchTime(int mc, int module, unsigned long latchtime) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_TIMEREG;
    latchtime = latchtime & 0x0fffffffL;
	mc_outp(mc, reg, (mc_inp(mc, reg) & 0xf0000000L) | latchtime);
}

DMPAPI(unsigned long) mcssi_ReadLatchTime(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_TIMEREG;
	return (mc_inp(mc, reg) & ~(0xf0000000L));
}

DMPAPI(void) mcssi_SetNumberBITS(int mc, int module, unsigned long numberbits) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	numberbits = numberbits & 0x1fL;
	mc_outp(mc, reg, (mc_inp(mc, reg) & 0x07ffffffL) | (numberbits << 27));
}

DMPAPI(unsigned long) mcssi_ReadNumberBITS(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	return (mc_inp(mc, reg) & ~(0x07ffffffL)) >> 27;
}

DMPAPI(void) mcssi_SetNumberRUNS(int mc, int module, unsigned long numberruns) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	numberruns = numberruns & 0x1fL;
	mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffe0ffffL) | (numberruns << 16));
}

DMPAPI(unsigned long) mcssi_ReadNumberRUNS(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	return (mc_inp(mc, reg) & ~(0xffe0ffffL)) >> 16;
}

DMPAPI(void) mcssi_SetCntMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff0fffL) | mode);
}

DMPAPI(void) mcssi_SetExternalBit(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	mc_outp(mc, reg, mc_inp(mc, reg) | (0x01L << 10));
}

DMPAPI(void) mcssi_ClearExternalBit(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffbffL));
}

DMPAPI(void) mcssi_SetChkErrBit(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	mc_outp(mc, reg, mc_inp(mc, reg) | (0x01L << 9));
}

DMPAPI(void) mcssi_ClearChkErrBit(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffdffL));
}

DMPAPI(void) mcssi_SetGAY2BINBit(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	mc_outp(mc, reg, mc_inp(mc, reg) | (0x01L << 8));
}

DMPAPI(void) mcssi_ClearGAY2BINBit(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffeffL));
}

DMPAPI(unsigned long) mcssi_ReadRunNumber(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_SSI_STATREG) >> 8) & 0x1fL);
}

DMPAPI(unsigned long) mcssi_ReadSSISTAT(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_SSI_STATREG) >> 5) & 0x07L);
}

DMPAPI(bool) mcssi_CheckERRBit(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_STATREG;
	if((mc_inp(mc, reg) & 0x08L) != 0) return true;
	return false;
}

DMPAPI(void) mcssi_ClearERRBit(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_STATREG;
	mc_outp(mc, reg, mc_inp(mc, reg) | 0x08L);
}

DMPAPI(unsigned long) mcssi_ReadCAPSTAT(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_SSI_STATREG) & 0x03L);
}

DMPAPI(void) mcssi_SetRLDTRIG(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CAPCTRLREG;
	mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffcfffffL) | mode);
}

DMPAPI(void) mcssi_SetCAPMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CAPCTRLREG;
	mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffcffffL) | mode);
}

DMPAPI(void) mcssi_SetCAPINT(int mc, int module, unsigned long number) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CAPCTRLREG;
    number = number & 0x1FL;
	mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff0effL) | (number << 11));
}

DMPAPI(void) mcssi_SetCapInterval(int mc, int module, unsigned long number) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CAPCTRLREG;
    number = number & 0x1FFL;
	mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffe00L) | number);
}

DMPAPI(unsigned long) mcssi_ReadCAPFIFO(int mc, int module, unsigned long* data) {
    unsigned long tmp = mc_inp(mc, MCSIF_modOffset[module] + MCSIF_SSI_CAPFIFOREG);
    
    if (data != NULL) *data = tmp;
    return 0L;
}

DMPAPI(unsigned long) mcssi_ReadCtrlREG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG);
}


// MCM INTERRUPT SETTING FUNCTION //
static unsigned char int_routing_table[16] = {0xff, 0x08, 0xff, 0x02, 0x04, 0x05, 0x07, 0x06, 0xff, 0x01, 0x03, 0x09, 0x0b, 0xff, 0x0d, 0x0f};
DMPAPI(unsigned char) GetMCIRQ(void) {
    return (unsigned char)(pci_In32(mcm_p, 0x3c) & 0xffL);
}

DMPAPI(void) Set_MCIRQ(unsigned char irq) {
	pci_Out32(mcm_p, 0x3c, (pci_In32(mcm_p, 0x3c) & 0xffffff00L) | (unsigned long) irq);
    sb1_Write8(0xb4, (sb1_Read8(0xb4) & 0xf0) | int_routing_table[irq]);
}
