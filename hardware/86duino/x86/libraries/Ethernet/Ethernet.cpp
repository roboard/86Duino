#include "SwsSock.h"
#include "Ethernet.h"
#include "sws_nmgr.h"

void EthernetClass::process()
{
    NetworkManager::getInstance().process();
}

EthernetClass::EthernetClass(NetworkInterface &iface)
{
    this->_swsData = new (std::nothrow) SwsSockClass(&iface);
    this->_macAddress[0] = 0xFF;
    this->_macAddress[1] = 0xFF;
    this->_macAddress[2] = 0xFF;
    this->_macAddress[3] = 0xFF;
    this->_macAddress[4] = 0xFF;
    this->_macAddress[5] = 0xFF;
}

EthernetClass::EthernetClass(const char *ifname, NetworkInterface &iface)
{
    this->_swsData = new (std::nothrow) SwsSockClass(ifname, &iface);
    this->_macAddress[0] = 0xFF;
    this->_macAddress[1] = 0xFF;
    this->_macAddress[2] = 0xFF;
    this->_macAddress[3] = 0xFF;
    this->_macAddress[4] = 0xFF;
    this->_macAddress[5] = 0xFF;
}

EthernetClass::EthernetClass(const char *ifname)
{
    this->_swsData = new (std::nothrow) SwsSockClass(ifname);
    this->_macAddress[0] = 0xFF;
    this->_macAddress[1] = 0xFF;
    this->_macAddress[2] = 0xFF;
    this->_macAddress[3] = 0xFF;
    this->_macAddress[4] = 0xFF;
    this->_macAddress[5] = 0xFF;
}

EthernetClass::~EthernetClass()
{
    SwsSockClass *SwsSock = (SwsSockClass *)this->_swsData;

    if (SwsSock == NULL)
        return;

    delete SwsSock;
    this->_swsData = NULL;
}

uint8_t *EthernetClass::localMAC()
{
    SwsSockClass *SwsSock = (SwsSockClass *)this->_swsData;

    if (SwsSock)
        SwsSock->getMACAddress(this->_macAddress);

    return this->_macAddress;
}

void EthernetClass::MACAddress(uint8_t *mac_address)
{
    memcpy(mac_address, localMAC(), 6);
}

int EthernetClass::begin(uint8_t *mac_address)
{
    SwsSockClass *SwsSock = (SwsSockClass *)this->_swsData;

    if (SwsSock == NULL)
        return 0;

    return SwsSock->init(mac_address);
}

int EthernetClass::begin(void)
{
    return begin(this->_macAddress);
}

int EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip)
{
    // Assume the DNS server will be the machine on the same network as the local IP
    // but with last octet being '1'
    IPAddress dns_server = local_ip;
    dns_server[3] = 1;
    return begin(mac_address, local_ip, dns_server);
}

int EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server)
{
    // Assume the gateway will be the machine on the same network as the local IP
    // but with last octet being '1'
    IPAddress gateway = local_ip;
    gateway[3] = 1;
    return begin(mac_address, local_ip, dns_server, gateway);
}

int EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server, IPAddress gateway)
{
    IPAddress subnet(255, 255, 255, 0);
    return begin(mac_address, local_ip, dns_server, gateway, subnet);
}

int EthernetClass::begin(uint8_t *mac, IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet)
{
    SwsSockClass *SwsSock = (SwsSockClass *)this->_swsData;

    if (SwsSock == NULL)
        return 0;

    return SwsSock->init(mac, local_ip, subnet, gateway, dns_server);
}

int EthernetClass::maintain()
{
#if 0
    int rc = DHCP_CHECK_NONE;
    if(_dhcp != NULL){
        //we have a pointer to dhcp, use it
        rc = _dhcp->checkLease();
        switch ( rc ){
            case DHCP_CHECK_NONE:
                //nothing done
                break;
            case DHCP_CHECK_RENEW_OK:
            case DHCP_CHECK_REBIND_OK:
                //we might have got a new IP.
                W5100.setIPAddress(_dhcp->getLocalIp().raw_address());
                W5100.setGatewayIp(_dhcp->getGatewayIp().raw_address());
                W5100.setSubnetMask(_dhcp->getSubnetMask().raw_address());
                _dnsServerAddress = _dhcp->getDnsServerIp();
                break;
            default:
                //this is actually a error, it will retry though
            break;
        }
    }
    return rc;
#else
    return 0;
#endif
}

