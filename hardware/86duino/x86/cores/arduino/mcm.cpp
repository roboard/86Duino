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
  
  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)
*/

#include <stdio.h>
#include <conio.h>
#include <dos.h>

#include "io.h"

#include "mcm.h"

#if defined(DJGPP)
    #include <go32.h>
    #include <dpmi.h>
    #include <sys/farptr.h>
    #include <pc.h>
#include <dos.h>
#endif

bool MC_useMMIO = false;


void set_MMIO(void) {
    MC_useMMIO = true;
}

void set_PortIO(void) {
    MC_useMMIO = false;
}

#define DEVICE (0x10)
/******************************************************************************/
static void write_mc_pcireg(unsigned idx, unsigned long val) {
    unsigned long cf8 = (0x01L << 31)  // Type-1 PCI configuration
                      + (0x00L << 16)  // Bus 0x00
                      + (DEVICE << 11)  // Device 0x10
                      + (0x00L << 8)   // Fun 0x00;
                      + idx;

    io_DisableINT();
	io_outpdw(0x0cf8, cf8 & 0xfffffffcL);
	io_outpdw(0x0cfc, val);
    io_RestoreINT();
}

static unsigned long read_mc_pcireg(unsigned idx) {
    unsigned long tmp;
    unsigned long cf8 = (0x01L << 31)  // Type-1 PCI configuration
                      + (0x00L << 16)  // Bus 0x00
                      + (DEVICE << 11)  // Device 0x10
                      + (0x00L << 8)   // Fun 0x00;
                      + idx;

    io_DisableINT();
	io_outpdw(0x0cf8, cf8 & 0xfffffffcL);
    tmp = io_inpdw(0x0cfc);
    io_RestoreINT();

    return tmp;
}



/* static bool USE_8051B = false;
void Master_8051B(void) {
    USE_8051B = true;
    write_mc_pcireg(0x40, (read_mc_pcireg(0x40) & 0x00ffffffL) + 0x01000000L);
}

void Master_DX2(void) {
    USE_8051B = false;
    write_mc_pcireg(0x40, read_mc_pcireg(0x40) & 0x00ffffffL);
}



// I/O functions to 8051B server
unsigned long mc8051_setbaseaddr(void) {
  init_8051_io();
  return MOTOR_CONR_BASE;
}

void b8051_outpb(unsigned int addr, unsigned char val) {
    write_8051_xdata_byte(addr, val);
}

unsigned char b8051_inpb(unsigned int addr) {
    return read_8051_xdata_byte(addr);
}

// read/write 32bit to 8051 motor controller register : need to follow special order.
static unsigned long read_8051_xdata_dword(unsigned int addr) {
  // for 8051 to read 32 bit PWM register, read from MSB to LSB
  int i;
  unsigned long b;
  unsigned long d;
  d = 0;
  
  // for(i = 3; i >= 0; i--)
  // {
    // b = read_8051_xdata_byte(addr + i);
    // printf("read byte %04X = %02X\n", addr + i, b);
    // d <<= 8;
    // d |= b;
  // }
  
  //disable_8051_int1();
  for(i = 0; i < 4; i++)
  {
    b = (unsigned long)read_8051_xdata_byte(addr + i);
    //printf("read byte %04X = %02X\n", addr + i, b);
    d |= (b << (i * 8));
  }
  //enable_8051_int1();
  return d;
}

static void write_8051_xdata_dword(unsigned int addr, unsigned long d) {
  // for 8051 to write 32 bit PWM register, write from LSB to MSB
  int i;
  unsigned char b;
  
  // for(i = 0; i < 4; i++)
  // {
    // b = d & 0xff;
    // d >>= 8;
    // printf("write byte %04X %02X\n", addr + i, b);
    // write_8051_xdata_byte(addr + i, b);
  // }
  
  //disable_8051_int1();
  for(i = 0; i < 4; i++)
  {
    b = (d >> (i * 8)) & 0xff;
    //printf("write byte %04X %02X\n", addr + i, b);
    write_8051_xdata_byte(addr + i, b);
  }
  //enable_8051_int1();
}

// get register base I/O address from motor index
static unsigned int get_mc_base(int mc) {
    if(mc == MC_GENERAL) return MOTOR_CONR_BASE;
    return MOTOR_CONR_BASE + 0x40 + (0xd0 * mc);
}

static void mc8051_outp(int mc, unsigned long idx, unsigned long val) {
  //printf("mc_outp : %d %02X %08lX\n", mc, idx, val);
  //io_outpdw(PWM_baseAddr + 0xc0, 1UL<<(mc+1));  // paging to corresponding reg window 
  //io_outpdw(PWM_baseAddr + (unsigned)idx, val);

  unsigned int mc_base;
  mc_base = get_mc_base(mc);
  // unmask, all 32bit is writable
  write_8051_xdata_byte(MCGINTSTA, 0);
  //printf("write dword %04X %08lX\n", mc_base + (unsigned)idx, val);
  write_8051_xdata_dword(mc_base + (unsigned)idx, val);
}

static unsigned long mc8051_inp(int mc, unsigned long idx) {
  //io_outpdw(PWM_baseAddr + 0xc0, 1UL<<(mc+1));  // paging to corresponding reg window 
  //return io_inpdw(PWM_baseAddr + (unsigned)idx);

  unsigned int mc_base;
  mc_base = get_mc_base(mc);
  // unmask, all 32bit is writable
  write_8051_xdata_byte(MCGINTSTA, 0);
  //printf("read dword %04X\n", mc_base + (unsigned)idx);
  return read_8051_xdata_dword(mc_base + (unsigned)idx);
}

static void mc8051_outpb(int mc, unsigned long idx, unsigned char val) {
  //io_outpdw(PWM_baseAddr + 0xc0, 1UL<<(mc+1));  // paging to corresponding reg window 
  //io_outpb(PWM_baseAddr + (unsigned)idx, val);

  unsigned int mc_base;
  unsigned long real_idx;
  int byte_ofs;
  unsigned long ld;
  unsigned char mask;
  mc_base = get_mc_base(mc);
  real_idx = idx & (~3L);
  byte_ofs = (int)(idx % 4L);
  // set data mask
  mask = (~(1 << byte_ofs)) & 0x0f;
  write_8051_xdata_byte(MCGINTSTA, mask);
  // shift to 32bit data
  ld = ((unsigned long)val) << (byte_ofs * 8);
  // write shifted 32bit data
  write_8051_xdata_dword(mc_base + (unsigned)real_idx, ld);
}

static unsigned char mc8051_inpb(int mc, unsigned long idx) {
  //io_outpdw(PWM_baseAddr + 0xc0, 1UL<<(mc+1));  // paging to corresponding reg window 
  //return io_inpb(PWM_baseAddr + (unsigned)idx);

  unsigned int mc_base;
  unsigned long real_idx;
  int byte_ofs;
  unsigned long ld;
  unsigned char b;
  mc_base = get_mc_base(mc);
  real_idx = idx & (~3L);
  byte_ofs = (int)(idx % 4L);
  // read 32bit data
  ld = read_8051_xdata_dword(mc_base + (unsigned)real_idx);
  // shift data to 8bit
  b = (unsigned char)((ld >> (byte_ofs * 8)) & 0xff);
  return b;
} */







