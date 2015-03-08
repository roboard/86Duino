/*******************************************************************************

  irq_dos.h - Part of DM&P Vortex86 IRQ Library
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


/************************  Primitive IRQ System for DOS  **********************/
#if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)


// well ... then many intentional & ugly #if ... #endif :p
#if defined     DMP_DOS_DJGPP

    static void (* volatile _IRQ_wrappers[16])(void);

    #ifdef __cplusplus
    extern "C" {
    #endif
    int (* volatile _IRQ_handlers[16])(int);
    volatile unsigned long _IRQ_oldHandlers[16];   // offset of old IRQ handler
    volatile unsigned long _IRQ_oldHandlers2[16];  // selector of old IRQ handler

    #define _IRQ_STACK_SIZE (256 * 1024)  // (64 * 1024)
    unsigned char* volatile _IRQ_commonStack;
    volatile int _IRQ_stackInUse;

    void _irq_IsrWrapper0(void);
    void _irq_IsrWrapper1(void);
    void _irq_IsrWrapper2(void);
    void _irq_IsrWrapper3(void);
    void _irq_IsrWrapper4(void);
    void _irq_IsrWrapper5(void);
    void _irq_IsrWrapper6(void);
    void _irq_IsrWrapper7(void);
    void _irq_IsrWrapper8(void);
    void _irq_IsrWrapper9(void);
    void _irq_IsrWrapper10(void);
    void _irq_IsrWrapper11(void);
    void _irq_IsrWrapper12(void);
    void _irq_IsrWrapper13(void);
    void _irq_IsrWrapper14(void);
    void _irq_IsrWrapper15(void);
    #ifdef __cplusplus
    }
    #endif

