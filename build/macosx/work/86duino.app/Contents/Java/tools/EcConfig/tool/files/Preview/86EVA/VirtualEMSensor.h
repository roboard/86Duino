#ifndef __VIRTUAL_EM_SENSOR_H
#define __VIRTUAL_EM_SENSOR_H

#include "Arduino.h"
#include "Ethercat.h"

#define ON   (0x01)
#define OFF  (0x00)

class MyVirtualEMSensor {
private:
    void* _slave;
    unsigned int updateCount;
    EthercatAudioRawType rawData;
    int _bufSize;
    
public:
    MyVirtualEMSensor();
    void init(void* slave);
    int begin(void);
    
    double readTemperature(void);
    double readHumidity(void);
    int readPressure(void);
    int readPM25(void);
    int readCO2(void);
    int readFlammableGas(void);
    int readCO(void);
    int readO2(void);
    int readIllumination(void);
    int readPIR(void);
    int readBatteryStatus(void);
    int light(int value);
    int digitalWrite(uint8_t pin, uint8_t value);
    int digitalRead(uint8_t pin);
    
    int setVolume(uint16_t volume);
    int play(EthercatAudioRawType rawData);
    int play(char* buf, int size);
    int update(void);
};

#endif
