/*******************************************************************************

  io_irq.h - Part of DM&P Vortex86 Base I/O Library
  Copyright (c) 2014 AAA <aaa@dmp.com.tw>. All right reserved.

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


/***********************  Recursive IRQ Disable/Enable  ***********************/
#if defined(DMP_DOS_DJGPP)
    #define DISABLE_INT()       __asm__ __volatile__("cli" : : : "memory")
    #define ENABLE_INT()        __asm__ __volatile__("sti" : : : "memory")
#elif defined(DMP_DOS_WATCOM)
    #define DISABLE_INT()       _disable()
    #define ENABLE_INT()        _enable()
#elif defined(DMP_DOS_BC)
    #define DISABLE_INT()       disable()
    #define ENABLE_INT()        enable()
#else
    #define DISABLE_INT()
    #define ENABLE_INT()
#endif

// well ... bad design:(
#define _io_SetCSCNT(val) do { \
                              ATOMIC_INT_SET(&irq_cscnt, val); \
                          } while(0)

#define _io_GetCSCNT()    ATOMIC_INT_GET(&irq_cscnt)

#define _io_DisableINT()  do { \
                              if (ATOMIC_INT_TEST(&irq_cscnt, 0)) { DISABLE_INT(); } \
                              ATOMIC_INT_INC(&irq_cscnt); \
                              ATOMIC_MEM_BARRIER(); \
                          } while(0)

#define _io_RestoreINT()  do { \
                              ATOMIC_MEM_BARRIER(); \
                              if (!ATOMIC_INT_TEST(&irq_cscnt, 0)) { \
                                  ATOMIC_INT_DEC(&irq_cscnt); \
                                  if (ATOMIC_INT_TEST(&irq_cscnt, 0)) { ENABLE_INT(); } \
                              } \
                          } while(0)

#define _io_EnableINT()   do { \
                              ATOMIC_MEM_BARRIER(); \
                              if (!ATOMIC_INT_TEST(&irq_cscnt, 0)) { \
                                  ATOMIC_INT_SET(&irq_cscnt, 0); \
                                  ENABLE_INT(); \
                              } \
                          } while(0)


/*
    DMPAPI(void) io_SetCSCNT(int val) {
        ATOMIC_INT_SET(&irq_cscnt, val);
    } DPMI_END_OF_LOCKED_FUNC(io_SetCSCNT)

    DMPAPI(int) io_GetCSCNT(void) {
        return ATOMIC_INT_GET(&irq_cscnt);
    } DPMI_END_OF_LOCKED_FUNC(io_GetCSCNT)

    DMPAPI(void) io_DisableINT(void) {
        if (ATOMIC_INT_TEST(&irq_cscnt, 0))
        {
            DISABLE_INT();
        }
        // getting interrupts disabled, it is now safe to update irq_cscnt
        ATOMIC_INT_INC(&irq_cscnt);
    } DPMI_END_OF_LOCKED_FUNC(io_DisableINT)

    DMPAPI(void) io_RestoreINT(void) {
        if (ATOMIC_INT_TEST(&irq_cscnt, 0)) return;
        // having interrupts disabled, it is safe to update irq_cscnt
        ATOMIC_INT_DEC(&irq_cscnt);
        if (ATOMIC_INT_TEST(&irq_cscnt, 0))
        {
            ENABLE_INT();
        }
    } DPMI_END_OF_LOCKED_FUNC(io_RestoreINT)

    DMPAPI(void) io_EnableINT(void) {
        if (ATOMIC_INT_TEST(&irq_cscnt, 0)) return;
        // having interrupts disabled, it is safe to update irq_cscnt
        ATOMIC_INT_SET(&irq_cscnt, 0);
        ENABLE_INT();
    } DPMI_END_OF_LOCKED_FUNC(io_EnableINT)
*/

/*-------------------  end. Recursive IRQ Disable/Enable  --------------------*/

