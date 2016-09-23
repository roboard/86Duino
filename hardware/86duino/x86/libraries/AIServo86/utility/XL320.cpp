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

//XL-320 Commands
#define XL_PING                     1
#define XL_READ_DATA                2
#define XL_WRITE_DATA               3
#define XL_REG_WRITE                4
#define XL_ACTION                   5
#define XL_RESET                    6
#define XL_REBOOT                   8
#define XL_SYNC_READ                130
#define XL_SYNC_WRITE               131
#define XL_BYTE_READ                1
#define XL_BYTE_READ_POS            2

static unsigned char syncMsg[150];
static unsigned char instMsg[128];
static unsigned char recvMsg[64];

unsigned short update_crc(unsigned short crc_accum, unsigned char *data_blk_ptr, unsigned short data_blk_size)
{
	unsigned short i, j;
	unsigned short crc_table[256] = {0x0000,
	                                0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
	                                0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027,
	                                0x0022, 0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D,
	                                0x8077, 0x0072, 0x0050, 0x8055, 0x805F, 0x005A, 0x804B,
	                                0x004E, 0x0044, 0x8041, 0x80C3, 0x00C6, 0x00CC, 0x80C9,
	                                0x00D8, 0x80DD, 0x80D7, 0x00D2, 0x00F0, 0x80F5, 0x80FF,
	                                0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1, 0x00A0, 0x80A5,
	                                0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1, 0x8093,
	                                0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
	                                0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197,
	                                0x0192, 0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE,
	                                0x01A4, 0x81A1, 0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB,
	                                0x01FE, 0x01F4, 0x81F1, 0x81D3, 0x01D6, 0x01DC, 0x81D9,
	                                0x01C8, 0x81CD, 0x81C7, 0x01C2, 0x0140, 0x8145, 0x814F,
	                                0x014A, 0x815B, 0x015E, 0x0154, 0x8151, 0x8173, 0x0176,
	                                0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162, 0x8123,
	                                0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
	                                0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104,
	                                0x8101, 0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D,
	                                0x8317, 0x0312, 0x0330, 0x8335, 0x833F, 0x033A, 0x832B,
	                                0x032E, 0x0324, 0x8321, 0x0360, 0x8365, 0x836F, 0x036A,
	                                0x837B, 0x037E, 0x0374, 0x8371, 0x8353, 0x0356, 0x035C,
	                                0x8359, 0x0348, 0x834D, 0x8347, 0x0342, 0x03C0, 0x83C5,
	                                0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1, 0x83F3,
	                                0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
	                                0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7,
	                                0x03B2, 0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E,
	                                0x0384, 0x8381, 0x0280, 0x8285, 0x828F, 0x028A, 0x829B,
	                                0x029E, 0x0294, 0x8291, 0x82B3, 0x02B6, 0x02BC, 0x82B9,
	                                0x02A8, 0x82AD, 0x82A7, 0x02A2, 0x82E3, 0x02E6, 0x02EC,
	                                0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2, 0x02D0, 0x82D5,
	                                0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1, 0x8243,
	                                0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
	                                0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264,
	                                0x8261, 0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E,
	                                0x0234, 0x8231, 0x8213, 0x0216, 0x021C, 0x8219, 0x0208,
	                                0x820D, 0x8207, 0x0202 };

	for(j = 0; j < data_blk_size; j++)
	{
		i = ((unsigned short)(crc_accum >> 8) ^ *data_blk_ptr++) & 0xFF;
		crc_accum = (crc_accum << 8) ^ crc_table[i];
	}

	return crc_accum;
}

AIServoBus_ROBOTIS_XL320::AIServoBus_ROBOTIS_XL320()
{
	TX_DELAY = 400;
}

void AIServoBus_ROBOTIS_XL320::begin(HardwareSerial &ai_uart, unsigned long baud)
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
	syncMsg[2] = 0xFD;
	syncMsg[3] = 0x00;
	syncMsg[4] = 0xFE;
	syncMsg[7] = 0x83;
	syncMsg[8] = XL_GOAL_POSITION_L;
	syncMsg[9] = 0;
	syncMsg[10]= 2;
	syncMsg[11]= 0;
	
	instMsg[0] = 0xFF;
	instMsg[1] = 0xFF;
	instMsg[2] = 0xFD;
	instMsg[3] = 0x00;
}

void AIServoBus_ROBOTIS_XL320::update(bool enableMixing)
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
		
		syncMsg[ 12 + count*3] = cs->id;
		syncMsg[ 13 + count*3] = position & 0xFF;
		syncMsg[ 14 + count*3] = (position >> 8) & 0xFF;
		count++;
		if(count == 25)
		{
			totalMsgs = 14 + count*3;
			syncMsg[5] = 7 + count*3;
			syncMsg[6] = 0;
			CRC = update_crc(0, syncMsg, totalMsgs-2);
			syncMsg[12 + count*3] = (CRC & 0xFF);
			syncMsg[13 + count*3] = ((CRC >> 8) & 0xFF);
			for(int i = 0; i < totalMsgs; i++)
				uart->write(syncMsg[i]);
			count = 0;
		}
	}
	if(count > 0)
	{
		totalMsgs = 14 + count*3;
		syncMsg[5] = 7 + count*3;
		syncMsg[6] = 0;
		CRC = update_crc(0, syncMsg, totalMsgs-2);
		syncMsg[12 + count*3] = (CRC & 0xFF);
		syncMsg[13 + count*3] = ((CRC >> 8) & 0xFF);
		for(int i = 0; i < totalMsgs; i++)
			uart->write(syncMsg[i]);
	}
}

