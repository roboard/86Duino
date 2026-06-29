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
#include "io.h"

#define SWSSOCK_CFG "Z:\\sws_sock.ini"

SwsSockClass SwsSock;
bool SwsSockClass::_initialized = false;
int SwsSockClass::_promiscuous = 0;
int SwsSockClass::_pktdrvidx = 1;

static inline void forcePromiscuousMode(bool enable)
{
    uint8_t address1[6], address2[6];
    uint32_t iobase;
    size_t len;
    void *pci;

    if (io_Init() == false)
        return;

    if ((pci = pci_Find(0x17F3, 0x6040)) == NULL)
        goto END;
    iobase = pci_In32(pci, 0x10);
    pci_Free(pci);

    if (iobase == 0xFFFFFFFF)
        goto END;
    if (!(iobase & 1))
        goto END;
    iobase &= 0xFFFFFF00;

    ((uint16_t *)&address1[0])[0] = io_inpw(iobase + 0x68);
    ((uint16_t *)&address1[2])[0] = io_inpw(iobase + 0x6A);
    ((uint16_t *)&address1[4])[0] = io_inpw(iobase + 0x6C);
    io_Close();

    len = sizeof(address2);
    SWS_DatalinkGetProperty(1, SWS_kDatalinkProperty_HwAddress, address2, &len);

    for (int i = 0; i < 6; i++)
    {
        if (address1[i] != address2[i])
            goto END;
    }
    io_outpw(iobase, (io_inpw(iobase) & ~(1 << 5)) | (!!enable << 5));

END:
    io_Close();
}

int SwsSockClass::init(uint32_t ip_address, uint32_t subnet, uint32_t gateway, uint32_t dns)
{
    FILE *opt;

    if (this->_initialized)
    {
        this->_initialized = false;
        SWS_SockCleanup();
    }

    if ((opt = fopen(SWSSOCK_CFG, "w")) == NULL)
        return 0;

    fprintf(opt, "[PacketDriverInterface%d]\n", this->_pktdrvidx);
    fprintf(opt, "ipaddress=%d.%d.%d.%d\n", ((uint8_t *)&ip_address)[0], ((uint8_t *)&ip_address)[1], ((uint8_t *)&ip_address)[2], ((uint8_t *)&ip_address)[3]);
    fprintf(opt, "ipsubnet=%d.%d.%d.%d\n", ((uint8_t *)&subnet)[0], ((uint8_t *)&subnet)[1], ((uint8_t *)&subnet)[2], ((uint8_t *)&subnet)[3]);
    fprintf(opt, "ipgateway=%d.%d.%d.%d\n", ((uint8_t *)&gateway)[0], ((uint8_t *)&gateway)[1], ((uint8_t *)&gateway)[2], ((uint8_t *)&gateway)[3]);
    fprintf(opt, "dhcp=0\n");
    fprintf(opt, "\n");
    fprintf(opt, "[dns]\n");
    fprintf(opt, "hostname=86Duino\n");
    fprintf(opt, "server=%d.%d.%d.%d\n", ((uint8_t *)&dns)[0], ((uint8_t *)&dns)[1], ((uint8_t *)&dns)[2], ((uint8_t *)&dns)[3]);
    fflush(opt);
    fclose(opt);

    if (SWS_CfgSetName(SWSSOCK_CFG, NULL) == 0)
        return 0;
    if (SWS_SockStartup(0, NULL))
        return 0;
    this->_pktdrvidx++;
    this->_initialized = true;

    if (getLocalIp() == 0x0100007FUL)
        return 0;

    return 1;
}

int SwsSockClass::setDhcp(unsigned long timeout, unsigned long responseTimeout)
{
    FILE *opt;

    if (this->_initialized)
    {
        this->_initialized = false;
        SWS_SockCleanup();
    }

    opt = fopen(SWSSOCK_CFG, "w");

    if (opt == NULL)
        return 0;

    fprintf(opt, "[PacketDriverInterface%d]\n", this->_pktdrvidx);
    fprintf(opt, "dhcp=1\n");
    fprintf(opt, "\n");
    fprintf(opt, "[dns]\n");
    fprintf(opt, "hostname=86Duino\n");
    fflush(opt);
    fclose(opt);

    if (SWS_CfgSetName(SWSSOCK_CFG, NULL) == 0)
        return 0;
    if (SWS_SockStartup(0, NULL))
        return 0;
    this->_pktdrvidx++;
    this->_initialized = true;

    if (getLocalIp() == 0x0100007FUL)
        return 0;

    return 1;
}

int SwsSockClass::setPromiscuousMode(bool enable)
{
    if (this->_initialized == false)
        return 0;
    if (this->_promiscuous == 0 && enable == false)
        return 0;

    this->_promiscuous += !!enable - !enable;

    if ((enable && this->_promiscuous == 1) || (!enable && this->_promiscuous == 0))
    {
        uint32_t uVal = !!enable;
        SWS_DatalinkSetProperty(1, SWS_kDatalinkProperty_Promiscuity, &uVal, sizeof(uVal));
        forcePromiscuousMode(enable);
    }

    return 0;
}

int SwsSockClass::isInitialized()
{
    return !!this->_initialized;
}

uint32_t SwsSockClass::getLocalIp()
{
    char hostname[128];
    uint32_t ipaddr = 0;

    if (this->_initialized == false)
        return 0;

    if (SWS_gethostname(hostname, sizeof(hostname)) == 0)
    {
        SWS_hostent *host = SWS_gethostbyname(hostname);
        ipaddr = *(uint32_t *)host->h_addr_list[0];
    }

    return ipaddr;
}

uint32_t SwsSockClass::getSubnetMask()
{
    uint32_t subnet = 0;
    size_t len = sizeof(subnet);

    if (this->_initialized == false)
        return 0;

    SWS_DatalinkGetProperty(1, SWS_kDatalinkProperty_SubnetMask, &subnet, &len);
    return SWS_ntohl(subnet);
}

uint32_t SwsSockClass::getGatewayIp()
{
    uint32_t gateway = 0;
    size_t len = sizeof(gateway);

    if (this->_initialized == false)
        return 0;

    SWS_DatalinkGetProperty(1, SWS_kDatalinkProperty_Gateway, &gateway, &len);
    return SWS_ntohl(gateway);
}

uint32_t SwsSockClass::getDnsServerIp()
{
    if (this->_initialized == false)
        return 0;

    return SWS_ntohl(SWS_DnsGetPriServer());
}

void SwsSockClass::getMACAddress(uint8_t *mac_address)
{
    size_t len = 6;

    if (this->_initialized == false)
    {
        memset(mac_address, 0xFF, len);
        return;
    }

    SWS_DatalinkGetProperty(1, SWS_kDatalinkProperty_HwAddress, mac_address, &len);
}
