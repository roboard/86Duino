/*
  modbus-tcp.h - Modbus TCP Library for libmodbus-dmp
  Copyright (c) 2018 Johnson Hung <Dyhung@dmp.com.tw>. All right reserved.

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
  
  (libmodbus-dmp is derived from libmodbus: http://libmodbus.org/ )
*/

#ifndef __MODBUS_TCP_H
#define __MODBUS_TCP_H

#define MODBUS_TCP_DEFAULT_PORT         (502)
#define MODBUS_TCP_SLAVE                (0xFF)
#define MODBUS_TCP_MAX_ADU_LENGTH       (260)

#define _MODBUS_TCP_HEADER_LENGTH       (7)
#define _MODBUS_TCP_PRESET_REQ_LENGTH   (12)
#define _MODBUS_TCP_PRESET_RSP_LENGTH   (8)
#define _MODBUS_TCP_CHECKSUM_LENGTH     (0)

modbus_t* modbus_new_tcp(const char *ip_address, int port);
int modbus_tcp_listen(modbus_t *ctx, int nb_connection);
int modbus_tcp_accept(modbus_t *ctx, int *s);
int modbus_tcp_select(modbus_t *ctx);

int modbus_tcp_config(char *path, char *ip_address, 
                      char *subnet, char *gateway, char *dns);
void modbus_tcp_unconfig();

#endif
