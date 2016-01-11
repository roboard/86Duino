#ifndef __DMP_EEPROM__
#define __DMP_EEPROM__

#include "Arduino.h"

#define 	EEMEM
#define 	eeprom_is_ready() (1)
#define 	eeprom_busy_wait()   do {} while (!eeprom_is_ready())

DMPAPI(uint8_t) 	eeprom_read_byte (const uint8_t *__p);
 
DMPAPI(uint16_t) 	eeprom_read_word (const uint16_t *__p);
 
DMPAPI(uint32_t) 	eeprom_read_dword (const uint32_t *__p);
 
DMPAPI(float) 	eeprom_read_float (const float *__p);
 
DMPAPI(void) 	eeprom_read_block (void *__dst, const void *__src, size_t __n);
 
DMPAPI(void) 	eeprom_write_byte (uint8_t *__p, uint8_t __value);
 
DMPAPI(void) 	eeprom_write_word (uint16_t *__p, uint16_t __value);
 
DMPAPI(void) 	eeprom_write_dword (uint32_t *__p, uint32_t __value);
 
DMPAPI(void) 	eeprom_write_float (float *__p, float __value);
 
DMPAPI(void) 	eeprom_write_block (const void *__src, void *__dst, size_t __n);
 
DMPAPI(void) 	eeprom_update_byte (uint8_t *__p, uint8_t __value);
 
DMPAPI(void) 	eeprom_update_word (uint16_t *__p, uint16_t __value);
 
DMPAPI(void) 	eeprom_update_dword (uint32_t *__p, uint32_t __value);
 
DMPAPI(void) 	eeprom_update_float (float *__p, float __value);
 
DMPAPI(void) 	eeprom_update_block (const void *__src, void *__dst, size_t __n);

#define 	_EEPUT(addr, val)   eeprom_write_byte ((uint8_t *)(addr), (uint8_t)(val))
 
#define 	__EEPUT(addr, val)   eeprom_write_byte ((uint8_t *)(addr), (uint8_t)(val))
 
#define 	_EEGET(var, addr)   (var) = eeprom_read_byte ((const uint8_t *)(addr))
 
#define 	__EEGET(var, addr)   (var) = eeprom_read_byte ((const uint8_t *)(addr))

#endif