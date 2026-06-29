#ifndef __NETWORK_EOE_H
#define __NETWORK_EOE_H

#include "Ethercat.h"
#include "sws_nif.h"

class NetworkEoE : public NetworkInterface
{
public:
    NetworkEoE(EthercatDevice &device, uint8_t port = 0);
    NetworkEoE(EthercatMaster &master, uint16_t slave_no, uint8_t port = 0);
    ~NetworkEoE();

    int begin() override;
    void end() override;

    NetIfaceType getType() const override;
    EthernetFrame requestFrame() override;
    EthernetFrame receiveFrame(size_t *psize) override;
    void sendFrame(EthernetFrame frame, size_t size) override;
    void releaseFrame(EthernetFrame frame) override;
    void process() override;

private:
    EthercatDevice *_device;
    EthercatMaster *_master;
    uint16_t _slaveNo;
    uint8_t _port;
    void *_privateData;
    int processRxFrame();
    int processTxFrame();

};

#endif
