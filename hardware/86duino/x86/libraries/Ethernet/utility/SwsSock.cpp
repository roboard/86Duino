/*
SwsSock.cpp - Part of DM&P Vortex86 Ethernet library
Copyright (c) 2013 DY Hung <Dyhung@dmp.com.tw>. All right reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

(If you need a commercial license, please contact soc@dmp.com.tw
to get more information.)
*/

#include "SwsSock.h"
#include "sws_nmgr.h"

#include "sws_sock.h"
#include "sws_cfg.h"
#include "sws_dlnk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "io.h"
#include "ini.h"

static inline uint16_t r6040_phy_read(uint32_t iobase, uint16_t phyaddr, uint16_t reg)
{
    int limit = 2048;
    
    io_outpw(iobase + 0x20, (1 << 13) | (phyaddr << 8) | reg);
    while (limit--) {
        if (!(io_inpw(iobase + 0x20) & (1 << 13)))
            break;
    }
    
    return io_inpw(iobase + 0x24);
}

static inline uint32_t r6040_find_iobase(int nic)
{
    #define MAX_R6040_NUM   (2)
    int bus[MAX_R6040_NUM] = {0, 0};
    int dev[MAX_R6040_NUM] = {8, 9};
    int fun[MAX_R6040_NUM] = {0, 0};
    uint32_t iobase;
    uint8_t macaddr[6];
    size_t len = sizeof(macaddr);

    SWS_DatalinkGetProperty(nic + 1, SWS_kDatalinkProperty_HwAddress, macaddr, &len);

    for (int i = 0; i < MAX_R6040_NUM; i++)
    {
        void *pci = pci_Alloc(bus[i], dev[i], fun[i]);
        if (pci != NULL)
        {
            uint8_t address[6];
            iobase = pci_In32(pci, 0x10) & 0xFFFFFFF0UL;
            ((uint16_t *)address)[0] = io_inpw(iobase + 0x68);
            ((uint16_t *)address)[1] = io_inpw(iobase + 0x6A);
            ((uint16_t *)address)[2] = io_inpw(iobase + 0x6C);
            pci_Free(pci);
            if (address[0] == macaddr[0] &&
                address[1] == macaddr[1] &&
                address[2] == macaddr[2] &&
                address[3] == macaddr[3] &&
                address[4] == macaddr[4] &&
                address[5] == macaddr[5])
                return iobase;
        }
    }

    return 0;
}

static inline uint16_t r6040_find_phyaddr(uint32_t iobase)
{
    uint8_t phyaddr = 0;

    if (iobase == 0)
        return 0;

    while (r6040_phy_read(iobase, phyaddr, 0x02) == 0xFFFF) {
        if (++phyaddr == 0)
            break;
    }

    return phyaddr;
}

static inline void r6040_set_promiscuous(uint32_t iobase, int enable)
{
    io_outpw(iobase, (io_inpw(iobase) & ~(1 << 5)) | (!!enable << 5));
}

#define SWSSOCK_CFG "Z:\\sws_sock.ini"

bool SwsSockClass::_vnetStartup = false;
bool SwsSockClass::_swsStartup = false;

struct SwsSock_t
{
    NetworkInterface *iface;
    int nic;
    int chip;
    int initialized;
    int promiscuous;
    uint32_t iobase;
    uint16_t phyaddr;
    bool isPacketDriver;
};

SwsSockClass::SwsSockClass(NetworkInterface *iface)
{
    this->_privateData = NULL;
    this->_iface = iface;
    this->_ifname.clear();
    NetworkManager::getInstance().scanPhysicalInterfaces();
    NetworkManager::getInstance().addInterface(iface);
    this->_ifname = NetworkManager::getInstance().getInterfaceName(iface);
}

SwsSockClass::SwsSockClass(const char *ifname)
{
    this->_privateData = NULL;
    this->_iface = NULL;
    this->_ifname = ifname;
    NetworkManager::getInstance().scanPhysicalInterfaces();
}

