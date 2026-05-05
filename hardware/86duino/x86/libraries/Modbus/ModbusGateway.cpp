/*
  ModbusGateway.cpp - 86Duino Modbus Gateway library.
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

#include "Modbus.h"
#include "ModbusSerialHAL.h"
#include "ModbusEtherHAL.h"
#include "modbus-core.h"
#include "io.h"
#include <stdlib.h>
#include <string.h>

#define MAX_CLIENT_NUM  (128)
#define TIME_SEC        (0)
#define TIME_USEC       (500000)

#define MODBUS_MAX_GATEWAY_NODE     (256)
#define MODBUS_MAX_MESSAGE_LENGTH   (513)

struct modbus_gateway_t
{
    modbus_hal_t hal;
    modbus_t *ctx;
    int mode;

    uint8_t req[MODBUS_MAX_MESSAGE_LENGTH];
    uint8_t rsp[MODBUS_MAX_MESSAGE_LENGTH];
    int req_length;
    int rsp_length;

    uint8_t node_req[MODBUS_MAX_MESSAGE_LENGTH];
    uint8_t node_rsp[MODBUS_MAX_MESSAGE_LENGTH];
    int node_req_length;
    int node_rsp_length;

    ModbusMasterNode *nodes[MODBUS_MAX_GATEWAY_NODE];
    ModbusMasterNode *node;
    int nodeNum;
    int pollState;
    int nodePollState;

    unsigned long timeout;
    unsigned long recvTime;
};

ModbusGateway::ModbusGateway()
{
    this->_privateData = NULL;
}

ModbusGateway::~ModbusGateway()
{
    end();
}

bool ModbusGateway::begin(int mode, SerialInterface &serial)
{
    modbus_gateway_t *gateway;

    if (mode != MODBUS_RTU && mode != MODBUS_ASCII)
        return false;

    if ((gateway = (modbus_gateway_t *)malloc(sizeof(*gateway))) == NULL)
        return false;
    memset(gateway, 0, sizeof(*gateway));
    gateway->mode = mode;
    gateway->timeout = 1000 * TIME_SEC + TIME_USEC / 1000;
    gateway->hal.available = modbus_serial_hal_available;
    gateway->hal.flush = modbus_serial_hal_flush;
    gateway->hal.write = modbus_serial_hal_write;
    gateway->hal.read = modbus_serial_hal_read;
    gateway->hal.user_data = &serial;

    switch (gateway->mode) {
    case MODBUS_RTU:
        gateway->ctx = modbus_new_rtu_ext(&gateway->hal);
        break;
    case MODBUS_ASCII:
        gateway->ctx = modbus_new_ascii_ext(&gateway->hal);
        break;
    }
    if (gateway->ctx == NULL) {
        free(gateway);
        return false;
    }
    modbus_skip_slave(gateway->ctx, TRUE);
    modbus_set_response_timeout(gateway->ctx, TIME_SEC, TIME_USEC);
    serial.setTimeout(0, 0);
    this->_privateData = gateway;

    return true;
}

bool ModbusGateway::begin(int mode, uint16_t port)
{
    return begin(mode, port, Ethernet);
}

bool ModbusGateway::begin(int mode, uint16_t port, EthernetClass &ethernet)
{
    modbus_gateway_t *gateway;
    modbus_ether_hal_t *ether;

    if (mode != MODBUS_TCP)
        return false;

    if ((ether = new modbus_ether_hal_t(port, ethernet)) == NULL)
        return false;
    ether->pClient = NULL;

    if ((gateway = (modbus_gateway_t *)malloc(sizeof(*gateway))) == NULL) {
        delete ether;
        return false;
    }
    memset(gateway, 0, sizeof(*gateway));
    gateway->mode = mode;
    gateway->timeout = 1000 * TIME_SEC + TIME_USEC / 1000;
    gateway->hal.user_data = ether;
    gateway->hal.connect = modbus_ether_hal_connect;
    gateway->hal.disconnect = modbus_ether_hal_disconnect;
    gateway->hal.available = modbus_ether_hal_available;
    gateway->hal.flush = modbus_ether_hal_flush;
    gateway->hal.write = modbus_ether_hal_write;
    gateway->hal.read = modbus_ether_hal_read;
    gateway->hal.server_begin = modbus_ether_hal_server_begin;
    gateway->hal.server_available = modbus_ether_hal_server_available;

    if ((gateway->ctx = modbus_new_tcp_ext(&gateway->hal, NULL, port)) == NULL) {
        free(gateway);
        delete ether;
        return false;
    }
    modbus_skip_slave(gateway->ctx, TRUE);
    modbus_tcp_listen2(gateway->ctx, MAX_CLIENT_NUM);
    modbus_set_response_timeout(gateway->ctx, TIME_SEC, TIME_USEC);
    this->_privateData = gateway;

    return true;
}

void ModbusGateway::end()
{
    modbus_gateway_t *gateway = (modbus_gateway_t *)this->_privateData;

    if (gateway == NULL)
        return;

    modbus_close(gateway->ctx);
    modbus_free(gateway->ctx);
    switch (gateway->mode) {
    case MODBUS_RTU:
    case MODBUS_ASCII:
        break;
    case MODBUS_TCP:
        delete (modbus_ether_hal_t *)gateway->hal.user_data;
        break;
    }
    free(gateway);
    this->_privateData = NULL;
}

bool ModbusGateway::connect(ModbusMasterNode &mNode)
{
    modbus_gateway_t *gateway = (modbus_gateway_t *)this->_privateData;

    if (gateway == NULL)
        return false;

    for (int i = 0; i < gateway->nodeNum; i++) {
        if (mNode.slaveID == gateway->nodes[i]->slaveID)
            return false;
    }
    gateway->nodes[gateway->nodeNum++] = &mNode;
    mNode.master->setTimeout(1000 * TIME_SEC + TIME_USEC / 1000);
    
    return true;
}

void ModbusGateway::setTimeout(uint32_t milliseconds)
{
    modbus_gateway_t *gateway = (modbus_gateway_t *)this->_privateData;
    if (gateway == NULL)
        return;
    gateway->timeout = milliseconds;
}

int ModbusGateway::nodePoll()
{
    modbus_gateway_t *gateway = (modbus_gateway_t *)this->_privateData;
    modbus_t *ctx = (modbus_t *)gateway->node->master->getModbusHandle();
    unsigned long nowtime;
    
    if (gateway->node->master->gateway != NULL && gateway->node->master->gateway != this)
        return 0;
    
    if (gateway->nodePollState == 0) {
        modbus_set_slave(ctx, gateway->node->slaveID);
        modbus_send_raw_request(ctx, gateway->node_req, gateway->node_req_length);
        gateway->nodePollState = 1;
        gateway->node->master->gateway = this;
        gateway->recvTime = timer_NowTime();
    }
    
    if (gateway->nodePollState == 1) {
        nowtime = timer_NowTime();
        gateway->node_rsp_length = _modbus_receive_msg_nowait(ctx, gateway->node_rsp, MSG_CONFIRMATION);
        if (gateway->node_rsp_length < 0 || nowtime - gateway->recvTime > gateway->node->master->getTimeout()) {
            gateway->nodePollState = 0;
            gateway->node->master->gateway = NULL;
            modbus_flush(ctx);
            return -1;
        }
        if (gateway->node_rsp_length > 0) {
            gateway->nodePollState = 0;
            gateway->node->master->gateway = NULL;
            gateway->rsp_length = modbus_shrink_data_unit(ctx, gateway->node_rsp, gateway->node_rsp_length, gateway->rsp);
            return gateway->rsp_length;
        }
    }
    
    return 0;
}

int ModbusGateway::poll()
{
    modbus_gateway_t *gateway = (modbus_gateway_t *)this->_privateData;
    unsigned long nowtime;

    if (gateway == NULL)
        return 0;

    if (gateway->pollState < 2 && gateway->mode == MODBUS_TCP) {
        int s = modbus_tcp_available2(gateway->ctx);
        if (s == -1)
            return 0;
        modbus_set_socket(gateway->ctx, s);
    }

    nowtime = timer_NowTime();

    if (gateway->pollState == 0 && modbus_receive_available(gateway->ctx) > 0) {
        gateway->pollState = 1;
        gateway->recvTime = nowtime;
    }

    if (gateway->pollState == 1) {
        gateway->req_length = modbus_receive_nowait(gateway->ctx, gateway->req);
        if (gateway->req_length > 0) {
            int slave = modbus_query_slave(gateway->ctx, gateway->req, gateway->req_length);
            gateway->pollState = 0;
            for (int i = 0; i < gateway->nodeNum; i++) {
                if (slave == gateway->nodes[i]->slaveID) {
                    gateway->node = gateway->nodes[i];
                    gateway->node_req_length = modbus_shrink_data_unit(gateway->ctx, gateway->req, gateway->req_length, gateway->node_req);
                    gateway->pollState = 2;
                    break;
                }
            }
            if (gateway->pollState == 0)
                modbus_ignore_confirmation(gateway->ctx);
        } else if (gateway->req_length < 0 || nowtime - gateway->recvTime > gateway->timeout) {
            modbus_flush(gateway->ctx);
            modbus_receive_reset(gateway->ctx);
            gateway->pollState = 0;
        }
    }

    if (gateway->pollState == 2) {
        int rc = nodePoll();
        if (rc < 0) {
            gateway->pollState = 0;
            modbus_reply_exception(gateway->ctx, gateway->req, MODBUS_EXCEPTION_GATEWAY_TARGET);
        } else if (rc > 0) {
            gateway->pollState = 0;
            return modbus_send_raw_response(gateway->ctx, gateway->rsp, gateway->rsp_length, gateway->req, gateway->req_length);
        }
    }

    return 0;
}