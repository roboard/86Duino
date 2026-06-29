#ifndef __VIRTUAL_MIC_H
#define __VIRTUAL_MIC_H

#include "Arduino.h"
#include "Ethercat.h"

class MyVirtualMicrophone {
private:
    void* _slave;
    unsigned int updateCount;
    EthercatAudioRawType rawData;
    int _bufSize;
    
public:
    MyVirtualMicrophone();
    void init(void* slave);
    int begin(void);
    int setGain(uint16_t gain0, uint16_t gain1);
    int record(EthercatAudioRawType rawData);
    int record(char* buf, int size);
    int update(void);
};

#endif