SwsSockClass::SwsSockClass(const char *ifname, NetworkInterface *iface)
{
    this->_privateData = NULL;
    this->_iface = iface;
    this->_ifname.clear();
    NetworkManager::getInstance().scanPhysicalInterfaces();
    NetworkManager::getInstance().addInterface(iface, ifname);
    this->_ifname = NetworkManager::getInstance().getInterfaceName(iface);
}

SwsSockClass::~SwsSockClass()
{
    close();
}

void SwsSockClass::close()
{
    SwsSock_t *sock = (SwsSock_t *)this->_privateData;

    if (sock == NULL)
        return;

    free(sock);
    this->_privateData = NULL;
}

int SwsSockClass::init(uint8_t *mac, uint32_t ip_address, uint32_t subnet, uint32_t gateway, uint32_t dns)
{
    NetworkInterface *iface;
    SwsSock_t *sock;
    INIFile ini;
    char section[32], ipaddress[32], ipsubnet[32], ipgateway[32], dnsserver[32];
    int nic;

    if ((iface = NetworkManager::getInstance().getInterface(this->_ifname)) == NULL)
        return 0;
    if ((nic = NetworkManager::getInstance().getAdapterIndex(iface)) < 0)
        return 0;

    if (this->_privateData)
        this->close();
    if ((sock = (SwsSock_t*)malloc(sizeof(SwsSock_t))) == NULL)
        return 0;
    memset(sock, 0, sizeof(SwsSock_t));
    sock->iface = iface;
    sock->nic = nic;
    sock->isPacketDriver = sock->iface->isPacketDriver();
    if (sock->iface == this->_iface)
        sock->iface->setMACAddress(mac);

    if (this->_swsStartup) {
        this->_swsStartup = false;
        SWS_SockCleanup();
        io_Close();
    }
    sock->initialized = 0;

    if (ini.open(SWSSOCK_CFG) < 0) {
        free(sock);
        return 0;
    }

    for (int i = 0; i < sock->nic; i++) {
        char value[64];
        sprintf(section, "PacketDriverInterface%d", i + 1);
        if (ini.get(section, "dhcp", value, sizeof(value)) == NULL &&
            ini.get(section, "ipaddress", value, sizeof(value)) == NULL) {
            ini.add(section, "ipaddress", (char *)"192.168.0.0");
        }
    }

    sprintf(section, "PacketDriverInterface%d", sock->nic + 1);
    sprintf(ipaddress, "%d.%d.%d.%d", ((uint8_t *)&ip_address)[0], ((uint8_t *)&ip_address)[1], ((uint8_t *)&ip_address)[2], ((uint8_t *)&ip_address)[3]);
    sprintf(ipsubnet, "%d.%d.%d.%d", ((uint8_t *)&subnet)[0], ((uint8_t *)&subnet)[1], ((uint8_t *)&subnet)[2], ((uint8_t *)&subnet)[3]);
    sprintf(ipgateway, "%d.%d.%d.%d", ((uint8_t *)&gateway)[0], ((uint8_t *)&gateway)[1], ((uint8_t *)&gateway)[2], ((uint8_t *)&gateway)[3]);
    sprintf(dnsserver, "%d.%d.%d.%d", ((uint8_t *)&dns)[0], ((uint8_t *)&dns)[1], ((uint8_t *)&dns)[2], ((uint8_t *)&dns)[3]);
    ini.del(section);
    ini.add(section, "ipaddress", ipaddress);
    ini.add(section, "ipsubnet", ipsubnet);
    ini.add(section, "ipgateway", ipgateway);
    ini.add(section, "dhcp", 0);

    for (int i = sock->nic + 1; ; i++) {
        char ip[32] = {'\0'}, subnet[32] = {'\0'}, gateway[32] = {'\0'}, dhcp[32] = {'\0'};
        char value[64];
        sprintf(section, "PacketDriverInterface%d", i + 1);
        if (ini.get(section, "dhcp", value, sizeof(value)) == NULL &&
            ini.get(section, "ipaddress", value, sizeof(value)) == NULL)
            break;
        ini.get(section, "ipaddress", ip, sizeof(ip));
        ini.get(section, "ipsubnet", subnet, sizeof(subnet));
        ini.get(section, "ipgateway", gateway, sizeof(gateway));
        ini.get(section, "dhcp", dhcp, sizeof(dhcp));
        ini.del(section);
        if (ip[0] != '\0')
            ini.add(section, "ipaddress", ip);
        if (subnet[0] != '\0')
            ini.add(section, "ipsubnet", subnet);
        if (gateway[0] != '\0')
            ini.add(section, "ipgateway", gateway);
        if (dhcp[0] != '\0')
            ini.add(section, "dhcp", dhcp);
    }

    ini.del("dns");
    ini.add("dns", "hostname", (char *)"86Duino");
    ini.add("dns", "server", dnsserver);
    ini.close();

    if (io_Init() == false) {
        free(sock);
        return 0;
    }
    if (SWS_CfgSetName(SWSSOCK_CFG, NULL) == 0 ||
        SWS_SockStartup(0, NULL) != 0) {
        io_Close();
        free(sock);
        return 0;
    }

    this->_swsStartup = true;
    sock->initialized = 1;
    sock->iobase = r6040_find_iobase(sock->nic);
    sock->phyaddr = r6040_find_phyaddr(sock->iobase);
    if (sock->iobase > 0) {
        sock->chip = NIC_R6040;
    } else {
        switch (sock->iface->getType()) {
        case NIF_TYPE_PHYSICAL: sock->chip = NIC_OTHERS;  break;
        case NIF_TYPE_VIRTUAL:  sock->chip = NIC_VNET;    break;
        case NIF_TYPE_BRIDGE:   sock->chip = NIC_BRIDGE;  break;
        case NIF_TYPE_EOE:      sock->chip = NIC_EOE;     break;
        default:                sock->chip = NIC_UNKNOWN; break;
        }
    }
    this->_privateData = sock;

    return 1;
}

