#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <go32.h>
#include <dpmi.h>
#include <sys/farptr.h>
#include <sys/time.h>
#include <pc.h>
#include <dos.h>
#include <sys/nearptr.h>
#include <conio.h>

#ifdef HANDLE_PRAGMA_PACK_PUSH_POP
    #undef HANDLE_PRAGMA_PACK_PUSH_POP
#endif

#define HANDLE_PRAGMA_PACK_PUSH_POP 1  // enable "#pragma pack" in DJGPP

#pragma pack(push, 1)  // avoid to optimize memory alignment of "struct" members

#pragma pack(pop)

typedef struct PCI_config
{
	unsigned short int bus;
	unsigned char device;
	unsigned char function;
	int A9160;
}PCI_config;

PCI_config configs;
unsigned long lfb_linaddr;
int           lfb_selector;
int           mmio_selector;
unsigned long mmio_linaddr;

static void io_outpdw(unsigned addr, unsigned long val) {
    outportl(addr, val);
}

//inport: double word
static unsigned long io_inpdw(unsigned addr) {
    return inportl(addr);
}

static int IO_edIrqCnt = 0;

static void io_EnableIRQ(void) {
    if (IO_edIrqCnt > 0)  IO_edIrqCnt--;
    if (IO_edIrqCnt == 0) enable();
}

static void io_DisableIRQ(void) {
    if (IO_edIrqCnt == 0) disable();
    IO_edIrqCnt++;
}

static unsigned long dpmi_LinMapAlloc(unsigned long phyaddr, unsigned long size) {
        __dpmi_meminfo meminfo;

        if (phyaddr < 0x100000L) return phyaddr;  // only needs to map non-DOS memory (DPMI host has already mapped DOS memory below 1Mb)
		
        meminfo.address = phyaddr;
        meminfo.size    = size;
        if (__dpmi_physical_address_mapping(&meminfo) != 0) return 0x00000000L;
        __dpmi_lock_linear_region(&meminfo);      // avoid virtual memory paging
        return meminfo.address;
}

static void dpmi_LinMapFree(unsigned long linaddr) {
        __dpmi_meminfo meminfo;

        if (linaddr >= 0x100000L)
        {
            meminfo.address = linaddr;
            //if (__dpmi_free_physical_address_mapping(&meminfo) != 0) return 0;  // not work on some DPMI servers
            __dpmi_free_physical_address_mapping(&meminfo);
        }
}

static int dpmi_SelAlloc(unsigned long linaddr, unsigned long size) {
        int selector;
    
        if ((selector = __dpmi_allocate_ldt_descriptors(1)) < 0) return -1;
    
        if (__dpmi_set_segment_base_address(selector, linaddr) == 0) //&&
        if (__dpmi_set_segment_limit(selector, size - 1) == 0)
        //if (__dpmi_set_segment_limit(selector, ((size-1) >= 0xffff)? size-1 : 0xffff) != 0)
            return selector;

        __dpmi_free_ldt_descriptor(selector);
        return -1;
}

static int dpmi_SelFree(int selector) {
        if (selector < 0) return 1;

        if (__dpmi_free_ldt_descriptor(selector) != 0) return 0;
        return 1;
}

static unsigned long int read_pci_reg(unsigned long bus, unsigned long device, unsigned long function, unsigned long offset) {
    unsigned long tmp;

    io_DisableIRQ();
	io_outpdw(0x0cf8, (0x80000000L+((bus & 0xFFL) << 16) + ((device & 0x1FL) << 11) + ((function & 0x07L) << 8) + ((offset & 0x3FL) << 0)) & 0xfffffffcL);
    tmp = 0xffffffff & io_inpdw(0x0cfc);
	io_EnableIRQ();
	
	return tmp;
}

static void write_pci_reg(unsigned long bus, unsigned long device, unsigned long function, unsigned long offset, unsigned long value)
{
	io_DisableIRQ();
	io_outpdw(0x0cf8, (0x80000000L+((bus & 0xFFL) << 16) + ((device & 0x1FL) << 11) + ((function & 0x07L) << 8) + ((offset & 0x3FL) << 0)) & 0xfffffffcL);
	io_outpdw(0x0cfc, value);
	io_EnableIRQ();
}

static unsigned long int pci_get_mmioaddress()
{
	return read_pci_reg(configs.bus, configs.device, configs.function, 0x14);
}

