#include "stdarg.h"
#include "VirtualCNC.h"

#define COUNTMAX    (30)
#define BUFFERSIZE  (256)

MyVirtualCNC::MyVirtualCNC()
{
    _slave = _buffer = NULL;
    _machineOn = false;
    updateCount = 0;
}

void MyVirtualCNC::init(void * slave)
{
    if (slave == NULL) return;
    
    _slave = slave;
    _buffer = (char*)malloc(BUFFERSIZE);
    
    // Temp
    ((EthercatDevice_DmpStepper_Generic*)_slave)->machineServoOn();
    vid = ((_EthercatDevice_DmpCommonDriver*)_slave)->getVendorID();
    pid = ((_EthercatDevice_DmpCommonDriver*)_slave)->getProductCode();
}

int MyVirtualCNC::begin(void)
{
    if (_slave == NULL) return -1;
}

int MyVirtualCNC::machineOn(void)
{
    int ret = 0;
    bool _tout = true;
    unsigned long t = 0;
    
    if (_slave == NULL) return -1;

    ret = ((EthercatDevice_DmpStepper_Generic*)_slave)->machineServoOn();
    
    t = millis();
    while ((millis() - t) < 1000) {
        if (((EthercatDevice_DmpStepper_Generic*)_slave)->machineIsServoOn() == 1) { _tout = false; break; }
        updateCount++;
        if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpStepper_Generic*)_slave)->update(); } // 設 0，立即生效
    }
    
    if (_tout) return -1;
    
    return ret;
}

int MyVirtualCNC::disableSoftLimit(void)
{
    int ret = 0;
    bool _tout = true;
    unsigned long t = 0;
    
    if (_slave == NULL) return -1;

    t = millis();
    while ((millis() - t) < 1000) {
        ((EthercatDevice_DmpStepper_Generic*)_slave)->machineServoOn();
        if (((EthercatDevice_DmpStepper_Generic*)_slave)->machineIsServoOn() == 1) { _tout = false; break; }
        updateCount++;
        if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpStepper_Generic*)_slave)->update(); } // 設 0，立即生效
    }
    
    if (_tout) return -1;
    
    ret = ((EthercatDevice_DmpStepper_Generic*)_slave)->machineDisableSoftLimit(); // It is sdo, block function
    
    return ret;
}

unsigned char MyVirtualCNC::getLimitInputs()
{
    //if (vid == 0x00000BC3 && pid == 0x0086D0D6)
    return ((EthercatDevice_DmpStepper_Generic*)_slave)->sdoUpload8(0x50E3, 0);
    
    //return 0;
}

unsigned char MyVirtualCNC::getHomeInputs()
{
    //if (vid == 0x00000BC3 && pid == 0x0086D0E8)
    return ((EthercatDevice_DmpStepper_Generic*)_slave)->sdoUpload8(0x50E3, 0);
    
    //return 0;
}

unsigned char MyVirtualCNC::getAlarmInputs()
{
    // if (vid == 0x00000BC3 && pid == 0x0086D0E8)
    return ((EthercatDevice_DmpStepper_Generic*)_slave)->sdoUpload8(0x50E4, 0);
    
    //return 0;
}

void MyVirtualCNC::setDACVoltage(double outV)
{
    //if (vid == 0x00000BC3 && pid == 0x0086D0E8)
    ((EthercatDevice_DmpStepper_Generic*)_slave)->sdoDownload(0x5040, 0, &outV, sizeof(outV));
}

uint8_t MyVirtualCNC::motorIsMoving()
{
    uint8_t state;
    ((EthercatDevice_DmpStepper_Generic*)_slave)->sdoUpload(0x50E1, 0, &state, sizeof(state));
    
    return (state & 0x04) >> 2;
}

int MyVirtualCNC::gcode(char *format, ...)
{
    if (_buffer == NULL) {
        printf("No _buffer\n");
        return -1;
    }
    
    if (_slave == NULL) return -1;
    
    if (_machineOn == false) {
        _machineOn = true;
        ((EthercatDevice_DmpStepper_Generic*)_slave)->machineServoOn();
        ((EthercatDevice_DmpStepper_Generic*)_slave)->update();
    }

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpStepper_Generic*)_slave)->update(); }
    
    va_list aptr;
    int ret;

    va_start(aptr, format);
    ret = vsprintf(_buffer, format, aptr);
    va_end(aptr);
    
    // if (((_EthercatDevice_DmpCommonDriver*)_slave)->getVendorID() == 0x00000BC3 && ((_EthercatDevice_DmpCommonDriver*)_slave)->getProductCode() == 0x0086D0E8)
        // return ((_EthercatDevice_DmpCommonDriver*)_slave)->sdoDownload(0x50FF, 0, _buffer, 64);
   
    return ((EthercatDevice_DmpStepper_Generic*)_slave)->machineGcode(_buffer);
}

int MyVirtualCNC::gcode(const String &s) {
    return gcode("%s", s.c_str());
}
