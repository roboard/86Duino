/***************************  Primitive MMIO Access  **************************/
__dmp_inline(void) mmio_outpdw(IO_BASE_t* base, unsigned long offset, unsigned long val) {
    #if   defined   DMP_DOS_DJGPP
        _farpokel((unsigned short)base->mmio_selector, offset, val);
    #elif defined   USE_PCIDEBUG
        _MemWriteLong(base->addr + offset, val);
    #elif defined(DMP_DOS_WATCOM) || defined(USE_WINIO3) || defined(USE_PHYMEM)
        *(volatile unsigned long*)(base->addr + offset) = val;
    #endif
}
    
__dmp_inline(unsigned long) mmio_inpdw(IO_BASE_t* base, unsigned long offset) {
    #if   defined   DMP_DOS_DJGPP
        return _farpeekl((unsigned short)base->mmio_selector, offset);
    #elif defined   USE_PCIDEBUG
        return _MemReadLong(base->addr + offset);
    #elif defined(DMP_DOS_WATCOM) || defined(USE_WINIO3) || defined(USE_PHYMEM)
        return *(volatile unsigned long*)(base->addr + offset);
    #else
        return 0L;
    #endif
}

__dmp_inline(void) mmio_outpw(IO_BASE_t* base, unsigned long offset, unsigned short val) {
    #if   defined   DMP_DOS_DJGPP
        _farpokew((unsigned short)base->mmio_selector, offset, val);
    #elif defined   USE_PCIDEBUG
        _MemWriteShort(base->addr + offset, val);
    #elif defined(DMP_DOS_WATCOM) || defined(USE_WINIO3) || defined(USE_PHYMEM)
        *(volatile unsigned short*)(base->addr + offset) = val;
    #endif
}

__dmp_inline(unsigned short) mmio_inpw(IO_BASE_t* base, unsigned long offset) {
    #if   defined   DMP_DOS_DJGPP
        return _farpeekw((unsigned short)base->mmio_selector, offset);
    #elif defined   USE_PCIDEBUG
        return _MemReadShort(base->addr + offset);
    #elif defined(DMP_DOS_WATCOM) || defined(USE_WINIO3) || defined(USE_PHYMEM)
        return *(volatile unsigned short*)(base->addr + offset);
    #else
        return 0;
    #endif
}

__dmp_inline(void) mmio_outpb(IO_BASE_t* base, unsigned long offset, unsigned char val) {
    #if   defined   DMP_DOS_DJGPP
        _farpokeb((unsigned short)base->mmio_selector, offset, val);
    #elif defined   USE_PCIDEBUG
        _MemWriteChar(base->addr + offset, val);
    #elif defined(DMP_DOS_WATCOM) || defined(USE_WINIO3) || defined(USE_PHYMEM)
        *(volatile unsigned char*)(base->addr + offset) = val;
    #endif
}

__dmp_inline(unsigned char) mmio_inpb(IO_BASE_t* base, unsigned long offset) {
    #if   defined   DMP_DOS_DJGPP
        return _farpeekb((unsigned short)base->mmio_selector, offset);
    #elif defined   USE_PCIDEBUG
        return _MemReadChar(base->addr + offset);
    #elif defined(DMP_DOS_WATCOM) || defined(USE_WINIO3) || defined(USE_PHYMEM)
        return *(volatile unsigned char*)(base->addr + offset);
    #else
        return 0;
    #endif
}
/*-----------------------  end. Primitive MMIO Access  -----------------------*/
