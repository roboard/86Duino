/*
  UTFT.cpp - Arduino/chipKit library support for Color TFT LCD Boards
  Copyright (C)2010-2014 Henning Karlsen. All right reserved
  
  Modified 17 July 2014 by Oswald Kuo for 86Duino
  Modified 1 September 2014 by Android Lin for 86Duino
  
  This library is the continuation of my ITDB02_Graph, ITDB02_Graph16
  and RGB_GLCD libraries for Arduino and chipKit. As the number of 
  supported display modules and controllers started to increase I felt 
  it was time to make a single, universal library as it will be much 
  easier to maintain in the future.

  Basic functionality of this library was origianlly based on the 
  demo-code provided by ITead studio (for the ITDB02 modules) and 
  NKC Electronics (for the RGB GLCD module/shield).

  This library supports a number of 8bit, 16bit and serial graphic 
  displays, and will work with both Arduino and chipKit boards. For a 
  full list of tested display modules and controllers, see the 
  document UTFT_Supported_display_modules_&_controllers.pdf.

  When using 8bit and 16bit display modules there are some 
  requirements you must adhere to. These requirements can be found 
  in the document UTFT_Requirements.pdf.
  There are no special requirements when using serial displays.

  You can always find the latest version of the library at 
  http://electronics.henningkarlsen.com/

  If you make any modifications or improvements to the code, I would 
  appreciate that you share the code with me so that I might include 
  it in the next release. I can be contacted through 
  http://electronics.henningkarlsen.com/contact.php.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the CC BY-NC-SA 3.0 license.
  Please see the included documents for further information.

  Commercial use of this library requires you to buy a license that
  will allow commercial use. This includes using the library,
  modified or not, as a tool to sell products.

  The license applies to all part of the library including the 
  examples and tools supplied with the library.
*/

#include "UTFT.h"
#include "pins_arduino.h"
#include "io.h"

#if defined __86DUINO_ONE
	#include "hardware/HW_One.h"
#elif defined __86DUINO_EDUCAKE
    #include "hardware/HW_Cake.h"
#elif defined __86DUINO_ZERO
    #include "hardware/HW_Zero.h"
#else
    #error "Unsupported 86Duino board!"
#endif

static int UseVirtualPort = 1; // this is only available for 86Duino one
// Include hardware-specific functions for the correct MCU
static unsigned int pin_p_v[2][5];
UTFT::UTFT()
{

}

UTFT::UTFT(byte model, int RS, int WR, int CS, int RST, int SER)
{ 
	word	dsx[] = {239, 239, 239, 239, 239, 239, 175, 175, 239, 127, 127, 239, 271, 479, 239, 239, 239, 239, 239, 239, 479, 319, 239, 175, 127, 239, 239, 319, 319, 799, 127};
	word	dsy[] = {319, 399, 319, 319, 319, 319, 219, 219, 399, 159, 127, 319, 479, 799, 319, 319, 319, 319, 319, 319, 799, 479, 319, 219, 159, 319, 319, 479, 479, 479, 159};
	byte	dtm[] = {16, 16, 16, 8, 8, 16, 8, SERIAL_4PIN, 16, SERIAL_5PIN, SERIAL_5PIN, 16, 16, 16, 8, 16, LATCHED_16, 8, 16, 8, 16, 16, 16, 8, SERIAL_5PIN, SERIAL_5PIN, SERIAL_4PIN, 16, 16, 16, SERIAL_5PIN};

	disp_x_size =			dsx[model];
	disp_y_size =			dsy[model];
	display_transfer_mode =	dtm[model];
	display_model =			model;

	__p1 = RS;
	__p2 = WR;
	__p3 = CS;
	__p4 = RST;
	__p5 = SER;

	unsigned int port;
	unsigned int value;
	int crossbar_bit;
  
	crossbar_bit = pinMap[RS];
	port = GPIODATABASE + 4*(crossbar_bit/8);
	value = 1<<(crossbar_bit%8);
	pin_p_v[0][0] = port;
	pin_p_v[1][0] = value;
	
	crossbar_bit = pinMap[WR];
	port = GPIODATABASE + 4*(crossbar_bit/8);
	value = 1<<(crossbar_bit%8);
	pin_p_v[0][1] = port;
	pin_p_v[1][1] = value;
	
	crossbar_bit = pinMap[CS];
	port = GPIODATABASE + 4*(crossbar_bit/8);
	value = 1<<(crossbar_bit%8);
	pin_p_v[0][2] = port;
	pin_p_v[1][2] = value;
	
	crossbar_bit = pinMap[RST];
	port = GPIODATABASE + 4*(crossbar_bit/8);
	value = 1<<(crossbar_bit%8);
	pin_p_v[0][3] = port;
	pin_p_v[1][3] = value;
	
	crossbar_bit = pinMap[SER];
	port = GPIODATABASE + 4*(crossbar_bit/8);
	value = 1<<(crossbar_bit%8);
	pin_p_v[0][4] = port;
	pin_p_v[1][4] = value;

	if (display_transfer_mode == SERIAL_4PIN)
	{
		display_transfer_mode = 1;
		display_serial_mode = SERIAL_4PIN;
	}
	if (display_transfer_mode == SERIAL_5PIN)
	{
		display_transfer_mode = 1;
		display_serial_mode = SERIAL_5PIN;
	}

	if (display_transfer_mode != 1)
	{
		_set_direction_registers(display_transfer_mode);
		
		if (display_transfer_mode == LATCHED_16)
		{
			digitalWrite_f(__p5, LOW);
			pinMode(42, OUTPUT);
			digitalWrite(42, LOW);
		}
	}

}

void UTFT::LCD_Write_COM(char VL)  
{   
	if (display_transfer_mode != 1)
	{
		digitalWrite_f(__p1, LOW);
		LCD_Writ_Bus(0x00, VL, display_transfer_mode);
	}
	else
		LCD_Writ_Bus(0x00, VL, display_transfer_mode);
}

void UTFT::LCD_Write_DATA(char VH,char VL)
{
	if (display_transfer_mode != 1)
	{
		digitalWrite_f(__p1, HIGH);
		LCD_Writ_Bus(VH, VL, display_transfer_mode);
	}
	else
	{
		LCD_Writ_Bus(0x01, VH, display_transfer_mode);
		LCD_Writ_Bus(0x01, VL, display_transfer_mode);
	}
}

