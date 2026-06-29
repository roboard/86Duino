#ifndef __VIRTUAL_ENCODER_H
#define __VIRTUAL_ENCODER_H

#include "Arduino.h"
#include "Ethercat.h"
#include "EncoderInterface.h"

class MyVirtualEncoder : public _Encoder_Interface {
private:
    void* _slave;
    unsigned int updateCount;
    int _pn;
    
public:
    MyVirtualEncoder();
    void init(void* slave, int portNumber);
    int begin(void);
    
    void write(unsigned long value);
    unsigned long read(void);
    int directionRead();
    
    void setMode(uint8_t mode);
    void setIndexReset(bool _on_off);
    void setDigitalFilter(unsigned long width);
    void setInputPolarity(bool pinA, bool pinB, bool pinZ);
    void setRange(unsigned long val, bool condition = false);
    uint32_t getRange();
};

#endif