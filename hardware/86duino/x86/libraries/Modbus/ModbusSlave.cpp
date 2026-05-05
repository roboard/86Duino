/*
  ModbusSlave.cpp - 86Duino Modbus slave library for communicating with
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

  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)
*/

#include "ModbusSlave.h"
#include "ModbusSerialHAL.h"
#include "ModbusEtherHAL.h"
#include "modbus-core.h"
#include "io.h"
#include <stdlib.h>
#include <string.h>

#define MAX_CLIENT_NUM  (128)
#define TIME_SEC        (1)
#define TIME_USEC       (0)

static ModbusSlaveNode *currentNode = NULL;

static int modbus_cb_read_coils(int function, int address, int num, void *data)
{
    if (currentNode->cbFunc[MODBUS_CB_READ_COILS] == NULL)
        return MODBUS_SUCCESS;
    return currentNode->cbFunc[MODBUS_CB_READ_COILS](function, address, num);
}

static int modbus_cb_read_discrete_inputs(int function, int address, int num, void *data)
{
    if (currentNode->cbFunc[MODBUS_CB_READ_DISCRETE_INPUTS] == NULL)
        return MODBUS_SUCCESS;
    return currentNode->cbFunc[MODBUS_CB_READ_DISCRETE_INPUTS](function, address, num);
}

static int modbus_cb_read_holding_registers(int function, int address, int num, void *data)
{
    if (currentNode->cbFunc[MODBUS_CB_READ_HOLDING_REGISTERS] == NULL)
        return MODBUS_SUCCESS;
    return currentNode->cbFunc[MODBUS_CB_READ_HOLDING_REGISTERS](function, address, num);
}

static int modbus_cb_read_input_registers(int function, int address, int num, void *data)
{
    if (currentNode->cbFunc[MODBUS_CB_READ_INPUT_REGISTERS] == NULL)
        return MODBUS_SUCCESS;
    return currentNode->cbFunc[MODBUS_CB_READ_INPUT_REGISTERS](function, address, num);
}

static int modbus_cb_write_single_coil(int function, int address, int num, void *data)
{
    if (currentNode->cbFunc[MODBUS_CB_WRITE_SINGLE_COIL] == NULL)
        return MODBUS_SUCCESS;
    return currentNode->cbFunc[MODBUS_CB_WRITE_SINGLE_COIL](function, address, num);
}

static int modbus_cb_write_single_register(int function, int address, int num, void *data)
{
    if (currentNode->cbFunc[MODBUS_CB_WRITE_SINGLE_REGISTER] == NULL)
        return MODBUS_SUCCESS;
    return currentNode->cbFunc[MODBUS_CB_WRITE_SINGLE_REGISTER](function, address, num);
}

static int modbus_cb_write_multiple_coils(int function, int address, int num, void *data)
{
    if (currentNode->cbFunc[MODBUS_CB_WRITE_MULTIPLE_COILS] == NULL)
        return MODBUS_SUCCESS;
    return currentNode->cbFunc[MODBUS_CB_WRITE_MULTIPLE_COILS](function, address, num);
}

static int modbus_cb_write_multiple_registers(int function, int address, int num, void *data)
{
    if (currentNode->cbFunc[MODBUS_CB_WRITE_MULTIPLE_REGISTERS] == NULL)
        return MODBUS_SUCCESS;
    return currentNode->cbFunc[MODBUS_CB_WRITE_MULTIPLE_REGISTERS](function, address, num);
}

static int modbus_cb_mask_write_register(int function, int address, int num, void *data)
{
    if (currentNode->cbFunc[MODBUS_CB_MASK_WRITE_REGISTER] == NULL)
        return MODBUS_SUCCESS;
    return currentNode->cbFunc[MODBUS_CB_MASK_WRITE_REGISTER](function, address, num);
}

struct modbus_slave_node_t
{
    modbus_slave_node_t *next;
    ModbusSlaveNode *node;

