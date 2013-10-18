#ifndef HardwareSerial_h
#define HardwareSerial_h

#include "com.h"
#include "Stream.h"

class HardwareSerial : public Stream
{
  private:
    COMPort *handle;
	int port;
    unsigned long baudrate;
    unsigned char format;
    unsigned long timeout;
    bool peek_stored;
    uint8_t peek_val;
  public:
    HardwareSerial(int com_port, unsigned long com_buadrate, unsigned char com_format, unsigned long com_timeout);
    void begin(unsigned long);
    void begin(unsigned long, uint8_t);
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
    //using Print::write; // pull in write(str) and write(buf, size) from Print
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

#include "USBAPI.h" // It is Serial class on 86Duino
extern HardwareSerial Serial1;
extern HardwareSerial Serial2;
extern HardwareSerial Serial3;
extern HardwareSerial Serial485;

extern void serialEventRun(void) __attribute__((weak));

#endif
