#ifndef _twi_h
#define _twi_h

  #include "Arduino.h"

  #ifndef TWI_BUFFER_LENGTH
  #define TWI_BUFFER_LENGTH 32
  #endif

  #define I2CSW_NORMAL    (0x10)
  #define I2CSW_LEGO      (0x20)

  void twi_init(uint8_t);
  void twi_init(uint8_t, uint32_t);
  void twi_disable(uint8_t);
  void twi_setFrequency(uint8_t, uint32_t);
  void twi_setAddress(uint8_t, uint8_t);
  uint8_t twi_readFrom(uint8_t, uint8_t, uint8_t*, uint8_t, uint8_t, uint8_t);
  uint8_t twi_writeTo(uint8_t, uint8_t, uint8_t*, uint8_t, uint8_t, uint8_t);
  uint8_t twi_transmit(uint8_t, const uint8_t*, uint8_t);
  void twi_attachSlaveRxEvent( uint8_t, void (*)(volatile uint8_t*, int) );
  void twi_attachSlaveTxEvent( uint8_t, void (*)(void) );
  void twi_wait_mode(uint8_t, uint8_t);
  void twi_write_wait(uint8_t);
  bool twi_isBusy(uint8_t dev);
  void twi_stop(uint8_t);
  void twi_releaseBus(uint8_t);
  void twi_setTimeoutInMicros(uint8_t, uint32_t, bool);
  void twi_handleTimeout(uint8_t, bool);
  bool twi_manageTimeoutFlag(uint8_t, bool);
  int I2C_ISR(int irq, void* data);

#endif
