/*******************************************************************************

  io_wat.h - Part of DM&P Vortex86 Base I/O Library
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


/***************************  Watcom DPMI Interface  **************************/
#ifdef DMP_DOS_WATCOM

#pragma pack(push, 1)
    typedef union 
    {
        struct
        {
            unsigned long edi;
            unsigned long esi;
            unsigned long ebp;
            unsigned long res;
            unsigned long ebx;
            unsigned long edx;
            unsigned long ecx;
            unsigned long eax;
        } d;

        struct 
        {
            unsigned short di, di_hi;
            unsigned short si, si_hi;
            unsigned short bp, bp_hi;
            unsigned short res, res_hi;
            unsigned short bx, bx_hi;
            unsigned short dx, dx_hi;
            unsigned short cx, cx_hi;
            unsigned short ax, ax_hi;
            unsigned short flags;
            unsigned short es;
            unsigned short ds;
            unsigned short fs;
            unsigned short gs;
            unsigned short ip;
            unsigned short cs;
            unsigned short sp;
            unsigned short ss;
        } x;

        struct
        {
            unsigned char edi[4];
            unsigned char esi[4];
            unsigned char ebp[4];
            unsigned char res[4];
            unsigned char bl, bh, ebx_b2, ebx_b3;
            unsigned char dl, dh, edx_b2, edx_b3;
            unsigned char cl, ch, ecx_b2, ecx_b3;
            unsigned char al, ah, eax_b2, eax_b3;
        } h;

    } __dpmi_regs;

    typedef struct
    {
        unsigned long handle;
        unsigned long size;
        unsigned long address;

    } __dpmi_meminfo;
#pragma pack(pop)

static int __dpmi_int(int _vector, __dpmi_regs *_regs) {
    union  REGS  regs;
    struct SREGS sregs;

    memset(&regs,  0, sizeof(regs));
    memset(&sregs, 0, sizeof(sregs));

    _regs->x.flags = 0;
    _regs->x.sp = 0;
    _regs->x.ss = 0;

    regs.w.ax  = 0x300;
    regs.h.bl  = _vector;
    regs.h.bh  = 0x00;
    regs.w.cx  = 0x0000;
    sregs.es   = FP_SEG(_regs);
    regs.x.edi = FP_OFF(_regs);
    int386x(0x31, &regs, &regs, &sregs);
    
    if ((regs.x.cflag & 0x01) == 0) return 0;
    return -1;
}

static int __dpmi_physical_address_mapping(__dpmi_meminfo *info) {
    union  REGS  regs;

    memset(&regs,  0, sizeof(regs));

    regs.w.ax  = 0x800;
    regs.w.bx  = (unsigned short)(info->address >> 16);
    regs.w.cx  = (unsigned short)(info->address & 0xffffL);
    regs.w.si  = (unsigned short)(info->size >> 16);
    regs.w.di  = (unsigned short)(info->size & 0xffffL);
    int386(0x31, &regs, &regs);

    if ((regs.x.cflag & 0x01) == 0)
    {
        info->address = ((unsigned long)regs.w.bx << 16) + (unsigned long)regs.w.cx;
        return 0;
    }
    return -1;
}

static int __dpmi_free_physical_address_mapping(__dpmi_meminfo *info) {
    union  REGS  regs;

    memset(&regs,  0, sizeof(regs));

    regs.w.ax  = 0x801;
    regs.w.bx  = (unsigned short)(info->address >> 16);
    regs.w.cx  = (unsigned short)(info->address & 0xffffL);
    int386(0x31, &regs, &regs);

    if ((regs.x.cflag & 0x01) == 0) return 0;
    return -1;
}

static int __dpmi_lock_linear_region(__dpmi_meminfo *info) {
    union  REGS  regs;

    memset(&regs,  0, sizeof(regs));

    regs.w.ax  = 0x600;
    regs.w.bx  = (unsigned short)(info->address >> 16);
    regs.w.cx  = (unsigned short)(info->address & 0xffffL);
    regs.w.si  = (unsigned short)(info->size >> 16);
    regs.w.di  = (unsigned short)(info->size & 0xffffL);
    int386(0x31, &regs, &regs);

    if ((regs.x.cflag & 0x01) == 0) return 0;
    return -1;
}

static int __dpmi_unlock_linear_region(__dpmi_meminfo *info) {
    union  REGS  regs;

    memset(&regs,  0, sizeof(regs));

    regs.w.ax  = 0x601;
    regs.w.bx  = (unsigned short)(info->address >> 16);
    regs.w.cx  = (unsigned short)(info->address & 0xffffL);
    regs.w.si  = (unsigned short)(info->size >> 16);
    regs.w.di  = (unsigned short)(info->size & 0xffffL);
    int386(0x31, &regs, &regs);

    if ((regs.x.cflag & 0x01) == 0) return 0;
    return -1;
}

