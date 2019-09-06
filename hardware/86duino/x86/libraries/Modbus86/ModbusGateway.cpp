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

#include "Modbus86.h"
#include "modbus.h"
#include "io.h"

#define MAX_CLIENT_NUM  (128)
#define TIME_SEC        (0)
#define TIME_USEC       (500000)

ModbusGateway::ModbusGateway()
{
    nodeNum = 0;
    pollState = 0;
    nodePollState = 0;
    timeout = 1000 * TIME_SEC + TIME_USEC / 1000;
}

bool ModbusGateway::begin(int mode, HardwareSerial &serial)
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

    return true;
}

bool ModbusGateway::begin(int mode)
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

    return true;
}

bool ModbusGateway::connect(ModbusMasterNode &mNode)
{
    int i;
    
    for (i = 0; i < nodeNum; i++) {
        if (mNode.slaveID == nodes[i]->slaveID)
            return false;
    }
    nodes[nodeNum++] = &mNode;
    mNode.master->setTimeout(1000 * TIME_SEC + TIME_USEC / 1000);
    
    return true;
}

void ModbusGateway::setTimeout(uint32_t milliseconds)
{
    timeout = milliseconds;
}

int ModbusGateway::nodePoll()
{
    modbus_t *ctx = (modbus_t *)node->master->mbus;
    unsigned long nowTime;
    
    if (node->master->gateway != NULL && node->master->gateway != this)
        return 0;
    
    if (nodePollState == 0) {
        // Serial.println("Node Request");
        // for (int i = 0; i < node_req_length; i++) {
            // Serial.print(node_req[i], HEX);
            // Serial.print(" ");
        // }
        // Serial.println();
        modbus_set_slave(ctx, node->slaveID);
        modbus_send_raw_request(ctx, node_req, node_req_length);
        nodePollState = 1;
        node->master->gateway = this;
        recvTime = timer_NowTime();
    }
    
    if (nodePollState == 1) {
        nowTime = timer_NowTime();
        node_rsp_length = _modbus_receive_msg_nowait(ctx, node_rsp, MSG_CONFIRMATION);
        if (node_rsp_length < 0 || nowTime - recvTime > node->master->timeout) {
            nodePollState = 0;
            node->master->gateway = NULL;
            // Serial.print("Node Response Error:");
            // Serial.println(node_rsp_length);
            // for (int i = 0; i < 10; i++) {
                // Serial.print(node_rsp[i], HEX);
                // Serial.print(" ");
            // }
            // Serial.println();
            modbus_flush(ctx);
            return -1;
        }
        if (node_rsp_length > 0) {
            nodePollState = 0;
            node->master->gateway = NULL;
            // Serial.println("Node Response");
            // for (int i = 0; i < node_rsp_length; i++) {
                // Serial.print(node_rsp[i], HEX);
                // Serial.print(" ");
            // }
            // Serial.println();
            rsp_length = modbus_shrink_data_unit(ctx, node_rsp, node_rsp_length, rsp);
            return rsp_length;
        }
    }
    
    return 0;
}

int ModbusGateway::poll()
{
    modbus_t *ctx = (modbus_t *)mbus;
    int i, slave, rc, s;
    unsigned long nowTime;
    
    if (pollState < 2 && _mode == MODBUS_TCP) {
        s = modbus_tcp_select(ctx);
        if (s == -1)
            return 0;
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
            pollState = 0;
            for (i = 0; i < nodeNum; i++) {
                if (slave == nodes[i]->slaveID) {
                    node = nodes[i];
                    // for (i = 0; i < req_length; i++) {
                        // Serial.print(req[i], HEX);
                        // Serial.print(" ");
                    // }
                    // Serial.println("==> Request");
                    node_req_length = modbus_shrink_data_unit(ctx, req, req_length, node_req);
                    pollState = 2;
                    break;
                }
            }
            if (pollState == 0)
                modbus_ignore_confirmation(ctx);
        } else if (req_length < 0 || nowTime - recvTime > timeout) {
            modbus_flush(ctx);
            modbus_receive_reset(ctx);
            pollState = 0;
        }
    }
    
    if (pollState == 2) {
        rc = nodePoll();
        if (rc < 0) {
            pollState = 0;
            modbus_reply_exception(ctx, req, MODBUS_EXCEPTION_GATEWAY_TARGET);
        } else if (rc > 0) {
            pollState = 0;
            // for (i = 0; i < rsp_length; i++) {
                // Serial.print(rsp[i], HEX);
                // Serial.print(" ");
            // }
            // Serial.println("==> Response");
            return modbus_send_raw_response(ctx, rsp, rsp_length, req, req_length);
        }
    }
    
    return 0;
}