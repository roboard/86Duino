/*******************************************************************************

  io.cpp - DM&P Vortex86 Base I/O Library
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


#define __IO_LIB
////////////////////////////////////////////////////////////////////////////////
//
//    I/O library for DM&P Vortex86 CPUs
//    ----------------------------------
//    note that most of functions in this lib assume no paging issue when 
//    using them in ISR; so to use this lib in ISR in DJGPP, it is suggested 
//    to employ PMODE/DJ or CWSDPR0 or HDPMI instead of CWSDPMI.
////////////////////////////////////////////////////////////////////////////////



#include <dmpcfg.h>

/*
#if defined(DMP_WIN32_MSVC)
    #define USE_WINIO3       // use Yariv Kaplan's WinIO library 3.0 (allowing MMIO, http://www.internals.com/)
    //#define USE_WINIO2     // use Yariv Kaplan's WinIO library 2.0 (has bug on MMIO, http://www.internals.com/)
    //#define USE_PCIDEBUG   // use Kasiwano's PciDebug library      (http://www.otto.to/~kasiwano/)
    //#define USE_PHYMEM     // use cyb70289's PhyMem library        (http://www.codeproject.com/KB/system/phymem.aspx)
#endif
*/

#if defined     DMP_DOS_DJGPP
    #include <stdlib.h>

    #include <go32.h>
    #include <dpmi.h>

    #include <sys/farptr.h>
    #include <pc.h>
    #include <dos.h>
#elif defined   DMP_DOS_WATCOM
    #include <stdlib.h>
    #include <string.h>

    #include <conio.h>
    #include <i86.h>
#elif defined   DMP_DOS_BC
    #include <stdlib.h>

    #include <conio.h>
    #include <dos.h>
#elif defined(USE_WINIO2) || defined(USE_WINIO3)
    #include <windows.h>
    #include <winio.h>
#elif defined   USE_PCIDEBUG
	#include <windows.h>
    #include <pcifunc.h>
#elif defined   USE_PHYMEM
	#include <windows.h>
    #include <pmdll.h>
#elif defined   DMP_LINUX
    #include <stdlib.h>
    #include <unistd.h>
    #include <pthread.h>

    #include <sys/io.h>
    #include <sys/mman.h>     // for mmap()
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <errno.h>

    //#include <sys/times.h>  // for times()
    #include <time.h>         // for clock_gettime()
#endif

#include "io.h"
#include "io_atom.h"

#ifndef DMP_DOS_DJGPP
    #define DPMI_END_OF_LOCKED_FUNC(fname)
    #define DPMI_END_OF_LOCKED_STATIC_FUNC(fname)
#endif



////////////////////////////////////////////////////////////////////////////////
////////////////////////////   Internal Functions   ////////////////////////////
////////////////////////////////////////////////////////////////////////////////


/* =============================================
 *   Error Logging Functions
 * =============================================
 */
#include "err.h"


/* =============================================
 *   DPMI Interface for Watcom C/C++
 * =============================================
 */
#include "io_wat.h"


/* =============================================
 *   Recursive IRQ Disable/Enable Functions
 *   --------------------------------------
 *   these functions can work in both user-context and interrupt-context without 
 *   taking care the shared counter: irq_cscnt.
 * =============================================
 */
ATOMIC_INT_t irq_cscnt = ATOMIC_INT_INIT(0);
#include "io_irq.h"


/* =============================================
 *   Primitive I/O Access
 * =============================================
 */
#include "io_prim.h"





////////////////////////////////////////////////////////////////////////////////
/////////////////////////////   Public Functions   /////////////////////////////
////////////////////////////////////////////////////////////////////////////////


/* =============================================
 *   DPMI Functions
 * =============================================
 */
#include "io_dpmi.h"



/**********************  IRQ & i8259-PIC Util Functions  **********************/

DMPAPI(void) io_SetCSCNT(int val) {
    _io_SetCSCNT(val);
} DPMI_END_OF_LOCKED_FUNC(io_SetCSCNT)

DMPAPI(int) io_GetCSCNT(void) {
    return _io_GetCSCNT();
} DPMI_END_OF_LOCKED_FUNC(io_GetCSCNT)

#if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)

    DMPAPI(void) io_DisableINT(void) {
        _io_DisableINT();
    } DPMI_END_OF_LOCKED_FUNC(io_DisableINT)

    DMPAPI(void) io_RestoreINT(void) {
        _io_RestoreINT();
    } DPMI_END_OF_LOCKED_FUNC(io_RestoreINT)

    DMPAPI(void) io_EnableINT(void) {
        _io_EnableINT();
    } DPMI_END_OF_LOCKED_FUNC(io_EnableINT)

#else
    DMPAPI(void) io_DisableINT(void) {
        return;
    }
    DMPAPI(void) io_RestoreINT(void) {
        return;
    }
    DMPAPI(void) io_EnableINT(void) {
        return;
    }
#endif


/* =============================================
 *   i8259-PIC Util Functions
 * =============================================
 */
#include "io_pic.h"


static bool init_irqsystem(void) {
    #if defined(DMP_DOS_DJGPP)
        DPMI_LOCK_VAR(irq_cscnt);
        
        DPMI_LOCK_FUNC(io_SetCSCNT);
        DPMI_LOCK_FUNC(io_GetCSCNT);
        DPMI_LOCK_FUNC(io_DisableINT);
        DPMI_LOCK_FUNC(io_RestoreINT);
        DPMI_LOCK_FUNC(io_EnableINT);

        DPMI_LOCK_FUNC(i8259_outb);
        DPMI_LOCK_FUNC(i8259_inb);
        DPMI_LOCK_FUNC(i8259_DisableIRQ);
        DPMI_LOCK_FUNC(i8259_EnableIRQ);
        DPMI_LOCK_FUNC(i8259_IsIRQEnabled);
        DPMI_LOCK_FUNC(i8259_SendEOI);
        DPMI_LOCK_FUNC(i8259_GetINTVEC);
    #endif

    return true;
}

