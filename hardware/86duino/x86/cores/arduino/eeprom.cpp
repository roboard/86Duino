#include "io.h"
#include "avr/eeprom.h"

#define EEPROMSIZE_BANK0 (200)

static unsigned char read_cmos(unsigned char address)
{
  if(address >= EEPROMSIZE_BANK0) // 0~199
	return 0;

#if (defined(DMP_DOS_BC) || defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM))
  io_DisableINT();
#elif defined (DMP_LINUX)
  lockCMOS();
#endif
  void *pciDev = NULL;
  // south bridge register C0H bit 3 controls CMOS page select
  unsigned long int reg;
  unsigned char result;
  
  pciDev = pci_Alloc(0x00, 0x07, 0x00);
  if(pciDev == NULL)
  {
#if (defined(DMP_DOS_BC) || defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM))
    Serial.print("CMOS device doesn't exist\n");
#elif (defined(DMP_LINUX))
    printf("CMOS device doesn't exist\n");
#endif
    return 0;
  }
  
  reg = pci_In32(pciDev, 0xc0);

  if(address == 20 || address == 22)//special case
  {
  	//Set bit 3 to access high 128 bytes RTC SRAM
	pci_Out32(pciDev, 0xc0, reg | 0x00000008);
	address = (address == 20)? 26:27;
	io_outpb(0x70, address);
  }
  else if(address < 100) // 0~99 low page 
  {
	//clear bit 3 to access low 128 bytes RTC SRAM
	pci_Out32(pciDev, 0xc0, reg & 0xfffffff7);
	io_outpb(0x70, address + 28);
  }
  else// 100~199 high page
  {
	//Set bit 3 to access high 128 bytes RTC SRAM
	pci_Out32(pciDev, 0xc0, reg | 0x00000008);
	address -= 100;
	io_outpb(0x70, address + 28);
  }
  result = io_inpb(0x71);
  
  // Restore old register value
  pci_Out32(pciDev, 0xc0, reg);
  
  pci_Free(pciDev);

#if (defined(DMP_DOS_BC) || defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM))
  io_RestoreINT();
#elif defined (DMP_LINUX)
  unLockCMOS();
#endif

  return result;
}

static void write_cmos(unsigned char address, unsigned char buf)
{
  if(address >= EEPROMSIZE_BANK0) // 0~199
	return ;

#if (defined(DMP_DOS_BC) || defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM))
  io_DisableINT();
#elif defined (DMP_LINUX)
  lockCMOS();
#endif

  void *pciDev = NULL;
  pciDev = pci_Alloc(0x00, 0x07, 0x00);
  if(pciDev == NULL)
  {
#if (defined(DMP_DOS_BC) || defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM))
    Serial.print("CMOS device doesn't exist\n");
#elif (defined(DMP_LINUX))
    printf("CMOS device doesn't exist\n");
#endif
    return;
  }

  unsigned long int reg;
  reg = pci_In32(pciDev, 0xc0);
  if(address == 20 || address == 22) //special case
  {
    //Set bit 3 to access high 128 bytes RTC SRAM
	pci_Out32(pciDev, 0xc0, reg | 0x00000008);
        address = (address == 20)? 26:27;
        io_outpb(0x70, address);
  }
  else if(address < 100) // 0~99 low page 
  {
	//clear bit 3 to access low 128 bytes RTC SRAM
	pci_Out32(pciDev, 0xc0, reg & 0xfffffff7);
	io_outpb(0x70, address + 28);
  }
  else// 100~199 high page
  {
	//Set bit 3 to access high 128 bytes RTC SRAM
	pci_Out32(pciDev, 0xc0, reg | 0x00000008);
	address -= 100;
	io_outpb(0x70, address + 28);
  }
  io_outpb(0x71, buf);
  // Restore old register value
  pci_Out32(pciDev, 0xc0, reg);
  pci_Free(pciDev);
  
#if (defined(DMP_DOS_BC) || defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM))
  io_RestoreINT();
#elif defined (DMP_LINUX)
  unLockCMOS();
#endif

}

