#include "Arduino.h"
#include "io.h"
#define PULL_DOWN     (0x02)
#define PULL_UP       (0x01)

int crossbar_bit;
static unsigned short crossbar_ioaddr = 0;

void pinMode(uint8_t pin, uint8_t mode) { 
	if(pin >= PINS) return;
	if(crossbar_ioaddr == 0) crossbar_ioaddr = sb_Read16(0x64)&0xfffe;
	if(crossbar_ioaddr == 0 || crossbar_ioaddr == 0xff) return;

	crossbar_bit = pinMap[pin];
	
	if(crossbar_bit <= 31)
		io_outpb(crossbar_ioaddr + 0x90 + crossbar_bit, 0x01);
	else
		io_outpb(crossbar_ioaddr + 0x80 + (crossbar_bit/8), 0x01);
	
	if (mode == INPUT)
	{
	    io_outpb(crossbar_ioaddr + 0x30 + pinMap[pin], PULL_DOWN);
	    io_outpb(0xf202 + 4*(crossbar_bit/8), io_inpb(0xf202 + 4*(crossbar_bit/8))&~(1<<(crossbar_bit%8)));   
	}
	else if (mode == INPUT_PULLUP)
	{ 
	    io_outpb(crossbar_ioaddr + 0x30 + pinMap[pin], PULL_UP);
	    io_outpb(0xf202 + 4*(crossbar_bit/8), io_inpb(0xf202 + 4*(crossbar_bit/8))&~(1<<(crossbar_bit%8)));
	}     
	else
	    io_outpb(0xf202 + 4*(crossbar_bit/8), io_inpb(0xf202 + 4*(crossbar_bit/8))|(1<<(crossbar_bit%8)));      
}

void digitalWrite(uint8_t pin, uint8_t val) {
    unsigned int port;
    unsigned int value;
	if(pin >= PINS) return;
    
	crossbar_bit = pinMap[pin];
    port = 0xf200 + 4*(crossbar_bit/8);
    value = 1<<(crossbar_bit%8);
    
	if((crossbar_ioaddr + 0x90 + crossbar_bit) != 0x01)
	{
		Close_Pwm(pin);
		io_outpb(crossbar_ioaddr + 0x90 + crossbar_bit, 0x01);
    }

    io_DisableINT();
    if (val == LOW) 
        io_outpb(port, io_inpb(port)&(~value));
    else
        io_outpb(port, io_inpb(port)|value);
    io_RestoreINT();
}

int digitalRead(uint8_t pin) {
	if(pin >= PINS) return 0xffff;
	
	crossbar_bit = pinMap[pin];
	
	if((crossbar_ioaddr + 0x90 + crossbar_bit) != 0x01)
	{
		Close_Pwm(pin);
		io_outpb(crossbar_ioaddr + 0x90 + crossbar_bit, 0x01);
	}  
	
	if(io_inpb(0xf200 + 4*(crossbar_bit/8))&(1<<(crossbar_bit%8))) return HIGH;
	return LOW;
}
