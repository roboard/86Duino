/*
  TwoWire.cpp - TWI/I2C library for Wiring & Arduino
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

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
 
  Modified 2012 by Todd Krein (todd@krein.org) to implement repeated starts
  Modified 01 January 2016 by Android Lin
*/

#include <stdlib.h>
#include <string.h>

#include "_Wire.h"

// Initialize Class Variables //////////////////////////////////////////////////

uint8_t* _TwoWire::rxBuffer = NULL;
uint8_t _TwoWire::rxBufferIndex = 0;
uint8_t _TwoWire::rxBufferLength = 0;

uint8_t _TwoWire::txAddress = 0;
uint8_t* _TwoWire::txBuffer = NULL;
uint8_t _TwoWire::txBufferIndex = 0;
uint8_t _TwoWire::txBufferLength = 0;

uint8_t _TwoWire::transmitting = 0;
void (*_TwoWire::user_onRequest)(void) = NULL;
void (*_TwoWire::user_onReceive)(int) = NULL;

// Constructors ////////////////////////////////////////////////////////////////

_TwoWire::_TwoWire(uint8_t dev) : i2c_dev(dev) {
  i2c_buflen = BUFFER_LENGTH;
}

// Public Methods //////////////////////////////////////////////////////////////

void _TwoWire::changeBufferLength(uint8_t buffsize) {
  i2c_buflen = buffsize;
}

void _TwoWire::begin(void)
{
  rxBufferIndex = 0;
  rxBufferLength = 0;

  txBufferIndex = 0;
  txBufferLength = 0;

  if (rxBuffer == NULL) rxBuffer = (uint8_t*) malloc(sizeof(uint8_t) * i2c_buflen);
  if (txBuffer == NULL) txBuffer = (uint8_t*) malloc(sizeof(uint8_t) * i2c_buflen);
  
  twi_init(i2c_dev);
  twi_wait_mode(1, 1);
}

void _TwoWire::begin(bool wait)
{
  rxBufferIndex = 0;
  rxBufferLength = 0;

  txBufferIndex = 0;
  txBufferLength = 0;

  if (rxBuffer == NULL) rxBuffer = (uint8_t*) malloc(sizeof(uint8_t) * i2c_buflen);
  if (txBuffer == NULL) txBuffer = (uint8_t*) malloc(sizeof(uint8_t) * i2c_buflen);
  
  twi_init(i2c_dev);
  twi_wait_mode(wait, wait);
}

void _TwoWire::begin(uint8_t address)
{
  begin();
  twi_setAddress(i2c_dev, address);
  twi_attachSlaveTxEvent(i2c_dev, onRequestService);
  twi_attachSlaveRxEvent(i2c_dev, onReceiveService);
  twi_wait_mode(1, 1);
}

void _TwoWire::begin(uint32_t speed, uint8_t address)
{
  rxBufferIndex = 0;
  rxBufferLength = 0;

  txBufferIndex = 0;
  txBufferLength = 0;

  if (rxBuffer == NULL) rxBuffer = (uint8_t*) malloc(sizeof(uint8_t) * i2c_buflen);
  if (txBuffer == NULL) txBuffer = (uint8_t*) malloc(sizeof(uint8_t) * i2c_buflen);
  
  twi_init(i2c_dev, speed);
  twi_setAddress(i2c_dev, address);
  twi_attachSlaveTxEvent(i2c_dev, onRequestService);
  twi_attachSlaveRxEvent(i2c_dev, onReceiveService);
  twi_wait_mode(1, 1);
}

void _TwoWire::begin(int address)
{
    begin((uint8_t)address);
}

void _TwoWire::end(void)
{
    twi_disable(i2c_dev);
}

void _TwoWire::setClock(uint32_t speed)
{
    twi_setFrequency(i2c_dev, speed);
}

void _TwoWire::setWireTimeout(uint32_t timeout, bool reset_with_timeout)
{
    twi_setTimeoutInMicros(i2c_dev, timeout, reset_with_timeout);
}

bool _TwoWire::getWireTimeoutFlag(void)
{
    return (twi_manageTimeoutFlag(i2c_dev, false));
}

void _TwoWire::clearWireTimeoutFlag(void)
{
    twi_manageTimeoutFlag(i2c_dev, true);
}
    
uint8_t _TwoWire::requestFrom(uint8_t address, uint8_t quantity, uint8_t sendStop, uint8_t wait)
{
  if(quantity == 0) return 0;
  
  // clamp to buffer length
  if(quantity > BUFFER_LENGTH){
    quantity = BUFFER_LENGTH;
  }
  
  // perform blocking read into buffer
  uint8_t read = twi_readFrom(i2c_dev, address, rxBuffer, quantity, sendStop, wait);

  // set rx buffer iterator vars
  rxBufferIndex = 0;
  rxBufferLength = read;

  return read;
}

uint8_t _TwoWire::requestFrom(int address, int quantity, int sendStop, int wait)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)sendStop, (uint8_t)wait);
}

