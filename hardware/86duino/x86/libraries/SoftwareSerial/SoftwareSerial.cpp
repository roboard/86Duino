/*
  SFSerial.cpp - SoftwareSerial library for 86Duino
  Copyright (c) 2014 Android Lin <acen@dmp.com.tw>. All right reserved.

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

// 
// Includes
// 
#include <Arduino.h>
#include <SoftwareSerial.h>
//
// Lookup table
//
typedef struct _DELAY_TABLE
{
  long baud;
  unsigned short rx_delay_centering;
  unsigned short rx_delay_intrabit;
  unsigned short rx_delay_stopbit;
  unsigned short tx_delay;
} DELAY_TABLE;

//int _receivePin  = 0; // interrupt pin EA0
//int _transmitPin = 3;

#define pgm_read_word(word)   (*(unsigned short*)(word))
#define pgm_read_dword(dword) (*(unsigned long*)(dword))

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
	#error This version of SoftwareSerial supports only 20, 16 and 8MHz processors
#endif

//
// Statics
//
SoftwareSerial *SoftwareSerial::active_object = 0;
char SoftwareSerial::_receive_buffer[_SS_MAX_RX_BUFF]; 
volatile uint8_t SoftwareSerial::_receive_buffer_tail = 0;
volatile uint8_t SoftwareSerial::_receive_buffer_head = 0;

//
// Private methods
//

/* static */ 
inline void SoftwareSerial::tunedDelay(uint16_t delaytime) { 
	delayMicroseconds(delaytime);
}

// This function sets the current object as the "listening"
// one and returns true if it replaces another 
bool SoftwareSerial::listen()
{
	int i;
	if (active_object != this)
	{
		_buffer_overflow = false;
		io_DisableINT();
		_receive_buffer_head = _receive_buffer_tail = 0;
		active_object = this;
		io_RestoreINT();
		for(i=0; i<EXTERNAL_NUM_INTERRUPTS; i++)
			if(pinMap2[i] == _receivePin) break;
		if(i == EXTERNAL_NUM_INTERRUPTS) i = 0;
		attachInterrupt(i, SoftwareSerial::handle_interrupt, FALLING);
		return true;
	}
	
	return false;
}

//
// The receive routine called by the interrupt handler
//
void SoftwareSerial::recv()
{
	uint8_t d = 0;
	io_outpb(CROSSBARBASE + 0x90 + pinMap[_receivePin], 0x01); // switch to GPIO
	// If RX line is high, then we don't see any start bit
	// so interrupt is probably not for us
	if (_inverse_logic ? rx_pin_read() : !rx_pin_read())
	{
		// Wait approximately 1/2 of a bit width to "center" the sample
		tunedDelay(_rx_delay_centering);
		//DebugPulse(_DEBUG_PIN2, 1);
		
		// Read each of the 8 bits
		for (uint8_t i=0x1; i; i <<= 1)
		{
			tunedDelay(_rx_delay_intrabit);
			//DebugPulse(_DEBUG_PIN2, 1);
			uint8_t noti = ~i;
			if (rx_pin_read())
				d |= i;
			else // else clause added to ensure function timing is ~balanced
				d &= noti;
		}
		
		// skip the stop bit
		tunedDelay(_rx_delay_stopbit);
		//DebugPulse(_DEBUG_PIN2, 1);
		
		if (_inverse_logic)
			d = ~d;
		
		// if buffer full, set the overflow flag and return
		if ((_receive_buffer_tail + 1) % _SS_MAX_RX_BUFF != _receive_buffer_head) 
		{
			// save new data in buffer: tail points to where byte goes
			_receive_buffer[_receive_buffer_tail] = d; // save new byte
			_receive_buffer_tail = (_receive_buffer_tail + 1) % _SS_MAX_RX_BUFF;
		} 
		else 
		{
			_buffer_overflow = true;
		}
	}
	io_outpb(CROSSBARBASE + 0x90 + pinMap[_receivePin], 0x08); // switch to Encoder
}

void SoftwareSerial::tx_pin_write(uint8_t pin_state)
{
	if (pin_state == LOW)
		digitalWrite(_transmitPin, LOW);
	else
		digitalWrite(_transmitPin, HIGH);
}

