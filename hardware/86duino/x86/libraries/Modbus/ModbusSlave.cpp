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

#include "HardwareSerial.h"
#include "modbus-core.h"
#include "io.h"

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

ModbusSlave::ModbusSlave()
{
    _mode = MODBUS_RTU;
    mbus = NULL;
    currentNode = NULL;
    timeout = 1000 * TIME_SEC + TIME_USEC / 1000;
    handle_num = 0;
}

bool ModbusSlave::initialize()
{
    modbus_t *ctx = (modbus_t *)mbus;
    
    modbus_set_callback(ctx, MODBUS_FC_READ_COILS              , modbus_cb_read_coils              , NULL);
    modbus_set_callback(ctx, MODBUS_FC_READ_DISCRETE_INPUTS    , modbus_cb_read_discrete_inputs    , NULL);
    modbus_set_callback(ctx, MODBUS_FC_READ_HOLDING_REGISTERS  , modbus_cb_read_holding_registers  , NULL);
    modbus_set_callback(ctx, MODBUS_FC_READ_INPUT_REGISTERS    , modbus_cb_read_input_registers    , NULL);
    modbus_set_callback(ctx, MODBUS_FC_WRITE_SINGLE_COIL       , modbus_cb_write_single_coil       , NULL);
    modbus_set_callback(ctx, MODBUS_FC_WRITE_SINGLE_REGISTER   , modbus_cb_write_single_register   , NULL);
    modbus_set_callback(ctx, MODBUS_FC_WRITE_MULTIPLE_COILS    , modbus_cb_write_multiple_coils    , NULL);
    modbus_set_callback(ctx, MODBUS_FC_WRITE_MULTIPLE_REGISTERS, modbus_cb_write_multiple_registers, NULL);
    modbus_set_callback(ctx, MODBUS_FC_MASK_WRITE_REGISTER     , modbus_cb_mask_write_register     , NULL);
    
    return true;
}

int ModbusSlave::poll()
{
    modbus_t *ctx = (modbus_t *)mbus;
    ModbusSlaveNode *tempNode;
    unsigned long nowTime;
    int s, slave = -1;
    
    if (pollState != 2 && _mode == MODBUS_TCP) {
        s = modbus_tcp_select(ctx);
        if (s == -1)
            return 0;;
        modbus_set_socket(ctx, s);
    }

    nowTime = timer_NowTime();

    if (pollState == 0 && modbus_receive_available(ctx) > 0) {
        pollState = 1;
        recvTime = nowTime;
    }
    
    if (pollState == 1) {

        req_length = modbus_receive_nowait(ctx, req);
        if (req_length > 0) {
            slave = modbus_query_slave(ctx, req, req_length);
            pollState = 2;
            handle_num = 0;
            tempNode = headNode;
            while (tempNode != NULL) {
                if (slave == MODBUS_BROADCAST_ADDRESS) {
                    tempNode->handled++;
                    handle_num++;
                } else if (slave == tempNode->slaveID) {
                    tempNode->handled++;
                    handle_num++;
                    break;
                }
                tempNode = tempNode->nextNode;
            }
            if (handle_num == 0)
                modbus_ignore_confirmation(ctx);
        } else if (nowTime - recvTime > timeout) {
            modbus_flush(ctx);
            modbus_receive_reset(ctx);
            pollState = 0;
        }
    }
    
    return slave;
}

int ModbusSlave::poll(ModbusSlaveNode *node)
{
    modbus_t *ctx = (modbus_t *)mbus;
    ModbusSlaveNode *tempNode;

    poll();

    if (pollState == 2) {
        if (node->handled == 1) {
            currentNode = node;
            modbus_reply(ctx, req, req_length, (modbus_mapping_t *)node->table);
            node->handled++;
            return req_length;
        } else if (node->handled == 2) {
            handle_num = 0;
        }
        
        if (handle_num == 0) {
            pollState = 0;
            tempNode = headNode;
            while (tempNode != NULL) {
                tempNode->handled = 0;
                tempNode = tempNode->nextNode;
            }
        }
    }
    
    return 0;
}

