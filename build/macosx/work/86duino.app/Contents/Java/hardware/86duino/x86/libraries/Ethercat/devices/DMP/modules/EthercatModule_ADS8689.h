#ifndef __WZ_ETHERCAT_MODULE_ADS8689_H
#define __WZ_ETHERCAT_MODULE_ADS8689_H

#include "EthercatModule_AnalogInput.h"

class EthercatModule_ADS8689 : public EthercatModule_AnalogInput
{
public:
    EthercatModule_ADS8689();
    virtual ~EthercatModule_ADS8689();

    int attach(uint8_t moduleNo, EthercatDevice *device, int indexDistance);
    int detach();
    int remapping();
    int unmapping();

    int getChannelCount() const;
    int analogRead(int ch);
    double voltageRead(int ch);

    int isOverHighLimit(int ch);
    int isUnderLowLimit(int ch);
    int isHardwareError(int ch);

    int setRange(int ch, int range);
    int setHighLimit(int ch, double voltage);
    int setLowLimit(int ch, double voltage);
    int reset(int ch);

    int getRange(int ch);
    double getHighLimit(int ch);
    double getLowLimit(int ch);

private:
    void *_privateData;
};

#endif
