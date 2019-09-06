/*
  modbus-serial.h - Modbus Serial Library for libmodbus-dmp
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

#ifndef __MODBUS_SERIAL_H
#define __MODBUS_SERIAL_H

ssize_t _modbus_serial_send(modbus_t *ctx, const uint8_t *req, int req_length);
ssize_t _modbus_serial_recv(modbus_t *ctx, uint8_t *rsp, int rsp_length);
int _modbus_serial_set_slave(modbus_t *ctx, int slave);
int _modbus_serial_receive(modbus_t *ctx, uint8_t *req);
int _modbus_serial_flush(modbus_t *ctx);
int _modbus_serial_connect(modbus_t *ctx);
int _modbus_serial_connect_assign(modbus_t *ctx, void *port);
int _modbus_serial_available(modbus_t *ctx);
void _modbus_serial_close(modbus_t *ctx);
void _modbus_serial_free(modbus_t *ctx);
modbus_t* modbus_new_serial(const modbus_backend_t *modbus_backend, const char *device,
                            int baud, char parity, int data_bit, int stop_bit);

#endif
