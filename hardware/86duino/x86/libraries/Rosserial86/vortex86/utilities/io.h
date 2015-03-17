/*******************************************************************************

  io.h - DM&P Vortex86 Base I/O Library
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

  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)

*******************************************************************************/


#ifndef __IO_H
#define __IO_H

#include <stdio.h>
#include <dmpcfg.h>


#ifdef __cplusplus
extern "C" {
#endif

/* =============================================
 *   IRQ & PIC Util Functions
 * =============================================
 */
DMPAPI(void) io_DisableINT(void);
DMPAPI(void) io_RestoreINT(void);
DMPAPI(void) io_EnableINT(void);

DMPAPI(void) i8259_DisableIRQ(int irq);
DMPAPI(void) i8259_EnableIRQ(int irq);
DMPAPI(bool) i8259_IsIRQEnabled(int irq);
DMPAPI(void) i8259_SendEOI(int irq);
DMPAPI(int)  i8259_GetINTVEC(int irq);


/* =============================================
 *   DPMI Functions
 * =============================================
 */
#if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM)
    DMPAPI(unsigned long) dpmi_LinMapAlloc(unsigned long phyaddr, unsigned long size);
    DMPAPI(void) dpmi_LinMapFree(unsigned long linaddr);
    DMPAPI(int)  dpmi_SelAlloc(unsigned long linaddr, unsigned long size);
    DMPAPI(bool) dpmi_SelFree(int selector);
    DMPAPI(bool) dpmi_LockCode(void* code_addr, unsigned long code_size);
    DMPAPI(bool) dpmi_LockData(void* data_addr, unsigned long data_size);
    DMPAPI(bool) dpmi_UnlockCode(void* code_addr, unsigned long code_size);
    DMPAPI(bool) dpmi_UnlockData(void* data_addr, unsigned long data_size);
    DMPAPI(int)  dpmi_Cli(void);
    DMPAPI(int)  dpmi_Sti(void);
#endif

#if defined(DMP_DOS_DJGPP)
    #ifndef DMP_DOS_DJGPP_NO_VIRMEM
        #define DPMI_END_OF_LOCKED_FUNC(fname)         void fname##_end(void) { }
        #define DPMI_END_OF_LOCKED_STATIC_FUNC(fname)  static void fname##_end(void) { }

        // note that to use the DPMI_LOCK_FUNC() marco, you should avoid compiling 
        // the program, where the locked function is, with gcc optimization options 
        // such as -O2 or -O3.
        #define DPMI_LOCK_FUNC(fname)   dpmi_LockCode((void*)fname, (unsigned long)fname##_end - (unsigned long)fname)
        #define DPMI_LOCK_VAR(varname)  dpmi_LockData((void*)&varname, sizeof(varname))
    #else
        #define DPMI_END_OF_LOCKED_FUNC(fname)
        #define DPMI_END_OF_LOCKED_STATIC_FUNC(fname)
        #define DPMI_LOCK_FUNC(fname)
        #define DPMI_LOCK_VAR(varname)
    #endif

    // note that DPMI_MEMORY_ALL_LOCK() macro should be put in the .C/.CPP file  
    // where the main function locates, and DPMI_MEMORY_HEAP_UNLOCK(flag) 
    // should be put within the main function. The two macros ensure that 
    // .data, .bss, .text, and stack segments are locked, provided that the 
    // total size of these segments is less than the size of physical memory, 
    // and ensure that the heap is unlocked.
    #include <crt0.h>
    #define DPMI_MEMORY_ALL_LOCK(flag)      int _crt0_startup_flags = (flag) | _CRT0_FLAG_LOCK_MEMORY | _CRT0_FLAG_NONMOVE_SBRK;
    #define DPMI_MEMORY_HEAP_UNLOCK(flag)   _crt0_startup_flags &= ~_CRT0_FLAG_LOCK_MEMORY & (~(flag));
#endif


/* =============================================
 *   OS Kernel Functions
 * =============================================
 */
DMPAPI(void*) ker_Malloc(size_t size);
DMPAPI(void) ker_Mfree(void* p);


/* =============================================
 *   I/O Access Functions
 * =============================================
 */
DMPAPI(void*) io_Alloc(int io_type, unsigned long io_phyaddr, unsigned long io_size);
//-- values of io_type:
     #define IO_USE_PORTIO   (0)
     #define IO_USE_MMIO     (1)

DMPAPI(bool) io_Free(void* handle);

DMPAPI(void) io_Out32(void* handle, unsigned long offset, unsigned long val);
DMPAPI(unsigned long) io_In32(void* handle, unsigned long offset);

DMPAPI(void) io_Out16(void* handle, unsigned long offset, unsigned short val);
DMPAPI(unsigned short) io_In16(void* handle, unsigned long offset);

DMPAPI(void) io_Out8(void* handle, unsigned long offset, unsigned char val);
DMPAPI(unsigned char) io_In8(void* handle, unsigned long offset);

// just slight faster MMIO function calls for slower CPUs
DMPAPI(void) io_Out32M(void* handle, unsigned long offset, unsigned long val);
DMPAPI(void) io_Out16M(void* handle, unsigned long offset, unsigned short val);
DMPAPI(void) io_Out8M(void* handle, unsigned long offset, unsigned char val);
DMPAPI(unsigned long)  io_In32M(void* handle, unsigned long offset);
DMPAPI(unsigned short) io_In16M(void* handle, unsigned long offset);
DMPAPI(unsigned char)  io_In8M(void* handle, unsigned long offset);

// just slight faster port-I/O function calls for slower CPUs
DMPAPI(void) io_Out32P(void* handle, unsigned long offset, unsigned long val);
DMPAPI(void) io_Out16P(void* handle, unsigned long offset, unsigned short val);
DMPAPI(void) io_Out8P(void* handle, unsigned long offset, unsigned char val);
DMPAPI(unsigned long)  io_In32P(void* handle, unsigned long offset);
DMPAPI(unsigned short) io_In16P(void* handle, unsigned long offset);
DMPAPI(unsigned char)  io_In8P(void* handle, unsigned long offset);


/* =============================================
 *   PCI-CFG Access Functions
 * =============================================
 */
DMPAPI(void*) pci_Alloc(unsigned char bus, unsigned char dev, unsigned char fun);
DMPAPI(void*) pci_Find(unsigned short vid, unsigned short did);
DMPAPI(bool)  pci_Free(void* handle);

DMPAPI(void) pci_Out32(void* handle, unsigned char offset, unsigned long val);
DMPAPI(unsigned long) pci_In32(void* handle, unsigned char offset);

DMPAPI(void) pci_Out16(void* handle, unsigned char offset, unsigned short val);
DMPAPI(unsigned short) pci_In16(void* handle, unsigned char offset);

DMPAPI(void) pci_Out8(void* handle, unsigned char offset, unsigned char val);
DMPAPI(unsigned char) pci_In8(void* handle, unsigned char offset);


/* =============================================
 *   Vortex86 Util Functions
 * =============================================
 */
DMPAPI(int) vx86_CpuID(void);
//-- return values:
     #define CPU_UNSUPPORTED            (0)
     #define CPU_VORTEX86SX             (11)
     #define CPU_VORTEX86DX_UNKNOWN     (20)
     #define CPU_VORTEX86DX_A           (21)
     #define CPU_VORTEX86DX_C           (22)
     #define CPU_VORTEX86DX_D           (23)
     #define CPU_VORTEX86MX             (31)
     #define CPU_VORTEX86MX_PLUS        (32)
     #define CPU_VORTEX86DX2            (41)
     #define CPU_VORTEX86DX3            (42)
     #define CPU_VORTEX86EX             (51)

DMPAPI(unsigned long) vx86_CpuCLK(void);
DMPAPI(unsigned long) vx86_DramCLK(void);

DMPAPI(void) vx86_NBSB_Write(int fun, unsigned char offset, unsigned long val);
DMPAPI(unsigned long) vx86_NBSB_Read(int fun, unsigned char offset);

DMPAPI(void) vx86_NBSB_Write16(int fun, unsigned char offset, unsigned short val);
DMPAPI(unsigned short) vx86_NBSB_Read16(int fun, unsigned char offset);

DMPAPI(void) vx86_NBSB_Write8(int fun, unsigned char offset, unsigned char val);
DMPAPI(unsigned char) vx86_NBSB_Read8(int fun, unsigned char offset);


/* =============================================
 *   CPU-Time Util Functions
 * =============================================
 */
#ifdef DMP_DOS_BC
    DMPAPI(void) timer_GetClocks64(unsigned long* nowclocks_lsb, unsigned long* nowclocks_msb);
#else
    DMPAPI(unsigned long long) timer_GetClocks64(void);
#endif
DMPAPI(unsigned long) timer_GetClocks(void);
DMPAPI(void) timer_Delay(unsigned long ms);
DMPAPI(void) timer_DelayMicroseconds(unsigned long us);
DMPAPI(unsigned long) timer_NowTime(void);  // unit: ms


/* ========================================
 *   I/O LIB Setup Functions
 * ========================================
 */
DMPAPI(bool) io_Init(void);
DMPAPI(bool) io_Close(void);

#ifdef __cplusplus
}
#endif


