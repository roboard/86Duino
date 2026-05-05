#ifndef ___SERIAL_INTERFACE_H
#define ___SERIAL_INTERFACE_H

#include <stdlib.h>
#include <stdint.h>

#include "Stream.h"

class SerialInterface : public Stream {
    
    public:
        virtual size_t write(uint8_t data) { return 0; };
        virtual size_t write(const uint8_t *buffer, size_t size) { return 0; };
        virtual int available(void) { return 0; };
        virtual int peek(void) { return 0; };
        virtual int read(void) { return 0; };
        virtual void flush(int w) {};
        virtual void setTimeout(unsigned long rxtimeout, unsigned long txtimeout) {};
    
};

#endif