void UTFT::LCD_Write_DATA(char VL)
{
	if (display_transfer_mode != 1)
	{
		digitalWrite_f(__p1, HIGH);
		LCD_Writ_Bus(0x00, VL, display_transfer_mode);
	}
	else
		LCD_Writ_Bus(0x01, VL, display_transfer_mode);
}

void UTFT::LCD_Write_COM_DATA(char com1,int dat1)
{
     LCD_Write_COM(com1);
     LCD_Write_DATA(dat1>>8, dat1);
}

void UTFT::InitLCD(byte orientation)
{
	orient = orientation;
	_hw_special_init();

	pinMode(__p1,OUTPUT);
	pinMode(__p2,OUTPUT);
	pinMode(__p3,OUTPUT);
	pinMode(__p4,OUTPUT);
	
	To_GPIO(__p1);
	To_GPIO(__p2);
	To_GPIO(__p3);
	To_GPIO(__p4);
	
	if ((display_transfer_mode == LATCHED_16) or ((display_transfer_mode == 1) and (display_serial_mode == SERIAL_5PIN)))
	{
		pinMode(__p5,OUTPUT);
    	To_GPIO(__p5);
	}
	
	if (display_transfer_mode != 1)
		_set_direction_registers(display_transfer_mode);

	digitalWrite_f(__p4, HIGH);
	delay(5); 
	digitalWrite_f(__p4, LOW);
	delay(15);
	digitalWrite_f(__p4, HIGH);
	delay(15);

	digitalWrite_f(__p3, LOW);

	switch(display_model)
	{
#ifndef DISABLE_HX8347A
	#include "tft_drivers/hx8347a/initlcd.h"
#endif
#ifndef DISABLE_ILI9327
	#include "tft_drivers/ili9327/initlcd.h"
#endif
#ifndef DISABLE_SSD1289
	#include "tft_drivers/ssd1289/initlcd.h"
#endif
#ifndef DISABLE_ILI9325C
	#include "tft_drivers/ili9325c/initlcd.h"
#endif
#ifndef DISABLE_ILI9325D
	#include "tft_drivers/ili9325d/default/initlcd.h"
#endif
#ifndef DISABLE_ILI9325D_ALT
	#include "tft_drivers/ili9325d/alt/initlcd.h"
#endif
#ifndef DISABLE_HX8340B_8
	#include "tft_drivers/hx8340b/8/initlcd.h"
#endif
#ifndef DISABLE_HX8340B_S
	#include "tft_drivers/hx8340b/s/initlcd.h"
#endif
#ifndef DISABLE_ST7735
	#include "tft_drivers/st7735/initlcd.h"
#endif
#ifndef DISABLE_PCF8833
	#include "tft_drivers/pcf8833/initlcd.h"
#endif
#ifndef DISABLE_S1D19122
	#include "tft_drivers/s1d19122/initlcd.h"
#endif
#ifndef DISABLE_HX8352A
	#include "tft_drivers/hx8352a/initlcd.h"
#endif
#ifndef DISABLE_SSD1963_480
	#include "tft_drivers/ssd1963/480/initlcd.h"
#endif
#ifndef DISABLE_SSD1963_800
	#include "tft_drivers/ssd1963/800/initlcd.h"
#endif
#ifndef DISABLE_SSD1963_800_ALT
	#include "tft_drivers/ssd1963/800alt/initlcd.h"
#endif
#ifndef DISABLE_S6D1121
	#include "tft_drivers/s6d1121/initlcd.h"
#endif
#ifndef DISABLE_ILI9320
	#include "tft_drivers/ili9320/initlcd.h"
#endif
#ifndef DISABLE_ILI9481
	#include "tft_drivers/ili9481/initlcd.h"
#endif
#ifndef DISABLE_S6D0164
	#include "tft_drivers/s6d0164/initlcd.h"
#endif
#ifndef DISABLE_ST7735S
	#include "tft_drivers/st7735s/initlcd.h"
#endif
#ifndef DISABLE_ILI9341_S4P
	#include "tft_drivers/ili9341/s4p/initlcd.h"
#endif
#ifndef DISABLE_ILI9341_S5P
	#include "tft_drivers/ili9341/s5p/initlcd.h"
#endif
#ifndef DISABLE_R61581
	#include "tft_drivers/r61581/initlcd.h"
#endif
#ifndef DISABLE_ILI9486
	#include "tft_drivers/ili9486/initlcd.h"
#endif
#ifndef DISABLE_CPLD
	#include "tft_drivers/cpld/initlcd.h"
#endif
	}

	digitalWrite_f(__p3, HIGH); 

	setColor(255, 255, 255);
	setBackColor(0, 0, 0);
	cfont.font = 0;
	_transparent = false;

}

