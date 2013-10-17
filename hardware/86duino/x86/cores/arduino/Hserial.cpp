#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <inttypes.h>
#include "Arduino.h"
//#include "wiring_private.h"


#include "com.h"
COMPort *handle = NULL;
unsigned char buf[13] = {0};

#if (RAMEND < 1000)
  #define SERIAL_BUFFER_SIZE 16
#else
  #define SERIAL_BUFFER_SIZE 64
#endif

struct ring_buffer
{
  unsigned char buffer[SERIAL_BUFFER_SIZE];
  volatile unsigned int head;
  volatile unsigned int tail;
};

#if defined(USBCON)
  ring_buffer rx_buffer = { { 0 }, 0, 0};
  ring_buffer tx_buffer = { { 0 }, 0, 0};
#endif
#if defined(UBRRH) || defined(UBRR0H)
  ring_buffer rx_buffer  =  { { 0 }, 0, 0 };
  ring_buffer tx_buffer  =  { { 0 }, 0, 0 };
#endif
#if defined(UBRR1H)
  ring_buffer rx_buffer1  =  { { 0 }, 0, 0 };
  ring_buffer tx_buffer1  =  { { 0 }, 0, 0 };
#endif
#if defined(UBRR2H)
  ring_buffer rx_buffer2  =  { { 0 }, 0, 0 };
  ring_buffer tx_buffer2  =  { { 0 }, 0, 0 };
#endif
#if defined(UBRR3H)
  ring_buffer rx_buffer3  =  { { 0 }, 0, 0 };
  ring_buffer tx_buffer3  =  { { 0 }, 0, 0 };
#endif


// Public Methods //////////////////////////////////////////////////////////////

void HardwareSerial::begin(unsigned long baud)
{

	if ((handle = com_Init(COM1)) == NULL) 
	{
		printf("COM init fail!!\n");
	}
	com_SetBPS(handle, baud);
	com_SetFormat(handle,BYTESIZE8|NOPARITY|STOPBIT1);  
	com_FlushTxQueue(handle);
	com_FlushRxQueue(handle);
	com_SetTimeOut(handle,100);  
}

/*void HardwareSerial::begin(unsigned long baud, byte config)
{
  if ((handle = com_Init(COM1)) == NULL) 
  {
		printf("COM init fail!!\n");
	}
  com_SetBPS(handle, baud);  
} */

void HardwareSerial::end()
{
	com_FlushWFIFO(handle);
	com_Close(handle);
}

int HardwareSerial::available(void)
{
    return com_QueryRxQueue(handle);
} 

int HardwareSerial::peek(void)
{
  /*if (_rx_buffer->head == _rx_buffer->tail) {
    return -1;
  } else {
    return _rx_buffer->buffer[_rx_buffer->tail];
  } */
}

int HardwareSerial::read(void)
{
    unsigned char c = com_Read(handle);
    return c;
}

void HardwareSerial::flush()
{
    com_FlushTxQueue(handle);
}

size_t HardwareSerial::write(uint8_t c)
{
	com_Write(handle,c);
	return 1;
} 

HardwareSerial::operator bool() {
	return true;
} 

HardwareSerial Serial1;
// Preinstantiate Objects //////////////////////////////////////////////////////
