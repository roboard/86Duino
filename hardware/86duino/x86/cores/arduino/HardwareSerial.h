/*
  HardwareSerial.h - Hardware serial library for Wiring
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

  Modified 28 September 2010 by Mark Sproul
  Modified 14 August 2012 by Alarus
  Modified 01 November 2013 by Android Lin 
*/

#ifndef HardwareSerial_h
#define HardwareSerial_h

#include "com.h"
#include "Stream.h"

class HardwareSerial : public Stream
{
  private:
	int port;
    unsigned long baudrate;
    unsigned char format;
    unsigned long rxtimeout;
    unsigned long txtimeout;
    bool peek_stored;
    uint8_t peek_val;
  public:
    COMPort *handle;
    HardwareSerial(int com_port, unsigned long com_buadrate, unsigned char com_format, unsigned long com_rxtimeout, unsigned long com_txtimeout);
    void begin(unsigned long);
    void begin(unsigned long, int);
    void begin(unsigned long, uint8_t);
    void begin(unsigned long , uint8_t, int);
    void end();
    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    virtual void flush(void);
    virtual size_t write(uint8_t);
    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
    using Print::write; // pull in write(str) and write(buf, size) from Print
    operator bool();
};

// Define config for Serial.begin(baud, config);
#define SERIAL_5N1 (BYTESIZE5|NOPARITY|STOPBIT1)
#define SERIAL_6N1 (BYTESIZE6|NOPARITY|STOPBIT1)
#define SERIAL_7N1 (BYTESIZE7|NOPARITY|STOPBIT1)
#define SERIAL_8N1 (BYTESIZE8|NOPARITY|STOPBIT1)
#define SERIAL_5N2 (BYTESIZE5|NOPARITY|STOPBIT2)
#define SERIAL_6N2 (BYTESIZE6|NOPARITY|STOPBIT2)
#define SERIAL_7N2 (BYTESIZE7|NOPARITY|STOPBIT2)
#define SERIAL_8N2 (BYTESIZE8|NOPARITY|STOPBIT2)
#define SERIAL_5E1 (BYTESIZE5|EVENPARITY|STOPBIT1)
#define SERIAL_6E1 (BYTESIZE6|EVENPARITY|STOPBIT1)
#define SERIAL_7E1 (BYTESIZE7|EVENPARITY|STOPBIT1)
#define SERIAL_8E1 (BYTESIZE8|EVENPARITY|STOPBIT1)
#define SERIAL_5E2 (BYTESIZE5|EVENPARITY|STOPBIT2)
#define SERIAL_6E2 (BYTESIZE6|EVENPARITY|STOPBIT2)
#define SERIAL_7E2 (BYTESIZE7|EVENPARITY|STOPBIT2)
#define SERIAL_8E2 (BYTESIZE8|EVENPARITY|STOPBIT2)
#define SERIAL_5O1 (BYTESIZE5|ODDPARITY|STOPBIT1)
#define SERIAL_6O1 (BYTESIZE6|ODDPARITY|STOPBIT1)
#define SERIAL_7O1 (BYTESIZE7|ODDPARITY|STOPBIT1)
#define SERIAL_8O1 (BYTESIZE8|ODDPARITY|STOPBIT1)
#define SERIAL_5O2 (BYTESIZE5|ODDPARITY|STOPBIT2)
#define SERIAL_6O2 (BYTESIZE6|ODDPARITY|STOPBIT2)
#define SERIAL_7O2 (BYTESIZE7|ODDPARITY|STOPBIT2)
#define SERIAL_8O2 (BYTESIZE8|ODDPARITY|STOPBIT2)

#include "USBAPI.h"
#define COM_FullDuplex    (1000)
#define COM_HalfDuplex    (2000)
extern HardwareSerial Serial1;
extern HardwareSerial Serial2;
extern HardwareSerial Serial3;
extern HardwareSerial Serial485;
extern HardwareSerial Serial232;
extern HardwareSerial* HWSerial[4];
extern void serialEventRun(void) __attribute__((weak));

#endif
