/*
This file is part of the GSM3 communications library for Arduino
-- Multi-transport communications platform
-- Fully asynchronous
-- Includes code for the Arduino-Telefonica GSM/GPRS Shield V1
-- Voice calls
-- SMS
-- TCP/IP connections
-- HTTP basic clients

This library has been developed by Telefónica Digital - PDI -
- Physical Internet Lab, as part as its collaboration with
Arduino and the Open Hardware Community. 

September-December 2012
Modified 20 April 2014 by Android Lin for 86Duino

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

The latest version of this library can always be found at
https://github.com/BlueVia/Official-Arduino
*/
#include "GSM3SoftSerial.h"


#if defined (__86DUINO_ZERO) || defined (__86DUINO_ONE) || defined (__86DUINO_EDUCAKE) 
#define __TXPIN__ 3
#define __RXPIN__ 42
#endif

#define __XON__ 0x11
#define __XOFF__ 0x13

#define _GSMSOFTSERIALFLAGS_ESCAPED_ 0x01
#define _GSMSOFTSERIALFLAGS_SENTXOFF_ 0x02

#define pgm_read_word(addr)   (*(const unsigned short*)(addr))
#define pgm_read_dword(addr) (*(const unsigned long*)(addr))

extern bool hwcomIsUsed;
extern int hwcomport;

typedef struct _DELAY_TABLE
{
	long baud;
	unsigned short rx_delay_centering;
	unsigned short rx_delay_intrabit;
	unsigned short rx_delay_stopbit;
	unsigned short tx_delay;
} DELAY_TABLE;

//
// Lookup table
//
#define __PARAGRAPHGUARD__ 50

#if defined (__86DUINO_ZERO) || defined (__86DUINO_ONE) || defined (__86DUINO_EDUCAKE)
// time unit is microseconds
static const DELAY_TABLE table[] = 
{
	//  baud    rxcenter   rxintra    rxstop    tx
	{ 19200,     0,        50,        50,       49,   },
	{ 14400,     9,        67,        67,       66,   },
	{ 9600,     26,        100,       100,      100,  },
	{ 4800,     78,        204,       204,      204,  },
	{ 2400,     182,       410,       410,      412,  },
	{ 1200,     390,       826,       826,      828,  },
	{ 600,      806,       1654,      1654,     1660, },
	{ 300,      1638,      3308,      3308,     3324, },
};

static int pinMap2[EXTERNAL_NUM_INTERRUPTS] = {42, 43, 44, 18, 19, 20, 33, 34, 35, 36, 37, 38};

#else

#error This version of GSM3SoftSerial supports only 20, 16 and 8MHz processors

#endif

GSM3SoftSerial* GSM3SoftSerial::_activeObject=0;

GSM3SoftSerial::GSM3SoftSerial():
	_rx_delay_centering(0),
	_rx_delay_intrabit(0),
	_rx_delay_stopbit(0),
	_tx_delay(0),
	cb(this)
{
    
	//setTX();
	//setRX();
}

int GSM3SoftSerial::begin(long speed)
{   
	_rx_delay_centering = _rx_delay_intrabit = _rx_delay_stopbit = _tx_delay = 0;
	
	setTX();
	setRX();
	
	for (unsigned i=0; i<sizeof(table)/sizeof(table[0]); ++i)
	{
		long baud = pgm_read_dword(&table[i].baud);
		if (baud == speed)
		{
			_rx_delay_centering = pgm_read_word(&table[i].rx_delay_centering);
			_rx_delay_intrabit = pgm_read_word(&table[i].rx_delay_intrabit);
			_rx_delay_stopbit = pgm_read_word(&table[i].rx_delay_stopbit);
			_tx_delay = pgm_read_word(&table[i].tx_delay);
			break;
		}
	}

	// Set up RX interrupts, but only if we have a valid RX baud rate
	/*
	if (_rx_delay_stopbit)
	{
		pinMode(__RXPIN__, INPUT_PULLUP);
		tunedDelay(_tx_delay); // if we were low this establishes the end
	}
    */
    io_DisableINT();
    io_outpb(CROSSBARBASE + 0x90 + pinMap[__RXPIN__], 0x08);
	_activeObject = this;
	io_RestoreINT();
}