static unsigned long int pci_get_fbbaseaddress()
{
	return read_pci_reg(configs.bus, configs.device, configs.function, 0x10);
}

static unsigned long int pci_get_mmiosize()
{
	unsigned long tmp;
	unsigned long addr = read_pci_reg(configs.bus, configs.device, configs.function, 0x14);
	write_pci_reg(configs.bus, configs.device, configs.function, 0x14, 0xFFFFFFFF);
	tmp = read_pci_reg(configs.bus, configs.device, configs.function, 0x14);
	write_pci_reg(configs.bus, configs.device, configs.function, 0x14, addr);
	return (~tmp)+1;
}

static unsigned long int pci_get_fbsize()
{
	unsigned long tmp;
	unsigned long addr = read_pci_reg(configs.bus, configs.device, configs.function, 0x10);
	
	write_pci_reg(configs.bus, configs.device, configs.function, 0x10, 0xFFFFFFFF);
	tmp = read_pci_reg(configs.bus, configs.device, configs.function, 0x10);
	write_pci_reg(configs.bus, configs.device, configs.function, 0x10, addr);
	return (~tmp)+1;
}

static int pci_GetMMIOAddr(unsigned long* mmioaddr, unsigned long* mmiosize) {
	*mmioaddr = pci_get_mmioaddress();
	*mmiosize = pci_get_mmiosize();
	return 1;
}

typedef struct
{
    unsigned char  vesaSignature[4];        // VBE signature
    unsigned short vesaVersion;             // VBE version
    unsigned long  oemStringPtr;            // pointer to OEM string
    unsigned long  capabilities;            // capabilities of graphics controller:
                                            //     b0  = 0  DAC is fixed 6 bits per primary color
                                            //         = 1  DAC width is switchable to 8 bits per primary color
                                            //     b1  = 0  Controller is VGA compatible
                                            //         = 1  Controller is not VGA compatible
                                            //     b2  = 0  Normal DAC operation
                                            //         = 1  Must programming DAC with blank bit (i.e., during blank period only; see also 4F09h)
                                            //     b3-31 = Reserved

    unsigned long  videoModePtr;            // pointer to video mode list (terminated with 0FFFFh).

    unsigned short totalMemory;             // VRAM size in 64KB

    unsigned short oemSoftwareRev;          // OEM software revision
    unsigned long  oemVendorNamePtr;        // pointer to vendor name string
    unsigned long  oemProductNamePtr;       // pointer to product name string
    unsigned long  oemProductRevPtr;        // pointer to product revision string
    
    unsigned char  reserved[222];           // reserved for VBE implementation scratch area
    unsigned char  oemData[256];            // data area for OEM strings

} VESAINFO_t;  // general information of VESA BIOS

