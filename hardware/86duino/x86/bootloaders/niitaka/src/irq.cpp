/*******************************************************************************

  irq.cpp - DM&P Vortex86 IRQ Library
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


#define __IRQ_LIB
////////////////////////////////////////////////////////////////////////////////
//
//    IRQ library for DM&P Vortex86 CPUs
//    ----------------------------------
//    all functions in this lib can be used in interrupt context.
////////////////////////////////////////////////////////////////////////////////



#include <dmpcfg.h>

#if defined     DMP_DOS_DJGPP
    #include <stdio.h>
    #include <stdlib.h>
    #include <go32.h>
    #include <dpmi.h>
#elif defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)
    #include <stdio.h>
    #include <stdlib.h>
    #include <dos.h>
#endif

#include "io.h"
#include "io_atom.h"
#include "irq.h"

extern ATOMIC_INT_t irq_cscnt;  // defined in io.cpp; well ... bad design for speed:(
#include "io_irq.h"

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
 *   Shared IRQ Warpper
 * =============================================
 */
#define IS_LEVEL_TRIGGER(irq_no)    ((IRQ_lists[irq_no].flags & IRQ_LEVEL_TRIGGER) != 0L)
#define IS_EDGE_TRIGGER(irq_no)     ((IRQ_lists[irq_no].flags & IRQ_LEVEL_TRIGGER) == 0L)
#define ALLOW_FPU(irq_no)           ((IRQ_lists[irq_no].flags & IRQ_USE_FPU) != 0L)
#define NOT_ALLOW_FPU(irq_no)       ((IRQ_lists[irq_no].flags & IRQ_USE_FPU) == 0L)
#define ALLOW_INTR(irq_no)          ((IRQ_lists[irq_no].flags & IRQ_DISABLE_INTR) == 0L)
#define DISABLE_INTR(irq_no)        ((IRQ_lists[irq_no].flags & IRQ_DISABLE_INTR) != 0L)
#define CHAIN_OLD_ISR(irq_no)       ((IRQ_lists[irq_no].flags & IRQ_CHAIN_OLDISR) != 0L)

typedef struct
{
    int (*func)(int, void*);
    void* data;

} IRQ_ISR_t;

#define IRQ_STATE_NONE      (0)
#define IRQ_STATE_INUSE     (1)
#define IRQ_STATE_PENDING   (2)
typedef struct
{
    unsigned long flags;
    volatile int  state;
    volatile unsigned long errflags;

    bool old_irq_mask;
    unsigned long old_fpu_env[27];

    int isrs_num;
    int isrs_size;
    IRQ_ISR_t* isrs;

} IRQ_DESC_t;

static IRQ_DESC_t IRQ_lists[16];

#define IRQ_DPCSTAT_NONE        (0x00)
#define IRQ_DPCSTAT_WAITING     (0x02)
#define IRQ_DPCSTAT_RUNNING     (0x04)
typedef struct _IRQ_DPC_t
{
    volatile int  state;
    unsigned long flags;
    
    void (*func)(void*);
    void* data;

    struct _IRQ_DPC_t* next;

} IRQ_DPC_t;

