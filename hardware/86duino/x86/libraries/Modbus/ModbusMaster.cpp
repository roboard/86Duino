/*
  ModbusMaster.cpp - 86Duino library for communicating with Modbus slaves
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
  
  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)

*/

#include <errno.h>

#include "ModbusMaster.h"
#include "ModbusSerialHAL.h"
#include "ModbusEtherHAL.h"
#include "modbus-core.h"
#include <stdlib.h>
#include <string.h>

#define TIME_SEC                (1)
#define TIME_USEC               (0)

struct modbus_master_t
{
    modbus_hal_t hal;
    modbus_t *ctx;
    int mode;

    uint32_t timeout;
    uint8_t bitData[2000];
};

ModbusMaster::ModbusMaster()
{
    this->_privateData = NULL;
    this->gateway = NULL;
}

ModbusMaster::~ModbusMaster()
{
    end();
}

bool ModbusMaster::begin(int mode, SerialInterface &serial)
{
    modbus_master_t *master;

    if (mode != MODBUS_RTU && mode != MODBUS_ASCII)
        return false;

    if ((master = (modbus_master_t *)malloc(sizeof(*master))) == NULL)
        return false;
    memset(master, 0, sizeof(*master));
    master->mode = mode;
    master->timeout = 1000 * TIME_SEC + TIME_USEC / 1000;
    master->hal.available = modbus_serial_hal_available;
    master->hal.flush = modbus_serial_hal_flush;
    master->hal.write = modbus_serial_hal_write;
    master->hal.read = modbus_serial_hal_read;
    master->hal.user_data = &serial;

    switch (master->mode) {
    case MODBUS_RTU:
        master->ctx = modbus_new_rtu_ext(&master->hal);
        break;
    case MODBUS_ASCII:
        master->ctx = modbus_new_ascii_ext(&master->hal);
        break;
    }
    if (master->ctx == NULL) {
        free(master);
        return false;
    }
    modbus_set_response_timeout(master->ctx, TIME_SEC, TIME_USEC);
    serial.setTimeout(0, 0);
    this->_privateData = master;

    return true;
}

bool ModbusMaster::begin(int mode, IPAddress &modbus_server_ip, uint16_t modbus_server_port)
{
    return begin(mode, modbus_server_ip, modbus_server_port, Ethernet);
}

bool ModbusMaster::begin(int mode, IPAddress &modbus_server_ip, uint16_t modbus_server_port, EthernetClass &ethernet)
{
    modbus_master_t *master;
    modbus_ether_hal_t *ether;
    char server_ip[16];

    if (mode != MODBUS_TCP)
        return false;

    if ((ether = new modbus_ether_hal_t(502, ethernet)) == NULL)
        return false;
    ether->pClient = NULL;

    if ((master = (modbus_master_t *)malloc(sizeof(*master))) == NULL) {
        delete ether;
        return false;
    }
    memset(master, 0, sizeof(*master));
    master->mode = mode;
    master->timeout = 1000 * TIME_SEC + TIME_USEC / 1000;
    master->hal.user_data = ether;
    master->hal.connect = modbus_ether_hal_connect;
    master->hal.disconnect = modbus_ether_hal_disconnect;
    master->hal.available = modbus_ether_hal_available;
    master->hal.flush = modbus_ether_hal_flush;
    master->hal.write = modbus_ether_hal_write;
    master->hal.read = modbus_ether_hal_read;
    master->hal.server_begin = modbus_ether_hal_server_begin;
    master->hal.server_available = modbus_ether_hal_server_available;

    sprintf(server_ip, "%d.%d.%d.%d", modbus_server_ip[0]
                                    , modbus_server_ip[1]
                                    , modbus_server_ip[2]
                                    , modbus_server_ip[3]);
    if ((master->ctx = modbus_new_tcp_ext(&master->hal, server_ip, modbus_server_port)) == NULL) {
        free(master);
        delete ether;
        return false;
    }
    if (modbus_connect(master->ctx) == -1) {
        modbus_free(master->ctx);
        free(master);
        delete ether;
        return false;
    }
    modbus_set_response_timeout(master->ctx, TIME_SEC, TIME_USEC);
    this->_privateData = master;

    return true;
}

void ModbusMaster::end()
{
    modbus_master_t *master = (modbus_master_t *)this->_privateData;

    if (master == NULL)
        return;

    modbus_close(master->ctx);
    modbus_free(master->ctx);
    switch (master->mode) {
    case MODBUS_RTU:
    case MODBUS_ASCII:
        break;
    case MODBUS_TCP:
        delete (modbus_ether_hal_t *)master->hal.user_data;
        break;
    }
    free(master);
    this->_privateData = NULL;
    this->gateway = NULL;
}