typedef struct
{
    unsigned short modeAttributes;          // Video mode attributes:
                                            //     b0  =  check whether the mode is supported
                                            //            0 = mode not supported
                                            //            1 = mode supported
                                            //     b1  =  1 (Reserved)
                                            //     b2  =  check whether BIOS output functions are supported
                                            //            0 = BIOS output functions not supported
                                            //            1 = BIOS output functions supported
                                            //     b3  =  monochrome/color mode
                                            //            0 = Monochrome mode
                                            //            1 = Color mode
                                            //     b4  =  mode type
                                            //            0 = text mode
                                            //            1 = graphics mode
                                            //     b5  =  check whether the mode is VGA compatible
                                            //            0 = yes
                                            //            1 = no
                                            //     b6  =  check whether window switching (bank switching) is avaiable
                                            //            0 = yes
                                            //            1 = no
                                            //     b7  =  check whether linear frame buffer mode is avaiable
                                            //            0 = no
                                            //            1 = yes
                                            //     b8  =  (VBE3.0 only) check whether double scan mode is supported
                                            //            0 = no
                                            //            1 = yes
                                            //     b9  =  (VBE3.0 only) check whether interlaced mode is supported
                                            //            0 = no
                                            //            1 = yes
                                            //     b10 =  (VBE3.0 only) check whether H/W triple buffering is supported
                                            //            0 = no
                                            //            1 = yes
                                            //     b11 =  (VBE3.0 only) check whether H/W stereoscopic display is supported
                                            //            0 = no
                                            //            1 = yes
                                            //     b12 =  (VBE3.0 only) check whether dual display start address is supported
                                            //            0 = no
                                            //            1 = yes
                                            //     b13-15 = reserved

    unsigned char  winAAttributes;          // window A/B attributes:
    unsigned char  winBAttributes;          //     b0  =  relocatable window(s) supported
                                            //            0 = fixed window only
                                            //            1 = relocatable window(s) are supported
                                            //     b1  =  window readable
                                            //            0 = window is not readable
                                            //            1 = window is readable
                                            //     b2  =  window writeable
                                            //            0 = window is not writeable
                                            //            1 = window is writeable
                                            //     b3-7 = reserved
    unsigned short winGranularity;          // window granularity (the unit for windows positioning) in KB
    unsigned short winSize;                 // window size in KB
    unsigned short winASegment;             // window A start segment address (real-mode format)
    unsigned short winBSegment;             // window B start segment address (real-mode format)
    unsigned long  winFuncPtr;              // far function pointer (real-mode format) to window positioning function (equivalent to AX=4F05h)

    unsigned short bytesPerScanLine;        // bytes per scanline
    unsigned short xResolution;             // horizontal resolution in pixel or chars
    unsigned short yResolution;             // vertical resolution in pixel or chars
    unsigned char  xCharSize;               // character cell width in pixel
    unsigned char  yCharSize;               // character cell height in pixel
    unsigned char  numberOfPlanes;          // number of memory planes
    unsigned char  bitsPerPixel;            // bits per pixel
    unsigned char  numberOfBanks;           // number of banks
    unsigned char  memoryModel;             // memory model type:
                                            //     00h  =  Text
                                            //     01h  =  CGA
                                            //     02h  =  Hercules
                                            //     03h  =  EGA 16 color
                                            //     04h  =  Packed pixels
                                            //     05h  =  Non chain 4 256 color modes
                                            //     06h  =  Direct 15/16/24 bit
                                            //     07h  =  YUV (luminance-chrominance, alos called YIQ)
    unsigned char  BankSize;                // memory planes size in KB
    unsigned char  numberOfImagePages;      // number of image pages
    unsigned char  reserved_page;           // reserved for page function

    unsigned char  redMaskSize;             // size of red mask in bits
    unsigned char  redMaskPos;              // LSB bit position of red mask
    unsigned char  greenMaskSize;           // size of green mask in bits
    unsigned char  greenMaskPos;            // LSB bit position of green mask
    unsigned char  blueMaskSize;            // size of blue mask in bits
    unsigned char  blueMaskPos;             // LSB bit position of blue mask
    unsigned char  reservedMaskSize;        // size of reserved mask in bits
    unsigned char  reservedMaskPos;         // LSB bit position of reserved mask
    unsigned char  directColorModeInfo;     // direct color mode attributes:
                                            //     b0  =  color ramp mode
                                            //            0 = the color ramp is fixed
                                            //            1 = the color ramp is programmable
                                            //     b1  =  reserved field mode
                                            //            0 = the reserved mask field must be reserved
                                            //            1 = the reserved mask field can be used
                                            //     b2-7 = Reserved

    unsigned long  physBasePtr;             // (VBE2.0 only) physical address for flat frame buffer
    unsigned long  offScreenMemOffset;      // (VBE2.0 only) pointer to the start of off screen memory
    unsigned short offScreenMemSize;        // (VBE2.0 only) size of off screen memory in KB

    unsigned short linBytesPerScanLine;     // (VBE3.0 only) bytes per scanline in linear framebuffer mode
    unsigned char  bnkNumberOfImagePages;   // (VBE3.0 only) number of image pages in banked memory mode
    unsigned char  linNumberOfImagePages;   // (VBE3.0 only) number of image pages in linear framebuffer mode

    unsigned char  linRedMaskSize;          // (VBE3.0 only) size of red mask in bits in linear framebuffer mode
    unsigned char  linRedMaskPos;           // (VBE3.0 only) LSB bit position of red mask in linear framebuffer mode
    unsigned char  linGreenMaskSize;        // (VBE3.0 only) size of green mask in bits in linear framebuffer mode
    unsigned char  linGreenMaskPos;         // (VBE3.0 only) LSB bit position of green mask in linear framebuffer mode
    unsigned char  linBlueMaskSize;         // (VBE3.0 only) size of blue mask in bits in linear framebuffer mode
    unsigned char  linBlueMaskPos;          // (VBE3.0 only) LSB bit position of blue mask in linear framebuffer mode
    unsigned char  linReservedMaskSize;     // (VBE3.0 only) size of reserved mask in bits in linear framebuffer mode
    unsigned char  linReservedMaskPos;      // (VBE3.0 only) LSB bit position of reserved mask in linear framebuffer mode

    unsigned long  maxPixelClock;           // (VBE3.0 only) maximum pixel clock in Hz

    unsigned char  reserved[189];           // reserved field

} VESAMODEINFO_t;  // information of VESA mode