/*********************  DPMI Functions (for internal use)  ********************/
/* #if defined(DJGPP)

static unsigned long dpmi_LinMapAlloc(unsigned long phyaddr, unsigned long size) {
        __dpmi_meminfo meminfo;

        if (phyaddr < 0x100000L) return phyaddr;  // only needs to map non-DOS memory (DPMI host has already mapped DOS memory below 1Mb)

        meminfo.address = phyaddr;
        meminfo.size    = size;
        if (__dpmi_physical_address_mapping(&meminfo) != 0) return 0x00000000L;

        __dpmi_lock_linear_region(&meminfo);      // avoid virtual memory paging
        return meminfo.address;
}

static void dpmi_LinMapFree(unsigned long linaddr) {
        __dpmi_meminfo meminfo;

        if (linaddr >= 0x100000L)
        {
            meminfo.address = linaddr;
            //if (__dpmi_free_physical_address_mapping(&meminfo) != 0) return false;  // not work on some DPMI servers
            __dpmi_free_physical_address_mapping(&meminfo);
        }
}

static int dpmi_SelAlloc(unsigned long linaddr, unsigned long size) {
        int selector;
    
        if ((selector = __dpmi_allocate_ldt_descriptors(1)) < 0) return -1;
    
        if (__dpmi_set_segment_base_address(selector, linaddr) == 0) //&&
        if (__dpmi_set_segment_limit(selector, size - 1) == 0)
        //if (__dpmi_set_segment_limit(selector, ((size-1) >= 0xffff)? size-1 : 0xffff) != 0)
            return selector;

        __dpmi_free_ldt_descriptor(selector);
        return -1;
}

static bool dpmi_SelFree(int selector) {
        if (selector < 0) return true;

        if (__dpmi_free_ldt_descriptor(selector) != 0) return false;
        return true;
}

#endif */
/*----------------  end of DPMI Functions (for internal use)  ----------------*/



static unsigned long MMIO_baseAddr = 0L;
static int           mmio_selector = -1;
static unsigned long mmio_linaddr  = 0x00000000L;

static void mmio_outpdw(unsigned long offs, unsigned long val) {
    #ifdef DJGPP
        _farpokel(mmio_selector, offs, val);
    #endif
}

static unsigned long mmio_inpdw(unsigned long offs) {
    #ifdef DJGPP
        return _farpeekl(mmio_selector, offs);
    #endif
}

static void mmio_outpb(unsigned long offs, unsigned char val) {
    #ifdef DJGPP
        _farpokeb(mmio_selector, offs, val);
    #endif
}

static unsigned char mmio_inpb(unsigned long offs) {
    #ifdef DJGPP
        return _farpeekb(mmio_selector, offs);
    #endif
}



static unsigned PWM_baseAddr = 0;
unsigned long mc_setbaseaddr(void) {
    if (MC_useMMIO == true && MMIO_baseAddr == 0L)
    {
        MMIO_baseAddr = read_mc_pcireg(0x14);
        #ifdef DJGPP
            mmio_linaddr  = dpmi_LinMapAlloc(MMIO_baseAddr, 0x1000);
            mmio_selector = dpmi_SelAlloc(mmio_linaddr, 0x1000);
        #endif        
        return 0L;
    }
    else if(PWM_baseAddr == 0L)
    {
        PWM_baseAddr = (unsigned)(read_mc_pcireg(0x10) & 0x0000fff0L);
        return (unsigned long)PWM_baseAddr;
    }
}

int mc_getIRQ(void) {
	return (int)(read_mc_pcireg(0x3c) & 0xFF);
}

void mc_outp(int mc, unsigned long idx, unsigned long val) {
    // if (USE_8051B == true) { mc8051_outp(mc, idx, val); return; }

    if (MC_useMMIO == true)
    {
        if (mc == MC_GENERAL)
            mmio_outpdw(idx, val);
        else
            mmio_outpdw(0x40L + (unsigned long)mc * 0xd0L + idx, val);
    }
    else
    {   
        io_DisableINT();
        io_outpdw(PWM_baseAddr + 0xd0, 1UL<<(mc+1));  // paging to corresponding reg window 
        io_outpdw(PWM_baseAddr + (unsigned)idx, val);
        io_RestoreINT();
    }
}

unsigned long mc_inp(int mc, unsigned long idx) {
    unsigned long tmp;
    // if (USE_8051B == true) return mc8051_inp(mc, idx);

    if (MC_useMMIO == true)
    {
        if (mc == MC_GENERAL)
            return mmio_inpdw(idx);
        else
            return mmio_inpdw(0x40L + (unsigned long)mc * 0xd0L + idx);
    }
    else
    {   
        io_DisableINT();
        io_outpdw(PWM_baseAddr + 0xd0, 1UL<<(mc+1));  // paging to corresponding reg window 
        tmp = io_inpdw(PWM_baseAddr + (unsigned)idx);
        io_RestoreINT();
        return tmp;
    }
}

void mc_outpb(int mc, unsigned long idx, unsigned char val) {
    // if (USE_8051B == true) { mc8051_outpb(mc, idx, val); return; }

    if (MC_useMMIO == true)
    {
        if (mc == MC_GENERAL)
            mmio_outpb(idx, val);
        else
            mmio_outpb(0x40L + (unsigned long)mc * 0xd0L + idx, val);
    }
    else
    {   
        io_DisableINT();
        io_outpdw(PWM_baseAddr + 0xd0, 1UL<<(mc+1));  // paging to corresponding reg window 
        io_outpb(PWM_baseAddr + (unsigned)idx, val);
        io_RestoreINT();
    }
}

