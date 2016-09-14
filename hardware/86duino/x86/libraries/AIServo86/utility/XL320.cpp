/*
 Dynamixel.cpp - XL-320 Half Duplex USART Comunication
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

#include "XL320.h"

//XL-320 EEPROM
#define XL_MODEL_NUMBER_L           0
#define XL_MODEL_NUMBER_H           1
#define XL_VERSION                  2
#define XL_ID                       3
#define XL_BAUD_RATE                4
#define XL_RETURN_DELAY_TIME        5
#define XL_CW_ANGLE_LIMIT_L         6
#define XL_CW_ANGLE_LIMIT_H         7
#define XL_CCW_ANGLE_LIMIT_L        8
#define XL_CCW_ANGLE_LIMIT_H        9
#define XL_CONTROL_MODE             11
#define XL_LIMIT_TEMPERATURE        12
#define XL_DOWN_LIMIT_VOLTAGE       13
#define XL_UP_LIMIT_VOLTAGE         14
#define XL_MAX_TORQUE_L             15
#define XL_MAX_TORQUE_H             16
#define XL_RETURN_LEVEL             17
#define XL_ALARM_SHUTDOWN           18

//XL-320 RAM
#define XL_TORQUE_ENABLE            24
#define XL_LED                      25
#define XL_D_GAIN                   27
#define XL_I_GAIN                   28
#define XL_P_GAIN                   29
#define XL_GOAL_POSITION_L          30
#define XL_GOAL_POSITION_H          31
#define XL_GOAL_SPEED_L             32
#define XL_GOAL_SPEED_H             33
#define XL_TORQUE_LIMIT_L           35
#define XL_TORQUE_LIMIT_H           36
#define XL_PRESENT_POSITION_L       37
#define XL_PRESENT_POSITION_H       38
#define XL_PRESENT_SPEED_L          39
#define XL_PRESENT_SPEED_H          40
#define XL_PRESENT_LOAD_L           41
#define XL_PRESENT_LOAD_H           42
#define XL_PRESENT_VOLTAGE          45
#define XL_PRESENT_TEMPERATURE      46
#define XL_REGISTERED_INSTRUCTION   47
#define XL_MOVING                   49
#define XL_HARDWARE_ERROR_STATUS    50
#define XL_PUNCH_L                  51
#define XL_PUNCH_H                  52

#define XL_START                    255
//XL-320 Commands
#define XL_PING                     1
#define XL_READ_DATA                2
#define XL_WRITE_DATA               3
#define XL_REG_WRITE                4
#define XL_ACTION                   5
#define XL_RESET                    6
#define XL_SYNC_WRITE               131
#define XL_BYTE_READ                1
#define XL_BYTE_READ_POS            2

//XL-320 Colors
#define XL_RED_LED                  1
#define XL_GREEN_LED                2
#define XL_YELLOW_LED               3
#define XL_BLUE_LED                 4
#define XL_PINK_LED                 5
#define XL_BLUE_GREEN_LED           6
#define XL_WHITE_LED                7

static unsigned char syncMsg[150];
static unsigned char calc_chksum(unsigned char *data, int len)
{
	unsigned char chksum = 0;
	for(int i = 2; i < (len - 1); i++)
		chksum += data[i];
	chksum = (~chksum) & 0xFF;
	return chksum;
}

XL320Bus::XL320Bus()
{
	TX_DELAY = 400;
}

void XL320Bus::begin(HardwareSerial &ai_uart, unsigned long baud)
{
	if(&ai_uart == HWSerial[4] || &ai_uart == HWSerial[6])
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

void XL320Bus::update(bool enableMixing)
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

int XL320Bus::read_error()
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
			uart->read();                                    // XL-320 ID
			uart->read();                                    // Length
			Error_Byte = uart->read();                       // Error
			return (Error_Byte);
		}
	}
	return (-1);											 // No XL Response
}

int XL320Bus::boost()
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

int XL320Bus::reset(unsigned char id)
{
	Checksum = (~(id + 2 + XL_RESET))&0xFF;
	
	uart->write(XL_START);
	uart->write(XL_START);
	uart->write(id);
	uart->write(2);
	uart->write(XL_RESET);
	uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 6);

    return (read_error());
}

int XL320Bus::ping(unsigned char id)
{
	Checksum = (~(id + XL_READ_DATA + XL_PING))&0xFF;
	
	uart->write(XL_START);
	uart->write(XL_START);
	uart->write(id);
	uart->write(XL_READ_DATA);
	uart->write(XL_PING);
	uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 6);
    
    return (read_error());
}

int XL320Bus::setID(unsigned char id, unsigned char newID)
{
	if(newID > 252)
		return -1;
	Checksum = (~(id + 4 + XL_WRITE_DATA + XL_ID + newID))&0xFF;

    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
	uart->write(4);
    uart->write(XL_WRITE_DATA);
    uart->write(XL_ID);
    uart->write(newID);
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 8);
    
    return (read_error());
}

int XL320Bus::setBaudrate(unsigned char id, unsigned long baud)
{
	unsigned char Baud_Rate;
	if (baud == 9600)
		Baud_Rate = 0;
	else if (baud == 57600)
		Baud_Rate = 1;
	else if (baud == 115200)
		Baud_Rate = 2;
	else
		Baud_Rate = 3;
    Checksum = (~(id + 4 + XL_WRITE_DATA + XL_BAUD_RATE + Baud_Rate))&0xFF;
	
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
	uart->write(4);
    uart->write(XL_WRITE_DATA);
    uart->write(XL_BAUD_RATE);
    uart->write(Baud_Rate);
    uart->write(Checksum);
    delayMicroseconds(TX_DELAY * 8);
    
    return (read_error());
}

int XL320Bus::move(unsigned char id, int Position)
{
	if(Position > 1023) Position = 1023;
	if(Position < 0) Position = 0;
	char Position_H, Position_L;
    Position_H = (Position >> 8) & 0xFF;
    Position_L = Position & 0xFF;
	Checksum = (~(id + 5 + XL_WRITE_DATA + XL_GOAL_POSITION_L + Position_L + Position_H))&0xFF;
    
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
    uart->write(5);
    uart->write(XL_WRITE_DATA);
    uart->write(XL_GOAL_POSITION_L);
    uart->write(Position_L);
    uart->write(Position_H);
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 9);

    return (read_error());
}

int XL320Bus::moveSpeed(unsigned char id, int Position, int Speed)
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
	Checksum = (~(id + 7 + XL_WRITE_DATA + XL_GOAL_POSITION_L + Position_L + Position_H + Speed_L + Speed_H))&0xFF;
 
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
    uart->write(7);
    uart->write(XL_WRITE_DATA);
    uart->write(XL_GOAL_POSITION_L);
    uart->write(Position_L);
    uart->write(Position_H);
    uart->write(Speed_L);
    uart->write(Speed_H);
    uart->write(Checksum);
    delayMicroseconds(TX_DELAY * 11);
    
    return (read_error());
}

int XL320Bus::setEndless(unsigned char id, bool Status)
{
	if(Status)
	{
		Checksum = (~(id + 4 + XL_WRITE_DATA + XL_CONTROL_MODE + 1))&0xFF;

		uart->write(XL_START);
		uart->write(XL_START);
		uart->write(id);
		uart->write(4);
		uart->write(XL_WRITE_DATA);
		uart->write(XL_CONTROL_MODE);
		uart->write(1);
		uart->write(Checksum);
		delayMicroseconds(TX_DELAY * 8);

		return(read_error());
	}
	else
	{
		turn(id, 0, 0);
		Checksum = (~(id + 4 + XL_WRITE_DATA + XL_CONTROL_MODE + 2))&0xFF;

		uart->write(XL_START);
		uart->write(XL_START);
		uart->write(id);
		uart->write(4);
		uart->write(XL_WRITE_DATA);
		uart->write(XL_CONTROL_MODE);
		uart->write(2);
		uart->write(Checksum);
		delayMicroseconds(TX_DELAY * 8);

		return (read_error());
	}
}

int XL320Bus::turn(unsigned char id, bool SIDE, int Speed)
{
	if(Speed > 1023) Speed = 1023;
	if(Speed < 0) Speed = 0;
	if (SIDE == 0) //Left
	{
		char Speed_H, Speed_L;
		Speed_H = (Speed >> 8) & 0xFF;
		Speed_L = Speed & 0xFF;
		Checksum = (~(id + 5 + XL_WRITE_DATA + XL_GOAL_SPEED_L + Speed_L + Speed_H))&0xFF;

		uart->write(XL_START);
		uart->write(XL_START);
		uart->write(id);
		uart->write(5);
		uart->write(XL_WRITE_DATA);
		uart->write(XL_GOAL_SPEED_L);
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
		Checksum = (~(id + 5 + XL_WRITE_DATA + XL_GOAL_SPEED_L + Speed_L + Speed_H))&0xFF;

		uart->write(XL_START);
		uart->write(XL_START);
		uart->write(id);
		uart->write(5);
		uart->write(XL_WRITE_DATA);
		uart->write(XL_GOAL_SPEED_L);
		uart->write(Speed_L);
		uart->write(Speed_H);
		uart->write(Checksum);
		delayMicroseconds(TX_DELAY * 9);

		return(read_error());
	}
}

int XL320Bus::moveRW(unsigned char id, int Position)
{
	if(Position > 1023) Position = 1023;
	if(Position < 0) Position = 0;
    char Position_H, Position_L;
    Position_H = (Position >> 8) & 0xFF;
    Position_L = Position & 0xFF;
    Checksum = (~(id + 5 + XL_REG_WRITE + XL_GOAL_POSITION_L + Position_L + Position_H))&0xFF;

    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
    uart->write(5);
    uart->write(XL_REG_WRITE);
    uart->write(XL_GOAL_POSITION_L);
    uart->write(Position_L);
    uart->write(Position_H);
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 9);
	
    return (read_error());
}

int XL320Bus::moveSpeedRW(unsigned char id, int Position, int Speed)
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
    Checksum = (~(id + 7 + XL_REG_WRITE + XL_GOAL_POSITION_L + Position_L + Position_H + Speed_L + Speed_H))&0xFF;
	
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
    uart->write(7);
    uart->write(XL_REG_WRITE);
    uart->write(XL_GOAL_POSITION_L);
    uart->write(Position_L);
    uart->write(Position_H);
    uart->write(Speed_L);
    uart->write(Speed_H);
    uart->write(Checksum);
    delayMicroseconds(TX_DELAY * 11);
    
    return (read_error()); 
}

void XL320Bus::action()
{
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(254); // BROADCAST_ID
    uart->write(2);
    uart->write(XL_ACTION);
    uart->write(250);
	delayMicroseconds(TX_DELAY * 6);
}

int XL320Bus::torqueStatus(unsigned char id, bool Status)
{
    Checksum = (~(id + 4 + XL_WRITE_DATA + XL_TORQUE_ENABLE + Status))&0xFF;

    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
    uart->write(4);
    uart->write(XL_WRITE_DATA);
    uart->write(XL_TORQUE_ENABLE);
    uart->write(Status);
    uart->write(Checksum);
    delayMicroseconds(TX_DELAY * 8);
    
    return (read_error());
}

int XL320Bus::setLed(unsigned char id, unsigned char color)
{    
    Checksum = (~(id + 4 + XL_WRITE_DATA + XL_LED + color))&0xFF;

    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
    uart->write(4);
    uart->write(XL_WRITE_DATA);
    uart->write(XL_LED);
    uart->write(color);
    uart->write(Checksum);
    delayMicroseconds(TX_DELAY * 8);
    
    return (read_error());
}

int XL320Bus::readTemperature(unsigned char id)
{	
    Checksum = (~(id + 4 + XL_READ_DATA + XL_PRESENT_TEMPERATURE + XL_BYTE_READ))&0xFF;
    
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
    uart->write(4);
    uart->write(XL_READ_DATA);
    uart->write(XL_PRESENT_TEMPERATURE);
    uart->write(XL_BYTE_READ);
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
			uart->read();                            // XL-320 ID
			uart->read();                            // Length
			if( (Error_Byte = uart->read()) != 0 )   // Error
				return (Error_Byte*(-1));
			Temperature_Byte = uart->read();         // Temperature
		}
    }
	return (Temperature_Byte);               // Returns the read temperature
}

int XL320Bus::readPosition(unsigned char id)
{	
    Checksum = (~(id + 4 + XL_READ_DATA + XL_PRESENT_POSITION_L + XL_BYTE_READ_POS))&0xFF;
  
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
    uart->write(4);
    uart->write(XL_READ_DATA);
    uart->write(XL_PRESENT_POSITION_L);
    uart->write(XL_BYTE_READ_POS);
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
			uart->read();                            // XL-320 ID
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

int XL320Bus::readVoltage(unsigned char id)
{    
    Checksum = (~(id + 4 + XL_READ_DATA + XL_PRESENT_VOLTAGE + XL_BYTE_READ))&0xFF;
    
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
    uart->write(4);
    uart->write(XL_READ_DATA);
    uart->write(XL_PRESENT_VOLTAGE);
    uart->write(XL_BYTE_READ);
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
			uart->read();                            // XL-320 ID
			uart->read();                            // Length
			if( (Error_Byte = uart->read()) != 0 )   // Error
				return (Error_Byte*(-1));
			Voltage_Byte = uart->read();             // Voltage
		}
    }
	return (Voltage_Byte);               // Returns the read Voltage
}

int XL320Bus::setVoltageLimit(unsigned char id, unsigned char DVoltage, unsigned char UVoltage)
{
	Checksum = (~(id + 5 + XL_WRITE_DATA + XL_DOWN_LIMIT_VOLTAGE + DVoltage + UVoltage))&0xFF;
	
	uart->write(XL_START);
	uart->write(XL_START);
	uart->write(id);
	uart->write(5);
	uart->write(XL_WRITE_DATA);
	uart->write(XL_DOWN_LIMIT_VOLTAGE);
    uart->write(DVoltage);
    uart->write(UVoltage);
	uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 9);
	
    return (read_error());
}

int XL320Bus::setSpeed(unsigned char id, int Speed)
{
	if(Speed > 1023) Speed = 1023;
	if(Speed < 0) Speed = 0;
	char Speed_H, Speed_L;
    Speed_H = Speed >> 8;
    Speed_L = Speed;
	Checksum = (~(id + 5 + XL_WRITE_DATA + XL_GOAL_SPEED_L + Speed_H + Speed_L))&0xFF;
	
	uart->write(XL_START);
	uart->write(XL_START);
	uart->write(id);
	uart->write(5);
	uart->write(XL_WRITE_DATA);
	uart->write(XL_GOAL_SPEED_L);
    uart->write(Speed_L);
    uart->write(Speed_H);
	uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 9);
	
    return (read_error());
}

int XL320Bus::setAngleLimit(unsigned char id, int CWLimit, int CCWLimit)
{
	char CW_H, CW_L, CCW_H, CCW_L;
    CW_H = (CWLimit >> 8) & 0xFF;    
    CW_L = CWLimit & 0xFF;
    CCW_H = (CCWLimit >> 8) & 0xFF;
    CCW_L = CCWLimit & 0xFF;  
	Checksum = (~(id + 7 + XL_WRITE_DATA + XL_CW_ANGLE_LIMIT_L + CW_H + CW_L + CCW_H + CCW_L))&0xFF;
	
	uart->write(XL_START);                     
	uart->write(XL_START);
	uart->write(id);
	uart->write(7);
	uart->write(XL_WRITE_DATA);
	uart->write(XL_CW_ANGLE_LIMIT_L);
    uart->write(CW_L);
	uart->write(CW_H);
    uart->write(CCW_L);
	uart->write(CCW_H);
	uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 11);
	
    return (read_error()); 
}

int XL320Bus::setMaxTorque(unsigned char id, int MaxTorque)
{
    char MaxTorque_H, MaxTorque_L;
    MaxTorque_H = (MaxTorque >> 8) & 0xFF;
    MaxTorque_L = MaxTorque & 0xFF;
	Checksum = (~(id + 5 + XL_WRITE_DATA + XL_MAX_TORQUE_L + MaxTorque_L + MaxTorque_H))&0xFF;
    
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
    uart->write(5);
    uart->write(XL_WRITE_DATA);
    uart->write(XL_MAX_TORQUE_L);
    uart->write(MaxTorque_L);
    uart->write(MaxTorque_H);
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 9);
	
    return (read_error());
}

int XL320Bus::setSRL(unsigned char id, unsigned char SRL)
{
	if (SRL > 2)
		return -1;
	Checksum = (~(id + 4 + XL_WRITE_DATA + XL_RETURN_LEVEL + SRL))&0xFF;
	
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
	uart->write(4);
    uart->write(XL_WRITE_DATA);
    uart->write(XL_RETURN_LEVEL);
    uart->write(SRL);
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 8);
    
    return (read_error());
}

int XL320Bus::setRDT(unsigned char id, unsigned char RDT)
{
	Checksum = (~(id + 4 + XL_WRITE_DATA + XL_RETURN_DELAY_TIME + (RDT/2)))&0xFF;
	
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
	uart->write(4);
    uart->write(XL_WRITE_DATA);
    uart->write(XL_RETURN_DELAY_TIME);
    uart->write((RDT/2));
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 8);
    
    return (read_error());
}

int XL320Bus::setShutdownAlarm(unsigned char id, unsigned char SALARM)
{    
	Checksum = (~(id + 4 + XL_ALARM_SHUTDOWN + XL_WRITE_DATA + SALARM))&0xFF;
	
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
	uart->write(4);
    uart->write(XL_WRITE_DATA);
    uart->write(XL_ALARM_SHUTDOWN);
    uart->write(SALARM);
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 8);
    
    return (read_error());
}

int XL320Bus::setPunch(unsigned char id, int Punch)
{
    char Punch_H, Punch_L;
    Punch_H = (Punch >> 8) & 0xFF;
    Punch_L = Punch & 0xFF;
	Checksum = (~(id + 5 + XL_WRITE_DATA + XL_PUNCH_L + Punch_L + Punch_H))&0xFF;
    
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
    uart->write(5);
    uart->write(XL_WRITE_DATA);
    uart->write(XL_PUNCH_L);
    uart->write(Punch_L);
    uart->write(Punch_H);
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 9);
	
    return (read_error());
}

int XL320Bus::setDGain(unsigned char id, unsigned char D)
{
	Checksum = (~(id + 4 + XL_D_GAIN + XL_WRITE_DATA + D))&0xFF;
	
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
	uart->write(4);
    uart->write(XL_WRITE_DATA);
    uart->write(XL_D_GAIN);
    uart->write(D);
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 8);
    
    return (read_error());
}

int XL320Bus::setIGain(unsigned char id, unsigned char I)
{
	Checksum = (~(id + 4 + XL_I_GAIN + XL_WRITE_DATA + I))&0xFF;
	
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
	uart->write(4);
    uart->write(XL_WRITE_DATA);
    uart->write(XL_I_GAIN);
    uart->write(I);
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 8);
    
    return (read_error());
}

int XL320Bus::setPGain(unsigned char id, unsigned char P)
{
	Checksum = (~(id + 4 + XL_P_GAIN + XL_WRITE_DATA + P))&0xFF;
	
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
	uart->write(4);
    uart->write(XL_WRITE_DATA);
    uart->write(XL_P_GAIN);
    uart->write(P);
    uart->write(Checksum);
	delayMicroseconds(TX_DELAY * 8);
    
    return (read_error());
}

int XL320Bus::moving(unsigned char id)
{	
    Checksum = (~(id + 4  + XL_READ_DATA + XL_MOVING + XL_BYTE_READ))&0xFF;
    
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
    uart->write(4);
    uart->write(XL_READ_DATA);
    uart->write(XL_MOVING);
    uart->write(XL_BYTE_READ);
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
			uart->read();                            // XL-320 ID
			uart->read();                            // Length
			if( (Error_Byte = uart->read()) != 0 )   // Error
				return (Error_Byte*(-1));
			Moving_Byte = uart->read();
		}
    }
	return (Moving_Byte);
}

int XL320Bus::RWStatus(unsigned char id)
{	
    Checksum = (~(id + 4  + XL_READ_DATA + XL_REGISTERED_INSTRUCTION + XL_BYTE_READ))&0xFF;
    
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
    uart->write(4);
    uart->write(XL_READ_DATA);
    uart->write(XL_REGISTERED_INSTRUCTION);
    uart->write(XL_BYTE_READ);
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
			uart->read();                            // XL-320 ID
			uart->read();                            // Length
			if( (Error_Byte = uart->read()) != 0 )   // Error
				return (Error_Byte*(-1));
			RWS_Byte = uart->read();
		}
    }
	return (RWS_Byte);
}

int XL320Bus::readRDT(unsigned char id)
{	
    Checksum = (~(id + 4  + XL_READ_DATA + XL_RETURN_DELAY_TIME + XL_BYTE_READ))&0xFF;
    
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
    uart->write(4);
    uart->write(XL_READ_DATA);
    uart->write(XL_RETURN_DELAY_TIME);
    uart->write(XL_BYTE_READ);
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
			uart->read();                            // XL-320 ID
			uart->read();                            // Length
			if( (Error_Byte = uart->read()) != 0 )   // Error
				return (Error_Byte*(-1));
			RWS_Byte = uart->read();
		}
    }
	return (RWS_Byte * 2);
}

int XL320Bus::readHardwareError(unsigned char id)
{
	Checksum = (~(id + 4  + XL_READ_DATA + XL_HARDWARE_ERROR_STATUS + XL_BYTE_READ))&0xFF;
    
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
    uart->write(4);
    uart->write(XL_READ_DATA);
    uart->write(XL_HARDWARE_ERROR_STATUS);
    uart->write(XL_BYTE_READ);
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
			uart->read();                            // XL-320 ID
			uart->read();                            // Length
			if( (Error_Byte = uart->read()) != 0 )   // Error
				return (Error_Byte*(-1));
			RWS_Byte = uart->read();
		}
    }
	return (RWS_Byte);
}

int XL320Bus::readSpeed(unsigned char id)
{	
    Checksum = (~(id + 4  + XL_READ_DATA + XL_PRESENT_SPEED_L + XL_BYTE_READ_POS))&0xFF;
	
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
    uart->write(4);
    uart->write(XL_READ_DATA);
    uart->write(XL_PRESENT_SPEED_L);
    uart->write(XL_BYTE_READ_POS);
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
			uart->read();                            // XL-320 ID
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

int XL320Bus::readLoad(unsigned char id)
{	
    Checksum = (~(id + 4  + XL_READ_DATA + XL_PRESENT_LOAD_L + XL_BYTE_READ_POS))&0xFF;
	
    uart->write(XL_START);
    uart->write(XL_START);
    uart->write(id);
    uart->write(4);
    uart->write(XL_READ_DATA);
    uart->write(XL_PRESENT_LOAD_L);
    uart->write(XL_BYTE_READ_POS);
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
			uart->read();                            // XL-320 ID
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
