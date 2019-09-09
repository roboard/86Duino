/*
  modbus-rtu.h - Modbus RTU Library for libmodbus-dmp
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

#ifndef __MODBUS_RTU_H
#define __MODBUS_RTU_H

#define MODBUS_RTU_MAX_ADU_LENGTH       (256)

#define _MODBUS_RTU_HEADER_LENGTH       (1)
#define _MODBUS_RTU_PRESET_REQ_LENGTH   (6)
#define _MODBUS_RTU_PRESET_RSP_LENGTH   (2)
#define _MODBUS_RTU_CHECKSUM_LENGTH     (2)

modbus_t* modbus_new_rtu(const char *device, int baud, char parity,
                                    int data_bit, int stop_bit);

#endif