unsigned char mc_inpb(int mc, unsigned long idx) {
    unsigned char tmp;
    // // if (USE_8051B == true) return mc8051_inpb(mc, idx);

    if (MC_useMMIO == true)
    {
        if (mc == MC_GENERAL)
            return mmio_inpb(idx);
        else
            return mmio_inpb(0x40L + (unsigned long)mc * 0xd0L + idx);
        return 0;
    }
    else
    {   
        io_DisableINT();
        io_outpdw(PWM_baseAddr + 0xd0, 1UL<<(mc+1));  // paging to corresponding reg window 
        tmp = io_inpb(PWM_baseAddr + (unsigned)idx);
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

bool mc_IsEnabled(int mc) {
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

bool mc_SetMode(int mc, unsigned long mode) {
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


void mc_LockLDRDYs(unsigned long group1, unsigned long group2, unsigned long group3) {
    unsigned long reg1 = group1 | (group2 << 12) | (group3 << 24);
    unsigned long reg2 = group3 >> 8;

    mc_outp(MC_GENERAL, MCG_LDRDYSYNCREG1, mc_inp(MC_GENERAL, MCG_LDRDYSYNCREG1) & ~reg1);
    mc_outp(MC_GENERAL, MCG_LDRDYSYNCREG2, mc_inp(MC_GENERAL, MCG_LDRDYSYNCREG2) & ~reg2);
}

void mc_UnlockLDRDYs(unsigned long group1, unsigned long group2, unsigned long group3) {
    unsigned long reg1 = group1 | (group2 << 12) | (group3 << 24);
    unsigned long reg2 = group3 >> 8;

    mc_outp(MC_GENERAL, MCG_LDRDYSYNCREG1, mc_inp(MC_GENERAL, MCG_LDRDYSYNCREG1) | reg1);
    mc_outp(MC_GENERAL, MCG_LDRDYSYNCREG2, mc_inp(MC_GENERAL, MCG_LDRDYSYNCREG2) | reg2);
}

void mc_LockLDRDY(int mc, int module) {
    unsigned long group[3] = {0L};
    
    group[mc/4] = 1L << ((mc & 0x03) * 3 + module);
    mc_LockLDRDYs(group[0], group[1], group[2]);
}

void mc_UnlockLDRDY(int mc, int module) {
    unsigned long group[3] = {0L};
    
    group[mc/4] = 1L << ((mc & 0x03) * 3 + module);
    mc_UnlockLDRDYs(group[0], group[1], group[2]);
}



void mc_EnableMulti(unsigned long group1, unsigned long group2, unsigned long group3) {
    unsigned long val1 = (group1 & 0x00000fffL) + ((group2 & 0x00000fffL) << 12) + ((group3 & 0x00000ffL) << 24);
    unsigned long val2 = ((group3 & 0x00000f00L) >> 8) + ((group1 & 0x000f0000L) >> 12) + ((group2 & 0x000f0000L) >> 8) + ((group3 & 0x000f0000L) >> 4);

    // 注意這裡的 enable 順序是反的
    mc_outp(MC_GENERAL, MCG_ENABLEREG2, mc_inp(MC_GENERAL, MCG_ENABLEREG2) | val2);
    mc_outp(MC_GENERAL, MCG_ENABLEREG1, mc_inp(MC_GENERAL, MCG_ENABLEREG1) | val1);
}

void mc_DisableMulti(unsigned long group1, unsigned long group2, unsigned long group3) {
    unsigned long val1 = (group1 & 0x00000fffL) + ((group2 & 0x00000fffL) << 12) + ((group3 & 0x00000ffL) << 24);
    unsigned long val2 = ((group3 & 0x00000f00L) >> 8) + ((group1 & 0x000f0000L) >> 12) + ((group2 & 0x000f0000L) >> 8) + ((group3 & 0x000f0000L) >> 4);

    mc_outp(MC_GENERAL, MCG_ENABLEREG1, mc_inp(MC_GENERAL, MCG_ENABLEREG1) & (~val1));
    mc_outp(MC_GENERAL, MCG_ENABLEREG2, mc_inp(MC_GENERAL, MCG_ENABLEREG2) & (~val2));
}

unsigned long mc_ReadEnableREG1(void) {
    return mc_inp(MC_GENERAL, MCG_ENABLEREG1);
}
unsigned long mc_ReadEnableREG2(void) {
    return mc_inp(MC_GENERAL, MCG_ENABLEREG2);
}



void mc_SetRouting_MC2SIF(unsigned long group1, unsigned long group2, unsigned long group3) {
    mc_outp(MC_GENERAL, MCG_G1ROUTEREG1, mc_inp(MC_GENERAL, MCG_G1ROUTEREG1) | group1);
    mc_outp(MC_GENERAL, MCG_G2ROUTEREG1, mc_inp(MC_GENERAL, MCG_G2ROUTEREG1) | group2);
    mc_outp(MC_GENERAL, MCG_G3ROUTEREG1, mc_inp(MC_GENERAL, MCG_G3ROUTEREG1) | group3);
}

void mc_RemoveRouting_MC2SIF(unsigned long group1, unsigned long group2, unsigned long group3) {
    mc_outp(MC_GENERAL, MCG_G1ROUTEREG1, mc_inp(MC_GENERAL, MCG_G1ROUTEREG1) & (~group1));
    mc_outp(MC_GENERAL, MCG_G2ROUTEREG1, mc_inp(MC_GENERAL, MCG_G2ROUTEREG1) & (~group2));
    mc_outp(MC_GENERAL, MCG_G3ROUTEREG1, mc_inp(MC_GENERAL, MCG_G3ROUTEREG1) & (~group3));
}

void mc_SetRouting_SIF2MC(unsigned long group1, unsigned long group2, unsigned long group3) {
    mc_outp(MC_GENERAL, MCG_G1ROUTEREG2, mc_inp(MC_GENERAL, MCG_G1ROUTEREG2) | group1);
    mc_outp(MC_GENERAL, MCG_G2ROUTEREG2, mc_inp(MC_GENERAL, MCG_G2ROUTEREG2) | group2);
    mc_outp(MC_GENERAL, MCG_G3ROUTEREG2, mc_inp(MC_GENERAL, MCG_G3ROUTEREG2) | group3);
}

void mc_RemoveRouting_SIF2MC(unsigned long group1, unsigned long group2, unsigned long group3) {
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

void mcpwm_Enable(int mc, int module) {
    unsigned long val  = 1L << ((mc*3 + module) & 31);
    unsigned long reg  = ((mc*3 + module) < 32)? MCG_ENABLEREG1 : MCG_ENABLEREG2;
    
    mc_outp(MC_GENERAL, reg, mc_inp(MC_GENERAL, reg) | val);
}

void mcpwm_Disable(int mc, int module) {
    unsigned long val  = 1L << ((mc*3 + module) & 31);
    unsigned long reg  = ((mc*3 + module) < 32)? MCG_ENABLEREG1 : MCG_ENABLEREG2;

    mc_outp(MC_GENERAL, reg, mc_inp(MC_GENERAL, reg) & ~val);
}

void mcpwm_DisableProtect(int mc, int module) {
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



void mcpwm_SetWaveform(int mc, int module, unsigned long mode) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x3fffffffL) | mode);
}

void mcpwm_SetWidth(int mc, int module, unsigned long period, unsigned long phase0) {
    unsigned long reg = MCPWM_modOffset[module];

    mc_outp(mc, reg + MCPWM_PERREG, period);
    mc_outp(mc, reg + MCPWM_P0REG,  phase0);
}

void mcpwm_SetSamplCycle(int mc, int module, unsigned long sc) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xe0000000L) | (sc & 0x1fffffffL));
}

void mcpwm_SetSCRESET(int mc, int module) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_LDREG;

    mc_outpb(mc, reg, mc_inpb(mc, reg) | 0x02);
}

void mcpwm_ClearSCRESET(int mc, int module) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_LDREG;

    mc_outpb(mc, reg, mc_inpb(mc, reg) & 0xfd);
}

void mcpwm_SetSCLAST(int mc, int module) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_LDREG;

    mc_outpb(mc, reg, mc_inpb(mc, reg) | 0x01);
}

void mcpwm_ClearSCLAST(int mc, int module) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_LDREG;

    mc_outpb(mc, reg, mc_inpb(mc, reg) & 0xfe);
}

