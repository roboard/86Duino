#include "VirtualForceSensor.h"

#define COUNTMAX    (30)

MyVirtualForceSensor::MyVirtualForceSensor() {
    _slave = NULL;
}

void MyVirtualForceSensor::init(void * slave)
{
    if (slave == NULL) return;
    
    _slave = slave;
}

int MyVirtualForceSensor::begin(void)
{
    if (_slave == NULL) return -1;
}

uint32_t MyVirtualForceSensor::getRaw(int ch)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->getRaw(ch);
}

float MyVirtualForceSensor::getStress(int ch)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->getStress(ch);
}

float MyVirtualForceSensor::getFz(void)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->getFz();
}

float MyVirtualForceSensor::getMx(void)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->getMx();
}

float MyVirtualForceSensor::getMy(void)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->getMy();
}

int MyVirtualForceSensor::userSoftReset(void)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->userSoftReset();
}

int MyVirtualForceSensor::resetMCP4461(void)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->resetMCP4461();
}

int MyVirtualForceSensor::setFootLength(uint32_t length)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->setFootLength(length);
}

int MyVirtualForceSensor::setFootWidth(uint32_t width)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->setFootWidth(width);
}

int MyVirtualForceSensor::setSensorFilter(int ch, float filter)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->setSensorFilter(ch, filter);
}

int MyVirtualForceSensor::setSensorScale(int ch, float scale)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->setSensorScale(ch, scale);
}

int MyVirtualForceSensor::setMCP4461Reg(uint16_t address, uint16_t value)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->setMCP4461Reg(address, value);
}

int MyVirtualForceSensor::setAD7124Filter(int ch, uint32_t filter)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->setAD7124Filter(ch, filter);
}

int MyVirtualForceSensor::setAD7124PGA(int ch, uint8_t pga)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->setAD7124PGA(ch, pga);
}

int MyVirtualForceSensor::setAD7124Offset(int ch, uint32_t offset)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->setAD7124Offset(ch, offset);
}

int MyVirtualForceSensor::setAD7124Gain(int ch, uint32_t gain)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->setAD7124Gain(ch, gain);
}

int MyVirtualForceSensor::setAutoPGAEnable(int ch, bool enable)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->setAutoPGAEnable(ch, enable);
}

int MyVirtualForceSensor::setAutoAdjustMCP4461()
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->setAutoAdjustMCP4461();
}

int MyVirtualForceSensor::setAutoAD7124MidValue()
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->setAutoAD7124MidValue();
}

uint32_t MyVirtualForceSensor::getFootLength()
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->getFootLength();
}

uint32_t MyVirtualForceSensor::getFootWidth()
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->getFootWidth();
}

uint16_t MyVirtualForceSensor::getMCP4461Reg(uint16_t address)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->getMCP4461Reg(address);
}

uint16_t MyVirtualForceSensor::getCalculationMCP4461(uint16_t ch)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->getCalculationMCP4461(ch);
}

uint32_t MyVirtualForceSensor::getAD7124Filter(int ch)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->getAD7124Filter(ch);
}

uint32_t MyVirtualForceSensor::getAD7124Offset(int ch)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->getAD7124Offset(ch);
}

uint32_t MyVirtualForceSensor::getAD7124Gain(int ch)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->getAD7124Gain(ch);
}

uint8_t MyVirtualForceSensor::getAD7124PGA(int ch)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->getAD7124PGA(ch);
}

float MyVirtualForceSensor::getSensorFilter(int ch)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->getSensorFilter(ch);
}

float MyVirtualForceSensor::getSensorScale(int ch)
{
    if (_slave == NULL) return -1;
    
    return ((EthercatDevice_DmpForce_Generic*)_slave)->getSensorScale(ch);
}
