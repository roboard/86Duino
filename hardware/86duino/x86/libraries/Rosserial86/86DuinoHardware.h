/*
  86DuinoHardWare.h - Part of DM&P Vortex86 Rosserial86 library
  Copyright (c) 2015 DY Hung <Dyhung@dmp.com.tw>. All right reserved.

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

#ifndef ROS_86DUINO_HARDWARE_H
#define ROS_86DUINO_HARDWARE_H

class Vortex86Hardware;

class x86DuinoHardware
{
public:
	x86DuinoHardware();
	x86DuinoHardware(char *pName, long baud = 57600L);
	~x86DuinoHardware();
	
	void setBaud(long baud);
	long getBaud();
	
	void init();
	void init(char *pName);
	
	int read();
	
	void write(unsigned char *pBuf, int nByte);
	
	unsigned long time();
	void delay(unsigned long millisecond);

	// only for DOS
	bool setDhcp();
	bool setEthernet(char *ip);
	bool setEthernet(char *ip, char *dns);
	bool setEthernet(char *ip, char *dns, char *gateway);
	bool setEthernet(char *ip, char *dns, char *gateway, char *subnet);
	
private:
	char portName[128];
	long baud_;
	Vortex86Hardware *x86;
};

#endif
