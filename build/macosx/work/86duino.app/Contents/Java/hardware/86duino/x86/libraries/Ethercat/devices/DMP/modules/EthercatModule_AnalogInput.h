#ifndef __WZ_ETHERCAT_MODULE_ANALOG_INPUT_H
#define __WZ_ETHERCAT_MODULE_ANALOG_INPUT_H

#include "EthercatDevice.h"

class EthercatModule_AnalogInput
{
public:
    virtual ~EthercatModule_AnalogInput() = default;

    virtual int attach(uint8_t moduleNo, EthercatDevice *device, int indexDistance) { return 0; }
    virtual int detach() { return 0; }
    virtual int remapping() { return 0; }
    virtual int unmapping() { return 0; }

    virtual int getChannelCount() const = 0;
    virtual int analogRead(int ch) = 0;
    virtual double voltageRead(int ch) { return 0.0; }

    virtual int isOverHighLimit(int ch) { return 0; }
    virtual int isUnderLowLimit(int ch) { return 0; }
    virtual int isHardwareError(int ch) { return 0; }

    virtual int setRange(int ch, int range) { return 0; }
    virtual int setHighLimit(int ch, double voltage) { return 0; }
    virtual int setLowLimit(int ch, double voltage) { return 0; }
    virtual int reset(int ch) { return 0; }

    virtual int getRange(int ch) { return 0; }
    virtual double getHighLimit(int ch) { return 0; }
    virtual double getLowLimit(int ch) { return 0; }
};

#endif
