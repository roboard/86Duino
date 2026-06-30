#ifndef __WZ_ETHERCAT_MODULE_SERIAL_INTERFACE_H
#define __WZ_ETHERCAT_MODULE_SERIAL_INTERFACE_H

#include "EthercatDevice.h"

class EthercatModule_SerialInterface
{
public:
    virtual ~EthercatModule_SerialInterface() = default;

    virtual int attach(uint8_t moduleNo, EthercatDevice *device, int indexDistance) { return 0; }
    virtual int detach() { return 0; }
    virtual int update() { return 0; }
    virtual int remapping() { return 0; }
    virtual int unmapping() { return 0; }

    virtual int uartSetBaud(uint32_t baud) = 0;
    virtual int uartSetFormat(uint8_t format) = 0;
    virtual int uartSetFlowControl(int control) = 0;
    virtual int uartClearFIFO() = 0;
    virtual int uartIs485() = 0;
    virtual int uartGetRTS() = 0;
    virtual int uartGetCTS() = 0;
    virtual int uartGetDTR() = 0;
    virtual int uartGetDSR() = 0;
    virtual int uartSetRTS(uint8_t value) = 0;
    virtual int uartSetDTR(uint8_t value) = 0;
    virtual int uartSend(void *buf, size_t size) = 0;
    virtual int uartReceive(void *buf, size_t size) = 0;
    virtual int uartClearTxQueue() = 0;
    virtual int uartClearRxQueue() = 0;
    virtual int uartQueryTxQueue() = 0;
    virtual int uartQueryRxQueue() = 0;
    virtual int uartTxQueueEmpty() = 0;
    virtual int uartRxQueueEmpty() = 0;
    virtual int uartTxQueueFull() = 0;
    virtual int uartRxQueueFull() = 0;
};

#endif