uint8_t SoftwareSerial::rx_pin_read()
{
	return digitalRead(_receivePin);
}

inline void SoftwareSerial::handle_interrupt()
{
  if (active_object)
  {
    active_object->recv();
  }
}

//
// Constructor
//
SoftwareSerial::SoftwareSerial(uint8_t receivePin, uint8_t transmitPin, bool inverse_logic /* = false */) : 
	_rx_delay_centering(0),
	_rx_delay_intrabit(0),
	_rx_delay_stopbit(0),
	_tx_delay(0),
	_buffer_overflow(false),
	_inverse_logic(inverse_logic)
{
	setTX(transmitPin);
	setRX(receivePin);
}


//
// Destructor
//
SoftwareSerial::~SoftwareSerial()
{
	end();
}

void SoftwareSerial::setTX(uint8_t tx)
{
	pinMode(tx, OUTPUT);
	digitalWrite(tx, HIGH);
	_transmitPin = tx;
}

void SoftwareSerial::setRX(uint8_t rx)
{
	pinMode(rx, INPUT);
	if (!_inverse_logic)
		digitalWrite(rx, HIGH);  // pullup for normal logic!
	_receivePin = rx;
}

//
// Public methods
//

void SoftwareSerial::begin(long speed)
{
	_rx_delay_centering = _rx_delay_intrabit = _rx_delay_stopbit = _tx_delay = 0;
	
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
	if (_rx_delay_stopbit)
	{
		pinMode(_receivePin, INPUT_PULLUP);
		tunedDelay(_tx_delay); // if we were low this establishes the end
	}

	listen();
}

void SoftwareSerial::end()
{
	pinMode(_receivePin, INPUT);
}


// Read data from buffer
int SoftwareSerial::read()
{
	if (!isListening())
		return -1;
	
	// Empty buffer?
	if (_receive_buffer_head == _receive_buffer_tail)
		return -1;
	
	// Read from "head"
	uint8_t d = _receive_buffer[_receive_buffer_head]; // grab next byte
	_receive_buffer_head = (_receive_buffer_head + 1) % _SS_MAX_RX_BUFF;
	return d;
}

int SoftwareSerial::available()
{
	if (!isListening())
		return 0;

	return (_receive_buffer_tail + _SS_MAX_RX_BUFF - _receive_buffer_head) % _SS_MAX_RX_BUFF;
}

size_t SoftwareSerial::write(uint8_t b)
{
	if (_tx_delay == 0)
	{
		setWriteError();
		return 0;
	}

	io_DisableINT();
	
	// Write the start bit
	tx_pin_write(_inverse_logic ? HIGH : LOW);
	tunedDelay(_tx_delay);
	
	// Write each of the 8 bits
	if (_inverse_logic)
	{
		for (byte mask = 0x01; mask; mask <<= 1)
		{
			if (b & mask) // choose bit
				tx_pin_write(LOW); // send 1
			else
				tx_pin_write(HIGH); // send 0
			
			tunedDelay(_tx_delay);
		}
		
		tx_pin_write(LOW); // restore pin to natural state
	}
	else
	{
		for (byte mask = 0x01; mask; mask <<= 1)
		{
			if (b & mask) // choose bit
				tx_pin_write(HIGH); // send 1
			else
				tx_pin_write(LOW); // send 0
			
			tunedDelay(_tx_delay);
		}
		
		tx_pin_write(HIGH); // restore pin to natural state
	}
	
	io_RestoreINT();
	tunedDelay(_tx_delay);
	
	return 1;
}

void SoftwareSerial::flush()
{
	if (!isListening())
	  return;
	
	io_DisableINT();
	_receive_buffer_head = _receive_buffer_tail = 0;
	io_RestoreINT();
}

int SoftwareSerial::peek()
{
	if (!isListening())
		return -1;
	
	// Empty buffer?
	if (_receive_buffer_head == _receive_buffer_tail)
		return -1;
	
	// Read from "head"
	return _receive_buffer[_receive_buffer_head];
}
