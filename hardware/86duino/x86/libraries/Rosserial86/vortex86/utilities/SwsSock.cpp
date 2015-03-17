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

#include "sws_cfg.h"
#include "sws_dlnk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SWSSOCK_CFG  "sws_sock.tmp"

// SwsSockClass SwsSock;

SwsSockClass::SwsSockClass()
{
	inited = 0;
}

int SwsSockClass::setDhcp(unsigned long timeout, unsigned long responseTimeout)
{	
	// set DHCP config
	FILE *opt;
	opt = fopen(SWSSOCK_CFG, "w");
	
	if (opt == NULL)
		return 0;
		
	fprintf(opt, "[PacketDriverInterface1]\n");
	fprintf(opt, "dhcp=1\n");
	fprintf(opt, "\n");
	fprintf(opt, "[dns]\n");
	fprintf(opt, "hostname=86Duino\n");
	fflush(opt);
	fclose(opt);
		
	if (SWS_CfgSetName(SWSSOCK_CFG, NULL) == 0)
		return 0;

	inited = 1;
	if (SWS_SockStartup(0, NULL)) {
		SWS_SockExit();
		return 0;
	}
		
	if (getULLocalIp() == 0x0100007FUL) {
		SWS_SockExit();
		return 0;
	}
	
	return 1;
}

SWS_u_long SwsSockClass::getULLocalIp()
{
	char host_name[128];
	struct SWS_hostent *Host;
	SWS_u_long ipaddr;
	
	if (SWS_gethostname(host_name, sizeof(host_name)) != 0) {
		ipaddr = 0UL;
	}
	else {
		Host = SWS_gethostbyname(host_name);
		ipaddr = *(SWS_u_long*)Host->h_addr_list[0];
	}
	
	return ipaddr;
}

uint8_t *SwsSockClass::getLocalIp()
{
	SWS_u_long ipaddr = getULLocalIp();
	
	_SwsSockLocalIp[0] = (uint8_t)(ipaddr);
	_SwsSockLocalIp[1] = (uint8_t)(ipaddr >> 8);
	_SwsSockLocalIp[2] = (uint8_t)(ipaddr >> 16);
	_SwsSockLocalIp[3] = (uint8_t)(ipaddr >> 24);
	
	return (uint8_t*)_SwsSockLocalIp;
}

SWS_u_long SwsSockClass::getULSubnetMask()
{
	SWS_u_long ipaddr;
	size_t len;
	
	len = sizeof(ipaddr);
	if (SWS_DatalinkGetProperty(1, SWS_kDatalinkProperty_SubnetMask, &ipaddr, &len) != 0) {
		ipaddr = 0UL;
	}
	
	return ipaddr;
}

uint8_t *SwsSockClass::getSubnetMask()
{
	SWS_u_long ipaddr = getULSubnetMask();
	
	_SwsSockNetMask[3] = (uint8_t)(ipaddr);
	_SwsSockNetMask[2] = (uint8_t)(ipaddr >> 8);
	_SwsSockNetMask[1] = (uint8_t)(ipaddr >> 16);
	_SwsSockNetMask[0] = (uint8_t)(ipaddr >> 24);
	
	return (uint8_t*)_SwsSockNetMask;
}

SWS_u_long SwsSockClass::getULGatewayIp()
{
	SWS_u_long ipaddr;
	size_t len;
	
	len = sizeof(ipaddr);
	if (SWS_DatalinkGetProperty(1, SWS_kDatalinkProperty_Gateway, &ipaddr, &len) != 0) {
		ipaddr = 0UL;
	}
	
	return ipaddr;
}

uint8_t *SwsSockClass::getGatewayIp()
{
	SWS_u_long ipaddr = getULGatewayIp();
	
	_SwsSockGateway[3] = (uint8_t)(ipaddr);
	_SwsSockGateway[2] = (uint8_t)(ipaddr >> 8);
	_SwsSockGateway[1] = (uint8_t)(ipaddr >> 16);
	_SwsSockGateway[0] = (uint8_t)(ipaddr >> 24);
	
	return (uint8_t*)_SwsSockGateway;
}

SWS_u_long SwsSockClass::getULDnsServerIp()
{
	return SWS_DnsGetPriServer();
}

