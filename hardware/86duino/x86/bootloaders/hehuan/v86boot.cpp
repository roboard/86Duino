#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <errno.h>
#include <string.h>
#include "process.h"
#define USE_COMMON
#include "common.h"
#include "usb.h"
#include "io.h"

// #define DEBUG_MODE
#define __EDUCAKE
// #define __ONE_ZERO

/***************************  Bootloader Functions ****************************/
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

#if defined __ONE_ZERO
	io_outpb(0x0A88, 0x01); // RICH-IO PORT -> GPIO
	io_outpb(0xf222, io_inpb(0xf222) & ~0x01); // GPIO -> INPUT
	mode = io_inpb(0xf220) & 0x01; // Read SPICS1(GPIO80) PIN
	io_outpb(0x0A88, 0x08); // GPIO -> RICH-IO PORT
#elif defined __EDUCAKE
	if((io_inpb(0x2ee) & 0x40) == 0) mode = 1;
#endif

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


/*
+------------------------+
| Package size | command | 
+------------------------+
*/

// the size of member in the package
#define OLDCOMM_BYTES         (1)
#define PSIZE_BYTES           (4)
#define COMM_BYTES            (2)
#define FNAME_BYTES           (32)
#define FSIZE_BYTES           (4)
#define LAST_BYTES            (1)
#define CHSUM_BYTES           (1)
#define ERRCODE_BYTES         (2)
#define VERSION_BYTES         (32)
#define WFDONE_BYTES          (9)
#define VSDONE_BYTES          (41)

// command
#define GET_BIOS_VER          (0x0010)
#define GET_BOOTLOADER_VER    (0x0011)
#define BURN_BOOTLOADER       (0x0012)
#define BURN_BIOS             (0x0013)
#define BURN_PROGRAM          (0x0014)
#define BURN_VERSIONFILE      (0x0015)

// Error code
#define EMALLOC               (0x0001)
#define ECOMMAND              (0x0002)
#define EFILENAME             (0x0004)
#define EFILELENGTH           (0x0008)
#define EOPENFILE             (0x0010)
#define EWRITEFILE            (0x0020)
#define ELAST                 (0x0040)
#define ECHECKSUM             (0x0080)

// bootloader state
#define GETPSZIE              (0x50)
#define GETPACKAGE            (0x51)
#define GETCOMMAND            (0x52)
#define ANALYSIS              (0x53)
#define GETFILENAME           (0x54)
#define GETFILESIZE           (0x55)
#define WRITEFILE             (0x56)
#define LASTSTEP              (0x57)
#define SENDRESULT            (0x58)
#define WRITEFILEDONE         (0x59)
#define GETVERSION            (0x5A)
#define GETVERSIONDONE        (0x5B)
#define BURN86BIOS            (0x5C)

// isLast flag
#define YES                   (0x01)
#define NO                    (0x02)

// CMOS
#define VERBITS               (32)
#define BTLRADDR              (147)
#define BIOSADDR              (150)

unsigned char Cal_checksum(unsigned char* cch, unsigned long packagesize) {
	unsigned long i;
	unsigned char checksum = 0;
	for(i=0; i<(packagesize - 1); i++)
		checksum = checksum + cch[i];
	checksum = ~(checksum);
	return checksum;
}