#define bool unsigned char
#define false (0==1)
#define true (0==0)

static bool vesa_GetInfo(VESAINFO_t* info, char* oemstring, unsigned short* modelist) {
    #if defined(DJGPP) || defined(__WATCOMC__)
        __dpmi_regs regs;

        // ^^^^^^^^^^^^  get VBE 2.0 information  ^^^^^^^^^^^^^^
        memset(info, 0, sizeof(VESAINFO_t));
        strncpy((char*)info->vesaSignature, "VBE2", 4);
        dosmemput(info, sizeof(VESAINFO_t), __tb);

        regs.x.ax = 0x4f00;
        regs.x.di = __tb & 0x0f;  // __tb is the DOS transfer buffer, allocated by DJGPP in start-up code
        regs.x.es = (__tb >> 4) & 0xffff;
        __dpmi_int(0x10, &regs);

        if (regs.x.ax != 0x004f) return false;  // fails to call the VESA function

        dosmemget(__tb, sizeof(VESAINFO_t), info);
        if (strncmp((char*)info->vesaSignature, "VESA", 4) != 0) return false;  // invalid VESA information

        // modify VBE 2.0 OEM-data pointers (only for Vortex86MX/MX+ VBIOS)
        info->oemVendorNamePtr  = (unsigned long)info + ((info->oemVendorNamePtr  & 0xffff0000L) >> 12) + (info->oemVendorNamePtr  & 0xffffL) - (unsigned long)__tb;
        info->oemProductNamePtr = (unsigned long)info + ((info->oemProductNamePtr & 0xffff0000L) >> 12) + (info->oemProductNamePtr & 0xffffL) - (unsigned long)__tb;
        info->oemProductRevPtr  = (unsigned long)info + ((info->oemProductRevPtr  & 0xffff0000L) >> 12) + (info->oemProductRevPtr  & 0xffffL) - (unsigned long)__tb;


        // ^^^^^^^^^^^^  get OEM string  ^^^^^^^^^^^^^^^^^^^^^^^
        if (oemstring != NULL)
        {
            #ifdef DJGPP
                unsigned long straddr = ((info->oemStringPtr & 0xffff0000L) >> 12) + (info->oemStringPtr & 0xffffL);

                for (_farsetsel(_dos_ds); ; oemstring++, straddr++)
                    if ((oemstring[0] = _farnspeekb(straddr)) == 0x00) break;
            #else
                char* straddr = (char*)(((info->oemStringPtr & 0xffff0000L) >> 12) + (info->oemStringPtr & 0xffffL));

                while ((*oemstring++ = *straddr++) != 0x00);
            #endif
        }

        // ^^^^^^^^^^^^  get VESA mode list  ^^^^^^^^^^^^^^^^^^^
        if (modelist != NULL)
        {
            #ifdef DJGPP
                unsigned long modeaddr = ((info->videoModePtr & 0xffff0000L) >> 12) + (info->videoModePtr & 0xffffL);
                printf("modelist in\n");
                for (_farsetsel(_dos_ds); ; modelist++, modeaddr += 2)
                {
                    printf("modeaddr : %x\n", _farnspeekw(modeaddr));
                    getch();
                    _farsetsel(_dos_ds);
                    if ((modelist[0] = _farnspeekw(modeaddr)) == 0xffff) break;
                }
            #else
                unsigned short* modeaddr = (unsigned short*)(((info->videoModePtr & 0xffff0000L) >> 12) + (info->videoModePtr & 0xffffL));

                while ((*modelist++ = *modeaddr++) != 0xffff);
            #endif
        }

        return true;
    #else
        union  REGS  regs;
        struct SREGS sregs;

        // ^^^^^^^^^^^^  get VBE 2.0 information  ^^^^^^^^^^^^^^
        memset(info, 0, sizeof(VESAINFO_t));
        strncpy((char*)info->vesaSignature, "VBE2", 4);
        
        regs.x.ax = 0x4f00;
        regs.x.di = FP_OFF(info);
        sregs.es  = FP_SEG(info);
        int86x(0x10, &regs, &regs, &sregs);

        if (regs.x.ax != 0x004f) return false;  // fails to call the VESA function
        if (strncmp((char*)info->vesaSignature, "VESA", 4) != 0) return false;  // invalid VESA information

        // modify VBE 2.0 OEM-data pointers (only for Vortex86MX/MX+ VBIOS)
        info->oemVendorNamePtr  = (unsigned long)MK_FP(info->oemVendorNamePtr  >> 16, info->oemVendorNamePtr  & 0xffffL);
        info->oemProductNamePtr = (unsigned long)MK_FP(info->oemProductNamePtr >> 16, info->oemProductNamePtr & 0xffffL);
        info->oemProductRevPtr  = (unsigned long)MK_FP(info->oemProductRevPtr  >> 16, info->oemProductRevPtr  & 0xffffL);

        // ^^^^^^^^^^^^  get OEM string  ^^^^^^^^^^^^^^^^^^^^^^^
        if (oemstring != NULL)
        {
            char far* straddr = (char far*)MK_FP(info->oemStringPtr >> 16, info->oemStringPtr & 0xffff);

            for ( ; (oemstring[0] = straddr[0]) != 0x00; oemstring++, straddr++)
        }

        // ^^^^^^^^^^^^  get VESA mode list  ^^^^^^^^^^^^^^^^^^^
        if (modelist != NULL)
        {
            unsigned short far* modeaddr = (unsigned short far*)MK_FP(info->videoModePtr >> 16, info->videoModePtr & 0xffff);

            for ( ; (modelist[0] = modeaddr[0]) != 0xffff; modelist++, modeaddr++)
        }

        return true;
    #endif
}

