#include "VirtualMpg.h"

#define COUNTMAX    (30)

MyVirtualMpg::MyVirtualMpg() {
    _slave = NULL;
    updateCount = 0;
}

void MyVirtualMpg::init(void * slave)
{
    if (slave == NULL) return;
    
    _slave = slave;
    prev_time = millis();
    prev_val = 0;
    dir = 1;
}

void MyVirtualMpg::begin()
{
    prev_time = millis();
    prev_val = 0;
}

void MyVirtualMpg::setDigitalFilter(unsigned long time_us)
{
    if (_slave == NULL) return;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpHID_Generic*)_slave)->update(); }
    
    ((EthercatDevice_DmpHID_Generic*)_slave)->mpgSetNoiseFilter(time_us);
}

int MyVirtualMpg::write(unsigned long data)
{
    if (_slave == NULL) return 0;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpHID_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpHID_Generic*)_slave)->mpgWriteEncoder(data);
}

int MyVirtualMpg::writeRaw(unsigned long data)
{
    if (_slave == NULL) return 0;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpHID_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpHID_Generic*)_slave)->mpgWriteEncoderRaw(data);
}

int MyVirtualMpg::read()
{
    if (_slave == NULL) return 0;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpHID_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpHID_Generic*)_slave)->mpgReadEncoder();
}

int MyVirtualMpg::readRaw()
{
    if (_slave == NULL) return 0;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpHID_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpHID_Generic*)_slave)->mpgReadEncoderRaw();
}

int MyVirtualMpg::currentAxis()
{
    if (_slave == NULL) return 0;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpHID_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpHID_Generic*)_slave)->mpgReadAxis();
}

int MyVirtualMpg::currentRatio()
{
    if (_slave == NULL) return 0; 

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpHID_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpHID_Generic*)_slave)->mpgReadRatio();
}

bool MyVirtualMpg::isEnabled()
{
    if (_slave == NULL) return 0;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpHID_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpHID_Generic*)_slave)->mpgReadEnableSwitch() > 0 ? true : false;
}

bool MyVirtualMpg::isEmergencyStop()
{
    if (_slave == NULL) return 0;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpHID_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpHID_Generic*)_slave)->mpgReadEmergencyStop() > 0 ? true : false;
}

int MyVirtualMpg::readIncrement()
{
    int now_val, diff_val; 

    if (_slave == NULL) return 0;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpHID_Generic*)_slave)->update(); }
    
    now_val = read();
    diff_val = now_val - prev_val;
    prev_val = now_val;

    return diff_val;
}

double MyVirtualMpg::readVelocity()
{
    int now_val, diff_val;
    unsigned long now_time, diff_time;

    if (_slave == NULL) return 0;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpHID_Generic*)_slave)->update(); }
    
    now_val = read();
    now_time = millis(); 

    diff_val = now_val - prev_val; 
    if (diff_val == 0) return 0;
    diff_time = now_time - prev_time; 
    if (diff_time == 0) return 0;

    prev_val = now_val; 
    prev_time = now_time; 

    return (double)diff_val / ((double)diff_time / 1000.0); // unit: second
}

int MyVirtualMpg::invertDirection(void)
{
    if (_slave == NULL) return 0;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpHID_Generic*)_slave)->update(); }
    
    if (dir == 1) {
        ((EthercatDevice_DmpHID_Generic*)_slave)->mpgInvertEncoderDirection(true);
        dir = -1;
    } else {
        ((EthercatDevice_DmpHID_Generic*)_slave)->mpgInvertEncoderDirection(false);
        dir = 1;
    }
    return dir; 
}

void MyVirtualMpg::attachInterrupt(void (*isr)(int))
{
    if (_slave == NULL) return;

    ((EthercatDevice_DmpHID_Generic*)_slave)->mpgSetCallback(isr);
}

void MyVirtualMpg::detachInterrupt()
{
    if (_slave == NULL) return;

    ((EthercatDevice_DmpHID_Generic*)_slave)->mpgSetCallback(NULL);
}