void *ModbusMaster::getModbusHandle()
{
    modbus_master_t *master = (modbus_master_t *)this->_privateData;
    if (master == NULL)
        return NULL;
    return master->ctx;
}

int ModbusMaster::getModbusMode()
{
    modbus_master_t *master = (modbus_master_t *)this->_privateData;
    if (master == NULL)
        return MODBUS_RTU;
    return master->mode;
}

uint32_t ModbusMaster::getTimeout()
{
    modbus_master_t *master = (modbus_master_t *)this->_privateData;
    if (master == NULL)
        return 0;
    return master->timeout;
}

void ModbusMaster::setTimeout(uint32_t milliseconds)
{
    modbus_master_t *master = (modbus_master_t *)this->_privateData;
    uint32_t sec, usec;

    if (master == NULL)
        return;

    master->timeout = milliseconds;
    sec = master->timeout / 1000;
    usec = (master->timeout % 1000) * 1000;

    modbus_set_response_timeout(master->ctx, sec, usec);
}

uint8_t ModbusMaster::getErrorNumber(int err)
{
    uint8_t status = MODBUS_RESPONSE_TIMEOUT;
    
    switch (err) {
    case EMBXILFUN:
        status = MODBUS_ILLEGAL_FUNCTION;
        break;
    case EMBXILADD:
        status = MODBUS_ILLEGAL_DATA_ADDRESS;
        break;
    case EMBXILVAL:
        status = MODBUS_ILLEGAL_DATA_VALUE;
        break;
    case EMBXSFAIL:
        status = MODBUS_SERVER_DEVICE_FAILURE;
        break;
    case EMBBADSLAVE:
        status = MODBUS_INVALID_SLAVE_ID;
        break;
    case EMBBADDATA:
        status = MODBUS_INVALID_FUNCTION;
        break;
    case EMBBADCRC:
        status = MODBUS_INVALID_CRC;
        break;
    };
    
    return status;
}

uint8_t ModbusMaster::readCoils(uint8_t slave_id, uint16_t address, uint16_t size, uint16_t *data)
{
    modbus_master_t *master = (modbus_master_t *)this->_privateData;
    int rc;

    if (master == NULL)
        return MODBUS_INVALID_BUS;

    rc = modbus_set_slave(master->ctx, slave_id);
    if (rc < 0)
        return MODBUS_INVALID_SLAVE_ID;

    rc = modbus_read_bits(master->ctx, address, size, master->bitData);
    if ((slave_id == MODBUS_BROADCAST_ADDRESS && rc <= 0) ||
        (slave_id != MODBUS_BROADCAST_ADDRESS && rc != size)) {
        modbus_flush(master->ctx);
        return getErrorNumber(errno);
    }

    for (int i = 0; i < size; i++) {
        int m = i >> 4;
        int n = i & 0x0F;
        if (master->bitData[i])
            data[m] |= (1 << n);
        else
            data[m] &= ~(1 << n);
    }

    return MODBUS_SUCCESS;
}

uint8_t ModbusMaster::readDiscreteInputs(uint8_t slave_id, uint16_t address, uint16_t size, uint16_t *data)
{
    modbus_master_t *master = (modbus_master_t *)this->_privateData;
    int rc;

    if (master == NULL)
        return MODBUS_INVALID_BUS;

    rc = modbus_set_slave(master->ctx, slave_id);
    if (rc < 0)
        return MODBUS_INVALID_SLAVE_ID;

    rc = modbus_read_input_bits(master->ctx, address, size, master->bitData);
    if ((slave_id == MODBUS_BROADCAST_ADDRESS && rc <= 0) ||
        (slave_id != MODBUS_BROADCAST_ADDRESS && rc != size)) {
        modbus_flush(master->ctx);
        return getErrorNumber(errno);
    }

    for (int i = 0; i < size; i++) {
        int m = i >> 4;
        int n = i & 0x0F;
        if (master->bitData[i])
            data[m] |= (1 << n);
        else
            data[m] &= ~(1 << n);
    }

    return MODBUS_SUCCESS;
}

uint8_t ModbusMaster::readHoldingRegisters(uint8_t slave_id, uint16_t address, uint16_t size, uint16_t *data)
{
    modbus_master_t *master = (modbus_master_t *)this->_privateData;
    int rc;

    if (master == NULL)
        return MODBUS_INVALID_BUS;

    rc = modbus_set_slave(master->ctx, slave_id);
    if (rc < 0)
        return MODBUS_INVALID_SLAVE_ID;

    rc = modbus_read_registers(master->ctx, address, size, data);
    if ((slave_id == MODBUS_BROADCAST_ADDRESS && rc <= 0) ||
        (slave_id != MODBUS_BROADCAST_ADDRESS && rc != size)) {
        modbus_flush(master->ctx);
        return getErrorNumber(errno);
    }

    return MODBUS_SUCCESS;
}

