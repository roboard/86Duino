#include "SwsSock.h"
#include "Ethernet.h"
#include "Dhcp.h"
#include "io.h"

uint8_t EthernetClass::MAC_address[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

uint8_t *EthernetClass::localMAC()
{
	int i;
	void *mac_io;
	
	if (io_Init() == false)
		;
	else if ((mac_io = io_Alloc(IO_USE_MMIO, 0xFFFFFFB0UL, 0x06UL)) == NULL)
		io_Close();
	else {
		for (i = 0; i < 6; i++) MAC_address[i] = io_In8(mac_io, i);
		io_Free(mac_io);
		io_Close();
	}
	return &MAC_address[0];
}

int EthernetClass::begin(uint8_t *mac_address)
{
	return begin();
}

int EthernetClass::begin(void)
{
	static DhcpClass s_dhcp;
	int ret;

	_dhcp = &s_dhcp;
	
	// Now try to get our config info from a DHCP server
	ret = _dhcp->beginWithDHCP();
	if(ret == 1)
	{
		// We've successfully found a DHCP server and got our configuration info, so set things
		// accordingly
		SwsSock.setIPAddress(_dhcp->getLocalIp().raw_address());
		SwsSock.setGatewayIp(_dhcp->getGatewayIp().raw_address());
		SwsSock.setSubnetMask(_dhcp->getSubnetMask().raw_address());
		SwsSock.setDnsServerIp(_dhcp->getDnsServerIp().raw_address());
	}

	return ret;
}

void EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip)
{
	// Assume the DNS server will be the machine on the same network as the local IP
	// but with last octet being '1'
	IPAddress dns_server = local_ip;
	dns_server[3] = 1;
	begin(mac_address, local_ip, dns_server);
}

void EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server)
{
	// Assume the gateway will be the machine on the same network as the local IP
	// but with last octet being '1'
	IPAddress gateway = local_ip;
	gateway[3] = 1;
	begin(mac_address, local_ip, dns_server, gateway);
}

void EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server, IPAddress gateway)
{
	IPAddress subnet(255, 255, 255, 0);
	begin(mac_address, local_ip, dns_server, gateway, subnet);
}

void EthernetClass::begin(uint8_t *mac, IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet)
{
	SwsSock.setIPAddress(local_ip._address);
	SwsSock.setDnsServerIp(dns_server._address);
	SwsSock.setGatewayIp(gateway._address);
	SwsSock.setSubnetMask(subnet._address);
	SwsSock.init();
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

IPAddress EthernetClass::localIP()
{
	IPAddress ret;
	SwsSock.getIPAddress(ret.raw_address());
	return ret;
}

IPAddress EthernetClass::subnetMask()
{
	IPAddress ret;
	SwsSock.getSubnetMask(ret.raw_address());
	return ret;
}

IPAddress EthernetClass::gatewayIP()
{
	IPAddress ret;
	SwsSock.getGatewayIp(ret.raw_address());
	return ret;
}

IPAddress EthernetClass::dnsServerIP()
{
	IPAddress ret;
	SwsSock.getDnsServerIp(ret.raw_address());
	return ret;
}

EthernetClass Ethernet;
