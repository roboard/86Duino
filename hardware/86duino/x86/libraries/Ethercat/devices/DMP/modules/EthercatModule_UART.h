#ifndef __WZ_ETHERCAT_MODULE_UART_H
#define __WZ_ETHERCAT_MODULE_UART_H

#include "EthercatModule_SerialInterface.h"

class EthercatModule_UART : public EthercatModule_SerialInterface
{
public:
    EthercatModule_UART();
    virtual ~EthercatModule_UART();

    int attach(uint8_t moduleNo, EthercatDevice *device, int indexDistance);
    int detach();
    int update();

    int uartSetBaud(uint32_t baud);
    int uartSetFormat(uint8_t format);
    int uartSetFlowControl(int control);
    int uartClearFIFO();
    int uartIs485();
    int uartGetRTS();
    int uartGetCTS();
    int uartGetDTR();
    int uartGetDSR();
    int uartSetRTS(uint8_t value);
    int uartSetDTR(uint8_t value);
    int uartSend(void *buf, size_t size);
    int uartReceive(void *buf, size_t size);
    int uartClearTxQueue();
    int uartClearRxQueue();
    int uartQueryTxQueue();
    int uartQueryRxQueue();
    int uartTxQueueEmpty();
    int uartRxQueueEmpty();
    int uartTxQueueFull();
    int uartRxQueueFull();

private:
    void *_privateData;
};

#endif
