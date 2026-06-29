#define __IO_LIB
////////////////////////////////////////////////////////////////////////////////
//
//    I/O library for DM&P Vortex86 CPUs
//    ----------------------------------
//    note that all functions in this lib assume no paging issue when using 
//    them in ISR; so to use this lib in ISR in DJGPP, it is suggested to 
//    employ PMODE instead of CWSDPMI.
////////////////////////////////////////////////////////////////////////////////



#include "dmpcfg.h"

#if defined(DMP_WIN32_MSVC)  // currently used in io.cpp & rcservo.cpp
    #define USE_WINIO3       // use Yariv Kaplan's WinIO library 3.0 (allowing MMIO, http://www.internals.com/)
    //#define USE_WINIO2     // use Yariv Kaplan's WinIO library 2.0 (has bug on MMIO, http://www.internals.com/)
    //#define USE_PCIDEBUG   // use Kasiwano's PciDebug library      (http://www.otto.to/~kasiwano/)
    //#define USE_PHYMEM     // use cyb70289's PhyMem library        (http://www.codeproject.com/KB/system/phymem.aspx)
#endif

#if defined     DMP_DOS_DJGPP
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdarg.h>

    #include <go32.h>
    #include <dpmi.h>

    #include <sys/farptr.h>
    #include <pc.h>
    #include <dos.h>
#elif defined   DMP_DOS_WATCOM
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdarg.h>
    #include <string.h>

    #include <conio.h>
    #include <i86.h>
#elif defined   DMP_DOS_BC
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdarg.h>

    #include <conio.h>
    #include <dos.h>
#elif defined(USE_WINIO2) || defined(USE_WINIO3)
    #include <stdio.h>
    #include <windows.h>
    #include <winio.h>
#elif defined   USE_PCIDEBUG
    #include <stdio.h>
	#include <windows.h>
    #include <pcifunc.h>
#elif defined   USE_PHYMEM
    #include <stdio.h>
	#include <windows.h>
    #include <pmdll.h>
#elif defined   DMP_LINUX
    #include <sys/io.h>
#endif

#include "io.h"



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


/* =============================================
 *   i8259-PIC Util Functions
 * =============================================
 */
#include "io_pic.h"


/* =============================================
 *   Recursive IRQ Disable/Enable Functions
 *   --------------------------------------
 *   these functions can work in both user-context and interrupt-context without 
 *   taking care the shared counter: irq_cscnt.
 * =============================================
 */
#if defined(DMP_DOS_DJGPP)
    //#define DISABLE_INT()    disable()
    //#define ENABLE_INT()     enable()
    #define DISABLE_INT()    __asm__ __volatile__("cli" : : : "memory")
    #define ENABLE_INT()     __asm__ __volatile__("sti" : : : "memory")
#elif defined(DMP_DOS_BC)
    #define DISABLE_INT()    disable()
    #define ENABLE_INT()     enable()
#elif defined(DMP_DOS_WATCOM)
    #define DISABLE_INT()    _disable()
    #define ENABLE_INT()     _enable()
#elif defined(DMP_LINUX)
    #define DISABLE_INT()    __asm__ __volatile__("cli" : : : "memory")  // only for Vortex86SX/DX/MX+/DX2/EX
    #define ENABLE_INT()     __asm__ __volatile__("sti" : : : "memory")  // only for Vortex86SX/DX/MX+/DX2/EX
#else
    // platforms not supporting interrupt-enable/disable
    #define DISABLE_INT()
    #define ENABLE_INT()
#endif

// TODO: we just assume "int" operations are atomic on Vortex86SX/DX/MX+/DX2/EX, 
//       but this bad design should be modified in the future.
static int irq_cscnt = 0;

DMPAPI(void) io_SetCSCNT(int val) {  // well ... bad design too:(
    irq_cscnt = val;
}
#ifdef DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_FUNC(io_SetCSCNT)
#endif

