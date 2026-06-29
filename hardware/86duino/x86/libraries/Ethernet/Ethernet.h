#ifndef ethernet_h
#define ethernet_h

#include <Arduino.h>
#include "EthernetClient.h"
#include "EthernetServer.h"
#include "Dhcp.h"

#if defined(ETHERNET_REMOTE)
#include "ipcnet.h"
#include "vdpmem.h"
#endif

class EthernetClass
{
private:
    static DhcpClass *_dhcp;
    static uint8_t _macAddress[6];

public:
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

    void MACAddress(uint8_t *mac_address);
    IPAddress localIP();
    IPAddress subnetMask();
    IPAddress gatewayIP();
    IPAddress dnsServerIP();
    uint8_t *localMAC();

    friend class EthernetClient;
    friend class EthernetServer;
};

extern EthernetClass Ethernet;

#endif
