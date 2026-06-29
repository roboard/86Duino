#include "VirtualSpeaker.h"

#define COUNTMAX    (30)

MyVirtualSpeaker::MyVirtualSpeaker() {
    _slave = NULL;
    updateCount = 0;
    _bufSize = 256;
}

void MyVirtualSpeaker::init(void * slave)
{
    if (slave == NULL) return;
    
    _slave = slave;
    memset(rawData, 0, sizeof(rawData));
}

int MyVirtualSpeaker::begin(void)
{
    if (_slave == NULL) return -1;
}

int MyVirtualSpeaker::setVolume(uint16_t volume)
{
    if (_slave == NULL) return -1;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpMIC_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpMIC_Generic*)_slave)->speakerSetVolume(volume);
}

int MyVirtualSpeaker::play(EthercatAudioRawType rawData)
{
    if (_slave == NULL) return -1;
    
    ((EthercatDevice_DmpMIC_Generic*)_slave)->update();
    
    return ((EthercatDevice_DmpMIC_Generic*)_slave)->speakerWrite(rawData);
}

int MyVirtualSpeaker::play(char* buf, int size)
{
    int i, bufSize;
    if (_slave == NULL) return -1;

    if (size == _bufSize) {
        memcpy((void*)rawData, (void*)buf, _bufSize);
    } else {
        bufSize = (size > _bufSize) ? _bufSize : size;
          
        for (i=0; i<bufSize; i++)
            *(((unsigned char*)rawData) + i) = buf[i];
        
        if (bufSize < _bufSize) {
            for (; i<_bufSize; i++)
                *(((unsigned char*)rawData) + i) = 0;
        }
    }
    
    ((EthercatDevice_DmpMIC_Generic*)_slave)->update();
    ((EthercatDevice_DmpMIC_Generic*)_slave)->update();
    
    return ((EthercatDevice_DmpMIC_Generic*)_slave)->speakerWrite(rawData);
}

int MyVirtualSpeaker::update()
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpMIC_Generic*)_slave)->update();
}