void UTFT::setXY(word x1, word y1, word x2, word y2)
{
	if (orient == LANDSCAPE)
	{
		swap(word, x1, y1);
		swap(word, x2, y2)
		y1 = disp_y_size-y1;
		y2 = disp_y_size-y2;
		swap(word, y1, y2)
	}

	switch(display_model)
	{
#ifndef DISABLE_HX8347A
	#include "tft_drivers/hx8347a/setxy.h"
#endif
#ifndef DISABLE_HX8352A
	#include "tft_drivers/hx8352a/setxy.h"
#endif
#ifndef DISABLE_ILI9327
	#include "tft_drivers/ili9327/setxy.h"
#endif
#ifndef DISABLE_SSD1289
	#include "tft_drivers/ssd1289/setxy.h"
#endif
#ifndef DISABLE_ILI9325C
	#include "tft_drivers/ili9325c/setxy.h"
#endif
#ifndef DISABLE_ILI9325D
	#include "tft_drivers/ili9325d/default/setxy.h"
#endif
#ifndef DISABLE_ILI9325D_ALT
	#include "tft_drivers/ili9325d/alt/setxy.h"
#endif
#ifndef DISABLE_HX8340B_8
	#include "tft_drivers/hx8340b/8/setxy.h"
#endif
#ifndef DISABLE_HX8340B_S
	#include "tft_drivers/hx8340b/s/setxy.h"
#endif
#ifndef DISABLE_ST7735
	#include "tft_drivers/st7735/setxy.h"
#endif
#ifndef DISABLE_S1D19122
	#include "tft_drivers/s1d19122/setxy.h"
#endif
#ifndef DISABLE_PCF8833
	#include "tft_drivers/pcf8833/setxy.h"
#endif
#ifndef DISABLE_SSD1963_480
	#include "tft_drivers/ssd1963/480/setxy.h"
#endif
#ifndef DISABLE_SSD1963_800
	#include "tft_drivers/ssd1963/800/setxy.h"
#endif
#ifndef DISABLE_SSD1963_800_ALT
	#include "tft_drivers/ssd1963/800alt/setxy.h"
#endif
#ifndef DISABLE_S6D1121
	#include "tft_drivers/s6d1121/setxy.h"
#endif
#ifndef DISABLE_ILI9320
	#include "tft_drivers/ili9320/setxy.h"
#endif
#ifndef DISABLE_ILI9481
	#include "tft_drivers/ili9481/setxy.h"
#endif
#ifndef DISABLE_S6D0164
	#include "tft_drivers/s6d0164/setxy.h"
#endif
#ifndef DISABLE_ST7735S
	#include "tft_drivers/st7735s/setxy.h"
#endif
#ifndef DISABLE_ILI9341_S4P
	#include "tft_drivers/ili9341/s4p/setxy.h"
#endif
#ifndef DISABLE_ILI9341_S5P
	#include "tft_drivers/ili9341/s5p/setxy.h"
#endif
#ifndef DISABLE_R61581
	#include "tft_drivers/r61581/setxy.h"
#endif
#ifndef DISABLE_ILI9486
	#include "tft_drivers/ili9486/setxy.h"
#endif
#ifndef DISABLE_CPLD
	#include "tft_drivers/cpld/setxy.h"
#endif
	}
}

void UTFT::clrXY()
{
	if (orient == PORTRAIT)
		setXY(0, 0, disp_x_size, disp_y_size);
	else
		setXY(0, 0, disp_y_size, disp_x_size);
}

void UTFT::drawRect(int x1, int y1, int x2, int y2)
{
	if (x1>x2)
	{
		swap(int, x1, x2);
	}
	if (y1>y2)
	{
		swap(int, y1, y2);
	}

	drawHLine(x1, y1, x2-x1);
	drawHLine(x1, y2, x2-x1);
	drawVLine(x1, y1, y2-y1);
	drawVLine(x2, y1, y2-y1);
}

void UTFT::drawRoundRect(int x1, int y1, int x2, int y2)
{
	if (x1>x2)
	{
		swap(int, x1, x2);
	}
	if (y1>y2)
	{
		swap(int, y1, y2);
	}
	if ((x2-x1)>4 && (y2-y1)>4)
	{
		drawPixel(x1+1, y1+1);
		drawPixel(x2-1, y1+1);
		drawPixel(x1+1, y2-1);
		drawPixel(x2-1, y2-1);
		drawHLine(x1+2, y1, x2-x1-4);
		drawHLine(x1+2, y2, x2-x1-4);
		drawVLine(x1, y1+2, y2-y1-4);
		drawVLine(x2, y1+2, y2-y1-4);
	}
}

void UTFT::fillRect(int x1, int y1, int x2, int y2)
{
	if (x1>x2)
	{
		swap(int, x1, x2);
	}
	if (y1>y2)
	{
		swap(int, y1, y2);
	}
	if (display_transfer_mode == 16)
	{
		digitalWrite_f(__p3, LOW);
		setXY(x1, y1, x2, y2);
		digitalWrite_f(__p1, HIGH);
		_fast_fill_16(fch, fcl, ((long(x2-x1)+1)*(long(y2-y1)+1)));
		digitalWrite_f(__p3, HIGH);
	}
	else if ((display_transfer_mode == 8) and (fch == fcl))
	{
		digitalWrite_f(__p3, LOW);
		setXY(x1, y1, x2, y2);
		digitalWrite_f(__p1, HIGH);
		_fast_fill_8(fch, ((long(x2-x1)+1)*(long(y2-y1)+1)));
		digitalWrite_f(__p3, HIGH);
	}
	else
	{
		if (orient == PORTRAIT)
		{
			for (int i=0; i<((y2-y1)/2)+1; i++)
			{
				drawHLine(x1, y1+i, x2-x1);
				drawHLine(x1, y2-i, x2-x1);
			}
		}
		else
		{
			for (int i=0; i<((x2-x1)/2)+1; i++)
			{
				drawVLine(x1+i, y1, y2-y1);
				drawVLine(x2-i, y1, y2-y1);
			}
		}
	}
}

void UTFT::fillRoundRect(int x1, int y1, int x2, int y2)
{
	if (x1>x2)
	{
		swap(int, x1, x2);
	}
	if (y1>y2)
	{
		swap(int, y1, y2);
	}

	if ((x2-x1)>4 && (y2-y1)>4)
	{
		for (int i=0; i<((y2-y1)/2)+1; i++)
		{
			switch(i)
			{
			case 0:
				drawHLine(x1+2, y1+i, x2-x1-4);
				drawHLine(x1+2, y2-i, x2-x1-4);
				break;
			case 1:
				drawHLine(x1+1, y1+i, x2-x1-2);
				drawHLine(x1+1, y2-i, x2-x1-2);
				break;
			default:
				drawHLine(x1, y1+i, x2-x1);
				drawHLine(x1, y2-i, x2-x1);
			}
		}
	}
}

