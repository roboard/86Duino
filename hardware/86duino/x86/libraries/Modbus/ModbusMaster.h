/*
  ModbusMaster.h - 86Duino library for communicating with Modbus slaves
  over RS232/485/Ethernet (via RTU/ASCII/TCP protocol).
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
  
  ( The API of the ModbusMaster library is designed by referring to 
    Doc Walker's Arduino ModbusMaster library, which is originally released 
    under Apache License 2.0: http://github.com/4-20ma/ModbusMaster )

*/

#ifndef ModbusMaster_h
#define ModbusMaster_h

#include "HardwareSerial.h"
#include "IPAddress.h"
#include "ModbusCommon.h"

#ifndef lowWord
#define lowWord(ww) ((uint16_t) ((ww) & 0xffff))
#endif

#ifndef highWord
#define highWord(ww) ((uint16_t) ((ww) >> 16))
#endif

/* Modbus Read/Write Data Mode */
#define MODBUS_DATAMODE_BIT     (0)
#define MODBUS_DATAMODE_UINT16  (1)

class ModbusGateway;

class ModbusMaster {
    public:
        ModbusMaster();
        
        bool begin(int mode, HardwareSerial &serial);
        bool begin(int mode, IPAddress &modbus_server_ip);
        
        void setTimeout(uint32_t milliseconds);
        
        uint8_t readCoils(uint8_t slave_id, uint16_t address, uint16_t size, uint16_t *data);
        uint8_t readDiscreteInputs(uint8_t slave_id, uint16_t address, uint16_t size, uint16_t *data);
        uint8_t readHoldingRegisters(uint8_t slave_id, uint16_t address, uint16_t size, uint16_t *data);
        uint8_t readInputRegisters(uint8_t slave_id, uint16_t address, uint16_t size, uint16_t *data);
        uint8_t writeSingleCoil(uint8_t slave_id, uint16_t address, uint16_t value);
        uint8_t writeSingleRegister(uint8_t slave_id, uint16_t address, uint16_t value);
        uint8_t writeMultipleCoils(uint8_t slave_id, uint16_t address, uint16_t size, uint16_t *data);
        uint8_t writeMultipleRegisters(uint8_t slave_id, uint16_t address, uint16_t size, uint16_t *data);
        uint8_t maskWriteRegister(uint8_t slave_id, uint16_t address, uint16_t and_mask, uint16_t or_mask);
        uint8_t readWriteMultipleRegisters(uint8_t slave_id,
                                           uint16_t write_address, uint16_t write_size, uint16_t *write_data,
                                           uint16_t read_address, uint16_t read_size, uint16_t *read_data);
        
    private:
        enum { MODBUS_BIT_BUFFER_SIZE = 0x7B0 };
        
        unsigned long timeout;
        
        ModbusGateway *gateway;
        
        int _mode;
        void *mbus;
        uint8_t bitData[MODBUS_BIT_BUFFER_SIZE];
        
        uint8_t getErrorNumber(int err);
        
        friend class ModbusMasterNode;
        friend class ModbusGateway;
};

class ModbusMasterNode {
    public:
        ModbusMasterNode();
        
        bool begin(ModbusMaster &bus);
        bool begin(uint8_t slave_id, ModbusMaster &bus);
        
        uint16_t getResponseBuffer(uint8_t, int mode = MODBUS_DATAMODE_UINT16);
        void     clearResponseBuffer();
        uint8_t  setTransmitBuffer(uint8_t, uint16_t, int mode = MODBUS_DATAMODE_UINT16);
        void     clearTransmitBuffer();

        void beginTransmission(uint16_t);
        void sendBit(bool);
        void send(uint8_t);
        void send(uint16_t);
        void send(uint32_t);
        uint8_t available(void);
        uint16_t receive(void);

        uint8_t readCoils(uint16_t, uint16_t);
        uint8_t readDiscreteInputs(uint16_t, uint16_t);
        uint8_t readHoldingRegisters(uint16_t, uint16_t);
        uint8_t readInputRegisters(uint16_t, uint16_t);
        uint8_t writeSingleCoil(uint16_t, uint16_t);
        uint8_t writeSingleRegister(uint16_t, uint16_t);
        uint8_t writeMultipleCoils(uint16_t, uint16_t);
        uint8_t writeMultipleCoils();
        uint8_t writeMultipleRegisters(uint16_t, uint16_t);
        uint8_t writeMultipleRegisters();
        uint8_t maskWriteRegister(uint16_t, uint16_t, uint16_t);
        uint8_t readWriteMultipleRegisters(uint16_t, uint16_t, uint16_t, uint16_t);
        uint8_t readWriteMultipleRegisters(uint16_t, uint16_t);

    private:
        enum { MODBUS_BUFFER_SIZE = 0x7B };
    
        ModbusMaster *master;
        uint8_t slaveID;

        uint16_t readData[MODBUS_BUFFER_SIZE];
        uint8_t  readSize;
        uint8_t  readIndex;
        
        uint16_t writeData[MODBUS_BUFFER_SIZE];
        uint16_t writeAddress;
        uint8_t  writeSize;
        uint8_t  writeCoilSize;
        
        friend class ModbusMaster;
        friend class ModbusGateway;
};

#endif