static bool close_irqsystem(void) {
    return true;
}
/*-------------------  end. IRQ & i8259-PIC Util Functions  ------------------*/



/***************************  I/O Access Functions  ***************************/
#define IO_NUM_BASEPOOL     (32)

#define IO_USE_NULLIO       (-1)

typedef struct
{
    int iotype;

    #if defined     DMP_DOS_DJGPP
        int mmio_selector;  // segment selector for MMIO space
    #elif defined   DMP_LINUX
        unsigned long mmio_realaddr;
    #elif defined   USE_WINIO3
        tagPhysStruct mmio_info;
    #endif
    
    unsigned long addr;
    unsigned long size;

} IO_BASE_t;

static IO_BASE_t IO_basePool[IO_NUM_BASEPOOL];
static IO_BASE_t IO_defaultBase;

// lock functions for IO_basePool[...]
#if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)

    #define IO_POOL_LOCK()      _io_DisableINT()
    #define IO_POOL_UNLOCK()    _io_RestoreINT()
    #define IO_POOL_INITLOCK()  (true)
    #define IO_POOL_FREELOCK()  do {} while(0)

#elif defined(DMP_LINUX)

    static pthread_mutex_t IO_basePoolMutex;
    #define IO_POOL_LOCK()      pthread_mutex_lock(&IO_basePoolMutex)
    #define IO_POOL_UNLOCK()    pthread_mutex_unlock(&IO_basePoolMutex)
    #define IO_POOL_FREELOCK()  pthread_mutex_destroy(&IO_basePoolMutex)

    __dmp_inline(bool) IO_POOL_INITLOCK() {
        if (pthread_mutex_init(&IO_basePoolMutex, NULL) != 0) return false; else return true;
    }

#elif defined(DMP_WINDOWS)

    static CRITICAL_SECTION IO_basePoolMutex;
    #define IO_POOL_LOCK()      EnterCriticalSection(&IO_basePoolMutex)
    #define IO_POOL_UNLOCK()    LeaveCriticalSection(&IO_basePoolMutex)
    #define IO_POOL_FREELOCK()  DeleteCriticalSection(&IO_basePoolMutex)

    __dmp_inline(bool) IO_POOL_INITLOCK() {
        InitializeCriticalSection(&IO_basePoolMutex);
        return true;
    }

    /*
    static HANDLE IO_basePoolMutex;
    #define IO_POOL_LOCK()      do { if (WaitForSingleObject(IO_basePoolMutex, INFINITE) != WAIT_OBJECT_0) err_print("%s: fail to lock IO_basePool!\n", __FUNCTION__); } while(0)
    #define IO_POOL_UNLOCK()    do { if (!ReleaseMutex(IO_basePoolMutex)) err_print("%s: fail to unlock IO_basePool!\n", __FUNCTION__); } while(0)
    #define IO_POOL_FREELOCK()  CloseHandle(IO_basePoolMutex)

    __dmp_inline(bool) IO_POOL_INITLOCK() {
        if ((IO_basePoolMutex = CreateMutex(NULL, FALSE,  NULL)) == NULL) return false; else return true;
    }
    */
#endif

#ifdef DMP_LINUX
    static int IO_mmioFD = -1;
#endif

static bool init_iosystem(void) {
    int i;

    for (i=0; i<IO_NUM_BASEPOOL; i++)
        IO_basePool[i].iotype = IO_USE_NULLIO;

    IO_defaultBase.iotype = IO_USE_PORTIO;
    IO_defaultBase.addr   = 0L;
    IO_defaultBase.size   = 0x10000L;

    if (IO_POOL_INITLOCK() == false)
    {
         err_print("%s: fail to initialize the lock of IO_basePool!\n", __FUNCTION__);
         return false;
    }

    #if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)
        return true;
    #elif defined(USE_WINIO2) || defined(USE_WINIO3)
        if (!InitializeWinIo())
        {
            err_print("%s: fail to initialize WinIO!\n", __FUNCTION__);
            return false;
        }
        return true;
    #elif defined(USE_PCIDEBUG)
        if (getdllstatus() != DLLSTATUS_NOERROR)
        {
            err_print("%s: fail to initialize PCIDEBUG!\n", __FUNCTION__);
            return false;
        }
        return true;
    #elif defined(USE_PHYMEM)
        if (LoadPhyMemDriver() == FALSE)
        {
            err_print("%s: fail to initialize PHYMEM!\n", __FUNCTION__);
            return false;
        }
        return true;
    #elif defined(DMP_LINUX)
        if (iopl(3) != 0)
		{
            err_print("%s: fail to set iopl(level=3) (%d)!\n", __FUNCTION__, errno);
            return false;
        }
        
        if ((IO_mmioFD = open("/dev/mem", O_RDWR | O_SYNC)) == -1)
            err_print("%s: fail to open /dev/mem (%d)!\n", __FUNCTION__, errno);

        return true;
    #endif
}

static bool close_iosystem(void) {
    IO_POOL_FREELOCK();

    #if defined(USE_WINIO2) || defined(USE_WINIO3)
        ShutdownWinIo();
        return true;
    #elif defined(USE_PHYMEM)
        UnloadPhyMemDriver();
        return true;
    #elif defined(DMP_LINUX)
        if (IO_mmioFD != -1) close(IO_mmioFD);
    #else
        return true;
    #endif
}

static IO_BASE_t* get_io_base(void) {
    IO_BASE_t* basePool = IO_basePool;
    IO_BASE_t* base = NULL;
    int i;

    IO_POOL_LOCK();  // need to lock in multi-tasking cases
    {
        for (i=0; i<IO_NUM_BASEPOOL; i++)
            if (basePool[i].iotype == IO_USE_NULLIO)
            {
                base = &basePool[i];
                base->iotype = IO_USE_PORTIO;
                break;
            }
    }
    IO_POOL_UNLOCK();

    return base;
}

