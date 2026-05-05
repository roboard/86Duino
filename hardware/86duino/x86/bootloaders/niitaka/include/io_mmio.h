/*******************************************************************************

  io_mmio.h - Part of DM&P Vortex86 Base I/O Library
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


/***************************  Primitive MMIO Access  **************************/
__dmp_inline(void) mmio_outpdw(IO_BASE_t* base, unsigned long offset, unsigned long val) {
    #if   defined   DMP_DOS_DJGPP
        _farpokel((unsigned short)base->mmio_selector, offset, val);
    #elif defined   USE_PCIDEBUG
        _MemWriteLong(base->addr + offset, val);
    #elif defined(DMP_DOS_WATCOM) || defined(USE_WINIO3) || defined(USE_PHYMEM) || defined(DMP_LINUX)
        *(volatile unsigned long*)(base->addr + offset) = val;
        ATOMIC_MEM_BARRIER();        
    #endif
}
    
__dmp_inline(unsigned long) mmio_inpdw(IO_BASE_t* base, unsigned long offset) {
    #if   defined   DMP_DOS_DJGPP
        return _farpeekl((unsigned short)base->mmio_selector, offset);
    #elif defined   USE_PCIDEBUG
        return _MemReadLong(base->addr + offset);
    #elif defined(DMP_DOS_WATCOM) || defined(USE_WINIO3) || defined(USE_PHYMEM) || defined(DMP_LINUX)
        unsigned long result = *(volatile unsigned long*)(base->addr + offset); 
        ATOMIC_MEM_BARRIER();
        return result;
    #else
        return 0L;
    #endif
}

__dmp_inline(void) mmio_outpw(IO_BASE_t* base, unsigned long offset, unsigned short val) {
    #if   defined   DMP_DOS_DJGPP
        _farpokew((unsigned short)base->mmio_selector, offset, val);
    #elif defined   USE_PCIDEBUG
        _MemWriteShort(base->addr + offset, val);
    #elif defined(DMP_DOS_WATCOM) || defined(USE_WINIO3) || defined(USE_PHYMEM) || defined(DMP_LINUX)
        *(volatile unsigned short*)(base->addr + offset) = val;
        ATOMIC_MEM_BARRIER();        
    #endif
}

__dmp_inline(unsigned short) mmio_inpw(IO_BASE_t* base, unsigned long offset) {
    #if   defined   DMP_DOS_DJGPP
        return _farpeekw((unsigned short)base->mmio_selector, offset);
    #elif defined   USE_PCIDEBUG
        return _MemReadShort(base->addr + offset);
    #elif defined(DMP_DOS_WATCOM) || defined(USE_WINIO3) || defined(USE_PHYMEM) || defined(DMP_LINUX)
        unsigned short result = *(volatile unsigned short*)(base->addr + offset);
        ATOMIC_MEM_BARRIER();
        return result;
    #else
        return 0;
    #endif
}

__dmp_inline(void) mmio_outpb(IO_BASE_t* base, unsigned long offset, unsigned char val) {
    #if   defined   DMP_DOS_DJGPP
        _farpokeb((unsigned short)base->mmio_selector, offset, val);
    #elif defined   USE_PCIDEBUG
        _MemWriteChar(base->addr + offset, val);
    #elif defined(DMP_DOS_WATCOM) || defined(USE_WINIO3) || defined(USE_PHYMEM) || defined(DMP_LINUX)
        *(volatile unsigned char*)(base->addr + offset) = val;
        ATOMIC_MEM_BARRIER();        
    #endif
}

__dmp_inline(unsigned char) mmio_inpb(IO_BASE_t* base, unsigned long offset) {
    #if   defined   DMP_DOS_DJGPP
        return _farpeekb((unsigned short)base->mmio_selector, offset);
    #elif defined   USE_PCIDEBUG
        return _MemReadChar(base->addr + offset);
    #elif defined(DMP_DOS_WATCOM) || defined(USE_WINIO3) || defined(USE_PHYMEM) || defined(DMP_LINUX)
        unsigned char result = *(volatile unsigned char*)(base->addr + offset);
        ATOMIC_MEM_BARRIER();
        return result;
    #else
        return 0;
    #endif
}
/*-----------------------  end. Primitive MMIO Access  -----------------------*/