    int handled;
    ModbusSlave *bus;
    uint8_t slaveId;

    modbus_mapping_t *mapping;

    uint16_t coils[64 * 1024];
    uint16_t discrete_inputs[64 * 1024];
    uint16_t holding_registers[64 * 1024];
    uint16_t input_registers[64 * 1024];
};

struct modbus_slave_t
{
    modbus_slave_node_t *head;
    modbus_hal_t hal;
    modbus_t *ctx;
    int mode;

    int pollState;
    unsigned long timeout;
    unsigned long recvTime;

    uint8_t req[MAX_MESSAGE_LENGTH];
    int req_length;
    int handle_num;
};

ModbusSlave::ModbusSlave()
{
    this->_privateData = NULL;
}

ModbusSlave::~ModbusSlave()
{
    end();
}

bool ModbusSlave::begin(int mode, SerialInterface &serial)
{
    modbus_slave_t *slave;

    if (mode != MODBUS_RTU && mode != MODBUS_ASCII)
        return false;

    if ((slave = (modbus_slave_t *)malloc(sizeof(*slave))) == NULL)
        return false;
    memset(slave, 0, sizeof(*slave));
    slave->mode = mode;
    slave->timeout = 1000 * TIME_SEC + TIME_USEC / 1000;
    slave->hal.available = modbus_serial_hal_available;
    slave->hal.flush = modbus_serial_hal_flush;
    slave->hal.write = modbus_serial_hal_write;
    slave->hal.read = modbus_serial_hal_read;
    slave->hal.user_data = &serial;

    switch (slave->mode) {
    case MODBUS_RTU:
        slave->ctx = modbus_new_rtu_ext(&slave->hal);
        break;
    case MODBUS_ASCII:
        slave->ctx = modbus_new_ascii_ext(&slave->hal);
        break;
    }
    if (slave->ctx == NULL) {
        free(slave);
        return false;
    }
    modbus_skip_slave(slave->ctx, TRUE);
    modbus_set_response_timeout(slave->ctx, TIME_SEC, TIME_USEC);
    modbus_set_callback(slave->ctx, MODBUS_FC_READ_COILS              , modbus_cb_read_coils              , NULL);
    modbus_set_callback(slave->ctx, MODBUS_FC_READ_DISCRETE_INPUTS    , modbus_cb_read_discrete_inputs    , NULL);
    modbus_set_callback(slave->ctx, MODBUS_FC_READ_HOLDING_REGISTERS  , modbus_cb_read_holding_registers  , NULL);
    modbus_set_callback(slave->ctx, MODBUS_FC_READ_INPUT_REGISTERS    , modbus_cb_read_input_registers    , NULL);
    modbus_set_callback(slave->ctx, MODBUS_FC_WRITE_SINGLE_COIL       , modbus_cb_write_single_coil       , NULL);
    modbus_set_callback(slave->ctx, MODBUS_FC_WRITE_SINGLE_REGISTER   , modbus_cb_write_single_register   , NULL);
    modbus_set_callback(slave->ctx, MODBUS_FC_WRITE_MULTIPLE_COILS    , modbus_cb_write_multiple_coils    , NULL);
    modbus_set_callback(slave->ctx, MODBUS_FC_WRITE_MULTIPLE_REGISTERS, modbus_cb_write_multiple_registers, NULL);
    modbus_set_callback(slave->ctx, MODBUS_FC_MASK_WRITE_REGISTER     , modbus_cb_mask_write_register     , NULL);
    serial.setTimeout(0, 0);
    this->_privateData = slave;

    return true;
}

bool ModbusSlave::begin(int mode, uint16_t port)
{
    return begin(mode, port, Ethernet);
}

