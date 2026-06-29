#ifndef ___WZ_ETHERCAT_DEVICE_DMP_EM_H
#define ___WZ_ETHERCAT_DEVICE_DMP_EM_H

#include "EthercatDevice_DmpCommon.h"
#include <stdint.h>

class EthercatDevice_QECRxxEMxS;

#define LOW     (0x00)
#define HIGH    (0x01)

typedef uint16_t EthercatAudioRawType[128];

class EthercatDevice_DmpEM_Generic : public _EthercatDevice_DmpCommonDriver {

public:
    EthercatDevice_DmpEM_Generic(uint32_t product = 0);
    ~EthercatDevice_DmpEM_Generic();

    int attach(uint16_t slave_id, EthercatMaster *master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int attach(uint16_t slave_id, EthercatMaster &master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int detach();
    int update();

    /*********************************************
     *                Environment                *
     *********************************************/
    double environmentReadTemperature();
    double environmentReadHumidity();
    int environmentReadPressure();
    int environmentReadPM25();
    int environmentReadCO2();
    int environmentReadFlammableGas();
    int environmentReadCO();
    int environmentReadO2();
    int environmentReadIllumination();
    int environmentReadPIR();

    /*********************************************
     *                  Battery                  *
     *********************************************/
    int batteryReadStatus();

    /*********************************************
     *              Emergency Light              *
     *********************************************/
    int emergencyLightWrite(uint8_t value);

    /*********************************************
     *                Digital I/O                *
     *********************************************/
    int digitalWrite(int pin, uint8_t value);
    int digitalRead(int pin);

    /*********************************************
     *                  Speaker                  *
     *********************************************/
    int speakerSetVolume(uint16_t volume);
    int speakerWrite(EthercatAudioRawType raw);

protected:
    bool speakerEnable;

private:
    void *private_data;

};

class EthercatDevice_QECRxxEMxS : public EthercatDevice_DmpEM_Generic {
public:
    EthercatDevice_QECRxxEMxS();
};

#endif
