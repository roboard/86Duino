#include "VirtualLcm.h"

#define COUNTMAX    (30)

MyVirtualLcm::MyVirtualLcm() {
    _slave = NULL;
    updateCount = 0;
}

void MyVirtualLcm::init(void * slave)
{
    if (slave == NULL) return;
    
    _slave = slave;
}

void MyVirtualLcm::begin(void)
{
    if (_slave == NULL) return;
}

void MyVirtualLcm::home(void)
{
    if (_slave == NULL) return;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpHID_Generic*)_slave)->update(); }
    
    ((EthercatDevice_DmpHID_Generic*)_slave)->lcmGotoXY(1, 1);
}

void MyVirtualLcm::setCursor(uint8_t x, uint8_t y)
{
    if (_slave == NULL) return;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpHID_Generic*)_slave)->update(); }
    
    ((EthercatDevice_DmpHID_Generic*)_slave)->lcmGotoXY(x, y);
}

size_t MyVirtualLcm::write(uint8_t data)
{
    if (_slave == NULL) return 0;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpHID_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpHID_Generic*)_slave)->lcmWrite(data);
}

void MyVirtualLcm::clear()
{
    if (_slave == NULL) return;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpHID_Generic*)_slave)->update(); }
    
    ((EthercatDevice_DmpHID_Generic*)_slave)->lcmClear();
}