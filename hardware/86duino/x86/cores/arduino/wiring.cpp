/*
  Copyright (c) 2013 Android Lin <acen@dmp.com.tw>. All right reserved.

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
*/

#include "Arduino.h"
#include "io.h"
#include "USBCore.h"
#include "mcm.h"
#include "irq.h"
#include "stdio.h"

void *USBDEV = NULL;
unsigned long CLOCKS_PER_MICROSEC = 300L; // The default value is 300Mhz for 86Duino, you should call init() to set it automatically.
unsigned long VORTEX86EX_CLOCKS_PER_MS = 300000L; // The default value is 300000 for 86Duino, you should call init() to set it automatically.
bool Global_irq_Init = false;

extern void setPinStatus(void);

unsigned long millis() {
	return timer_NowTime();
}

void delay(unsigned long ms) {
	timer_Delay(ms);
}

// For 86Duino IDE, only use micros() after calling init()
unsigned long micros() {
	return (unsigned long) (timer_GetClocks64()/CLOCKS_PER_MICROSEC);
}

void delayMicroseconds(unsigned long us) {
    timer_DelayMicroseconds(us);
}

bool init() {
	int i, crossbarBase, gpioBase;
	if(io_Init() == false) return false;
    timer_NowTime(); // initialize timer
    CLOCKS_PER_MICROSEC = vx86_CpuCLK();
    VORTEX86EX_CLOCKS_PER_MS = CLOCKS_PER_MICROSEC*1000UL;
    
    // Set IRQ4 as level-trigger
    io_outpb(0x4D0, io_inpb(0x4D0) | 0x10);
    
	//set corssbar Base Address
	crossbarBase = sb_Read16(SB_CROSSBASE) & 0xfffe;
	if(crossbarBase == 0 || crossbarBase == 0xfffe)
	{
		sb_Write16(SB_CROSSBASE, CROSSBARBASE | 0x01);
		crossbarBase = CROSSBARBASE;
	}
	
	#if defined CRB_DEBUGMODE
	for(i=0; i<CRBTABLE_SIZE; i++) io_outpb(crossbarBase+i, CROSSBARTABLE[i]);
	#endif
	
	// Force set HIGH speed ISA on SB
	sb_Write(SB_FCREG, sb_Read(SB_FCREG) | 0x8000C000L);
	
	//set SB GPIO Base Address
	gpioBase = sb_Read16(SB_GPIOBASE) & 0xfffe;
	if(gpioBase == 0 || gpioBase == 0xfffe)
	{
		sb_Write16(SB_GPIOBASE, GPIOCTRLBASE | 0x01);
		gpioBase = GPIOCTRLBASE;
	}
	
	// Enable GPIO 0 ~ 7 
	io_outpdw(gpioBase, 0x00ff);
	
	// set GPIO Port 0~7 dircetion & data Address
	for(i=0;i<8;i++)
		io_outpdw(gpioBase + (i+1)*4,((GPIODIRBASE + i*4)<<16) + GPIODATABASE + i*4);

	setPinStatus();
	
	// set ADC Base Address
	sb_Write(0xbc, sb_Read(0xbc) & (~(1L<<28)));  // active adc
	sb1_Write16(0xde, sb1_Read16(0xde) | 0x02);   // not Available for 8051A Access ADC
	sb1_Write(0xe0, 0x0050fe00L); // baseaddr = 0xfe00, disable irq
	io_outpb(0xfe01, 0x00);
    for(i=0; (io_inpb(0xfe02) & 0x01) != 0 && i<16; i++) io_inpb(0xfe04); // clear ADC FIFO

	// set MCM Base Address
	set_MMIO();
	mcmInit();
	for(i=0; i<4; i++)
		mc_SetMode(i, MCMODE_PWM_SIFB);

	// init wdt1
	wdt_init();

	if(Global_irq_Init == false)
	{
		// set MCM IRQ
		if(irq_Init() == false) 
	    {
	        printf("MCM IRQ init fail\n"); return false;
	    }
	    
	    if(irq_Setting(GetMCIRQ(), IRQ_LEVEL_TRIGGER + IRQ_DISABLE_INTR) == false)
	    {
	        printf("MCM IRQ Setting fail\n"); return false;
	    }
	    Set_MCIRQ(GetMCIRQ());
	    Global_irq_Init = true;
	}
    
	//CDC
	USBDEV = CreateUSBDevice();
	if(USBDEV == NULL)
	{
		printf("init error\n");
		return false;
	}
    
	usb_SetUSBPins(USBDEV, 7, 0, 7, 1);
	usb_SetTimeOut(USBDEV, 0L, 500L); // USB RX timerout is 0ms and TX timeout is 500ms
	if(usb_Init(USBDEV) == false)
	{
		printf("init2 error\n");
		return false;
	}

	//io_Close();
	return true;
}

/*
// only for 86Duino PLC board
unsigned short getDeviceID(void) {
	unsigned char mask = 0xC3;
	int port = 7;
	unsigned char data;
	data = io_inpb(GPIODIRBASE + port*4) & mask;
	io_outpb(GPIODIRBASE + port*4, data); // switch GP72, GP73, GP74, GP75 to input direction
	return (io_inpb(GPIODATABASE + port*4) >> 2) & 0x0F;
}
*/