void GSM3SoftSerial::close()
{
	_activeObject=0;
}

size_t GSM3SoftSerial::write(uint8_t c)
{   
	if (hwcomIsUsed == false && _tx_delay == 0)
		return 0;

	// Characters to be escaped under XON/XOFF control with Quectel
	if(c==0x11)
	{
		this->finalWrite(0x77);
		return this->finalWrite(0xEE);
	}

	if(c==0x13)
	{
		this->finalWrite(0x77);
		return this->finalWrite(0xEC);
	}

	if(c==0x77)
	{
		this->finalWrite(0x77);
		return this->finalWrite(0x88);
	}
	
	return this->finalWrite(c);
}

size_t GSM3SoftSerial::finalWrite(uint8_t c)
{
	if(hwcomIsUsed == true) return HWSerial[hwcomport]->write(c);
	
	io_DisableINT();

	// Write the start bit
	tx_pin_write(LOW);
	tunedDelay(_tx_delay);

	// Write each of the 8 bits
	for (byte mask = 0x01; mask; mask <<= 1)
	{
		if (c & mask) // choose bit
			tx_pin_write(HIGH); // send 1
		else
			tx_pin_write(LOW); // send 0
		tunedDelay(_tx_delay);
	}

	tx_pin_write(HIGH); // restore pin to natural state
	
	io_RestoreINT();
	tunedDelay(_tx_delay);
				
	return 1;
}

void GSM3SoftSerial::tunedDelay(uint16_t delaytime) { 
	delayMicroseconds(delaytime);
}

void GSM3SoftSerial::tx_pin_write(uint8_t pin_state)
{
  // Direct port manipulation is faster than digitalWrite/Read
  if (pin_state == LOW)
		digitalWrite(__TXPIN__, LOW);
	else
		digitalWrite(__TXPIN__, HIGH);
}

void GSM3SoftSerial::setTX()
{
	pinMode(__TXPIN__, OUTPUT);
	digitalWrite(__TXPIN__, HIGH);
}

void GSM3SoftSerial::setRX()
{   
	int i;
	pinMode(__RXPIN__, INPUT);
	digitalWrite(__RXPIN__, HIGH); 
    
    set_MMIO();
	mc_setbaseaddr();
	for(i=0; i<4; i++)
		mc_SetMode(i, MCMODE_PWM_SIFB);
		
#if defined (__86DUINO_ZERO) || defined (__86DUINO_ONE) || defined (__86DUINO_EDUCAKE)
	for(i=0; i<EXTERNAL_NUM_INTERRUPTS; i++)
		if(pinMap2[i] == __RXPIN__) break;
	if(i == EXTERNAL_NUM_INTERRUPTS) i = 0;
	attachInterrupt(i, GSM3SoftSerial::handle_interrupt, FALLING);
#endif
	// This line comes from the High Middle Ages...
	// attachInterrupt(__RXINT__, GSM3SoftSerial::handle_interrupt, FALLING);
}

void GSM3SoftSerial::handle_interrupt()
{
	if(_activeObject)
		_activeObject->recv();
}

uint8_t GSM3SoftSerial::rx_pin_read()
{
  // Digital port manipulation
  return digitalRead(__RXPIN__); 
}