bool ModbusSlave::begin(int mode, uint16_t port, EthernetClass &ethernet)
{
    modbus_slave_t *slave;
    modbus_ether_hal_t *ether;

    if (mode != MODBUS_TCP)
        return false;

    if ((ether = new modbus_ether_hal_t(port, ethernet)) == NULL)
        return false;
    ether->pClient = NULL;

    if ((slave = (modbus_slave_t *)malloc(sizeof(*slave))) == NULL) {
        delete ether;
        return false;
    }
    memset(slave, 0, sizeof(*slave));
    slave->mode = mode;
    slave->timeout = 1000 * TIME_SEC + TIME_USEC / 1000;
    slave->hal.user_data = ether;
    slave->hal.connect = modbus_ether_hal_connect;
    slave->hal.disconnect = modbus_ether_hal_disconnect;
    slave->hal.available = modbus_ether_hal_available;
    slave->hal.flush = modbus_ether_hal_flush;
    slave->hal.write = modbus_ether_hal_write;
    slave->hal.read = modbus_ether_hal_read;
    slave->hal.server_begin = modbus_ether_hal_server_begin;
    slave->hal.server_available = modbus_ether_hal_server_available;

    if ((slave->ctx = modbus_new_tcp_ext(&slave->hal, NULL, port)) == NULL) {
        free(slave);
        delete ether;
        return false;
    }
    modbus_skip_slave(slave->ctx, TRUE);
    modbus_tcp_listen2(slave->ctx, MAX_CLIENT_NUM);
    modbus_set_response_timeout(slave->ctx, TIME_SEC, TIME_USEC);
    modbus_set_callback(slave->ctx, MODBUS_FC_READ_COILS              , modbus_cb_read_coils              , NULL);
    modbus_set_callback(slave->ctx, MODBUS_FC_READ_DISCRETE_INPUTS    , modbus_cb_read_discrete_inputs    , NULL);
    modbus_set_callback(slave->ctx, MODBUS_FC_READ_HOLDING_REGISTERS  , modbus_cb_read_holding_registers  , NULL);
    modbus_set_callback(slave->ctx, MODBUS_FC_READ_INPUT_REGISTERS    , modbus_cb_read_input_registers    , NULL);
    modbus_set_callback(slave->ctx, MODBUS_FC_WRITE_SINGLE_COIL       , modbus_cb_write_single_coil       , NULL);
    modbus_set_callback(slave->ctx, MODBUS_FC_WRITE_SINGLE_REGISTER   , modbus_cb_write_single_register   , NULL);
    modbus_set_callback(slave->ctx, MODBUS_FC_WRITE_MULTIPLE_COILS    , modbus_cb_write_multiple_coils    , NULL);
    modbus_set_callback(slave->ctx, MODBUS_FC_WRITE_MULTIPLE_REGISTERS, modbus_cb_write_multiple_registers, NULL);
    modbus_set_callback(slave->ctx, MODBUS_FC_MASK_WRITE_REGISTER     , modbus_cb_mask_write_register     , NULL);
    this->_privateData = slave;

    return true;
}

void ModbusSlave::end()
{
    modbus_slave_t *slave = (modbus_slave_t *)this->_privateData;

    if (slave == NULL)
        return;

    modbus_close(slave->ctx);
    modbus_free(slave->ctx);
    switch (slave->mode) {
    case MODBUS_RTU:
    case MODBUS_ASCII:
        break;
    case MODBUS_TCP:
        delete (modbus_ether_hal_t *)slave->hal.user_data;
        break;
    }
    free(slave);
    this->_privateData = NULL;
}

int ModbusSlave::getModbusMode()
{
    modbus_slave_t *slave = (modbus_slave_t *)this->_privateData;
    if (slave == NULL)
        return MODBUS_RTU;
    return slave->mode;
}

void ModbusSlave::setTimeout(uint32_t milliseconds)
{
    modbus_slave_t *slave = (modbus_slave_t *)this->_privateData;
    if (slave == NULL)
        return;
    slave->timeout = milliseconds;
}