int AIServoBus_ROBOTIS_XL320::read_error()
{
	Time_Counter = 0;
	while((uart->available() < 11) & (Time_Counter < TIME_OUT))
	{
		Time_Counter++;
		delayMicroseconds(1000);
	}
	
	while(uart->available() > 0)
	{
		recvMsg[0] = uart->read();
		if( (recvMsg[0] == 255) & (uart->peek() == 255) )
		{
			recvMsg[1] = uart->read();                       // 0xFF
			recvMsg[2] = uart->read();                       // 0xFD
			recvMsg[3] = uart->read();                       // 0x00
			recvMsg[4] = uart->read();                       // XL-320 ID
			recvMsg[5] = uart->read();                       // Length_L
			recvMsg[6] = uart->read();                       // Length_H
			recvMsg[7] = uart->read();                       // 0x55
			recvMsg[8] = uart->read();                       // Error
			recvMsg[9] = uart->read();                       // CRC_L
			recvMsg[10] = uart->read();                      // CRC_H
			CRC = update_crc(0, recvMsg, 9);
			if((recvMsg[9] != (CRC&0xFF)) || (recvMsg[10] != ((CRC >> 8)&0xFF)))
				return -2;
			return (recvMsg[8]);
		}
	}
	return (-1);											 // No XL Response
}

int AIServoBus_ROBOTIS_XL320::boost()
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

int AIServoBus_ROBOTIS_XL320::reset(unsigned char id)
{
	instMsg[4] = id;
	instMsg[5] = 3;
	instMsg[6] = 0;
	instMsg[7] = XL_RESET;
	CRC = update_crc(0, instMsg, 8);
	instMsg[8] = CRC & 0xFF;
	instMsg[9] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 10; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 10);

    return (read_error());
}

int AIServoBus_ROBOTIS_XL320::ping(unsigned char id)
{
	instMsg[4] = id;
	instMsg[5] = 3;
	instMsg[6] = 0;
	instMsg[7] = XL_PING;
	CRC = update_crc(0, instMsg, 8);
	instMsg[8] = CRC & 0xFF;
	instMsg[9] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 10; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 10);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_XL320::reboot(unsigned char id)
{
	instMsg[4] = id;
	instMsg[5] = 3;
	instMsg[6] = 0;
	instMsg[7] = XL_REBOOT;
	CRC = update_crc(0, instMsg, 8);
	instMsg[8] = CRC & 0xFF;
	instMsg[9] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 10; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 10);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_XL320::setID(unsigned char id, unsigned char newID)
{
	if(newID > 252)
		return -1;

	instMsg[4] = id;
	instMsg[5] = 6;
	instMsg[6] = 0;
	instMsg[7] = XL_WRITE_DATA;
	instMsg[8] = XL_ID;
	instMsg[9] = 0;
	instMsg[10] = newID;
	CRC = update_crc(0, instMsg, 11);
	instMsg[11] = CRC & 0xFF;
	instMsg[12] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 13; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 13);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_XL320::setBaudrate(unsigned char id, unsigned long baud)
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
	
	instMsg[4] = id;
	instMsg[5] = 6;
	instMsg[6] = 0;
	instMsg[7] = XL_WRITE_DATA;
	instMsg[8] = XL_BAUD_RATE;
	instMsg[9] = 0;
	instMsg[10] = Baud_Rate;
	CRC = update_crc(0, instMsg, 11);
	instMsg[11] = CRC & 0xFF;
	instMsg[12] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 13; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 13);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_XL320::move(unsigned char id, int Position)
{
	if(Position > 1023) Position = 1023;
	if(Position < 0) Position = 0;
	char Position_H, Position_L;
    Position_H = (Position >> 8) & 0xFF;
    Position_L = Position & 0xFF;
	
	instMsg[4] = id;
	instMsg[5] = 7;
	instMsg[6] = 0;
	instMsg[7] = XL_WRITE_DATA;
	instMsg[8] = XL_GOAL_POSITION_L;
	instMsg[9] = 0;
	instMsg[10] = Position_L;
	instMsg[11] = Position_H;
	CRC = update_crc(0, instMsg, 12);
	instMsg[12] = CRC & 0xFF;
	instMsg[13] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 14; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 14);

    return (read_error());
}

