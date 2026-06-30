#ifndef __WZ_ETHERCAT_MODULE_DIGITAL_OUTPUT_H
#define __WZ_ETHERCAT_MODULE_DIGITAL_OUTPUT_H

#include "EthercatDevice.h"

class EthercatModule_DigitalOutput
{
public:
    virtual ~EthercatModule_DigitalOutput() = default;

    virtual int attach(uint8_t moduleNo, EthercatDevice *device, int indexDistance) { return 0; }
    virtual int detach() { return 0; }
    virtual int remapping() { return 0; }
    virtual int unmapping() { return 0; }

    virtual int getChannelCount() const = 0;
    virtual int digitalWrite(int ch, uint8_t value) = 0;
};

#endif