static int __dpmi_get_segment_base_address(int selector, unsigned long *addr)
{
    union  REGS  regs;

    memset(&regs,  0, sizeof(regs));

    regs.w.ax  = 0x006;
    regs.w.bx  = selector;
    int386(0x31, &regs, &regs);

    if ((regs.x.cflag & 0x01) == 0)
    {
        *addr = ((unsigned long)regs.w.cx << 16) + ((unsigned long)regs.w.dx & 0xffff);
        return 0;
    }
    return -1;
}

static int _go32_dpmi_lock_data(void *lockaddr, unsigned long locksize) {
    unsigned long  addr;
    __dpmi_meminfo info;
    int            my_ds = 0;

    _asm { mov WORD PTR my_ds, ds };
    if (__dpmi_get_segment_base_address(my_ds, &addr) != 0) return -1;

    info.size = locksize;
    info.address = addr + (unsigned long)lockaddr;
    return __dpmi_lock_linear_region(&info);
}

static int _go32_dpmi_lock_code(void *lockaddr, unsigned long locksize)
{
    unsigned long  addr;
    __dpmi_meminfo info;
    int            my_cs = 0;

    _asm { mov WORD PTR my_cs, cs };
    if (__dpmi_get_segment_base_address(my_cs, &addr) != 0) return -1;

    info.size = locksize;
    info.address = addr + (unsigned long)lockaddr;
    return __dpmi_lock_linear_region(&info);
}

static int __dpmi_allocate_ldt_descriptors(int count) {
    union  REGS  regs;

    memset(&regs,  0, sizeof(regs));

    regs.w.ax  = 0x0000;
    regs.w.cx  = count;
    int386(0x31, &regs, &regs);

    if ((regs.x.cflag & 0x01) == 0) return (int)regs.w.ax & 0xffff;
    return -1;
}

static int __dpmi_free_ldt_descriptor(int descriptor) {
    union  REGS  regs;

    memset(&regs,  0, sizeof(regs));

    regs.w.ax  = 0x0001;
    regs.w.bx  = descriptor;
    int386(0x31, &regs, &regs);

    if ((regs.x.cflag & 0x01) == 0) return 0;
    return -1;
}

static int __dpmi_set_segment_base_address(int _selector, unsigned long _address) {
    union  REGS  regs;

    memset(&regs,  0, sizeof(regs));

    regs.w.ax  = 0x0007;
    regs.w.bx  = _selector;
    regs.w.cx  =  (unsigned short)(_address >> 16);
    regs.w.dx  =  (unsigned short)(_address & 0xffffL);
    int386(0x31, &regs, &regs);

    if ((regs.x.cflag & 0x01) == 0) return 0;
    return -1;
}

static int __dpmi_set_segment_limit(int _selector, unsigned long _limit) {
    union  REGS  regs;

    memset(&regs,  0, sizeof(regs));

    regs.w.ax  = 0x0008;
    regs.w.bx  = _selector;
    regs.w.cx  =  (unsigned short)(_limit >> 16);
    regs.w.dx  =  (unsigned short)(_limit & 0xffffL);
    int386(0x31, &regs, &regs);

    if ((regs.x.cflag & 0x01) == 0) return 0;
    return -1;
}

static int __dpmi_allocate_dos_memory(int _paragraphs, int *_ret_selector) {
    union  REGS  regs;

    memset(&regs,  0, sizeof(regs));

    regs.w.ax  = 0x100;
    regs.w.bx  = _paragraphs;
    int386(0x31, &regs, &regs);

    if ((regs.x.cflag & 0x01) == 0)
    {
        *_ret_selector = (int)regs.w.dx & 0xffff;
        return (int)regs.w.ax & 0xffff;
    }
    return -1;
}

static int __dpmi_free_dos_memory(int _selector) {
    union  REGS  regs;

    memset(&regs,  0, sizeof(regs));

    regs.w.ax  = 0x101;
    regs.w.dx  = _selector;
    int386(0x31, &regs, &regs);
    
    if ((regs.x.cflag & 0x01) == 0) return 0;
    return -1;
}


static void dosmemput(void *buffer, int length, int offset) {
    memcpy((void*)offset, buffer, length);
}

static void dosmemget(int offset, int length, void *buffer) {
    memcpy(buffer, (void*)offset, length);
}


#define WAT_TB_SIZE (8192)
static unsigned long __tb;
static int __tb_sel = -1;

static bool setup_tb(void) {
    int tmp;
    
    if (__tb_sel == -1)
    {
        if ((tmp = __dpmi_allocate_dos_memory((WAT_TB_SIZE+15)>>4, &__tb_sel)) == -1) return false;

        __tb = (unsigned long)(tmp << 4);
    }
    
    return true;
}

static void free_tb(void) {
    if (__tb_sel == -1) return;

    __dpmi_free_dos_memory(__tb_sel);
    __tb_sel = -1;
}

static unsigned short _go32_my_cs(void) {
    unsigned short my_cs = 0;

    _asm { mov WORD PTR my_cs, cs };
    return my_cs;
}

static unsigned short _go32_my_ds(void) {
    unsigned short my_ds = 0;

    _asm { mov WORD PTR my_ds, ds };
    return my_ds;
}
#endif
/*-------------  end. Watcom DPMI Interface (for internal use)  --------------*/
