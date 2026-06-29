#ifndef TwoWire_h
#define TwoWire_h

#include <inttypes.h>
#include "Stream.h"
#include "utility/twi.h"

#define BUFFER_LENGTH    (32)
#define STANDARDMODE     (100000UL)
#define FASTMODE         (400000UL)
#define HIGHSPEEDMODE    (3300000UL)

#define NOSETWAIT    (99)
class TwoWire : public Stream
{
  private:
    uint8_t i2c_dev;
    uint32_t i2c_buflen;
    static uint8_t* rxBuffer;
    static uint8_t rxBufferIndex;
    static uint8_t rxBufferLength;

    static uint8_t txAddress;
    static uint8_t* txBuffer;
    static uint8_t txBufferIndex;
    static uint8_t txBufferLength;

    static uint8_t transmitting;
    static void (*user_onRequest)(void);
    static void (*user_onReceive)(int);
    static void onRequestService(void);
    static void onReceiveService(volatile uint8_t*, int);
  public:
    TwoWire(uint8_t);
    void changeBufferLength(uint8_t buffsize=128);
    void begin();
    void begin(bool);
    void begin(uint8_t);
    void begin(uint32_t, uint8_t slaveaddr=0x7f);
    void begin(int);
    void end(void);
    void setClock(uint32_t speed);
    void setWireTimeout(uint32_t timeout = 25000, bool reset_with_timeout = false);
    bool getWireTimeoutFlag(void);
    void clearWireTimeoutFlag(void);
    bool isBusy(void);
    void beginTransmission(uint8_t, uint8_t wait=NOSETWAIT);
    void beginTransmission(int, int wait=NOSETWAIT);
    uint8_t endTransmission(void);
    uint8_t endTransmission(uint8_t);
    uint8_t requestFrom(uint8_t, uint8_t, uint8_t sendStop=1, uint8_t wait=NOSETWAIT);
    uint8_t requestFrom(int, int, int sendStop=1, int wait=NOSETWAIT);
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *, size_t);
    virtual int available(void);
    virtual int read(void);
    virtual int peek(void);
	virtual void flush(void);
    void onReceive( void (*)(int) );
    void onRequest( void (*)(void) );

    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
    using Print::write;
};

class TwoWireLEGO : public Stream
{
  private:
    int i2c_dev;
    static uint8_t rxBuffer[];
    static uint8_t rxBufferIndex;
    static uint8_t rxBufferLength;

    static uint8_t txAddress;
    static uint8_t txBuffer[];
    static uint8_t txBufferIndex;
    static uint8_t txBufferLength;

    static uint8_t transmitting;
  public:
    TwoWireLEGO(int);
    void begin(unsigned long nHz=10000L);
    void beginTransmission(uint8_t);
    void beginTransmission(int);
    uint8_t endTransmission(void);
    uint8_t endTransmission(uint8_t);
    uint8_t requestFrom(uint8_t, uint8_t);
    uint8_t requestFrom(uint8_t, uint8_t, uint8_t);
    uint8_t requestFrom(int, int);
    uint8_t requestFrom(int, int, int);
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *, size_t);
    virtual int available(void);
    virtual int read(void);
    virtual int peek(void);
	virtual void flush(void);
};

extern TwoWireLEGO WireLEGO;
extern TwoWire Wire;

#if defined (__86DUINO_EX2)
extern TwoWire Wire1;
extern TwoWireLEGO Wire1LEGO;
#endif

#endif