int AIServoBus_ROBOTIS_XL320::moveSpeed(unsigned char id, int Position, int Speed)
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
	
	instMsg[4] = id;
	instMsg[5] = 9;
	instMsg[6] = 0;
	instMsg[7] = XL_WRITE_DATA;
	instMsg[8] = XL_GOAL_POSITION_L;
	instMsg[9] = 0;
	instMsg[10] = Position_L;
	instMsg[11] = Position_H;
	instMsg[12] = Speed_L;
	instMsg[13] = Speed_H;
	CRC = update_crc(0, instMsg, 14);
	instMsg[14] = CRC & 0xFF;
	instMsg[15] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 16; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 16);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_XL320::setEndless(unsigned char id, bool Status)
{
	if(Status)
	{
		instMsg[4] = id;
		instMsg[5] = 6;
		instMsg[6] = 0;
		instMsg[7] = XL_WRITE_DATA;
		instMsg[8] = XL_CONTROL_MODE;
		instMsg[9] = 0;
		instMsg[10] = 1;
		CRC = update_crc(0, instMsg, 11);
		instMsg[11] = CRC & 0xFF;
		instMsg[12] = (CRC>>8) & 0xFF;
		
		for(int i = 0; i < 13; i++)
			uart->write(instMsg[i]);
		
		delayMicroseconds(TX_DELAY * 13);

		return(read_error());
	}
	else
	{
		turn(id, 0, 0);
		
		instMsg[4] = id;
		instMsg[5] = 6;
		instMsg[6] = 0;
		instMsg[7] = XL_WRITE_DATA;
		instMsg[8] = XL_CONTROL_MODE;
		instMsg[9] = 0;
		instMsg[10] = 2;
		CRC = update_crc(0, instMsg, 11);
		instMsg[11] = CRC & 0xFF;
		instMsg[12] = (CRC>>8) & 0xFF;
		
		for(int i = 0; i < 13; i++)
			uart->write(instMsg[i]);
		
		delayMicroseconds(TX_DELAY * 13);

		return (read_error());
	}
}

int AIServoBus_ROBOTIS_XL320::turn(unsigned char id, bool SIDE, int Speed)
{
	if(Speed > 1023) Speed = 1023;
	if(Speed < 0) Speed = 0;
	if (SIDE == 0) //Left
	{
		char Speed_H, Speed_L;
		Speed_H = (Speed >> 8) & 0xFF;
		Speed_L = Speed & 0xFF;
		
		instMsg[4] = id;
		instMsg[5] = 7;
		instMsg[6] = 0;
		instMsg[7] = XL_WRITE_DATA;
		instMsg[8] = XL_GOAL_SPEED_L;
		instMsg[9] = 0;
		instMsg[10] = Speed_L;
		instMsg[11] = Speed_H;
		CRC = update_crc(0, instMsg, 12);
		instMsg[12] = CRC & 0xFF;
		instMsg[13] = (CRC>>8) & 0xFF;
		
		for(int i = 0; i < 14; i++)
			uart->write(instMsg[i]);
		
		delayMicroseconds(TX_DELAY * 14);
		
		return(read_error());	
	}
	else //Right
	{
		char Speed_H, Speed_L;
		Speed_H = ((Speed >> 8) + 4) & 0xFF;
		Speed_L = Speed & 0xFF;
		
		instMsg[4] = id;
		instMsg[5] = 7;
		instMsg[6] = 0;
		instMsg[7] = XL_WRITE_DATA;
		instMsg[8] = XL_GOAL_SPEED_L;
		instMsg[9] = 0;
		instMsg[10] = Speed_L;
		instMsg[11] = Speed_H;
		CRC = update_crc(0, instMsg, 12);
		instMsg[12] = CRC & 0xFF;
		instMsg[13] = (CRC>>8) & 0xFF;
		
		for(int i = 0; i < 14; i++)
			uart->write(instMsg[i]);
		
		delayMicroseconds(TX_DELAY * 14);

		return(read_error());
	}
}

int AIServoBus_ROBOTIS_XL320::moveRW(unsigned char id, int Position)
{
	if(Position > 1023) Position = 1023;
	if(Position < 0) Position = 0;
    char Position_H, Position_L;
    Position_H = (Position >> 8) & 0xFF;
    Position_L = Position & 0xFF;
    
	instMsg[4] = id;
	instMsg[5] = 7;
	instMsg[6] = 0;
	instMsg[7] = XL_REG_WRITE;
	instMsg[8] = XL_GOAL_POSITION_L;
	instMsg[9] = 0;
	instMsg[10] = Position_L;
	instMsg[11] = Position_H;
	CRC = update_crc(0, instMsg, 12);
	instMsg[12] = CRC & 0xFF;
	instMsg[13] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 14; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 14);
	
    return (read_error());
}

int AIServoBus_ROBOTIS_XL320::moveSpeedRW(unsigned char id, int Position, int Speed)
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
	
	instMsg[4] = id;
	instMsg[5] = 9;
	instMsg[6] = 0;
	instMsg[7] = XL_REG_WRITE;
	instMsg[8] = XL_GOAL_POSITION_L;
	instMsg[9] = 0;
	instMsg[10] = Position_L;
	instMsg[11] = Position_H;
	instMsg[12] = Speed_L;
	instMsg[13] = Speed_H;
	CRC = update_crc(0, instMsg, 14);
	instMsg[14] = CRC & 0xFF;
	instMsg[15] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 16; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 16);
    
    return (read_error()); 
}

void AIServoBus_ROBOTIS_XL320::action()
{
	instMsg[4] = 254;
	instMsg[5] = 3;
	instMsg[6] = 0;
	instMsg[7] = XL_ACTION;
	CRC = update_crc(0, instMsg, 8);
	instMsg[8] = CRC & 0xFF;
	instMsg[9] = (CRC>>8) & 0xFF;
	delayMicroseconds(TX_DELAY * 10);
}