DMPAPI(uint8_t) eeprom_read_byte (const uint8_t *__p) {
	return read_cmos((uint8_t) __p);
}
 
DMPAPI(uint16_t) 	eeprom_read_word (const uint16_t *__p) {
	uint16_t tmp;
	tmp = read_cmos((uint8_t) __p); // read high byte
	tmp = (tmp << 8) + read_cmos(((uint8_t) __p)+1); // read low byte
	return tmp;
}
 
DMPAPI(uint32_t) 	eeprom_read_dword (const uint32_t *__p) {
	uint32_t tmp;
	tmp = read_cmos((uint8_t) __p); // read 3rd high byte
	tmp = (tmp << 8) + read_cmos(((uint8_t) __p)+1); // read 2nd high byte
	tmp = (tmp << 8) + read_cmos(((uint8_t) __p)+2); // read 1st high byte
	tmp = (tmp << 8) + read_cmos(((uint8_t) __p)+3); // read low byte
	return tmp;
}
 
DMPAPI(float) 	eeprom_read_float (const float *__p) {
	float tmp;
	unsigned char* p = (unsigned char*)&tmp;
	p[0] = read_cmos((uint8_t) __p);
	p[1] = read_cmos(((uint8_t) __p)+1);
	p[2] = read_cmos(((uint8_t) __p)+2);
	p[3] = read_cmos(((uint8_t) __p)+3);
	return tmp;
}
 
DMPAPI(void) 	eeprom_read_block (void *__dst, const void *__src, size_t __n) {
	uint8_t* _dst = (uint8_t*) __dst;
	for(int i=0; i<__n; i++) _dst[i] = read_cmos(((uint8_t) __src)+i);
}
 
DMPAPI(void) 	eeprom_write_byte (uint8_t *__p, uint8_t __value) {
	write_cmos((uint8_t) __p, __value);
}
 
DMPAPI(void) 	eeprom_write_word (uint16_t *__p, uint16_t __value) {
	write_cmos((uint8_t) __p, (__value & 0xff00) >> 8);
	write_cmos(((uint8_t) __p)+1, __value & 0x00ff);
}
 
DMPAPI(void) 	eeprom_write_dword (uint32_t *__p, uint32_t __value) {
	write_cmos((uint8_t) __p, (__value & 0xff000000L) >> 24);
	write_cmos(((uint8_t) __p)+1, (__value & 0x00ff0000L) >> 16);
	write_cmos(((uint8_t) __p)+2, (__value & 0x0000ff00L) >> 8);
	write_cmos(((uint8_t) __p)+3, __value & 0x000000ffL);
}
 
DMPAPI(void) 	eeprom_write_float (float *__p, float __value) {
	unsigned char* p = (unsigned char*)&__value;
	write_cmos((uint8_t) __p, p[0]);
	write_cmos(((uint8_t) __p)+1, p[1]);
	write_cmos(((uint8_t) __p)+2, p[2]);
	write_cmos(((uint8_t) __p)+3, p[3]);
}
 
DMPAPI(void) 	eeprom_write_block (const void *__src, void *__dst, size_t __n) {
	uint8_t* _src = (uint8_t*) __src;
	for(int i=0; i<__n; i++) write_cmos(((uint8_t) __dst)+i, _src[i]);
}
 
DMPAPI(void) 	eeprom_update_byte (uint8_t *__p, uint8_t __value) {
	eeprom_write_byte(__p, __value);
}
 
DMPAPI(void) 	eeprom_update_word (uint16_t *__p, uint16_t __value) {
	eeprom_write_word(__p, __value);
}
 
DMPAPI(void) 	eeprom_update_dword (uint32_t *__p, uint32_t __value) {
	eeprom_write_dword(__p, __value);
}
 
DMPAPI(void) 	eeprom_update_float (float *__p, float __value) {
	eeprom_write_float(__p, __value);
}
 
DMPAPI(void) 	eeprom_update_block (const void *__src, void *__dst, size_t __n) {
	eeprom_write_block(__src, __dst, __n);
}