uint8_t ModbusMaster::readInputRegisters(uint8_t slave_id, uint16_t address, uint16_t size, uint16_t *data)
{
    modbus_master_t *master = (modbus_master_t *)this->_privateData;
    int rc;

    if (master == NULL)
        return MODBUS_INVALID_BUS;

    rc = modbus_set_slave(master->ctx, slave_id);
    if (rc < 0)
        return MODBUS_INVALID_SLAVE_ID;

    rc = modbus_read_input_registers(master->ctx, address, size, data);
    if ((slave_id == MODBUS_BROADCAST_ADDRESS && rc <= 0) ||
        (slave_id != MODBUS_BROADCAST_ADDRESS && rc != size)) {
        modbus_flush(master->ctx);
        return getErrorNumber(errno);
    }

    return MODBUS_SUCCESS;
}

uint8_t ModbusMaster::writeSingleCoil(uint8_t slave_id, uint16_t address, uint16_t value)
{
    modbus_master_t *master = (modbus_master_t *)this->_privateData;
    int rc;

    if (master == NULL)
        return MODBUS_INVALID_BUS;

    rc = modbus_set_slave(master->ctx, slave_id);
    if (rc < 0)
        return MODBUS_INVALID_SLAVE_ID;
    
    rc = modbus_write_bit(master->ctx, address, value);
    if ((slave_id == MODBUS_BROADCAST_ADDRESS && rc <= 0) ||
        (slave_id != MODBUS_BROADCAST_ADDRESS && rc != 1)) {
        modbus_flush(master->ctx);
        return getErrorNumber(errno);
    }
    
    return MODBUS_SUCCESS;
}

uint8_t ModbusMaster::writeSingleRegister(uint8_t slave_id, uint16_t address, uint16_t value)
{
    modbus_master_t *master = (modbus_master_t *)this->_privateData;
    int rc;

    if (master == NULL)
        return MODBUS_INVALID_BUS;

    rc = modbus_set_slave(master->ctx, slave_id);
    if (rc < 0)
        return MODBUS_INVALID_SLAVE_ID;
    
    rc = modbus_write_register(master->ctx, address, value);
    if ((slave_id == MODBUS_BROADCAST_ADDRESS && rc <= 0) ||
        (slave_id != MODBUS_BROADCAST_ADDRESS && rc != 1)) {
        modbus_flush(master->ctx);
        return getErrorNumber(errno);
    }
    
    return MODBUS_SUCCESS;
}

uint8_t ModbusMaster::writeMultipleCoils(uint8_t slave_id, uint16_t address, uint16_t size, uint16_t *data)
{
    modbus_master_t *master = (modbus_master_t *)this->_privateData;
    int rc;

    if (master == NULL)
        return MODBUS_INVALID_BUS;

    rc = modbus_set_slave(master->ctx, slave_id);
    if (rc < 0)
        return MODBUS_INVALID_SLAVE_ID;
    
    for (int i = 0; i < size; i++) {
        int m = i >> 4;
        int n = i & 0x0F;
        master->bitData[i] = (data[m] & (1 << n)) ? 1 : 0;
    }
    
    rc = modbus_write_bits(master->ctx, address, size, master->bitData);
    if ((slave_id == MODBUS_BROADCAST_ADDRESS && rc <= 0) ||
        (slave_id != MODBUS_BROADCAST_ADDRESS && rc != size)) {
        modbus_flush(master->ctx);
        return getErrorNumber(errno);
    }
    
    return MODBUS_SUCCESS;
}

uint8_t ModbusMaster::writeMultipleRegisters(uint8_t slave_id, uint16_t address, uint16_t size, uint16_t *data)
{
    modbus_master_t *master = (modbus_master_t *)this->_privateData;
    int rc;

    if (master == NULL)
        return MODBUS_INVALID_BUS;

    rc = modbus_set_slave(master->ctx, slave_id);
    if (rc < 0)
        return MODBUS_INVALID_SLAVE_ID;
    
    rc = modbus_write_registers(master->ctx, address, size, data);
    if ((slave_id == MODBUS_BROADCAST_ADDRESS && rc <= 0) ||
        (slave_id != MODBUS_BROADCAST_ADDRESS && rc != size)) {
        modbus_flush(master->ctx);
        return getErrorNumber(errno);
    }
    
    return MODBUS_SUCCESS;
}