int AIServoBus_ROBOTIS_XL320::torqueStatus(unsigned char id, bool Status)
{
	instMsg[4] = id;
	instMsg[5] = 6;
	instMsg[6] = 0;
	instMsg[7] = XL_WRITE_DATA;
	instMsg[8] = XL_TORQUE_ENABLE;
	instMsg[9] = 0;
	instMsg[10] = Status;
	CRC = update_crc(0, instMsg, 11);
	instMsg[11] = CRC & 0xFF;
	instMsg[12] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 13; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 13);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_XL320::setLed(unsigned char id, unsigned char color)
{    
    instMsg[4] = id;
	instMsg[5] = 6;
	instMsg[6] = 0;
	instMsg[7] = XL_WRITE_DATA;
	instMsg[8] = XL_LED;
	instMsg[9] = 0;
	instMsg[10] = color;
	CRC = update_crc(0, instMsg, 11);
	instMsg[11] = CRC & 0xFF;
	instMsg[12] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 13; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 13);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_XL320::readTemperature(unsigned char id)
{	
    instMsg[4] = id;
	instMsg[5] = 7;
	instMsg[6] = 0;
	instMsg[7] = XL_READ_DATA;
	instMsg[8] = XL_PRESENT_TEMPERATURE;
	instMsg[9] = 0;
	instMsg[10] = XL_BYTE_READ;
	instMsg[11] = 0;
	CRC = update_crc(0, instMsg, 12);
	instMsg[12] = CRC & 0xFF;
	instMsg[13] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 14; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 14);
	
    Temperature_Byte = -1;
    Time_Counter = 0;
    while((uart->available() < 12) & (Time_Counter < TIME_OUT))
	{
		Time_Counter++;
		delayMicroseconds(1000);
    }
	
    while (uart->available() > 0)
	{
		recvMsg[0] = uart->read();
		if ( (recvMsg[0] == 255) & (uart->peek() == 255) )
		{
			recvMsg[1] = uart->read();               // 0xFF
			recvMsg[2] = uart->read();               // 0xFD
			recvMsg[3] = uart->read();               // 0x00
			recvMsg[4] = uart->read();               // XL-320 ID
			recvMsg[5] = uart->read();               // Length_L
			recvMsg[6] = uart->read();               // Length_H
			recvMsg[7] = uart->read();               // 0x55
			recvMsg[8] = uart->read();               // Error
			recvMsg[9] = uart->read();               // Temperature
			recvMsg[10] = uart->read();              // CRC_L
			recvMsg[11] = uart->read();              // CRC_H
			CRC = update_crc(0, recvMsg, 10);
			if((recvMsg[10] != (CRC&0xFF)) || (recvMsg[11] != ((CRC >> 8)&0xFF)))
				return -2;
			if (recvMsg[8] != 0)
				return recvMsg[8];
			Temperature_Byte = recvMsg[9];
		}
    }
	return (Temperature_Byte);                        // Returns the read temperature
}

int AIServoBus_ROBOTIS_XL320::readPosition(unsigned char id)
{
	instMsg[4] = id;
	instMsg[5] = 7;
	instMsg[6] = 0;
	instMsg[7] = XL_READ_DATA;
	instMsg[8] = XL_PRESENT_POSITION_L;
	instMsg[9] = 0;
	instMsg[10] = XL_BYTE_READ_POS;
	instMsg[11] = 0;
	CRC = update_crc(0, instMsg, 12);
	instMsg[12] = CRC & 0xFF;
	instMsg[13] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 14; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 14);
	
    Position_Long_Byte = -1;
    Time_Counter = 0;
    while((uart->available() < 13) & (Time_Counter < TIME_OUT))
	{
		Time_Counter++;
		delayMicroseconds(1000);
    }
	
	while (uart->available() > 0)
	{
		recvMsg[0] = uart->read();
		if ( (recvMsg[0] == 255) & (uart->peek() == 255) )
		{
			recvMsg[1] = uart->read();               // 0xFF
			recvMsg[2] = uart->read();               // 0xFD
			recvMsg[3] = uart->read();               // 0x00
			recvMsg[4] = uart->read();               // XL-320 ID
			recvMsg[5] = uart->read();               // Length_L
			recvMsg[6] = uart->read();               // Length_H
			recvMsg[7] = uart->read();               // 0x55
			recvMsg[8] = uart->read();               // Error
			recvMsg[9] = uart->read();               // Position_L
			recvMsg[10] = uart->read();              // Position_H
			recvMsg[11] = uart->read();              // CRC_L
			recvMsg[12] = uart->read();              // CRC_H
			CRC = update_crc(0, recvMsg, 11);
			if((recvMsg[11] != (CRC&0xFF)) || (recvMsg[12] != ((CRC >> 8)&0xFF)))
				return -2;
			if (recvMsg[8] != 0)
				return recvMsg[8];
			Position_Long_Byte = (recvMsg[10] << 8) + recvMsg[9];
		}
    }
	return (Position_Long_Byte);        // Returns the read Position
}

