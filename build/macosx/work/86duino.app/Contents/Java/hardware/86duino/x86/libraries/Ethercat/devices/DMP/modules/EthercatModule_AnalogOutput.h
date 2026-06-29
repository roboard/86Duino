#ifndef __WZ_ETHERCAT_MODULE_ANALOG_OUTPUT_H
#define __WZ_ETHERCAT_MODULE_ANALOG_OUTPUT_H

#include "EthercatDevice.h"

class EthercatModule_AnalogOutput
{
public:
    virtual ~EthercatModule_AnalogOutput() = default;

    virtual int attach(uint8_t moduleNo, EthercatDevice *device, int indexDistance) { return 0; }
    virtual int detach() { return 0; }
    virtual int remapping() { return 0; }
    virtual int unmapping() { return 0; }

    virtual int getChannelCount() const = 0;
    virtual int analogWrite(int ch, int32_t value) = 0;
    virtual int voltageWrite(int ch, double voltage) { return 0; }
    virtual int currentWrite(int ch, double current) { return 0; }

    virtual int isOpenCircuit(int ch) { return 0; }
    virtual int isShortCircuit(int ch) { return 0; }
    virtual int isOverTemperature(int ch) { return 0; }
    virtual int isHardwareError(int ch) { return 0; }

    virtual int setRange(int ch, int range) { return 0; }
    virtual int setSlewRateEnable(int ch, bool enable) { return 0; }
    virtual int setSlewRateClock(int ch, int rate) { return 0; }
    virtual int setSlewRateStepSize(int ch, int step) { return 0; }
    virtual int setAlarmCode(int ch, int code) { return 0; }
    virtual int setOpenCircuitAction(int ch, int action) { return 0; }
    virtual int setShortCircuitAction(int ch, int action) { return 0; }
    virtual int setOverTemperatureAction(int ch, int action) { return 0; }
    virtual int reset(int ch) { return 0; }

    virtual int getRange(int ch) { return 0; }
    virtual int getSlewRateEnable(int ch) { return 0; }
    virtual int getSlewRateClock(int ch) { return 0; }
    virtual int getSlewRateStepSize(int ch) { return 0; }
    virtual int getAlarmCode(int ch) { return 0; }
    virtual int getOpenCircuitAction(int ch) { return 0; }
    virtual int getShortCircuitAction(int ch) { return 0; }
    virtual int getOverTemperatureAction(int ch) { return 0; }
};

#endif
