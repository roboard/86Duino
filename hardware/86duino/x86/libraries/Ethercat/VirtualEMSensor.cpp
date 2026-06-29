#include "VirtualEMSensor.h"

#define COUNTMAX    (30)

MyVirtualEMSensor::MyVirtualEMSensor() {
    _slave = NULL;
    updateCount = 0;
    _bufSize = 256;
}

void MyVirtualEMSensor::init(void * slave)
{
    if (slave == NULL) return;
    
    _slave = slave;
    memset(rawData, 0, sizeof(rawData));
}

int MyVirtualEMSensor::begin(void)
{
    if (_slave == NULL) return -1;
}

double MyVirtualEMSensor::readTemperature(void)
{
    if (_slave == NULL) return -1;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpEM_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpEM_Generic*)_slave)->environmentReadTemperature();
}

double MyVirtualEMSensor::readHumidity(void)
{
    if (_slave == NULL) return -1;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpEM_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpEM_Generic*)_slave)->environmentReadHumidity();
}

int MyVirtualEMSensor::readPressure(void)
{
    if (_slave == NULL) return -1;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpEM_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpEM_Generic*)_slave)->environmentReadPressure();
}

int MyVirtualEMSensor::readPM25(void)
{
    if (_slave == NULL) return -1;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpEM_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpEM_Generic*)_slave)->environmentReadPM25();
}

int MyVirtualEMSensor::readCO2(void)
{
    if (_slave == NULL) return -1;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpEM_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpEM_Generic*)_slave)->environmentReadCO2();
}

int MyVirtualEMSensor::readFlammableGas(void)
{
    if (_slave == NULL) return -1;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpEM_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpEM_Generic*)_slave)->environmentReadFlammableGas();
}

int MyVirtualEMSensor::readCO(void)
{
    if (_slave == NULL) return -1;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpEM_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpEM_Generic*)_slave)->environmentReadCO();
}

int MyVirtualEMSensor::readO2(void)
{
    if (_slave == NULL) return -1;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpEM_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpEM_Generic*)_slave)->environmentReadO2();
}

int MyVirtualEMSensor::readIllumination(void)
{
    if (_slave == NULL) return -1;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpEM_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpEM_Generic*)_slave)->environmentReadIllumination();
}

int MyVirtualEMSensor::readPIR(void)
{
    if (_slave == NULL) return -1;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpEM_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpEM_Generic*)_slave)->environmentReadPIR();
}

int MyVirtualEMSensor::readBatteryStatus(void)
{
    if (_slave == NULL) return -1;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpEM_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpEM_Generic*)_slave)->batteryReadStatus();
}

int MyVirtualEMSensor::light(int value)
{
    if (_slave == NULL) return -1;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpEM_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpEM_Generic*)_slave)->emergencyLightWrite(value);
}

int MyVirtualEMSensor::digitalWrite(uint8_t pin, uint8_t value)
{
    if (_slave == NULL) return -1;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpEM_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpEM_Generic*)_slave)->digitalWrite(pin, value);
}

int MyVirtualEMSensor::digitalRead(uint8_t pin)
{
    if (_slave == NULL) return -1;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpEM_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpEM_Generic*)_slave)->digitalRead(pin);
}

int MyVirtualEMSensor::setVolume(uint16_t volume)
{
    if (_slave == NULL) return -1;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpEM_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpEM_Generic*)_slave)->speakerSetVolume(volume);
}

int MyVirtualEMSensor::play(EthercatAudioRawType rawData)
{
    if (_slave == NULL) return -1;
    
    ((EthercatDevice_DmpEM_Generic*)_slave)->update();
    
    return ((EthercatDevice_DmpEM_Generic*)_slave)->speakerWrite(rawData);
}

int MyVirtualEMSensor::play(char* buf, int size)
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
    
    ((EthercatDevice_DmpEM_Generic*)_slave)->update();
    ((EthercatDevice_DmpEM_Generic*)_slave)->update();
    
    return ((EthercatDevice_DmpEM_Generic*)_slave)->speakerWrite(rawData);
}

int MyVirtualEMSensor::update()
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpEM_Generic*)_slave)->update();
}
