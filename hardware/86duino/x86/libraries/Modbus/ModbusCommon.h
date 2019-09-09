/*
  ModbusCommon.h - 86Duino Modbus library.
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

#ifndef ModbusCommon_h
#define ModbusCommon_h

/* Modbus Mode */
#define MODBUS_RTU                      (0)
#define MODBUS_ASCII                    (1)
#define MODBUS_TCP                      (2)

/* Modbus Function Code */
#define MODBUS_READ_COILS               (0x01)
#define MODBUS_READ_DISCRETE_INPUTS     (0x02)
#define MODBUS_READ_HOLDING_REGISTERS   (0x03)
#define MODBUS_READ_INPUT_REGISTERS     (0x04)
#define MODBUS_WRITE_SINGLE_COIL        (0x05)
#define MODBUS_WRITE_SINGLE_REGISTER    (0x06)
#define MODBUS_READ_EXCEPTION_STATUS    (0x07)
#define MODBUS_WRITE_MULTIPLE_COILS     (0x0F)
#define MODBUS_WRITE_MULTIPLE_REGISTERS (0x10)
#define MODBUS_REPORT_SLAVE_ID          (0x11)
#define MODBUS_MASK_WRITE_REGISTER      (0x16)
#define MODBUS_WRITE_AND_READ_REGISTERS (0x17)
#define MODBUS_SYNC_READ_REGISTERS      (0x65)
#define MODBUS_SYNC_WRITE_REGISTERS     (0x66)

/* Modbus Exception Code */
#define MODBUS_SUCCESS                                  (0x00)
#define MODBUS_ILLEGAL_FUNCTION                         (0x01)
#define MODBUS_ILLEGAL_DATA_ADDRESS                     (0x02)
#define MODBUS_ILLEGAL_DATA_VALUE                       (0x03)
#define MODBUS_SERVER_DEVICE_FAILURE                    (0x04)
#define MODBUS_ACKNOWLEDGE                              (0x05)
#define MODBUS_SERVER_DEVICE_BUSY                       (0x06)
#define MODBUS_MEMORY_PARITY_ERROR                      (0x08)
#define MODBUS_GATEWAY_PATH_UNAVAILABLE                 (0x0A)
#define MODBUS_GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND  (0x0B)
#define MODBUS_INVALID_SLAVE_ID                         (0xE0)
#define MODBUS_INVALID_FUNCTION                         (0xE1)
#define MODBUS_RESPONSE_TIMEOUT                         (0xE2)
#define MODBUS_INVALID_CRC                              (0xE3)
#define MODBUS_ILLEGAL_DATA_MODE                        (0xE4)
#define MODBUS_INVALID_BUS                              (0xE5)

#endif