/******************************  Inline Functions  ****************************/
#ifdef DMP_LIB_DLL  // use no inline functions for DLL
#ifdef __cplusplus
extern "C" {
#endif
    DMPAPI(void) io_outpdw(unsigned short addr, unsigned long val);
    DMPAPI(void) io_outpw(unsigned short addr, unsigned short val);
    DMPAPI(void) io_outpb(unsigned short addr, unsigned char val);
    DMPAPI(unsigned long)  io_inpdw(unsigned short addr);
    DMPAPI(unsigned short) io_inpw(unsigned short addr);
    DMPAPI(unsigned char)  io_inpb(unsigned short addr);

    DMPAPI(void) nb_Write(unsigned char offset, unsigned long val);
    DMPAPI(void) nb_Write16(unsigned char offset, unsigned short val);
    DMPAPI(void) nb_Write8(unsigned char offset, unsigned char val);
    DMPAPI(unsigned long)  nb_Read(unsigned char offset);
    DMPAPI(unsigned short) nb_Read16(unsigned char offset);
    DMPAPI(unsigned char)  nb_Read8(unsigned char offset);

    DMPAPI(void) nb1_Write(unsigned char offset, unsigned long val);
    DMPAPI(void) nb1_Write16(unsigned char offset, unsigned short val);
    DMPAPI(void) nb1_Write8(unsigned char offset, unsigned char val);
    DMPAPI(unsigned long)  nb1_Read(unsigned char offset);
    DMPAPI(unsigned short) nb1_Read16(unsigned char offset);
    DMPAPI(unsigned char)  nb1_Read8(unsigned char offset);

    DMPAPI(void) sb_Write(unsigned char offset, unsigned long val);
    DMPAPI(void) sb_Write16(unsigned char offset, unsigned short val);
    DMPAPI(void) sb_Write8(unsigned char offset, unsigned char val);
    DMPAPI(unsigned long)  sb_Read(unsigned char offset);
    DMPAPI(unsigned short) sb_Read16(unsigned char offset);
    DMPAPI(unsigned char)  sb_Read8(unsigned char offset);

    DMPAPI(void) sb1_Write(unsigned char offset, unsigned long val);
    DMPAPI(void) sb1_Write16(unsigned char offset, unsigned short val);
    DMPAPI(void) sb1_Write8(unsigned char offset, unsigned char val);
    DMPAPI(unsigned long)  sb1_Read(unsigned char offset);
    DMPAPI(unsigned short) sb1_Read16(unsigned char offset);
    DMPAPI(unsigned char)  sb1_Read8(unsigned char offset);
#ifdef __cplusplus
}
#endif
#endif