unsigned long mcpwm_ReadSCLAST(int mc, int module) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_LDREG;

    return mc_inp(mc, reg) & 1L;
}

unsigned long mcpwm_ReadSCREG(int mc, int module) {
    return mc_inp(mc, MCPWM_modOffset[module] + MCPWM_STATREG1) & 0x1fffffffL;
}



unsigned long mcpwm_ReadSTATREG1(int mc, int module) {
    return mc_inp(mc, MCPWM_modOffset[module] + MCPWM_STATREG1);
}
unsigned long mcpwm_ReadSTATREG2(int mc, int module) {
    return mc_inp(mc, MCPWM_modOffset[module] + MCPWM_STATREG2);
}



void mcpwm_SetDeadband(int mc, int module, unsigned long db) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_OUTCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xff000000L) | (db & 0x00ffffffL));
}

void mcpwm_SetOutMask(int mc, int module, unsigned long mask) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_OUTCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xf0ffffffL) | (mask & 0x0f000000L));
}

void mcpwm_SetOutPolarity(int mc, int module, unsigned long pol) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_OUTCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xcfffffffL) | (pol & 0x30000000L));
}

void mcpwm_SetMPWM(int mc, int module, unsigned long mpwm) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_OUTCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x3fffffffL) | (mpwm & 0xc0000000L));
}



void mcpwm_SetEVTMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_EVTREG2;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x3fffffffL) | (mode << 30));
}

void mcpwm_SetEVT(int mc, int module, unsigned long pwm, unsigned long pwmstat, unsigned long sc) {
    unsigned long reg1 = MCPWM_modOffset[module] + MCPWM_EVTREG1;
    unsigned long reg2 = MCPWM_modOffset[module] + MCPWM_EVTREG2;

    mc_outp(mc, reg1, pwm);
    mc_outp(mc, reg2, (mc_inp(mc, reg2) & 0xc0000000L) | (pwmstat << 29) | sc);
}



void mcpwm_SetFAU1Mode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffc3fL) | (mode << 6));
}
void mcpwm_SetFAU2Mode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfff0ffffL) | (mode << 16));
}
void mcpwm_SetFAU3Mode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xc3ffffffL) | (mode << 26));
}

void mcpwm_SetFAU1Output(int mc, int module, unsigned long mode) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffffffc0L) | mode);
}
void mcpwm_SetFAU2Output(int mc, int module, unsigned long mode) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff03ffL) | (mode << 10));
}
void mcpwm_SetFAU3Output(int mc, int module, unsigned long mode) {
    unsigned long reg = MCPWM_modOffset[module] + MCPWM_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfc0fffffL) | (mode << 20));
}



void mcpwm_ReloadPWM(int mc, int module, unsigned char mode) {
    mc_outpb(mc, MCPWM_modOffset[module] + MCPWM_LDREG + 3L, mode);
}

void mcpwm_ReloadEVT(int mc, int module, unsigned char mode) {
    mc_outpb(mc, MCPWM_modOffset[module] + MCPWM_LDREG + 1L, mode);
}

void mcpwm_ReloadOUT(int mc, int module, unsigned char mode) {
    mc_outpb(mc, MCPWM_modOffset[module] + MCPWM_LDREG + 2L, mode);
}

void mcpwm_ReloadOUT_Unsafe(int mc, int module, unsigned char mode) {
    mcpwm_DisableProtect(mc, module);
    mc_outpb(mc, MCPWM_modOffset[module] + MCPWM_LDREG + 2L, mode);
}

void mcspwm_ReloadOUT_Unsafe(int mc, unsigned char mode) {
    mcpwm_DisableProtect(mc, MCPWM_MODULEA);
    mcpwm_DisableProtect(mc, MCPWM_MODULEB);
    mcpwm_DisableProtect(mc, MCPWM_MODULEC);
    mc_outpb(mc, MCPWM_modOffset[MCPWM_MODULEA] + MCPWM_LDREG + 2L, mode);
}


unsigned char mcpwm_ReadReloadPWM(int mc, int module) {
    return mc_inpb(mc, MCPWM_modOffset[module] + MCPWM_LDREG + 3L) & 0xf0;
}

unsigned char mcpwm_ReadReloadEVT(int mc, int module) {
    return mc_inpb(mc, MCPWM_modOffset[module] + MCPWM_LDREG + 1L) & 0xf0;
}

unsigned char mcpwm_ReadReloadOUT(int mc, int module) {
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

void mcsv_Enable(int mc, int module) {
    mcpwm_Enable(mc, module);
}

void mcsv_Disable(int mc, int module) {
    mcpwm_Disable(mc, module);
}

void mcsv_SetMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x1fffffffL) | mode);
}


void mcsv_SetWidth(int mc, int module, unsigned long period, unsigned long phase0) {
    unsigned long reg = MCSV_modOffset[module];

    mc_outp(mc, reg + MCSV_PERREG, period);
    mc_outp(mc, reg + MCSV_P0REG,  phase0);
}

void mcsv_SetSamplCycle(int mc, int module, unsigned long sc) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xe0000000L) | (sc & 0x1fffffffL));
}



/************************** 2012-04-26 Modify for EX **************************/
void mcsv_SetDDA(int mc, int module, unsigned long dir, unsigned long dda) {
    mc_outp(mc, MCSV_modOffset[module] + MCSV_DDAREG, (dir << 31) | (dda & 0x7fffffffL));
}

unsigned long mcsv_ReadDDA(int mc, int module) {
	return mc_inp(mc, MCSV_modOffset[module] + MCSV_DDAREG);
}

void mcsv_SetStatPulse(int mc, int module) {
    int offset = 9 + module*10;
    mc_outp(mc, MCSV_EXTCTRLREG, mc_inp(mc, MCSV_EXTCTRLREG) | (1L << offset));
}

void mcsv_ClearStatPulse(int mc, int module) {
    int offset = 9 + module*10;
    mc_outp(mc, MCSV_EXTCTRLREG, mc_inp(mc, MCSV_EXTCTRLREG) & ~(1L << offset));
}

void mcsv_SetDirOwner(int mc, int module) {
    int offset = 8 + module*10;
    mc_outp(mc, MCSV_EXTCTRLREG, mc_inp(mc, MCSV_EXTCTRLREG) | (1L << offset));
}

void mcsv_ClearDirOwner(int mc, int module) {
    int offset = 8 + module*10;
    mc_outp(mc, MCSV_EXTCTRLREG, mc_inp(mc, MCSV_EXTCTRLREG) & ~(1L << offset));
}

 void mcsv_SetDDAInt(int mc, int module, unsigned long ddaint) {
   int offset = 4 + module*10;
   unsigned long mask = ~(0x0fL << offset); // 新增
   mc_outp(mc, MCSV_EXTCTRLREG, (mc_inp(mc, MCSV_EXTCTRLREG) & mask) | (ddaint << offset)); // 改變
}