bool ModbusSlave::begin(int mode, HardwareSerial &serial)
{
    modbus_t *ctx = NULL;
    int rc;
    
    if (serial.handle == NULL)
        return false;
    
    if (mode == MODBUS_RTU)
        ctx = modbus_new_rtu("USB_COM", 115200, 'N', 8, 1);
    else if (mode == MODBUS_ASCII)
        ctx = modbus_new_ascii("USB_COM", 115200, 'N', 8, 1);
    
    if (ctx == NULL)
        return false;
    
    modbus_skip_slave(ctx, TRUE);
    rc = modbus_connect_assign(ctx, serial.handle);
    if (rc == -1) {
        modbus_free(ctx);
        return false;
    }
    modbus_set_response_timeout(ctx, TIME_SEC, TIME_USEC);
    
    _mode = mode;
    mbus = (void *)ctx;
    
    if (initialize() == false) {
        modbus_free(ctx);
        return false;
    }

    return true;
}

bool ModbusSlave::begin(int mode)
{
    modbus_t *ctx = NULL;
    int rc;
    
    if (mode == MODBUS_TCP)
        ctx = modbus_new_tcp(NULL, 502);
    
    if (ctx == NULL)
        return false;

    modbus_skip_slave(ctx, TRUE);
    modbus_tcp_listen(ctx, MAX_CLIENT_NUM);
    modbus_set_response_timeout(ctx, TIME_SEC, TIME_USEC);
    
    _mode = mode;
    mbus = (void *)ctx;
    
    if (initialize() == false) {
        modbus_free(ctx);
        return false;
    }

    return true;
}

void ModbusSlave::setTimeout(uint32_t milliseconds)
{
    timeout = milliseconds;
}

void ModbusSlave::addNode(ModbusSlaveNode *node)
{
    ModbusSlaveNode *tempNode;
    
    if (headNode == NULL)
        headNode = node;
    else {
        tempNode = headNode;
        do {
            if (node == tempNode)
                return;
            if (tempNode->nextNode == NULL)
                break;
            tempNode = tempNode->nextNode;
        } while (tempNode != NULL);
        tempNode->nextNode = node;
    }
}

ModbusSlaveNode::ModbusSlaveNode()
{
    slave = NULL;
    slaveID = MODBUS_BROADCAST_ADDRESS;
    nextNode = NULL;
    handled = 0;
}

bool ModbusSlaveNode::begin(ModbusSlave &bus)
{
    ModbusSlave *s = &bus;
    
    if (s == NULL)
        return false;
    
    if (s->_mode == MODBUS_TCP)
        return begin(MODBUS_TCP_SLAVE, bus);
    
    return begin(MODBUS_BROADCAST_ADDRESS, bus);
}

bool ModbusSlaveNode::begin(uint8_t slave_id, ModbusSlave &bus)
{
    slave = &bus;
    
    if (slave == NULL)
        return false;
    
    table = (void *)modbus_mapping_create();
    if (table == NULL)
        return false;
    
    slave->addNode(this);
    
    slaveID = slave_id;
    
    return true;
}

int ModbusSlaveNode::poll()
{
    modbus_mapping_t *tab = (modbus_mapping_t *)table;

    if (tab == NULL)
        return 0;

    if (slave == NULL)
        return 0;

    modbus_mapping_bits_assign(tab, coils, 0, MODBUS_MAX_TABLE_SIZE);
    modbus_mapping_input_bits_assign(tab, discrete_inputs, 0, MODBUS_MAX_TABLE_SIZE);
    modbus_mapping_registers_assign(tab, holding_registers, 0, MODBUS_MAX_TABLE_SIZE);
    modbus_mapping_input_registers_assign(tab, input_registers, 0, MODBUS_MAX_TABLE_SIZE);

    return slave->poll(this);
}

int ModbusSlaveNode::poll(uint16_t *buf, uint16_t size)
{
    modbus_mapping_t *tab = (modbus_mapping_t *)table;

    if (tab == NULL)
        return 0;

    if (slave == NULL)
        return 0;

    modbus_mapping_bits_assign(tab, buf, 0, size);
    modbus_mapping_input_bits_assign(tab, buf, 0, size);
    modbus_mapping_registers_assign(tab, buf, 0, size);
    modbus_mapping_input_registers_assign(tab, buf, 0, size);

    return slave->poll(this);
}

