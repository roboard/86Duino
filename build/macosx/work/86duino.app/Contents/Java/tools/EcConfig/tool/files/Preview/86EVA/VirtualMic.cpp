#include "VirtualMic.h"

#define COUNTMAX    (30)

MyVirtualMicrophone::MyVirtualMicrophone() {
    _slave = NULL;
    updateCount = 0;
    _bufSize = 256;
}

void MyVirtualMicrophone::init(void * slave)
{
    if (slave == NULL) return;
    
    _slave = slave;
    memset(rawData, 0, sizeof(rawData));
}

int MyVirtualMicrophone::begin(void)
{
    if (_slave == NULL) return -1;
}

int MyVirtualMicrophone::setGain(uint16_t gain0, uint16_t gain1)
{
    if (_slave == NULL) return -1;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpMIC_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpMIC_Generic*)_slave)->microphoneSetGain(gain0, gain1);
}

int MyVirtualMicrophone::record(EthercatAudioRawType rawData)
{
    if (_slave == NULL) return -1;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpMIC_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpMIC_Generic*)_slave)->microphoneRead(rawData);
}

int MyVirtualMicrophone::record(char* buf, int size)
{
    if (_slave == NULL || size > _bufSize) return -1;
    
    int bufSize = (size > _bufSize) ? _bufSize : size;
    
    ((EthercatDevice_DmpMIC_Generic*)_slave)->update();
    
    if (((EthercatDevice_DmpMIC_Generic*)_slave)->microphoneRead(rawData) == 0) {
        for (int i=0; i<bufSize; i++)
            buf[i] = *(((unsigned char*)rawData) + i);
    } else
        return -2;
    
    return bufSize;
}

int MyVirtualMicrophone::update()
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpMIC_Generic*)_slave)->update();
}

