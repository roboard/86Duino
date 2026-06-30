/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 *
 * Modified 10 December 2013 by DY Hung
 */
#ifndef __DMP_SWSSOCK_H
#define __DMP_SWSSOCK_H

#include "sws_nif.h"
#include <stdint.h>
#include <string>

#define NIC_UNKNOWN (0)
#define NIC_R6040   (1)
#define NIC_OTHERS  (2)
#define NIC_VNET    (3)
#define NIC_EOE     (4)
#define NIC_BRIDGE  (5)

enum EnetLinkstatus {
    UNKNOWN,
    LINK_ON,
    LINK_OFF
};

class SwsSockClass
{
public:
    SwsSockClass(NetworkInterface *iface);
    SwsSockClass(const char *ifname);
    SwsSockClass(const char *ifname, NetworkInterface *iface);
    ~SwsSockClass();

    int init(uint8_t *mac, uint32_t ip_address, uint32_t subnet, uint32_t gateway, uint32_t dns);
    int init(uint8_t *mac);
    int isInitialized();
    int getChip();
    int setPromiscuousMode(bool enable);
    int setMulticast(bool enable);
    EnetLinkstatus getLinkStatus();

    void getMACAddress(uint8_t *mac_address);
    uint32_t getLocalIP();
    uint32_t getSubnetMask();
    uint32_t getGatewayIP();
    uint32_t getDnsServerIP();

    void setMACAddress(uint8_t *mac_address);
    void setLocalIP(uint32_t local_ip);
    void setSubnetMask(uint32_t subnet);
    void setGatewayIP(uint32_t gateway);
    void setDnsServerIP(uint32_t dns_server);

private:
    void close();
    NetworkInterface *_iface;
    std::string _ifname;
    static bool _vnetStartup;
    static bool _swsStartup;
    void *_privateData;
};

#endif