int SwsSockClass::init(uint8_t *mac)
{
    NetworkInterface *iface;
    SwsSock_t *sock;
    INIFile ini;
    char section[32];
    int nic;

    if ((iface = NetworkManager::getInstance().getInterface(this->_ifname)) == NULL)
        return 0;
    if ((nic = NetworkManager::getInstance().getAdapterIndex(iface)) < 0)
        return 0;

    if (this->_privateData)
        this->close();
    if ((sock = (SwsSock_t*)malloc(sizeof(SwsSock_t))) == NULL)
        return 0;
    memset(sock, 0, sizeof(SwsSock_t));
    sock->iface = iface;
    sock->nic = nic;
    sock->isPacketDriver = sock->iface->isPacketDriver();
    if (sock->iface == this->_iface)
        sock->iface->setMACAddress(mac);

    if (this->_swsStartup) {
        this->_swsStartup = false;
        SWS_SockCleanup();
        io_Close();
    }
    sock->initialized = 0;

    if (ini.open(SWSSOCK_CFG) < 0)
        return 0;

    for (int i = 0; i < sock->nic; i++) {
        char value[64];
        sprintf(section, "PacketDriverInterface%d", i + 1);
        if (ini.get(section, "dhcp", value, sizeof(value)) == NULL &&
            ini.get(section, "ipaddress", value, sizeof(value)) == NULL) {
            ini.add(section, "ipaddress", (char *)"192.168.0.0");
        }
    }

    sprintf(section, "PacketDriverInterface%d", sock->nic + 1);
    ini.del(section);
    ini.add(section, "dhcp", 1);

    for (int i = sock->nic + 1; ; i++) {
        char ip[32] = {'\0'}, subnet[32] = {'\0'}, gateway[32] = {'\0'}, dhcp[32] = {'\0'};
        char value[64];
        sprintf(section, "PacketDriverInterface%d", i + 1);
        if (ini.get(section, "dhcp", value, sizeof(value)) == NULL &&
            ini.get(section, "ipaddress", value, sizeof(value)) == NULL)
            break;
        ini.get(section, "ipaddress", ip, sizeof(ip));
        ini.get(section, "ipsubnet", subnet, sizeof(subnet));
        ini.get(section, "ipgateway", gateway, sizeof(gateway));
        ini.get(section, "dhcp", dhcp, sizeof(dhcp));
        ini.del(section);
        if (ip[0] != '\0')
            ini.add(section, "ipaddress", ip);
        if (subnet[0] != '\0')
            ini.add(section, "ipsubnet", subnet);
        if (gateway[0] != '\0')
            ini.add(section, "ipgateway", gateway);
        if (dhcp[0] != '\0')
            ini.add(section, "dhcp", dhcp);
    }

    ini.del("dns");
    ini.add("dns", "hostname", (char *)"86Duino");
    ini.close();

    if (io_Init() == false)
        return 0;
    if (SWS_CfgSetName(SWSSOCK_CFG, NULL) == 0 ||
        SWS_SockStartup(0, NULL) != 0) {
        io_Close();
        return 0;
    }

    this->_swsStartup = true;
    sock->initialized = 1;
    sock->iobase = r6040_find_iobase(sock->nic);
    sock->phyaddr = r6040_find_phyaddr(sock->iobase);
    if (sock->iobase > 0) {
        sock->chip = NIC_R6040;
    } else {
        switch (sock->iface->getType()) {
        case NIF_TYPE_PHYSICAL: sock->chip = NIC_OTHERS;  break;
        case NIF_TYPE_VIRTUAL:  sock->chip = NIC_VNET;    break;
        case NIF_TYPE_BRIDGE:   sock->chip = NIC_BRIDGE;  break;
        case NIF_TYPE_EOE:      sock->chip = NIC_EOE;     break;
        default:                sock->chip = NIC_UNKNOWN; break;
        }
    }
    this->_privateData = sock;

    return 1;
}

