#ifndef Dhcp_h
#define Dhcp_h

#include "EthernetUdp.h"

class DhcpClass {

public:
	IPAddress getLocalIp();
	IPAddress getSubnetMask();
	IPAddress getGatewayIp();
	IPAddress getDnsServerIp();

	int beginWithDHCP(unsigned long timeout = 10000, unsigned long responseTimeout = 4000);
	int checkLease();
};

#endif
