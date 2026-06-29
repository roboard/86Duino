#include "SwsSock.h"

#include "Dhcp.h"

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
    return SwsSock.getLocalIp();
}

IPAddress DhcpClass::getSubnetMask()
{
    return SwsSock.getSubnetMask();
}

IPAddress DhcpClass::getGatewayIp()
{
    return SwsSock.getGatewayIp();
}

IPAddress DhcpClass::getDnsServerIp()
{
    return SwsSock.getDnsServerIp();
}
