/*
  CANBus.cpp - DM&P Vortex86 CAN bus library
  Copyright (c) 2014 DY Hung <Dyhung@dmp.com.tw>. All right reserved.

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

#include "CANBus.h"

Vortex86CAN CAN;

Vortex86CAN::Vortex86CAN()
{
	can = NULL;
	_remoteExt = CAN_STDID;
	_remoteID = 0UL;
	
	RxIndex = 0;
	RxSize = 0;
	TxSize = 0;
}

void Vortex86CAN::begin()
{
	begin(CAN_10KBPS);
}

void Vortex86CAN::begin(uint32_t speed)
{
	if (can != NULL) com_Close(can);
	
	if ((can = com_Init(CAN_BUS)) == NULL) return;
	com_SetBPS(can, speed);
	com_SetTimeOut(can, 0L, 500L);
	com_EnableBypass(can);
}

void Vortex86CAN::beginTransmission(uint32_t id, uint8_t ext)
{
	TxFrame.identifier = id;
	TxFrame.type = ext;
	TxSize = 0;
}

uint8_t Vortex86CAN::endTransmission(void)
{
	int i, curByte;
	uint8_t error = 0;
	
	if (can == NULL)
		error = -1;
	else
	{
		i = curByte = 0;
		while (curByte < TxSize)
		{
			TxFrame.Data.byte[i++] = TxBuf[curByte++];
			if (i == 8)
			{
				TxFrame.length = i;
				if (com_WriteCAN(can, &TxFrame) == false)
				{
					error = -1;
					break;
				}
				i = 0;
			}
		}
		if (i > 0)
		{
			TxFrame.length = i;
			if (com_WriteCAN(can, &TxFrame) == false)
			{
				error = -1;
			}
		}
	}
	TxSize = 0;
	
	return error;
}

uint8_t Vortex86CAN::requestFrom(void)
{
	RxIndex = RxSize = 0;
	
	if (can == NULL)
		;
	else if (com_QueryRxQueue(can) > 0)
	{
		com_ReadCAN(can, &RxFrame);
		_remoteID = RxFrame.identifier;
		_remoteExt = RxFrame.type;
		RxSize = RxFrame.length;
	}
	
	return RxSize;
}

size_t Vortex86CAN::write(uint8_t val)
{
	return write(&val, 1);
}

size_t Vortex86CAN::write(const uint8_t *buf, size_t len)
{
	int i = 0;
	
	while (TxSize < CAN_TX_BUFFER_MAX_SIZE && i < len)
		TxBuf[TxSize++] = buf[i++];
	
	return i;
}

int Vortex86CAN::available(void)
{
	return (RxSize - RxIndex);
}

int Vortex86CAN::read(void)
{
	int val = -1;
	
	if (RxIndex < RxSize)
		val = RxFrame.Data.byte[RxIndex++];
	
	return val;
}

int Vortex86CAN::peek(void)
{
	int val = -1;
	
	if (RxIndex < RxSize)
		val = RxFrame.Data.byte[RxIndex];
	
	return val;
}

void Vortex86CAN::flush(void)
{
	RxIndex = RxSize;
}

uint8_t Vortex86CAN::init_Mask(uint8_t num, uint8_t ext, uint32_t ulData)
{
	int _ext;
	uint32_t filt, mask;
	
	if (can == NULL) return CAN_FAIL;
	
	com_DisableBypass(can);
	com_GetIDFilter(can, num, &_ext, &filt, &mask);
	com_AddIDFilter(can, num, ext & 0x01, filt, ulData);
	
	return CAN_OK;
}

uint8_t Vortex86CAN::init_Filt(uint8_t num, uint8_t ext, uint32_t ulData)
{
	int _ext;
	uint32_t filt, mask;
	
	if (can == NULL) return CAN_FAIL;
	
	com_DisableBypass(can);
	com_GetIDFilter(can, num, &_ext, &filt, &mask);
	com_AddIDFilter(can, num, ext & 0x01, ulData, mask);
	
	return CAN_OK;
}

uint8_t Vortex86CAN::sendMsgBuf(uint32_t id, uint8_t ext, uint8_t len, uint8_t *buf)
{
	uint8_t ret;
	
	beginTransmission(id, ext);
	write(buf, len);
	ret = endTransmission();
	
	return (ret == 0) ? (CAN_OK) : (CAN_FAIL);
}

uint8_t Vortex86CAN::readMsgBuf(uint8_t *len, uint8_t *buf)
{
	int i, res;
	
	if ((res = available()) == 0)
		res = requestFrom();
	
	*len = res;
	for (i = 0; i < res; i++)
		buf[i] = read();
	
	return (*len > 0) ? (CAN_OK) : (CAN_NOMSG);
}

uint8_t Vortex86CAN::checkReceive(void)
{
	uint8_t res;
	
	if ((res = available()) == 0)
		res = requestFrom();
		
	return (res > 0) ? (CAN_MSGAVAIL) : (CAN_NOMSG);
}

uint32_t Vortex86CAN::getCanId(void)
{
	return remoteID();
}
