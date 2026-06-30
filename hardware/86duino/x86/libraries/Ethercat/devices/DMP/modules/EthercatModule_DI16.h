#ifndef __WZ_ETHERCAT_MODULE_DI16_H
#define __WZ_ETHERCAT_MODULE_DI16_H

#include "EthercatModule_DigitalInput.h"

class EthercatModule_DI16 : public EthercatModule_DigitalInput
{
public:
    EthercatModule_DI16();
    virtual ~EthercatModule_DI16();

    int attach(uint8_t moduleNo, EthercatDevice *device, int indexDistance);
    int detach();
    int remapping();
    int unmapping();

    int getChannelCount() const;
    int digitalRead(int ch);

private:
    void *_privateData;
};

#endif