int SwsSockClass::isInitialized()
{
    SwsSock_t *sock = (SwsSock_t *)this->_privateData;
    if (sock == NULL)
        return 0;
    return sock->initialized;
}

int SwsSockClass::getChip()
{
    SwsSock_t *sock = (SwsSock_t *)this->_privateData;
    if (sock == NULL)
        return NIC_UNKNOWN;
    return sock->chip;
}

int SwsSockClass::setPromiscuousMode(bool enable)
{
    SwsSock_t *sock = (SwsSock_t *)this->_privateData;

    if (sock == NULL)
        return 0;
    if (sock->initialized == 0)
        return 0;
    if (sock->promiscuous == 0 && enable == false)
        return 0;

    sock->promiscuous += !!enable - !enable;

    if ((enable && sock->promiscuous == 1) || (!enable && sock->promiscuous == 0))
    {
        uint32_t uVal = !!enable;
        SWS_DatalinkSetProperty(sock->nic + 1, SWS_kDatalinkProperty_Promiscuity, &uVal, sizeof(uVal));
        sock->iface->setPromiscuousMode(!!enable);
    }

    return 1;
}

int SwsSockClass::setMulticast(bool enable)
{
    SwsSock_t *sock = (SwsSock_t *)this->_privateData;

    if (sock == NULL)
        return 0;
    if (sock->initialized == 0)
        return 0;
    if (sock->isPacketDriver)
        return setPromiscuousMode(enable);

    return 1;
}

EnetLinkstatus SwsSockClass::getLinkStatus()
{
    SwsSock_t *sock = (SwsSock_t *)this->_privateData;

    if (sock == NULL)
        return UNKNOWN;
    if (sock->chip != NIC_R6040)
        return UNKNOWN;

    return ((r6040_phy_read(sock->iobase, sock->phyaddr, 0x01) >> 2) & 1) ? (LINK_ON) : (LINK_OFF);
}