int AIServoBus_ROBOTIS_XL320::readVoltage(unsigned char id)
{    
    instMsg[4] = id;
	instMsg[5] = 7;
	instMsg[6] = 0;
	instMsg[7] = XL_READ_DATA;
	instMsg[8] = XL_PRESENT_VOLTAGE;
	instMsg[9] = 0;
	instMsg[10] = XL_BYTE_READ;
	instMsg[11] = 0;
	CRC = update_crc(0, instMsg, 12);
	instMsg[12] = CRC & 0xFF;
	instMsg[13] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 14; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 14);
	
    Voltage_Byte = -1;
    Time_Counter = 0;
    while((uart->available() < 12) & (Time_Counter < TIME_OUT))
	{
		Time_Counter++;
		delayMicroseconds(1000);
    }
	
	while (uart->available() > 0)
	{
		recvMsg[0] = uart->read();
		if ( (recvMsg[0] == 255) & (uart->peek() == 255) )
		{
			recvMsg[1] = uart->read();               // 0xFF
			recvMsg[2] = uart->read();               // 0xFD
			recvMsg[3] = uart->read();               // 0x00
			recvMsg[4] = uart->read();               // XL-320 ID
			recvMsg[5] = uart->read();               // Length_L
			recvMsg[6] = uart->read();               // Length_H
			recvMsg[7] = uart->read();               // 0x55
			recvMsg[8] = uart->read();               // Error
			recvMsg[9] = uart->read();               // Voltage
			recvMsg[10] = uart->read();              // CRC_L
			recvMsg[11] = uart->read();              // CRC_H
			CRC = update_crc(0, recvMsg, 10);
			if((recvMsg[10] != (CRC&0xFF)) || (recvMsg[11] != ((CRC >> 8)&0xFF)))
				return -2;
			if (recvMsg[8] != 0)
				return recvMsg[8];
			Voltage_Byte = recvMsg[9];
		}
    }
	return (Voltage_Byte);                           // Returns the read Voltage
}

int AIServoBus_ROBOTIS_XL320::setVoltageLimit(unsigned char id, unsigned char DVoltage, unsigned char UVoltage)
{
	instMsg[4] = id;
	instMsg[5] = 7;
	instMsg[6] = 0;
	instMsg[7] = XL_WRITE_DATA;
	instMsg[8] = XL_DOWN_LIMIT_VOLTAGE;
	instMsg[9] = 0;
	instMsg[10] = DVoltage;
	instMsg[11] = UVoltage;
	CRC = update_crc(0, instMsg, 12);
	instMsg[12] = CRC & 0xFF;
	instMsg[13] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 14; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 14);
	
    return (read_error());
}

int AIServoBus_ROBOTIS_XL320::setSpeed(unsigned char id, int Speed)
{
	if(Speed > 1023) Speed = 1023;
	if(Speed < 0) Speed = 0;
	char Speed_H, Speed_L;
    Speed_H = Speed >> 8;
    Speed_L = Speed;
	
	instMsg[4] = id;
	instMsg[5] = 7;
	instMsg[6] = 0;
	instMsg[7] = XL_WRITE_DATA;
	instMsg[8] = XL_GOAL_SPEED_L;
	instMsg[9] = 0;
	instMsg[10] = Speed_L;
	instMsg[11] = Speed_H;
	CRC = update_crc(0, instMsg, 12);
	instMsg[12] = CRC & 0xFF;
	instMsg[13] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 14; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 14);
	
    return (read_error());
}

int AIServoBus_ROBOTIS_XL320::setAngleLimit(unsigned char id, int CWLimit, int CCWLimit)
{
	char CW_H, CW_L, CCW_H, CCW_L;
    CW_H = (CWLimit >> 8) & 0xFF;    
    CW_L = CWLimit & 0xFF;
    CCW_H = (CCWLimit >> 8) & 0xFF;
    CCW_L = CCWLimit & 0xFF;  
	
	instMsg[4] = id;
	instMsg[5] = 9;
	instMsg[6] = 0;
	instMsg[7] = XL_WRITE_DATA;
	instMsg[8] = XL_CW_ANGLE_LIMIT_L;
	instMsg[9] = 0;
	instMsg[10] = CW_L;
	instMsg[11] = CW_H;
	instMsg[12] = CCW_L;
	instMsg[13] = CCW_H;
	CRC = update_crc(0, instMsg, 14);
	instMsg[14] = CRC & 0xFF;
	instMsg[15] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 16; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 16);
	
    return (read_error()); 
}

int AIServoBus_ROBOTIS_XL320::setMaxTorque(unsigned char id, int MaxTorque)
{
    char MaxTorque_H, MaxTorque_L;
    MaxTorque_H = (MaxTorque >> 8) & 0xFF;
    MaxTorque_L = MaxTorque & 0xFF;
	
	instMsg[4] = id;
	instMsg[5] = 7;
	instMsg[6] = 0;
	instMsg[7] = XL_WRITE_DATA;
	instMsg[8] = XL_MAX_TORQUE_L;
	instMsg[9] = 0;
	instMsg[10] = MaxTorque_L;
	instMsg[11] = MaxTorque_H;
	CRC = update_crc(0, instMsg, 12);
	instMsg[12] = CRC & 0xFF;
	instMsg[13] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 14; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 14);
	
    return (read_error());
}