void _TwoWire::beginTransmission(uint8_t address, uint8_t wait)
{
  // indicate that we are transmitting
  transmitting = 1;
  // set address of targeted slave
  txAddress = address;
  // reset tx buffer iterator vars
  txBufferIndex = 0;
  txBufferLength = 0;
  
  twi_write_wait(wait);
}

void _TwoWire::beginTransmission(int address, int wait)
{
  beginTransmission((uint8_t)address, (uint8_t)wait);
}

bool _TwoWire::isBusy() {
  return twi_isBusy(i2c_dev);
}

//
//	Originally, 'endTransmission' was an f(void) function.
//	It has been modified to take one parameter indicating
//	whether or not a STOP should be performed on the bus.
//	Calling endTransmission(false) allows a sketch to 
//	perform a repeated start. 
//
//	WARNING: Nothing in the library keeps track of whether
//	the bus tenure has been properly ended with a STOP. It
//	is very possible to leave the bus in a hung state if
//	no call to endTransmission(true) is made. Some I2C
//	devices will behave oddly if they do not see a STOP.
//
uint8_t _TwoWire::endTransmission(uint8_t sendStop)
{
  //if(txBufferLength == 0) return 0;
  // transmit buffer (blocking)
  int8_t ret = twi_writeTo(i2c_dev, txAddress, txBuffer, txBufferLength, 1, sendStop);

  // reset tx buffer iterator vars
  txBufferIndex = 0;
  txBufferLength = 0;
  // indicate that we are done transmitting
  transmitting = 0;

  return ret;
}

//	This provides backwards compatibility with the original
//	definition, and expected behaviour, of endTransmission
//
uint8_t _TwoWire::endTransmission(void)
{
  return endTransmission(true);
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t _TwoWire::write(uint8_t data)
{
  int master, writeLenght;

  master = transmitting;
  writeLenght = txBufferLength;

  if(master){
  // in master transmitter mode
    // don't bother if buffer is full
    if(writeLenght >= BUFFER_LENGTH){
      setWriteError();
      return 0;
    }

    // put byte in tx buffer
    txBuffer[txBufferIndex] = data;
    ++txBufferIndex;
    // update amount in buffer   
    txBufferLength = txBufferIndex;

  }else{
  // in slave send mode
    // reply to master
    twi_transmit(i2c_dev, &data, 1);
  }
  return 1;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t _TwoWire::write(const uint8_t *data, size_t quantity)
{
  int master;

  master = transmitting;

  if(master){
  // in master transmitter mode
    for(size_t i = 0; i < quantity; ++i){
      write(data[i]);
    }
  }else{
  // in slave send mode
    // reply to master
    twi_transmit(i2c_dev, data, quantity);
  }
  return quantity;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int _TwoWire::available(void)
{
  return rxBufferLength - rxBufferIndex;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int _TwoWire::read(void)
{
  int value = -1;

  // get each successive byte on each call
  if(rxBufferIndex < rxBufferLength){
    value = rxBuffer[rxBufferIndex];
    ++rxBufferIndex;
  }

  return value;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int _TwoWire::peek(void)
{
  int value = -1;

  if(rxBufferIndex < rxBufferLength){
    value = rxBuffer[rxBufferIndex];
  }

  return value;
}

void _TwoWire::flush(void)
{
  // XXX: to be implemented.
}

// behind the scenes function that is called when data is received
void _TwoWire::onReceiveService(volatile uint8_t* inBytes, int numBytes)
{
  // don't bother if user hasn't registered a callback
  if(user_onReceive == NULL){
    return;
  }
  // don't bother if rx buffer is in use by a master requestFrom() op
  // i know this drops data, but it allows for slight stupidity
  // meaning, they may not have read all the master requestFrom() data yet
  if(rxBufferIndex < rxBufferLength){
    return;
  }
  // copy twi rx buffer into local read buffer
  // this enables new reads to happen in parallel
  for(uint8_t i = 0; i < numBytes; ++i){
    rxBuffer[i] = inBytes[i];    
  }
  // set rx iterator vars
  rxBufferIndex = 0;
  rxBufferLength = numBytes;
  // alert user program
  user_onReceive(numBytes);
}

// behind the scenes function that is called when data is requested
void _TwoWire::onRequestService(void)
{
  // don't bother if user hasn't registered a callback
  if(user_onRequest == NULL){
    return;
  }
  // reset tx buffer iterator vars
  // !!! this will kill any pending pre-master sendTo() activity
  txBufferIndex = 0;
  txBufferLength = 0;
  // alert user program
  user_onRequest();
}

// sets function called on slave write
void _TwoWire::onReceive( void (*function)(int) )
{
  user_onReceive = function;
}

// sets function called on slave read
void _TwoWire::onRequest( void (*function)(void) )
{
  user_onRequest = function;
}


_TwoWire _Wire = _TwoWire(0);

#if defined (__86DUINO_EX2)
_TwoWire _Wire1 = _TwoWire(1);
#endif
