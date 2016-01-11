#ifndef twi_h
#define twi_h

  #include "Arduino.h"

  #ifndef TWI_BUFFER_LENGTH
  #define TWI_BUFFER_LENGTH 32
  #endif

  #define I2CSW_NORMAL    (0x10)
  #define I2CSW_LEGO      (0x20)

  void twi_init(void);
  void twi_init(uint32_t);
  void twi_setAddress(uint8_t);
  uint8_t twi_readFrom(uint8_t, uint8_t*, uint8_t, uint8_t);
  uint8_t twi_writeTo(uint8_t, uint8_t*, uint8_t, uint8_t, uint8_t);
  uint8_t twi_transmit(const uint8_t*, uint8_t);
  void twi_attachSlaveRxEvent( void (*)(uint8_t*, int) );
  void twi_attachSlaveTxEvent( void (*)(void) );

  void twi_stop(void);
  void twi_releaseBus(void);
  int I2C_ISR(int irq, void* data);

  // software WIRE
  bool twisw_init(unsigned devs, int i2c0mode, unsigned long i2c0clkdelay, int i2c1mode, unsigned long i2c1clkdelay);
  uint8_t twisw_readFrom(uint8_t, uint8_t*, uint8_t, uint8_t);
  uint8_t twisw_writeTo(uint8_t, uint8_t*, uint8_t, uint8_t);

#endif