int AIServoBus_ROBOTIS_XL320::setSRL(unsigned char id, unsigned char SRL)
{
	if (SRL > 2)
		return -1;
	
	instMsg[4] = id;
	instMsg[5] = 6;
	instMsg[6] = 0;
	instMsg[7] = XL_WRITE_DATA;
	instMsg[8] = XL_RETURN_LEVEL;
	instMsg[9] = 0;
	instMsg[10] = SRL;
	CRC = update_crc(0, instMsg, 11);
	instMsg[11] = CRC & 0xFF;
	instMsg[12] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 13; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 13);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_XL320::setRDT(unsigned char id, unsigned char RDT)
{
	instMsg[4] = id;
	instMsg[5] = 6;
	instMsg[6] = 0;
	instMsg[7] = XL_WRITE_DATA;
	instMsg[8] = XL_RETURN_DELAY_TIME;
	instMsg[9] = 0;
	instMsg[10] = RDT;
	CRC = update_crc(0, instMsg, 11);
	instMsg[11] = CRC & 0xFF;
	instMsg[12] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 13; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 13);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_XL320::setShutdownAlarm(unsigned char id, unsigned char SALARM)
{    
	instMsg[4] = id;
	instMsg[5] = 6;
	instMsg[6] = 0;
	instMsg[7] = XL_WRITE_DATA;
	instMsg[8] = XL_ALARM_SHUTDOWN;
	instMsg[9] = 0;
	instMsg[10] = SALARM;
	CRC = update_crc(0, instMsg, 11);
	instMsg[11] = CRC & 0xFF;
	instMsg[12] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 13; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 13);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_XL320::setPunch(unsigned char id, int Punch)
{
    char Punch_H, Punch_L;
    Punch_H = (Punch >> 8) & 0xFF;
    Punch_L = Punch & 0xFF;
	
	instMsg[4] = id;
	instMsg[5] = 7;
	instMsg[6] = 0;
	instMsg[7] = XL_WRITE_DATA;
	instMsg[8] = XL_PUNCH_L;
	instMsg[9] = 0;
	instMsg[10] = Punch_L;
	instMsg[11] = Punch_H;
	CRC = update_crc(0, instMsg, 12);
	instMsg[12] = CRC & 0xFF;
	instMsg[13] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 14; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 14);
	
    return (read_error());
}

int AIServoBus_ROBOTIS_XL320::setDGain(unsigned char id, unsigned char D)
{
	instMsg[4] = id;
	instMsg[5] = 6;
	instMsg[6] = 0;
	instMsg[7] = XL_WRITE_DATA;
	instMsg[8] = XL_D_GAIN;
	instMsg[9] = 0;
	instMsg[10] = D;
	CRC = update_crc(0, instMsg, 11);
	instMsg[11] = CRC & 0xFF;
	instMsg[12] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 13; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 13);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_XL320::setIGain(unsigned char id, unsigned char I)
{
	instMsg[4] = id;
	instMsg[5] = 6;
	instMsg[6] = 0;
	instMsg[7] = XL_WRITE_DATA;
	instMsg[8] = XL_I_GAIN;
	instMsg[9] = 0;
	instMsg[10] = I;
	CRC = update_crc(0, instMsg, 11);
	instMsg[11] = CRC & 0xFF;
	instMsg[12] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 13; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 13);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_XL320::setPGain(unsigned char id, unsigned char P)
{
	instMsg[4] = id;
	instMsg[5] = 6;
	instMsg[6] = 0;
	instMsg[7] = XL_WRITE_DATA;
	instMsg[8] = XL_P_GAIN;
	instMsg[9] = 0;
	instMsg[10] = P;
	CRC = update_crc(0, instMsg, 11);
	instMsg[11] = CRC & 0xFF;
	instMsg[12] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 13; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 13);
    
    return (read_error());
}

int AIServoBus_ROBOTIS_XL320::moving(unsigned char id)
{
    instMsg[4] = id;
	instMsg[5] = 7;
	instMsg[6] = 0;
	instMsg[7] = XL_READ_DATA;
	instMsg[8] = XL_MOVING;
	instMsg[9] = 0;
	instMsg[10] = XL_BYTE_READ;
	instMsg[11] = 0;
	CRC = update_crc(0, instMsg, 12);
	instMsg[12] = CRC & 0xFF;
	instMsg[13] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 14; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 14);
	
    Moving_Byte = -1;
    Time_Counter = 0;
    while((uart->available() < 12) & (Time_Counter < TIME_OUT))
	{
		Time_Counter++;
		delayMicroseconds(1000);
    }
	
	while (uart->available() > 0)
	{
		recvMsg[0] = uart->read();
		if ( (recvMsg[0] == 255) & (uart->peek() == 255) )
		{
			recvMsg[1] = uart->read();               // 0xFF
			recvMsg[2] = uart->read();               // 0xFD
			recvMsg[3] = uart->read();               // 0x00
			recvMsg[4] = uart->read();               // XL-320 ID
			recvMsg[5] = uart->read();               // Length_L
			recvMsg[6] = uart->read();               // Length_H
			recvMsg[7] = uart->read();               // 0x55
			recvMsg[8] = uart->read();               // Error
			recvMsg[9] = uart->read();               // Moving_Byte
			recvMsg[10] = uart->read();              // CRC_L
			recvMsg[11] = uart->read();              // CRC_H
			CRC = update_crc(0, recvMsg, 10);
			if((recvMsg[10] != (CRC&0xFF)) || (recvMsg[11] != ((CRC >> 8)&0xFF)))
				return -2;
			if (recvMsg[8] != 0)
				return recvMsg[8];
			Moving_Byte = recvMsg[9];
		}
    }
	return (Moving_Byte);                             // Returns the read Moving_Byte
}