void UTFT::drawCircle(int x, int y, int radius)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x1 = 0;
	int y1 = radius;
 
	digitalWrite_f(__p3, LOW);
	setXY(x, y + radius, x, y + radius);
	LCD_Write_DATA(fch, fcl);
	setXY(x, y - radius, x, y - radius);
	LCD_Write_DATA(fch, fcl);
	setXY(x + radius, y, x + radius, y);
	LCD_Write_DATA(fch, fcl);
	setXY(x - radius, y, x - radius, y);
	LCD_Write_DATA(fch, fcl);
 
	while(x1 < y1)
	{
		if(f >= 0) 
		{
			y1--;
			ddF_y += 2;
			f += ddF_y;
		}
		x1++;
		ddF_x += 2;
		f += ddF_x;    
		setXY(x + x1, y + y1, x + x1, y + y1);
		LCD_Write_DATA(fch, fcl);
		setXY(x - x1, y + y1, x - x1, y + y1);
		LCD_Write_DATA(fch, fcl);
		setXY(x + x1, y - y1, x + x1, y - y1);
		LCD_Write_DATA(fch, fcl);
		setXY(x - x1, y - y1, x - x1, y - y1);
		LCD_Write_DATA(fch, fcl);
		setXY(x + y1, y + x1, x + y1, y + x1);
		LCD_Write_DATA(fch, fcl);
		setXY(x - y1, y + x1, x - y1, y + x1);
		LCD_Write_DATA(fch, fcl);
		setXY(x + y1, y - x1, x + y1, y - x1);
		LCD_Write_DATA(fch, fcl);
		setXY(x - y1, y - x1, x - y1, y - x1);
		LCD_Write_DATA(fch, fcl);
	}
	digitalWrite_f(__p3, HIGH);
	clrXY();
}

void UTFT::fillCircle(int x, int y, int radius)
{
	for(int y1=-radius; y1<=0; y1++) 
		for(int x1=-radius; x1<=0; x1++)
			if(x1*x1+y1*y1 <= radius*radius) 
			{
				drawHLine(x+x1, y+y1, 2*(-x1));
				drawHLine(x+x1, y-y1, 2*(-x1));
				break;
			}
}

void UTFT::clrScr()
{
	long i;
	
	digitalWrite_f(__p3, LOW);
	clrXY();
	if (display_transfer_mode != 1)
		digitalWrite_f(__p1, HIGH);
	if (display_transfer_mode == 16)
		_fast_fill_16(0, 0, ((disp_x_size+1)*(disp_y_size+1)));
	else if (display_transfer_mode == 8)
		_fast_fill_8(0, ((disp_x_size+1)*(disp_y_size+1)));
	else
	{
		for (i=0; i<((disp_x_size+1)*(disp_y_size+1)); i++)
		{
			if (display_transfer_mode != 1)
				LCD_Writ_Bus(0, 0, display_transfer_mode);
			else
			{
				LCD_Writ_Bus(1, 0, display_transfer_mode);
				LCD_Writ_Bus(1, 0, display_transfer_mode);
			}
		}
	}
	digitalWrite_f(__p3, HIGH);
}

void UTFT::fillScr(byte r, byte g, byte b)
{
	word color = ((r&248)<<8 | (g&252)<<3 | (b&248)>>3);
	fillScr(color);
}

void UTFT::fillScr(word color)
{
	long i;
	char ch, cl;
	
	ch = byte(color>>8);
	cl = byte(color & 0xFF);

	digitalWrite_f(__p3, LOW);
	clrXY();
	if (display_transfer_mode != 1)
		digitalWrite_f(__p1, HIGH);
	if (display_transfer_mode == 16)
		_fast_fill_16(ch, cl, ((disp_x_size+1)*(disp_y_size+1)));
	else if ((display_transfer_mode == 8) and (ch == cl))
		_fast_fill_8(ch, ((disp_x_size+1)*(disp_y_size+1)));
	else
	{
		for (i=0; i<((disp_x_size+1)*(disp_y_size+1)); i++)
		{
			if (display_transfer_mode != 1)
				LCD_Writ_Bus(ch, cl, display_transfer_mode);
			else
			{
				LCD_Writ_Bus(1, ch, display_transfer_mode);
				LCD_Writ_Bus(1, cl, display_transfer_mode);
			}
		}
	}
	digitalWrite_f(__p3, HIGH);
}

void UTFT::setColor(byte r, byte g, byte b)
{
	fch = ((r&248)|g>>5);
	fcl = ((g&28)<<3|b>>3);
}

void UTFT::setColor(word color)
{
	fch = byte(color>>8);
	fcl = byte(color & 0xFF);
}

word UTFT::getColor()
{
	return (fch<<8) | fcl;
}

void UTFT::setBackColor(byte r, byte g, byte b)
{
	bch = ((r&248)|g>>5);
	bcl = ((g&28)<<3|b>>3);
	_transparent = false;
}

void UTFT::setBackColor(uint32_t color)
{
	if (color == VGA_TRANSPARENT)
		_transparent = true;
	else
	{
		bch = byte(color>>8);
		bcl = byte(color & 0xFF);
		_transparent = false;
	}
}

word UTFT::getBackColor()
{
	return (bch<<8) | bcl;
}

void UTFT::setPixel(word color)
{
	LCD_Write_DATA((color>>8),(color&0xFF));	// rrrrrggggggbbbbb
}

void UTFT::drawPixel(int x, int y)
{
	digitalWrite_f(__p3, LOW);
	setXY(x, y, x, y);
	setPixel((fch<<8)|fcl);
	digitalWrite_f(__p3, HIGH);
	clrXY();
}

void UTFT::drawLine(int x1, int y1, int x2, int y2)
{
	if (y1 == y2)
		drawHLine(x1, y1, x2-x1);
	else if (x1 == x2)
		drawVLine(x1, y1, y2-y1);
	else
	{
		unsigned int	dx = (x2 > x1 ? x2 - x1 : x1 - x2);
		short			xstep =  x2 > x1 ? 1 : -1;
		unsigned int	dy = (y2 > y1 ? y2 - y1 : y1 - y2);
		short			ystep =  y2 > y1 ? 1 : -1;
		int				col = x1, row = y1;

		digitalWrite_f(__p3, LOW);
		if (dx < dy)
		{
			int t = - (dy >> 1);
			while (true)
			{
				setXY(col, row, col, row);
				LCD_Write_DATA(fch, fcl);
				if (row == y2)
					return;
				row += ystep;
				t += dx;
				if (t >= 0)
				{
					col += xstep;
					t   -= dy;
				}
			} 
		}
		else
		{
			int t = -(dx >> 1);
			while (true)
			{
				setXY (col, row, col, row);
				LCD_Write_DATA (fch, fcl);
				if (col == x2)
					return;
				col += xstep;
				t += dy;
				if (t >= 0)
				{
					row += ystep;
					t   -= dx;
				}
			} 
		}
		digitalWrite_f(__p3, HIGH);
	}
	clrXY();
}