#if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)
    static void dos_save_fpu(void* buf)
    {
        #if defined     DMP_DOS_DJGPP
            __asm__ __volatile__ ("fnsave %0" : "=m" (((char*)buf)[0]));
            //__asm__ __volatile__ ("fnsave %0; fwait" : "=m" (((char*)buf)[0]));
        #elif defined   DMP_DOS_WATCOM
            _asm {
                mov eax, DWORD PTR buf
                fnsave [eax]
                //fwait
            }
        #else
            unsigned int buf_off = FP_OFF(buf);
            unsigned int buf_seg = FP_SEG(buf);
            
            asm {
                push ds;
                push bx;
                mov bx, WORD PTR buf_off;
                mov ax, WORD PTR buf_seg;
                mov ds, ax;
                fnsave [bx];
                //fwait
                pop bx;
                pop ds;
            }
        #endif
    } DPMI_END_OF_LOCKED_STATIC_FUNC(dos_save_fpu)

    static void dos_restore_fpu(void* buf)
    {
        #if defined     DMP_DOS_DJGPP
            __asm__ __volatile__ ("frstor %0" : : "m" (((char*)buf)[0]));
            //__asm__ __volatile__ ("frstor %0; fwait" : : "m" (((char*)buf)[0]));
        #elif defined   DMP_DOS_WATCOM
            _asm {
                mov eax, DWORD PTR buf
                frstor [eax]
                //fwait
            }
        #else
            unsigned int buf_off = FP_OFF(buf);
            unsigned int buf_seg = FP_SEG(buf);
            
            asm {
                push ds;
                push bx;
                mov bx, WORD PTR buf_off;
                mov ax, WORD PTR buf_seg;
                mov ds, ax;
                frstor [bx];
                //fwait
                pop bx;
                pop ds;
            }
        #endif
    } DPMI_END_OF_LOCKED_STATIC_FUNC(dos_restore_fpu)


    static IRQ_DPC_t* volatile IRQ_dpcHead = NULL;
    static volatile int IRQ_dpcUseFPU = 0;
    static volatile unsigned long IRQ_dpcErrFlags = 0L;
    static void dos_do_dpc(void)  // we have CLI when entering this function
    {
        IRQ_DPC_t* head;
        IRQ_DPC_t* tmp;
        unsigned long dpc_fpu_env[27];
        bool dpc_use_fpu = false;
        int numscan = 0;

        while (IRQ_dpcHead != NULL)
        {
            if (++numscan > 500)
            {
                IRQ_dpcErrFlags |= IRQERR_NUMEROUS_DPC;
                break;
            }

            if ((IRQ_dpcUseFPU != 0) && (dpc_use_fpu == false))
            {
                dos_save_fpu(dpc_fpu_env);
                dpc_use_fpu = true;
            }
            
            head = IRQ_dpcHead;
            IRQ_dpcHead = NULL;
            IRQ_dpcUseFPU = 0;
            
            while (head != NULL)
            {
                tmp = head;
                head = head->next;

                if ((tmp->state & IRQ_DPCSTAT_WAITING) == 0L)
                    IRQ_dpcErrFlags |= IRQERR_UNEXPECTED_DPC;
                else
                    tmp->state &= ~IRQ_DPCSTAT_WAITING;

                tmp->state |= IRQ_DPCSTAT_RUNNING;
                _io_EnableINT();
                {
                    tmp->func(tmp->data);
                }
                _io_DisableINT();
                tmp->state &= ~IRQ_DPCSTAT_RUNNING;
            }
        } // end while (IRQ_dpcHead...

        if (dpc_use_fpu == true) dos_restore_fpu(dpc_fpu_env);
    } DPMI_END_OF_LOCKED_STATIC_FUNC(dos_do_dpc)


    static volatile int IRQ_sysenter = 0;
    static _IRQ_HANDLER(int) dos_shared_irq_wrapper(int irq) {
        int i, numisr, chainisr, numscan = 0;

        IRQ_sysenter++;
        _io_SetCSCNT(1);  // allow _io_EnableINT() & _io_DisableINT() to work correctly

        //-- 1. disable the current IRQ and send EOI to allow other IRQ interrupts
        if (IS_LEVEL_TRIGGER(irq)) i8259_DisableIRQ(irq);
        i8259_SendEOI(irq);
        
        //-- 2. handle IRQ reentry (only possible in edge-trigger IRQ)
        if (IRQ_lists[irq].state != IRQ_STATE_NONE)
        {
            if (IS_LEVEL_TRIGGER(irq))
            {
                IRQ_lists[irq].errflags |= IRQERR_INCORRECT_REENTRY;
                i8259_EnableIRQ(irq);
            }

            IRQ_lists[irq].state = IRQ_STATE_PENDING;
            goto IRQ_WRAPPER_EXIT;
        }
        IRQ_lists[irq].state = IRQ_STATE_INUSE;

        //-- 3. issue STI if necessary
        if (ALLOW_INTR(irq)) _io_EnableINT();

        //-- 4. save FPU context if necessary
        if (ALLOW_FPU(irq)) dos_save_fpu(IRQ_lists[irq].old_fpu_env);

        //-- 5. scan all ISRs
        while (1)
        {
            for (numisr=0, chainisr=0, i=0; i<IRQ_lists[irq].isrs_num; i++)
                switch (IRQ_lists[irq].isrs[i].func(irq, IRQ_lists[irq].isrs[i].data))
                {
                    case ISR_HANDLED:
                        numisr++;
                        break;
                    case ISR_POSSESS:  // not supported for edge-triggerred IRQ
                        numisr++;
                        if (IS_LEVEL_TRIGGER(irq)) i = IRQ_lists[irq].isrs_num;  // exit the scanning loop
                        break;
                    case ISR_CHAIN:
                        chainisr++;
                        break;
                }
            
            _io_DisableINT();
            {
                if ((numisr == 0) && (numscan == 0))
                    IRQ_lists[irq].errflags |= IRQERR_UNHANDLED_INTR;

                if (chainisr != 0)
                    IRQ_lists[irq].errflags |= IRQERR_USELESS_CHAINING;

                if (IRQ_lists[irq].state != IRQ_STATE_PENDING)
                {
                    _io_RestoreINT();
                    break;
                }
                if (++numscan > 500)
                {
                    IRQ_lists[irq].errflags |= IRQERR_NUMEROUS_INTR;
                    _io_RestoreINT();
                    break;
                }
                IRQ_lists[irq].state = IRQ_STATE_INUSE;  // change IRQ_STATE_PENDING to IRQ_STATE_INUSE (non-pending)
            }
            _io_RestoreINT();
        }

        //-- 6. restore the old context and then exit the IRQ wrapper
        if (ALLOW_FPU(irq)) dos_restore_fpu(IRQ_lists[irq].old_fpu_env);

        _io_DisableINT();
        IRQ_lists[irq].state = IRQ_STATE_NONE;

        // for level-triggered IRQ, we will disable it if no ISR can handle it (to avoid continuous interrupt-triggering)
        if (IS_LEVEL_TRIGGER(irq) && ((IRQ_lists[irq].errflags & IRQERR_UNHANDLED_INTR) == 0L)) i8259_EnableIRQ(irq);

        if (IRQ_sysenter == 1) dos_do_dpc();

    IRQ_WRAPPER_EXIT:
        _io_SetCSCNT(0);
        IRQ_sysenter--;
        return _IRQ_POSSESS;
    } DPMI_END_OF_LOCKED_STATIC_FUNC(dos_shared_irq_wrapper)


    static volatile int IRQ_sysenter_chaining = 0;
    // NOTE: to chain the old ISR, it doesn't allow to access 8259 and to
    //       enable the interrupt flag. And DPC isn't supported in this case.
    static _IRQ_HANDLER(int) dos_shared_irq_wrapper_chaining(int irq) {
        int i, numisr, chainisr;
    
        IRQ_sysenter_chaining++;
        _io_SetCSCNT(1);  // allow _io_EnableINT() & _io_DisableINT() to work correctly

        if (ALLOW_FPU(irq)) dos_save_fpu(IRQ_lists[irq].old_fpu_env);

        for (numisr=0, chainisr=0, i=0; i<IRQ_lists[irq].isrs_num; i++)
            switch (IRQ_lists[irq].isrs[i].func(irq, IRQ_lists[irq].isrs[i].data))
            {
                case ISR_HANDLED:
                    numisr++;
                    break;
                case ISR_POSSESS:  // not supported for edge-triggerred IRQ
                    numisr++;
                    if (IS_LEVEL_TRIGGER(irq)) i = IRQ_lists[irq].isrs_num;  // exit the scanning loop
                    break;
                case ISR_CHAIN:
                    numisr++;
                    chainisr++;
                    break;
                case ISR_CHAIN_POSSESS:
                    numisr++;
                    chainisr++;
                    if (IS_LEVEL_TRIGGER(irq)) i = IRQ_lists[irq].isrs_num;  // exit the scanning loop
                    break;
            }

        if (ALLOW_FPU(irq)) dos_restore_fpu(IRQ_lists[irq].old_fpu_env);

        _io_DisableINT();  // need this if the user unexpectedly enables the interrupt flag in their ISR
        _io_SetCSCNT(0);
        IRQ_sysenter_chaining--;

        if ((chainisr != 0) || (numisr == 0))
            return _IRQ_CHAIN;
        else
        {
            i8259_SendEOI(irq);
            return _IRQ_POSSESS;
        }
    } DPMI_END_OF_LOCKED_STATIC_FUNC(dos_shared_irq_wrapper_chaining)
