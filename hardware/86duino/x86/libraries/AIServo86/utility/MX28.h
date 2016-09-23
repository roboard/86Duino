/*
 Dynamixel.h - MX-28 Half Duplex USART Comunication
 Copyright (c) 2011 Savage Electronics.
 Created by Savage on 27/01/11.
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,  
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 
 Contact: savageelectronics@gmail.com 
 Web:     http://savageelectrtonics.blogspot.com/
 Autor:   Josue Alejandro Savage
 
 Modified by Sayter <sayter@dmp.com.tw> on 13/09/16
 */

#ifndef __MX28_h
#define __MX28_h

#include "AIServoBus.h"

class AIServoBus_ROBOTIS_MX28 : public AIServoBus {
public:
	AIServoBus_ROBOTIS_MX28();
	
	void update(bool enableMixing);
	long long s2a() { return 87891; }
	int maxSpeed() { return 300; }
	int setMaxSpeed(unsigned char id) { return setSpeed(id, 1023); }
	int release(unsigned char id) { return torqueStatus(id, 0); }
	
	void begin(HardwareSerial &ai_uart, unsigned long baud = 1000000);
	int boost(void);
	int reset(unsigned char id);
	int ping(unsigned char id);
	int setID(unsigned char id, unsigned char newID);
	int setBaudrate(unsigned char id, unsigned long baud);
	int move(unsigned char id, int Position);
	int moveSpeed(unsigned char id, int Position, int Speed);
	int setEndless(unsigned char id,bool Status);
	int turn(unsigned char id, bool SIDE, int Speed);
	int moveRW(unsigned char id, int Position);
	int moveSpeedRW(unsigned char id, int Position, int Speed);
	
	void action(void);
	
	int setAngleLimit(unsigned char id, int CWLimit, int CCWLimit);
	int setVoltageLimit(unsigned char id, unsigned char DVoltage, unsigned char UVoltage);
	int setSpeed(unsigned char id, int Speed);
	int setMaxTorque(unsigned char id, int MaxTorque);
	int setSRL(unsigned char id, unsigned char SRL);
	int setRDT(unsigned char id, unsigned char RDT);
	int setLEDAlarm(unsigned char id, unsigned char LEDAlarm);
	int setShutdownAlarm(unsigned char id, unsigned char SALARM);
	int setPunch(unsigned char id, int Punch);
	
	int moving(unsigned char id);
	int lockRegister(unsigned char id);
	int RWStatus(unsigned char id);
	
	int readTemperature(unsigned char id);
	int readVoltage(unsigned char id);
	int readPosition(unsigned char id);
	int readSpeed(unsigned char id);
	int readLoad(unsigned char id);
	int readRDT(unsigned char id);
	
	int torqueStatus(unsigned char id, bool Status);
	int ledStatus(unsigned char id, bool Status);
	
	int setAcceleration(unsigned char id, unsigned char Acceleration);
	int setDGain(unsigned char id, unsigned char D);
	int setIGain(unsigned char id, unsigned char I);
	int setPGain(unsigned char id, unsigned char P);
private:
	const int TIME_OUT = 5;
	
	unsigned char Checksum; 
	unsigned char Direction_Pin;
	unsigned char Time_Counter;
	unsigned char Incoming_Byte;               
	unsigned char Position_High_Byte;
	unsigned char Position_Low_Byte;
	unsigned char Speed_High_Byte;
	unsigned char Speed_Low_Byte;
	unsigned char Load_High_Byte;
	unsigned char Load_Low_Byte;
	
	int Moving_Byte;
	int RWS_Byte;
	int Speed_Long_Byte;
	int Load_Long_Byte;
	int Position_Long_Byte;
	int Temperature_Byte;
	int Voltage_Byte;
	int Error_Byte;
	  
	int read_error();
};

#endif