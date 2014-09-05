/*
  UTouch.cpp - Arduino/chipKit library support for Color TFT LCD Touch screens 
  Copyright (C)2010-2014 Henning Karlsen. All right reserved
  
  Modified 17 July 2014 by Oswald Kuo for 86Duino
  Modified 1 September 2014 by Android Lin for 86Duino
  
  Basic functionality of this library are based on the demo-code provided by
  ITead studio. You can find the latest version of the library at
  http://www.henningkarlsen.com/electronics

  If you make any modifications or improvements to the code, I would appreciate
  that you share the code with me so that I might include it in the next release.
  I can be contacted through http://www.henningkarlsen.com/electronics/contact.php

  This library is free software; you can redistribute it and/or
  modify it under the terms of the CC BY-NC-SA 3.0 license.
  Please see the included documents for further information.
*/

#include "UTouch.h"
#include "UTouchCD.h"

static unsigned int utouch_pin_p_v[2][5];

UTouch::UTouch(byte tclk, byte tcs, byte din, byte dout, byte irq) {
	T_CLK	= tclk;
	T_CS	= tcs;
	T_DIN	= din;
	T_DOUT	= dout;
	T_IRQ	= irq;
  
	unsigned int port;
	unsigned int value;
	int crossbar_bit;
  
	crossbar_bit = pinMap[tclk];
	port = GPIODATABASE + 4*(crossbar_bit/8);
	value = 1<<(crossbar_bit%8);
	utouch_pin_p_v[0][0] = port;
	utouch_pin_p_v[1][0] = value;
	
	crossbar_bit = pinMap[tcs];
	port = GPIODATABASE + 4*(crossbar_bit/8);
	value = 1<<(crossbar_bit%8);
	utouch_pin_p_v[0][1] = port;
	utouch_pin_p_v[1][1] = value;
	
	crossbar_bit = pinMap[din];
	port = GPIODATABASE + 4*(crossbar_bit/8);
	value = 1<<(crossbar_bit%8);
	utouch_pin_p_v[0][2] = port;
	utouch_pin_p_v[1][2] = value;
	
	crossbar_bit = pinMap[dout];
	port = GPIODATABASE + 4*(crossbar_bit/8);
	value = 1<<(crossbar_bit%8);
	utouch_pin_p_v[0][3] = port;
	utouch_pin_p_v[1][3] = value;
	
	crossbar_bit = pinMap[irq];
	port = GPIODATABASE + 4*(crossbar_bit/8);
	value = 1<<(crossbar_bit%8);
	utouch_pin_p_v[0][4] = port;
	utouch_pin_p_v[1][4] = value;
}

void UTouch::InitTouch(byte orientation) {
	orient					= orientation;
	_default_orientation	= CAL_S>>31;
	touch_x_left			= (CAL_X>>14) & 0x3FFF;
	touch_x_right			= CAL_X & 0x3FFF;
	touch_y_top				= (CAL_Y>>14) & 0x3FFF;
	touch_y_bottom			= CAL_Y & 0x3FFF;
	disp_x_size				= (CAL_S>>12) & 0x0FFF;
	disp_y_size				= CAL_S & 0x0FFF;
	prec					= 10;

	pinMode(T_CLK,  OUTPUT);
	pinMode(T_CS,   OUTPUT);
	pinMode(T_DIN,  OUTPUT);
	pinMode(T_DOUT,  INPUT);
	pinMode(T_IRQ,  OUTPUT);

    To_GPIO(T_CLK);
    To_GPIO(T_CS);
    To_GPIO(T_DIN);
    To_GPIO(T_DOUT);
    To_GPIO(T_IRQ);
    
    digitalWrite_f(T_CS,  HIGH);
	digitalWrite_f(T_CLK, HIGH);
	digitalWrite_f(T_DIN, HIGH);
	digitalWrite_f(T_IRQ, HIGH);
}

void UTouch::read() {
	unsigned long tx = 0, temp_x = 0;
	unsigned long ty = 0, temp_y = 0;
	unsigned long minx = 99999, maxx = 0;
	unsigned long miny = 99999, maxy = 0;
	int datacount = 0;

	digitalWrite_f(T_CS, LOW);                    

	pinMode(T_IRQ, INPUT);
	for (int i=0; i<prec; i++)
	{
		if (!digitalRead(T_IRQ))
		{
			touch_WriteData(0x90);         
			digitalWrite_f(T_CLK, HIGH);
			digitalWrite_f(T_CLK, LOW); 
			temp_x=touch_ReadData();

			if (!digitalRead(T_IRQ))
			{
				touch_WriteData(0xD0);      
				digitalWrite_f(T_CLK, HIGH);
				digitalWrite_f(T_CLK, LOW); 
				temp_y = touch_ReadData();

				if ((temp_x>0) and (temp_x<4096) and (temp_y>0) and (temp_y<4096))
				{
					tx += temp_x;
					ty += temp_y;
					if (prec>5)
					{
						if (temp_x<minx)
							minx = temp_x;
						if (temp_x>maxx)
							maxx = temp_x;
						if (temp_y<miny)
							miny = temp_y;
						if (temp_y>maxy)
							maxy = temp_y;
					}
					datacount++;
				}
			}
		}
	}
	pinMode(T_IRQ, OUTPUT);

	if (prec > 5)
	{
		tx = tx - (minx + maxx);
		ty = ty - (miny + maxy);
		datacount -= 2;
	}

	digitalWrite_f(T_CS, HIGH);                    
	if ((datacount == (prec-2)) or (datacount == PREC_LOW))
	{
		if (orient == _default_orientation)
		{
			TP_X = ty / datacount;
			TP_Y = tx / datacount;
		}
		else
		{
			TP_X = tx / datacount;
			TP_Y = ty / datacount;
		}
	}
	else
	{
		TP_X = -1;
		TP_Y = -1;
	}
}

