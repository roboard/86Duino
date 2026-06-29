/*
 * Copyright (c) 2025 by WeiZe Hung <dyhung@dmp.com.tw>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */
#ifndef __ETHERNET_CONFIG_H
#define __ETHERNET_CONFIG_H

#ifdef DUAL_PORT_NIC
    #define MAX_NIC_NUM (2)
#else
    #define MAX_NIC_NUM (1)
#endif

#endif
