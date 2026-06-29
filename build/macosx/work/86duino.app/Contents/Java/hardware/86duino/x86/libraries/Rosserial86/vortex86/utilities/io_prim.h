/*******************************************************************************

  io_prim.h - Part of DM&P Vortex86 Base I/O Library
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


#if defined(DMP_DOS_BC)
    // 386 instructions
    #define EMIT_DWORD(x) \
        __emit__((unsigned char)x);__emit__((unsigned char)(x>>8));\
        __emit__((unsigned char)(x>>16));__emit__((unsigned char)(x>>24))

    #define MOV_EAX(x) __emit__(0x66, 0xb8); EMIT_DWORD(x)

    #define MOV_VAR_EAX(x) __emit__(0x66); _asm mov WORD PTR x, ax
    #define MOV_VAR_EDX(x) __emit__(0x66); _asm mov WORD PTR x, dx

    #define MOV_EAX_VAR(x) __emit__(0x66); _asm mov ax, WORD PTR x

    #define OUT_EDX_EAX __emit__(0x66); _asm out dx, ax
    #define IN_EAX_EDX __emit__(0x66); _asm in ax, dx
    
    #define PUSH_EAX __emit__(0x66); _asm push ax
    #define POP_EAX __emit__(0x66); _asm pop ax


    // note: BC doesn't allow inline assembly in inline functions
    static void _portio_outpdw(unsigned short addr, unsigned long val) {
        //_asm push dx;
        //PUSH_EAX;
        
        _asm mov dx, WORD PTR addr;
        MOV_EAX_VAR(val);
        OUT_EDX_EAX;

        //POP_EAX;
        //_asm pop dx;
    }

    static unsigned long _portio_inpdw(unsigned short addr) {
        unsigned long retval;

        //_asm push dx;
        //PUSH_EAX;    

        _asm mov dx, WORD PTR addr;
        IN_EAX_EDX;
        MOV_VAR_EAX(retval);

        //POP_EAX;
        //_asm pop dx;
    
        return retval;
    }
#endif



/****************************  Primitive I/O Access  **************************/

/* ========================================
 *   Port-I/O Access
 * ========================================
 */
__dmp_inline(void) portio_outpdw(unsigned short addr, unsigned long val) {
    #if defined     DMP_DOS_DJGPP
        outportl(addr, val);
    #elif defined   DMP_DOS_WATCOM
        outpd(addr, val);
    #elif defined   DMP_DOS_BC
        _portio_outpdw(addr, val);
    #elif defined(USE_WINIO2) || defined(USE_WINIO3)
	    SetPortVal(addr, val, 4);
    #elif defined   USE_PCIDEBUG
		_IoWriteLong((ULONG)addr, val);
    #elif defined   USE_PHYMEM
        WritePortLong(addr, val);
    #elif defined	DMP_WINCE_MSVC
        _asm {
            mov dx, WORD PTR addr
            mov eax, val
            out dx, eax
        }
    #elif defined   DMP_LINUX
        outl(val, addr);
    #endif
}
    
__dmp_inline(unsigned long) portio_inpdw(unsigned short addr) {
    #if defined     DMP_DOS_DJGPP
        return inportl(addr);
    #elif defined   DMP_DOS_WATCOM
        return inpd(addr);
    #elif defined   DMP_DOS_BC
        return _portio_inpdw(addr);
    #elif defined(USE_WINIO2) || defined(USE_WINIO3)
        unsigned long val;
	    GetPortVal(addr, &val, 4);
	    return val;
    #elif defined   USE_PCIDEBUG
		return _IoReadLong((ULONG)addr);
    #elif defined   USE_PHYMEM
        return ReadPortLong(addr);
    #elif defined	DMP_WINCE_MSVC
        unsigned long val;

        _asm {
            mov dx, WORD PTR addr
            in eax, dx
            mov val, eax
        }
        return val;
    #elif defined   DMP_LINUX
        return inl(addr);
    #endif
}