void UTFT::drawHLine(int x, int y, int l)
{
	if (l<0)
	{
		l = -l;
		x -= l;
	}
	digitalWrite_f(__p3, LOW);
	setXY(x, y, x+l, y);
	if (display_transfer_mode == 16)
	{
		digitalWrite_f(__p1, HIGH);
		_fast_fill_16(fch, fcl, l);
	}
	else if ((display_transfer_mode == 8) and (fch == fcl))
	{
		digitalWrite_f(__p1, HIGH);
		_fast_fill_8(fch, l);
	}
	else
	{
		for (int i=0; i<l+1; i++)
		{
			LCD_Write_DATA(fch, fcl);
		}
	}
	digitalWrite_f(__p3, HIGH);
	clrXY();
}

void UTFT::drawVLine(int x, int y, int l)
{
	if (l<0)
	{
		l = -l;
		y -= l;
	}
	digitalWrite_f(__p3, LOW);
	setXY(x, y, x, y+l);
	if (display_transfer_mode == 16)
	{
		digitalWrite_f(__p1, HIGH);
		_fast_fill_16(fch, fcl, l);
	}
	else if ((display_transfer_mode == 8) and (fch == fcl))
	{
		digitalWrite_f(__p1, HIGH);
		_fast_fill_8(fch, l);
	}
	else
	{
		for (int i=0; i<l+1; i++)
		{
			LCD_Write_DATA(fch, fcl);
		}
	}
	digitalWrite_f(__p3, HIGH);
	clrXY();
}

