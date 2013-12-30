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

#ifndef SWS_SOCK_COMPAT
#define SWS_SOCK_COMPAT
#endif
#include "sws_sock.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

class SwsSockClass {

public:
	SwsSockClass();
	
	int setDhcp(unsigned long, unsigned long);
	
	uint8_t *getLocalIp();
	uint8_t *getSubnetMask();
	uint8_t *getGatewayIp();
	uint8_t *getDnsServerIp();
	
	u_long getULLocalIp();
	u_long getULSubnetMask();
	u_long getULGatewayIp();
	u_long getULDnsServerIp();
	
	void setIPAddress(uint8_t*);
	void setGatewayIp(uint8_t*);
	void setSubnetMask(uint8_t*);
	void setDnsServerIp(uint8_t*);
	void getIPAddress(uint8_t*);
	void getGatewayIp(uint8_t*);
	void getSubnetMask(uint8_t*);
	void getDnsServerIp(uint8_t*);
	
	void init();

private:
	int inited;
	
	uint8_t _SwsSockLocalIp[4];
	uint8_t _SwsSockGateway[4];
	uint8_t _SwsSockNetMask[4];
	uint8_t _SwsSockDnsServerIp[4];
};

extern SwsSockClass SwsSock;


#ifdef __cplusplus
}
#endif

#endif