__dmp_inline(void) portio_outpw(unsigned short addr, unsigned short val) {
    #if defined     DMP_DOS_DJGPP
        outportw(addr, val);
    #elif defined   DMP_DOS_WATCOM
        outpw(addr, val);
    #elif defined   DMP_DOS_BC
        outpw(addr, val);
    #elif defined(USE_WINIO2) || defined(USE_WINIO3)
	    SetPortVal(addr, val, 2);
    #elif defined   USE_PCIDEBUG
		_IoWriteShort((ULONG)addr, val);
    #elif defined   USE_PHYMEM
        WritePortWord(addr, val);
    #elif defined	DMP_WINCE_MSVC
        _asm {
            mov dx, WORD PTR addr
            mov ax, WORD PTR val
            out dx, ax
        }
    #elif defined   DMP_LINUX
        outw(val, addr);
    #endif
}

__dmp_inline(unsigned short) portio_inpw(unsigned short addr) {
    #if defined     DMP_DOS_DJGPP
        return inportw(addr);
    #elif defined   DMP_DOS_WATCOM
        return (unsigned short)inpw(addr);
    #elif defined   DMP_DOS_BC
        return inpw(addr);
    #elif defined(USE_WINIO2) || defined(USE_WINIO3)
        unsigned long val;
	    GetPortVal(addr, &val, 2);
	    return (unsigned short)val;
    #elif defined   USE_PCIDEBUG
		return _IoReadShort((ULONG)addr);
    #elif defined   USE_PHYMEM
        return ReadPortWord(addr);
    #elif defined	DMP_WINCE_MSVC
        unsigned short val;

        _asm {
            mov dx, WORD PTR addr
            in ax, dx
            mov WORD PTR val, ax
        }
        return val;
    #elif defined   DMP_LINUX
        return inw(addr);
    #endif
}

__dmp_inline(void) portio_outpb(unsigned short addr, unsigned char val) {
    #if defined     DMP_DOS_DJGPP
        outportb(addr, val);
    #elif defined   DMP_DOS_WATCOM
        outp(addr, val);
    #elif defined   DMP_DOS_BC
        outp(addr, val);
    #elif defined(USE_WINIO2) || defined(USE_WINIO3)
	    SetPortVal(addr, val, 1);
    #elif defined   USE_PCIDEBUG
		_IoWriteChar((ULONG)addr, val);
    #elif defined   USE_PHYMEM
        WritePortByte(addr, val);
    #elif defined	DMP_WINCE_MSVC
        _asm {
            mov dx, WORD PTR addr
            mov al, val
            out dx, al
        }
    #elif defined   DMP_LINUX
        outb(val, addr);
    #endif
}

__dmp_inline(unsigned char) portio_inpb(unsigned short addr) {
    #if defined     DMP_DOS_DJGPP
        return inportb(addr);
    #elif defined   DMP_DOS_WATCOM
        return (unsigned char)inp(addr);
    #elif defined   DMP_DOS_BC
        return inp(addr);
    #elif defined(USE_WINIO2) || defined(USE_WINIO3)
        unsigned long val;
	    GetPortVal(addr, &val, 1);
	    return (unsigned char)val;
    #elif defined   USE_PCIDEBUG
		return _IoReadChar((ULONG)addr);
    #elif defined   USE_PHYMEM
        return ReadPortByte(addr);
    #elif defined	DMP_WINCE_MSVC
        unsigned char val;

        _asm {
            mov dx, WORD PTR addr
            in al, dx
            mov val, al
        }
        return val;
    #elif defined   DMP_LINUX
        return inb(addr);
    #endif
}


/* ========================================
 *   PCI CFG Access
 *   --------------------------------------
 *   these functions are all implemented as atomic operations
 * ========================================
 */
__dmp_inline(void) pci_outdw(unsigned long addr, unsigned long val) {  // assume that addr % 4 == 0
    #if defined     USE_PCIDEBUG
        _pciConfigWriteLong((addr & 0x0fffffffL) >> 8, addr & 0xfcL, val);
    #elif defined   USE_PHYMEM
        WritePCI((addr>>16) & 0xffL,  // bus no.
                 (addr>>11) & 0x1fL,  // dev no.
                 (addr>>8)  & 0x07L,  // fun no.
                 addr & 0xfcL,        // reg offset
                 4, &val);
    #else
        _io_DisableINT();
        {
            portio_outpdw(0x0cf8, addr); //portio_outpdw(0x0cf8, addr & 0xfffffffcL);
            portio_outpdw(0x0cfc, val);
        }
        _io_RestoreINT();
    #endif
}

