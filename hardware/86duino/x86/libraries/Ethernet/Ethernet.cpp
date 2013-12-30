/*
  Ethernet.cpp - Part of DM&P Vortex86 Ethernet library
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
#include "Ethernet.h"
#include "Dhcp.h"

int EthernetClass::begin(uint8_t *mac_address)
{
	static DhcpClass s_dhcp;
	int ret;

	_dhcp = &s_dhcp;
	
	// Now try to get our config info from a DHCP server
	ret = _dhcp->beginWithDHCP(mac_address);
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
