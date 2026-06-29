#ifndef __DMP_SWS_VN_H
#define __DMP_SWS_VN_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef void *EthernetFrame;

class VirtualNetwork
{
public:
    VirtualNetwork() {
        this->_macAddress[0] = 0xFF;
        this->_macAddress[1] = 0xFF;
        this->_macAddress[2] = 0xFF;
        this->_macAddress[3] = 0xFF;
        this->_macAddress[4] = 0xFF;
        this->_macAddress[5] = 0xFF;
        this->_promiscuous = false;
        this->_receivedCallback = NULL;
        this->_receivedData = NULL;
        this->_sendCompletedCallback = NULL;
        this->_sendCompletedData = NULL;
    }

    virtual void setMACAddress(uint8_t *mac_address) {
        memcpy(this->_macAddress, mac_address, sizeof(this->_macAddress));
    }
    virtual void getMACAddress(uint8_t *mac_address) {
        memcpy(mac_address, this->_macAddress, sizeof(this->_macAddress));
    }
    virtual void setPromiscuousMode(bool enable) {
        this->_promiscuous = enable;
    }
    virtual void attachReceivedCallback(void (*callback)(void *), void *user_data) {
        this->_receivedCallback = callback;
        this->_receivedData = user_data;
    }
    virtual void attachSendCompletedCallback(void (*callback)(void *), void *user_data) {
        this->_sendCompletedCallback = callback;
        this->_sendCompletedData = user_data;
    }

    virtual EthernetFrame requestFrame() { return NULL; }
    virtual void sendFrame(EthernetFrame frame, size_t size) {}

    virtual EthernetFrame receiveFrame(size_t *psize) { return NULL; }
    virtual void releaseFrame(EthernetFrame frame) {}

    virtual int setMode(int mode) { return 0; }
    virtual int setMulticastList(void *pData, size_t size) { return 0; }

    virtual void poll() {}

protected:
    uint8_t _macAddress[6];
    bool _promiscuous;
    void *_receivedData;
    void *_sendCompletedData;
    void (*_receivedCallback)(void *);
    void (*_sendCompletedCallback)(void *);

};

#endif