void mcsv_SetDDAMode(int mc, int module, unsigned long mode) {
   int offset = 2 + module*10;
   unsigned long mask = ~(0x03L << offset); // 新增
   mc_outp(mc, MCSV_EXTCTRLREG, (mc_inp(mc, MCSV_EXTCTRLREG) & mask) | (mode << offset)); // 改變
}

unsigned long mcsv_ReadDDASTAT(int mc, int module) {
    int offset = module*10;
    return ((mc_inp(mc, MCSV_EXTCTRLREG) >> offset) & 0x00000003L);
}

unsigned long mcsv_ReadDDAMode(int mc, int module) {
    int offset = 2 + module*10;
    return ((mc_inp(mc, MCSV_EXTCTRLREG) >> offset) & 0x00000003L);
}

unsigned long mcsv_ReadDDAInt(int mc, int module) {
    int offset = 4 + module*10;
    return ((mc_inp(mc, MCSV_EXTCTRLREG) >> offset) & 0x0000000fL);
}

unsigned long mcsv_ReadDirOwner(int mc, int module) {
    int offset = 8 + module*10;
    return ((mc_inp(mc, MCSV_EXTCTRLREG) >> offset) & 1L);
}

unsigned long mcsv_ReadStatPulse(int mc, int module) {
    int offset = 9 + module*10;
    return ((mc_inp(mc, MCSV_EXTCTRLREG) >> offset) & 1L);
}
/******************************** End for EX **********************************/


// may cause racing condition if SCLAST <> 0
void mcsv_SetSCRESET(int mc, int module) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_LDREG;

    mc_outpb(mc, reg, mc_inpb(mc, reg) | 0x02);
}

// may cause racing condition if SCLAST <> 0
void mcsv_ClearSCRESET(int mc, int module) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_LDREG;

    mc_outpb(mc, reg, mc_inpb(mc, reg) & 0xfd);
}


void mcsv_SetSCLAST(int mc, int module) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_LDREG;

    mc_outpb(mc, reg, mc_inpb(mc, reg) | 0x01);
}

void mcsv_ClearSCLAST(int mc, int module) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_LDREG;

    mc_outpb(mc, reg, mc_inpb(mc, reg) & 0xfe);
}

unsigned long mcsv_ReadSCLAST(int mc, int module) {
    return mc_inp(mc, MCSV_modOffset[module] + MCSV_LDREG) & 1L;
}



unsigned long mcsv_ReadSCREG(int mc, int module) {
    return mc_inp(mc, MCSV_modOffset[module] + MCSV_STATREG1) & 0x1fffffffL;
}

unsigned long mcsv_ReadFBSTAT(int mc, int module) {
    return (mc_inp(mc, MCSV_modOffset[module] + MCSV_STATREG1) >> 30) & 0x03L;
}



unsigned long mcsv_ReadSTATREG1(int mc, int module) {
    return mc_inp(mc, MCSV_modOffset[module] + MCSV_STATREG1);
}
unsigned long mcsv_ReadSTATREG2(int mc, int module) {
    return mc_inp(mc, MCSV_modOffset[module] + MCSV_STATREG2);
}



void mcsv_SetEVTMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_EVTREG2;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x3fffffffL) | (mode << 30));
}

void mcsv_SetEVT(int mc, int module, unsigned long pcnt, unsigned long sc) {
    unsigned long reg1 = MCSV_modOffset[module] + MCSV_EVTREG1;
    unsigned long reg2 = MCSV_modOffset[module] + MCSV_EVTREG2;

    mc_outp(mc, reg1, pcnt);
    mc_outp(mc, reg2, (mc_inp(mc, reg2) & 0xc0000000L) | (sc & 0x1fffffffL));
}


void mcsv_SetFAU1Mode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffc3fL) | (mode << 6));
}
void mcsv_SetFAU2Mode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfff0ffffL) | (mode << 16));
}
void mcsv_SetFAU3Mode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xc3ffffffL) | (mode << 26));
}

void mcsv_SetFAU1Output(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffffffc0L) | mode);
}
void mcsv_SetFAU2Output(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff03ffL) | (mode << 10));
}
void mcsv_SetFAU3Output(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_FOUTREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfc0fffffL) | (mode << 20));
}

void mcsv_SetDDAPOS(int mc, int module, unsigned long ddapos){
    mc_outp(mc, module * 0x04 + MCSV_DDAPOSREG, ddapos);
}

unsigned long mcsv_ReadDDAPOS(int mc, int module){
    return mc_inp(mc, module * 0x04 + MCSV_DDAPOSREG);
}

void mcsv_SetSWPIN(int mc, int module){
    unsigned long reg = MCSV_modOffset[module] + MCSV_OUTCTRLREG;
    mc_outp(mc, reg, mc_inp(mc, reg) | 0x80000000L);   
}

void mcsv_ClearSWPIN(int mc, int module){
    unsigned long reg = MCSV_modOffset[module] + MCSV_OUTCTRLREG;
    mc_outp(mc, reg, mc_inp(mc, reg) & 0x7fffffffL);
}



// may cause racing condition if SCLAST <> 0
void mcsv_SetPABInitOut(int mc, int module, unsigned char mode) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_LDREG;

    mc_outpb(mc, reg, (mc_inpb(mc, reg) & 0xf3) | mode);
}

void mcsv_SetOutMask(int mc, int module, unsigned long mask) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_OUTCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xf0ffffffL) | (mask & 0x0f000000L));
}

void mcsv_SetOutPolarity(int mc, int module, unsigned long pol) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_OUTCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xdfffffffL) | (pol & 0x20000000L));
}

void mcsv_SetDIR(int mc, int module, unsigned long dir) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_OUTCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xbfffffffL) | (dir & 0x40000000L));
}

void mcsv_SetDIRDeadband(int mc, int module, unsigned long db) {
    unsigned long reg = MCSV_modOffset[module] + MCSV_OUTCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xff000000L) | (db & 0x00ffffffL));
}



void mcsv_ReloadSV(int mc, int module, unsigned char mode) {
    mc_outpb(mc, MCSV_modOffset[module] + MCSV_LDREG + 3L, mode);
}

void mcsv_ReloadEVT(int mc, int module, unsigned char mode) {
    mc_outpb(mc, MCSV_modOffset[module] + MCSV_LDREG + 1L, mode);
}

void mcsv_ReloadOUT(int mc, int module, unsigned char mode) {
    mc_outpb(mc, MCSV_modOffset[module] + MCSV_LDREG + 2L, mode);
}

unsigned char mcsv_ReadReloadSV(int mc, int module) {
    return mc_inpb(mc, MCSV_modOffset[module] + MCSV_LDREG + 3L) & 0xf0;
}

unsigned char mcsv_ReadReloadEVT(int mc, int module) {
    return mc_inpb(mc, MCSV_modOffset[module] + MCSV_LDREG + 1L) & 0xf0;
}