__dmp_inline(unsigned long) pci_indw(unsigned long addr) {  // assume that addr % 4 == 0
    #if defined     USE_PCIDEBUG
        return _pciConfigReadLong((addr & 0x0fffffffL) >> 8, addr & 0xfcL);
    #elif defined   USE_PHYMEM
        unsigned long tmp;
        ReadPCI((addr>>16) & 0xffL,  // bus no.
                (addr>>11) & 0x1fL,  // dev no.
                (addr>>8)  & 0x07L,  // fun no.
                addr & 0xfcL,        // reg offset
                4, &tmp);
        return tmp;
    #else
        unsigned long tmp;

        _io_DisableINT();
        {
            portio_outpdw(0x0cf8, addr); //portio_outpdw(0x0cf8, addr & 0xfffffffcL);
            tmp = portio_inpdw(0x0cfc);
        }
        _io_RestoreINT();
        return tmp;
    #endif
}

__dmp_inline(void) pci_outw(unsigned long addr, unsigned short val) {  // assume that addr % 4 < 3
    #if defined(USE_PCIDEBUG) || defined(USE_PHYMEM)
        int i = (int)(addr & 0x03L) << 3; //* 8;

	    pci_outdw(addr, (pci_indw(addr) & ~(0xffffL << i)) | ((unsigned long)val << i));
    #else
        int i = (int)(addr & 0x03L) << 3; //* 8;

        _io_DisableINT();
        {
            portio_outpdw(0x0cf8, addr & 0xfffffffcL);
            portio_outpdw(0x0cfc, (portio_inpdw(0x0cfc) & ~(0xffffL << i)) | ((unsigned long)val << i));
        }
        _io_RestoreINT();
    #endif
}

__dmp_inline(unsigned short) pci_inw(unsigned long addr) {  // assume that addr % 4 < 3
    #if defined(USE_PCIDEBUG) || defined(USE_PHYMEM)
        int i = (int)(addr & 0x03L) << 3; //* 8;

        return (unsigned short)(pci_indw(addr) >> i);
    #else
        int i = (int)(addr & 0x03L) << 3; //* 8;
        unsigned short tmp;

        _io_DisableINT();
        {
            portio_outpdw(0x0cf8, addr & 0xfffffffcL);
            tmp = (unsigned short)(portio_inpdw(0x0cfc) >> i);
        }
        _io_RestoreINT();
        return tmp;
    #endif
}

__dmp_inline(void) pci_outb(unsigned long addr, unsigned char val) {
    #if defined(USE_PCIDEBUG) || defined(USE_PHYMEM)
        int i = (int)(addr & 0x03L) << 3; //* 8;

	    pci_outdw(addr, (pci_indw(addr) & ~(0xffL << i)) | ((unsigned long)val << i));
    #else
        int i = (int)(addr & 0x03L) << 3; //* 8;

        _io_DisableINT();
        {
            portio_outpdw(0x0cf8, addr & 0xfffffffcL);
            portio_outpdw(0x0cfc, (portio_inpdw(0x0cfc) & ~(0xffL << i)) | ((unsigned long)val << i));
        }
        _io_RestoreINT();
    #endif
}

__dmp_inline(unsigned char) pci_inb(unsigned long addr) {
    #if defined(USE_PCIDEBUG) || defined(USE_PHYMEM)
        int i = (int)(addr & 0x03L) << 3; //* 8;

        return (unsigned char)(pci_indw(addr) >> i);
    #else
        int i = (int)(addr & 0x03L) << 3; //* 8;
        unsigned char tmp;

        _io_DisableINT();
        {
            portio_outpdw(0x0cf8, addr & 0xfffffffcL);
            tmp = (unsigned char)(portio_inpdw(0x0cfc) >> i);
        }
        _io_RestoreINT();
        return tmp;
    #endif
}
/*------------------------  end. Primitive I/O Access  -----------------------*/
