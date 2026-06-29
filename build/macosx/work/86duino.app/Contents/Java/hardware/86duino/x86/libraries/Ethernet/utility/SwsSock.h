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

#include <stdint.h>
#include "sws_sock.h"

class SwsSockClass
{
public:
    int init(uint32_t ip_address, uint32_t subnet, uint32_t gateway, uint32_t dns);
    int setDhcp(unsigned long, unsigned long);
    int setPromiscuousMode(bool enable);
    int isInitialized();

    void getMACAddress(uint8_t *mac_address);
    uint32_t getLocalIp();
    uint32_t getSubnetMask();
    uint32_t getGatewayIp();
    uint32_t getDnsServerIp();

private:
    static bool _initialized;
    static int _promiscuous;
    static int _pktdrvidx;

};

extern SwsSockClass SwsSock;

#endif
