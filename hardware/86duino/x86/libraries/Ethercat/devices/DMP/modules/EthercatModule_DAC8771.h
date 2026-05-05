#ifndef __WZ_ETHERCAT_MODULE_DAC8771_H
#define __WZ_ETHERCAT_MODULE_DAC8771_H

#include "EthercatModule_AnalogOutput.h"

class EthercatModule_DAC8771 : public EthercatModule_AnalogOutput
{
public:
    EthercatModule_DAC8771();
    virtual ~EthercatModule_DAC8771();

    int attach(uint8_t moduleNo, EthercatDevice *device, int indexDistance);
    int detach();
    int remapping();
    int unmapping();

    int getChannelCount() const;
    int analogWrite(int ch, int32_t value);
    int voltageWrite(int ch, double voltage);
    int currentWrite(int ch, double current);

    int isOpenCircuit(int ch);
    int isShortCircuit(int ch);
    int isOverTemperature(int ch);
    int isHardwareError(int ch);

    int setRange(int ch, int range);
    int setSlewRateEnable(int ch, bool enable);
    int setSlewRateClock(int ch, int rate);
    int setSlewRateStepSize(int ch, int step);
    int setAlarmCode(int ch, int code);
    int setOpenCircuitAction(int ch, int action);
    int setShortCircuitAction(int ch, int action);
    int setOverTemperatureAction(int ch, int action);
    int reset(int ch);

    int getRange(int ch);
    int getSlewRateEnable(int ch);
    int getSlewRateClock(int ch);
    int getSlewRateStepSize(int ch);
    int getAlarmCode(int ch);
    int getOpenCircuitAction(int ch);
    int getShortCircuitAction(int ch);
    int getOverTemperatureAction(int ch);

private:
    void *_privateData;
};

#endif
