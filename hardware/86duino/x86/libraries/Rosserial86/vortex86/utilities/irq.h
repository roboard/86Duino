/*******************************************************************************

  irq.h - DM&P Vortex86 IRQ Library
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


#ifndef __IRQ_H
#define __IRQ_H

#include <dmpcfg.h>


#ifdef __cplusplus
extern "C" {
#endif

/* =============================================
 *   DOS IRQ Functions
 * =============================================
 */
#if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)
    #ifdef DMP_DOS_DJGPP
        // for avoiding gcc to optimize user-defined irq handlers with -fcaller-saves
        #define _IRQ_HANDLER(rtype) rtype __attribute__((optimize("O1")))
    #else
        #define _IRQ_HANDLER(rtype) rtype
    #endif

    DMPAPI(bool) _irq_Init(void);
    DMPAPI(bool) _irq_Close(void);
    DMPAPI(bool) _irq_InstallISR(int irq, int (*irq_handler)(int));
    DMPAPI(bool) _irq_UninstallISR(int irq);

    #define _IRQ_CHAIN      (0)  // irq_handler() returns this to chain the old ISR
    #define _IRQ_POSSESS    (1)  // irq_handler() returns this to skip the old ISR
#endif


/* =============================================
 *   SoftIRQ Functions
 * =============================================
 */
#define IRQ_DPC_NONE        (0x00L)
#define IRQ_DPC_USEFPU      (0x10L)
DMPAPI(void*) irq_AllocDPC(void (*dpc)(void*), void* dpc_data, unsigned long flags);
DMPAPI(bool)  irq_FreeDPC(void* dpc_handle);

#define IRQ_DPC_FAIL        (0)
#define IRQ_DPC_RETRIG      (1)
#define IRQ_DPC_SUCCESS     (2)
DMPAPI(int) irq_TriggerDPC(void* dpc_handle);

#define IRQERR_UNEXPECTED_DPC   (0x10L)
#define IRQERR_NUMEROUS_DPC     (0x20L)
DMPAPI(unsigned long) irq_GetDPCErrFlags(void);


/* =============================================
 *   IRQ LIB Setup Functions
 * =============================================
 */
DMPAPI(bool) irq_InstallISR(int irq, int (*isr)(int, void*), void* isr_data);
DMPAPI(bool) irq_UninstallISR(int irq, void* isr_data);

// supported return values for user-registered isr(...)
#if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)
    #define ISR_NONE            (0)
    #define ISR_HANDLED         (1)
    #define ISR_POSSESS         (2)  // only for DOS and level-triggered IRQ
    #define ISR_CHAIN           (3)  // only for DOS and IRQ with the IRQ_CHAIN_OLDISR option
    #define ISR_CHAIN_POSSESS   (4)  // only for DOS and IRQ with the IRQ_CHAIN_OLDISR option
#endif

#define IRQ_EDGE_TRIGGER            (0x00L)
#define IRQ_LEVEL_TRIGGER           (0x01L)
#define IRQ_DISABLE_INTR            (0x10L)
#define IRQ_USE_FPU                 (0x40L)
#define IRQ_CHAIN_OLDISR            (0x80000000L)
DMPAPI(bool) irq_Setting(int irq, unsigned long flags);
DMPAPI(unsigned long) irq_GetSetting(int irq);

#define IRQERR_INCORRECT_REENTRY    (0x01L)
#define IRQERR_UNHANDLED_INTR       (0x02L)
#define IRQERR_NUMEROUS_INTR        (0x04L)
#define IRQERR_USELESS_CHAINING     (0x08L)
DMPAPI(unsigned long) irq_GetErrFlags(int irq);

DMPAPI(bool) irq_InInterrupt(void);

DMPAPI(bool) irq_Init(void);
DMPAPI(bool) irq_Close(void);

#ifdef __cplusplus
}
#endif
#endif

