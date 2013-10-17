#include "Arduino.h"
#include "io.h"
#define USE_COMMON
#include "common.h"
#include "usb.h"

void *USBDEV = NULL;
unsigned long millis() {
  return timer_nowtime();
}

void delay(unsigned long ms) {
	delay_ms(ms);
}

unsigned long long int micros() {
	#define CLOCKS_PER_MICROSEC (299UL)
	return (getclocks64()/CLOCKS_PER_MICROSEC);
}

void delayMicroseconds(unsigned long us) {
    delay_us(us);
}

bool init() {
	int i;
	if(io_Init() == false) return false;

	//set corssbar Base Address
	sb_Write16(0x64,0x0A01);
	//printf("SB C0~C3 REG: 0x%08X\n", sb_Read(0xc0));
	sb_Write(0xc0, sb_Read(0xc0) | 0x8000C000L);
	//printf("SB C0~C3 REG: 0x%08X\n", sb_Read(0xc0));
	//set GPIO Base Address
	sb_Write16(0x62,0xf101);
	// GPIO enable
	io_outpdw(0xf100,0x00ff);
	//set GPIO P0~9 dircetion & data Address
	//io_outpdw(0xf100,0x03ff);
	for(i=0;i<8;i++) {
		io_outpdw(0xf100 + (i+1)*4,((0xf202 + i*4)<<16) + 0xf200 + i*4);
		//io_outpb((sb_Read16(0x64)&0xfffe)+i,0x00);
	}
	  
	//setADC Base Address
	sb_Write(0xbc, sb_Read(0xbc) & (~(1L<<28)));  // active adc
	sb1_Write16(0xde, sb1_Read16(0xde) | 0x02);   // not Available for 8051A Access ADC
	sb1_Write(0xe0, 0x0010fe00L); // baseaddr = 0xfe00, disable irq
	
	
	//CDC
	USBDEV = CreateUSBDevice();
    if(USBDEV == NULL)
    {
        printf("init error\n");
        return false;
    }
    usb_SetUSBPins(USBDEV, 7, 0, 7, 1);
    if(usb_Init(USBDEV) == false)
    {
        printf("init2 error\n");
        return false;
    }
	
	//io_Close();
	return true;
}
