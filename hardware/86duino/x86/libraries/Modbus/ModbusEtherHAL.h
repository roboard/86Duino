/*
  ModbusEtherHAL.h - Modbus Ether HAL library.
  Copyright (c) 2025 Johnson Hung <Dyhung@dmp.com.tw>. All right reserved.

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

#ifndef ModbusEtherHAL_h
#define ModbusEtherHAL_h

#include "Ethernet.h"
#include <stdlib.h>
#include <stdint.h>

struct modbus_ether_hal_t
{
    EthernetServer Server;
    EthernetClient Client;
    EthernetClient *pClient;

    modbus_ether_hal_t(uint16_t port, EthernetClass &ethernet)
        : Server(port, ethernet), Client(ethernet), pClient(NULL)  {}
};

int modbus_ether_hal_connect(void *user_data, const char *ipaddress, uint16_t port);
int modbus_ether_hal_disconnect(void *user_data);
int modbus_ether_hal_available(void *user_data);
int modbus_ether_hal_flush(void *user_data);
int modbus_ether_hal_write(void *user_data, void *buf, size_t len);
int modbus_ether_hal_read(void *user_data, void *buf, size_t len);
int modbus_ether_hal_server_begin(void *user_data);
int modbus_ether_hal_server_available(void *user_data);

#endif