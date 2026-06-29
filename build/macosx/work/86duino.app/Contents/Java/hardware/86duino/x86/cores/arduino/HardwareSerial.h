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
#include "SerialInterface.h"

#ifdef _86DUINO
    #include "v86board.h"
#endif

#if defined (__86DUINO_ZERO)

    #define Serial1 _Serial1
    #define SerialPCIE _SerialPCIE

#elif defined (__86DUINO_ONE) || defined (__86DUINO_AI)

    #define Serial1 _Serial1
    #define Serial2 _Serial2
    #define Serial3 _Serial3
    #define Serial485 _Serial485 

#elif defined (__86DUINO_EDUCAKE)

    #define Serial1 _Serial1
    #define Serial2 _Serial2
    #define Serial3 _Serial3
    #define Serial232 _Serial232

#elif defined (__86DUINO_QEC) || defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9)  || defined (__86DUINO_QEC_15) || defined (__86DUINO_QEC_M2) || defined (__86DUINO_CYF001) || defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC_PPC104) || defined (__86DUINO_QEC_PPC9)
    
    #define Serial _Serial3
    #define Serial1 _Serial1
    
    #define SerialCOM _Serial485
    #define Serial485 _Serial485

#elif defined (__86DUINO_DUO)
 
    #define Serial1 _Serial1
    #define Serial2 _Serial2
    #define Serial3 _Serial3
    #define Serial485 _Serial485
    #define SerialPCIE _SerialPCIE

#elif defined (__86DUINO_HMI043T)

    #define Serial1 _Serial1
    #define Serial5 _Serial5
    #define Serial6 _Serial232

#else
    
    #error No this 86Duino board
    
#endif

class HardwareSerial : public SerialInterface
{
  private:
	int port;
    unsigned long baudrate;
    unsigned char format;
    unsigned long rxtimeout;
    unsigned long txtimeout;
    bool peek_stored;
    int peek_val;
  public:
    COMPort *handle;
    HardwareSerial(int com_port, unsigned long com_buadrate, unsigned char com_format, unsigned long com_rxtimeout, unsigned long com_txtimeout);
    void begin(unsigned long);
    void begin(unsigned long, int);
    void begin(unsigned long, uint8_t);
    void begin(unsigned long, uint8_t, int);
    void end();
    using Stream::setTimeout;
    void setTimeout(unsigned long rxtimeout, unsigned long txtimeout);
    int available(void);
    int peek(void);
    int read(void);
    int availableForWrite(void);
    void flush(int w=0);
    size_t write(uint8_t);
    size_t write(const uint8_t *buffer, size_t size);
    inline size_t write(unsigned short n) { return write((uint8_t)n); }
    inline size_t write(short n) { return write((uint8_t)n); }
    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
    using Print::write; // pull in write(str) and write(buf, size) from Print
    operator bool();
};

// Define config for Serial.begin(baud, config);
#define SERIAL_5N1 ((uint8_t)(BYTESIZE5|NOPARITY|STOPBIT1))
#define SERIAL_6N1 ((uint8_t)(BYTESIZE6|NOPARITY|STOPBIT1))
#define SERIAL_7N1 ((uint8_t)(BYTESIZE7|NOPARITY|STOPBIT1))
#define SERIAL_8N1 ((uint8_t)(BYTESIZE8|NOPARITY|STOPBIT1))
#define SERIAL_5N2 ((uint8_t)(BYTESIZE5|NOPARITY|STOPBIT2))
#define SERIAL_6N2 ((uint8_t)(BYTESIZE6|NOPARITY|STOPBIT2))
#define SERIAL_7N2 ((uint8_t)(BYTESIZE7|NOPARITY|STOPBIT2))
#define SERIAL_8N2 ((uint8_t)(BYTESIZE8|NOPARITY|STOPBIT2))
#define SERIAL_5E1 ((uint8_t)(BYTESIZE5|EVENPARITY|STOPBIT1))
#define SERIAL_6E1 ((uint8_t)(BYTESIZE6|EVENPARITY|STOPBIT1))
#define SERIAL_7E1 ((uint8_t)(BYTESIZE7|EVENPARITY|STOPBIT1))
#define SERIAL_8E1 ((uint8_t)(BYTESIZE8|EVENPARITY|STOPBIT1))
#define SERIAL_5E2 ((uint8_t)(BYTESIZE5|EVENPARITY|STOPBIT2))
#define SERIAL_6E2 ((uint8_t)(BYTESIZE6|EVENPARITY|STOPBIT2))
#define SERIAL_7E2 ((uint8_t)(BYTESIZE7|EVENPARITY|STOPBIT2))
#define SERIAL_8E2 ((uint8_t)(BYTESIZE8|EVENPARITY|STOPBIT2))
#define SERIAL_5O1 ((uint8_t)(BYTESIZE5|ODDPARITY|STOPBIT1))
#define SERIAL_6O1 ((uint8_t)(BYTESIZE6|ODDPARITY|STOPBIT1))
#define SERIAL_7O1 ((uint8_t)(BYTESIZE7|ODDPARITY|STOPBIT1))
#define SERIAL_8O1 ((uint8_t)(BYTESIZE8|ODDPARITY|STOPBIT1))
#define SERIAL_5O2 ((uint8_t)(BYTESIZE5|ODDPARITY|STOPBIT2))
#define SERIAL_6O2 ((uint8_t)(BYTESIZE6|ODDPARITY|STOPBIT2))
#define SERIAL_7O2 ((uint8_t)(BYTESIZE7|ODDPARITY|STOPBIT2))
#define SERIAL_8O2 ((uint8_t)(BYTESIZE8|ODDPARITY|STOPBIT2))

#include "USBAPI.h"
#define COM_FullDuplex    (1000)
#define COM_HalfDuplex    (2000)

extern HardwareSerial _Serial1;
extern HardwareSerial _Serial2;
extern HardwareSerial _Serial3;
extern HardwareSerial _Serial5;
extern HardwareSerial _Serial6;
extern HardwareSerial _Serial485;
extern HardwareSerial _Serial232;
extern HardwareSerial _Serial9;
extern HardwareSerial* HWSerial[7];

#ifndef __HARDWARE_SERIAL_LIB
	extern void serialEventRun();
#else
	extern void serialEventRun() __attribute__((weak));
#endif

#endif
