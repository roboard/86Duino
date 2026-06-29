#ifndef __VIRTUAL_FORCE_SENSOR_H
#define __VIRTUAL_FORCE_SENSOR_H

#include "Arduino.h"
#include "Ethercat.h"

class MyVirtualForceSensor {
private:
    void* _slave;
    unsigned int updateCount;
    
public:
    MyVirtualForceSensor();
    void init(void* slave);
    int begin(void);
    
    uint32_t getRaw(int ch);

    float getStress(int ch);
    float getFz();
    float getMx();
    float getMy();

    int userSoftReset();
    int resetMCP4461();

    int setFootLength(uint32_t length);
    int setFootWidth(uint32_t width);
    int setSensorFilter(int ch, float filter);
    int setSensorScale(int ch, float scale);
    int setMCP4461Reg(uint16_t address, uint16_t value);
    int setAD7124Filter(int ch, uint32_t filter);
    int setAD7124PGA(int ch, uint8_t pga);
    int setAD7124Offset(int ch, uint32_t offset);
    int setAD7124Gain(int ch, uint32_t gain);
    int setAutoPGAEnable(int ch, bool enable);
    int setAutoAdjustMCP4461();
    int setAutoAD7124MidValue();

    uint32_t getFootLength();
    uint32_t getFootWidth();
    uint16_t getMCP4461Reg(uint16_t address);
    uint16_t getCalculationMCP4461(uint16_t ch);
    uint32_t getAD7124Filter(int ch);
    uint32_t getAD7124Offset(int ch);
    uint32_t getAD7124Gain(int ch);
    uint8_t getAD7124PGA(int ch);
    float getSensorFilter(int ch);
    float getSensorScale(int ch);
};

#endif