#endif





////////////////////////////////////////////////////////////////////////////////
/////////////////////////////   Public Functions   /////////////////////////////
////////////////////////////////////////////////////////////////////////////////


/* =============================================
 *   Primitive IRQ System for DOS
 * =============================================
 */
#include "irq_dos.h"



/* =============================================
 *   SoftIRQ Functions
 * =============================================
 */
DMPAPI(void*) irq_AllocDPC(void (*dpc)(void*), void* dpc_data, unsigned long flags) {
    IRQ_DPC_t* handle;

    if (dpc == NULL)
    {
        err_print("%s: dpc is NULL!\n", __FUNCTION__);
        return NULL;
    }

    #if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)
        if ((handle = (IRQ_DPC_t*)ker_Malloc(sizeof(IRQ_DPC_t))) == NULL)
        {
            err_print("%s: fail to allocate DPC!\n", __FUNCTION__);
            return NULL;
        }
        handle->state = IRQ_DPCSTAT_NONE;
        handle->flags = flags;
        handle->func  = dpc;
        handle->data  = dpc_data;
        handle->next  = NULL;
    #else
        handle = NULL;
    #endif

    return (void*)handle;
}

DMPAPI(bool) irq_FreeDPC(void* dpc_handle) {
    if (dpc_handle == NULL) return false;

    #if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)
    {
        int state;

        _io_DisableINT();
        {
            state = ((IRQ_DPC_t*)dpc_handle)->state;
        }
        _io_RestoreINT();
        if (state != IRQ_DPCSTAT_NONE) return false;
    }
    #endif

    ker_Mfree(dpc_handle);
    return true;
}

