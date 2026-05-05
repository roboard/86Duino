#ifndef __MYEVA_H
#define __MYEVA_H

#include <stdint.h>
#include "Arduino.h"
#include "Ethercat.h"
#include "VirtualSerial.h"
#include "VirtualLcm.h"
#include "VirtualMpg.h"
#include "VirtualKeypad.h"
#include "VirtualLcd.h"
#include "VirtualMic.h"
#include "VirtualSpeaker.h"
#include "VirtualServo.h"
#include "VirtualCNC.h"
#include "VirtualEncoder.h"
#include "VirtualEMSensor.h"
#include "VirtualForceSensor.h"
//If ChatGPT use the C++11 feature on for loops, will need to include initializer_list header
#include <initializer_list>

enum MyVirtualError {
    EVAERR_NO_CONFIG_FILE = -1000,
    EVAERR_NO_ENOUGH_MEMORY = -1001
};

extern EthercatDevice_QECR11MP3S Slave0;

class MyEVA {

    public:
        MyEVA();
        int begin(void (*myCallback)(void) = NULL);
        void end();

        int digitalDataRead(int pin);
        int digitalDataWrite(int pin, uint8_t data);
        int integerDataRead(int pin);
        int integerDataWrite(int pin, int32_t data);
        double decimalDataRead(int pin);
        int decimalDataWrite(int pin, double data);
        int integerDataPinMode(int pin, uint32_t range);
        int digitalWrite(int pin, uint8_t val);
        int digitalRead(int pin);
        int analogPinMode(int pin, uint32_t range);
        int analogWrite(int pin, int32_t value);
        int analogRead(int pin);
        int tone(int pin, uint32_t freq, uint32_t duration);
        double getUsVoltage(short alias);
        double getUpVoltage(short alias);
        double getIsCurrent(short alias);
        double getIpCurrent(short alias);
        double getTemperature(short alias);
        int getWorkingHours(short alias);
        int getBootTimes(short alias);
};

const char* getMachineName(int aliasAddress);

extern MyEVA EVA;
extern EthercatMaster EcatMaster;

extern MyVirtualCNC VirtualCNC1;
extern MyVirtualCNC VirtualCNC2;
extern MyVirtualCNC VirtualCNC3;
extern MyVirtualCNC VirtualCNC4;
extern MyVirtualServo VirtualServo1;
extern MyVirtualServo VirtualServo2;
extern MyVirtualServo VirtualServo3;
extern MyVirtualServo VirtualServo4;
extern MyVirtualServo VirtualServo5;
extern MyVirtualServo VirtualServo6;
extern MyVirtualEncoder VirtualEncoder1;
extern MyVirtualEncoder VirtualEncoder2;
extern MyVirtualEncoder VirtualEncoder3;
extern MyVirtualEncoder VirtualEncoder4;
extern MyVirtualEncoder VirtualEncoder5;
extern MyVirtualEncoder VirtualEncoder6;
#endif