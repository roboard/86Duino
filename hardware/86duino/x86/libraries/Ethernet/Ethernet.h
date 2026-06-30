#ifndef ethernet_h
#define ethernet_h

#include "EthernetConfig.h"
#include "EthernetClient.h"
#include "EthernetServer.h"
#include "sws_nmgr.h"
#include "sws_vsw.h"
#include <Arduino.h>

enum EthernetLinkStatus {
    Unknown,
    LinkON,
    LinkOFF
};

enum EthernetHardwareStatus {
    EthernetNoHardware,
    EthernetW5100,
    EthernetW5200,
    EthernetW5500,
    EthernetR6040,
    EthernetRTL8111,
    EthernetEoE,
    EthernetVirtualNetwork
};

class EthernetClass
{
public:
    EthernetClass(NetworkInterface &iface);
    EthernetClass(const char *ifname);
    EthernetClass(const char *ifname, NetworkInterface &iface);
    ~EthernetClass();
    // Initialise the Ethernet shield to use the provided MAC address and gain the rest of the
    // configuration through DHCP.
    // Returns 0 if the DHCP configuration failed, and 1 if it succeeded
    int begin(uint8_t *mac_address);
    int begin(uint8_t *mac_address, IPAddress local_ip);
    int begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server);
    int begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server, IPAddress gateway);
    int begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet);

    int begin(void);
    int maintain();
    EthernetLinkStatus linkStatus();
    EthernetHardwareStatus hardwareStatus();

    void MACAddress(uint8_t *mac_address);
    IPAddress localIP();
    IPAddress subnetMask();
    IPAddress gatewayIP();
    IPAddress dnsServerIP();
    uint8_t *localMAC();

    void setMACAddress(uint8_t *mac_address);
    void setLocalIP(IPAddress local_ip);
    void setSubnetMask(IPAddress subnet);
    void setGatewayIP(IPAddress gateway);
    void setDnsServerIP(IPAddress dns_server);
    void setRetransmissionTimeout(uint16_t milliseconds) {}
    void setRetransmissionCount(uint8_t num) {}

    static void process();

    friend class EthernetClient;
    friend class EthernetServer;
    friend class EthernetUDP;

private:
    void *_swsData;
    uint8_t _macAddress[6];

    int isInitialized();
    int setPromiscuousMode(bool enable);
    int setMulticast(bool enable);
};

extern EthernetClass Ethernet;

#ifdef DUAL_PORT_NIC
extern EthernetClass Ethernet1;
#endif

#endif
