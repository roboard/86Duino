#ifndef __WZ_ETHERCAT_MODULE_DIGITAL_INPUT_H
#define __WZ_ETHERCAT_MODULE_DIGITAL_INPUT_H

#include "EthercatDevice.h"

class EthercatModule_DigitalInput
{
public:
    virtual ~EthercatModule_DigitalInput() = default;

    virtual int attach(uint8_t moduleNo, EthercatDevice *device, int indexDistance) { return 0; }
    virtual int detach() { return 0; }
    virtual int remapping() { return 0; }
    virtual int unmapping() { return 0; }

    virtual int getChannelCount() const = 0;
    virtual int digitalRead(int ch) = 0;
};

#endif