static void __attribute__((noinline, noclone)) get_irq_wrappers(void (* volatile wrappers[])(void)) {
    volatile int x = 0;

    if (x != 0)  // avoid DJGPP to optimize the inline assembly
    {
        // just don't want to write IRQ wrappers in a separate ASM file:p
        __asm__ volatile (
            ".align 4                                           \n\t"
            "__irq_IsrWrapper0:                                 \n\t"
            "    subl $8, %%esp                                 \n\t"  // allocate space for the pointer of the old ISR
            "    pushal                                         \n\t"
            "    movl $0, %%ebx                                 \n\t"
            "    jmp __irq_CommonWrapper                        \n\t"
            "                                                   \n\t"
            ".align 4                                           \n\t"
            "__irq_IsrWrapper1:                                 \n\t"
            "    subl $8, %%esp                                 \n\t"
            "    pushal                                         \n\t"
            "    movl $1, %%ebx                                 \n\t"
            "    jmp __irq_CommonWrapper                        \n\t"
            "                                                   \n\t"
            ".align 4                                           \n\t"
            "__irq_IsrWrapper2:                                 \n\t"
            "    subl $8, %%esp                                 \n\t"
            "    pushal                                         \n\t"
            "    movl $2, %%ebx                                 \n\t"
            "    jmp __irq_CommonWrapper                        \n\t"
            "                                                   \n\t"
            ".align 4                                           \n\t"
            "__irq_IsrWrapper3:                                 \n\t"
            "    subl $8, %%esp                                 \n\t"
            "    pushal                                         \n\t"
            "    movl $3, %%ebx                                 \n\t"
            "    jmp __irq_CommonWrapper                        \n\t"
            "                                                   \n\t"
            ".align 4                                           \n\t"
            "__irq_IsrWrapper4:                                 \n\t"
            "    subl $8, %%esp                                 \n\t"
            "    pushal                                         \n\t"
            "    movl $4, %%ebx                                 \n\t"
            "    jmp __irq_CommonWrapper                        \n\t"
            "                                                   \n\t"
            ".align 4                                           \n\t"
            "__irq_IsrWrapper5:                                 \n\t"
            "    subl $8, %%esp                                 \n\t"
            "    pushal                                         \n\t"
            "    movl $5, %%ebx                                 \n\t"
            "    jmp __irq_CommonWrapper                        \n\t"
            "                                                   \n\t"
            ".align 4                                           \n\t"
            "__irq_IsrWrapper6:                                 \n\t"
            "    subl $8, %%esp                                 \n\t"
            "    pushal                                         \n\t"
            "    movl $6, %%ebx                                 \n\t"
            "    jmp __irq_CommonWrapper                        \n\t"
            "                                                   \n\t"
            ".align 4                                           \n\t"
            "__irq_IsrWrapper7:                                 \n\t"
            "    subl $8, %%esp                                 \n\t"
            "    pushal                                         \n\t"
            "    movl $7, %%ebx                                 \n\t"
            "    jmp __irq_CommonWrapper                        \n\t"
            "                                                   \n\t"
            ".align 4                                           \n\t"
            "__irq_IsrWrapper8:                                 \n\t"
            "    subl $8, %%esp                                 \n\t"
            "    pushal                                         \n\t"
            "    movl $8, %%ebx                                 \n\t"
            "    jmp __irq_CommonWrapper                        \n\t"
            "                                                   \n\t"
            ".align 4                                           \n\t"
            "__irq_IsrWrapper9:                                 \n\t"
            "    subl $8, %%esp                                 \n\t"
            "    pushal                                         \n\t"
            "    movl $9, %%ebx                                 \n\t"
            "    jmp __irq_CommonWrapper                        \n\t"
            "                                                   \n\t"
            ".align 4                                           \n\t"
            "__irq_IsrWrapper10:                                \n\t"
            "    subl $8, %%esp                                 \n\t"
            "    pushal                                         \n\t"
            "    movl $10, %%ebx                                \n\t"
            "    jmp __irq_CommonWrapper                        \n\t"
            "                                                   \n\t"
            ".align 4                                           \n\t"
            "__irq_IsrWrapper11:                                \n\t"
            "    subl $8, %%esp                                 \n\t"
            "    pushal                                         \n\t"
            "    movl $11, %%ebx                                \n\t"
            "    jmp __irq_CommonWrapper                        \n\t"
            "                                                   \n\t"
            ".align 4                                           \n\t"
            "__irq_IsrWrapper12:                                \n\t"
            "    subl $8, %%esp                                 \n\t"
            "    pushal                                         \n\t"
            "    movl $12, %%ebx                                \n\t"
            "    jmp __irq_CommonWrapper                        \n\t"
            "                                                   \n\t"
            ".align 4                                           \n\t"
            "__irq_IsrWrapper13:                                \n\t"
            "    subl $8, %%esp                                 \n\t"
            "    pushal                                         \n\t"
            "    movl $13, %%ebx                                \n\t"
            "    jmp __irq_CommonWrapper                        \n\t"
            "                                                   \n\t"
            ".align 4                                           \n\t"
            "__irq_IsrWrapper14:                                \n\t"
            "    subl $8, %%esp                                 \n\t"
            "    pushal                                         \n\t"
            "    movl $14, %%ebx                                \n\t"
            "    jmp __irq_CommonWrapper                        \n\t"
            "                                                   \n\t"
            ".align 4                                           \n\t"
            "__irq_IsrWrapper15:                                \n\t"
            "    subl $8, %%esp                                 \n\t"
            "    pushal                                         \n\t"
            "    movl $15, %%ebx                                \n\t"
            "    jmp __irq_CommonWrapper                        \n\t"
            "                                                   \n\t"
            ".align 4                                           \n\t"
            "__irq_CommonWrapper:                               \n\t"
            "    pushw %%ds                                     \n\t"  // 1. restore the user's DS
            "    pushw %%es                                     \n\t"
            "    pushw %%fs                                     \n\t"
            "    pushw %%gs                                     \n\t"
            "                                                   \n\t"
            "    movw %%cs:___djgpp_ds_alias, %%ax              \n\t"
            "    movw %%ax, %%ds                                \n\t"
            "    movw %%ax, %%es                                \n\t"
            "    movw %%ax, %%fs                                \n\t"
            "    movw %%ax, %%gs                                \n\t"
            "                                                   \n\t"
            "    incl __IRQ_stackInUse                          \n\t"  // 2. switch ISR's stack (here isn't the best solution; when this ISR is interrupted
            "    jnz __irq_stack_switched                       \n\t"  //    by another IRQ, that IRQ's ISR may switch to its stack that may has a smaller size.)
            "                                                   \n\t"
            "    movw %%ss, %%cx                                \n\t"  //    get the old stack
            "    movl %%esp, %%esi                              \n\t"
            "    movw %%ax, %%ss                                \n\t"  //    switch to the new stack (note you must always update SS first, and then ESP.)
            "    movl __IRQ_commonStack, %%esp                  \n\t"
            "    pushw %%cx                                     \n\t"  //    backup the old stack in the new stack
            "    pushl %%esi                                    \n\t"
            "                                                   \n\t"
            "__irq_stack_switched:                              \n\t"  // 3. call the IRQ handler (assume the handler is at "cli" when returning.)
        //  "    cld                                            \n\t"
            "    cli                                            \n\t"  //    (DJGPP seems not disable the interrupt flag when entering ISR?)
            "    movl __IRQ_handlers(,%%ebx,4), %%eax           \n\t"
            "    pushl %%ebx                                    \n\t"  //    pass the IRQ number as the argument of the IRQ handler
            "    call *%%eax                                    \n\t"
            "    addl $4, %%esp                                 \n\t"  //    remove the argument of the IRQ handler
            "    cli                                            \n\t"  //    must ensure the interrupt flag is disabled before the next step
            "                                                   \n\t"
            "    cmpl $0, __IRQ_stackInUse                      \n\t"  // 4. restore the old stack (we assume _IRQ_handler() has "cli" before return.)
            "    jnz __irq_stack_restored                       \n\t"
            "    popl %%esi                                     \n\t"
            "    popw %%cx                                      \n\t"
            "    movw %%cx, %%ss                                \n\t"
            "    movl %%esi, %%esp                              \n\t"
            "__irq_stack_restored:                              \n\t"
            "    decl __IRQ_stackInUse                          \n\t"
            "                                                   \n\t"
            "    orl %%eax, %%eax                               \n\t"  // 5. restore all registers and exit the ISR
            "    jz __irq_chain_old_isr                         \n\t"  //    chain the old ISR if the IRQ handler returns 0 (note %eax = return value;
            "                                                   \n\t"  //    also, the IRQ handler should avoid sending EOI when it wants to chian the old ISR.)
            "    popw %%gs                                      \n\t"
            "    popw %%fs                                      \n\t"
            "    popw %%es                                      \n\t"
            "    popw %%ds                                      \n\t"
            "    popal                                          \n\t"
            "    addl $8, %%esp                                 \n\t"
            "    sti                                            \n\t"  //    must issue "sti" here in DPMI protected-mode interrupt
            "    iret                                           \n\t"
            "                                                   \n\t"
            "__irq_chain_old_isr:                               \n\t"
            "    movl __IRQ_oldHandlers(,%%ebx,4), %%eax        \n\t"
            "    movl __IRQ_oldHandlers2(,%%ebx,4), %%ecx       \n\t"
            "    popw %%gs                                      \n\t"
            "    popw %%fs                                      \n\t"
            "    popw %%es                                      \n\t"
            "    popw %%ds                                      \n\t"
            "    movl %%eax, %%ss:32(%%esp)                     \n\t"
            "    movl %%ecx, %%ss:36(%%esp)                     \n\t"
            "    popal                                          \n\t"
            "    lret                                           \n\t"  //    jump to the old ISR
            :
            :
            : "memory"
        );
    }



#elif defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)

    #if defined     DMP_DOS_WATCOM
        #define __INTERRUPT__   __interrupt __far
        #define __INTARGS__
        typedef void (__interrupt __far *_IRQ_ISR_t)();

        #define MAKE_WRAPPER(num)  \
        static void __INTERRUPT__ _irq_IsrWrapper##num(__INTARGS__) {  \
            if ((_IRQ_handlers[num])(num) == 0)  \
                _chain_intr(_IRQ_oldHandlers[num]);  \
        }
    #else
        #define __INTERRUPT__   interrupt far
        #ifdef __cplusplus
            #define __INTARGS__     ...
        #else
            #define __INTARGS__
        #endif
        typedef void interrupt (far *_IRQ_ISR_t)(__INTARGS__);

        // note: in Borland C/C++ 3.1, _chain_intr() has a bug that would crash ISR that has no local variable; 
        //       and you should backup the 16-bit MSB of all 32-bit registers (EAX, EBX, ...) if the IRQ handler 
        //       will employ 32-bit registers.
        #define MAKE_WRAPPER(num)  \
        static void __INTERRUPT__ _irq_IsrWrapper##num(__INTARGS__) {  \
            volatile int x=0;  \
            __emit__(0x66); _asm pusha;\
            if ((_IRQ_handlers[num])(num) == 0)  \
            {  \
                __emit__(0x66); _asm popa;\
                _chain_intr(_IRQ_oldHandlers[num]);  \
            }  \
            __emit__(0x66); _asm popa;\
        }
    #endif
    static _IRQ_ISR_t _IRQ_wrappers[16];

    static int (*_IRQ_handlers[16])(int);
    static _IRQ_ISR_t _IRQ_oldHandlers[16];

    MAKE_WRAPPER(0)
    MAKE_WRAPPER(1)
    MAKE_WRAPPER(2)
    MAKE_WRAPPER(3)
    MAKE_WRAPPER(4)
    MAKE_WRAPPER(5)
    MAKE_WRAPPER(6)
    MAKE_WRAPPER(7)
    MAKE_WRAPPER(8)
    MAKE_WRAPPER(9)
    MAKE_WRAPPER(10)
    MAKE_WRAPPER(11)
    MAKE_WRAPPER(12)
    MAKE_WRAPPER(13)
    MAKE_WRAPPER(14)
    MAKE_WRAPPER(15)