int AIServoBus_ROBOTIS_XL320::RWStatus(unsigned char id)
{
    instMsg[4] = id;
	instMsg[5] = 7;
	instMsg[6] = 0;
	instMsg[7] = XL_READ_DATA;
	instMsg[8] = XL_REGISTERED_INSTRUCTION;
	instMsg[9] = 0;
	instMsg[10] = XL_BYTE_READ;
	instMsg[11] = 0;
	CRC = update_crc(0, instMsg, 12);
	instMsg[12] = CRC & 0xFF;
	instMsg[13] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 14; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 14);
	
    RWS_Byte = -1;
    Time_Counter = 0;
    while((uart->available() < 12) & (Time_Counter < TIME_OUT))
	{
		Time_Counter++;
		delayMicroseconds(1000);
    }
	
	while (uart->available() > 0)
	{
		recvMsg[0] = uart->read();
		if ( (recvMsg[0] == 255) & (uart->peek() == 255) )
		{
			recvMsg[1] = uart->read();               // 0xFF
			recvMsg[2] = uart->read();               // 0xFD
			recvMsg[3] = uart->read();               // 0x00
			recvMsg[4] = uart->read();               // XL-320 ID
			recvMsg[5] = uart->read();               // Length_L
			recvMsg[6] = uart->read();               // Length_H
			recvMsg[7] = uart->read();               // 0x55
			recvMsg[8] = uart->read();               // Error
			recvMsg[9] = uart->read();               // RWS_Byte
			recvMsg[10] = uart->read();              // CRC_L
			recvMsg[11] = uart->read();              // CRC_H
			CRC = update_crc(0, recvMsg, 10);
			if((recvMsg[10] != (CRC&0xFF)) || (recvMsg[11] != ((CRC >> 8)&0xFF)))
				return -2;
			if (recvMsg[8] != 0)
				return recvMsg[8];
			RWS_Byte = recvMsg[9];
		}
    }
	return (RWS_Byte);                               // Returns the read RWS_Byte
}

int AIServoBus_ROBOTIS_XL320::readRDT(unsigned char id)
{	
    instMsg[4] = id;
	instMsg[5] = 7;
	instMsg[6] = 0;
	instMsg[7] = XL_READ_DATA;
	instMsg[8] = XL_RETURN_DELAY_TIME;
	instMsg[9] = 0;
	instMsg[10] = XL_BYTE_READ;
	instMsg[11] = 0;
	CRC = update_crc(0, instMsg, 12);
	instMsg[12] = CRC & 0xFF;
	instMsg[13] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 14; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 14);
	
    RWS_Byte = -1;
    Time_Counter = 0;
    while((uart->available() < 12) & (Time_Counter < TIME_OUT))
	{
		Time_Counter++;
		delayMicroseconds(1000);
    }
	
    while (uart->available() > 0)
	{
		recvMsg[0] = uart->read();
		if ( (recvMsg[0] == 255) & (uart->peek() == 255) )
		{
			recvMsg[1] = uart->read();               // 0xFF
			recvMsg[2] = uart->read();               // 0xFD
			recvMsg[3] = uart->read();               // 0x00
			recvMsg[4] = uart->read();               // XL-320 ID
			recvMsg[5] = uart->read();               // Length_L
			recvMsg[6] = uart->read();               // Length_H
			recvMsg[7] = uart->read();               // 0x55
			recvMsg[8] = uart->read();               // Error
			recvMsg[9] = uart->read();               // RWS_Byte
			recvMsg[10] = uart->read();              // CRC_L
			recvMsg[11] = uart->read();              // CRC_H
			CRC = update_crc(0, recvMsg, 10);
			if((recvMsg[10] != (CRC&0xFF)) || (recvMsg[11] != ((CRC >> 8)&0xFF)))
				return -2;
			if (recvMsg[8] != 0)
				return recvMsg[8];
			RWS_Byte = recvMsg[9];
		}
    }
	return (RWS_Byte);                                // Returns the read RWS_Byte
}

int AIServoBus_ROBOTIS_XL320::readHardwareError(unsigned char id)
{
	instMsg[4] = id;
	instMsg[5] = 7;
	instMsg[6] = 0;
	instMsg[7] = XL_READ_DATA;
	instMsg[8] = XL_HARDWARE_ERROR_STATUS;
	instMsg[9] = 0;
	instMsg[10] = XL_BYTE_READ;
	instMsg[11] = 0;
	CRC = update_crc(0, instMsg, 12);
	instMsg[12] = CRC & 0xFF;
	instMsg[13] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 14; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 14);
	
    RWS_Byte = -1;
    Time_Counter = 0;
    while((uart->available() < 12) & (Time_Counter < TIME_OUT))
	{
		Time_Counter++;
		delayMicroseconds(1000);
    }
	
    while (uart->available() > 0)
	{
		recvMsg[0] = uart->read();
		if ( (recvMsg[0] == 255) & (uart->peek() == 255) )
		{
			recvMsg[1] = uart->read();               // 0xFF
			recvMsg[2] = uart->read();               // 0xFD
			recvMsg[3] = uart->read();               // 0x00
			recvMsg[4] = uart->read();               // XL-320 ID
			recvMsg[5] = uart->read();               // Length_L
			recvMsg[6] = uart->read();               // Length_H
			recvMsg[7] = uart->read();               // 0x55
			recvMsg[8] = uart->read();               // Error
			recvMsg[9] = uart->read();               // RWS_Byte
			recvMsg[10] = uart->read();              // CRC_L
			recvMsg[11] = uart->read();              // CRC_H
			CRC = update_crc(0, recvMsg, 10);
			if((recvMsg[10] != (CRC&0xFF)) || (recvMsg[11] != ((CRC >> 8)&0xFF)))
				return -2;
			if (recvMsg[8] != 0)
				return recvMsg[8];
			RWS_Byte = recvMsg[9];
		}
    }
	return (RWS_Byte);                                // Returns the read RWS_Byte
}

