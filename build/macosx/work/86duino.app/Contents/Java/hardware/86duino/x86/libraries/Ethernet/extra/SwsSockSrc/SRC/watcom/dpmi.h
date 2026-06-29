/*
 * DPMI support
 */
#pragma pack ( push, 0)
typedef union {
  struct {
    unsigned long edi;
    unsigned long esi;
    unsigned long ebp;
    unsigned long res;
    unsigned long ebx;
    unsigned long edx;
    unsigned long ecx;
    unsigned long eax;
  } d;
  struct {
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
  struct {
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
#pragma pack ( pop)

#pragma pack ( push, 0)
typedef struct {
  unsigned short offset16;
  unsigned short segment;
} __dpmi_raddr;
#pragma pack ( pop)

typedef void __interrupt __dpmi_rmcb( union INTPACK);

extern int	__dpmi_dos_alloc( unsigned short paras, unsigned long* segment, unsigned short* selector);
#pragma aux __dpmi_dos_alloc = \
  "mov eax, 0x100" \
  "int 0x31" \
  "jc error" \
  "mov [edi], eax" \
  "mov [esi], dx" \
  "sub eax, eax" \
  "error:" \
  parm [bx] [edi] [esi]\
  modify [dx] \
  value [eax]

extern int	__dpmi_dos_free( unsigned short selector);
#pragma aux __dpmi_dos_free = \
  "mov eax, 0x101" \
  "int 0x31" \
  "jc error" \
  "sub eax, eax" \
  "error:" \
  parm [dx] \
  value [eax]

extern int	__dpmi_int(int, __dpmi_regs *);
#pragma aux __dpmi_int = \
  "sub bh, bh" \
  "sub ecx, ecx" \
  "mov eax, 0x300" \
  "int 0x31" \
  "jc error" \
  "sub eax, eax" \
  "error:" \
  parm [bl] [edi] \
  modify [bh ecx] \
  value [eax]

extern int	__dpmi_rmcb_alloc( __dpmi_rmcb *, __dpmi_regs *, __dpmi_raddr *);
#pragma aux __dpmi_rmcb_alloc = \
  "push ds" \
  "mov ax, cs" \
  "mov ds, ax" \
  "mov eax, 0x303" \
  "int 0x31" \
  "pop ds" \
  "jc error" \
  "mov [ebx], dx" \
  "mov [ebx+2], cx" \
  "sub eax, eax" \
  "error:" \
  parm [esi] [edi] [ebx]\
  modify [cx dx] \
  value [eax]

extern int	__dpmi_rmcb_free( __dpmi_raddr *);
#pragma aux __dpmi_rmcb_free = \
  "mov cx, [ebx]" \
  "mov dx, [ebx+2]" \
  "mov eax, 0x304" \
  "int 0x31" \
  "jc error" \
  "sub eax, eax" \
  "error:" \
  parm [ebx]\
  modify [cx dx] \
  value [eax]

extern int __dpmi_disable( void);
#pragma aux __dpmi_disable = \
  "mov eax, 0x900"\
  "int 0x31" \
  "and eax, 1" \
   value [eax]

extern int __dpmi_enable( void);
#pragma aux __dpmi_enable = \
  "mov eax, 0x901"\
  "int 0x31" \
  "and eax, 1" \
   value [eax]

__inline void __dpmi_yield( void) {
  __dpmi_regs r;
  r.x.ax = 0x1680;
  r.x.ss = r.x.sp = r.x.flags = 0;
  __dpmi_int( 0x2f, &r);
}

extern int	__dpmi_lock( unsigned long addr, unsigned long len);
#pragma aux __dpmi_lock = \
  "mov ecx, ebx" \
  "and ecx, 0xffff" \
  "shr ebx, 16" \
  "mov edi, esi" \
  "and edi, 0xffff" \
  "shr esi, 16" \
  "mov eax, 0x600" \
  "int 0x31" \
  "jc error" \
  "sub eax, eax" \
  "error:" \
  parm [ebx] [esi]\
  modify [ecx edi] \
  value [eax]

extern int	__dpmi_unlock( unsigned long addr, unsigned long len);
#pragma aux __dpmi_unlock = \
  "mov ecx, ebx" \
  "and ecx, 0xffff" \
  "shr ebx, 16" \
  "mov edi, esi" \
  "and edi, 0xffff" \
  "shr esi, 16" \
  "mov eax, 0x601" \
  "int 0x31" \
  "jc error" \
  "sub eax, eax" \
  "error:" \
  parm [ebx] [esi]\
  modify [ecx edi] \
  value [eax]

#define dosmemget( offset, len, buffer) \
  memcpy( buffer, (const void*)(offset), len)

#define dosmemput( buffer, len, offset) \
  memcpy( (void*)(offset), buffer, len)
