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

#include "SerialInterface.h"
#include "Ethernet.h"

class ModbusGateway
{
public:
    ModbusGateway();
    ~ModbusGateway();

    bool begin(int mode, SerialInterface &serial);
    bool begin(int mode, uint16_t port = 502);
    bool begin(int mode, uint16_t port, EthernetClass &ethernet);
    void end();

    bool connect(ModbusMasterNode &mNode);
    void setTimeout(uint32_t milliseconds);

    int poll();

private:
    int nodePoll();
    void *_privateData;
};

#endif