int AIServoBus_ROBOTIS_XL320::readSpeed(unsigned char id)
{
	instMsg[4] = id;
	instMsg[5] = 7;
	instMsg[6] = 0;
	instMsg[7] = XL_READ_DATA;
	instMsg[8] = XL_PRESENT_SPEED_L;
	instMsg[9] = 0;
	instMsg[10] = XL_BYTE_READ_POS;
	instMsg[11] = 0;
	CRC = update_crc(0, instMsg, 12);
	instMsg[12] = CRC & 0xFF;
	instMsg[13] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 14; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 14);
	
    Speed_Long_Byte = -1;
    Time_Counter = 0;
    while((uart->available() < 13) & (Time_Counter < TIME_OUT))
	{
		Time_Counter++;
		delayMicroseconds(1000);
    }
	
	while (uart->available() > 0)
	{
		recvMsg[0] = uart->read();
		if ( (recvMsg[0] == 255) & (uart->peek() == 255) )
		{
			recvMsg[1] = uart->read();               // 0xFF
			recvMsg[2] = uart->read();               // 0xFD
			recvMsg[3] = uart->read();               // 0x00
			recvMsg[4] = uart->read();               // XL-320 ID
			recvMsg[5] = uart->read();               // Length_L
			recvMsg[6] = uart->read();               // Length_H
			recvMsg[7] = uart->read();               // 0x55
			recvMsg[8] = uart->read();               // Error
			recvMsg[9] = uart->read();               // Speed_L
			recvMsg[10] = uart->read();              // Speed_H
			recvMsg[11] = uart->read();              // CRC_L
			recvMsg[12] = uart->read();              // CRC_H
			CRC = update_crc(0, recvMsg, 11);
			if((recvMsg[11] != (CRC&0xFF)) || (recvMsg[12] != ((CRC >> 8)&0xFF)))
				return -2;
			if (recvMsg[8] != 0)
				return recvMsg[8];
			Speed_Long_Byte = (recvMsg[10] << 8) + recvMsg[9];
		}
    }
	return (Speed_Long_Byte);                        // Returns the read Speed
}

int AIServoBus_ROBOTIS_XL320::readLoad(unsigned char id)
{
    instMsg[4] = id;
	instMsg[5] = 7;
	instMsg[6] = 0;
	instMsg[7] = XL_READ_DATA;
	instMsg[8] = XL_PRESENT_LOAD_L;
	instMsg[9] = 0;
	instMsg[10] = XL_BYTE_READ_POS;
	instMsg[11] = 0;
	CRC = update_crc(0, instMsg, 12);
	instMsg[12] = CRC & 0xFF;
	instMsg[13] = (CRC>>8) & 0xFF;
	
	for(int i = 0; i < 14; i++)
		uart->write(instMsg[i]);
	
	delayMicroseconds(TX_DELAY * 14);
	
    Load_Long_Byte = -1;
    Time_Counter = 0;
    while((uart->available() < 13) & (Time_Counter < TIME_OUT))
	{
		Time_Counter++;
		delayMicroseconds(1000);
    }
	
	while (uart->available() > 0)
	{
		recvMsg[0] = uart->read();
		if ( (recvMsg[0] == 255) & (uart->peek() == 255) )
		{
			recvMsg[1] = uart->read();               // 0xFF
			recvMsg[2] = uart->read();               // 0xFD
			recvMsg[3] = uart->read();               // 0x00
			recvMsg[4] = uart->read();               // XL-320 ID
			recvMsg[5] = uart->read();               // Length_L
			recvMsg[6] = uart->read();               // Length_H
			recvMsg[7] = uart->read();               // 0x55
			recvMsg[8] = uart->read();               // Error
			recvMsg[9] = uart->read();               // Load_L
			recvMsg[10] = uart->read();              // Load_H
			recvMsg[11] = uart->read();              // CRC_L
			recvMsg[12] = uart->read();              // CRC_H
			CRC = update_crc(0, recvMsg, 11);
			if((recvMsg[11] != (CRC&0xFF)) || (recvMsg[12] != ((CRC >> 8)&0xFF)))
				return -2;
			if (recvMsg[8] != 0)
				return recvMsg[8];
			Load_Long_Byte = (recvMsg[10] << 8) + recvMsg[9];
		}
    }
	return (Load_Long_Byte);        // Returns the read Load
}
