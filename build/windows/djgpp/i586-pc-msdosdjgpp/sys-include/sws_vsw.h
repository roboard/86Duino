#ifndef __DMP_SWS_VSW_H
#define __DMP_SWS_VSW_H

#include "sws_nif.h"
#include <vector>
#include <string>

struct NetworkPort;
class VirtualSwitch : public NetworkInterface
{
public:
    VirtualSwitch();
    virtual ~VirtualSwitch();

    bool addPort(NetworkInterface* iface);
    bool addPort(NetworkInterface& iface);
    bool addPort(std::string ifname);
    void clearPorts();

    int begin() override;
    void end() override;
    void process() override;
    bool isSwitched() override;
    NetIfaceType getType() const override;
    EthernetFrame requestFrame() override;
    EthernetFrame receiveFrame(size_t *psize) override;
    void sendFrame(EthernetFrame frame, size_t size) override;
    void releaseFrame(EthernetFrame frame) override;

private:
    static void PacketRecv(void* userData);
    void processReceived(EthernetFrame frame, size_t size, NetworkInterface* iface);
    void processTransmit(EthernetFrame frame, size_t size, NetworkInterface* iface);
    void processRxFrame();
    void processTxFrame();
    void checkAging();
    std::vector<NetworkPort*> _ports;
    void* _privateData;
};

#endif
