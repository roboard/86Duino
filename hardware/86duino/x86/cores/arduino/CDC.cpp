#include <stdio.h>
#include "arduino.h"
#include "usb.h"
#include "USBAPI.h"
#define USE_COMMON
#include "common.h"


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
  			peek_val = -1;
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
	delay_ms(10);
	return result;
}

Serial_ Serial;

