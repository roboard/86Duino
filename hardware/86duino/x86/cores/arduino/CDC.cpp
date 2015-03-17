/* Copyright (c) 2011, Peter Barrett  
**  
** Permission to use, copy, modify, and/or distribute this software for  
** any purpose with or without fee is hereby granted, provided that the  
** above copyright notice and this permission notice appear in all copies.  
** 
** THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL  
** WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED  
** WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR  
** BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES  
** OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,  
** WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,  
** ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS  
** SOFTWARE.

  Modified 01 November 2013 by Android Lin  
*/

#include <stdio.h>
#include "Arduino.h"
#include "USBCore.h"
#include "USBAPI.h"


void Serial_::begin(uint16_t baud_count)
{
}

 
void Serial_::end(void)
{
    if(USBDEV == NULL) return;
    usb_Close(USBDEV);
}


void Serial_::accept(void) 
{
}


int Serial_::available(void)
{
	if(USBDEV == NULL) return 0;
	return usb_QueryRxQueue(USBDEV);
}


int Serial_::peek(void)
{
	if(USBDEV == NULL) return -1;
	if(peek_stored == true)
		return peek_val;
  	else
  	{
		if((peek_val = usb_Read(USBDEV)) == 0xFFFF)
  			return -1;//peek_val = -1;
  		peek_stored = true;
		return peek_val;
	}
}


int Serial_::read(void)
{
	int c;
	if(USBDEV == NULL) return -1;
	if(peek_stored == true)
	{
		peek_stored = false;
		return peek_val;
	}
	else
	{
		c = usb_Read(USBDEV);
		return (c == 0xFFFF) ? -1 : c;
	}	
}


void Serial_::flush(void)
{
	if(USBDEV == NULL) return;
	usb_FlushTxQueue(USBDEV);
}


size_t Serial_::write(uint8_t c)
{
	if (USBDEV != NULL && usb_Ready(USBDEV) != false)
	{
		return (usb_Write(USBDEV, c) == true) ? 1 : 0;
	}
		
	return 0;
}


Serial_::operator bool() {
	bool result = false;
	if (usb_Ready(USBDEV) != false) 
		result = true;
	timer_Delay(10);
	return result;
}

Serial_ Serial;