static void free_io_base(IO_BASE_t* base) {
    IO_POOL_LOCK();  // need to lock in multi-tasking cases
    {
        if (base != NULL) base->iotype = IO_USE_NULLIO;
    }
    IO_POOL_UNLOCK();
}


/* =============================================
 *   Primitive MMIO Access
 * =============================================
 */
#include "io_mmio.h"


DMPAPI(void*) io_Alloc(int io_type, unsigned long io_phyaddr, unsigned long io_size) {
	IO_BASE_t* base;
    
    if ((base = get_io_base()) == NULL)
    {
        err_print("%s: no free I/O handle!\n", __FUNCTION__);
        return NULL;
    }
	base->iotype = io_type;
	base->addr   = io_phyaddr;
	base->size   = io_size;
    
    switch (base->iotype)
    {
        case IO_USE_PORTIO:
            if ((base->addr > 0xffffL) || ((0xffffL - base->addr) < (base->size - 1L)))
            {
                err_print("%s: infeasible port-I/O region!\n", __FUNCTION__);
                goto FAIL_IO_ALLOC;
            }
            break;
        case IO_USE_MMIO:
            if ((0xffffffffL - base->addr) < (base->size - 1L))
            {
                err_print("%s: infeasible MMIO region!\n", __FUNCTION__);
                goto FAIL_IO_ALLOC;
            }

            #if defined     DMP_DOS_DJGPP
                if ((base->addr = dpmi_LinMapAlloc(base->addr, base->size)) != 0L) //&&
                if ((base->mmio_selector = dpmi_SelAlloc(base->addr, base->size)) < 0)
                    dpmi_LinMapFree(base->addr);

                if ((base->addr == 0L) || (base->mmio_selector < 0))
                {
                    err_print("%s: fail to map MMIO region!\n", __FUNCTION__);
                    goto FAIL_IO_ALLOC;
                }
            #elif defined   DMP_LINUX
                if (IO_mmioFD != -1)
                {
                    unsigned long offset_in_page = base->addr % sysconf(_SC_PAGE_SIZE);
                    base->addr = base->addr - offset_in_page;  // ensure the base address is page-aligned
                    base->size = base->size + offset_in_page;
                    base->mmio_realaddr = (unsigned long)mmap(NULL, base->size, PROT_READ | PROT_WRITE, MAP_SHARED, IO_mmioFD, base->addr);
                    base->addr = ((void*)(base->mmio_realaddr) == (void *)-1)? base->mmio_realaddr : base->mmio_realaddr + offset_in_page;
                }

                if ((IO_mmioFD == -1) || ((void*)(base->addr) == (void *)-1))
                {
                    err_print("%s: fail to map MMIO region (%d)!\n", __FUNCTION__, errno);
                    goto FAIL_IO_ALLOC;
                }
            #elif defined   USE_WINIO3
                base->mmio_info.pvPhysAddress = (DWORD64)(DWORD32)(base->addr);
                base->mmio_info.dwPhysMemSizeInBytes = (DWORD64)(DWORD32)(base->size);
                
                base->addr = (unsigned long)MapPhysToLin(base->mmio_info);
                if ((unsigned char*)(base->addr) == NULL)
                {
                    err_print("%s: fail to map MMIO region!\n", __FUNCTION__);
                    goto FAIL_IO_ALLOC;
                }
            #elif defined   USE_PHYMEM
                base->addr = (unsigned long)MapPhyMem(base->addr, base->size);
                if ((unsigned char*)(base->addr) == NULL)
                {
                    err_print("%s: fail to map MMIO region!\n", __FUNCTION__);
                    goto FAIL_IO_ALLOC;
                }
            #elif defined(DMP_DOS_WATCOM) || defined(USE_PCIDEBUG)
                // do nothing ...
            #else
                err_print("%s: MMIO is not supported!\n", __FUNCTION__);
                goto FAIL_IO_ALLOC;
            #endif

            break;
        default:
            err_print("%s: unknown io_type!\n", __FUNCTION__);
            goto FAIL_IO_ALLOC;
    }
    return (void*)base;

FAIL_IO_ALLOC:

    free_io_base(base);
    return NULL;
}

DMPAPI(bool) io_Free(void* handle) {
    bool result = true;
	IO_BASE_t* base;

    if ((base = (IO_BASE_t*)handle) == NULL) return false;

    switch (base->iotype)
    {
        case IO_USE_PORTIO:
            // do nothing ...
            break;
        case IO_USE_MMIO:
            #if defined     DMP_DOS_DJGPP
                dpmi_LinMapFree(base->addr);
                dpmi_SelFree(base->mmio_selector);
            #elif defined   DMP_LINUX
                if ((IO_mmioFD != -1) && ((void*)base->addr != (void *)-1)) //&&
                if (munmap((void*)(base->mmio_realaddr), base->size) == -1)
                {
                    err_print("%s: fail to free MMIO region (%d)!\n", __FUNCTION__, errno);
                    result = false;
                }
            #elif defined   USE_WINIO3
                UnmapPhysicalMemory(base->mmio_info);
            #elif defined   USE_PHYMEM
                UnmapPhyMem((void*)base->addr, base->size);
            #endif
            break;
        default:
            err_print("%s: unknown io-type!\n", __FUNCTION__);
            result = false;
            break;
    }
    free_io_base(base);

    return result;
}


DMPAPI(void) io_Out32(void* handle, unsigned long offset, unsigned long val) {
    IO_BASE_t* base = (handle == NULL)? &IO_defaultBase : (IO_BASE_t*)handle;

    switch (base->iotype)
    {
        case IO_USE_PORTIO:
            portio_outpdw((unsigned short)(base->addr + offset), val);
            break;
        case IO_USE_MMIO:
            mmio_outpdw(base, offset, val);
            break;
    }
}

