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

#include "HardwareSerial.h"
#include "ModbusCommon.h"

/* Modbus Callback Function Index */
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

class ModbusSlave {
    public:
        ModbusSlave();
        
        bool begin(int mode, HardwareSerial &serial);
        bool begin(int mode);
        
        void setTimeout(uint32_t milliseconds);

    private:
        /* Modbus Maximum Message Size */
        enum { MODBUS_MAX_MESSAGE_LENGTH = 513 };

        bool initialize();
        
        int poll();
        int poll(ModbusSlaveNode *node);
        
        void addNode(ModbusSlaveNode *node);
        
        ModbusSlaveNode *headNode;
        
        int pollState;
        unsigned long timeout;
        unsigned long recvTime;

        uint8_t req[MODBUS_MAX_MESSAGE_LENGTH];
        int req_length;
        int handle_num;
        
        int _mode;
        void *mbus;
        
        friend class ModbusSlaveNode;
};

class ModbusSlaveNode {
    private:
        enum { MODBUS_CB_NUM = 9,
               MODBUS_MAX_TABLE_SIZE = 64 * 1024 };

        ModbusSlaveNode *nextNode;
        int handled;
        ModbusSlave *slave;
        uint8_t slaveID;
        
        void *table;
        
        uint16_t coils[MODBUS_MAX_TABLE_SIZE];
        uint16_t discrete_inputs[MODBUS_MAX_TABLE_SIZE];
        uint16_t holding_registers[MODBUS_MAX_TABLE_SIZE];
        uint16_t input_registers[MODBUS_MAX_TABLE_SIZE];

        friend class ModbusSlave;
        
    public:
        ModbusSlaveNode();
        
        bool begin(ModbusSlave &bus);
        bool begin(uint8_t slave_id, ModbusSlave &bus);
        
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
};

#endif
