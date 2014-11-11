#include "SwsSock.h"

#include <string.h>
#include <stdlib.h>
#include "Dhcp.h"
#include "Arduino.h"

int DhcpClass::beginWithDHCP(unsigned long timeout, unsigned long responseTimeout)
{
	return SwsSock.setDhcp(timeout, responseTimeout);
}

int DhcpClass::checkLease()
{
	return 0;
}

IPAddress DhcpClass::getLocalIp()
{
	uint8_t _dhcpLocalIp[4];
	memcpy(_dhcpLocalIp, SwsSock.getLocalIp(), 4);
	return IPAddress(_dhcpLocalIp);
}

IPAddress DhcpClass::getSubnetMask()
{
	uint8_t _dhcpSubnetMask[4];
	memcpy(_dhcpSubnetMask, SwsSock.getSubnetMask(), 4);
    return IPAddress(_dhcpSubnetMask);
}

IPAddress DhcpClass::getGatewayIp()
{
	uint8_t _dhcpGatewayIp[4];
	memcpy(_dhcpGatewayIp, SwsSock.getGatewayIp(), 4);
    return IPAddress(_dhcpGatewayIp);
}

IPAddress DhcpClass::getDnsServerIp()
{
	uint8_t _dhcpDnsServerIp[4];
	memcpy(_dhcpDnsServerIp, SwsSock.getDnsServerIp(), 4);
    return IPAddress(_dhcpDnsServerIp);
}
