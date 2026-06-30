#ifndef __DMP_SWS_NMGR_H
#define __DMP_SWS_NMGR_H

#include "sws_nif.h"
#include <string>
#include <vector>
#include <map>

class NetworkManager
{
public:
    static NetworkManager& getInstance();
    NetworkManager(const NetworkManager&) = delete;
    void operator=(const NetworkManager&) = delete;

    void scanPhysicalInterfaces();
    void process();

    int getAdapterIndex(NetworkInterface* iface);
    size_t getInterfaceCount() const;
    NetworkInterface* getInterface(size_t index);
    NetworkInterface* getInterface(std::string ifname);
    std::string getInterfaceName(NetworkInterface* iface);

    bool addInterface(NetworkInterface* iface);
    bool addInterface(NetworkInterface& iface);
    bool addInterface(NetworkInterface* iface, std::string ifname);
    bool addInterface(NetworkInterface& iface, std::string ifname);

private:
    NetworkManager();
    ~NetworkManager();
    std::map<std::string, NetworkInterface*> _iface_map;
    std::vector<NetworkInterface*> _iface_vec;
    bool _scanned;
};

#endif
