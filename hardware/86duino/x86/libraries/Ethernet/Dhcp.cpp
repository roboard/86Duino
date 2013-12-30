/*
  Dhcp.cpp - Part of DM&P Vortex86 Ethernet library
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

#ifndef SWS_SOCK_COMPAT
#define SWS_SOCK_COMPAT
#endif
#include "sws_sock.h"
#include "SwsSock.h"

#include <string.h>
#include <stdlib.h>
#include "Dhcp.h"
#include "Arduino.h"

int DhcpClass::beginWithDHCP(uint8_t *mac, unsigned long timeout, unsigned long responseTimeout)
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