static bool vesa_GetModeInfo(unsigned short mode, VESAMODEINFO_t* modeinfo) {
    #if defined(DJGPP) || defined(__WATCOMC__)
        __dpmi_regs regs;

        memset(modeinfo, 0, sizeof(VESAMODEINFO_t));
        dosmemput(modeinfo, sizeof(VESAMODEINFO_t), __tb);

        regs.x.ax = 0x4f01;
        regs.x.cx = mode;
        regs.x.di = __tb & 0x0f;
        regs.x.es = (__tb >> 4) & 0xffff;
        __dpmi_int(0x10, &regs);

        dosmemget(__tb, sizeof(VESAMODEINFO_t), modeinfo);
    #else
        union  REGS  regs;
        struct SREGS sregs;

        memset(modeinfo, 0, sizeof(VESAMODEINFO_t));

        regs.x.ax = 0x4f01;
        regs.x.cx = mode;
        regs.x.di = FP_OFF(modeinfo);
        sregs.es  = FP_SEG(modeinfo);
        int86x(0x10, &regs, &regs, &sregs);
    #endif

    if (regs.x.ax != 0x004f) return false;  // fails to call the VESA function
    return true;
}

static int vesa_SetVESAMode(unsigned short mode);
void MX_PCI_FindVGA(int A9160)
{
	unsigned long int content = 0x00;
	unsigned long int device_ID = 0, vender_ID = 0;
	for(unsigned long bus = 0; bus < 256; bus++)
	{
		for(unsigned long device = 0; device < 32; device++)
		{
			for(unsigned long function = 0; function < 8; function++)
			{
				//Check if device exist.
				content = read_pci_reg(bus, device, function, 0);
				if(function == 0)
				{
					device_ID = (content & 0xFFFF0000) >> 16;
					vender_ID = content & 0x0000FFFF;
				}
				if((content & 0x0000FFFF) == 0x0000FFFF) // Vender ID = 0xFFFF
					continue;
				else if(((content & 0x0000FFFF) == 0) && ((content & 0xFFFF0000) == 0))
				{
					continue;
				}
				else if((device_ID == ((content & 0xFFFF0000) >> 16)) && ((vender_ID == (content & 0x0000FFFF))) && function != 0) //same device
				{
					continue;
				}
				
				vender_ID = content & 0x0000FFFF;
				device_ID = (content & 0xFFFF0000) >> 16;
                
				//Check class code
				content = read_pci_reg(bus, device, function, 8);
				content = content & 0xffffff00L;
				
				if((content == 0x00010000L) || (content == 0x03000000L) || (content == 0x04000000L))
				{
					content = read_pci_reg(bus, device, function, 4);
					write_pci_reg(bus, device, function, 4, content |= 0x107);
					if((device_ID == 0x2200) && (vender_ID == 0x17F3))
					{
						if(A9160 == 0)
							continue;
					}
					else
					{
						if(A9160 == 1)
							continue;
					}
					configs.A9160 = A9160;
					configs.bus = bus;
					configs.device = device;
					configs.function = function;
					
					return;
				}
			}
		}
	}
}