int ModbusSlave::poll()
{
    modbus_slave_t *slave = (modbus_slave_t *)this->_privateData;
    modbus_slave_node_t *temp;
    unsigned long nowtime;
    int slaveid = -1;

    if (slave->pollState != 2 && slave->mode == MODBUS_TCP) {
        int s = modbus_tcp_available2(slave->ctx);
        if (s == -1)
            return -1;
        modbus_set_socket(slave->ctx, s);
    }

    nowtime = timer_NowTime();

    if (slave->pollState == 0 && modbus_receive_available(slave->ctx) > 0) {
        slave->pollState = 1;
        slave->recvTime = nowtime;
    }

    if (slave->pollState == 1) {
        slave->req_length = modbus_receive_nowait(slave->ctx, slave->req);
        if (slave->req_length > 0) {
            slaveid = modbus_query_slave(slave->ctx, slave->req, slave->req_length);
            slave->pollState = 2;
            slave->handle_num = 0;
            temp = slave->head;
            while (temp != NULL) {
                if (slaveid == MODBUS_BROADCAST_ADDRESS) {
                    temp->handled++;
                    slave->handle_num++;
                } else if (slaveid == temp->slaveId) {
                    temp->handled++;
                    slave->handle_num++;
                    break;
                }
                temp = temp->next;
            }
            if (slave->handle_num == 0)
                modbus_ignore_confirmation(slave->ctx);
        } else if (nowtime - slave->recvTime > slave->timeout) {
            modbus_flush(slave->ctx);
            modbus_receive_reset(slave->ctx);
            slave->pollState = 0;
        }
    }

    return slaveid;
}

int ModbusSlave::poll(void *data)
{
    modbus_slave_t *slave = (modbus_slave_t *)this->_privateData;
    modbus_slave_node_t *node = (modbus_slave_node_t *)data;
    modbus_slave_node_t *temp;

    poll();

    if (slave->pollState == 2) {
        if (node->handled == 1) {
            currentNode = node->node;
            modbus_reply(slave->ctx, slave->req, slave->req_length, (modbus_mapping_t *)node->mapping);
            node->handled++;
            return slave->req_length;
        } else if (node->handled == 2) {
            slave->handle_num = 0;
        }

        if (slave->handle_num == 0) {
            slave->pollState = 0;
            temp = slave->head;
            while (temp != NULL) {
                temp->handled = 0;
                temp = temp->next;
            }
        }
    }

    return 0;
}

void ModbusSlave::addNode(void *data)
{
    modbus_slave_t *slave = (modbus_slave_t *)this->_privateData;
    modbus_slave_node_t *node = (modbus_slave_node_t *)data;
    modbus_slave_node_t *temp;

    if (slave == NULL)
        return;

    if (slave->head == NULL)
        slave->head = node;
    else {
        temp = slave->head;
        do {
            if (node == temp)
                return;
            if (temp->next == NULL)
                break;
            temp = temp->next;
        } while (temp != NULL);
        temp->next = node;
    }
}

ModbusSlaveNode::ModbusSlaveNode()
{
    this->_privateData = NULL;
}

ModbusSlaveNode::~ModbusSlaveNode()
{
    detach();
}

bool ModbusSlaveNode::attach(ModbusSlave &bus)
{
    return attach((bus.getModbusMode() == MODBUS_TCP) ? MODBUS_TCP_SLAVE : MODBUS_BROADCAST_ADDRESS, bus);
}

bool ModbusSlaveNode::attach(uint8_t slave_id, ModbusSlave &bus)
{
    modbus_slave_node_t *node;

    if ((node = (modbus_slave_node_t *)malloc(sizeof(*node))) == NULL)
        return false;
    memset(node, 0, sizeof(*node));
    node->bus = &bus;
    node->slaveId = slave_id;
    node->node = this;

    if ((node->mapping = modbus_mapping_create()) == NULL) {
        free(node);
        return false;
    }
    node->bus->addNode(node);
    this->_privateData = node;

    return true;
}