#if !defined(DMP_LIB_DLL) || defined(__IO_LIB)
    DMP_INLINE(void) io_outpdw(unsigned short addr, unsigned long val) {
        io_Out32(NULL, (unsigned long)addr, val);
    }
    DMP_INLINE(void) io_outpw(unsigned short addr, unsigned short val) {
        io_Out16(NULL, (unsigned long)addr, val);
    }
    DMP_INLINE(void) io_outpb(unsigned short addr, unsigned char val) {
        io_Out8(NULL, (unsigned long)addr, val);
    }
    DMP_INLINE(unsigned long) io_inpdw(unsigned short addr) {
        return io_In32(NULL, (unsigned long)addr);
    }
    DMP_INLINE(unsigned short) io_inpw(unsigned short addr) {
        return io_In16(NULL, (unsigned long)addr);
    }
    DMP_INLINE(unsigned char) io_inpb(unsigned short addr) {
        return io_In8(NULL, (unsigned long)addr);
    }


    DMP_INLINE(void) nb_Write(unsigned char offset, unsigned long val) {
        vx86_NBSB_Write(0, offset, val);
    }
    DMP_INLINE(void) nb_Write16(unsigned char offset, unsigned short val) {
        vx86_NBSB_Write16(0, offset, val);
    }
    DMP_INLINE(void) nb_Write8(unsigned char offset, unsigned char val) {
        vx86_NBSB_Write8(0, offset, val);
    }
    DMP_INLINE(unsigned long) nb_Read(unsigned char offset) {
        return vx86_NBSB_Read(0, offset);
    }
    DMP_INLINE(unsigned short) nb_Read16(unsigned char offset) {
        return vx86_NBSB_Read16(0, offset);
    }
    DMP_INLINE(unsigned char) nb_Read8(unsigned char offset) {
        return vx86_NBSB_Read8(0, offset);
    }


    DMP_INLINE(void) nb1_Write(unsigned char offset, unsigned long val) {
        vx86_NBSB_Write(1, offset, val);
    }
    DMP_INLINE(void) nb1_Write16(unsigned char offset, unsigned short val) {
        vx86_NBSB_Write16(1, offset, val);
    }
    DMP_INLINE(void) nb1_Write8(unsigned char offset, unsigned char val) {
        vx86_NBSB_Write8(1, offset, val);
    }
    DMP_INLINE(unsigned long) nb1_Read(unsigned char offset) {
        return vx86_NBSB_Read(1, offset);
    }
    DMP_INLINE(unsigned short) nb1_Read16(unsigned char offset) {
        return vx86_NBSB_Read16(1, offset);
    }
    DMP_INLINE(unsigned char) nb1_Read8(unsigned char offset) {
        return vx86_NBSB_Read8(1, offset);
    }


    DMP_INLINE(void) sb_Write(unsigned char offset, unsigned long val) {
        vx86_NBSB_Write(2, offset, val);
    }
    DMP_INLINE(void) sb_Write16(unsigned char offset, unsigned short val) {
        vx86_NBSB_Write16(2, offset, val);
    }
    DMP_INLINE(void) sb_Write8(unsigned char offset, unsigned char val) {
        vx86_NBSB_Write8(2, offset, val);
    }
    DMP_INLINE(unsigned long) sb_Read(unsigned char offset) {
        return vx86_NBSB_Read(2, offset);
    }
    DMP_INLINE(unsigned short) sb_Read16(unsigned char offset) {
        return vx86_NBSB_Read16(2, offset);
    }
    DMP_INLINE(unsigned char) sb_Read8(unsigned char offset) {
        return vx86_NBSB_Read8(2, offset);
    }


    DMP_INLINE(void) sb1_Write(unsigned char offset, unsigned long val) {
        vx86_NBSB_Write(3, offset, val);
    }
    DMP_INLINE(void) sb1_Write16(unsigned char offset, unsigned short val) {
        vx86_NBSB_Write16(3, offset, val);
    }
    DMP_INLINE(void) sb1_Write8(unsigned char offset, unsigned char val) {
        vx86_NBSB_Write8(3, offset, val);
    }
    DMP_INLINE(unsigned long) sb1_Read(unsigned char offset) {
        return vx86_NBSB_Read(3, offset);
    }
    DMP_INLINE(unsigned short) sb1_Read16(unsigned char offset) {
        return vx86_NBSB_Read16(3, offset);
    }
    DMP_INLINE(unsigned char) sb1_Read8(unsigned char offset) {
        return vx86_NBSB_Read8(3, offset);
    }
#endif
/*--------------------------  end. Inline Functions  -------------------------*/

#endif

