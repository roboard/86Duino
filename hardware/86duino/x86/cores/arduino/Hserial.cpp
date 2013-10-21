#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Arduino.h"

#include "Hserial.h"

#define  COM1_TX    (0x9A)
#define  COM1_RX    (0x9B)
#define  COM2_TX    (0x9E)
#define  COM2_RX    (0x9F)
#define  COM3_TX    (0x9C)
#define  COM3_RX    (0x9D)

// Public Methods //////////////////////////////////////////////////////////////
HardwareSerial::HardwareSerial(int com_port, unsigned long com_baudrate, unsigned char com_format, unsigned long com_timeout) {
	port        = com_port;
	baudrate    = com_baudrate;
	format      = com_format;
	timeout     = com_timeout;
	peek_stored = false;
	handle      = NULL;
}


void HardwareSerial::begin(unsigned long baud) {
	begin(baud, format);
}

void HardwareSerial::begin(unsigned long baud, byte config) {
    unsigned short crossbar_ioaddr = 0;
	if(handle != NULL) return;
	if ((handle = com_Init(port)) == NULL) 
	{
		printf("COM init fail!!\n");
		return;
	}
	
	com_SetBPS(handle, baud);
	com_SetFormat(handle, config);  
	com_FlushTxQueue(handle);
	com_FlushRxQueue(handle);
	com_SetTimeOut(handle, timeout);
	crossbar_ioaddr = sb_Read16(0x64)&0xfffe;
	if(port == COM1)
	{
		io_outpb(crossbar_ioaddr + COM1_TX, 0x08);
		io_outpb(crossbar_ioaddr + COM1_RX, 0x08);
	}
	else if(port == COM2)
	{
		io_outpb(crossbar_ioaddr + COM2_TX, 0x08);
		io_outpb(crossbar_ioaddr + COM2_RX, 0x08);
	}
	else if(port == COM3)
	{
		io_outpb(crossbar_ioaddr + COM3_TX, 0x08);
		io_outpb(crossbar_ioaddr + COM3_RX, 0x08);
	}
}

void HardwareSerial::end() {
	if(handle == NULL) return;
	com_FlushWFIFO(handle);
	com_Close(handle);
}

int HardwareSerial::available(void) {
    if(handle == NULL) return 0;
	return com_QueryRxQueue(handle);
} 

int HardwareSerial::peek(void) {
  	if(handle == NULL) return -1;
	if(peek_stored == true)
  		return peek_val;
  	else
  	{
  		if((peek_val = com_Read(handle)) == 0xFFFF)
  			peek_val = -1;
  		peek_stored = true;
		return peek_val;
	}
}

int HardwareSerial::read(void) {
	int c;
	if(handle == NULL) return -1;
	if(peek_stored == true)
	{
		peek_stored = false;
		return peek_val;
	}
	else
	{
		c = com_Read(handle);
		return (c == 0xFFFF) ? -1 : c;
	}
}

void HardwareSerial::flush() {
    if(handle == NULL) return;
	com_FlushTxQueue(handle);
}

size_t HardwareSerial::write(uint8_t c) {
	if(handle == NULL) return 0;
	return (com_Write(handle, c) == true) ? 1 : 0;
} 

HardwareSerial::operator bool() {
	return true;
} 

HardwareSerial Serial1(COM1, 115200L, BYTESIZE8|NOPARITY|STOPBIT1, 100L);
HardwareSerial Serial2(COM2, 115200L, BYTESIZE8|NOPARITY|STOPBIT1, 100L);
HardwareSerial Serial3(COM3, 115200L, BYTESIZE8|NOPARITY|STOPBIT1, 100L);
HardwareSerial Serial485(COM4, 115200L, BYTESIZE8|NOPARITY|STOPBIT1, 100L);
HardwareSerial Serial4(COM6, 115200L, BYTESIZE8|NOPARITY|STOPBIT1, 100L);
// Preinstantiate Objects //////////////////////////////////////////////////////