uint8_t ModbusMaster::maskWriteRegister(uint8_t slave_id, uint16_t address, uint16_t and_mask, uint16_t or_mask)
{
    modbus_master_t *master = (modbus_master_t *)this->_privateData;
    int rc;

    if (master == NULL)
        return MODBUS_INVALID_BUS;

    rc = modbus_set_slave(master->ctx, slave_id);
    if (rc < 0)
        return MODBUS_INVALID_SLAVE_ID;
    
    rc = modbus_mask_write_register(master->ctx, address, and_mask, or_mask);
    if ((slave_id == MODBUS_BROADCAST_ADDRESS && rc <= 0) ||
        (slave_id != MODBUS_BROADCAST_ADDRESS && rc != 1)) {
        modbus_flush(master->ctx);
        return getErrorNumber(errno);
    }
    
    return MODBUS_SUCCESS;
}

uint8_t ModbusMaster::readWriteMultipleRegisters(uint8_t slave_id,
                               uint16_t write_address, uint16_t write_size, uint16_t *write_data,
                               uint16_t read_address, uint16_t read_size, uint16_t *read_data)
{
    modbus_master_t *master = (modbus_master_t *)this->_privateData;
    int rc;

    if (master == NULL)
        return MODBUS_INVALID_BUS;

    rc = modbus_set_slave(master->ctx, slave_id);
    if (rc < 0)
        return MODBUS_INVALID_SLAVE_ID;
    
    rc = modbus_write_and_read_registers(master->ctx, write_address, write_size, write_data,
                                         read_address, read_size, read_data);
    if ((slave_id == MODBUS_BROADCAST_ADDRESS && rc <= 0) ||
        (slave_id != MODBUS_BROADCAST_ADDRESS && rc != read_size)) {
        modbus_flush(master->ctx);
        return getErrorNumber(errno);
    }
    
    return MODBUS_SUCCESS;
}

ModbusMasterNode::ModbusMasterNode()
{
    master = NULL;
    slaveID = MODBUS_BROADCAST_ADDRESS;
    
    readSize = 0;
    readIndex = 0;
    
    writeAddress = 0;
    writeSize = 0;
    writeCoilSize = 0;
}

bool ModbusMasterNode::attach(ModbusMaster &bus)
{
    return attach((bus.getModbusMode() == MODBUS_TCP) ? MODBUS_TCP_SLAVE : MODBUS_BROADCAST_ADDRESS, bus);
}

bool ModbusMasterNode::attach(uint8_t slave_id, ModbusMaster &bus)
{
    master = &bus;

    if (master == NULL)
        return false;

    slaveID = slave_id;

    return true;
}

bool ModbusMasterNode::begin(ModbusMaster &bus)
{
    return attach(bus);
}

bool ModbusMasterNode::begin(uint8_t slave_id, ModbusMaster &bus)
{
    return attach(slave_id, bus);
}

uint16_t ModbusMasterNode::getResponseBuffer(int index, int mode)
{
    if (mode == MODBUS_DATAMODE_BIT) {
        int i = index >> 4;
        int j = index & 0x0F;
        if (i >= readSize)
            return 0xFFFF;
        return (readData[i] & (1 << j)) ? 1 : 0;
    }
    return (index < readSize) ? readData[index] : 0xFFFF;
}

void ModbusMasterNode::clearResponseBuffer()
{
    memset(readData, 0, sizeof(readData));
}

uint8_t ModbusMasterNode::setTransmitBuffer(int index, uint16_t value, int mode)
{
    if (mode == MODBUS_DATAMODE_BIT) {
        int i = index >> 4;
        int j = index & 0x0F;
        if (i < MODBUS_BUFFER_SIZE) {
            if (value)
                writeData[i] |= (1 << j);
            else
                writeData[i] &= ~(1 << j);
            return MODBUS_SUCCESS;
        }
        return MODBUS_ILLEGAL_DATA_ADDRESS;
    }

    if (index < MODBUS_BUFFER_SIZE) {
        writeData[index] = value;
        return MODBUS_SUCCESS;
    }

    return MODBUS_ILLEGAL_DATA_ADDRESS;
}

void ModbusMasterNode::clearTransmitBuffer()
{
    memset(writeData, 0, sizeof(writeData));
}

void ModbusMasterNode::beginTransmission(uint16_t write_address)
{
    writeAddress = write_address;
    writeSize = 0;
    writeCoilSize = 0;
}

