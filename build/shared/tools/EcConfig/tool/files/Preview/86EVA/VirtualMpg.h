#ifndef __VIRTUAL_MPG_H
#define __VIRTUAL_MPG_H

#include "Arduino.h"
#include "Ethercat.h"

class MyVirtualMpg {
private:
    void* _slave;
    int prev_val;
    unsigned long prev_time;
    int dir;
    unsigned long updateCount;

public:
    MyVirtualMpg();
    void init(void* slave);
    void begin();
    int write(unsigned long data);
    int writeRaw(unsigned long data);
    int read();
    int readRaw();
    double readVelocity();
    int readIncrement();
    int currentAxis();
    int currentRatio();
    bool isEnabled();
    bool isEmergencyStop();
    void setDigitalFilter(unsigned long time_us);
    int invertDirection(void);
    void attachInterrupt(void (*isr)(int));
    void detachInterrupt();
};

#endif
