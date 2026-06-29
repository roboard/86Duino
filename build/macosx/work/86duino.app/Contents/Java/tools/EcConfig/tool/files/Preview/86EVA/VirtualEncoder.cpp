#include "stdarg.h"
#include "VirtualEncoder.h"

#define COUNTMAX    (30)
#define BUFFERSIZE  (256)

MyVirtualEncoder::MyVirtualEncoder()
{
    _slave = NULL;
    updateCount = 0;
}

void MyVirtualEncoder::init(void * slave, int portNumber)
{
    if (slave == NULL) return;
    
    _slave = slave;
    
    if (portNumber == 0)
        _pn = ECAT_ENCODER_1;
    else if (portNumber == 1)
        _pn = ECAT_ENCODER_2;
    else if (portNumber == 2)
        _pn = ECAT_ENCODER_3;
    else
        _pn = ECAT_ENCODER_1;
}

int MyVirtualEncoder::begin(void)
{
    if (_slave == NULL) return -1;
}

void MyVirtualEncoder::write(unsigned long value)
{
    if (_slave == NULL) return;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpStepper_Generic*)_slave)->update(); }
    
    ((EthercatDevice_DmpStepper_Generic*)_slave)->encoderWrite(_pn, value);
}

unsigned long MyVirtualEncoder::read(void)
{
    if (_slave == NULL) return -1;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpStepper_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpStepper_Generic*)_slave)->encoderRead(_pn);
}

int MyVirtualEncoder::directionRead(void)
{
    int ret;
    if (_slave == NULL) return -1;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpStepper_Generic*)_slave)->update(); }
    
    ret = ((EthercatDevice_DmpStepper_Generic*)_slave)->encoderDirectionRead(_pn);
    
    return (ret == 0) ? 1 : -1;
}

// For internal use
void MyVirtualEncoder::setMode(uint8_t mode) {
    if (_slave == NULL) return;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpStepper_Generic*)_slave)->update(); }
    
    ((EthercatDevice_DmpStepper_Generic*)_slave)->configEncoderMode(_pn, mode);
}

void MyVirtualEncoder::setIndexReset(bool _on_off)
{
    if (_slave == NULL) return;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpStepper_Generic*)_slave)->update(); }
    
    ((EthercatDevice_DmpStepper_Generic*)_slave)->configEncoderIndexReset(_pn, _on_off);
}

void MyVirtualEncoder::setDigitalFilter(unsigned long width) {
    if (_slave == NULL) return;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpStepper_Generic*)_slave)->update(); }
    
    ((EthercatDevice_DmpStepper_Generic*)_slave)->configEncoderDigitalFilter(_pn, width);
}

void MyVirtualEncoder::setInputPolarity(bool pinA, bool pinB, bool pinZ) {
    if (_slave == NULL) return;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpStepper_Generic*)_slave)->update(); }
    
    ((EthercatDevice_DmpStepper_Generic*)_slave)->configEncoderInputPolarity(_pn, pinA, pinB, pinZ);
}

void MyVirtualEncoder::setRange(unsigned long val, bool condition) {
    if (_slave == NULL) return;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpStepper_Generic*)_slave)->update(); }
    
    ((EthercatDevice_DmpStepper_Generic*)_slave)->configEncoderRange(_pn, val);
}

uint32_t MyVirtualEncoder::getRange() {
    if (_slave == NULL) return 0L;

    updateCount++;
    if (updateCount > COUNTMAX) { updateCount = 0; helperYield(); ((EthercatDevice_DmpStepper_Generic*)_slave)->update(); }
    
    return ((EthercatDevice_DmpStepper_Generic*)_slave)->getEncoderRange(_pn);
}