DMPAPI(unsigned long) irq_GetDPCErrFlags(void) {
    unsigned long errflags;

    #if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)
        _io_DisableINT();
        {
            errflags = IRQ_dpcErrFlags;
        }
        _io_RestoreINT();

        return errflags;
    #else
        return 0L;
    #endif
}


DMPAPI(int) irq_TriggerDPC(void* dpc_handle) {
    IRQ_DPC_t* handle = (IRQ_DPC_t*)dpc_handle;

    if (handle == NULL) return IRQ_DPC_FAIL;

    #if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)
        _io_DisableINT();
        {
            if ((handle->state & IRQ_DPCSTAT_WAITING) != 0)
            {
                _io_RestoreINT();
                return IRQ_DPC_RETRIG;
            }

            handle->state |= IRQ_DPCSTAT_WAITING;
            handle->next = IRQ_dpcHead;
            IRQ_dpcHead = handle;
            if ((handle->flags & IRQ_DPC_USEFPU) != 0L) IRQ_dpcUseFPU++;
        }
        _io_RestoreINT();
    #endif

    return IRQ_DPC_SUCCESS;
} DPMI_END_OF_LOCKED_FUNC(irq_TriggerDPC)



/* =============================================
 *   IRQ LIB Setup Functions
 * =============================================
 */