DMPAPI(unsigned long) io_In32(void* handle, unsigned long offset) {
    IO_BASE_t* base = (handle == NULL)? &IO_defaultBase : (IO_BASE_t*)handle;

    switch (base->iotype)
    {
        case IO_USE_PORTIO:
            return portio_inpdw((unsigned short)(base->addr + offset));
        case IO_USE_MMIO:
            return mmio_inpdw(base, offset);
    }
    return 0L;
}

DMPAPI(void) io_Out16(void* handle, unsigned long offset, unsigned short val) {
    IO_BASE_t* base = (handle == NULL)? &IO_defaultBase : (IO_BASE_t*)handle;

    switch (base->iotype)
    {
        case IO_USE_PORTIO:
            portio_outpw((unsigned short)(base->addr + offset), val);
            break;
        case IO_USE_MMIO:
            mmio_outpw(base, offset, val);
            break;
    }
}

DMPAPI(unsigned short) io_In16(void* handle, unsigned long offset) {
    IO_BASE_t* base = (handle == NULL)? &IO_defaultBase : (IO_BASE_t*)handle;

    switch (base->iotype)
    {
        case IO_USE_PORTIO:
            return portio_inpw((unsigned short)(base->addr + offset));
        case IO_USE_MMIO:
            return mmio_inpw(base, offset);
    }
    return 0;
}

DMPAPI(void) io_Out8(void* handle, unsigned long offset, unsigned char val) {
    IO_BASE_t* base = (handle == NULL)? &IO_defaultBase : (IO_BASE_t*)handle;

    switch (base->iotype)
    {
        case IO_USE_PORTIO:
            portio_outpb((unsigned short)(base->addr + offset), val);
            break;
        case IO_USE_MMIO:
            mmio_outpb(base, offset, val);
            break;
    }
}

DMPAPI(unsigned char) io_In8(void* handle, unsigned long offset) {
    IO_BASE_t* base = (handle == NULL)? &IO_defaultBase : (IO_BASE_t*)handle;

    switch (base->iotype)
    {
        case IO_USE_PORTIO:
            return portio_inpb((unsigned short)(base->addr + offset));
        case IO_USE_MMIO:
            return mmio_inpb(base, offset);
    }
    return 0;
}


// we need the following functions for slow CPUs such as Vortex86EX :p
DMPAPI(void) io_Out32M(void* handle, unsigned long offset, unsigned long val) {
    mmio_outpdw((IO_BASE_t*)handle, offset, val);
}
DMPAPI(unsigned long) io_In32M(void* handle, unsigned long offset) {
    return mmio_inpdw((IO_BASE_t*)handle, offset);
}
DMPAPI(void) io_Out16M(void* handle, unsigned long offset, unsigned short val) {
    mmio_outpw((IO_BASE_t*)handle, offset, val);
}
DMPAPI(unsigned short) io_In16M(void* handle, unsigned long offset) {
    return mmio_inpw((IO_BASE_t*)handle, offset);
}
DMPAPI(void) io_Out8M(void* handle, unsigned long offset, unsigned char val) {
    mmio_outpb((IO_BASE_t*)handle, offset, val);
}
DMPAPI(unsigned char) io_In8M(void* handle, unsigned long offset) {
    return mmio_inpb((IO_BASE_t*)handle, offset);
}

DMPAPI(void) io_Out32P(void* handle, unsigned long offset, unsigned long val) {
    portio_outpdw((unsigned short)(((IO_BASE_t*)handle)->addr + offset), val);
}
DMPAPI(unsigned long) io_In32P(void* handle, unsigned long offset) {
    return portio_inpdw((unsigned short)(((IO_BASE_t*)handle)->addr + offset));
}
DMPAPI(void) io_Out16P(void* handle, unsigned long offset, unsigned short val) {
    portio_outpw((unsigned short)(((IO_BASE_t*)handle)->addr + offset), val);
}
DMPAPI(unsigned short) io_In16P(void* handle, unsigned long offset) {
    return portio_inpw((unsigned short)(((IO_BASE_t*)handle)->addr + offset));
}
DMPAPI(void) io_Out8P(void* handle, unsigned long offset, unsigned char val) {
    portio_outpb((unsigned short)(((IO_BASE_t*)handle)->addr + offset), val);
}
DMPAPI(unsigned char) io_In8P(void* handle, unsigned long offset) {
    return portio_inpb((unsigned short)(((IO_BASE_t*)handle)->addr + offset));
}
/*------------------------  end. I/O Access Functions  -----------------------*/



/**************************  PCI-CFG Access Functions  ************************/
#define PCI_GET_CF8(bus, dev, fun)  (0x80000000UL + ((unsigned long)(bus) << 16) + ((unsigned long)(dev) << 11) + ((unsigned long)(fun) << 8))

#define PCI_NUM_BASEPOOL    (32)

typedef struct
{
    unsigned char bus;
    unsigned char dev;
    unsigned char fun;
    
    unsigned long addr;

} PCI_BASE_t;

static PCI_BASE_t PCI_basePool[PCI_NUM_BASEPOOL];

// lock functions for PCI_basePool[...]
#if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)

    #define PCI_POOL_LOCK()      _io_DisableINT()
    #define PCI_POOL_UNLOCK()    _io_RestoreINT()
    #define PCI_POOL_INITLOCK()  (true)
    #define PCI_POOL_FREELOCK()  do {} while(0)