unsigned char mcsv_ReadReloadOUT(int mc, int module) {
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

void mcsif_Enable(int mc, int module) {
    if (module == MCSIF_MODULEA)
        mcpwm_Enable(mc, module);
    else
        mc_outp(MC_GENERAL, MCG_ENABLEREG2, mc_inp(MC_GENERAL, MCG_ENABLEREG2) | (1L << (mc + 4)));
}

void mcsif_Disable(int mc, int module) {
    if (module == MCSIF_MODULEA)
        mcpwm_Disable(mc, module);
    else
        mc_outp(MC_GENERAL, MCG_ENABLEREG2, mc_inp(MC_GENERAL, MCG_ENABLEREG2) & ~(1L << (mc + 4)));
}

void mcsif_SetMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff1fffL) | mode);
}



void mcsif_SetSamplWin(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffffff00L) | mode);
}

void mcsif_SetSWPOL(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffcffL) | mode);
}

void mcsif_SetSWDeadband(int mc, int module, unsigned long db) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_DBREG, db & 0x00ffffffL);
}

unsigned long mcsif_ReadSWSTAT(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_CTRLREG) & 0x00000400L) >> 10;
}

void mcsif_SetInputFilter(int mc, int module, unsigned long filtime) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_FILREG, filtime & 0xc000ffffL);
}



void mcsif_EnableINT(int mc, int module, unsigned long intsort) {
    intsort = intsort << MCSIF_intOffset[module];
    mc_outp(mc, 0x00L, mc_inp(mc, 0x00L) | intsort);
}

void mcsif_DisableINT(int mc, int module, unsigned long intsort) {
    intsort = intsort << MCSIF_intOffset[module];
    mc_outp(mc, 0x00L, mc_inp(mc, 0x00L) & (~intsort));
}

bool mcsif_CheckINT(int mc, int module, unsigned long intsort) {
    intsort = intsort << MCSIF_intOffset[module];

    if ((mc_inp(mc, 0x04L) & intsort) == 0L) return false;
    return true;
}

void mcsif_ClearINT(int mc, int module, unsigned long intsort) {
    intsort = intsort << MCSIF_intOffset[module];
    mc_outp(mc, 0x04L, intsort);
}



// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^   Encoder Modes  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

void mcenc_SetCntMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_ENC_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff00ffL) | mode);
}

void mcenc_SetResetMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_ENC_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffffff00L) | mode);
}

void mcenc_SetTrigResetMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_ENC_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x1fffffffL) | mode);
}



void mcenc_SetPulCnt(int mc, int module, unsigned long pcnt) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_ENC_PCNTREG, pcnt & 0xffffffffL);
}

unsigned long mcenc_ReadPulCnt(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_PCNTREG) & 0xffffffffL;
}

unsigned long mcenc_ReadDIR(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_STATREG) >> 4) & 0x01L;
}

unsigned long mcenc_ReadPINS(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_STATREG) >> 5) & 0x07L;
}

void mcenc_SetCntMin(int mc, int module, unsigned long cmin) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_ENC_LIMREG1, cmin & 0xffffffffL);
}

void mcenc_SetCntMax(int mc, int module, unsigned long cmax) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_ENC_LIMREG2, cmax & 0xffffffffL);
}

void mcenc_SetCntIdx(int mc, int module, unsigned long cidx) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_ENC_IDXREG, cidx & 0xffffffffL);
}

void mcenc_SetCntEvt(int mc, int module, unsigned long cevt) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_ENC_EVTREG, cevt & 0xffffffffL);
}



void mcenc_SetIdxCond(int mc, int module, unsigned long mode, unsigned long abz) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_ENC_IDXCONDREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffffff18L) | (mode | abz));
}

bool mcenc_CheckIDXERR(int mc, int module) {
    if ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_STATREG) & 0x08L) != 0L) return true;
    return false;
}

void mcenc_ClearIDXERR(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_ENC_STATREG;

    mc_outp(mc, reg, mc_inp(mc, reg) | 0x08L);
}

bool mcenc_CheckIDXSTAT(int mc, int module) {
    if ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_STATREG) & 0x04L) != 0L) return true;
    return false;
}



void mcenc_SetCapMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_ENC_CAPCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfff0ffffL) | mode);
}

void mcenc_SetCapInterval(int mc, int module, unsigned long capivl) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_ENC_CAPCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffe00L) | (capivl & 0x1ffL));
}

unsigned long mcenc_ReadCAPSTAT(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_STATREG) & 0x03L;
}

unsigned long mcenc_ReadCAPFIFO(int mc, int module, unsigned long* data) {
    unsigned long tmp = mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_CAPFIFOREG);
    
    if (data != NULL) *data = tmp & 0x0fffffffL;
    return tmp & 0xf0000000L;
}

void mcenc_SetRLDTRIG(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_ENC_CAPCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffcfffffL) | mode);
}

unsigned long mcenc_ReadRLDTRIG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_CAPCTRLREG) & 0x00300000L;
}

unsigned long mcenc_ReadPINAInput(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_STATREG) >> 7) & 0x01L);
}

unsigned long mcenc_ReadPINBInput(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_STATREG) >> 6) & 0x01L);
}

unsigned long mcenc_ReadPINZInput(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_STATREG) >> 5) & 0x01L);
}



void mcenc_SetCtrlREG(int mc, int module, unsigned long val) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_ENC_CTRLREG, val);
}

unsigned long mcenc_ReadCtrlREG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_CTRLREG);
}

void mcenc_SetIdxCondREG(int mc, int module, unsigned long val) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_ENC_IDXCONDREG, val);
}

unsigned long mcenc_ReadIdxCondREG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_IDXCONDREG);
}

void mcenc_SetCapCtrlREG(int mc, int module, unsigned long val) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_ENC_CAPCTRLREG, val);
}

unsigned long mcenc_ReadCapCtrlREG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_ENC_CAPCTRLREG);
}



// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^   Hall Mode  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void mchall_SetCntMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_HALL_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff00ffL) | mode);
}

void mchall_SetResetMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_HALL_CTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffffffccL) | mode);
}

void mchall_SetDirPattern(int mc, int module, unsigned long pattern) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_HALL_CTRLREG;

    pattern = ((pattern & 0x0003fffcL) << 14) | ((pattern & 0x00000003L) << 6);
    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x0000ff3fL) | pattern);
}



void mchall_SetHallCnt(int mc, int module, unsigned long pcnt) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_HALL_PCNTREG, pcnt & 0xffffffffL);
}

unsigned long mchall_ReadHallCnt(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_PCNTREG) & 0xffffffffL;
}

unsigned long mchall_ReadDIR(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_STATREG) >> 4) & 0x01L;
}

unsigned long mchall_ReadPINS(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_STATREG) >> 5) & 0x07L;
}

void mchall_SetCntMin(int mc, int module, unsigned long cmin) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_HALL_LIMREG1, cmin & 0xffffffffL);
}

void mchall_SetCntMax(int mc, int module, unsigned long cmax) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_HALL_LIMREG2, cmax & 0xffffffffL);
}

void mchall_SetCntEvt(int mc, int module, unsigned long cevt) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_HALL_EVTREG, cevt & 0xffffffffL);
}



bool mchall_CheckSTERR(int mc, int module) {
    if ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_STATREG) & 0x08L) != 0L) return true;
    return false;
}