static int vesa_RemovePMFunctions() {

    if (mmio_selector >= 0)
    {
        dpmi_LinMapFree(mmio_linaddr); mmio_linaddr = 0x00000000L;
        dpmi_SelFree(mmio_selector);   mmio_selector = -1;
    }
    
    return 1;
}

static int vesa_SetupPMFunctions() {
    unsigned long mmio_size, mmio_linaddress;

    // use Vortex86MX/MX+ specific function (4F18h) to get MMIO address
    //pm_getmmioaddr = (void (*)(void))((unsigned char*)pminfo + pminfo->FunGetMMIOAddr);

    if (pci_GetMMIOAddr(&mmio_linaddress, &mmio_size) == 1) //&&
    if ((mmio_linaddr  = dpmi_LinMapAlloc(mmio_linaddress, mmio_size)) != 0x00000000L) //&&
    if ((mmio_selector = dpmi_SelAlloc(mmio_linaddr, mmio_size)) < 0)  // ugly code:p
    {
        dpmi_LinMapFree(mmio_linaddr);
        mmio_linaddr = 0x00000000L;
    }
	
    return 1;
}

void mx_Close()
{
	
	dpmi_SelFree(lfb_selector);
	dpmi_LinMapFree(lfb_linaddr);
    vesa_RemovePMFunctions();
}

static void vesa_SetScrollBIOS(unsigned short x, unsigned short y) {
    #if defined(DJGPP) || defined(__WATCOMC__)
        __dpmi_regs regs;

        regs.x.ax = 0x4f07;
        regs.x.bx = 0x0080;
        regs.x.cx = x;
        regs.x.dx = y;
        __dpmi_int(0x10, &regs);
    #else
        union REGS regs;

        regs.x.ax = 0x4f07;
        regs.x.bx = 0x0080;
        regs.x.cx = x;
        regs.x.dx = y;
        int86(0x10, &regs, &regs);
    #endif
}

static int vesa_SetVESAMode(unsigned short mode) {
    #if defined(DJGPP)
        __dpmi_regs regs;

        regs.x.ax = 0x4f02;
        regs.x.bx = mode;
        __dpmi_int(0x10, &regs);
    #endif

    if (regs.x.ax != 0x004f) return 0;  // the VESA function fails or is unsupported

    vesa_SetScrollBIOS(0, 0);
    return 1;
}

//VESAMODE
#define VESA_text           0x03
#define VESA_640x480x8bpp   0x101
#define VESA_800x600x4bpp   0x102
#define VESA_800x600x8bpp   0x103
#define VESA_1024x768x8bpp  0x105
#define VESA_640x480x16bpp  0x111
#define VESA_800x600x16bpp  0x114
#define VESA_1024x768x16bpp 0x117
#define VESA_1280x720x16bpp 0x148
#define CAP_CUSTOMIZE       0x00
int mx_Init()
{
    if (vesa_SetupPMFunctions() == 0)
    {
        printf("ERROR: fail to setup VBE 2.0 protected function!\n");
        return -1;
    }

	if((lfb_linaddr = dpmi_LinMapAlloc(pci_get_fbbaseaddress(), pci_get_fbsize())) == 0)
	{
		printf("ERROR: fail to linear mapping allocation\n");
		return -1;
	}
	
	if((lfb_selector = dpmi_SelAlloc(lfb_linaddr, pci_get_fbsize())) == -1)
	{
		printf("ERROR: fail to create selector\n");
		return -1;
	}
    
    // unsigned short modelist[1024];
    // char oemstring[1024];
    // VESAINFO_t info;
    // VESAMODEINFO_t modeinfo;
    
    // vesa_GetInfo(&info, oemstring, modelist);
    // for (int i = 0; modelist[i] != 0xffff; i++){
        // vesa_GetModeInfo(modelist[i], &modeinfo);
        // printf("mode = %x, x = %d, y = %d \n", modelist[i], modeinfo.xResolution, modeinfo.yResolution);
        // getch();
    // }
   
	return 0;
}

