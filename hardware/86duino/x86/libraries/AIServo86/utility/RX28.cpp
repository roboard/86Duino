/*
 Dynamixel.cpp - RX-28 Half Duplex USART Comunication
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

#include "RX28.h"

//RX-28 EEPROM
#define RX_MODEL_NUMBER_L           0
#define RX_MODEL_NUMBER_H           1
#define RX_VERSION                  2
#define RX_ID                       3
#define RX_BAUD_RATE                4
#define RX_RETURN_DELAY_TIME        5
#define RX_CW_ANGLE_LIMIT_L         6
#define RX_CW_ANGLE_LIMIT_H         7
#define RX_CCW_ANGLE_LIMIT_L        8
#define RX_CCW_ANGLE_LIMIT_H        9
#define RX_LIMIT_TEMPERATURE        11
#define RX_DOWN_LIMIT_VOLTAGE       12
#define RX_UP_LIMIT_VOLTAGE         13
#define RX_MAX_TORQUE_L             14
#define RX_MAX_TORQUE_H             15
#define RX_RETURN_LEVEL             16
#define RX_ALARM_LED                17
#define RX_ALARM_SHUTDOWN           18

//RX-28 RAM
#define RX_TORQUE_ENABLE            24
#define RX_LED                      25
#define RX_CW_COMPLIANCE_MARGIN     26
#define RX_CCW_COMPLIANCE_MARGIN    27
#define RX_CW_COMPLIANCE_SLOPE      28
#define RX_CCW_COMPLIANCE_SLOPE     29
#define RX_GOAL_POSITION_L          30
#define RX_GOAL_POSITION_H          31
#define RX_GOAL_SPEED_L             32
#define RX_GOAL_SPEED_H             33
#define RX_TORQUE_LIMIT_L           34
#define RX_TORQUE_LIMIT_H           35
#define RX_PRESENT_POSITION_L       36
#define RX_PRESENT_POSITION_H       37
#define RX_PRESENT_SPEED_L          38
#define RX_PRESENT_SPEED_H          39
#define RX_PRESENT_LOAD_L           40
#define RX_PRESENT_LOAD_H           41
#define RX_PRESENT_VOLTAGE          42
#define RX_PRESENT_TEMPERATURE      43
#define RX_REGISTERED_INSTRUCTION   44
#define RX_MOVING                   46
#define RX_LOCK                     47
#define RX_PUNCH_L                  48
#define RX_PUNCH_H                  49

#define RX_START                    255
//RX-28 Commands
#define RX_PING                     1
#define RX_READ_DATA                2
#define RX_WRITE_DATA               3
#define RX_REG_WRITE                4
#define RX_ACTION                   5
#define RX_RESET                    6
#define RX_SYNC_WRITE               131
#define RX_BYTE_READ                1
#define RX_BYTE_READ_POS            2

static unsigned char syncMsg[150];
static unsigned char calc_chksum(unsigned char *data, int len)
{
	unsigned char chksum = 0;
	for(int i = 2; i < (len - 1); i++)
		chksum += data[i];
	chksum = (~chksum) & 0xFF;
	return chksum;
}

AIServoBus_ROBOTIS_RX28::AIServoBus_ROBOTIS_RX28()
{
	TX_DELAY = 400;
}

void AIServoBus_ROBOTIS_RX28::begin(HardwareSerial &ai_uart, unsigned long baud)
{
	if(&ai_uart != HWSerial[4])
	{
		Serial.println("Failed to begin the bus.");
		return;
	}
	uart = &ai_uart;
	uart->begin(baud, COM_HalfDuplex);
	baudrate = baud;
	TX_DELAY = (1.0/(baud/(8000000.0))) + 1;
	syncMsg[0] = 0xFF;
	syncMsg[1] = 0xFF;
	syncMsg[2] = 0xFE;
	syncMsg[4] = 0x83;
	syncMsg[5] = 0x1E;
	syncMsg[6] = 0x02;
}

void AIServoBus_ROBOTIS_RX28::update(bool enableMixing)
{
	int count = 0;
	int totalMsgs;
	static AIServoList* tmpAIS;
	static AIServoList* cs;
	for(tmpAIS = aisHead; tmpAIS->next != NULL; tmpAIS = tmpAIS->next)
	{
		cs = tmpAIS->next;
		if(cs->endtime == 0L && cs->speed == 0) continue;
		long long position = cs->curposition/s2a();
		if(enableMixing) position += cs->mixoffset/s2a();
		if(position > 1023) position = 1023;
		if(position < 0) position = 0;
		
		syncMsg[ 7 + count*3] = cs->id;
		syncMsg[ 8 + count*3] = position & 0xFF;
		syncMsg[ 9 + count*3] = (position >> 8) & 0xFF;
		count++;
		if(count == 25)
		{
			totalMsgs = 8 + count*3;
			syncMsg[3] = 4 + count*3;
			syncMsg[7 + count*3] = calc_chksum(syncMsg, totalMsgs);
			for(int i = 0; i < totalMsgs; i++)
				uart->write(syncMsg[i]);
			count = 0;
		}
	}
	if(count > 0)
	{
		totalMsgs = 8 + count*3;
		syncMsg[3] = 4 + count*3;
		syncMsg[7 + count*3] = calc_chksum(syncMsg, totalMsgs);
		for(int i = 0; i < totalMsgs; i++)
			uart->write(syncMsg[i]);
	}
}

int AIServoBus_ROBOTIS_RX28::read_error()
{
	Time_Counter = 0;
	while((uart->available() < 5) & (Time_Counter < TIME_OUT))
	{
		Time_Counter++;
		delayMicroseconds(1000);
	}
	
	while(uart->available() > 0)
	{
		Incoming_Byte = uart->read();
		if( (Incoming_Byte == 255) & (uart->peek() == 255) )
		{
			uart->read();                                    // Start Bytes
			uart->read();                                    // RX-28 ID
			uart->read();                                    // Length
			Error_Byte = uart->read();                       // Error
			return (Error_Byte);
		}
	}
	return (-1);											 // No RX Response
}

int AIServoBus_ROBOTIS_RX28::boost()
{
	int res = 0;
	AIServoList* tmp = aisHead;
	while(tmp->next != NULL)
	{
		if(setRDT(tmp->next->id, 0) == 0)
			res++;
		tmp = tmp->next;
	}
	return res;
}

int AIServoBus_ROBOTIS_RX28::reset(unsigned char id)
{
	Checksum = (~(id + 2 + RX_RESET))&0xFF;
	
	uart->write(RX_START);
	uart->write(RX_START);
	uart->write(id);
	uart->write(2);
	uart->write(RX_RESET);
	uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 6);

    return (read_error());
}

int AIServoBus_ROBOTIS_RX28::ping(unsigned char id)
{
	Checksum = (~(id + RX_READ_DATA + RX_PING))&0xFF;
	
	uart->write(RX_START);
	uart->write(RX_START);
	uart->write(id);
	uart->write(RX_READ_DATA);
	uart->write(RX_PING);
	uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 6);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_RX28::setID(unsigned char id, unsigned char newID)
{
	if(newID > 252)
		return -1;
	Checksum = (~(id + 4 + RX_WRITE_DATA + RX_ID + newID))&0xFF;

    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
	uart->write(4);
    uart->write(RX_WRITE_DATA);
    uart->write(RX_ID);
    uart->write(newID);
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 8);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_RX28::setBaudrate(unsigned char id, unsigned long baud)
{
	unsigned char Baud_Rate = (2000000/baud) - 1;
    Checksum = (~(id + 4 + RX_WRITE_DATA + RX_BAUD_RATE + Baud_Rate))&0xFF;
	
    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
	uart->write(4);
    uart->write(RX_WRITE_DATA);
    uart->write(RX_BAUD_RATE);
    uart->write(Baud_Rate);
    uart->write(Checksum);
    delayMicroseconds(TX_DELAY * 8);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_RX28::move(unsigned char id, int Position)
{
	if(Position > 1023) Position = 1023;
	if(Position < 0) Position = 0;
	char Position_H, Position_L;
    Position_H = (Position >> 8) & 0xFF;
    Position_L = Position & 0xFF;
	Checksum = (~(id + 5 + RX_WRITE_DATA + RX_GOAL_POSITION_L + Position_L + Position_H))&0xFF;
    
    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
    uart->write(5);
    uart->write(RX_WRITE_DATA);
    uart->write(RX_GOAL_POSITION_L);
    uart->write(Position_L);
    uart->write(Position_H);
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 9);

    return (read_error());
}

int AIServoBus_ROBOTIS_RX28::moveSpeed(unsigned char id, int Position, int Speed)
{
	if(Position > 1023) Position = 1023;
	if(Position < 0) Position = 0;
	if(Speed > 1023) Speed = 1023;
	if(Speed < 0) Speed = 0;
    char Position_H, Position_L, Speed_H, Speed_L;
    Position_H = (Position >> 8) & 0xFF;
    Position_L = Position & 0xFF;
    Speed_H = Speed >> 8;
    Speed_L = Speed;
	Checksum = (~(id + 7 + RX_WRITE_DATA + RX_GOAL_POSITION_L + Position_L + Position_H + Speed_L + Speed_H))&0xFF;
 
    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
    uart->write(7);
    uart->write(RX_WRITE_DATA);
    uart->write(RX_GOAL_POSITION_L);
    uart->write(Position_L);
    uart->write(Position_H);
    uart->write(Speed_L);
    uart->write(Speed_H);
    uart->write(Checksum);
    delayMicroseconds(TX_DELAY * 11);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_RX28::setEndless(unsigned char id, bool Status)
{
	if(Status)
	{	
		char RX_CCW_AL_LT = 0; // Changing the CCW Angle Limits for Full Rotation.
		Checksum = (~(id + 5 + RX_WRITE_DATA + RX_CCW_ANGLE_LIMIT_L))&0xFF;

		uart->write(RX_START);
		uart->write(RX_START);
		uart->write(id);
		uart->write(5);
		uart->write(RX_WRITE_DATA);
		uart->write(RX_CCW_ANGLE_LIMIT_L );
		uart->write(RX_CCW_AL_LT);
		uart->write(RX_CCW_AL_LT);
		uart->write(Checksum);
		delayMicroseconds(TX_DELAY * 9);

		return(read_error());
	}
	else
	{
		turn(id, 0, 0);
		Checksum = (~(id + 5 + RX_WRITE_DATA + RX_CCW_ANGLE_LIMIT_L + 255 + 3))&0xFF;

		uart->write(RX_START);
		uart->write(RX_START);
		uart->write(id);
		uart->write(5);
		uart->write(RX_WRITE_DATA);
		uart->write(RX_CCW_ANGLE_LIMIT_L);
		uart->write(255);
		uart->write(3);
		uart->write(Checksum);
		delayMicroseconds(TX_DELAY * 9);

		return (read_error());
	}
}

int AIServoBus_ROBOTIS_RX28::turn(unsigned char id, bool SIDE, int Speed)
{
	if(Speed > 1023) Speed = 1023;
	if(Speed < 0) Speed = 0;
	if (SIDE == 0) //Left
	{
		char Speed_H, Speed_L;
		Speed_H = (Speed >> 8) & 0xFF;
		Speed_L = Speed & 0xFF;
		Checksum = (~(id + 5 + RX_WRITE_DATA + RX_GOAL_SPEED_L + Speed_L + Speed_H))&0xFF;

		uart->write(RX_START);
		uart->write(RX_START);
		uart->write(id);
		uart->write(5);
		uart->write(RX_WRITE_DATA);
		uart->write(RX_GOAL_SPEED_L);
		uart->write(Speed_L);
		uart->write(Speed_H);
		uart->write(Checksum);
		delayMicroseconds(TX_DELAY * 9);

		return(read_error());	
	}
	else //Right
	{
		char Speed_H, Speed_L;
		Speed_H = ((Speed >> 8) + 4) & 0xFF;
		Speed_L = Speed & 0xFF;
		Checksum = (~(id + 5 + RX_WRITE_DATA + RX_GOAL_SPEED_L + Speed_L + Speed_H))&0xFF;

		uart->write(RX_START);
		uart->write(RX_START);
		uart->write(id);
		uart->write(5);
		uart->write(RX_WRITE_DATA);
		uart->write(RX_GOAL_SPEED_L);
		uart->write(Speed_L);
		uart->write(Speed_H);
		uart->write(Checksum);
		delayMicroseconds(TX_DELAY * 9);

		return(read_error());
	}
}

int AIServoBus_ROBOTIS_RX28::moveRW(unsigned char id, int Position)
{
	if(Position > 1023) Position = 1023;
	if(Position < 0) Position = 0;
    char Position_H, Position_L;
    Position_H = (Position >> 8) & 0xFF;
    Position_L = Position & 0xFF;
    Checksum = (~(id + 5 + RX_REG_WRITE + RX_GOAL_POSITION_L + Position_L + Position_H))&0xFF;

    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
    uart->write(5);
    uart->write(RX_REG_WRITE);
    uart->write(RX_GOAL_POSITION_L);
    uart->write(Position_L);
    uart->write(Position_H);
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 9);
	
    return (read_error());
}

int AIServoBus_ROBOTIS_RX28::moveSpeedRW(unsigned char id, int Position, int Speed)
{
	if(Position > 1023) Position = 1023;
	if(Position < 0) Position = 0;
	if(Speed > 1023) Speed = 1023;
	if(Speed < 0) Speed = 0;
    char Position_H, Position_L, Speed_H, Speed_L;
    Position_H = (Position >> 8) & 0xFF;    
    Position_L = Position & 0xFF;
    Speed_H = (Speed >> 8) & 0xFF;
    Speed_L = Speed & 0xFF;
    Checksum = (~(id + 7 + RX_REG_WRITE + RX_GOAL_POSITION_L + Position_L + Position_H + Speed_L + Speed_H))&0xFF;
	
    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
    uart->write(7);
    uart->write(RX_REG_WRITE);
    uart->write(RX_GOAL_POSITION_L);
    uart->write(Position_L);
    uart->write(Position_H);
    uart->write(Speed_L);
    uart->write(Speed_H);
    uart->write(Checksum);
    delayMicroseconds(TX_DELAY * 11);
    
    return (read_error()); 
}

void AIServoBus_ROBOTIS_RX28::action()
{
    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(254); //BROADCAST_ID
    uart->write(2);
    uart->write(RX_ACTION);
    uart->write(250);
	delayMicroseconds(TX_DELAY * 6);
}

int AIServoBus_ROBOTIS_RX28::torqueStatus(unsigned char id, bool Status)
{
    Checksum = (~(id + 4 + RX_WRITE_DATA + RX_TORQUE_ENABLE + Status))&0xFF;

    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
    uart->write(4);
    uart->write(RX_WRITE_DATA);
    uart->write(RX_TORQUE_ENABLE);
    uart->write(Status);
    uart->write(Checksum);
    delayMicroseconds(TX_DELAY * 8);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_RX28::ledStatus(unsigned char id, bool Status)
{    
    Checksum = (~(id + 4 + RX_WRITE_DATA + RX_LED + Status))&0xFF;

    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
    uart->write(4);
    uart->write(RX_WRITE_DATA);
    uart->write(RX_LED);
    uart->write(Status);
    uart->write(Checksum);
    delayMicroseconds(TX_DELAY * 8);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_RX28::readTemperature(unsigned char id)
{	
    Checksum = (~(id + 4 + RX_READ_DATA + RX_PRESENT_TEMPERATURE + RX_BYTE_READ))&0xFF;
    
    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
    uart->write(4);
    uart->write(RX_READ_DATA);
    uart->write(RX_PRESENT_TEMPERATURE);
    uart->write(RX_BYTE_READ);
    uart->write(Checksum);
    delayMicroseconds(TX_DELAY * 8);
	
    Temperature_Byte = -1;
    Time_Counter = 0;
    while((uart->available() < 6) & (Time_Counter < TIME_OUT))
	{
		Time_Counter++;
		delayMicroseconds(1000);
    }
	
    while (uart->available() > 0)
	{
		Incoming_Byte = uart->read();
		if ( (Incoming_Byte == 255) & (uart->peek() == 255) )
		{
			uart->read();                            // Start Bytes
			uart->read();                            // RX-28 ID
			uart->read();                            // Length
			if( (Error_Byte = uart->read()) != 0 )   // Error
				return (Error_Byte*(-1));
			Temperature_Byte = uart->read();         // Temperature
		}
    }
	return (Temperature_Byte);               // Returns the read temperature
}

int AIServoBus_ROBOTIS_RX28::readPosition(unsigned char id)
{	
    Checksum = (~(id + 4 + RX_READ_DATA + RX_PRESENT_POSITION_L + RX_BYTE_READ_POS))&0xFF;
  
    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
    uart->write(4);
    uart->write(RX_READ_DATA);
    uart->write(RX_PRESENT_POSITION_L);
    uart->write(RX_BYTE_READ_POS);
    uart->write(Checksum);
    delayMicroseconds(TX_DELAY * 8);
	
    Position_Long_Byte = -1;
	Time_Counter = 0;
    while((uart->available() < 7) & (Time_Counter < TIME_OUT))
	{
		Time_Counter++;
		delayMicroseconds(1000);
    }
	
    while (uart->available() > 0)
	{
		Incoming_Byte = uart->read();
		if ( (Incoming_Byte == 255) & (uart->peek() == 255) )
		{
			uart->read();                            // Start Bytes
			uart->read();                            // RX-28 ID
			uart->read();                            // Length
			if( (Error_Byte = uart->read()) != 0 )   // Error
				return (Error_Byte*(-1));
    
			Position_Low_Byte = uart->read();            // Position Bytes
			Position_High_Byte = uart->read();
			Position_Long_Byte = Position_High_Byte << 8; 
			Position_Long_Byte = Position_Long_Byte + Position_Low_Byte;
		}
    }
	return (Position_Long_Byte);     // Returns the read position
}

int AIServoBus_ROBOTIS_RX28::readVoltage(unsigned char id)
{    
    Checksum = (~(id + 4 + RX_READ_DATA + RX_PRESENT_VOLTAGE + RX_BYTE_READ))&0xFF;
    
    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
    uart->write(4);
    uart->write(RX_READ_DATA);
    uart->write(RX_PRESENT_VOLTAGE);
    uart->write(RX_BYTE_READ);
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 8);
	
    Voltage_Byte = -1;
	Time_Counter = 0;
    while((uart->available() < 6) & (Time_Counter < TIME_OUT))
	{
		Time_Counter++;
		delayMicroseconds(1000);
    }
	
    while (uart->available() > 0)
	{
		Incoming_Byte = uart->read();
		if ( (Incoming_Byte == 255) & (uart->peek() == 255) )
		{
			uart->read();                            // Start Bytes
			uart->read();                            // RX-28 ID
			uart->read();                            // Length
			if( (Error_Byte = uart->read()) != 0 )   // Error
				return (Error_Byte*(-1));
			Voltage_Byte = uart->read();             // Voltage
		}
    }
	return (Voltage_Byte);               // Returns the read Voltage
}

int AIServoBus_ROBOTIS_RX28::setVoltageLimit(unsigned char id, unsigned char DVoltage, unsigned char UVoltage)
{
	Checksum = (~(id + 5 + RX_WRITE_DATA + RX_DOWN_LIMIT_VOLTAGE + DVoltage + UVoltage))&0xFF;
	
	uart->write(RX_START);
	uart->write(RX_START);
	uart->write(id);
	uart->write(5);
	uart->write(RX_WRITE_DATA);
	uart->write(RX_DOWN_LIMIT_VOLTAGE);
    uart->write(DVoltage);
    uart->write(UVoltage);
	uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 9);
	
    return (read_error());
}

int AIServoBus_ROBOTIS_RX28::setSpeed(unsigned char id, int Speed)
{
	if(Speed > 1023) Speed = 1023;
	if(Speed < 0) Speed = 0;
	char Speed_H, Speed_L;
    Speed_H = Speed >> 8;
    Speed_L = Speed;
	Checksum = (~(id + 5 + RX_WRITE_DATA + RX_GOAL_SPEED_L + Speed_H + Speed_L))&0xFF;
	
	uart->write(RX_START);
	uart->write(RX_START);
	uart->write(id);
	uart->write(5);
	uart->write(RX_WRITE_DATA);
	uart->write(RX_GOAL_SPEED_L);
    uart->write(Speed_L);
    uart->write(Speed_H);
	uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 9);
	
    return (read_error());
}

int AIServoBus_ROBOTIS_RX28::setAngleLimit(unsigned char id, int CWLimit, int CCWLimit)
{
	char CW_H, CW_L, CCW_H, CCW_L;
    CW_H = (CWLimit >> 8) & 0xFF;    
    CW_L = CWLimit & 0xFF;
    CCW_H = (CCWLimit >> 8) & 0xFF;
    CCW_L = CCWLimit & 0xFF;  
	Checksum = (~(id + 7 + RX_WRITE_DATA + RX_CW_ANGLE_LIMIT_L + CW_H + CW_L + CCW_H + CCW_L))&0xFF;
	
	uart->write(RX_START);                     
	uart->write(RX_START);
	uart->write(id);
	uart->write(7);
	uart->write(RX_WRITE_DATA);
	uart->write(RX_CW_ANGLE_LIMIT_L);
    uart->write(CW_L);
	uart->write(CW_H);
    uart->write(CCW_L);
	uart->write(CCW_H);
	uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 11);
	
    return (read_error()); 
}

int AIServoBus_ROBOTIS_RX28::setMaxTorque(unsigned char id, int MaxTorque)
{
    char MaxTorque_H, MaxTorque_L;
    MaxTorque_H = (MaxTorque >> 8) & 0xFF;
    MaxTorque_L = MaxTorque & 0xFF;
	Checksum = (~(id + 5 + RX_WRITE_DATA + RX_MAX_TORQUE_L + MaxTorque_L + MaxTorque_H))&0xFF;
    
    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
    uart->write(5);
    uart->write(RX_WRITE_DATA);
    uart->write(RX_MAX_TORQUE_L);
    uart->write(MaxTorque_L);
    uart->write(MaxTorque_H);
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 9);
	
    return (read_error());
}

int AIServoBus_ROBOTIS_RX28::setSRL(unsigned char id, unsigned char SRL)
{
	if (SRL > 2)
		return -1;
	Checksum = (~(id + 4 + RX_WRITE_DATA + RX_RETURN_LEVEL + SRL))&0xFF;
	
    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
	uart->write(4);
    uart->write(RX_WRITE_DATA);
    uart->write(RX_RETURN_LEVEL);
    uart->write(SRL);
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 8);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_RX28::setRDT(unsigned char id, unsigned char RDT)
{
	Checksum = (~(id + 4 + RX_WRITE_DATA + RX_RETURN_DELAY_TIME + RDT))&0xFF;
	
    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
	uart->write(4);
    uart->write(RX_WRITE_DATA);
    uart->write(RX_RETURN_DELAY_TIME);
    uart->write(RDT);
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 8);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_RX28::setLEDAlarm(unsigned char id, unsigned char LEDAlarm)
{    
	Checksum = (~(id + 4 + RX_WRITE_DATA + RX_ALARM_LED + LEDAlarm))&0xFF;
	
    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
	uart->write(4);
    uart->write(RX_WRITE_DATA);
    uart->write(RX_ALARM_LED);
    uart->write(LEDAlarm);
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 8);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_RX28::setShutdownAlarm(unsigned char id, unsigned char SALARM)
{    
	Checksum = (~(id + 4 + RX_ALARM_SHUTDOWN + RX_WRITE_DATA + SALARM))&0xFF;
	
    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
	uart->write(4);
    uart->write(RX_WRITE_DATA);
    uart->write(RX_ALARM_SHUTDOWN);
    uart->write(SALARM);
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 8);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_RX28::setCMargin(unsigned char id, unsigned char CWCMargin, unsigned char CCWCMargin)
{
	Checksum = (~(id + 5 + RX_WRITE_DATA + RX_CW_COMPLIANCE_MARGIN + CWCMargin + CCWCMargin))&0xFF;
	
	uart->write(RX_START);                     
	uart->write(RX_START);
	uart->write(id);
	uart->write(5);
	uart->write(RX_WRITE_DATA);
	uart->write(RX_CW_COMPLIANCE_MARGIN);
    uart->write(CWCMargin);
    uart->write(CCWCMargin);
	uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 9);
	
    return (read_error()); 
}

int AIServoBus_ROBOTIS_RX28::setCSlope(unsigned char id, unsigned char CWCSlope, unsigned char CCWCSlope)
{
	Checksum = (~(id + 5 + RX_WRITE_DATA + RX_CW_COMPLIANCE_SLOPE + CWCSlope + CCWCSlope))&0xFF;
	
	uart->write(RX_START);                     
	uart->write(RX_START);
	uart->write(id);
	uart->write(5);
	uart->write(RX_WRITE_DATA);
	uart->write(RX_CW_COMPLIANCE_SLOPE);
    uart->write(CWCSlope);
    uart->write(CCWCSlope);
	uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 9);
	
    return (read_error()); 
}

int AIServoBus_ROBOTIS_RX28::setPunch(unsigned char id, int Punch)
{
    char Punch_H, Punch_L;
    Punch_H = (Punch >> 8) & 0xFF;
    Punch_L = Punch & 0xFF;
	Checksum = (~(id + 5 + RX_WRITE_DATA + RX_PUNCH_L + Punch_L + Punch_H))&0xFF;
    
    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
    uart->write(5);
    uart->write(RX_WRITE_DATA);
    uart->write(RX_PUNCH_L);
    uart->write(Punch_L);
    uart->write(Punch_H);
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 9);
	
    return (read_error());
}

int AIServoBus_ROBOTIS_RX28::moving(unsigned char id)
{	
    Checksum = (~(id + 4  + RX_READ_DATA + RX_MOVING + RX_BYTE_READ))&0xFF;
    
    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
    uart->write(4);
    uart->write(RX_READ_DATA);
    uart->write(RX_MOVING);
    uart->write(RX_BYTE_READ);
    uart->write(Checksum);
    delayMicroseconds(TX_DELAY * 8);
	
    Moving_Byte = -1;
    Time_Counter = 0;
    while((uart->available() < 6) & (Time_Counter < TIME_OUT))
	{
		Time_Counter++;
		delayMicroseconds(1000);
    }
	
    while (uart->available() > 0)
	{
		Incoming_Byte = uart->read();
		if ( (Incoming_Byte == 255) & (uart->peek() == 255) )
		{
			uart->read();                            // Start Bytes
			uart->read();                            // RX-28 ID
			uart->read();                            // Length
			if( (Error_Byte = uart->read()) != 0 )   // Error
				return (Error_Byte*(-1));
			Moving_Byte = uart->read();
		}
    }
	return (Moving_Byte);
}

int AIServoBus_ROBOTIS_RX28::lockRegister(unsigned char id)
{    
	Checksum = (~(id + 4 + RX_WRITE_DATA + RX_LOCK + 1))&0xFF;
	
    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
	uart->write(4);
    uart->write(RX_WRITE_DATA);
    uart->write(RX_LOCK);
    uart->write(1);
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 8);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_RX28::RWStatus(unsigned char id)
{	
    Checksum = (~(id + 4  + RX_READ_DATA + RX_REGISTERED_INSTRUCTION + RX_BYTE_READ))&0xFF;
    
    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
    uart->write(4);
    uart->write(RX_READ_DATA);
    uart->write(RX_REGISTERED_INSTRUCTION);
    uart->write(RX_BYTE_READ);
    uart->write(Checksum);
    delayMicroseconds(TX_DELAY * 8);
	
    RWS_Byte = -1;
    Time_Counter = 0;
    while((uart->available() < 6) & (Time_Counter < TIME_OUT))
	{
		Time_Counter++;
		delayMicroseconds(1000);
    }
	
    while (uart->available() > 0)
	{
		Incoming_Byte = uart->read();
		if ( (Incoming_Byte == 255) & (uart->peek() == 255) )
		{
			uart->read();                            // Start Bytes
			uart->read();                            // RX-28 ID
			uart->read();                            // Length
			if( (Error_Byte = uart->read()) != 0 )   // Error
				return (Error_Byte*(-1));
			RWS_Byte = uart->read();
		}
    }
	return (RWS_Byte);
}

int AIServoBus_ROBOTIS_RX28::readRDT(unsigned char id)
{	
    Checksum = (~(id + 4  + RX_READ_DATA + RX_RETURN_DELAY_TIME + RX_BYTE_READ))&0xFF;
    
    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
    uart->write(4);
    uart->write(RX_READ_DATA);
    uart->write(RX_RETURN_DELAY_TIME);
    uart->write(RX_BYTE_READ);
    uart->write(Checksum);
    delayMicroseconds(TX_DELAY * 8);
	
    RWS_Byte = -1;
    Time_Counter = 0;
    while((uart->available() < 6) & (Time_Counter < TIME_OUT))
	{
		Time_Counter++;
		delayMicroseconds(1000);
    }
	
    while (uart->available() > 0)
	{
		Incoming_Byte = uart->read();
		if ( (Incoming_Byte == 255) & (uart->peek() == 255) )
		{
			uart->read();                            // Start Bytes
			uart->read();                            // RX-28 ID
			uart->read();                            // Length
			if( (Error_Byte = uart->read()) != 0 )   // Error
				return (Error_Byte*(-1));
			RWS_Byte = uart->read();
		}
    }
	return (RWS_Byte);
}

int AIServoBus_ROBOTIS_RX28::readSpeed(unsigned char id)
{	
    Checksum = (~(id + 4  + RX_READ_DATA + RX_PRESENT_SPEED_L + RX_BYTE_READ_POS))&0xFF;
	
    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
    uart->write(4);
    uart->write(RX_READ_DATA);
    uart->write(RX_PRESENT_SPEED_L);
    uart->write(RX_BYTE_READ_POS);
    uart->write(Checksum);
    delayMicroseconds(TX_DELAY * 8);
	
    Speed_Long_Byte = -1;
	Time_Counter = 0;
    while((uart->available() < 7) & (Time_Counter < TIME_OUT))
	{
		Time_Counter++;
		delayMicroseconds(1000);
    }
	
    while (uart->available() > 0)
	{
		Incoming_Byte = uart->read();
		if ( (Incoming_Byte == 255) & (uart->peek() == 255) )
		{
			uart->read();                            // Start Bytes
			uart->read();                            // RX-28 ID
			uart->read();                            // Length
			if( (Error_Byte = uart->read()) != 0 )   // Error
				return (Error_Byte*(-1));
			
			Speed_Low_Byte = uart->read();
			Speed_High_Byte = uart->read();
			Speed_Long_Byte = Speed_High_Byte << 8; 
			Speed_Long_Byte = Speed_Long_Byte + Speed_Low_Byte;
		}
    }
	return (Speed_Long_Byte);
}

int AIServoBus_ROBOTIS_RX28::readLoad(unsigned char id)
{	
    Checksum = (~(id + 4  + RX_READ_DATA + RX_PRESENT_LOAD_L + RX_BYTE_READ_POS))&0xFF;
	
    uart->write(RX_START);
    uart->write(RX_START);
    uart->write(id);
    uart->write(4);
    uart->write(RX_READ_DATA);
    uart->write(RX_PRESENT_LOAD_L);
    uart->write(RX_BYTE_READ_POS);
    uart->write(Checksum);
    delayMicroseconds(TX_DELAY * 8);
	
    Load_Long_Byte = -1;
	Time_Counter = 0;
    while((uart->available() < 7) & (Time_Counter < TIME_OUT))
	{
		Time_Counter++;
		delayMicroseconds(1000);
    }
	
    while (uart->available() > 0)
	{
		Incoming_Byte = uart->read();
		if ( (Incoming_Byte == 255) & (uart->peek() == 255) )
		{
			uart->read();                            // Start Bytes
			uart->read();                            // RX-28 ID
			uart->read();                            // Length
			if( (Error_Byte = uart->read()) != 0 )   // Error
				return (Error_Byte*(-1));
			
			Load_Low_Byte = uart->read();            // Position Bytes
			Load_High_Byte = uart->read();
			Load_Long_Byte = Load_High_Byte << 8; 
			Load_Long_Byte = Load_Long_Byte + Load_Low_Byte;
		}
    }
	return (Load_Long_Byte);     // Returns the read position
}
