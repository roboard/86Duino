/*******************************************************************************

  io_dpmi.h - Part of DM&P Vortex86 Base I/O Library
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

DMPAPI(int) dpmi_Cli(void) {
    #if defined(DMP_DOS_DJGPP)
        //int result = disable();
        int result = __dpmi_get_and_disable_virtual_interrupt_state();
        if (result == 0) return 0; else return 1;  // return the previous interrupt flag
    #else
        union  REGS  regs;

        memset(&regs,  0, sizeof(regs));
        regs.w.ax  = 0x0900;
        int386(0x31, &regs, &regs);

        if (regs.h.al == 0) return 0; else return 1;
    #endif
}

DMPAPI(int) dpmi_Sti(void) {
    #if defined(DMP_DOS_DJGPP)
        //int result = enable();
        int result = __dpmi_get_and_enable_virtual_interrupt_state();
        if (result == 0) return 0; else return 1;  // return the previous interrupt flag
    #else
        union  REGS  regs;

        memset(&regs,  0, sizeof(regs));
        regs.w.ax  = 0x0901;
        int386(0x31, &regs, &regs);

        if (regs.h.al == 0) return 0; else return 1;
    #endif
}
#endif
/*--------------------------  end. DPMI Functions  ---------------------------*/