unsigned long timer_nowtime(void)
{
	static int usetimer = 0;
    static uclock_t inittime;
    
    if (usetimer == 0)
    {
        //inittime  = biostime(0, 0);
        inittime = uclock();
        usetimer = 1;
    }
    
    return (unsigned long)((uclock() - inittime)*1000UL/UCLOCKS_PER_SEC);
}

void a9160_Init(unsigned int hor, unsigned int ver) {
    int mode;
    
    if (hor <= 640 && ver <= 480)
        mode = VESA_640x480x16bpp;
    else if (hor <= 800 && ver <= 600)
        mode = VESA_800x600x16bpp;
    else if (hor <= 1024 && ver <= 768)
        mode = VESA_1024x768x16bpp;
    else if (hor <= 1280 && ver <= 720)
        mode = VESA_1280x720x16bpp;
    else
        printf("No this resolution.\n");
    
    MX_PCI_FindVGA(1); 
    mx_Init();
    vesa_SetVESAMode(mode + 0x4000);
}

void a9160_Close(void) {
    vesa_SetVESAMode(VESA_text + 0x4000);
}

/*
void a9160_draw_xy(int x, int y, unsigned short color) {
    _farsetsel(lfb_selector);
    _farnspokew(x * 2 + y * 1024 * 2, color);
}
*/

int get_a9160_fb_selector(void) {
    return lfb_selector;
}



// DMA
typedef struct
{
    unsigned long vgaBaseAddr;              // DMA control and VRAM buffer base address:
                                            //     b0-25  = VRAM buffer base address
                                            //     b26-28 = reserved
                                            //     b29 =  DMA channel
                                            //            0 = from system memory to VRAM buffer
                                            //            1 = from VRAM buffer to system memory
                                            //     b30 =  DMA interrupt
                                            //            0 = disable
                                            //            1 = enable
                                            //     b31 =  end flag for DMA descriptions
                                            //            0 = not last descriptor
                                            //            1 = last descriptor
    
    unsigned long sysBaseAddr;              // system memory buffer base address
    
    unsigned long byteCount;                // Fence ID and transfer byte count:
                                            //     b0-21  = byte count
                                            //     b22 =  validity flag of the DMA descriptor
                                            //            0 = invalid descriptor
                                            //            1 = valid descriptor
                                            //     b23 =  reserved
                                            //            0 = from system memory to VRAM buffer
                                            //            1 = from VRAM buffer to system memory
                                            //     b24-31 = Fence ID

    unsigned long next;                     // physical address of the next DMA descriptor

} DMAINFO_t;  // Vortex86MX/MX+ VGA DMA descriptor
typedef int DMA_HANDLE_t;
static void (*pm_dmastart)(void) = NULL;
static void (*pm_dmaquery)(void) = NULL;
DMAINFO_t*    DMA_descriptor;
DMA_HANDLE_t  DMA_descHandle;
unsigned long DMA_descPhyAddr;
#define DMA_FAIL            (-1)
#define VESADMA_IDLE        (0x00)
#define VESADMA_BUSY        (0x01)
static DMA_HANDLE_t dma_Alloc(unsigned long size, unsigned long* phyaddr) {
    DMA_HANDLE_t dma_handle;

    #if defined(DJGPP)
        int tmp = __dpmi_allocate_dos_memory((size+15)>>4, &dma_handle);
        if (tmp == -1) return DMA_FAIL; else *phyaddr = (unsigned long)tmp << 4;
    #endif

    return dma_handle;
}