void ModbusSlaveNode::detach()
{
    modbus_slave_node_t *node = (modbus_slave_node_t *)this->_privateData;

    if (node == NULL)
        return;

    modbus_mapping_free(node->mapping);
    free(node);
    this->_privateData = NULL;
}

bool ModbusSlaveNode::begin(ModbusSlave &bus)
{
    return attach(bus);
}

bool ModbusSlaveNode::begin(uint8_t slave_id, ModbusSlave &bus)
{
    return attach(slave_id, bus);
}

int ModbusSlaveNode::poll()
{
    modbus_slave_node_t *node = (modbus_slave_node_t *)this->_privateData;

    if (node == NULL)
        return 0;

    modbus_mapping_bits_assign(node->mapping, node->coils, 0, sizeof(node->coils));
    modbus_mapping_input_bits_assign(node->mapping, node->discrete_inputs, 0, sizeof(node->discrete_inputs));
    modbus_mapping_registers_assign(node->mapping, node->holding_registers, 0, sizeof(node->holding_registers));
    modbus_mapping_input_registers_assign(node->mapping, node->input_registers, 0, sizeof(node->input_registers));

    return node->bus->poll(node);
}

int ModbusSlaveNode::poll(uint16_t *buf, uint16_t size)
{
    modbus_slave_node_t *node = (modbus_slave_node_t *)this->_privateData;

    if (node == NULL)
        return 0;

    modbus_mapping_bits_assign(node->mapping, buf, 0, size);
    modbus_mapping_input_bits_assign(node->mapping, buf, 0, size);
    modbus_mapping_registers_assign(node->mapping, buf, 0, size);
    modbus_mapping_input_registers_assign(node->mapping, buf, 0, size);

    return node->bus->poll(node);
}

int ModbusSlaveNode::readCoil(uint16_t address, uint16_t size, uint16_t *buffer)
{
    modbus_slave_node_t *node = (modbus_slave_node_t *)this->_privateData;
    int mapping_address;

    if (node == NULL)
        return MODBUS_INVALID_BUS;
    if (size < 1 || MODBUS_MAX_READ_BITS < size)
        return MODBUS_ILLEGAL_DATA_VALUE;

    mapping_address = address - node->mapping->start_bits;
    if (mapping_address < 0 || (mapping_address + size) > node->mapping->nb_bits)
        return MODBUS_ILLEGAL_DATA_ADDRESS;

    for (int i = 0; i < size; i++)
        buffer[i] = node->mapping->tab_bits[mapping_address + i];

    return MODBUS_SUCCESS;
}

int ModbusSlaveNode::readCoil(uint16_t address)
{
    uint16_t value;

    if (readCoil(address, 1, &value) != MODBUS_SUCCESS)
        return -1;

    return (value & 0x01) ? MODBUS_COIL_ON : MODBUS_COIL_OFF;
}

int ModbusSlaveNode::writeCoil(uint16_t address, uint16_t size, uint16_t *buffer)
{
    modbus_slave_node_t *node = (modbus_slave_node_t *)this->_privateData;
    int mapping_address;

    if (node == NULL)
        return MODBUS_INVALID_BUS;
    if (size < 1 || MODBUS_MAX_WRITE_BITS < size)
        return MODBUS_ILLEGAL_DATA_VALUE;

    mapping_address = address - node->mapping->start_bits;
    if (mapping_address < 0 || (mapping_address + size) > node->mapping->nb_bits)
        return MODBUS_ILLEGAL_DATA_ADDRESS;

    for (int i = 0; i < size; i++)
        node->mapping->tab_bits[mapping_address + i] = buffer[i] ? 1 : 0;

    return MODBUS_SUCCESS;
}

int ModbusSlaveNode::writeCoil(uint16_t address, uint16_t value)
{
    return writeCoil(address, 1, &value);
}

