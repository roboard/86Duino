#include <stdio.h>
#include "arduino.h"

#define DMP_86DUINO_MODE
#include "usb.h"
#include "usbapi.h"
#define USE_COMMON
#include "common.h"

#if defined(USBCON)
//#ifdef CDC_ENABLED


void Serial_::begin(uint16_t baud_count)
{
    /*
	USBDEV = CreateUSBDevice();
    if(USBDEV == NULL)
    {
        printf("init error\n");
        return;
    }
    usb_SetUSBPins(USBDEV, 7, 0, 7, 1);
    if(usb_Init(USBDEV) == false)
    {
        printf("init2 error\n");
        return;
    }
    */
    //while(usb_Ready(USBDEV) == false);
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
	return usb_QueryRxQueue(USBDEV);
}


int Serial_::peek(void)
{
/*
	ring_buffer *buffer = &cdc_rx_buffer;
	if (buffer->head == buffer->tail) {
		return -1;
	} else {
		return buffer->buffer[buffer->tail];
	}
*/
}


int Serial_::read(void)
{
	if (usb_RxQueueEmpty(USBDEV) == true)
		return -1;
	else
		return usb_Read(USBDEV);	
}


void Serial_::flush(void)
{
	usb_FlushTxQueue(USBDEV);
}


size_t Serial_::write(uint8_t c)
{
	if (usb_Ready(USBDEV) != false)
		usb_Write(USBDEV, c);
		
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

#endif /* if defined(USBCON) */