#define TIME_OUT    (8000)
const char* bootloader_ver = "Hehuan 1.0";
const char* bios_ver       = "Beta 0.9";
static char version[VERSION_BYTES] = {'\0'};
int main(void) {
	FILE* fp = NULL;
	int i, j, mode;
	bool old_blversion = false;
	long length = 0L, total_l = 0L, tmp = 0L;
	bool get_length = false, transfer = false, empty_program = false, reboot = false;
	unsigned long time;
	unsigned char rst_key = 0, old_command = 0;
	unsigned char* pstart_p, *package_p;
	
	// the variables for package
	char filename[FNAME_BYTES];
	unsigned long packageSize = 0L, ps = 0L, filesize = 0L;
	unsigned short command  = 0, errorcode = 0;
	unsigned char isLast = YES, checksum = 0;
	
	// for writing flash
	#define SECTION    (8192L) // write size 8k to disk every time
	unsigned long msize = 0L;

	/*
	// detect bootloader version
	for(i=0; i<VERBITS; i++) {version[i] = read_cmos(BTLRADDR+i); version[i] = '\0';}
	if(strcmp(version, bootloader_ver) != 0)
		for(i=0; i<VERBITS; i++) write_cmos(BTLRADDR+i, bootloader_ver[i]);
	
	// detect bios version
	for(i=0; i<VERBITS; i++) {version[i] = read_cmos(BIOSADDR+i); version[i] = '\0';}
	if(strcmp(version, bios_ver) != 0)
		for(i=0; i<VERBITS; i++) write_cmos(BIOSADDR+i, bios_ver[i]);
	*/
	 
	// Try to open 86duino.exe file in disk or flash
	fp = fopen("_86duino.exe", "rb");
	if(fp != NULL)
		fclose(fp);
	else
		empty_program = true;
	
	init();
	mode = GetBootLoaderMode();
	
	// Is pre-reset soft reset? if yes, go to bootloader
	rst_key = io_inpb(0xf21A);
	io_outpb(0xf21A, 0x00);
	if(fp != NULL && rst_key != 0x5a && mode == PRO_MODE)
		return spawnl(P_OVERLAY, "_86duino.exe", "_86duino.exe", NULL);
	
	// Init USB device
	void* USBDEV = CreateUSBDevice();
	if(USBDEV == NULL)
	{
		printf("CreateUSBDevice() return NULL\n");
		return -1;
	}
	usb_SetUSBPins(USBDEV, 7, 0, 7, 1);
	if(usb_Init(USBDEV) == false)
	{
		printf("Usb_Init() return NULL\n");
		return -1;
	}
    usb_SetTimeOut(USBDEV, 2000L);

    
	int _prev, _cur, bstate;
	EnableLLED();
	time = timer_nowtime();
	bstate = GETPSZIE;
	_prev = _cur = 0; // QueryRXQuere counter
	while((timer_nowtime() - time) < TIME_OUT)
	{
		LEDPulse();
		// If no _86duino.exe file in disk or flash, reading process without 8 sec limit until complete 
		if(empty_program == true) time = timer_nowtime();
		
		if(usb_Ready(USBDEV) == false) continue;
		
		_cur = usb_QueryRxQueue(USBDEV); // re-count if the value is changed
		if(_cur != _prev)
		{
			time = timer_nowtime();
			_prev = _cur;
		}
		
		switch(bstate)
		{
		case GETPSZIE:
			if(usb_QueryRxQueue(USBDEV) >= (OLDCOMM_BYTES + PSIZE_BYTES))
		    {
		    	old_command = usb_Read(USBDEV); // for version 090 uploader
				if(old_command == 0xff) usb_Write(USBDEV, 'H'); 
				for(i=0, packageSize=0L; i<PSIZE_BYTES; i++)
					packageSize = packageSize + ((unsigned long)usb_Read(USBDEV)<< (8*i));
				packageSize &= 0x0fffffffL;
#if defined DEBUG_MODE
				printf("package size = %ld\n", packageSize);
#endif
				ps = packageSize;
				// malloc memory space for package
				package_p = (unsigned char*)get_prospace(packageSize);
				if(package_p == NULL)
				{
#if defined DEBUG_MODE
					printf("ERROR: \'package size\' malloc fail.\n");
#endif
					errorcode |= EMALLOC;
					bstate = SENDRESULT;
					break;
				}
				pstart_p = package_p;
				
				// for version 090 uploader
				if(old_command == 0x01 || old_command == 0x02)
				{
					if(old_command == 0x01) command = BURN_PROGRAM; else command = BURN_BOOTLOADER;
					total_l = packageSize;
					old_blversion = true;
				}
				
				bstate = GETPACKAGE;
			}
			break;
		case GETPACKAGE:
			if(transfer == false)
		    {
				tmp = usb_QueryRxQueue(USBDEV);
				ps = ps - tmp;
				if(ps == 0L) transfer = true;
				for(i=0; i<tmp; i++)
				{
					*package_p = (unsigned char)usb_Read(USBDEV);
					package_p++;
				}
			}
			else
			{
				transfer = false;
				package_p = pstart_p;
				if(old_blversion == true)
					bstate = WRITEFILE;
				else
					bstate = GETCOMMAND;
			}
			break;
		case GETCOMMAND:
			for(i=0, command=0; i<COMM_BYTES; i++)
			{
				command = command + ((unsigned short)(*package_p) << (8*i));
				package_p++;
			}
#if defined DEBUG_MODE
			printf("command = %d\n", command);
#endif
			package_p = pstart_p;
			bstate = ANALYSIS;
			break;
		case ANALYSIS:
			switch(command)
			{
			case GET_BIOS_VER:    case GET_BOOTLOADER_VER:
			case BURN_BOOTLOADER: case BURN_BIOS:
			case BURN_PROGRAM:    case BURN_VERSIONFILE:
				// Calculate checksum
				for(length=0, checksum=0; length < (packageSize-1); length++)
				{
					checksum += *package_p;
					package_p++;
				}
				checksum = ~checksum;
#if defined DEBUG_MODE
				printf("checksum = %02X *package_p = %02X\n", checksum, *package_p);
#endif
				if(checksum != *package_p)
				{
					ker_Mfree((void*)pstart_p);
					errorcode |= ECHECKSUM;
					bstate = SENDRESULT;
					break;
				}
				package_p--;
				isLast = *package_p;
				package_p = pstart_p + COMM_BYTES;
				if(command == GET_BIOS_VER || command == GET_BOOTLOADER_VER)
					bstate = GETVERSION;
				else if(command == BURN_BIOS)
					bstate = BURN86BIOS;
				else
					bstate = GETFILENAME;
				break;
			default:
#if defined DEBUG_MODE
				printf("Receive a unavailable command\n");
#endif
				ker_Mfree((void*)pstart_p);
				errorcode |= ECOMMAND;
				bstate = SENDRESULT;
				break;
			}
			break;
		case GETFILENAME:
			for(i=0; i<FNAME_BYTES; i++)
			{
				filename[i] = (char)(*package_p);
				package_p++;
			}
			for(i=0; i<FNAME_BYTES; i++)
				if(filename[i] == '\0') break;
			if(i == FNAME_BYTES)
			{
				ker_Mfree((void*)pstart_p);
				errorcode |= EFILENAME;
				bstate = SENDRESULT;
				break;
			}
#if defined DEBUG_MODE
			printf("File name = %s\n", filename);
#endif
			bstate = GETFILESIZE;
			break;
		case GETFILESIZE:
			for(i=0, filesize=0L; i<FSIZE_BYTES; i++)
			{
				filesize = filesize + ((unsigned long)(*package_p) << (8*i));
				package_p++;
			}
#if defined DEBUG_MODE
			printf("File size = %ld\n", filesize);
#endif
			total_l = filesize;
			bstate = WRITEFILE;
			break;
		case WRITEFILE:
			fp = fopen("___86tmp.$$$", "wb");
			if(fp == NULL)
			{
				ker_Mfree((void*)pstart_p);
				errorcode |= EOPENFILE;
				bstate = SENDRESULT;
				break;
			}

			for(; total_l > 0L; total_l -= msize, package_p += msize)
			{
				msize = (total_l <= SECTION) ? total_l : SECTION;
				if(fwrite(package_p, sizeof(unsigned char), msize, fp) != msize)
				{
					ker_Mfree((void*)pstart_p);
					errorcode |= EWRITEFILE;
					bstate = SENDRESULT;
					break;
				}
				usb_Write(USBDEV, 'z');
				fflush(fp);
			}
		    fclose(fp);
		    
		    if(command == BURN_BOOTLOADER)
			{
		    	remove("_v86boot.exe");
				rename("___86tmp.$$$", "_v86boot.exe");
				if((fp = fopen("_86duino.exe", "rb")) != NULL)
					fclose(fp);
				else
					reboot = true;
		    }
		    else if(command == BURN_PROGRAM)
		    {
		    	if(empty_program == false)
		    		remove("_86duino.exe");
				rename("___86tmp.$$$", "_86duino.exe");
			}
			else
			{
				if((fp = fopen(filename, "r")) != NULL)
				{
					fclose(fp);
					remove(filename);
				}
				rename("___86tmp.$$$", filename);
			}
			
		    time = timer_nowtime(); // reset timer
		    ker_Mfree((void*)pstart_p);
			usb_Write(USBDEV, '1');
			if(old_blversion == true)
				goto END; // we don't care "isLast" flag and result package for old bootloader
			else
				bstate = SENDRESULT;
			break;
        case SENDRESULT:
        	package_p = (unsigned char*) ker_Malloc(WFDONE_BYTES);
        	packageSize = COMM_BYTES + ERRCODE_BYTES + CHSUM_BYTES;
        	for(i=0, j=0; j<PSIZE_BYTES; i++, j++)
				package_p[i] = (packageSize & (0xff << (8*j))) >> (8*j);
			for(j=0; j<COMM_BYTES; i++, j++)
				package_p[i] = (command & (0xff << (8*j))) >> (8*j);
			for(j=0; j<ERRCODE_BYTES; i++, j++)
				package_p[i] = (errorcode & (0xff << (8*j))) >> (8*j);
        	package_p[i] = Cal_checksum(package_p+PSIZE_BYTES, packageSize);
        	
        	usb_Send(USBDEV, package_p, WFDONE_BYTES);
        	
        	ker_Mfree((void*)package_p);
        	if(errorcode == 0 && isLast == NO)
        	{
        		time = timer_nowtime();
        		bstate = GETPSZIE;
        	}
			else
				goto END;
        	break;
		case GETVERSION:
			if(command == GET_BOOTLOADER_VER)
			{
				for(i=0; i<VERBITS; i++)
				{
					if(bootloader_ver[i] != '\0')
						version[i] = bootloader_ver[i];
					else
						break;
				}
				version[i] = '\0';
			}
			else if(command == GET_BIOS_VER)
			{
				fp = fopen("_bver.v86", "r");
				if(fp == NULL)
				{
					for(i=0; i<VERBITS; i++)
					{
						if(bios_ver[i] != '\0')
							version[i] = bios_ver[i];
						else
							break;
					}
					version[i] = '\0';
				}
				else
				{
					fgets(version, VERSION_BYTES, fp);
					fclose(fp);
				}
			}
			bstate = GETVERSIONDONE;
		    break;
		case GETVERSIONDONE:
			package_p = (unsigned char*) ker_Malloc(VSDONE_BYTES);
        	packageSize = COMM_BYTES + VERSION_BYTES + ERRCODE_BYTES + CHSUM_BYTES;
        	for(i=0, j=0; j<PSIZE_BYTES; i++, j++)
				package_p[i] = (packageSize & (0xff << (8*j))) >> (8*j);
			for(j=0; j<COMM_BYTES; i++, j++)
				package_p[i] = (command & (0xff << (8*j))) >> (8*j);
			for(j=0; j<VERSION_BYTES; i++, j++)
				package_p[i] = version[j];
			for(j=0; j<ERRCODE_BYTES; i++, j++)
				package_p[i] = (errorcode & (0xff << (8*j))) >> (8*j);
        	package_p[i] = Cal_checksum(package_p+PSIZE_BYTES, packageSize);
        	
        	usb_Send(USBDEV, package_p, VSDONE_BYTES);
        	
        	ker_Mfree((void*)package_p);
        	if(errorcode == 0 && isLast == NO)
        	{
        		time = timer_nowtime();
        		bstate = GETPSZIE;
        	}
			else
				goto END;
			break;
		case BURN86BIOS: // bootloader: anybios.exe -> coreboot.rom -> burn it -> versoin file
			fp = fopen("anybios.exe", "r");
			if(fp == NULL)
			{
				errorcode |= EOPENFILE;
				bstate = SENDRESULT;
				break;
			}
			fclose(fp);
			fp = fopen("coreboot.rom", "r");
			if(fp == NULL)
			{
				errorcode |= EOPENFILE;
				bstate = SENDRESULT;
				break;
			}
			fclose(fp);
			
			system("anybios.exe w coreboot.rom");
			remove("anybios.exe");
			remove("coreboot.rom");
			bstate = SENDRESULT;
			break;
		} // switch (bstate)...
	} // while((timer_nowtime() ...
	
	
END: // prepare to run user program
	delay_ms(100);
	usb_Close(USBDEV);
	DisableLLED();
	
	if(reboot == true)
		io_outpb(0x64, 0xfe); //reset CPU;
	else
		return spawnl(P_OVERLAY, "_86duino.exe", "_86duino.exe", NULL);
}