EthernetLinkStatus EthernetClass::linkStatus()
{
    SwsSockClass *SwsSock = (SwsSockClass *)this->_swsData;

    if (SwsSock == NULL)
        return Unknown;

    switch (SwsSock->getLinkStatus()) {
    case LINK_ON:  return LinkON;
    case LINK_OFF: return LinkOFF;
    default: break;
    }

    return Unknown;
}

EthernetHardwareStatus EthernetClass::hardwareStatus()
{
    SwsSockClass *SwsSock = (SwsSockClass *)this->_swsData;

    if (SwsSock == NULL)
        return EthernetNoHardware;

    switch (SwsSock->getChip()) {
    case NIC_R6040:  return EthernetR6040;
    case NIC_OTHERS: return EthernetRTL8111;
    case NIC_VNET:   return EthernetVirtualNetwork;
    case NIC_EOE:    return EthernetEoE;
    default: break;
    }

    return EthernetNoHardware;
}

IPAddress EthernetClass::localIP()
{
    SwsSockClass *SwsSock = (SwsSockClass *)this->_swsData;

    if (SwsSock == NULL)
        return IPAddress((uint32_t)0);

    return SwsSock->getLocalIP();
}

IPAddress EthernetClass::subnetMask()
{
    SwsSockClass *SwsSock = (SwsSockClass *)this->_swsData;

    if (SwsSock == NULL)
        return IPAddress((uint32_t)0);

    return SwsSock->getSubnetMask();
}

IPAddress EthernetClass::gatewayIP()
{
    SwsSockClass *SwsSock = (SwsSockClass *)this->_swsData;

    if (SwsSock == NULL)
        return IPAddress((uint32_t)0);

    return SwsSock->getGatewayIP();
}

IPAddress EthernetClass::dnsServerIP()
{
    SwsSockClass *SwsSock = (SwsSockClass *)this->_swsData;

    if (SwsSock == NULL)
        return IPAddress((uint32_t)0);

    return SwsSock->getDnsServerIP();
}

void EthernetClass::setMACAddress(uint8_t *mac_address)
{
    SwsSockClass *SwsSock = (SwsSockClass *)this->_swsData;

    if (SwsSock == NULL)
        return;

    SwsSock->setMACAddress(mac_address);
}

void EthernetClass::setLocalIP(IPAddress local_ip)
{
    SwsSockClass *SwsSock = (SwsSockClass *)this->_swsData;

    if (SwsSock == NULL)
        return;

    SwsSock->setLocalIP(local_ip);
}

void EthernetClass::setSubnetMask(IPAddress subnet)
{
    SwsSockClass *SwsSock = (SwsSockClass *)this->_swsData;

    if (SwsSock == NULL)
        return;

    SwsSock->setSubnetMask(subnet);
}

void EthernetClass::setGatewayIP(IPAddress gateway)
{
    SwsSockClass *SwsSock = (SwsSockClass *)this->_swsData;

    if (SwsSock == NULL)
        return;

    SwsSock->setGatewayIP(gateway);
}

void EthernetClass::setDnsServerIP(IPAddress dns_server)
{
    SwsSockClass *SwsSock = (SwsSockClass *)this->_swsData;

    if (SwsSock == NULL)
        return;

    SwsSock->setDnsServerIP(dns_server);
}

int EthernetClass::isInitialized()
{
    SwsSockClass *SwsSock = (SwsSockClass *)this->_swsData;

    if (SwsSock == NULL)
        return 0;

    return SwsSock->isInitialized();
}

int EthernetClass::setPromiscuousMode(bool enable)
{
    SwsSockClass *SwsSock = (SwsSockClass *)this->_swsData;

    if (SwsSock == NULL)
        return 0;

    return SwsSock->setPromiscuousMode(enable);
}

int EthernetClass::setMulticast(bool enable)
{
    SwsSockClass *SwsSock = (SwsSockClass *)this->_swsData;

    if (SwsSock == NULL)
        return 0;

    return SwsSock->setMulticast(enable);
}

#ifdef DUAL_PORT_NIC
    EthernetClass Ethernet("eth0");
    EthernetClass Ethernet1("eth1");
#else
    EthernetClass Ethernet("eth0");
#endif