void ModbusMasterNode::sendBit(bool data)
{
    uint16_t bitIndex = writeCoilSize % 16;

    if ((writeCoilSize >> 4) < MODBUS_BUFFER_SIZE) {
        if (bitIndex == 0)
            writeData[writeSize] = 0;
        if (data)
            writeData[writeSize] |= (1 << bitIndex);
        else
            writeData[writeSize] &= ~(1 << bitIndex);
        writeCoilSize++;
        writeSize = writeCoilSize >> 4;
    }
}

void ModbusMasterNode::send(uint8_t data)
{
    uint16_t value = data;
    send(value);
}

void ModbusMasterNode::send(uint16_t data)
{
    if (writeSize < MODBUS_BUFFER_SIZE) {
        writeData[writeSize++] = data;
        writeCoilSize = writeSize << 4;
    }
}

void ModbusMasterNode::send(uint32_t data)
{
    send((uint16_t)(data & 0xFFFF));
    send((uint16_t)(data >> 16));
}

uint8_t ModbusMasterNode::available()
{
    return readSize - readIndex;
}

uint16_t ModbusMasterNode::receive()
{
    if (readIndex < readSize)
        return readData[readIndex++];
    return 0xFFFF;
}


uint8_t ModbusMasterNode::readCoils(uint16_t read_address, uint16_t read_size)
{
    if (master == NULL)
        return MODBUS_INVALID_BUS;
    readSize = read_size;
    readIndex = 0;
    return master->readCoils(slaveID, read_address, read_size, readData);
}

uint8_t ModbusMasterNode::readDiscreteInputs(uint16_t read_address, uint16_t read_size)
{
    if (master == NULL)
        return MODBUS_INVALID_BUS;
    readSize = read_size;
    readIndex = 0;
    return master->readDiscreteInputs(slaveID, read_address, read_size, readData);
}

uint8_t ModbusMasterNode::readHoldingRegisters(uint16_t read_address, uint16_t read_size)
{
    if (master == NULL)
        return MODBUS_INVALID_BUS;
    readSize = read_size;
    readIndex = 0;
    return master->readHoldingRegisters(slaveID, read_address, read_size, readData);
}

uint8_t ModbusMasterNode::readInputRegisters(uint16_t read_address, uint16_t read_size)
{
    if (master == NULL)
        return MODBUS_INVALID_BUS;
    readSize = read_size;
    readIndex = 0;
    return master->readInputRegisters(slaveID, read_address, read_size, readData);
}

uint8_t ModbusMasterNode::writeSingleCoil(uint16_t write_address, uint16_t write_value)
{
    if (master == NULL)
        return MODBUS_INVALID_BUS;
    return master->writeSingleCoil(slaveID, write_address, write_value);
}

uint8_t ModbusMasterNode::writeSingleRegister(uint16_t write_address, uint16_t write_value)
{
    if (master == NULL)
        return MODBUS_INVALID_BUS;
    return master->writeSingleRegister(slaveID, write_address, write_value);
}

uint8_t ModbusMasterNode::writeMultipleCoils(uint16_t write_address, uint16_t write_size)
{
    if (master == NULL)
        return MODBUS_INVALID_BUS;
    return master->writeMultipleCoils(slaveID, write_address, write_size, writeData);
}

uint8_t ModbusMasterNode::writeMultipleCoils()
{
    return writeMultipleCoils(writeAddress, writeCoilSize);
}

uint8_t ModbusMasterNode::writeMultipleRegisters(uint16_t write_address, uint16_t write_size)
{
    if (master == NULL)
        return MODBUS_INVALID_BUS;
    return master->writeMultipleRegisters(slaveID, write_address, write_size, writeData);
}

uint8_t ModbusMasterNode::writeMultipleRegisters()
{
    return writeMultipleRegisters(writeAddress, writeSize);
}

uint8_t ModbusMasterNode::maskWriteRegister(uint16_t write_address, uint16_t and_mask, uint16_t or_mask)
{
    if (master == NULL)
        return MODBUS_INVALID_BUS;
    return master->maskWriteRegister(slaveID, write_address, and_mask, or_mask);
}

uint8_t ModbusMasterNode::readWriteMultipleRegisters(uint16_t read_address, uint16_t read_size,
                                                     uint16_t write_address, uint16_t write_size)
{
    if (master == NULL)
        return MODBUS_INVALID_BUS;
    readSize = read_size;
    readIndex = 0;
    return master->readWriteMultipleRegisters(slaveID, write_address, write_size, writeData,
                                                       read_address, read_size, readData);
}

uint8_t ModbusMasterNode::readWriteMultipleRegisters(uint16_t read_address, uint16_t read_size)
{
    return readWriteMultipleRegisters(read_address, read_size, writeAddress, writeSize);
}