static void get_irq_wrappers(_IRQ_ISR_t wrappers[]) {
#endif



    wrappers[0]  = _irq_IsrWrapper0;
    wrappers[1]  = _irq_IsrWrapper1;
    wrappers[2]  = _irq_IsrWrapper2;
    wrappers[3]  = _irq_IsrWrapper3;
    wrappers[4]  = _irq_IsrWrapper4;
    wrappers[5]  = _irq_IsrWrapper5;
    wrappers[6]  = _irq_IsrWrapper6;
    wrappers[7]  = _irq_IsrWrapper7;
    wrappers[8]  = _irq_IsrWrapper8;
    wrappers[9]  = _irq_IsrWrapper9;
    wrappers[10] = _irq_IsrWrapper10;
    wrappers[11] = _irq_IsrWrapper11;
    wrappers[12] = _irq_IsrWrapper12;
    wrappers[13] = _irq_IsrWrapper13;
    wrappers[14] = _irq_IsrWrapper14;
    wrappers[15] = _irq_IsrWrapper15;
} DPMI_END_OF_LOCKED_STATIC_FUNC(get_irq_wrappers)



static int _IRQ_inUse = 0;

DMPAPI(bool) _irq_Init(void) {
    if (_IRQ_inUse == 0)
    {
        int i;

        get_irq_wrappers(_IRQ_wrappers);
        for (i=0; i<16; i++) _IRQ_handlers[i] = NULL;

        #if defined(DMP_DOS_DJGPP)
            // allocate the dedicated stack for ISR
            if ((_IRQ_commonStack = (unsigned char*)ker_Malloc(_IRQ_STACK_SIZE)) == NULL) return false;
            _IRQ_commonStack = _IRQ_commonStack + (_IRQ_STACK_SIZE - 32);  // reserve 32 bytes for safety
            _IRQ_stackInUse  = -1;

            DPMI_LOCK_VAR(_IRQ_stackInUse);
            DPMI_LOCK_VAR(_IRQ_commonStack);
            DPMI_LOCK_VAR(_IRQ_handlers);
            DPMI_LOCK_VAR(_IRQ_oldHandlers);
            DPMI_LOCK_VAR(_IRQ_oldHandlers2);
        
            DPMI_LOCK_FUNC(get_irq_wrappers);
        #endif
    }

    _IRQ_inUse++;
    return true;
}

