#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <errno.h>
#include "process.h"
#define USE_COMMON
#include "common.h"
#include "usb.h"
#include "io.h"

void init(void) {
	int i;
	if(io_Init() == false)
	{
	  printf("Init fail\n");
	  return;
	}
	//set corssbar Base Address
	sb_Write16(0x64,0x0A01);
	//set GPIO Base Address
	sb_Write16(0x62,0xf101);
	// GPIO enable
	io_outpdw(0xf100,0x01ff);
	
	for(i=0;i<9;i++)
		io_outpdw(0xf100+(i+1)*4,((0xf202 + i*4)<<16)+0xf200 + i*4);
}

void EnableLLED(void) {
	io_outpb(0xf20e, io_inpb(0xf20e) | (1 << 1)); // L_LED dir -> OUTPUT
	io_outpb(0x0AA9, 0x01); // Crossbar -> GPIO
}

void DisableLLED(void) {
	io_outpb(0xf20e, io_inpb(0xf20e) & ~(1 << 1)); // L_LED dir -> INPUT
	io_outpb(0x0AA9, 0x08); // Crossbar -> PWM
}

void L_LED_OFF(void) {
    io_outpb(0xf20c, io_inpb(0xf20c) | (1 << 1));
}

void L_LED_ON(void) {
    io_outpb(0xf20c, io_inpb(0xf20c) & ~(1 << 1));
}

unsigned short LLEDPulse = 0;
void LEDPulse(void)
{
	LLEDPulse++;
	unsigned char p = LLEDPulse >> 8;
	if (p > 127)
		p = 254-p;
	p += p;
	if (((unsigned char)LLEDPulse) > p)
		L_LED_OFF();
	else
		L_LED_ON();
}

#define PRO_MODE    (0)
#define BTL_MODE    (1)
int GetBootLoaderMode(void) {
	int mode = 0;
	io_outpb(0x0A88, 0x01); // RICH-IO PORT -> GPIO
	io_outpb(0xf222, io_inpb(0xf222) & ~0x01); // GPIO -> INPUT
	mode = io_inpb(0xf220) & 0x01; // Read SPICS1(GPIO80) PIN
	io_outpb(0x0A88, 0x08); // GPIO -> RICH-IO PORT
	if(mode > 0)
		return PRO_MODE;
	else
		return BTL_MODE;
}

#define _SIZE    (100000L)
void* get_prospace(long pro_size) {
    void *p;
    unsigned long disk_freespace, memsize = _SIZE;
	struct dfree cdisk, adisk;
	
	// Get disk free space
	getdfree(1, &adisk); // Disk A
	getdfree(3, &cdisk); // Disk C
	if(cdisk.df_avail == 0)
		disk_freespace = (long) adisk.df_avail * (long) adisk.df_bsec * (long) adisk.df_sclus;
	else
		disk_freespace = (long) cdisk.df_avail * (long) cdisk.df_bsec * (long) cdisk.df_sclus;
	
	// Get memory free space
	while((p = ker_Malloc(memsize)) != NULL)
	{
		ker_Mfree((void*)p);
		memsize = memsize << 1;
	}
	
	if(memsize == _SIZE) return NULL;
	
	memsize = memsize >> 1;
	if(disk_freespace > memsize)
	{
		if(memsize >= pro_size)
			p = ker_Malloc(pro_size);
		else if(pro_size > memsize)
			return NULL;
	}
	else
	{
		if(disk_freespace >= pro_size)
			p = ker_Malloc(pro_size);
		else if(pro_size > disk_freespace)
			return NULL;
	}
	
	return p;
}

#define TIME_OUT    (8000)