void mchall_ClearSTERR(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_HALL_STATREG;

    mc_outp(mc, reg, mc_inp(mc, reg) | 0x08L);
}



void mchall_SetCapMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_HALL_CAPCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfff8ffffL) | mode);
}

void mchall_SetCapInterval(int mc, int module, unsigned long capivl) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_HALL_CAPCTRLREG;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffe00L) | (capivl & 0x1ffL));
}

unsigned long mchall_ReadCAPSTAT(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_STATREG) & 0x03L;
}

unsigned long mchall_ReadCAPFIFO(int mc, int module, unsigned long* data) {
    unsigned long tmp = mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_CAPFIFOREG);
    
    if (data != NULL) *data = tmp & 0x0fffffffL;
    return tmp & 0xf0000000L;
}

unsigned long mchall_ReadPINAInput(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_STATREG) >> 7) & 0x01L);
}

unsigned long mchall_ReadPINBInput(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_STATREG) >> 6) & 0x01L);
}

unsigned long mchall_ReadPINZInput(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_STATREG) >> 5) & 0x01L);
}



void mchall_SetCtrlREG(int mc, int module, unsigned long val) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_HALL_CTRLREG, val);
}

unsigned long mchall_ReadCtrlREG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_CTRLREG);
}

void mchall_SetCapCtrlREG(int mc, int module, unsigned long val) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_HALL_CAPCTRLREG, val);
}

unsigned long mchall_ReadCapCtrlREG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_HALL_CAPCTRLREG);
}



// ^^^^^^^^^^^^^^^^^^^^^^^   Pulse & Fault Capture Mode  ^^^^^^^^^^^^^^^^^^^^^^^

void mcpfau_SetCapMode1(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfff0ffffL) | mode);
}
void mcpfau_SetCapMode2(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfff0ffffL) | mode);
}
void mcpfau_SetCapMode3(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfff0ffffL) | mode);
}

void mcpfau_SetCapInterval1(int mc, int module, unsigned long capivl) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffe00L) | (capivl & 0x1ffL));
}
void mcpfau_SetCapInterval2(int mc, int module, unsigned long capivl) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffe00L) | (capivl & 0x1ffL));
}
void mcpfau_SetCapInterval3(int mc, int module, unsigned long capivl) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffe00L) | (capivl & 0x1ffL));
}

unsigned long mcpfau_ReadPIN1Input(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_STATREG) >> 7) & 0x01L);
}

unsigned long mcpfau_ReadPIN2Input(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_STATREG) >> 6) & 0x01L);
}

unsigned long mcpfau_ReadPIN3Input(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_STATREG) >> 5) & 0x01L);
}

unsigned long mcpfau_ReadCAPSTAT1(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_STATREG) & 0x03L;
}
unsigned long mcpfau_ReadCAPSTAT2(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_STATREG) & 0x0300L) >> 8;
}
unsigned long mcpfau_ReadCAPSTAT3(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_STATREG) & 0x0c00L) >> 10;
}

unsigned long mcpfau_ReadCAPFIFO1(int mc, int module, unsigned long* data) {
    unsigned long tmp = mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPFIFOREG1);
    
    if (data != NULL) *data = tmp & 0x0fffffffL;
    return tmp & 0xf0000000L;
}
unsigned long mcpfau_ReadCAPFIFO2(int mc, int module, unsigned long* data) {
    unsigned long tmp = mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPFIFOREG2);
    
    if (data != NULL) *data = tmp & 0x0fffffffL;
    return tmp & 0xf0000000L;
}
unsigned long mcpfau_ReadCAPFIFO3(int mc, int module, unsigned long* data) {
    unsigned long tmp = mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPFIFOREG3);
    
    if (data != NULL) *data = tmp & 0x0fffffffL;
    return tmp & 0xf0000000L;
}



void mcpfau_SetPolarity1(int mc, int module, unsigned long pol) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x7fffffffL) | pol);
}
void mcpfau_SetPolarity2(int mc, int module, unsigned long pol) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x7fffffffL) | pol);
}
void mcpfau_SetPolarity3(int mc, int module, unsigned long pol) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x7fffffffL) | pol);
}

void mcpfau_SetMask1(int mc, int module, unsigned long mask) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1;

	mc_outp(mc, reg, (mc_inp(mc, reg) & 0x8fffffffL) | mask);
}
void mcpfau_SetMask2(int mc, int module, unsigned long mask) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2;
    
    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x8fffffffL) | mask);
}
void mcpfau_SetMask3(int mc, int module, unsigned long mask) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0x8fffffffL) | mask);
}



void mcpfau_SetRLDTRIG1(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffcfffffL) | mode);
}
void mcpfau_SetRLDTRIG2(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffcfffffL) | mode);
}
void mcpfau_SetRLDTRIG3(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffcfffffL) | mode);
}

unsigned long mcpfau_ReadRLDTRIG1(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1) & 0x00300000L;
}
unsigned long mcpfau_ReadRLDTRIG2(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2) & 0x00300000L;
}
unsigned long mcpfau_ReadRLDTRIG3(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3) & 0x00300000L;
}



void mcpfau_SetEXTRIG1(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xff3fffffL) | mode);
}
void mcpfau_SetEXTRIG2(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xff3fffffL) | mode);
}
void mcpfau_SetEXTRIG3(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xff3fffffL) | mode);
}

unsigned long mcpfau_ReadEXTRIG1(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1) & 0x00c00000L;
}
unsigned long mcpfau_ReadEXTRIG2(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2) & 0x00c00000L;
}
unsigned long mcpfau_ReadEXTRIG3(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3) & 0x00c00000L;
}



unsigned long mcpfau_ReadFAU1STAT(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1) >> 28) & 0x01L;
}
unsigned long mcpfau_ReadFAU2STAT(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2) >> 28) & 0x01L;
}
unsigned long mcpfau_ReadFAU3STAT(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3) >> 28) & 0x01L;
}

void mcpfau_ClearFAU1STAT(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1;

    mc_outp(mc, reg, mc_inp(mc, reg) | (1L << 28));
}
void mcpfau_ClearFAU2STAT(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2;

    mc_outp(mc, reg, mc_inp(mc, reg) | (1L << 28));
}
void mcpfau_ClearFAU3STAT(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3;

    mc_outp(mc, reg, mc_inp(mc, reg) | (1L << 28));
}

void mcpfau_SetFAU1RELS(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xf3ffffffL) | mode);
}
void mcpfau_SetFAU2RELS(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xf3ffffffL) | mode);
}
void mcpfau_SetFAU3RELS(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xf3ffffffL) | mode);
}

void mcpfau_SetFAU1TRIG(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfcffffffL) | mode);
}
void mcpfau_SetFAU2TRIG(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfcffffffL) | mode);
}
void mcpfau_SetFAU3TRIG(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3;

    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfcffffffL) | mode);
}



unsigned long mcpfau_ReadCap1CtrlREG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1);
}
unsigned long mcpfau_ReadCap2CtrlREG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2);
}
unsigned long mcpfau_ReadCap3CtrlREG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3);
}

