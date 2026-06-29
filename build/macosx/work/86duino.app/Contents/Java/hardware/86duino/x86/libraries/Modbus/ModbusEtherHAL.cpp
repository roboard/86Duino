/*
  ModbusEtherHAL.cpp - Modbus Ether HAL library.
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

#include "ModbusEtherHAL.h"
#include "Ethernet.h"
#include <stdint.h>

int modbus_ether_hal_connect(void *user_data, const char *ipaddress, uint16_t port)
{
    modbus_ether_hal_t *ether = (modbus_ether_hal_t *)user_data;
    int octet1, octet2, octet3, octet4;

    ether->pClient = &ether->Client;
    sscanf(ipaddress, "%d.%d.%d.%d", &octet1, &octet2, &octet3, &octet4);

    IPAddress ip(octet1, octet2, octet3, octet4);

    return (ether->Client.connect(ip, port) > 0) ? (0) : (-1);
}

int modbus_ether_hal_disconnect(void *user_data)
{
    modbus_ether_hal_t *ether = (modbus_ether_hal_t *)user_data;

    if (ether->Server == true)
        ether->Server.stop();
    else
        ether->Client.stop();

    return 0;
}

int modbus_ether_hal_available(void *user_data)
{
    modbus_ether_hal_t *ether = (modbus_ether_hal_t *)user_data;

    if (ether->pClient == NULL)
        return 0;

    return ether->pClient->available();
}

int modbus_ether_hal_flush(void *user_data)
{
    modbus_ether_hal_t *ether = (modbus_ether_hal_t *)user_data;

    if (ether->pClient)
        ether->pClient->flush();

    return 0;
}

int modbus_ether_hal_write(void *user_data, void *buf, size_t len)
{
    modbus_ether_hal_t *ether = (modbus_ether_hal_t *)user_data;

    if (ether->pClient == NULL)
        return 0;

    return ether->pClient->write((const uint8_t *)buf, len);
}

int modbus_ether_hal_read(void *user_data, void *buf, size_t len)
{
    modbus_ether_hal_t *ether = (modbus_ether_hal_t *)user_data;

    if (ether->pClient == NULL)
        return 0;

    return ether->pClient->read((uint8_t *)buf, len);
}

int modbus_ether_hal_server_begin(void *user_data)
{
    modbus_ether_hal_t *ether = (modbus_ether_hal_t *)user_data;

    ether->Server.begin();

    return 0;
}

int modbus_ether_hal_server_available(void *user_data)
{
    modbus_ether_hal_t *ether = (modbus_ether_hal_t *)user_data;

    ether->Client = ether->Server.available();
    if (ether->Client.status() == 0)
        return -1;
    ether->pClient = &ether->Client;

    return ether->Client.getSocketNumber();
}
