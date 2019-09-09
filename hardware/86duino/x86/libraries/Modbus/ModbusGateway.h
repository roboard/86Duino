/*
  ModbusGateway.h - 86Duino Modbus Gateway library.
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

#ifndef ModbusGateway_h
#define ModbusGateway_h

#include "HardwareSerial.h"

class ModbusGateway {
    public:
        ModbusGateway();
        
        bool begin(int mode, HardwareSerial &serial);
        bool begin(int mode);
        
        bool connect(ModbusMasterNode &mNode);
        void setTimeout(uint32_t milliseconds);
        
        int poll();
        
    private:
        enum { MODBUS_MAX_GATEWAY_NODE = 256,
               MODBUS_MAX_MESSAGE_LENGTH = 513 };
        
        int nodePoll();
        
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
        
        int _mode;
        void *mbus;
};

#endif
