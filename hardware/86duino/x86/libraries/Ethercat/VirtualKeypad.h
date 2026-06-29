#ifndef __VIRTUAL_KEYPAD_H
#define __VIRTUAL_KEYPAD_H

#include "Arduino.h"
#include "Ethercat.h"

#define NO_KEY    ('\0')

class MyVirtualKeypad {
private:
    void* _slave;
    unsigned int updateCount;

public:
    MyVirtualKeypad();
    void init(void* slave);
    void begin();
    char waitForKey(void);
    char getKey();
};

#endif