uint8_t *SwsSockClass::getDnsServerIp()
{
	SWS_u_long ipaddr = getULDnsServerIp();
	
	_SwsSockDnsServerIp[3] = (uint8_t)(ipaddr);
	_SwsSockDnsServerIp[2] = (uint8_t)(ipaddr >> 8);
	_SwsSockDnsServerIp[1] = (uint8_t)(ipaddr >> 16);
	_SwsSockDnsServerIp[0] = (uint8_t)(ipaddr >> 24);
	
	return (uint8_t*)_SwsSockDnsServerIp;
}

void SwsSockClass::setIPAddress(uint8_t *ip)
{
	_SwsSockLocalIp[0] = ip[0];
	_SwsSockLocalIp[1] = ip[1];
	_SwsSockLocalIp[2] = ip[2];
	_SwsSockLocalIp[3] = ip[3];
}

void SwsSockClass::setGatewayIp(uint8_t *ip)
{
	_SwsSockGateway[0] = ip[0];
	_SwsSockGateway[1] = ip[1];
	_SwsSockGateway[2] = ip[2];
	_SwsSockGateway[3] = ip[3];
}

void SwsSockClass::setSubnetMask(uint8_t *ip)
{
	_SwsSockNetMask[0] = ip[0];
	_SwsSockNetMask[1] = ip[1];
	_SwsSockNetMask[2] = ip[2];
	_SwsSockNetMask[3] = ip[3];
}

void SwsSockClass::setDnsServerIp(uint8_t *ip)
{
	_SwsSockDnsServerIp[0] = ip[0];
	_SwsSockDnsServerIp[1] = ip[1];
	_SwsSockDnsServerIp[2] = ip[2];
	_SwsSockDnsServerIp[3] = ip[3];
}

void SwsSockClass::getIPAddress(uint8_t *ip)
{
	ip[0] = _SwsSockLocalIp[0];
	ip[1] = _SwsSockLocalIp[1];
	ip[2] = _SwsSockLocalIp[2];
	ip[3] = _SwsSockLocalIp[3];
}

void SwsSockClass::getGatewayIp(uint8_t *ip)
{
	ip[0] = _SwsSockGateway[0];
	ip[1] = _SwsSockGateway[1];
	ip[2] = _SwsSockGateway[2];
	ip[3] = _SwsSockGateway[3];
}

void SwsSockClass::getSubnetMask(uint8_t *ip)
{
	ip[0] = _SwsSockNetMask[0];
	ip[1] = _SwsSockNetMask[1];
	ip[2] = _SwsSockNetMask[2];
	ip[3] = _SwsSockNetMask[3];
}

void SwsSockClass::getDnsServerIp(uint8_t *ip)
{
	ip[0] = _SwsSockDnsServerIp[0];
	ip[1] = _SwsSockDnsServerIp[1];
	ip[2] = _SwsSockDnsServerIp[2];
	ip[3] = _SwsSockDnsServerIp[3];
}

void SwsSockClass::init()
{
	FILE *opt;
	
	opt = fopen(SWSSOCK_CFG, "w");
	
	if (opt == NULL)
		return;
	
	fprintf(opt, (inited == 0) ? "[PacketDriverInterface1]\n" : "[PacketDriverInterface2]\n");
	fprintf(opt, "ipaddress=%d.%d.%d.%d\n", _SwsSockLocalIp[0], _SwsSockLocalIp[1], _SwsSockLocalIp[2], _SwsSockLocalIp[3]);
	fprintf(opt, "ipsubnet=%d.%d.%d.%d\n", _SwsSockNetMask[0], _SwsSockNetMask[1], _SwsSockNetMask[2], _SwsSockNetMask[3]);
	fprintf(opt, "ipgateway=%d.%d.%d.%d\n", _SwsSockGateway[0], _SwsSockGateway[1], _SwsSockGateway[2], _SwsSockGateway[3]);
	fprintf(opt, "dhcp=0\n");
	fprintf(opt, "\n");
	fprintf(opt, "[dns]\n");
	fprintf(opt, "hostname=86Duino\n");
	fprintf(opt, "server=%d.%d.%d.%d\n", _SwsSockDnsServerIp[0], _SwsSockDnsServerIp[1], _SwsSockDnsServerIp[2], _SwsSockDnsServerIp[3]);
	fflush(opt);
	fclose(opt);

	if (SWS_CfgSetName(SWSSOCK_CFG, NULL) == 0)
		return;

	inited = 1;
	if (SWS_SockStartup(0, NULL)) {
		SWS_SockExit();
		return;
	}
}