#elif defined(DMP_LINUX)

    static pthread_mutex_t PCI_basePoolMutex;
    #define PCI_POOL_LOCK()      pthread_mutex_lock(&PCI_basePoolMutex)
    #define PCI_POOL_UNLOCK()    pthread_mutex_unlock(&PCI_basePoolMutex)
    #define PCI_POOL_FREELOCK()  pthread_mutex_destroy(&PCI_basePoolMutex)

    __dmp_inline(bool) PCI_POOL_INITLOCK() {
        if (pthread_mutex_init(&PCI_basePoolMutex, NULL) != 0) return false; else return true;
    }

#elif defined(DMP_WINDOWS)

    static CRITICAL_SECTION PCI_basePoolMutex;
    #define PCI_POOL_LOCK()      EnterCriticalSection(&PCI_basePoolMutex)
    #define PCI_POOL_UNLOCK()    LeaveCriticalSection(&PCI_basePoolMutex)
    #define PCI_POOL_FREELOCK()  DeleteCriticalSection(&PCI_basePoolMutex)

    __dmp_inline(bool) PCI_POOL_INITLOCK() {
        InitializeCriticalSection(&PCI_basePoolMutex);
        return true;
    }

#endif

static bool init_pcisystem(void) {
    int i;

    for (i=0; i<PCI_NUM_BASEPOOL; i++)
        PCI_basePool[i].addr = 0xffffffffL;

    if (PCI_POOL_INITLOCK() == false)
    {
         err_print("%s: fail to initialize the lock of PCI_basePool!\n", __FUNCTION__);
         return false;
    }

    return true;
}

static bool close_pcisystem(void) {
    PCI_POOL_FREELOCK();
    return true;
}

static PCI_BASE_t* get_pci_base(void) {
    PCI_BASE_t* basePool = PCI_basePool;
    PCI_BASE_t* base = NULL;
    int i;

    PCI_POOL_LOCK();  // need to lock in multi-tasking cases
    {
        for (i=0; i<PCI_NUM_BASEPOOL; i++)
            if (basePool[i].addr == 0xffffffffL)
            {
                base = &basePool[i];
                base->addr = 0L;
                break;
            }
    }
    PCI_POOL_UNLOCK();

    return base;
}

static void free_pci_base(PCI_BASE_t* base) {
    PCI_POOL_LOCK();  // need to lock in multi-tasking cases
    {
        if (base != NULL) base->addr = 0xffffffffL;
    }
    PCI_POOL_UNLOCK();
}

static void* alloc_pci(unsigned char bus, unsigned char dev, unsigned char fun) {
    PCI_BASE_t* base;

    if ((base = get_pci_base()) == NULL)
        err_print("%s: no free pci handle!\n", __FUNCTION__);
    else
    {
        base->bus  = bus;
        base->dev  = dev;
        base->fun  = fun;
        base->addr = PCI_GET_CF8(bus, dev, fun);
    }

    return (void*)base;
}


DMPAPI(void*) pci_Alloc(unsigned char bus, unsigned char dev, unsigned char fun) {
    // just ignore wrong dev & fun :p
    dev = dev & 0x1f; 
    fun = fun & 0x07;

    // check VID & DID
    if (pci_indw(PCI_GET_CF8(bus, dev, fun)) == 0xffffffffUL) //&&
    if (fun == 0)
    {
        err_print("%s: invalid PCI device!\n", __FUNCTION__);
        return NULL;
    }

    return alloc_pci(bus, dev, fun);
}

DMPAPI(void*) pci_Find(unsigned short vid, unsigned short did) {  // don't use this function in the init of io system
    unsigned long pciid = ((unsigned long)did << 16) + (unsigned long)vid;
    unsigned long tmpid, pciaddr;
    int bus, dev, fun;

    if (pciid == 0xffffffffUL) return NULL;

    for (bus=0; bus<256; bus++)
    for (dev=0; dev<32;  dev++)
    for (fun=0; fun<8;   fun++)
    {
        pciaddr = PCI_GET_CF8(bus, dev, fun);

        tmpid = pci_indw(pciaddr);
        if (tmpid == pciid) return pci_Alloc((unsigned char)bus, (unsigned char)dev, (unsigned char)fun);
        if (fun == 0)
        {
            if (tmpid == 0xffffffffUL)  // invalid PCI device
            {
                // NOTE: shouldn't do this optimization for Vortex86DX2/DX3/EX's internal PCI devices (where fun0 may be shutdown but fun1 is still enabled)
                if ((vx86_CpuID() == CPU_VORTEX86DX2) || (vx86_CpuID() == CPU_VORTEX86DX3) || (vx86_CpuID() == CPU_VORTEX86EX)) { /* do nothing ... */ } else break;
            }
            if ((pci_inb(pciaddr + 0x0eL) & 0x80) == 0) break;  // single-function PCI device
        }
    } // end for (fun=...

    return NULL;
}

DMPAPI(bool) pci_Free(void* handle) {
    if (handle == NULL) return false;

    free_pci_base((PCI_BASE_t*)handle);
    return true;
}


DMPAPI(void) pci_Out32(void* handle, unsigned char offset, unsigned long val) {
    PCI_BASE_t* base = (PCI_BASE_t*)handle;
    //pci_outdw(base->addr + (unsigned long)(offset & 0xfc), val);
    pci_outdw(base->addr + (unsigned long)offset, val);  // assume offset % 4 == 0
}

DMPAPI(unsigned long) pci_In32(void* handle, unsigned char offset) {
    PCI_BASE_t* base = (PCI_BASE_t*)handle;
    //return pci_indw(base->addr + (unsigned long)(offset & 0xfc));
    return pci_indw(base->addr + (unsigned long)offset);  // assume offset % 4 == 0
}

DMPAPI(void) pci_Out16(void* handle, unsigned char offset, unsigned short val) {
    PCI_BASE_t* base = (PCI_BASE_t*)handle;
    //pci_outw(base->addr + (unsigned long)(offset & 0xfe), val);
    pci_outw(base->addr + (unsigned long)offset, val);  // assume offset % 4 < 3
}

