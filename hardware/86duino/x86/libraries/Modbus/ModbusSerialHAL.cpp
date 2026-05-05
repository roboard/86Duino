/*
  ModbusSerialHAL.cpp - Modbus Serial HAL library.
  Copyright (c) 2019 Johnson Hung <Dyhung@dmp.com.tw>. All right reserved.

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

#include "ModbusSerialHAL.h"
#include "SerialInterface.h"
#include <stdint.h>

int modbus_serial_hal_available(void *user_data)
{
    return ((SerialInterface *)user_data)->available();
}

int modbus_serial_hal_flush(void *user_data)
{
    ((SerialInterface *)user_data)->flush(0);
    ((SerialInterface *)user_data)->flush(1);
    return 0;
}

int modbus_serial_hal_write(void *user_data, void *buf, size_t len)
{
    return ((SerialInterface *)user_data)->write((const uint8_t *)buf, len);
}

int modbus_serial_hal_read(void *user_data, void *buf, size_t len)
{
    SerialInterface *serial = (SerialInterface *)user_data;
    int n = 0;
    while (n < (int)len && serial->available() > 0)
        ((uint8_t *)buf)[n++] = serial->read();
    return n;
}