DMPAPI(bool) _irq_Close(void) {
    if (_IRQ_inUse == 1)
    {
        int i;

        for (i=0; i<16; i++) if (_IRQ_handlers[i] != NULL) return false;

        #if defined(DMP_DOS_DJGPP)
            ker_Mfree((void*)(_IRQ_commonStack - (_IRQ_STACK_SIZE - 32)));
        #endif
    }

    if (_IRQ_inUse != 0) _IRQ_inUse--;
    return true;
}


DMPAPI(bool) _irq_InstallISR(int irq, int (*irq_handler)(int)) {
    int intvec = i8259_GetINTVEC(irq);

    if ((irq < 0) || (irq > 15)) return false;
    if (_IRQ_handlers[irq] != NULL) return false;

    _IRQ_handlers[irq] = irq_handler;

    #if defined     DMP_DOS_DJGPP
    {
        __dpmi_paddr isr_addr;

        __dpmi_get_protected_mode_interrupt_vector(intvec, &isr_addr);
        _IRQ_oldHandlers[irq]  = isr_addr.offset32;
        _IRQ_oldHandlers2[irq] = (unsigned long)isr_addr.selector;
        
        isr_addr.offset32 = (unsigned long)_IRQ_wrappers[irq];
        isr_addr.selector = _go32_my_cs();
        __dpmi_set_protected_mode_interrupt_vector(intvec, &isr_addr);
    }
    #elif defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)
        _IRQ_oldHandlers[irq] = _dos_getvect(intvec);
        _dos_setvect(intvec, _IRQ_wrappers[irq]);
    #endif

    return true;
}

DMPAPI(bool) _irq_UninstallISR(int irq) {
    int intvec = i8259_GetINTVEC(irq);

    if ((irq < 0) || (irq > 15)) return false;
    if (_IRQ_handlers[irq] == NULL) return false;

    #if defined     DMP_DOS_DJGPP
    {
        __dpmi_paddr isr_addr;

        isr_addr.offset32 = _IRQ_oldHandlers[irq];
        isr_addr.selector = (unsigned short)_IRQ_oldHandlers2[irq];
       __dpmi_set_protected_mode_interrupt_vector(intvec, &isr_addr);
    }
    #elif defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)
        _dos_setvect(intvec, _IRQ_oldHandlers[irq]);
    #endif

    _IRQ_handlers[irq] = NULL;
    return true;
}


#endif
/*-------------------  end. Primitive IRQ System for DOS  --------------------*/