DMPAPI(unsigned short) pci_In16(void* handle, unsigned char offset) {
    PCI_BASE_t* base = (PCI_BASE_t*)handle;
    //return pci_inw(base->addr + (unsigned long)(offset & 0xfe));
    return pci_inw(base->addr + (unsigned long)offset);  // assume offset % 4 < 3
}

DMPAPI(void) pci_Out8(void* handle, unsigned char offset, unsigned char val) {
    PCI_BASE_t* base = (PCI_BASE_t*)handle;
    pci_outb(base->addr + (unsigned long)offset, val);
}

DMPAPI(unsigned char) pci_In8(void* handle, unsigned char offset) {
    PCI_BASE_t* base = (PCI_BASE_t*)handle;
    return pci_inb(base->addr + (unsigned long)offset);
}
/*----------------------  end. PCI-CFG Access Functions  ---------------------*/



/**************************  Vortex86 Util Functions  *************************/
#define VX86_NB     (0)
#define VX86_NB1    (1)
#define VX86_SB     (2)
#define VX86_SB1    (3)
#define VX86_IDE    (4)
static void* VX86_pciDev[VX86_IDE + 1] = {NULL};

static int VX86_cpuID = CPU_UNSUPPORTED;
static unsigned long VX86_cpuCLK  = 0L;
static unsigned long VX86_dramCLK = 0L;

static unsigned long VX86_cpuCLK_SX[] =   { 0L, 200L, 233L, 266L, 300L, 333L, 366L, 400L };         // for Vortex86SX
static unsigned long VX86_cpuCLK_DX_A[] = { 0L,    0L,    500L,  600L,  700L,  800L,  900L,  933L,  // for Vortex86DX ver. A/B/C
                                            966L,  1000L, 1033L, 1066L, 1066L, 1066L, 1100L, 1133L,
                                            1166L, 1200L, 1233L, 1266L, 1300L, 1333L, 1366L, 1400L,
                                            1433L, 1466L, 1500L };
static unsigned long VX86_cpuCLK_DX_D[] = { 0L,    0L,    500L,  600L,  700L,  800L,  833L,  866L,  // for Vortex86DX ver. D and Vortex86MX
                                            900L,  933L,  966L,  1000L, 1033L, 1066L, 1100L, 1133L,
                                            1166L, 1200L, 1233L, 1266L, 1300L, 1333L, 1366L, 1400L,
                                            1433L, 1466L, 1500L };
static unsigned long VX86_cpuCLK_MX[] =   { 500L,  600L,  700L,  833L,  866L,  900L,  933L,  966L,  // for Vortex86MX+/DX2
                                            1000L, 1033L, 1066L, 1100L, 1133L, 1166L, 1200L, 800L };
static unsigned long VX86_dramCLK_DX[] =  { 166L, 200L, 233L, 266L, 300L, 333L, 366L, 400L,         // for Vortex86DX/MX/MX+/DX2
                                            416L, 433L, 450L, 466L, 483L, 500L, 516L, 533L };

static int get_cpuid(void) {
    unsigned long id = nb_Read(0x90);

    switch (id)
	{
		case 0x31504D44L: //"DMP1"
            return CPU_VORTEX86SX;
		case 0x32504D44L: //"DMP2"
		{
            unsigned char nbrv = nb_Read8(0x08);
            unsigned char sbrv = sb_Read8(0x08);
            unsigned long ide  = pci_In32(VX86_pciDev[VX86_IDE], 0x00);

		    if ((nbrv == 1) && (sbrv == 1) && (ide == 0x101017f3L)) return CPU_VORTEX86DX_A;  // Vortex86DX ver. A
		    if ((nbrv == 1) && (sbrv == 2) && (ide != 0x101017f3L)) return CPU_VORTEX86DX_C;  // Vortex86DX ver. C (PBA/PBB)
		    if ((nbrv == 2) && (sbrv == 2) && (ide != 0x101017f3L)) return CPU_VORTEX86DX_D;  // Vortex86DX ver. D

            return CPU_VORTEX86DX_UNKNOWN;
        }
		case 0x33504D44L: //"DMP3"
			return CPU_VORTEX86MX;
		case 0x35504D44L: //"DMP5"
			return CPU_VORTEX86MX_PLUS;
		case 0x34504D44L: //"DMP4"
			return CPU_VORTEX86DX2;
		case 0x36504D44L: //"DMP6"
			return CPU_VORTEX86DX3;
		case 0x37504D44L: //"DMP7"
			return CPU_VORTEX86EX;
	}

	return CPU_UNSUPPORTED;
}

static unsigned long get_cpufreq(void) {
    if (vx86_CpuID() == CPU_VORTEX86SX)
    {
        unsigned long strapreg = nb_Read(0x60);
        return VX86_cpuCLK_SX[(int)(strapreg & 0x7L)];
    }
    else
    if ((vx86_CpuID() == CPU_VORTEX86DX_A) || (vx86_CpuID() == CPU_VORTEX86DX_C) || (vx86_CpuID() == CPU_VORTEX86DX_UNKNOWN))
    {
        unsigned long strapreg = nb_Read(0x60);
        if ((strapreg & 0x1fL) == 1L) return VX86_dramCLK_DX[(int)((strapreg >> 8) & 0x07L)];
        return VX86_cpuCLK_DX_A[(int)(strapreg & 0x1fL)];
    }
    else
    if ((vx86_CpuID() == CPU_VORTEX86DX_D) || (vx86_CpuID() == CPU_VORTEX86MX))
    {
        unsigned long strapreg = nb_Read(0x60);
        if ((strapreg & 0x1fL) == 1L) return VX86_dramCLK_DX[(int)((strapreg >> 8) & 0x07L)];
        return VX86_cpuCLK_DX_D[(int)(strapreg & 0x1fL)];
    }
    else
    if ((vx86_CpuID() == CPU_VORTEX86MX_PLUS) || (vx86_CpuID() == CPU_VORTEX86DX2))
    {
        unsigned long strapreg = nb_Read(0x60);
        return VX86_cpuCLK_MX[(int)(strapreg & 0x0fL)];
    }
    else
    if (vx86_CpuID() == CPU_VORTEX86DX3)
    {
        unsigned long strapreg2 = nb_Read(0x64);
        unsigned long cms  = (strapreg2 >> 12) & 0x07L;
        unsigned long cns  = strapreg2 & 0xffL;
        int           crs  = (int)((strapreg2 >> 8) & 0x03L);

        //if ((cns == 0x90) && (cms == 0x03) && (crs == 0x00)) return 800L;  // special case for "Hardware Default"
        return (25L*cns) / (cms * (1L<<crs));
    }
    else
    if (vx86_CpuID() == CPU_VORTEX86EX)
    {
        unsigned long strapreg2 = nb_Read(0x64);
        unsigned long cdiv = (strapreg2 >> 12) & 0x03L;
        unsigned long cms  = (strapreg2 >> 8) & 0x03L;
        unsigned long cns  = strapreg2 & 0xffL;
        int           crs  = (int)((strapreg2 >> 10) & 0x03L);
        return (25L*cns) / (cms * (1L<<crs) * (cdiv+2L));
    }

    return 0L;
}

