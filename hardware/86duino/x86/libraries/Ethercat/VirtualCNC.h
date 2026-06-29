#ifndef __VIRTUAL_CNC_H
#define __VIRTUAL_CNC_H

#include "Arduino.h"
#include "Ethercat.h"

class MyVirtualCNC {
private:
    void* _slave;
    unsigned int updateCount;
    char* _buffer;
    bool _machineOn;
    unsigned long vid, pid;
    
public:
    MyVirtualCNC();
    void init(void* slave);
    int begin(void);
    int machineOn(void);
    int disableSoftLimit(void);
    unsigned char getLimitInputs(void);
    unsigned char getHomeInputs(void);
    unsigned char getAlarmInputs(void);
    void setDACVoltage(double outV);
    uint8_t motorIsMoving(void);
    int gcode(char *format, ...);
    int gcode(const String &s);
    
    EthercatDevice_CiA402* cia402GetServo(int motor, EthercatDevice_CiA402 *servo);
};

#endif