void UTFT::printChar(byte c, int x, int y)
{
	byte i,ch;
	word j;
	word temp; 

	digitalWrite_f(__p3, LOW);
  
	if (!_transparent)
	{
		if (orient == PORTRAIT)
		{
			setXY(x, y, x+cfont.x_size-1, y+cfont.y_size-1);
	  
			temp = ((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;
			for(j=0; j<((cfont.x_size/8)*cfont.y_size); j++)
			{
				ch = pgm_read_byte(&cfont.font[temp]);
				for(i=0; i<8; i++)
				{   
					if((ch&(1<<(7-i)))!=0)   
					{
						setPixel((fch<<8)|fcl);
					} 
					else
					{
						setPixel((bch<<8)|bcl);
					}   
				}
				temp++;
			}
		}
		else
		{
			temp = ((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;

			for(j=0; j<((cfont.x_size/8)*cfont.y_size); j+=(cfont.x_size/8))
			{
				setXY(x, y+(j/(cfont.x_size/8)), x+cfont.x_size-1, y+(j/(cfont.x_size/8)));
				for (int zz=(cfont.x_size/8)-1; zz>=0; zz--)
				{
					ch = pgm_read_byte(&cfont.font[temp+zz]);
					for(i=0;i<8;i++)
					{   
						if((ch&(1<<i)) != 0)   
						{
							setPixel((fch<<8)|fcl);
						} 
						else
						{
							setPixel((bch<<8)|bcl);
						}   
					}
				}
				temp+=(cfont.x_size/8);
			}
		}
	}
	else
	{
		temp = ((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;
		for(j=0; j<cfont.y_size; j++) 
		{
			for(int zz=0; zz<(cfont.x_size/8); zz++)
			{
				ch = pgm_read_byte(&cfont.font[temp+zz]); 
				for(i=0; i<8; i++)
				{   
					setXY(x+i+(zz*8),y+j,x+i+(zz*8)+1,y+j+1);
				
					if((ch&(1<<(7-i)))!=0)   
					{
						setPixel((fch<<8)|fcl);
					} 
				}
			}
			temp += (cfont.x_size/8);
		}
	}

	digitalWrite_f(__p3, HIGH);
	clrXY();
}

void UTFT::rotateChar(byte c, int x, int y, int pos, int deg)
{
	byte i,j,ch;
	word temp; 
	int newx,newy;
	double radian;
	radian = deg*0.0175;  

	digitalWrite_f(__p3, LOW);

	temp = ((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;
	for(j=0; j<cfont.y_size; j++) 
	{
		for (int zz=0; zz<(cfont.x_size/8); zz++)
		{
			ch = pgm_read_byte(&cfont.font[temp+zz]); 
			for(i=0; i<8; i++)
			{   
				newx = x+(((i+(zz*8)+(pos*cfont.x_size))*cos(radian))-((j)*sin(radian)));
				newy = y+(((j)*cos(radian))+((i+(zz*8)+(pos*cfont.x_size))*sin(radian)));

				setXY(newx, newy, newx+1, newy+1);
				
				if((ch&(1<<(7-i))) != 0)   
				{
					setPixel((fch<<8)|fcl);
				} 
				else  
				{
					if (!_transparent)
						setPixel((bch<<8)|bcl);
				}   
			}
		}
		temp += (cfont.x_size/8);
	}
	digitalWrite_f(__p3, HIGH);
	clrXY();
}

void UTFT::print(char *st, int x, int y, int deg)
{
	int stl, i;

	stl = strlen(st);

	if (orient == PORTRAIT)
	{
	if (x == RIGHT)
		x = (disp_x_size+1)-(stl*cfont.x_size);
	if (x == CENTER)
		x = ((disp_x_size+1)-(stl*cfont.x_size))/2;
	}
	else
	{
	if (x == RIGHT)
		x = (disp_y_size+1)-(stl*cfont.x_size);
	if (x == CENTER)
		x = ((disp_y_size+1)-(stl*cfont.x_size))/2;
	}

	for (i=0; i<stl; i++)
		if (deg == 0)
			printChar(*st++, x + (i*(cfont.x_size)), y);
		else
			rotateChar(*st++, x, y, i, deg);
}

void UTFT::print(String st, int x, int y, int deg)
{
	char buf[st.length()+1];

	st.toCharArray(buf, st.length()+1);
	print(buf, x, y, deg);
}

void UTFT::printNumI(long num, int x, int y, int length, char filler)
{
	char buf[25];
	char st[27];
	boolean neg = false;
	int c = 0, f = 0;
  
	if (num == 0)
	{
		if (length != 0)
		{
			for (c=0; c<(length-1); c++)
				st[c] = filler;
			st[c] = 48;
			st[c+1] = 0;
		}
		else
		{
			st[0] = 48;
			st[1] = 0;
		}
	}
	else
	{
		if (num < 0)
		{
			neg = true;
			num = -num;
		}
	  
		while (num > 0)
		{
			buf[c] = 48+(num % 10);
			c++;
			num = (num-(num % 10))/10;
		}
		buf[c] = 0;
	  
		if (neg)
		{
			st[0] = 45;
		}
	  
		if (length > (c+neg))
		{
			for (int i=0; i<(length-c-neg); i++)
			{
				st[i+neg] = filler;
				f++;
			}
		}

		for (int i=0; i<c; i++)
		{
			st[i+neg+f] = buf[c-i-1];
		}
		st[c+neg+f] = 0;

	}

	print(st,x,y);
}

void UTFT::printNumF(double num, byte dec, int x, int y, char divider, int length, char filler)
{
	char st[27];
	boolean neg = false;

	if (dec < 1)
		dec = 1;
	else if (dec > 5)
		dec = 5;

	if (num < 0)
		neg = true;

	_convert_float(st, num, length, dec);

	if (divider != '.')
	{
		for (int i=0; i<sizeof(st); i++)
			if (st[i] == '.')
				st[i] = divider;
	}

	if (filler != ' ')
	{
		if (neg)
		{
			st[0] = '-';
			for (int i=1; i<sizeof(st); i++)
				if ((st[i] == ' ') || (st[i] == '-'))
					st[i] = filler;
		}
		else
		{
			for (int i=0; i<sizeof(st); i++)
				if (st[i] == ' ')
					st[i] = filler;
		}
	}

	print(st, x, y);
}

void UTFT::setFont(uint8_t* font)
{
	cfont.font = font;
	cfont.x_size = fontbyte(0);
	cfont.y_size = fontbyte(1);
	cfont.offset = fontbyte(2);
	cfont.numchars = fontbyte(3);
}

uint8_t* UTFT::getFont()
{
	return cfont.font;
}

uint8_t UTFT::getFontXsize()
{
	return cfont.x_size;
}

uint8_t UTFT::getFontYsize()
{
	return cfont.y_size;
}

void UTFT::drawBitmap(int x, int y, int sx, int sy, bitmapdatatype data, int scale)
{
	unsigned int col;
	int tx, ty, tc, tsx, tsy;

	if (scale == 1)
	{
		if (orient == PORTRAIT)
		{
			digitalWrite_f(__p3, LOW);
			setXY(x, y, x+sx-1, y+sy-1);
			for (tc=0; tc<(sx*sy); tc++)
			{
				col = pgm_read_word(&data[tc]);
				LCD_Write_DATA(col>>8, col & 0xff);
			}
			digitalWrite_f(__p3, HIGH);
		}
		else
		{
			digitalWrite_f(__p3, LOW);
			for (ty=0; ty<sy; ty++)
			{
				setXY(x, y+ty, x+sx-1, y+ty);
				for (tx=sx-1; tx>=0; tx--)
				{
					col = pgm_read_word(&data[(ty*sx)+tx]);
					LCD_Write_DATA(col>>8, col & 0xff);
				}
			}
			digitalWrite_f(__p3, HIGH);
		}
	}
	else
	{
		if (orient == PORTRAIT)
		{
			digitalWrite_f(__p3, LOW);
			for (ty=0; ty<sy; ty++)
			{
				setXY(x, y+(ty*scale), x+((sx*scale)-1), y+(ty*scale)+scale);
				for (tsy=0; tsy<scale; tsy++)
					for (tx=0; tx<sx; tx++)
					{
						col = pgm_read_word(&data[(ty*sx)+tx]);
						for (tsx=0; tsx<scale; tsx++)
							LCD_Write_DATA(col>>8,col & 0xff);
					}
			}
			digitalWrite_f(__p3, HIGH);
		}
		else
		{
			digitalWrite_f(__p3, LOW);
			for (ty=0; ty<sy; ty++)
			{
				for (tsy=0; tsy<scale; tsy++)
				{
					setXY(x, y+(ty*scale)+tsy, x+((sx*scale)-1), y+(ty*scale)+tsy);
					for (tx=sx-1; tx>=0; tx--)
					{
						col = pgm_read_word(&data[(ty*sx)+tx]);
						for (tsx=0; tsx<scale; tsx++)
							LCD_Write_DATA(col>>8,col & 0xff);
					}
				}
			}
			digitalWrite_f(__p3, HIGH);
		}
	}
	clrXY();
}

void UTFT::drawBitmap(int x, int y, int sx, int sy, bitmapdatatype data, int deg, int rox, int roy)
{
	unsigned int col;
	int tx, ty, newx, newy;
	double radian;
	radian = deg*0.0175;  

	if (deg == 0)
		drawBitmap(x, y, sx, sy, data);
	else
	{
		digitalWrite_f(__p3, LOW);
		for (ty=0; ty<sy; ty++)
			for (tx=0; tx<sx; tx++)
			{
				col = pgm_read_word(&data[(ty*sx)+tx]);

				newx = x+rox+(((tx-rox)*cos(radian))-((ty-roy)*sin(radian)));
				newy = y+roy+(((ty-roy)*cos(radian))+((tx-rox)*sin(radian)));

				setXY(newx, newy, newx, newy);
				LCD_Write_DATA(col>>8, col & 0xff);
			}
		digitalWrite_f(__p3, HIGH);
	}
	clrXY();
}

void UTFT::lcdOff()
{
	digitalWrite_f(__p3, LOW);
	switch (display_model)
	{
	case PCF8833:
		LCD_Write_COM(0x28);
		break;
	case CPLD:
		LCD_Write_COM_DATA(0x01, 0x0000);
		LCD_Write_COM(0x0F);   
		break;
	}
	digitalWrite_f(__p3, HIGH);
}

void UTFT::lcdOn()
{
	digitalWrite_f(__p3, LOW);
	switch (display_model)
	{
	case PCF8833:
		LCD_Write_COM(0x29);
		break;
	case CPLD:
		LCD_Write_COM_DATA(0x01, 0x0010);
		LCD_Write_COM(0x0F);   
		break;
	}
	digitalWrite_f(__p3, HIGH);
}

void UTFT::setContrast(char c)
{
	digitalWrite_f(__p3, LOW);
	switch (display_model)
	{
	case PCF8833:
		if (c>64) c = 64;
		LCD_Write_COM(0x25);
		LCD_Write_DATA(c);
		break;
	}
	digitalWrite_f(__p3, HIGH);
}

int UTFT::getDisplayXSize()
{
	if (orient == PORTRAIT)
		return disp_x_size + 1;
	else
		return disp_y_size + 1;
}

int UTFT::getDisplayYSize()
{
	if (orient == PORTRAIT)
		return disp_y_size + 1;
	else
		return disp_x_size + 1;
}

void UTFT::setBrightness(byte br)
{
	digitalWrite_f(__p3, LOW);
	switch (display_model)
	{
	case CPLD:
		if (br > 16) br = 16;
		LCD_Write_COM_DATA(0x01, br);
		LCD_Write_COM(0x0F);   
		break;
	}
	digitalWrite_f(__p3, HIGH);
}

void UTFT::setDisplayPage(byte page)
{
	digitalWrite_f(__p3, LOW);
	switch (display_model)
	{
	case CPLD:
		if (page > 7) page = 7;
		LCD_Write_COM_DATA(0x04, page);
		LCD_Write_COM(0x0F);   
		break;
	}
	digitalWrite_f(__p3, HIGH);
}

void UTFT::setWritePage(byte page)
{
	digitalWrite_f(__p3, LOW);
	switch (display_model)
	{
	case CPLD:
		if (page > 7) page = 7;
		LCD_Write_COM_DATA(0x05, page);
		LCD_Write_COM(0x0F);   
		break;
	}
	digitalWrite_f(__p3, HIGH);
}





// *** Hardwarespecific functions ***
void UTFT::_hw_special_init()
{
}

void UTFT::LCD_Writ_Bus(char VH,char VL, byte mode)
{   
	switch (mode)
	{
	case 1:
		if (display_serial_mode == SERIAL_4PIN)
		{
			if (VH == 1)
				digitalWrite_f(__p1, HIGH);
			else
				digitalWrite_f(__p1, LOW);
			digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		}
		else
		{
			if (VH == 1)
				digitalWrite_f(__p5, HIGH);
			else
				digitalWrite_f(__p5, LOW);
		}

		if (VL & 0x80)
			digitalWrite_f(__p1, HIGH);
		else
			digitalWrite_f(__p1, LOW);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		if (VL & 0x40)
			digitalWrite_f(__p1, HIGH);
		else
			digitalWrite_f(__p1, LOW);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		if (VL & 0x20)
			digitalWrite_f(__p1, HIGH);
		else
			digitalWrite_f(__p1, LOW);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		if (VL & 0x10)
			digitalWrite_f(__p1, HIGH);
		else
			digitalWrite_f(__p1, LOW);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		if (VL & 0x08)
			digitalWrite_f(__p1, HIGH);
		else
			digitalWrite_f(__p1, LOW);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		if (VL & 0x04)
			digitalWrite_f(__p1, HIGH);
		else
			digitalWrite_f(__p1, LOW);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		if (VL & 0x02)
			digitalWrite_f(__p1, HIGH);
		else
			digitalWrite_f(__p1, LOW);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		if (VL & 0x01)
			digitalWrite_f(__p1, HIGH);
		else
			digitalWrite_f(__p1, LOW);
		digitalWrite_f(__p2, LOW);
		digitalWrite_f(__p2, HIGH);
		break;
	case 8:
		#if defined __86DUINO_ONE
			if(UseVirtualPort == 1)
				virtual_port_output_value(0, (unsigned char)VH);
			else
				port_output_value(4, (unsigned char)VH);
		#else
        	virtual_port_output_value(0, (unsigned char)VH);
		#endif

		digitalWrite_f(__p2, LOW);
		digitalWrite_f(__p2, HIGH);
		
		#if defined __86DUINO_ONE
			if(UseVirtualPort == 1)
				virtual_port_output_value(0, (unsigned char)VL);
			else
				port_output_value(4, (unsigned char)VL);
		#else
        	virtual_port_output_value(0, (unsigned char)VL);
		#endif

		digitalWrite_f(__p2, LOW);
		digitalWrite_f(__p2, HIGH);
		break;
	case 16:
		#if defined __86DUINO_ONE
			if(UseVirtualPort == 1)
			{
				virtual_port_output_value(1, (unsigned char)VH);
	        	virtual_port_output_value(0, (unsigned char)VL);
	        }
			else
			{
				port_output_value(4, (unsigned char)VH);
				port_output_value(3, (unsigned char)VL);
			}
		#elif defined __86DUINO_EDUCAKE
	        virtual_port_output_value(1, (unsigned char)VH);
	        virtual_port_output_value(0, (unsigned char)VL);
		#endif

		digitalWrite_f(__p2, LOW);
		digitalWrite_f(__p2, HIGH);
		break;
	case LATCHED_16:
		virtual_port_output_value(0, (unsigned char)VH);

		digitalWrite_f(__p5, LOW);
		digitalWrite_f(__p5, HIGH);
		digitalWrite_f(__p5, LOW);
		digitalWrite_f(__p3, LOW);
		
		virtual_port_output_value(0, (unsigned char)VL);
		digitalWrite_f(__p2, LOW);
		digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p3, HIGH);
		break;
	}
}

void UTFT::_set_direction_registers(byte mode)
{
	if (mode != LATCHED_16)
	{
		#if defined __86DUINO_ONE
			if(UseVirtualPort == 1)
			{
			    // use virtual port 0
				for(int i=0; i<8; i++)
				{
					pinMode(i, OUTPUT);
					digitalWrite(i, HIGH);
				}
				
				if (mode == 16)
				{
					// use virtual port 1
					for(int i=8; i<16; i++)
					{
						pinMode(i, OUTPUT);
						digitalWrite(i, HIGH);
					}
				}
			}
			else
			{
				port_output_dir(4, 0xff);
				if (mode == 16) port_output_dir(3, 0xff);
			}
		#elif defined __86DUINO_EDUCAKE
	        // use virtual port 0
			for(int i=0; i<8; i++)
			{
				pinMode(i, OUTPUT);
				digitalWrite(i, HIGH);
			}
			
			if (mode == 16)
			{
				// use virtual port 1
				for(int i=8; i<16; i++)
				{
					pinMode(i, OUTPUT);
					digitalWrite(i, HIGH);
				}
			} 
		#elif defined __86DUINO_ZERO
			// use virtual port 0
			for(int i=0; i<8; i++)
			{
				pinMode(i, OUTPUT);
				digitalWrite(i, HIGH);
			}
		#endif
	}
	else
	{
		virtual_port_output_value(0, 0xff);
	}
}

void UTFT::_fast_fill_16(int ch, int cl, long pix)
{
	long blocks;

	#if defined __86DUINO_ONE
		if(UseVirtualPort == 1)
		{
		    virtual_port_output_value(1, (unsigned char)ch);
	    	virtual_port_output_value(0, (unsigned char)cl);
		}
		else
		{
		    port_output_value(4, (unsigned char)ch);
		    port_output_value(3, (unsigned char)cl);
		}
	#elif defined __86DUINO_EDUCAKE
	    virtual_port_output_value(1, (unsigned char)ch);
	    virtual_port_output_value(0, (unsigned char)cl);
	#endif // no include Zero

	blocks = pix/16;
	for (int i=0; i<blocks; i++)
	{
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
	}
	if ((pix % 16) != 0)
		for (int i=0; i<(pix % 16)+1; i++)
		{
			digitalWrite_f(__p2, LOW);
			digitalWrite_f(__p2, HIGH);
		}
}

void UTFT::_fast_fill_8(int ch, long pix)
{
	long blocks;

	#if defined __86DUINO_ONE
		if(UseVirtualPort == 1)
	    	virtual_port_output_value(0, (unsigned char)ch);
	    else
	        port_output_value(4, (unsigned char)ch);
	#else
		virtual_port_output_value(0, (unsigned char)ch);
	#endif

	blocks = pix/16;
	for (int i=0; i<blocks; i++)
	{
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
		digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);digitalWrite_f(__p2, LOW);digitalWrite_f(__p2, HIGH);
	}
	if ((pix % 16) != 0)
	{
		for (int i=0; i<(pix % 16)+1; i++)
		{
			digitalWrite_f(__p2, LOW);
			digitalWrite_f(__p2, HIGH);
			digitalWrite_f(__p2, LOW);
			digitalWrite_f(__p2, HIGH);
		}
	}
}

void UTFT::_convert_float(char *buf, double num, int width, byte prec)
{
	char format[10];
	
	sprintf(format, "%%%i.%if", width, prec);
	sprintf(buf, format, num);
}

void UTFT::port_output_dir(int port, unsigned char vlaue){
	unsigned short crossbar_ioaddr = sb_Read16(SB_CROSSBASE) & 0xfffe;
	int port_base = (GPIODIRBASE + 4*port)&0xffff;
	
	if(port <= 3)
	{
	  for(int i=0; i<8; i++)
	    io_outpb(crossbar_ioaddr + 0x90 + port*8 + i, 0x01);
	}
	else if(port == 4)
	{
	  for(int i=0; i<8; i++)
	    io_outpb(crossbar_ioaddr + 0x80 + ((32+i)/8), 0x01);
	} 
	io_DisableINT();
	io_outpb(port_base, vlaue);
	io_RestoreINT();
}
 
void UTFT::port_output_value(int port, unsigned char vlaue) {
	int port_base = (GPIODATABASE + 4*port)&0xffff;
	io_DisableINT();
	io_outpb(port_base, vlaue);
	io_RestoreINT();
}

// only for not 86Duino One board
void UTFT::virtual_port_output_value(int port, unsigned char value) {
	// virtual port 0: RX1(0), TX1(1), 2, 3, 4, 5, 6, 7 
	// virtual port 1: 8, 9, 10, 11, 12, 13, 14, 15
	if(port == 0)
	{
		for(int i=0; i<8; i++)
		{
			if(value & 0x01)
				digitalWrite(i, HIGH);
			else
				digitalWrite(i, LOW);
			value >>= 1;
		}
	}

	#if defined __86DUINO_EDUCAKE || defined __86DUINO_ONE
	    if(port == 1)
		{
			for(int i=8; i<16; i++)
			{
				if(value & 0x01)
					digitalWrite(i, HIGH);
				else
					digitalWrite(i, LOW);
				value >>= 1;
			}
		}
	#endif
}
                 
void UTFT::port_output_cport(int port, unsigned char vlaue) {
	int port_base = (GPIODATABASE + 4*port)&0xffff;
	io_DisableINT();
	io_outpb(port_base, io_inpb(port_base) & vlaue);
	io_RestoreINT();
}

void UTFT::port_output_sport(int port, unsigned char vlaue) {
	int port_base = (GPIODATABASE + 4*port)&0xffff;
	io_DisableINT();
	io_outpb(port_base, io_inpb(port_base) | vlaue);
	io_RestoreINT();
}

void UTFT::digitalWrite_f(uint8_t pin, uint8_t val) {
	int i = 0;
	if(pin == __p2)
		i = 1;
	else if(pin == __p1)
		i = 0;
	else if(pin == __p3)
		i = 2;
	else if(pin == __p4)
		i = 3;
	else if(pin == __p5)
		i = 4;          
	io_DisableINT();
	if (val == LOW) 
	    io_outpb(pin_p_v[0][i], io_inpb(pin_p_v[0][i])&(~pin_p_v[1][i]));
	else
	    io_outpb(pin_p_v[0][i], io_inpb(pin_p_v[0][i])|pin_p_v[1][i]);
	io_RestoreINT();
}

void UTFT::To_GPIO(uint8_t pin) {
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

void usePortMode(void) {
    UseVirtualPort = 0;
}

void usePinMode(void) {
    UseVirtualPort = 1;
}