DMPAPI(void) io_DisableINT(void) {
    if (irq_cscnt == 0)
    {
        DISABLE_INT();
    }
    // getting interrupts disabled, it is now safe to update irq_cscnt
    irq_cscnt++;
}
#ifdef DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_FUNC(io_DisableINT)
#endif

DMPAPI(void) io_RestoreINT(void) {
    if (irq_cscnt == 0) return;
    // having interrupts disabled, it is safe to update irq_cscnt
    if (--irq_cscnt == 0)
    {
        ENABLE_INT();
    }
}
#ifdef DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_FUNC(io_RestoreINT)
#endif

DMPAPI(void) io_EnableINT(void) {
    if (irq_cscnt == 0) return;
    // having interrupts disabled, it is safe to update irq_cscnt
    irq_cscnt = 0;
    ENABLE_INT();
}
#ifdef DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_FUNC(io_EnableINT)
#endif


static bool init_irqsystem(void) {
    #if defined(DMP_DOS_DJGPP)
        DPMI_LOCK_VAR(irq_cscnt);
        
        DPMI_LOCK_FUNC(io_SetCSCNT);
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



/***************************  I/O Access Functions  ***************************/
#define IO_NUM_BASEPOOL     (32)

#define IO_USE_NULLIO       (-1)

typedef struct
{
    int iotype;

    #if defined     DMP_DOS_DJGPP
        int mmio_selector;  // segment selector for MMIO space
    #elif defined   USE_WINIO3
        tagPhysStruct mmio_info;
    #endif
    
    unsigned long addr;
    unsigned long size;

} IO_BASE_t;

static IO_BASE_t IO_basePool[IO_NUM_BASEPOOL];
static IO_BASE_t IO_defaultBase;

static bool init_iosystem(void) {
    int i;

    for (i=0; i<IO_NUM_BASEPOOL; i++)
        IO_basePool[i].iotype = IO_USE_NULLIO;

    // TODO: IO_defaultBase shouldn't be used in WinXP or Linux Kernel Modes
    // #if defined(DMP_OS_KERNEL)
    //    IO_defaultBase.iotype = IO_USE_NULLIO;
    // #else
    IO_defaultBase.iotype = IO_USE_PORTIO;
    IO_defaultBase.addr   = 0L;
    IO_defaultBase.size   = 0x10000L;

    #if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)
        return true;
    #elif defined(USE_WINIO2) || defined(USE_WINIO3)
        if (!InitializeWinIo())
        {
            err_print((char*)"%s: fail to initialize WinIO!\n", __FUNCTION__);
            return false;
        }
        return true;
    #elif defined(USE_PCIDEBUG)
        if (getdllstatus() != DLLSTATUS_NOERROR)
        {
            err_print((char*)"%s: fail to initialize PCIDEBUG!\n", __FUNCTION__);
            return false;
        }
        return true;
    #elif defined(USE_PHYMEM)
        if (LoadPhyMemDriver() == FALSE)
        {
            err_print((char*)"%s: fail to initialize PHYMEM!\n", __FUNCTION__);
            return false;
        }
        return true;
    #elif defined(DMP_LINUX)
        if (iopl(3) != 0)
		{
            err_print((char*)"%s: fail to set iopl(3)!\n", __FUNCTION__);
            return false;
        }
        return true;
    #endif
}

static bool close_iosystem(void) {
    #if defined(USE_WINIO2) || defined(USE_WINIO3)
        ShutdownWinIo();
        return true;
    #elif defined(USE_PHYMEM)
        UnloadPhyMemDriver();
        return true;
    #else
        return true;
    #endif
}

static IO_BASE_t* get_io_base(void) {
    IO_BASE_t* basePool = IO_basePool;
    IO_BASE_t* base = NULL;
    int i;

    io_DisableINT();
    for (i=0; i<IO_NUM_BASEPOOL; i++)
        if (basePool[i].iotype == IO_USE_NULLIO)
        {
            base = &basePool[i];
            base->iotype = IO_USE_PORTIO;
            break;
        }
    io_RestoreINT();

    return base;
}

static void free_io_base(IO_BASE_t* base) {
    io_DisableINT();
    if (base != NULL) base->iotype = IO_USE_NULLIO;
    io_RestoreINT();
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
        err_print((char*)"%s: no free I/O handle!\n", __FUNCTION__);
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
                err_print((char*)"%s: infeasible port-I/O region!\n", __FUNCTION__);
                goto FAIL_IO_ALLOC;
            }
            
            // TODO: for WinXP or Linux Kernel Mode, alloc Port-I/O region here
            break;
        case IO_USE_MMIO:
            if ((0xffffffffL - base->addr) < (base->size - 1L))
            {
                err_print((char*)"%s: infeasible MMIO region!\n", __FUNCTION__);
                goto FAIL_IO_ALLOC;
            }

            #if defined     DMP_DOS_DJGPP
                if ((base->addr = dpmi_LinMapAlloc(base->addr, base->size)) != 0L) //&&
                if ((base->mmio_selector = dpmi_SelAlloc(base->addr, base->size)) < 0)
                    dpmi_LinMapFree(base->addr);

                if ((base->addr == 0L) || (base->mmio_selector < 0))
                {
                    err_print((char*)"%s: fail to map MMIO region!\n", __FUNCTION__);
                    goto FAIL_IO_ALLOC;
                }
            #elif defined   USE_WINIO3
                base->mmio_info.pvPhysAddress = (DWORD64)(DWORD32)(base->addr);
                base->mmio_info.dwPhysMemSizeInBytes = (DWORD64)(DWORD32)(base->size);
                
                base->addr = (unsigned long)MapPhysToLin(base->mmio_info);
                if ((unsigned char*)(base->addr) == NULL)
                {
                    err_print((char*)"%s: fail to map MMIO region!\n", __FUNCTION__);
                    goto FAIL_IO_ALLOC;
                }
            #elif defined   USE_PHYMEM
                base->addr = (unsigned long)MapPhyMem(base->addr, base->size);
                if ((unsigned char*)(base->addr) == NULL)
                {
                    err_print((char*)"%s: fail to map MMIO region!\n", __FUNCTION__);
                    goto FAIL_IO_ALLOC;
                }
            #elif defined(DMP_DOS_WATCOM) || defined(USE_PCIDEBUG)
                // do nothing ...
            #else
                err_print((char*)"%s: MMIO is not supported!\n", __FUNCTION__);
                goto FAIL_IO_ALLOC;
            #endif

            // TODO: for WinXP or Linux Kernel Mode, alloc MMIO region here
            break;
        default:
            err_print((char*)"%s: unknown io_type!\n", __FUNCTION__);
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
            // TODO: for WinXP or Linux Kernel Mode, free Port-I/O region here
            break;
        case IO_USE_MMIO:
            #if defined     DMP_DOS_DJGPP
                dpmi_LinMapFree(base->addr);
                dpmi_SelFree(base->mmio_selector);
            #elif defined   USE_WINIO3
                UnmapPhysicalMemory(base->mmio_info);
            #elif defined   USE_PHYMEM
                UnmapPhyMem((void*)base->addr, base->size);
            #else
                // TODO: for WinXP or Linux Kernel Mode, free MMIO region here
            #endif
            break;
        default:
            err_print((char*)"%s: unknown io-type!\n", __FUNCTION__);
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

static bool init_pcisystem(void) {
    int i;

    for (i=0; i<PCI_NUM_BASEPOOL; i++)
        PCI_basePool[i].addr = 0xffffffffL;

    #if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC) || defined(DMP_WINCE_MSVC)
        return true;
    #elif defined(USE_WINIO2) || defined(USE_WINIO3) || defined(USE_PCIDEBUG) || defined(USE_PHYMEM) || defined(DMP_LINUX)
        // having been initialized in init_iosystem(), so do noting here
        return true;
    #endif
}

static bool close_pcisystem(void) {
    #if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC) || defined(DMP_WINCE_MSVC)
        return true;
    #elif defined(USE_WINIO2) || defined(USE_WINIO3) || defined(USE_PCIDEBUG) || defined(USE_PHYMEM) || defined(DMP_LINUX)
        // will be closed in close_iosystem(), so do noting here
        return true;
    #endif
}

static PCI_BASE_t* get_pci_base(void) {
    PCI_BASE_t* basePool = PCI_basePool;
    PCI_BASE_t* base = NULL;
    int i;

    io_DisableINT();
    for (i=0; i<PCI_NUM_BASEPOOL; i++)
        if (basePool[i].addr == 0xffffffffL)
        {
            base = &basePool[i];
            base->addr = 0L;
            break;
        }
    io_RestoreINT();

    return base;
}

static void free_pci_base(PCI_BASE_t* base) {
    io_DisableINT();
    if (base != NULL) base->addr = 0xffffffffL;
    io_RestoreINT();
}

static void* alloc_pci(unsigned char bus, unsigned char dev, unsigned char fun) {
    PCI_BASE_t* base;

    if ((base = get_pci_base()) == NULL)
        err_print((char*)"%s: no free pci handle!\n", __FUNCTION__);
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
    unsigned long tmpid;

    // just ignore wrong dev & fun :p
    dev = dev & 0x1f; 
    fun = fun & 0x07;

    // get VID & DID
    io_DisableINT();
    tmpid = pci_indw(PCI_GET_CF8(bus, dev, fun));
    io_RestoreINT();

    if (tmpid == 0xffffffffUL)
	if (fun == 0)
    {
        err_print((char*)"%s: invalid PCI device!\n", __FUNCTION__);
        return NULL;
    }

    return alloc_pci(bus, dev, fun);
}

DMPAPI(void*) pci_Find(unsigned short vid, unsigned short did) {
    unsigned long pciid = ((unsigned long)did << 16) + (unsigned long)vid;
    unsigned long tmpid, pciaddr;
    unsigned char tmpht;
    int bus, dev, fun;

    if (pciid == 0xffffffffUL) return NULL;

    for (bus=0; bus<256; bus++)
    for (dev=0; dev<32;  dev++)
    for (fun=0; fun<8;   fun++)
    {
        pciaddr = PCI_GET_CF8(bus, dev, fun);

        io_DisableINT();
        tmpid = pci_indw(pciaddr);
        io_RestoreINT();
        
        if (tmpid == pciid) return pci_Alloc((unsigned char)bus, (unsigned char)dev, (unsigned char)fun);
        if (fun == 0)
        {
            if (tmpid == 0xffffffffUL) break;  // invalid PCI device (note: shouldn't do this optimization for Vortex86DX2's internal PCI devices)

            io_DisableINT();
            tmpht = pci_inb(pciaddr + 0x0eL);
            io_RestoreINT();
            if ((tmpht & 0x80) == 0) break;  // single-function PCI device
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

    io_DisableINT();
    //pci_outdw(base->addr + (unsigned long)(offset & 0xfc), val);
    pci_outdw(base->addr + (unsigned long)offset, val);  // assume offset % 4 == 0
    io_RestoreINT();
}

DMPAPI(unsigned long) pci_In32(void* handle, unsigned char offset) {
    PCI_BASE_t* base = (PCI_BASE_t*)handle;
    unsigned long tmp;

    io_DisableINT();
    //tmp = pci_indw(base->addr + (unsigned long)(offset & 0xfc));
    tmp = pci_indw(base->addr + (unsigned long)offset);  // assume offset % 4 == 0
    io_RestoreINT();

    return tmp;
}

DMPAPI(void) pci_Out16(void* handle, unsigned char offset, unsigned short val) {
    PCI_BASE_t* base = (PCI_BASE_t*)handle;

    io_DisableINT();
    //pci_outw(base->addr + (unsigned long)(offset & 0xfe), val);
    pci_outw(base->addr + (unsigned long)offset, val);  // assume offset % 4 < 3
    io_RestoreINT();
}

DMPAPI(unsigned short) pci_In16(void* handle, unsigned char offset) {
    PCI_BASE_t* base = (PCI_BASE_t*)handle;
    unsigned short tmp;

    io_DisableINT();
    //tmp = pci_inw(base->addr + (unsigned long)(offset & 0xfe));
    tmp = pci_inw(base->addr + (unsigned long)offset);  // assume offset % 4 < 3
    io_RestoreINT();

    return tmp;
}

DMPAPI(void) pci_Out8(void* handle, unsigned char offset, unsigned char val) {
    PCI_BASE_t* base = (PCI_BASE_t*)handle;

    io_DisableINT();
    pci_outb(base->addr + (unsigned long)offset, val);
    io_RestoreINT();
}

DMPAPI(unsigned char) pci_In8(void* handle, unsigned char offset) {
    PCI_BASE_t* base = (PCI_BASE_t*)handle;
    unsigned char tmp;

    io_DisableINT();
    tmp = pci_inb(base->addr + (unsigned long)offset);
    io_RestoreINT();

    return tmp;
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
		case 0x36504D44L:
			return CPU_VORTEX86DX3;
		case 0x37504D44L:
			return CPU_VORTEX86EX;
	}

	return CPU_UNSUPPORTED;
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
    if (vx86_CpuID() == CPU_VORTEX86EX || vx86_CpuID() == CPU_VORTEX86DX2 || vx86_CpuID() == CPU_VORTEX86DX3)
    {
        // South Bridge fun1 exists (note SB fun1 isn't a normal PCI device -- its vid & did are 0xffff)
        if ((VX86_pciDev[VX86_SB1]  = pci_Alloc(0x00, 0x07, 0x01)) == NULL) goto FAIL_INITVX86;
    }
    return true;

FAIL_INITVX86:
    for (i=0; i<(int)(sizeof(VX86_pciDev)/sizeof(VX86_pciDev[0])); i++)
    {
        pci_Free(VX86_pciDev[i]);
        VX86_pciDev[i] = NULL;
    }
    err_print((char*)"%s: fail to setup system PCI devices!\n", __FUNCTION__);
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
            err_print((char*)"%s: fail to init I/O access system!\n", __FUNCTION__);
            goto FAIL_IOINIT_IO;
        }
        if (init_pcisystem() == false)
        {
            err_print((char*)"%s: fail to init PCI-CFG access system!\n", __FUNCTION__);
            goto FAIL_IOINIT_PCI;
        }
        if (init_vx86() == false)
        {
            err_print((char*)"%s: fail to init Vortex86 util system!\n", __FUNCTION__);
            goto FAIL_IOINIT_VX86;
        }
        if (init_irqsystem() == false)
        {
            err_print((char*)"%s: fail to init IRQ util system!\n", __FUNCTION__);
            goto FAIL_IOINIT_IRQ;
        }
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
            err_print((char*)"%s: fail to close IRQ util system!\n", __FUNCTION__);
            result = false;
        }
        if (close_vx86() == false)
        {
            err_print((char*)"%s: fail to close Vortex86 util system!\n", __FUNCTION__);
            result = false;
        }
        if (close_pcisystem() == false)
        {
            err_print((char*)"%s: fail to close PCI-CFG access system!\n", __FUNCTION__);
            result = false;
        }
        if (close_iosystem()  == false)
        {
            err_print((char*)"%s: fail to close I/O access system!\n", __FUNCTION__);
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
    // TODO: for WinXP or Linux Kernel Mode, allocate kernel memory

    #if defined(DMP_DOS_DJGPP)
        size_t* p = (size_t*)malloc(size + sizeof(size_t));

        if (p != NULL) //&&
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
    // TODO: for WinXP or Linux Kernel Mode, free kernel memory

    #if defined(DMP_DOS_DJGPP)
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