DMPAPI(bool) irq_InstallISR(int irq, int (*isr)(int, void*), void* isr_data) {
    if (isr == NULL)
    {
        err_print("%s: isr is NULL!\n", __FUNCTION__);
        return false;
    }
    if ((irq < 0) || (irq > 15))
    {
        err_print("%s: wrong IRQ number!\n", __FUNCTION__);
        return false;
    }

    #if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)
    {
        IRQ_ISR_t* curlist;
        int i;

        curlist = IRQ_lists[irq].isrs;
        if ((curlist == NULL) || (IRQ_lists[irq].isrs_num >= IRQ_lists[irq].isrs_size))
        {
            IRQ_ISR_t* tmplist;

            if (curlist == NULL)
            {
                IRQ_lists[irq].isrs_size = 2;
                IRQ_lists[irq].isrs_num  = 0;
            }

            if ((tmplist = (IRQ_ISR_t*)ker_Malloc(sizeof(IRQ_ISR_t) * IRQ_lists[irq].isrs_size * 2)) == NULL)
            {
                err_print("%s: fail to re-allocate IRQ list!\n", __FUNCTION__);
                return false;
            }

            for (i=0; i<IRQ_lists[irq].isrs_num; i++) tmplist[i] = curlist[i];
            
            _io_DisableINT();
            {
                IRQ_lists[irq].isrs = tmplist;
                IRQ_lists[irq].isrs_size = IRQ_lists[irq].isrs_size * 2;
            }
            _io_RestoreINT();
            
            ker_Mfree(curlist);
        }
        
        IRQ_lists[irq].isrs[IRQ_lists[irq].isrs_num].func = isr;
        IRQ_lists[irq].isrs[IRQ_lists[irq].isrs_num].data = isr_data;
        _io_DisableINT();
        {
            IRQ_lists[irq].isrs_num++;
        }
        _io_RestoreINT();
        
        if (curlist == NULL)
        {
            bool result;
            
            IRQ_lists[irq].old_irq_mask = i8259_IsIRQEnabled(irq);
        
            result = (CHAIN_OLD_ISR(irq))? _irq_InstallISR(irq, dos_shared_irq_wrapper_chaining) : 
                                           _irq_InstallISR(irq, dos_shared_irq_wrapper);
            if (result == false)
            {
                ker_Mfree(IRQ_lists[irq].isrs);
                IRQ_lists[irq].isrs = NULL;

                err_print("%s: fail to install IRQ wrapper!\n", __FUNCTION__);
                return false;
            }

            i8259_EnableIRQ(irq);
        } // end if (curlist...
    }
    #endif

    return true;
}

DMPAPI(bool) irq_UninstallISR(int irq, void* isr_data) {
    if ((irq < 0) || (irq > 15))
    {
        err_print("%s: wrong IRQ number!\n", __FUNCTION__);
        return false;
    }

    #if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)
    {
        int i;

        if (IRQ_lists[irq].isrs == NULL)
        {
            err_print("%s: ISR list is empty!\n", __FUNCTION__);
            return false;
        }

        for (i=0; i<IRQ_lists[irq].isrs_num; i++)
            if (IRQ_lists[irq].isrs[i].data == isr_data) break;

        if (i == IRQ_lists[irq].isrs_num)
        {
            err_print("%s: find no specified ISR!\n", __FUNCTION__);
            return false;
        }

        _io_DisableINT();
        {
            if (IRQ_lists[irq].isrs_num > 1)
            {
                for (IRQ_lists[irq].isrs_num--; i<IRQ_lists[irq].isrs_num; i++)
                    IRQ_lists[irq].isrs[i] = IRQ_lists[irq].isrs[i+1];
            }
            else
            {
                if (_irq_UninstallISR(irq) == false)
                {
                    _io_RestoreINT();
                    err_print("%s: fail to uninstall IRQ wrapper!\n", __FUNCTION__);
                    return false;
                }
                if (IRQ_lists[irq].old_irq_mask == false) i8259_DisableIRQ(irq);

                ker_Mfree(IRQ_lists[irq].isrs);
                IRQ_lists[irq].isrs = NULL;
            } // end if (IRQ_lists[irq].isrs_num
        }
        _io_RestoreINT();
    }
    #endif

    return true;
}

