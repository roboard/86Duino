/*******************************************************************************

  io_pic.h - Part of DM&P Vortex86 Base I/O Library
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


/*****************************  i8259-PIC Access  *****************************/
#define I8259M_OCW2             (0x20)
#define I8259S_OCW2             (0xa0)
#define I8259M_IMR              (0x21)
#define I8259S_IMR              (0xa1)

#define I8259M_VECTOR_START     (0x08)
#define I8259S_VECTOR_START     (0x70)

#if defined(DMP_DOS_DJGPP) && !defined(DMP_DOS_DJGPP_NO_VIRMEM)  // port-I/O functions that is safe to use in interrupt context
    static void i8259_outb(unsigned short addr, unsigned char val) {
        __asm__ volatile ("outb %0, %1" : : "a" (val), "d" (addr));
    } DPMI_END_OF_LOCKED_STATIC_FUNC(i8259_outb)

    static unsigned char i8259_inb(unsigned short addr) {
        unsigned char val;
        __asm__ volatile ("inb %1, %0" : "=a" (val) : "d" (addr));
        return val;
    } DPMI_END_OF_LOCKED_STATIC_FUNC(i8259_inb)
#else
    #define i8259_outb(addr, val)   portio_outpb(addr, val)
    #define i8259_inb(addr)         portio_inpb(addr)
#endif

DMPAPI(void) i8259_DisableIRQ(int irq) {
    unsigned short imr = (irq > 7)? I8259S_IMR : I8259M_IMR;

    _io_DisableINT();
    {
        i8259_outb(imr, i8259_inb(imr) | (1 << (irq & 0x07)));
    }
    _io_RestoreINT();
} DPMI_END_OF_LOCKED_FUNC(i8259_DisableIRQ)

DMPAPI(void) i8259_EnableIRQ(int irq) {
    unsigned short imr = (irq > 7)? I8259S_IMR : I8259M_IMR;

    //if (irq > 7) i8259_EnableIRQ(2);
    _io_DisableINT();
    {
        i8259_outb(imr, i8259_inb(imr) & ~(1 << (irq & 0x07)));
    }
    _io_RestoreINT();
} DPMI_END_OF_LOCKED_FUNC(i8259_EnableIRQ)

DMPAPI(bool) i8259_IsIRQEnabled(int irq) {
    unsigned short imr = (irq > 7)? I8259S_IMR : I8259M_IMR;
    if ((i8259_inb(imr) & (1 << (irq & 0x07))) == 0) return true; else return false;
} DPMI_END_OF_LOCKED_FUNC(i8259_IsIRQEnabled)

DMPAPI(void) i8259_SendEOI(int irq) {
    if (irq > 7) i8259_outb(I8259S_OCW2, 0x20);
    i8259_outb(I8259M_OCW2, 0x20);
} DPMI_END_OF_LOCKED_FUNC(i8259_SendEOI)

DMPAPI(int) i8259_GetINTVEC(int irq) {
    return (irq <= 7)? I8259M_VECTOR_START + irq : I8259S_VECTOR_START + irq - 8;
} DPMI_END_OF_LOCKED_FUNC(i8259_GetINTVEC)
/*--------------------------  end. i8259-PIC Access  -------------------------*/