#define PROGRAM     (1)
#define BOOTLOADER  (2)
int main(void)
{
	FILE* fp = NULL;
	int i, mode, type;
	unsigned long time;
	unsigned char rst_key = 0;
	long length = 0L, total_l = 0L, tmp = 0L;
	bool get_length = false, transfer = false;
	bool empty_program = false, reboot = false;
	unsigned char* start_p;
	unsigned char* p;
	
	/* For writing flash*/
	#define SECTION    (8192L) // write size 8k to disk every time
	unsigned long msize = 0L;
	
	// 0. Try to open 86duino.exe file in disk or flash
	fp = fopen("86duino.exe", "rb");
	if(fp != NULL) fclose(fp); else empty_program = true;
	
	init();
	mode = GetBootLoaderMode();
	
	// 0.5 Is pre-reset soft reset? if yes, go to bootloader
	if(fp != NULL)
	{
		rst_key = io_inpb(0xf21A);
		io_outpb(0xf21A, 0x00);
		if(rst_key != 0x5a && mode == PRO_MODE)
			return spawnl(P_OVERLAY, "86duino.exe", "86duino.exe", NULL);
	}
	
	// 1. Init USB device
	void* USBDEV = CreateUSBDevice();
	if(USBDEV == NULL)
		{printf("CreateUSBDevice() return NULL\n"); return -1;}
	usb_SetUSBPins(USBDEV, 7, 0, 7, 1);
	if(usb_Init(USBDEV) == false)
		{printf("Usb_Init() return NULL\n"); return -1;}
	usb_SetTimeOut(USBDEV, 2000);

	EnableLLED(); 
	// 2. main loop, timeout 8 sec
	time = timer_nowtime();
	while((timer_nowtime() - time) < TIME_OUT)
	{
		LEDPulse();
		// 2.1 If no 86duino.exe file in disk or flash, reading process without 8 sec limit until complete 
		if(empty_program == true) time = timer_nowtime();
		
		if(usb_Ready(USBDEV) == false || usb_QueryRxQueue(USBDEV) == 0) continue;
		
		if(get_length == false)
		{
			// 2.2 Get file size
		    if(usb_QueryRxQueue(USBDEV) >= 5)
		    {
		    	type = usb_Read(USBDEV);
		    	for(i=0; i<4; i++) length = length + ((long)usb_Read(USBDEV) << (8*i));
		    	//2.3 Get program memory size
				if ((p = (unsigned char*)get_prospace(length)) == NULL)
				{
					printf("ERROR: \'program_size\' alloc fail.\n");
					break;
				}
				else
				{
					get_length = true;
					total_l = length;
					start_p = p;
				}
			}
		}
		else
		{
			// 2.3 Put file to memory
			tmp = usb_QueryRxQueue(USBDEV);
			length = length - tmp;
			if(length <= 0)	transfer = true;
			for(i=0; i<tmp; i++)
			{
				*p = (unsigned char)usb_Read(USBDEV);
				p++;
				time = timer_nowtime();
			}
			if(transfer == true) break;				
		}
	}
	
	p = start_p;
	if(transfer == false) {usb_Write(USBDEV, '3'); goto END;}
	
	fp = NULL;	
	if(type == BOOTLOADER)
		fp = fopen("____86bt.$$$", "wb");
	else if(type == PROGRAM)
		fp = fopen("____86pg.$$$", "wb");

	if(fp == NULL) {usb_Write(USBDEV, '4'); goto END;}
		
	// 3. Write file from memory to disk or flash
	for(; total_l > 0L; total_l -= msize, start_p += msize)
	{
		msize = (total_l <= SECTION) ? total_l : SECTION;
		if(fwrite(start_p, sizeof(unsigned char), msize, fp) != msize)
		{
			switch(errno) // return error message
			{
			case EACCES: usb_Write(USBDEV, '5'); break;
			case EAGAIN: usb_Write(USBDEV, '6'); break;
			case EBADF:  usb_Write(USBDEV, '7'); break;
			case EFBIG:  usb_Write(USBDEV, '8'); break;
			case EINTR:  usb_Write(USBDEV, '9'); break;
			case EIO:    usb_Write(USBDEV, 'a'); break;
			case ENOMEM: usb_Write(USBDEV, 'b'); break;
			case ENOSPC: usb_Write(USBDEV, 'c'); break;
			case ENXIO:  usb_Write(USBDEV, 'd'); break;
			case EPIPE:  usb_Write(USBDEV, 'e'); break;
			default:     usb_Write(USBDEV, 'f'); break;
			}
			fclose(fp);
			goto END;
		}

		usb_Write(USBDEV, 'z');
		fflush(fp);
	}
    fclose(fp);
   
    // 3.5 switch filename
	if(type == BOOTLOADER)
	{
    	remove("v86boot.exe");
		rename("____86bt.$$$", "v86boot.exe");
    	usb_Write(USBDEV, '1');
		if((fp = fopen("86duino.exe", "rb")) != NULL)
			fclose(fp);
		else
			reboot = true;
    }
    else if(type == PROGRAM)
    {
    	if(empty_program == true)
    		rename("____86pg.$$$", "86duino.exe");
    	else
    	{
	    	remove("86duino.exe");
			rename("____86pg.$$$", "86duino.exe");
	    }
    	usb_Write(USBDEV, '2');
	}
	
END: // 4. prepare to run user program
	delay_ms(100);
	ker_Mfree((void*)p);
	usb_Close(USBDEV);
	DisableLLED();
	
	if(reboot == true)
		io_outpb(0x64, 0xfe); //reset CPU;
	else
		return spawnl(P_OVERLAY, "86duino.exe", "86duino.exe", NULL);
}