void SwsSockClass::getMACAddress(uint8_t *mac_address)
{
    SwsSock_t *sock = (SwsSock_t *)this->_privateData;
    size_t len = 6;

    if (sock == NULL)
        return;
    if (sock->initialized == 0) {
        memset(mac_address, 0xFF, len);
        return;
    }

    SWS_DatalinkGetProperty(sock->nic + 1, SWS_kDatalinkProperty_HwAddress, mac_address, &len);
}

uint32_t SwsSockClass::getLocalIP()
{
    SwsSock_t *sock = (SwsSock_t *)this->_privateData;
    uint32_t ipaddr = 0;
    size_t len = sizeof(ipaddr);

    if (sock == NULL)
        return 0;
    if (sock->initialized == 0)
        return 0;

    SWS_DatalinkGetProperty(sock->nic + 1, SWS_kDatalinkProperty_Address, &ipaddr, &len);
    return SWS_ntohl(ipaddr);
}

uint32_t SwsSockClass::getSubnetMask()
{
    SwsSock_t *sock = (SwsSock_t *)this->_privateData;
    uint32_t subnet = 0;
    size_t len = sizeof(subnet);

    if (sock == NULL)
        return 0;
    if (sock->initialized == 0)
        return 0;

    SWS_DatalinkGetProperty(sock->nic + 1, SWS_kDatalinkProperty_SubnetMask, &subnet, &len);
    return SWS_ntohl(subnet);
}

uint32_t SwsSockClass::getGatewayIP()
{
    SwsSock_t *sock = (SwsSock_t *)this->_privateData;
    uint32_t gateway = 0;
    size_t len = sizeof(gateway);

    if (sock == NULL)
        return 0;
    if (sock->initialized == 0)
        return 0;

    SWS_DatalinkGetProperty(sock->nic + 1, SWS_kDatalinkProperty_Gateway, &gateway, &len);
    return SWS_ntohl(gateway);
}

uint32_t SwsSockClass::getDnsServerIP()
{
    SwsSock_t *sock = (SwsSock_t *)this->_privateData;

    if (sock == NULL)
        return 0;
    if (sock->initialized == 0)
        return 0;

    return SWS_ntohl(SWS_DnsGetPriServer());
}

void SwsSockClass::setMACAddress(uint8_t *mac_address)
{
    SwsSock_t *sock = (SwsSock_t *)this->_privateData;

    if (sock == NULL)
        return;
    if (sock->initialized == 0)
        return;

    SWS_DatalinkSetProperty(sock->nic + 1, SWS_kDatalinkProperty_HwAddress, mac_address, 6);
}

void SwsSockClass::setLocalIP(uint32_t local_ip)
{
    SwsSock_t *sock = (SwsSock_t *)this->_privateData;

    if (sock == NULL)
        return;
    if (sock->initialized == 0)
        return;

    local_ip = SWS_ntohl(local_ip);
    SWS_DatalinkSetProperty(sock->nic + 1, SWS_kDatalinkProperty_Address, &local_ip, sizeof(local_ip));
}

void SwsSockClass::setSubnetMask(uint32_t subnet)
{
    SwsSock_t *sock = (SwsSock_t *)this->_privateData;

    if (sock == NULL)
        return;
    if (sock->initialized == 0)
        return;

    subnet = SWS_ntohl(subnet);
    SWS_DatalinkSetProperty(sock->nic + 1, SWS_kDatalinkProperty_SubnetMask, &subnet, sizeof(subnet));
}

void SwsSockClass::setGatewayIP(uint32_t gateway)
{
    SwsSock_t *sock = (SwsSock_t *)this->_privateData;

    if (sock == NULL)
        return;
    if (sock->initialized == 0)
        return;

    gateway = SWS_ntohl(gateway);
    SWS_DatalinkSetProperty(sock->nic + 1, SWS_kDatalinkProperty_Gateway, &gateway, sizeof(gateway));
}

void SwsSockClass::setDnsServerIP(uint32_t dns_server)
{
    SwsSock_t *sock = (SwsSock_t *)this->_privateData;

    if (sock == NULL)
        return;
    if (sock->initialized == 0)
        return;

    SWS_DnsSetPriServer(SWS_ntohl(dns_server));
}