DMPAPI(bool) irq_Setting(int irq, unsigned long flags) {
    if ((irq < 0) || (irq > 15))
    {
        err_print("%s: wrong IRQ number!\n", __FUNCTION__);
        return false;
    }

    #if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)
        _io_DisableINT();
        {
            IRQ_lists[irq].flags = flags;
        }
        _io_RestoreINT();
    #endif

    return true;
}

DMPAPI(unsigned long) irq_GetSetting(int irq) {
    if ((irq < 0) || (irq > 15))
    {
        err_print("%s: wrong IRQ number!\n", __FUNCTION__);
        return 0L;
    }

    return IRQ_lists[irq].flags;
}

DMPAPI(unsigned long) irq_GetErrFlags(int irq) {
    unsigned long errflags;

    if ((irq < 0) || (irq > 15))
    {
        err_print("%s: wrong IRQ number!\n", __FUNCTION__);
        return 0;
    }

    #if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)
        _io_DisableINT();
        {
            errflags = IRQ_lists[irq].errflags;
        }
        _io_RestoreINT();

        return errflags;
    #else
        return 0L;
    #endif
}

DMPAPI(bool) irq_InInterrupt(void) {  // can be used without irq_Init()
    #if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)
        if (IRQ_sysenter != 0) return true;
        if (IRQ_sysenter_chaining != 0) return true;
    #endif
    return false;
} DPMI_END_OF_LOCKED_FUNC(irq_InInterrupt)



static int IRQ_inUse = 0;

DMPAPI(bool) irq_Init(void) {
    int i;
    
    if (IRQ_inUse == 0)
    {
        for (i=0; i<16; i++)
        {
            IRQ_lists[i].flags    = IRQ_LEVEL_TRIGGER;
            IRQ_lists[i].state    = IRQ_STATE_NONE;
            IRQ_lists[i].errflags = 0L;
            IRQ_lists[i].isrs     = NULL;
        }

        #if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)
            if (_irq_Init() == false)
            {
                err_print("%s: fail to init DOS IRQ system!\n", __FUNCTION__);
                return false;
            }

            IRQ_dpcHead     = NULL;
            IRQ_dpcUseFPU   = 0;
            IRQ_dpcErrFlags = 0L;
            IRQ_sysenter    = 0;
            IRQ_sysenter_chaining = 0;

            #if defined(DMP_DOS_DJGPP)
                DPMI_LOCK_VAR(IRQ_lists);
                DPMI_LOCK_VAR(IRQ_sysenter);
                DPMI_LOCK_VAR(IRQ_sysenter_chaining);
                DPMI_LOCK_VAR(IRQ_dpcHead);
                DPMI_LOCK_VAR(IRQ_dpcUseFPU);
                DPMI_LOCK_VAR(IRQ_dpcErrFlags);

                DPMI_LOCK_FUNC(dos_save_fpu);
                DPMI_LOCK_FUNC(dos_restore_fpu);
                DPMI_LOCK_FUNC(dos_do_dpc);
                DPMI_LOCK_FUNC(dos_shared_irq_wrapper);
                DPMI_LOCK_FUNC(dos_shared_irq_wrapper_chaining);

                DPMI_LOCK_FUNC(irq_TriggerDPC);
                DPMI_LOCK_FUNC(irq_InInterrupt);
            #endif
        #endif
    } // end if (IRQ_inUse...

    IRQ_inUse++;
    return true;
}

DMPAPI(bool) irq_Close(void) {
    int i;

    if (IRQ_inUse == 1)
    {
        for (i=0; i<16; i++)
            if (IRQ_lists[i].isrs != NULL)
            {
                err_print("%s: there exists ISRs to be uninstalled!\n", __FUNCTION__);
                return false;
            }

        #if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)
            if (_irq_Close() == false)
            {
                err_print("%s: fail to close DOS IRQ system!\n", __FUNCTION__);
                return false;
            }
        #endif
    } // end if (IRQ_inUse...

    if (IRQ_inUse > 0) IRQ_inUse--;
    return true;
}

