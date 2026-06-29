/******************************  DPMI Functions  ******************************/
#if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM)

DMPAPI(unsigned long) dpmi_LinMapAlloc(unsigned long phyaddr, unsigned long size) {
    __dpmi_meminfo meminfo;

    if (phyaddr < 0x100000L) return phyaddr;  // only needs to map non-DOS memory (DPMI host has already mapped DOS memory below 1Mb)

    meminfo.address = phyaddr;
    meminfo.size    = size;
    if (__dpmi_physical_address_mapping(&meminfo) != 0) return 0x00000000L;

    __dpmi_lock_linear_region(&meminfo);      // avoid virtual memory paging
    return meminfo.address;
}

DMPAPI(void) dpmi_LinMapFree(unsigned long linaddr) {
    __dpmi_meminfo meminfo;

    if (linaddr >= 0x100000L)
    {
        meminfo.address = linaddr;
        //if (__dpmi_free_physical_address_mapping(&meminfo) != 0) return false;  // not work on some DPMI servers
        __dpmi_free_physical_address_mapping(&meminfo);
    }
}

DMPAPI(int) dpmi_SelAlloc(unsigned long linaddr, unsigned long size) {
    int selector;

    if ((selector = __dpmi_allocate_ldt_descriptors(1)) < 0) return -1;

    if (__dpmi_set_segment_base_address(selector, linaddr) == 0) //&&
    if (__dpmi_set_segment_limit(selector, size - 1) == 0)
    //if (__dpmi_set_segment_limit(selector, ((size-1) >= 0xffff)? size-1 : 0xffff) != 0)
        return selector;

    __dpmi_free_ldt_descriptor(selector);
    return -1;
}

DMPAPI(bool) dpmi_SelFree(int selector) {
    if (selector < 0) return true;

    if (__dpmi_free_ldt_descriptor(selector) != 0) return false;
    return true;
}

DMPAPI(bool) dpmi_LockCode(void* code_addr, unsigned long code_size) {
    if (_go32_dpmi_lock_code(code_addr, code_size) != 0) return false;
    return true;
}

DMPAPI(bool) dpmi_LockData(void* data_addr, unsigned long data_size) {
    if (_go32_dpmi_lock_data(data_addr, data_size) != 0) return false;
    return true;
}

DMPAPI(bool) dpmi_UnlockCode(void* code_addr, unsigned long code_size) {
    unsigned long  code_start;
    __dpmi_meminfo code_info;
    
    if (__dpmi_get_segment_base_address(_go32_my_cs(), &code_start) != 0) return false;
    code_info.address = code_start + (unsigned long) code_addr;
    code_info.size    = code_size;
    
    if (__dpmi_unlock_linear_region(&code_info) != 0) return false;
    return true;
}

DMPAPI(bool) dpmi_UnlockData(void* data_addr, unsigned long data_size) {
    unsigned long  data_start;
    __dpmi_meminfo data_info;
    
    if (__dpmi_get_segment_base_address(_go32_my_ds(), &data_start) != 0) return false;
    data_info.address = data_start + (unsigned long) data_addr;
    data_info.size    = data_size;
    
    if (__dpmi_unlock_linear_region(&data_info) != 0) return false;
    return true;
}
#endif
/*--------------------------  end. DPMI Functions  ---------------------------*/
