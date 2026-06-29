#ifndef ___WZ_ETHERCAT_DEVICE_DMP_MIC_H
#define ___WZ_ETHERCAT_DEVICE_DMP_MIC_H

#include "EthercatDevice_DmpCommon.h"
#include <stdlib.h>
#include <stdint.h>

class EthercatDevice_QECR11MC1S;
class EthercatDevice_QECR11MC4S;
class EthercatDevice_QECR11MC8S;

#define ECAT_COUNTER_STOP               (0)
#define ECAT_COUNTER_ONESHOT            (1)
#define ECAT_COUNTER_CONTINUOUS         (2)
#define ECAT_COUNTER_CONTINUOUS_CLEAR   (3)

typedef uint16_t EthercatAudioRawType[128];

class EthercatDevice_DmpMIC_Generic : public _EthercatDevice_DmpCommonDriver {

public:
    EthercatDevice_DmpMIC_Generic(uint32_t product = 0);
    ~EthercatDevice_DmpMIC_Generic();

    int attach(uint16_t slave_id, EthercatMaster *master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int attach(uint16_t slave_id, EthercatMaster &master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int detach();
    int update();

    /*********************************************
     *                  Counter                  *
     *********************************************/
    int counterSetMode(uint8_t mode, uint16_t continuous_period_ms = 1);
    int counterOneshotTrigger();
    int counterOneshotIsLatched();
    int counterRead(size_t channel);

    /*********************************************
     *                 Microphone                *
     *********************************************/
    int microphoneSetGain(uint16_t gain0, uint16_t gain1);
    int microphoneRead(EthercatAudioRawType raw);

    /*********************************************
     *                  Speaker                  *
     *********************************************/
    int speakerSetVolume(uint16_t volume);
    int speakerWrite(EthercatAudioRawType raw);

protected:
    size_t counterChannels;
    bool speakerEnable;
    bool microphoneEnable;

private:
    void *private_data;

};

class EthercatDevice_QECR11MC1S : public EthercatDevice_DmpMIC_Generic {
public:
    EthercatDevice_QECR11MC1S();
};

class EthercatDevice_QECR11MC4S : public EthercatDevice_DmpMIC_Generic {
public:
    EthercatDevice_QECR11MC4S();
};

class EthercatDevice_QECR11MC8S : public EthercatDevice_DmpMIC_Generic {
public:
    EthercatDevice_QECR11MC8S();
};

#endif
