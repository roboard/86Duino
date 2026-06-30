#ifndef __WZ_ETHERCAT_MODULE_DO16_H
#define __WZ_ETHERCAT_MODULE_DO16_H

#include "EthercatModule_DigitalOutput.h"

class EthercatModule_DO16 : public EthercatModule_DigitalOutput
{
public:
    EthercatModule_DO16();
    virtual ~EthercatModule_DO16();

    int attach(uint8_t moduleNo, EthercatDevice *device, int indexDistance);
    int detach();
    int remapping();
    int unmapping();

    int getChannelCount() const;
    int digitalWrite(int ch, uint8_t value);

private:
    void *_privateData;
};

#endif