int ModbusSlaveNode::writeDiscreteInput(uint16_t address, uint16_t size, uint16_t *buffer)
{
    modbus_slave_node_t *node = (modbus_slave_node_t *)this->_privateData;
    int mapping_address;

    if (node == NULL)
        return MODBUS_INVALID_BUS;
    if (size < 1 || MODBUS_MAX_READ_BITS < size)
        return MODBUS_ILLEGAL_DATA_VALUE;

    mapping_address = address - node->mapping->start_input_bits;
    if (mapping_address < 0 || (mapping_address + size) > node->mapping->nb_input_bits)
        return MODBUS_ILLEGAL_DATA_ADDRESS;

    for (int i = 0; i < size; i++)
        node->mapping->tab_input_bits[mapping_address + i] = buffer[i];

    return MODBUS_SUCCESS;
}

int ModbusSlaveNode::writeDiscreteInput(uint16_t address, uint16_t value)
{
    return writeDiscreteInput(address, 1, &value);
}

int ModbusSlaveNode::readHoldingRegister(uint16_t address, uint16_t size, uint16_t *buffer)
{
    modbus_slave_node_t *node = (modbus_slave_node_t *)this->_privateData;
    int mapping_address;

    if (node == NULL)
        return MODBUS_INVALID_BUS;
    if (size < 1 || MODBUS_MAX_READ_REGISTERS < size)
        return MODBUS_ILLEGAL_DATA_VALUE;

    mapping_address = address - node->mapping->start_registers;
    if (mapping_address < 0 || (mapping_address + size) > node->mapping->nb_registers)
        return MODBUS_ILLEGAL_DATA_ADDRESS;

    for (int i = 0; i < size; i++)
        buffer[i] = node->mapping->tab_registers[mapping_address + i];

    return MODBUS_SUCCESS;
}

int ModbusSlaveNode::readHoldingRegister(uint16_t address)
{
    uint16_t value;

    if (readHoldingRegister(address, 1, &value) != MODBUS_SUCCESS)
        return -1;

    return value;
}

int ModbusSlaveNode::writeHoldingRegister(uint16_t address, uint16_t size, uint16_t *buffer)
{
    modbus_slave_node_t *node = (modbus_slave_node_t *)this->_privateData;
    int mapping_address;

    if (node == NULL)
        return MODBUS_INVALID_BUS;
    if (size < 1 || MODBUS_MAX_WRITE_REGISTERS < size)
        return MODBUS_ILLEGAL_DATA_VALUE;

    mapping_address = address - node->mapping->start_registers;
    if (mapping_address < 0 || (mapping_address + size) > node->mapping->nb_registers)
        return MODBUS_ILLEGAL_DATA_ADDRESS;

    for (int i = 0; i < size; i++)
         node->mapping->tab_registers[mapping_address + i] = buffer[i];

    return MODBUS_SUCCESS;
}

int ModbusSlaveNode::writeHoldingRegister(uint16_t address, uint16_t value)
{
    return writeHoldingRegister(address, 1, &value);
}

int ModbusSlaveNode::writeInputRegister(uint16_t address, uint16_t size, uint16_t *buffer)
{
    modbus_slave_node_t *node = (modbus_slave_node_t *)this->_privateData;
    int mapping_address;

    if (node == NULL)
        return MODBUS_INVALID_BUS;
    if (size < 1 || MODBUS_MAX_READ_REGISTERS < size)
        return MODBUS_ILLEGAL_DATA_VALUE;

    mapping_address = address - node->mapping->start_input_registers;
    if (mapping_address < 0 || (mapping_address + size) > node->mapping->nb_input_registers)
        return MODBUS_ILLEGAL_DATA_ADDRESS;

    for (int i = 0; i < size; i++)
        node->mapping->tab_input_registers[mapping_address + i] = buffer[i];

    return MODBUS_SUCCESS;
}

int ModbusSlaveNode::writeInputRegister(uint16_t address, uint16_t value)
{
    return writeInputRegister(address, 1, &value);
}