bool UTouch::dataAvailable(){
	bool avail;
	pinMode(T_IRQ, INPUT);
	avail = !(digitalRead(T_IRQ));
	pinMode(T_IRQ, OUTPUT);
	return avail;
}

int16_t UTouch::getX(){
	long c;

	if ((TP_X == -1) or (TP_Y == -1))
		return -1;
	if (orient == _default_orientation)
	{
		c = long(long(TP_X - touch_x_left) * (disp_x_size)) / long(touch_x_right - touch_x_left);
		if (c < 0)
			c = 0;
		if (c > disp_x_size)
			c = disp_x_size;
	}
	else
	{
		if (_default_orientation == PORTRAIT)
			c = long(long(TP_X - touch_y_top) * (-disp_y_size)) / long(touch_y_bottom - touch_y_top) + long(disp_y_size);
		else
			c = long(long(TP_X - touch_y_top) * (disp_y_size)) / long(touch_y_bottom - touch_y_top);
		if (c < 0)
			c = 0;
		if (c > disp_y_size)
			c = disp_y_size;
	}
	return c;
}

int16_t UTouch::getY()
{
	int c;

	if ((TP_X == -1) or (TP_Y == -1))
		return -1;
	if (orient == _default_orientation)
	{
		c = long(long(TP_Y - touch_y_top) * (disp_y_size)) / long(touch_y_bottom - touch_y_top);
		if (c < 0)
			c = 0;
		if (c > disp_y_size)
			c = disp_y_size;
	}
	else
	{
		if (_default_orientation == PORTRAIT)
			c = long(long(TP_Y - touch_x_left) * (disp_x_size)) / long(touch_x_right - touch_x_left);
		else
			c = long(long(TP_Y - touch_x_left) * (-disp_x_size)) / long(touch_x_right - touch_x_left) + long(disp_x_size);
		if (c < 0)
			c = 0;
		if (c > disp_x_size)
			c = disp_x_size;
	}
	return c;
}

void UTouch::setPrecision(byte precision)
{
	switch (precision)
	{
		case PREC_LOW:
			prec = 1;		// DO NOT CHANGE!
			break;
		case PREC_MEDIUM:
			prec = 12;	// Iterations + 2
			break;
		case PREC_HI:
			prec = 27;	// Iterations + 2
			break;
		case PREC_EXTREME:
			prec = 102;	// Iterations + 2
			break;
		default:
			prec = 12;	// Iterations + 2
			break;
	}
}

void UTouch::calibrateRead()
{
	unsigned long tx = 0;
	unsigned long ty = 0;

	digitalWrite_f(T_CS, LOW);                    

	touch_WriteData(0x90);        
	digitalWrite_f(T_CLK, HIGH);
	digitalWrite_f(T_CLK, LOW); 
	tx = touch_ReadData();

	touch_WriteData(0xD0);      
	digitalWrite_f(T_CLK, HIGH);
	digitalWrite_f(T_CLK, LOW); 
	ty = touch_ReadData();

	digitalWrite_f(T_CS, HIGH);                    

	TP_X = ty;
	TP_Y = tx;
}
void UTouch::touch_WriteData(byte data)
{
	byte temp;

	temp = data;
	digitalWrite_f(T_CLK, LOW);

	for(byte count=0; count<8; count++)
	{
		if(temp & 0x80)
			digitalWrite_f(T_DIN, HIGH);
		else
			digitalWrite_f(T_DIN, LOW);
		temp = temp << 1; 
		digitalWrite_f(T_CLK, LOW);
		digitalWrite_f(T_CLK, HIGH);
	}
}
word UTouch::touch_ReadData()
{
	word data = 0;

	for(byte count=0; count<12; count++)
	{
		data <<= 1;
		digitalWrite_f(T_CLK, HIGH);
		digitalWrite_f(T_CLK, LOW);
		if (digitalRead(T_DOUT))
			data++;
	}
	return(data);
}
void UTouch::digitalWrite_f(uint8_t pin, uint8_t val){
	int i = 0;
	if(pin == T_CLK)
		i=0;
	else if(pin == T_CS)
		i=1;
	else if(pin == T_DIN)
		i=2;
	else if(pin == T_DOUT)
		i=3;
	else if(pin == T_IRQ)
		i=4;          
	io_DisableINT();
	if (val == LOW) 
	    io_outpb(utouch_pin_p_v[0][i], io_inpb(utouch_pin_p_v[0][i]) & (~utouch_pin_p_v[1][i]));
	else
	    io_outpb(utouch_pin_p_v[0][i], io_inpb(utouch_pin_p_v[0][i]) | utouch_pin_p_v[1][i]);
	io_RestoreINT();
}

void UTouch::To_GPIO(uint8_t pin) {
	unsigned int port;
	unsigned int value;
	int crossbar_bit;
	unsigned short crossbar_ioaddr = 0;
	crossbar_ioaddr = sb_Read16(SB_CROSSBASE) & 0xfffe;
	crossbar_bit = pinMap[pin];
	port = GPIODATABASE + 4*(crossbar_bit/8);
  
	if(crossbar_bit > 31)
		io_outpb(crossbar_ioaddr + 0x80 + (crossbar_bit/8), 0x01);
	else if(crossbar_bit <= 31)
	{
		io_outpb(crossbar_ioaddr + 0x90 + crossbar_bit, 0x01);
		Close_Pwm(pin);
	}
}