void GSM3SoftSerial::recv()
{
  bool firstByte=true;
  byte thisHead;
  
  uint8_t d = 0;
	
  bool morebytes=false;
  bool fullbuffer;
  bool capturado_fullbuffer = 0;
  int i;
  byte oldTail;

  // If RX line is high, then we don't see any start bit
  // so interrupt is probably not for us
  io_outpb(CROSSBARBASE + 0x90 + pinMap[__RXPIN__], 0x01); // switch to GPIO
  if (!rx_pin_read())
  {
	do
	{
		oldTail=cb.getTail();
		// Wait approximately 1/2 of a bit width to "center" the sample
		tunedDelay(_rx_delay_centering);
		
		fullbuffer=(cb.availableBytes()<6);

		
		if(fullbuffer&&(!capturado_fullbuffer))
			tx_pin_write(LOW);

		
		// Read each of the 8 bits
		for (uint8_t i=0x1; i; i <<= 1)
		{
			tunedDelay(_rx_delay_intrabit);
			uint8_t noti = ~i;
			if (rx_pin_read())
				d |= i;
			else // else clause added to ensure function timing is ~balanced
				d &= noti;
			
			if(fullbuffer&&(!capturado_fullbuffer))
			{
			  if((uint8_t)__XOFF__ & i)
				tx_pin_write(HIGH);
			  else
				tx_pin_write(LOW);
			}
		}

		if(fullbuffer&&(!capturado_fullbuffer))
		{
			tunedDelay(_rx_delay_intrabit);
			tx_pin_write(HIGH);	
		}
		
		// So, we know the buffer is full, and we have sent a XOFF
		if (fullbuffer) 
		{
			capturado_fullbuffer =1;
			_flags |=_GSMSOFTSERIALFLAGS_SENTXOFF_;
		}


		// skip the stop bit
		if (!fullbuffer) tunedDelay(_rx_delay_stopbit);
		
		if(keepThisChar(&d))
		{
			cb.write(d);
			if(firstByte)
			{
				firstByte=false;
				thisHead=cb.getTail();
			}
		}
		
		
		// This part is new. It is used to detect the end of a "paragraph"
		// Caveat: the old fashion would let processor a bit of time between bytes, 
		// that here is lost
		// This active waiting avoids drifting
		morebytes=false;
		// TO-DO. This PARAGRAPHGUARD is empyric. We should test it for every speed
		for(i=0;i<__PARAGRAPHGUARD__;i++)
		{	
			tunedDelay(1);
			if(!rx_pin_read())
			{
				morebytes=true;
				break;
			}
		} 
	}while(morebytes);
	// If we find a line feed, we are at the end of a paragraph
	// check!
	if (fullbuffer)
	{
		// And... go handle it!
		if(mgr)
			mgr->manageMsg(thisHead, cb.getTail());
	}
	else if(d==10)
	{
		// And... go handle it!
		if(mgr)
			mgr->manageMsg(thisHead, cb.getTail());
	}
	else if (d==32)
	{
		// And... go handle it!
		if(mgr)
			mgr->manageMsg(thisHead, cb.getTail());
	}
  }
  io_outpb(CROSSBARBASE + 0x90 + pinMap[__RXPIN__], 0x08); // switch to Encoder
}

bool GSM3SoftSerial::keepThisChar(uint8_t* c)
{
	// Horrible things for Quectel XON/XOFF
	// 255 is the answer to a XOFF
	// It comes just once
	if((*c==255)&&(_flags & _GSMSOFTSERIALFLAGS_SENTXOFF_))
	{
		_flags ^= _GSMSOFTSERIALFLAGS_SENTXOFF_;
		return false;
	}

	// 0x77, w, is the escape character
	if(*c==0x77)
	{
		_flags |= _GSMSOFTSERIALFLAGS_ESCAPED_;
		return false;
	}
	
	// and these are the escaped codes
	if(_flags & _GSMSOFTSERIALFLAGS_ESCAPED_)
	{
		if(*c==0xEE)
			*c=0x11;
		else if(*c==0xEC)
			*c=0x13;
		else if(*c==0x88)
			*c=0x77;
			
		_flags ^= _GSMSOFTSERIALFLAGS_ESCAPED_;
		return true;
	}
	
	return true;
}

void GSM3SoftSerial::spaceAvailable()
{
	// If there is spaceAvailable in the buffer, lets send a XON
	finalWrite((byte)__XON__);
}


// This is here to avoid problems with Arduino compiler
void GSM3SoftSerialMgr::manageMsg(byte from, byte to){};

