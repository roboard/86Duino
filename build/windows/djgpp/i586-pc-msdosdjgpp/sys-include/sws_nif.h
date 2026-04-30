#ifndef __DMP_SWS_NIF_H
#define __DMP_SWS_NIF_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef void *EthernetFrame;

enum NetIfaceType {
    NIF_TYPE_PHYSICAL   = 0,
    NIF_TYPE_VIRTUAL    = 1,
    NIF_TYPE_BRIDGE     = 2,
    NIF_TYPE_EOE        = 3,
};

class NetworkInterface
{
public:
    NetworkInterface() {
        this->_macAddress[0] = 0xFF;
        this->_macAddress[1] = 0xFF;
        this->_macAddress[2] = 0xFF;
        this->_macAddress[3] = 0xFF;
        this->_macAddress[4] = 0xFF;
        this->_macAddress[5] = 0xFF;
        this->_isSwitched = false;
        this->_promiscuous = false;
        this->_receivedCallback = NULL;
        this->_receivedData = NULL;
        this->_sendCompletedCallback = NULL;
        this->_sendCompletedData = NULL;
    }
    virtual ~NetworkInterface() = default;

    virtual bool isPacketDriver() const {
        return false;
    }
    virtual void setSwitched(bool enable) {
        this->_isSwitched = enable;
    }
    virtual bool isSwitched() {
        return this->_isSwitched;
    }
    virtual int setMACAddress(uint8_t *mac_address) {
        memcpy(this->_macAddress, mac_address, sizeof(this->_macAddress));
        return 0;
    }
    virtual int getMACAddress(uint8_t *mac_address) {
        memcpy(mac_address, this->_macAddress, sizeof(this->_macAddress));
        return 0;
    }
    virtual int setPromiscuousMode(bool enable) {
        this->_promiscuous = enable;
        return 0;
    }
    virtual int setMode(int mode) { return 0; }
    virtual int setMulticastList(void *pData, size_t size) { return 0; }

    virtual int attachReceivedCallback(void (*callback)(void *), void *user_data) {
        this->_receivedCallback = callback;
        this->_receivedData = user_data;
        return 0;
    }
    virtual int attachSendCompletedCallback(void (*callback)(void *, int), void *user_data) {
        this->_sendCompletedCallback = callback;
        this->_sendCompletedData = user_data;
        return 0;
    }
    virtual void process() {}

    virtual int begin() = 0;
    virtual void end() = 0;
    virtual NetIfaceType getType() const = 0;
    virtual EthernetFrame requestFrame() = 0;
    virtual EthernetFrame receiveFrame(size_t *psize) = 0;
    virtual void sendFrame(EthernetFrame frame, size_t size) = 0;
    virtual void releaseFrame(EthernetFrame frame) = 0;

protected:
    uint8_t _macAddress[6];
    bool _isSwitched;
    bool _promiscuous;
    void *_receivedData;
    void *_sendCompletedData;
    void (*_receivedCallback)(void *);
    void (*_sendCompletedCallback)(void *, int);

};

#endif
