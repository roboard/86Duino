#ifndef __VIRTUAL_SPEAKER_H
#define __VIRTUAL_SPEAKER_H

#include "Arduino.h"
#include "Ethercat.h"

class MyVirtualSpeaker {
private:
    void* _slave;
    unsigned int updateCount;
    EthercatAudioRawType rawData;
    int _bufSize;
    
public:
    MyVirtualSpeaker();
    void init(void* slave);
    int begin(void);
    int setVolume(uint16_t volume);
    int play(EthercatAudioRawType rawData);
    int play(char* buf, int size);
    int update(void);
};

#endif
