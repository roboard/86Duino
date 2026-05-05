/*
  ModbusSlave.h - 86Duino Modbus slave library for communicating with
  Modbus master over RS232/485/Ethernet (via RTU/ASCII/TCP protocol).
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

  ( The API of the ModbusSlave library is designed by referring to both 
    smarmengol's Modbus Slave library:
    https://github.com/smarmengol/Modbus-Master-Slave-for-Arduino
    and Yaacov Zamir's ModbusSlave library:
    https://github.com/yaacov/ArduinoModbusSlave )
*/

#ifndef ModbusSlave_h
#define ModbusSlave_h

#include "SerialInterface.h"
#include "Ethernet.h"
#include "ModbusCommon.h"

/* Modbus Callback Function Index */
#define MODBUS_CB_NUM                       (9)
#define MODBUS_CB_READ_COILS                (0)
#define MODBUS_CB_READ_DISCRETE_INPUTS      (1)
#define MODBUS_CB_READ_HOLDING_REGISTERS    (2)
#define MODBUS_CB_READ_INPUT_REGISTERS      (3)
#define MODBUS_CB_WRITE_SINGLE_COIL         (4)
#define MODBUS_CB_WRITE_SINGLE_REGISTER     (5)
#define MODBUS_CB_WRITE_MULTIPLE_COILS      (6)
#define MODBUS_CB_WRITE_MULTIPLE_REGISTERS  (7)
#define MODBUS_CB_MASK_WRITE_REGISTER       (8)

/* Modbus Coil ON/OFF */
#define MODBUS_COIL_OFF                     (0)
#define MODBUS_COIL_ON                      (1)

class ModbusSlaveNode;

class ModbusSlave
{
public:
    ModbusSlave();
    ~ModbusSlave();

    bool begin(int mode, SerialInterface &serial);
    bool begin(int mode, uint16_t port = 502);
    bool begin(int mode, uint16_t port, EthernetClass &ethernet);
    void end();

    void setTimeout(uint32_t milliseconds);

private:
    void *_privateData;
    int getModbusMode();
    int poll();
    int poll(void *);
    void addNode(void *);

    friend class ModbusSlaveNode;
};

class ModbusSlaveNode
{
public:
    ModbusSlaveNode();
    ~ModbusSlaveNode();

    bool attach(ModbusSlave &bus);
    bool attach(uint8_t slave_id, ModbusSlave &bus);
    bool begin(ModbusSlave &bus);
    bool begin(uint8_t slave_id, ModbusSlave &bus);
    void detach();

    /* Modbus Slave Polling Function */
    int poll();
    int poll(uint16_t *buf, uint16_t size);

    /* Modbus Callback Functions */
    uint8_t (*cbFunc[MODBUS_CB_NUM])(uint8_t, uint16_t, uint16_t);

    /* Read/Write Coil Table */
    int readCoil(uint16_t address, uint16_t size, uint16_t *buffer);
    int readCoil(uint16_t address);
    int writeCoil(uint16_t address, uint16_t size, uint16_t *buffer);
    int writeCoil(uint16_t address, uint16_t value);

    /* Write Discrete Input Table */
    int writeDiscreteInput(uint16_t address, uint16_t size, uint16_t *buffer);
    int writeDiscreteInput(uint16_t address, uint16_t value);

    /* Read/Write Holding Register Table */
    int readHoldingRegister(uint16_t address, uint16_t size, uint16_t *buffer);
    int readHoldingRegister(uint16_t address);
    int writeHoldingRegister(uint16_t address, uint16_t size, uint16_t *buffer);
    int writeHoldingRegister(uint16_t address, uint16_t value);

    /* Write Input Register Table */
    int writeInputRegister(uint16_t address, uint16_t size, uint16_t *buffer);
    int writeInputRegister(uint16_t address, uint16_t value);

private:
    void *_privateData;
};

#endif