unsigned long mcpfau_ReadCapStatREG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_PFAU_STATREG);
}

#define MCSIF_PFAU_CAPCTRLREG3  (0x14L)
#define MCSIF_PFAU_CAPCTRLREG2  (0x20L)
#define MCSIF_PFAU_CAPCTRLREG1  (0x2cL)
void mcpfau_SetCap1INT(int mc, int module, unsigned long interval)
{
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1;
	interval = (interval << 11L) & 0x0000f800L;
    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff07ffL) + interval);
}

void mcpfau_SetCap2INT(int mc, int module, unsigned long interval)
{
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2;
	interval = (interval << 11L) & 0x0000f800L;
    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff07ffL) + interval);
}

void mcpfau_SetCap3INT(int mc, int module, unsigned long interval)
{
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3;
	interval = (interval << 11L) & 0x0000f800L;
    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff07ffL) + interval);
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^   SSI Mode  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void mcssi_SetPulCnt(int mc, int module, unsigned long pcnt) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_SSI_PCNTREG, pcnt & 0x0fffffffL);
}

unsigned long mcssi_ReadPulCnt(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_SSI_PCNTREG) & 0x0fffffffL;
}

unsigned long mcssi_ReadData(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_SSI_DATAREG) & 0xffffffffL;
}

void mcssi_SetClock(int mc, int module, unsigned long clk) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_SSI_CLKREG, clk & 0x0fffffffL);
}

unsigned long mcssi_ReadClock(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_SSI_CLKREG) & 0x0fffffffL;
}

void mcssi_SetWaitTime(int mc, int module, unsigned long waittime) {
    mc_outp(mc, MCSIF_modOffset[module] + MCSIF_SSI_WAITREG, waittime & 0x0fffffffL);
}

unsigned long mcssi_ReadWaitTime(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_SSI_WAITREG) & 0x0fffffffL;
}

void mcssi_SetLatchPhase(int mc, int module, unsigned long phase) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_TIMEREG;
	mc_outp(mc, reg, (mc_inp(mc, reg) & 0x7fffffffL) | phase);
}

unsigned long mcssi_ReadLatchPhase(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_TIMEREG;
	return (mc_inp(mc, reg) & ~(0x7fffffffL)) >> 31;
}

void mcssi_SetLatchTime(int mc, int module, unsigned long latchtime) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_TIMEREG;
    latchtime = latchtime & 0x0fffffffL;
	mc_outp(mc, reg, (mc_inp(mc, reg) & 0xf0000000L) | latchtime);
}

unsigned long mcssi_ReadLatchTime(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_TIMEREG;
	return (mc_inp(mc, reg) & ~(0xf0000000L));
}

void mcssi_SetNumberBITS(int mc, int module, unsigned long numberbits) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	numberbits = numberbits & 0x1fL;
	mc_outp(mc, reg, (mc_inp(mc, reg) & 0x07ffffffL) | (numberbits << 27));
}

unsigned long mcssi_ReadNumberBITS(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	return (mc_inp(mc, reg) & ~(0x07ffffffL)) >> 27;
}

void mcssi_SetNumberRUNS(int mc, int module, unsigned long numberruns) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	numberruns = numberruns & 0x1fL;
	mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffe0ffffL) | (numberruns << 16));
}

unsigned long mcssi_ReadNumberRUNS(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	return (mc_inp(mc, reg) & ~(0xffe0ffffL)) >> 16;
}

void mcssi_SetCntMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff0fffL) | mode);
}

void mcssi_SetExternalBit(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	mc_outp(mc, reg, mc_inp(mc, reg) | (0x01L << 10));
}

void mcssi_ClearExternalBit(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffbffL));
}

void mcssi_SetChkErrBit(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	mc_outp(mc, reg, mc_inp(mc, reg) | (0x01L << 9));
}

void mcssi_ClearChkErrBit(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffdffL));
}

void mcssi_SetGAY2BINBit(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	mc_outp(mc, reg, mc_inp(mc, reg) | (0x01L << 8));
}

void mcssi_ClearGAY2BINBit(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG;
	mc_outp(mc, reg, (mc_inp(mc, reg) & 0xfffffeffL));
}

unsigned long mcssi_ReadRunNumber(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_SSI_STATREG) >> 8) & 0x1fL);
}

unsigned long mcssi_ReadSSISTAT(int mc, int module) {
    return ((mc_inp(mc, MCSIF_modOffset[module] + MCSIF_SSI_STATREG) >> 5) & 0x07L);
}

bool mcssi_CheckERRBit(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_STATREG;
	if((mc_inp(mc, reg) & 0x08L) != 0) return true;
	return false;
}

void mcssi_ClearERRBit(int mc, int module) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_STATREG;
	mc_outp(mc, reg, mc_inp(mc, reg) | 0x08L);
}

unsigned long mcssi_ReadCAPSTAT(int mc, int module) {
    return (mc_inp(mc, MCSIF_modOffset[module] + MCSIF_SSI_STATREG) & 0x03L);
}

void mcssi_SetRLDTRIG(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CAPCTRLREG;
	mc_outp(mc, reg, mc_inp(mc, reg) & 0xffcfffffL | mode);
}

void mcssi_SetCAPMode(int mc, int module, unsigned long mode) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CAPCTRLREG;
	mc_outp(mc, reg, mc_inp(mc, reg) & 0xfffcffffL | mode);
}

void mcssi_SetCAPINT(int mc, int module, unsigned long number) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CAPCTRLREG;
    number = number & 0x1FL;
	mc_outp(mc, reg, mc_inp(mc, reg) & 0xffff0effL | (number << 11));
}

void mcssi_SetCapInterval(int mc, int module, unsigned long number) {
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_SSI_CAPCTRLREG;
    number = number & 0x1FFL;
	mc_outp(mc, reg, mc_inp(mc, reg) & 0xfffffe00L | number);
}

unsigned long mcssi_ReadCAPFIFO(int mc, int module, unsigned long* data) {
    unsigned long tmp = mc_inp(mc, MCSIF_modOffset[module] + MCSIF_SSI_CAPFIFOREG);
    
    if (data != NULL) *data = tmp;
    return 0L;
}

unsigned long mcssi_ReadCtrlREG(int mc, int module) {
    return mc_inp(mc, MCSIF_modOffset[module] + MCSIF_SSI_CTRLREG);
}


// MCM INTERRUPT SETTING FUNCTION //
static unsigned char int_routing_table[16] = {0xff, 0x08, 0xff, 0x02, 0x04, 0x05, 0x07, 0x06, 0xff, 0x01, 0x03, 0x09, 0x0b, 0xff, 0x0d, 0x0f};
unsigned char GetMCIRQ(void) {  
    return (unsigned char)(read_mc_pcireg(0x3c) & 0xffL);
}

void Set_MCIRQ(unsigned char irq) {
    write_mc_pcireg(0x3c, (read_mc_pcireg(0x3c) & 0xffffff00L) | (unsigned long) irq);
    sb1_Write8(0xb4, (sb1_Read8(0xb4) & 0xf0) | int_routing_table[irq]);
}