static unsigned long get_dramfreq(void) {
    if (vx86_CpuID() == CPU_VORTEX86SX)
    {
        unsigned long strapreg = nb_Read(0x60);
        if ((strapreg & 0x00000100L) == 0L) return 100L; else return 133L;
    }
    else
    if ((vx86_CpuID() == CPU_VORTEX86DX_A) || (vx86_CpuID() == CPU_VORTEX86DX_C) || (vx86_CpuID() == CPU_VORTEX86DX_UNKNOWN) || (vx86_CpuID() == CPU_VORTEX86DX_D) || (vx86_CpuID() == CPU_VORTEX86MX))
    {
        unsigned long strapreg = nb_Read(0x60);
        return VX86_dramCLK_DX[(int)((strapreg >> 8) & 0x07L)];
    }
    else
    if ((vx86_CpuID() == CPU_VORTEX86MX_PLUS) || (vx86_CpuID() == CPU_VORTEX86DX2))
    {
        unsigned long strapreg = nb_Read(0x60);
        return VX86_dramCLK_DX[(int)((strapreg >> 8) & 0x0fL)];
    }
    else
    if (vx86_CpuID() == CPU_VORTEX86DX3)
    {
        unsigned long strapreg2 = nb_Read(0x64);
        unsigned long dms  = (strapreg2 >> 28) & 0x07L;
        unsigned long dns  = (strapreg2 >> 16) & 0xffL;
        int           drs  = (int)((strapreg2 >> 24) & 0x03L);
        return (25L*dns) / (dms * (1L<<drs));
    }
    else
    if (vx86_CpuID() == CPU_VORTEX86EX)
    {
        unsigned long strapreg2 = nb_Read(0x64);
        unsigned long ddiv = (strapreg2 >> 14) & 0x01L;
        unsigned long cms  = (strapreg2 >> 8) & 0x03L;
        unsigned long cns  = strapreg2 & 0xffL;
        int           crs  = (int)((strapreg2 >> 10) & 0x03L);
        return (25L*cns) / (cms * (1L<<crs) * ((ddiv+1L)*2L));
    }

    return 0L;
}

static bool init_vx86(void) {
    int i;
    
    for (i=0; i<(int)(sizeof(VX86_pciDev)/sizeof(VX86_pciDev[0])); i++) VX86_pciDev[i] = NULL;

    // get North Bridge fun0 & South Bridge fun0 & IDE PCI-CFG
    if ((VX86_pciDev[VX86_NB]  = pci_Alloc(0x00, 0x00, 0x00)) == NULL) goto FAIL_INITVX86;
    if ((VX86_pciDev[VX86_SB]  = pci_Alloc(0x00, 0x07, 0x00)) == NULL) goto FAIL_INITVX86;
    if ((VX86_pciDev[VX86_IDE] = pci_Alloc(0x00, 0x0c, 0x00)) == NULL) goto FAIL_INITVX86;
    
    // now we are allowed to call get_cpuid()
    if ((VX86_cpuID = get_cpuid()) == CPU_UNSUPPORTED) goto FAIL_INITVX86;

    if (vx86_CpuID() != CPU_VORTEX86SX)
    {
        // North Bridge fun1 exists (note NB fun1 isn't a normal PCI device -- its vid & did are 0xffff)
        if ((VX86_pciDev[VX86_NB1]  = pci_Alloc(0x00, 0x00, 0x01)) == NULL) goto FAIL_INITVX86;
    }
    if ((vx86_CpuID() == CPU_VORTEX86DX2) || (vx86_CpuID() == CPU_VORTEX86DX3) || (vx86_CpuID() == CPU_VORTEX86EX))
    {
        // South Bridge fun1 exists (note SB fun1 isn't a normal PCI device -- its vid & did are 0xffff)
        if ((VX86_pciDev[VX86_SB1]  = pci_Alloc(0x00, 0x07, 0x01)) == NULL) goto FAIL_INITVX86;
    }

    // now we are allowed to call get_cpufreq() & get_dramfreq()
    VX86_cpuCLK  = get_cpufreq();
    VX86_dramCLK = get_dramfreq();
    return true;

FAIL_INITVX86:
    for (i=0; i<(int)(sizeof(VX86_pciDev)/sizeof(VX86_pciDev[0])); i++)
    {
        pci_Free(VX86_pciDev[i]);
        VX86_pciDev[i] = NULL;
    }
    err_print("%s: fail to setup system PCI devices!\n", __FUNCTION__);
    return false;
}