static unsigned long dmadesc_Create() {
    if (DMA_descriptor != NULL) return 0L;

    if ((DMA_descriptor = (DMAINFO_t*)malloc(sizeof(DMAINFO_t))) == NULL) return 0L;
    if ((DMA_descHandle = dma_Alloc(sizeof(DMAINFO_t), &DMA_descPhyAddr)) == DMA_FAIL)
    {
        free(DMA_descriptor); 
		DMA_descriptor = NULL;
        return 0L;
    }

    DMA_descriptor->next = DMA_descPhyAddr;  // last descriptor
    return DMA_descPhyAddr;
}

static void dmadesc_Set(unsigned long sys_phyaddr, unsigned long vga_baseaddr, unsigned long nbytes, bool show) {
	
    DMA_descriptor->sysBaseAddr = sys_phyaddr;
	if(show)
		DMA_descriptor->vgaBaseAddr = 0x80000000L + vga_baseaddr;
	else
		DMA_descriptor->vgaBaseAddr = 0xa0000000L + vga_baseaddr;
    DMA_descriptor->byteCount   = 0x00400000L + nbytes;

    #if defined(DJGPP)
        dosmemput(DMA_descriptor, sizeof(DMAINFO_t), DMA_descPhyAddr);
    #endif
}

static bool vesa_StartDMA(unsigned long descriptor_phyaddr) {
    unsigned short result;
    if (pm_dmastart == NULL) return false;

    #if defined(DJGPP)
        __asm__ volatile (
            " pushw %%es         ;" //" pushal             ;"
            " movw %%ax, %%es    ;"
            " movw $0x4f18, %%ax ;"
            " call *%1           ;"
            " popw %%es          ;" //" popal              ;"
            : "=a" (result)
            : "g" (pm_dmastart), "a" (mmio_selector), "b" ((char)0x00), //sub fun 00h of 4F19h
              "c" (descriptor_phyaddr));
    #endif

    if (result != 0x004f) return false;
    return true;
}

static unsigned short vesa_QueryDMA() {
    unsigned char  dmastate;
    unsigned short result;
    if (pm_dmaquery == NULL) return 0xffff;

    #if defined(DJGPP)
        __asm__ volatile (
            " pushw %%es         ;" //" pushal             ;"
            " movw %%ax, %%es    ;"
            " movw $0x4f18, %%ax ;"
            " call *%2           ;"
            " popw %%es          ;" //" popal              ;"
            : "=b" (dmastate), "=a" (result)
            : "g" (pm_dmaquery), "a" (mmio_selector), "b" ((char)0x01)); //sub fun 01h of 4F19h
    #endif

    if (result != 0x004f) return 0xffff;
    if (dmastate == 0x00) return VESADMA_IDLE; else return VESADMA_BUSY;
}

static void dma_Copy(DMA_HANDLE_t dma_handle, unsigned long phyaddr, void* buf, unsigned long size) {
    #if defined(DJGPP)
        dosmemget(phyaddr, size, buf);
    #else
        memcpy(buf, dma_handle, (size_t)size);
    #endif
}

static bool dma_Free(DMA_HANDLE_t dma_handle) {
    #if defined(DJGPP)
        if (__dpmi_free_dos_memory(dma_handle) == -1) return false; else return true;
    #else
        free(dma_handle);
        return true;
    #endif
}

static void dmadesc_Free() {
	if (DMA_descriptor == NULL) return;
    dma_Free(DMA_descHandle);
    free(DMA_descriptor); DMA_descriptor = NULL;
}

/*
#define DMA_LINE 60
DMA_HANDLE_t dma_handle;
unsigned long dmadesc_phyaddr, dma_phyaddr, height, width;
int pre_yN = -1, yN = 0, yT;
double pixelbyte;
void a9160_DMA_Init(unsigned long h, unsigned long w, double pb) {
    pixelbyte = pb;
    dma_handle = dma_Alloc(w * DMA_LINE * pb + 15, &dma_phyaddr);
	dmadesc_phyaddr = dmadesc_Create();
    yT = h / DMA_LINE;
}

void a9160_draw_xy(int width, int y, unsigned short color) {
    dosmemput((unsigned char *)((unsigned long)((unsigned long)img + y * width * DMA_LINE * pixelbyte)), width * DMA_LINE * pixelbyte, dma_phyaddr);
    dmadesc_Set(dma_phyaddr, 0 + y * width * DMA_LINE * pixelbyte, width * DMA_LINE * pixelbyte, true);
}
*/