int ModbusSlaveNode::readCoil(uint16_t address, uint16_t size, uint16_t *buffer)
{
    modbus_mapping_t *tab = (modbus_mapping_t *)table;
    int mapping_address = address - tab->start_bits;
    int i;

    if (size < 1 || MODBUS_MAX_READ_BITS < size)
        return MODBUS_ILLEGAL_DATA_VALUE;
    
    if (mapping_address < 0 || (mapping_address + size) > tab->nb_bits)
        return MODBUS_ILLEGAL_DATA_ADDRESS;
    
    for (i = 0; i < size; i++)
        buffer[i] = tab->tab_bits[mapping_address + i];
    
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
    modbus_mapping_t *tab = (modbus_mapping_t *)table;
    int mapping_address = address - tab->start_bits;
    int i;

    if (size < 1 || MODBUS_MAX_WRITE_BITS < size)
        return MODBUS_ILLEGAL_DATA_VALUE;
    
    if (mapping_address < 0 || (mapping_address + size) > tab->nb_bits)
        return MODBUS_ILLEGAL_DATA_ADDRESS;
    
    for (i = 0; i < size; i++)
        tab->tab_bits[mapping_address + i] = buffer[i] ? 1 : 0;
    
    return MODBUS_SUCCESS;
}

int ModbusSlaveNode::writeCoil(uint16_t address, uint16_t value)
{
    return writeCoil(address, 1, &value);
}

int ModbusSlaveNode::writeDiscreteInput(uint16_t address, uint16_t size, uint16_t *buffer)
{
    modbus_mapping_t *tab = (modbus_mapping_t *)table;
    int mapping_address = address - tab->start_input_bits;
    int i;

    if (size < 1 || MODBUS_MAX_READ_BITS < size)
        return MODBUS_ILLEGAL_DATA_VALUE;
    
    if (mapping_address < 0 || (mapping_address + size) > tab->nb_input_bits)
        return MODBUS_ILLEGAL_DATA_ADDRESS;
    
    for (i = 0; i < size; i++)
        tab->tab_input_bits[mapping_address + i] = buffer[i];
    
    return MODBUS_SUCCESS;
}

int ModbusSlaveNode::writeDiscreteInput(uint16_t address, uint16_t value)
{
    return writeDiscreteInput(address, 1, &value);
}

int ModbusSlaveNode::readHoldingRegister(uint16_t address, uint16_t size, uint16_t *buffer)
{
    modbus_mapping_t *tab = (modbus_mapping_t *)table;
    int mapping_address = address - tab->start_registers;
    int i;

    if (size < 1 || MODBUS_MAX_READ_REGISTERS < size)
        return MODBUS_ILLEGAL_DATA_VALUE;
    
    if (mapping_address < 0 || (mapping_address + size) > tab->nb_registers)
        return MODBUS_ILLEGAL_DATA_ADDRESS;
    
    for (i = 0; i < size; i++)
        buffer[i] = tab->tab_registers[mapping_address + i];
    
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
    modbus_mapping_t *tab = (modbus_mapping_t *)table;
    int mapping_address = address - tab->start_registers;
    int i;

    if (size < 1 || MODBUS_MAX_WRITE_REGISTERS < size)
        return MODBUS_ILLEGAL_DATA_VALUE;
    
    if (mapping_address < 0 || (mapping_address + size) > tab->nb_registers)
        return MODBUS_ILLEGAL_DATA_ADDRESS;
    
    for (i = 0; i < size; i++)
         tab->tab_registers[mapping_address + i] = buffer[i];
    
    return MODBUS_SUCCESS;
}

int ModbusSlaveNode::writeHoldingRegister(uint16_t address, uint16_t value)
{
    return writeHoldingRegister(address, 1, &value);
}

int ModbusSlaveNode::writeInputRegister(uint16_t address, uint16_t size, uint16_t *buffer)
{
    modbus_mapping_t *tab = (modbus_mapping_t *)table;
    int mapping_address = address - tab->start_input_registers;
    int i;

    if (size < 1 || MODBUS_MAX_READ_REGISTERS < size)
        return MODBUS_ILLEGAL_DATA_VALUE;
    
    if (mapping_address < 0 || (mapping_address + size) > tab->nb_input_registers)
        return MODBUS_ILLEGAL_DATA_ADDRESS;
    
    for (i = 0; i < size; i++)
        tab->tab_input_registers[mapping_address + i] = buffer[i];
    
    return MODBUS_SUCCESS;
}

int ModbusSlaveNode::writeInputRegister(uint16_t address, uint16_t value)
{
    return writeInputRegister(address, 1, &value);
}
