#include "VirtualKeypad.h"

#define COUNTMAX    (30)

MyVirtualKeypad::MyVirtualKeypad() {
    _slave = NULL;
    updateCount = 0;
}

void MyVirtualKeypad::init(void * slave)
{
    if (slave == NULL) return;
    
    _slave = slave;
}

void MyVirtualKeypad::begin()
{
    if (_slave == NULL) return;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpHID_Generic*)_slave)->update(); }
    
    ((EthercatDevice_DmpHID_Generic*)_slave)->keypadClear();
}

char MyVirtualKeypad::waitForKey(void)
{
    int c;
    if (_slave == NULL) return 0;
    
    while ((c = ((EthercatDevice_DmpHID_Generic*)_slave)->keypadRead()) == '\0') {
        helperYield();
        ((EthercatDevice_DmpHID_Generic*)_slave)->update();
    }

    return c; 
}

char MyVirtualKeypad::getKey(void)
{
    if (_slave == NULL) return 0; 

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpHID_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpHID_Generic*)_slave)->keypadRead();
}