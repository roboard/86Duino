#ifndef __VIRTUAL_LCM_H
#define __VIRTUAL_LCM_H

#include "Arduino.h"
#include "Ethercat.h"

class MyVirtualLcm : public Print {
private:
    void* _slave;
    unsigned int updateCount;

public:
    MyVirtualLcm();
    void init(void* slave);
    void begin();
    void home();
    void setCursor(uint8_t, uint8_t);
    virtual size_t write(uint8_t);
    void clear();
};

#endif