static bool close_vx86(void) {
    int i;

    for (i=0; i<(int)(sizeof(VX86_pciDev)/sizeof(VX86_pciDev[0])); i++)
    {
        pci_Free(VX86_pciDev[i]);
        VX86_pciDev[i] = NULL;
    }

    VX86_cpuID = CPU_UNSUPPORTED;
    return true;
}


DMPAPI(int) vx86_CpuID(void) {
    return VX86_cpuID;
}


DMPAPI(unsigned long) vx86_CpuCLK(void) {
    return VX86_cpuCLK;
}
DMPAPI(unsigned long) vx86_DramCLK(void) {
    return VX86_dramCLK;
}


DMPAPI(void) vx86_NBSB_Write(int fun, unsigned char offset, unsigned long val) {
    if (VX86_pciDev[VX86_NB + fun] == NULL) return;
    pci_Out32(VX86_pciDev[VX86_NB + fun], offset, val);
}
DMPAPI(void) vx86_NBSB_Write16(int fun, unsigned char offset, unsigned short val) {
    if (VX86_pciDev[VX86_NB + fun] == NULL) return;
    pci_Out16(VX86_pciDev[VX86_NB + fun], offset, val);
}
DMPAPI(void) vx86_NBSB_Write8(int fun, unsigned char offset, unsigned char val) {
    if (VX86_pciDev[VX86_NB + fun] == NULL) return;
    pci_Out8(VX86_pciDev[VX86_NB + fun], offset, val);
}

DMPAPI(unsigned long) vx86_NBSB_Read(int fun, unsigned char offset) {
    if (VX86_pciDev[VX86_NB + fun] == NULL) return 0xffffffffL;
    return pci_In32(VX86_pciDev[VX86_NB + fun], offset);
}
DMPAPI(unsigned short) vx86_NBSB_Read16(int fun, unsigned char offset) {
    if (VX86_pciDev[VX86_NB + fun] == NULL) return 0xffff;
    return pci_In16(VX86_pciDev[VX86_NB + fun], offset);
}
DMPAPI(unsigned char) vx86_NBSB_Read8(int fun, unsigned char offset) {
    if (VX86_pciDev[VX86_NB + fun] == NULL) return 0xff;
    return pci_In8(VX86_pciDev[VX86_NB + fun], offset);
}
/*----------------------  End. Vortex86 Util Functions  ----------------------*/



/* ========================================
 *   CPU-Time Util Functions
 * ========================================
 */
#include "io_timer.h"



/* ========================================
 *   I/O LIB Setup Functions
 * ========================================
 */
static int IO_inUse = 0;

DMPAPI(bool) io_Init(void) {  // NOTE: this function isn't reentrant
    if (IO_inUse == 0)
    {
        io_SetCSCNT(0);  // would be used by init_xxxx() functions

        if (init_iosystem() == false)
        {
            err_print("%s: fail to init I/O access system!\n", __FUNCTION__);
            goto FAIL_IOINIT_IO;
        }
        if (init_pcisystem() == false)
        {
            err_print("%s: fail to init PCI-CFG access system!\n", __FUNCTION__);
            goto FAIL_IOINIT_PCI;
        }
        if (init_vx86() == false)
        {
            err_print("%s: fail to init Vortex86 util system!\n", __FUNCTION__);
            goto FAIL_IOINIT_VX86;
        }
        if (init_irqsystem() == false)
        {
            err_print("%s: fail to init IRQ util system!\n", __FUNCTION__);
            goto FAIL_IOINIT_IRQ;
        }

        TIMER_initTime = 0L;  // needed to allow the correct reset of TIMER_initTime
        TIMER_initTime = timer_NowTime();
    } // end if (IO_inUse ...

    IO_inUse++;
    return true;

FAIL_IOINIT_IRQ:
    close_vx86();
FAIL_IOINIT_VX86:
    close_pcisystem();
FAIL_IOINIT_PCI:
    close_iosystem();
FAIL_IOINIT_IO:
    return false;
}

DMPAPI(bool) io_Close(void) {  // NOTE: this function isn't reentrant
    bool result = true;

    if (IO_inUse == 1)
    {
        if (close_irqsystem() == false)
        {
            err_print("%s: fail to close IRQ util system!\n", __FUNCTION__);
            result = false;
        }
        if (close_vx86() == false)
        {
            err_print("%s: fail to close Vortex86 util system!\n", __FUNCTION__);
            result = false;
        }
        if (close_pcisystem() == false)
        {
            err_print("%s: fail to close PCI-CFG access system!\n", __FUNCTION__);
            result = false;
        }
        if (close_iosystem()  == false)
        {
            err_print("%s: fail to close I/O access system!\n", __FUNCTION__);
            result = false;
        }
    } // end if (IO_inUse ...

    if (IO_inUse > 0) IO_inUse--;
    return result;
}



/* ========================================
 *   OS Kernel Functions
 * ========================================
 */
DMPAPI(void*) ker_Malloc(size_t size) {
    #if defined(DMP_DOS_DJGPP) && !defined(DMP_DOS_DJGPP_NO_VIRMEM)
        size_t* p = (size_t*)malloc(size + sizeof(size_t));

        if (p == NULL) return NULL;
        if (dpmi_LockData((void*)p, (unsigned long)size) == true)
        {
            p[0] = size;
            p++;
        }
        else
        {
            free((void*)p);
            p = NULL;
        }

        return p;
    #else
        return malloc(size);
    #endif
}

DMPAPI(void) ker_Mfree(void* p) {
    #if defined(DMP_DOS_DJGPP) && !defined(DMP_DOS_DJGPP_NO_VIRMEM)
        if (p != NULL)
        {
            size_t* p1 = (size_t*)p - 1;

            dpmi_UnlockData((void*)p1, (unsigned long)p1[0]);
            free(p1);
        }
    #else
        if (p!= NULL) free((void*)p);
    #endif
}

