#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "process.h"
#include "com.h"
#include "io.h"
#include "irq.h"
#include "dpmi.h"

// #define DEBUG_MODE
// #define __ONE_ZERO
// #define __EDUCAKE
// #define __AI
// #define __DUO
// #define __MOTION_9D
#define __3DP_COLOR

#if defined (__DUO) || defined (__MOTION_9D) || defined (__3DP_COLOR)
	#define HYB_BASEADDRESS     (0x00008A00)
	#define CROSSBARBASE        (0x00008000)
	#define NB_GPIO_CBA_REG 	(0xB0)
	#define NB_GPIO_INT_CBA_REG (0xB4)
	#define CONFIG_BASEADDR     (0x0D00)
	#define GPIOCTRLBASE        (0x0DC0)
	#define GPIODATABASE        (0x0DA0)
	#define GPIODIRBASE         (0x0DB0)
#elif defined (__ONE_ZERO) || defined (__EDUCAKE) || defined (__AI)
    #define SB_CROSSBASE  (0x64)
	#define CROSSBARBASE  (0x0A00)
	#define GPIOCTRLBASE  (0xF100)
	#define GPIODATABASE  (0xF200)
	#define GPIODIRBASE   (0xF202)
#endif

#define SB_FCREG      (0xC0)
#define SB_GPIOBASE   (0x62)
    
#define PRO_MODE    (0)
#define BTL_MODE    (1)

#if defined (__ONE_ZERO) || defined (__EDUCAKE) || defined (__AI)
    #define LED_MAX_COUNT    (100) // Under FastSpeed, the SOF interrupt interval is 1ms, so LED_MAX_COUNT is 100.
#elif defined (__DUO) || defined (__MOTION_9D) || defined (__3DP_COLOR)
	#define LED_MAX_COUNT    (800) // Under HiSpeed, the SOF interrupt interval is 125us, so LED_MAX_COUNT is 800.
#endif

#define TIME_OUT         (8000)
#define SECTION          (8192L) // write size 8k to disk every time
#define RESERVEDSPACE    (262144) // 256KB

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
#define BURN_OTHERFILE        (0x0015)
#define BURN_TOOL_FIRMWARE    (0x0016)
#define GET_BOARD_NAME        (0x0017)
#define BOOTLOADER_END        (0x0018)
#define RUN_TOOL_FIRMWARE     (0x0019)
#define DEL_TOOL_FIRMWARE     (0x001A)
#define GET_BOOTLOADER_NAME   (0x001B)
#define RUN_86ME_HANDSHANKE   (0x001C)
#define GET_86ME_FW_VERSION   (0x001D)

// Error code
#define EMALLOC               (0x0001)
#define ECOMMAND              (0x0002)
#define EFILENAME             (0x0004)
#define EFILELENGTH           (0x0008)
#define EOPENFILE             (0x0010)
#define EWRITEFILE            (0x0020)
#define ELAST                 (0x0040)
#define ECHECKSUM             (0x0080)
#define ENOTOOLFW             (0x0100)

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
#define GETBOARDNAME          (0x5D)
#define BOOTLOADEREND         (0x5E)

// isLast flag
#define YES                   (0x01)
#define NO                    (0x02)

// CMOS
#define VERBITS               (32)
#define BTLRADDR              (147)
#define BIOSADDR              (150)


COMPort *handle = NULL;
bool Global_irq_Init = false;

unsigned long CLOCKS_PER_MICROSEC = 400L; // The default value is 400Mhz for 86Duino slave, you should call init() to set it automatically.
unsigned long VORTEX86EX_CLOCKS_PER_MS = 400000L; // The default value is 300000 for 86Duino, you should call init() to set it automatically.

unsigned short DIR_BA[16] = {0x0DB0, 0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5, 0x0DB6, 0x0DB7, 
							 0x0DB8, 0x0DB9, 0x0DBA, 0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0DBF};
unsigned short DATA_BA[16] = {0x0DA0, 0x0DA1, 0x0DA2, 0x0DA3, 0x0DA4, 0x0DA5, 0x0DA6, 0x0DA7, 
							  0x0DA8, 0x0DA9, 0x0DAA, 0x0DAB, 0x0DAC, 0x0DAD, 0x0DAE, 0x0DAF};
unsigned short CONTROL_BA[16] = {0x0DC0, 0x0DC4, 0x0DC8, 0x0DCC, 0x0DD0, 0x0DD4, 0x0DD8, 0x0DDC, 
							     0x0DE0, 0x0DE4, 0x0DE8, 0x0DEC, 0x0DF0, 0x0DF4, 0x0DF8, 0x0DFC};

static int tx_led_count = 0;
static int rx_led_count = 0;

static unsigned short gpio_config_addr = 0x0000;
static unsigned short tx_led_data  = 0x0000;
static unsigned short tx_led_dir   = 0x0000;
static unsigned short rx_led_data  = 0x0000;
static unsigned short rx_led_dir   = 0x0000;
static char tx_led_pin = 0;
static char rx_led_pin = 0;

static void set_gpio_config_addr(void)
{
#if defined (__ONE_ZERO) || defined (__EDUCAKE) || defined (__AI)
    unsigned long value;
    
    value = sb_Read16(0x62) & 0xFFFE;
    if (!value)
        gpio_config_addr = GPIOCTRLBASE;
    else
        gpio_config_addr = value;
    value = gpio_config_addr | 0x01;
    sb_Write16(0x62, value);
#elif defined (__DUO) || defined (__MOTION_9D) || defined (__3DP_COLOR)
    gpio_config_addr = CONFIG_BASEADDR;
#endif
}

static void set_tx_led(char port, char pin)
{
    io_outpdw(gpio_config_addr + 0x00, io_inpdw(gpio_config_addr + 0x00) | 0x01L << port);
    
    // set data address
    if ((tx_led_data = io_inpw(gpio_config_addr + (4 + 4*port))) == 0x0000)
    #if defined (__ONE_ZERO) || defined (__EDUCAKE) || defined (__AI)
        io_outpw(gpio_config_addr + (4 + 4*port), (tx_led_data = (GPIODATABASE + port*4)));
    #else
        io_outpw(gpio_config_addr + (4 + 4*port), (tx_led_data = (GPIODATABASE + port)));
    #endif
    // set direction address
    if ((tx_led_dir  = io_inpw(gpio_config_addr + (6 + 4*port))) == 0x0000)
    #if defined (__ONE_ZERO) || defined (__EDUCAKE) || defined (__AI)
        io_outpw(gpio_config_addr + (6 + 4*port), (tx_led_dir = (GPIODIRBASE + port*4)));
    #else
        io_outpw(gpio_config_addr + (4 + 4*port), (tx_led_dir = (GPIODIRBASE + port)));
    #endif
    // set TX LED pin -> OUT
    io_outpb(tx_led_dir, io_inpb(tx_led_dir) | (1 << pin));
    tx_led_pin = pin;
}

static void set_rx_led(char port, char pin)
{
    io_outpdw(gpio_config_addr + 0x00, io_inpdw(gpio_config_addr + 0x00) | 0x01L << port);
    
    // set data address
    if ((rx_led_data = io_inpw(gpio_config_addr + (4 + 4*port))) == 0x0000)
    #if defined (__ONE_ZERO) || defined (__EDUCAKE) || defined (__AI)
        io_outpw(gpio_config_addr + (4 + 4*port), (rx_led_data = (GPIODATABASE + port*4)));
    #else
        io_outpw(gpio_config_addr + (4 + 4*port), (rx_led_data = (GPIODATABASE + port)));
    #endif
    // set direction address
    if ((rx_led_dir  = io_inpw(gpio_config_addr + (6 + 4*port))) == 0x0000)
    #if defined (__ONE_ZERO) || defined (__EDUCAKE) || defined (__AI)
        io_outpw(gpio_config_addr + (6 + 4*port), (rx_led_dir = (GPIODIRBASE + port*4)));
    #else
        io_outpw(gpio_config_addr + (6 + 4*port), (rx_led_dir = (GPIODIRBASE + port)));
    #endif
    // set RX LED pin -> OUT
    io_outpb(rx_led_dir, io_inpb(rx_led_dir) | (1 << pin));
    rx_led_pin = pin;
}

static void TX_LED_ON(void)
{
    io_outpb(tx_led_data, io_inpb(tx_led_data) & ~(1 << tx_led_pin));
    tx_led_count = LED_MAX_COUNT;
}

static void RX_LED_ON(void)
{
    io_outpb(rx_led_data, io_inpb(rx_led_data) & ~(1 << rx_led_pin));
    rx_led_count = LED_MAX_COUNT;
}

static void TX_LED_OFF(void)
{
    if (tx_led_data == 0) return;
    io_outpb(tx_led_data, io_inpb(tx_led_data) | (1 << tx_led_pin));
}

static void RX_LED_OFF(void)
{
    if (rx_led_data == 0) return;
    io_outpb(rx_led_data, io_inpb(rx_led_data) | (1 << rx_led_pin));
}

void _86Duino_RXLED_ON_callback(void) {
    RX_LED_ON();
}

void _86Duino_TXLED_ON_callback(void) {
    TX_LED_ON();
}

void _86Duino_LED_OFF_callback(void) {
    if(tx_led_count && !(--tx_led_count)) TX_LED_OFF();
    if(rx_led_count && !(--rx_led_count)) RX_LED_OFF();
}

void _86Duino_suspend_callback(void) {
    TX_LED_OFF();
    RX_LED_OFF();
}

void _86Duino_reset_callback(void) {
    if (handle == NULL) return;
    
    // the below behavier is only for compatible of Arduino Leonado (windows)
    if((com_GetBaud(handle) == 1200) && (com_GetDTR(handle) == 0))
    {   
        io_DisableINT();
        com_Disconnect(handle); // io_outpb(usb_on_off_data, io_inpb(usb_on_off_data) | (1 << usb_on_off_pin));
        com_Close(handle);
    #if defined (__ONE_ZERO) || defined (__EDUCAKE) || defined (__AI)
        io_outpb(0xf21A, 0x5A); // write soft reset key
    #elif defined (__DUO) || defined (__MOTION_9D) || defined (__3DP_COLOR)
        io_outpb(0x3FF, 0x5A); // write soft reset key
    #endif
        io_outpb(0x64, 0xfe); // reboot
    }
}

/***************************  Bootloader Functions ****************************/
bool init(void) {
	unsigned char mcm_irq;
#if defined (__ONE_ZERO) || defined (__EDUCAKE) || defined (__AI)
    int i, crossbarBase, gpioBase;
    void *pciDev = NULL;
#elif defined (__DUO) || defined (__MOTION_9D) || defined (__3DP_COLOR)
	int i, j;
    unsigned long data;
    void* pci_handle;
#endif
    
	if (io_Init() == false) return false;
    timer_NowTime(); // initialize timer
    CLOCKS_PER_MICROSEC = vx86_CpuCLK();
    VORTEX86EX_CLOCKS_PER_MS = CLOCKS_PER_MICROSEC*1000UL;
    
    // Set IRQ4 as level-trigger
    io_outpb(0x4D0, io_inpb(0x4D0) | 0x10);
#if defined (__ONE_ZERO) || defined (__EDUCAKE) || defined (__AI)
    crossbarBase = sb_Read16(SB_CROSSBASE) & 0xfffe;
	if (crossbarBase == 0 || crossbarBase == 0xfffe)
	{
		sb_Write16(SB_CROSSBASE, CROSSBARBASE | 0x01);
		crossbarBase = CROSSBARBASE;
	}
#elif defined (__DUO) || defined (__MOTION_9D) || defined (__3DP_COLOR)
    // Set Hybrid Base address
    pci_handle = pci_Alloc(0, 0x06, 0); // Hybrid
    if (pci_handle != NULL)
        pci_Out32(pci_handle, 0x48, HYB_BASEADDRESS | 0x03);
	else
    {
        printf("Hybrid init fail.");
        return false;
    }
    pci_Free(pci_handle);
    
	//set corssbar Base Address
    io_outpdw(HYB_BASEADDRESS+0x20, CROSSBARBASE | 0x03);
#endif

#if defined CRB_DEBUGMODE
	for (i=0; i<CRBTABLE_SIZE; i++) io_outpb(CROSSBARBASE+i, CROSSBARTABLE[i]);
#endif

	// Force set HIGH speed ISA on SB
	sb_Write(SB_FCREG, sb_Read(SB_FCREG) | 0x8000C000L);
#if defined (__ONE_ZERO) || defined (__EDUCAKE) || defined (__AI)
    //set SB GPIO Base Address
	gpioBase = sb_Read16(SB_GPIOBASE) & 0xfffe;
	if (gpioBase == 0 || gpioBase == 0xfffe)
	{
		sb_Write16(SB_GPIOBASE, GPIOCTRLBASE | 0x01);
		gpioBase = GPIOCTRLBASE;
	}

	// Enable GPIO 0 ~ 9
	io_outpdw(gpioBase, 0x03ff);

	// set GPIO Port 0~7 dircetion & data Address
	for (i=0;i<10;i++)
		io_outpdw(gpioBase + (i+1)*4,((GPIODIRBASE + i*4)<<16) + GPIODATABASE + i*4);

	pciDev = pci_Alloc(0x00, 0x0F, 0x00);
	pci_Out8(pciDev, 0x04, 0x05);
	pci_Out8(pciDev, 0x05, 0x01);
	pci_Free(pciDev);
    
#elif defined (__DUO) || defined (__MOTION_9D) || defined (__3DP_COLOR)
	// set SB GPIO Base Address
    pci_handle = pci_Alloc(0x00, 0x00, 0x00); // NB configuration space
	if (pci_handle == NULL) {printf("NB device don't exist\n"); return false;}
	pci_Out32(pci_handle, NB_GPIO_CBA_REG, (unsigned long)(CONFIG_BASEADDR | 0x01));
	pci_Free(pci_handle);

    io_outpdw(CONFIG_BASEADDR, 0xFFFF); // Enable GPIO 0 ~ 15
    
    // set GPIO Port 0 ~ 15 dircetion & data address
	for (i = 0; i < 16; i++) {
		data = ((unsigned long)DIR_BA[i] << 16) + DATA_BA[i];
		io_outpdw(CONFIG_BASEADDR + 0x04 + 0x04 * i, data); // dir+data
		io_outpdw(CONFIG_BASEADDR + 0x44 + 0x04 * i, (unsigned long)CONTROL_BA[i]); // control
	}
#endif
    
    // Set TX/RX LED pins and OFF them.
    set_gpio_config_addr();
#if defined (__ONE_ZERO) || defined (__EDUCAKE) || defined (__AI)
    set_tx_led(7, 2);
	set_rx_led(7, 3);
#elif defined (__86DUINO_DUO)
    set_tx_led(14, 6);
	set_rx_led(14, 7);
#endif
	TX_LED_OFF();
	RX_LED_OFF();
    
    // Setup USBCDC & USBHID
    handle = com_Init(USB_COM);
    if (handle == NULL)
    {
        printf("USB Device init fail\n");
        goto EXITUSBSETTING;
    }
    com_SetTimeOut(handle, 0L, 500L); // USB RX timerout is 0ms and TX timeout is 500ms
    
    com_SetControlLineStateCallback(handle, _86Duino_reset_callback); // Add the unique "Reset" function for 86Duino ver.
    com_SetDataInCallback(handle, _86Duino_TXLED_ON_callback); // light the TX LED when DataIn
    com_SetDataOutCallback(handle, _86Duino_RXLED_ON_callback); // light the RX LED when DataOut    
    com_SetSOFCallback(handle, _86Duino_LED_OFF_callback); // Examine the count of TX/RX LED in the SOF signal of ISR.
    
    // Add the unique "TX/RX LED Blink" function for 86Duino ver.
    com_EnableSOFInterrupt(handle); // Enable SOF interrupt to remain 100ms LED Blink.
    printf("USB Device init OK.\n");

EXITUSBSETTING:
    return true;
}

void EnableLLED(void) {
#if defined (__ONE_ZERO) || defined (__EDUCAKE) || defined (__AI)
    io_outpb(0xf20e, io_inpb(0xf20e) | (1 << 1)); // L_LED dir -> OUTPUT
	io_outpb(0x0AA9, 0x01); // Crossbar -> GPIO
#elif defined (__DUO)
    io_outpb(0x0DBE, io_inpb(0x0DBE) | (1 << 4)); // L_LED dir -> OUTPUT
    io_outpb(CROSSBARBASE + 0xFE, 0x08); // Crossbar -> GPIO
#endif
}

void DisableLLED(void) {
#if defined (__ONE_ZERO) || defined (__EDUCAKE) || defined (__AI)
    io_outpb(0xf20e, io_inpb(0xf20e) & ~(1 << 1)); // L_LED dir -> INPUT
	io_outpb(0x0AA9, 0x08); // Crossbar -> PWM
#elif defined (__DUO)
    io_outpb(0x0DBE, io_inpb(0x0DBE) & ~(1 << 4)); // L_LED dir -> INPUT
    io_outpb(CROSSBARBASE + 0xFE, 0x01); // Crossbar -> PWM
#endif
}

void L_LED_OFF(void) {
#if defined (__ONE_ZERO) || defined (__EDUCAKE) || defined (__AI)
    io_outpb(0xf20c, io_inpb(0xf20c) | (1 << 1));
#elif defined (__DUO)  
    io_outpb(0x0DAE, io_inpb(0x0DAE) | (1 << 4));
#endif
}

void L_LED_ON(void) {
#if defined (__ONE_ZERO) || defined (__EDUCAKE) || defined (__AI)
    io_outpb(0xf20c, io_inpb(0xf20c) & ~(1 << 1));
#elif defined (__DUO) 
    io_outpb(0x0DAE, io_inpb(0x0DAE) & ~(1 << 4));
#endif
}

unsigned short LLEDPulse = 0;
void LEDPulse(unsigned long* ledTStart, unsigned long ledTNow)
{
    if ((ledTNow - *ledTStart) > (8 * vx86_CpuCLK())) // delay 8us
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
        *ledTStart = timer_GetClocks();
    }
}

unsigned char getRstKey(void) {
    unsigned char tmp;

#if defined (__ONE_ZERO) || defined (__EDUCAKE) || defined (__AI)
    tmp = io_inpb(0xf21A);
	io_outpb(0xf21A, 0x00);
#elif defined (__DUO) || defined (__MOTION_9D) || defined (__3DP_COLOR)
    tmp = io_inpb(0x3FF);
	io_outpb(0x3FF, 0x00);
#endif

    return tmp;
}

int GetBootLoaderMode(void) {
	int mode = 0;
    
#if defined (__MOTION_9D) || defined (__3DP_COLOR)
    return BTL_MODE;
#elif defined (__DUO)
	io_outpb(0x0DBC, io_inpb(0x0DBC) & ~(1 << 4)); // SPI0_CS1 dir -> INPUT
    io_outpb(CROSSBARBASE + 0xFC, 0x08); // Crossbar -> GPIO
    if ((io_inpb(0x0DAC) & (1 << 4)) > 0)
        mode = 1;
    else
        mode = 0;
    io_outpb(CROSSBARBASE + 0xFC, 0x01);
#elif defined (__ONE_ZERO)
	io_outpb(0x0A88, 0x01); // RICH-IO PORT -> GPIO
	io_outpb(0xf222, io_inpb(0xf222) & ~0x01); // GPIO -> INPUT
	mode = io_inpb(0xf220) & 0x01; // Read SPICS1(GPIO80) PIN
	io_outpb(0x0A88, 0x08); // GPIO -> RICH-IO PORT
#elif defined (__EDUCAKE)
	//io_outpb(0x264, io_inpb(0x264) | 0x01);
	if((io_inpb(0x2ee) & 0x40) == 0) mode = 1;
#elif defined (__AI)
	mode = 1; // Now, for testing, it must be program mode 
#endif

	if (mode > 0)
		return PRO_MODE;
	else
		return BTL_MODE;
}

struct bld_info {
    char* ver;
    char* ver_n;
} BLD;

const char* bios_ver       = "Beta 0.9";
char old_filename[13]; // for getOldFilename()
static char version[VERSION_BYTES] = {'\0'};

void* get_prospace(unsigned long pro_size) {
    void *p = NULL;
    unsigned long disk_freespace, freememsize;
	struct dfree cdisk, adisk;
    __dpmi_free_mem_info mem_free_info;
	
	// Get disk free space
	getdfree(1, &adisk); // Disk A
	getdfree(3, &cdisk); // Disk C
	if (cdisk.df_avail == 0)
		disk_freespace = (long) adisk.df_avail * (long) adisk.df_bsec * (long) adisk.df_sclus;
	else
		disk_freespace = (long) cdisk.df_avail * (long) cdisk.df_bsec * (long) cdisk.df_sclus;

	// Get memory free space
	freememsize = mem_free_info.largest_available_free_block_in_bytes;
	
    if ((atoi(BLD.ver_n) >= 20191126) && (disk_freespace < RESERVEDSPACE))
        system("del *.b86");
    
    if ((freememsize >= pro_size) && (disk_freespace >= pro_size))
        p = ker_Malloc(pro_size);
	
	return p;
}

unsigned char Cal_checksum(unsigned char* cch, unsigned long packagesize) {
	unsigned long i;
	unsigned char checksum = 0;
	for (i=0; i<(packagesize - 1); i++)
		checksum = checksum + cch[i];
	checksum = ~(checksum);
	return checksum;
}

static void getOldFilename(char* filename) {
    char ch[12];
    int i, j;
    int rnum = int( rand() % 100000000 );
    for (i=0; i<12; i++) ch[i] = '\0';
    itoa(rnum, ch, 10);
    for (j=0; j<12; j++)     if (ch[j] == '\0') break;
    for (i=0; i<(8-j); i++)  old_filename[i] = '0';
    for (j=0; i<8; j++, i++) old_filename[i] = ch[j];
    old_filename[i++] = '.';
    old_filename[i++] = 'b';
    old_filename[i++] = '8';
    old_filename[i++] = '6';
}

int main(int argc, char* argv[]) {
	FILE* fp = NULL;
	int i, j, mode;
	bool old_blversion = false, _run_tool_fw = false;
	long length = 0L, total_l = 0L, tmp = 0L;
	bool get_length = false, transfer = false, empty_program = false, reboot = false;
	unsigned long _time;
	unsigned char rst_key = 0, old_command = 0;
	unsigned char* pstart_p, *package_p;
    int _prev, _cur, bstate;
    unsigned long ledTStart;
    // the name of another  fixed file is "_f86me.exe"
	
	// the variables for package
	char filename[FNAME_BYTES];
	unsigned long packageSize = 0L, ps = 0L, filesize = 0L;
	unsigned short command  = 0, errorcode = 0;
	unsigned char isLast = YES, checksum = 0;
	
	// for writing flash
	unsigned long msize = 0L;
	
    if (init() == false)
    {
        printf("init bootloader fail\n");
        return 1;
    }

    // set the bootloader version
    BLD.ver = "Jade 1.0";
    BLD.ver_n = "20191126";
    
    srand(time(0));
    ledTStart = timer_GetClocks();
    EnableLLED();
    
    // Is pre-reset soft reset? if yes, go to bootloader
	rst_key = getRstKey();
    printf("rst_key=%02X\n", rst_key);
    
	// Try to open typical _86duino.exe file in disk or flash
	fp = fopen("_86duino.exe", "rb");
	if (fp != NULL)
		fclose(fp);
	else
		empty_program = true;
    
    // Detect the status of external pin. EX. LOW = BL mode, HIGH = PRG mode.
	mode = GetBootLoaderMode();

    if (fp != NULL && rst_key != 0x5A && mode == PRO_MODE)
		return spawnl(P_OVERLAY, "_86duino.exe", "_86duino.exe", NULL);
	
	_time = timer_NowTime();
	bstate = GETPSZIE;
	_prev = _cur = 0; // QueryRXQuere counter
	while((timer_NowTime() - _time) < TIME_OUT)
	{
        // Breathing light (I'm alive!)
		LEDPulse(&ledTStart, timer_GetClocks());
        
		// If no _86duino.exe file in disk or flash, reading process without 8 sec limit until complete 
		if (empty_program == true) _time = timer_NowTime();
		
		if (com_Ready(handle) == false) continue;

		_cur = com_QueryRxQueue(handle); // re-count if the value is changed
		if (_cur != _prev)
		{
			_time  = timer_NowTime();
			_prev = _cur;
		}
		
		switch(bstate)
		{
		case GETPSZIE: 
			if (com_QueryRxQueue(handle) >= (OLDCOMM_BYTES + PSIZE_BYTES))
		    {
		    	old_command = com_Read(handle); // for version 090 uploader
				if (old_command == 0xff) com_Write(handle, 'H'); 
				for (i=0, packageSize=0L; i<PSIZE_BYTES; i++)
					packageSize = packageSize + ((unsigned long)com_Read(handle)<< (8*i));
				packageSize &= 0x0fffffffL;
                #if defined DEBUG_MODE
                    printf("package size = %ld\n", packageSize);
                #endif
				ps = packageSize;
				// malloc memory space for package
				package_p = (unsigned char*)get_prospace(packageSize);
				_time = timer_NowTime(); // reset timer
				if (package_p == NULL)
				{
                #if defined DEBUG_MODE
					printf("ERROR: \'package size\' malloc fail.\n");
					while(1);
                #endif
					errorcode |= EMALLOC;
					bstate = SENDRESULT;
					break;
				}

				pstart_p = package_p;
				
				// for version 090 uploader
				if (old_command == 0x01 || old_command == 0x02)
				{
					if (old_command == 0x01) command = BURN_PROGRAM; else command = BURN_BOOTLOADER;
					total_l = packageSize;
					old_blversion = true;
				}
				
				bstate = GETPACKAGE;
			}
			break;
		case GETPACKAGE:
			if (transfer == false)
		    {
				tmp = com_QueryRxQueue(handle);
				ps = ps - tmp;
				if (ps == 0L) transfer = true; // complite
				for (i=0; i<tmp; i++)
				{
					*package_p = (unsigned char)com_Read(handle);
					package_p++;
				}
                // if (tmp != 0) com_Write(handle, 'c'); // return ACK
                // printf("ps=%d\n", ps);
			}
			else
			{
				transfer = false;
				package_p = pstart_p;
				if (old_blversion == true)
					bstate = WRITEFILE;
				else
					bstate = GETCOMMAND;
			}
			break;
		case GETCOMMAND:
			for (i=0, command=0; i<COMM_BYTES; i++)
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
			case BURN_PROGRAM:    case BURN_TOOL_FIRMWARE:
            case GET_BOARD_NAME:  case BOOTLOADER_END:
            case RUN_TOOL_FIRMWARE: case DEL_TOOL_FIRMWARE:
            case GET_BOOTLOADER_NAME: case BURN_OTHERFILE:
            case GET_86ME_FW_VERSION:
				// Calculate checksum
				for (length=0, checksum=0; length < (packageSize-1); length++)
				{
					checksum += *package_p;
					package_p++;
				}
				checksum = ~checksum;
                #if defined DEBUG_MODE
                    printf("checksum = %02X *package_p = %02X\n", checksum, *package_p);
                #endif
				if (checksum != *package_p)
				{
					ker_Mfree((void*)pstart_p);
					errorcode |= ECHECKSUM;
					bstate = SENDRESULT;
					break;
				}
				package_p--;
				isLast = *package_p;
				package_p = pstart_p + COMM_BYTES;
				if (command == GET_BIOS_VER || command == GET_BOOTLOADER_VER || command == GET_BOOTLOADER_NAME || command == GET_86ME_FW_VERSION)
					bstate = GETVERSION;
				else if (command == BURN_BIOS)
					bstate = BURN86BIOS;
                else  if (command == GET_BOARD_NAME)
                    bstate = GETBOARDNAME;
                else  if (command == BOOTLOADER_END)
                    bstate = BOOTLOADEREND;
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
			for (i=0; i<FNAME_BYTES; i++)
			{
				filename[i] = (char)(*package_p);
				package_p++;
			}
			for (i=0; i<FNAME_BYTES; i++)
				if (filename[i] == '\0') break;
			if (i == FNAME_BYTES)
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
			for (i=0, filesize=0L; i<FSIZE_BYTES; i++)
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
			if (fp == NULL)
			{
				ker_Mfree((void*)pstart_p);
				errorcode |= EOPENFILE;
				bstate = SENDRESULT;
				break;
			}

			for (; total_l > 0L; total_l -= msize, package_p += msize)
			{
				msize = (total_l <= SECTION) ? total_l : SECTION;
				if (fwrite(package_p, sizeof(unsigned char), msize, fp) != msize)
				{
					ker_Mfree((void*)pstart_p);
					errorcode |= EWRITEFILE;
					bstate = SENDRESULT;
					break;
				}
				com_Write(handle, 'z');
				fflush(fp);
			}
		    fclose(fp);
		    
            if (command == RUN_TOOL_FIRMWARE)
            {
                if ((fp = fopen(filename, "r")) != NULL)
                {
                    fclose(fp);
                    _run_tool_fw = true;
                }
                else
                {
                    errorcode |= ENOTOOLFW;
                }
            }
            else if (command == DEL_TOOL_FIRMWARE)
            {
                if ((fp = fopen(filename, "r")) != NULL)
                {
                    fclose(fp);
                    remove(filename);
                }
                else
                {
                    errorcode |= ENOTOOLFW;
                }
            }
		    else if (command == BURN_BOOTLOADER)
			{
		    	remove("_v86boot.exe");
				rename("___86tmp.$$$", "_v86boot.exe");
				if ((fp = fopen("_86duino.exe", "rb")) != NULL)
					fclose(fp);
				else
					reboot = true;
		    }
		    else if (command == BURN_PROGRAM)
		    {
                if (atoi(BLD.ver_n) >= 20191126)
                {
                    getOldFilename(old_filename);
                    if ((fp = fopen(old_filename, "r")) != NULL)
                    {
                        fclose(fp);
                        remove(old_filename);
                    }
                    if (empty_program == false)
                        rename("_86duino.exe", old_filename); // old_filename may be "12345678.b86"
                }
                else if ((strcmp(BLD.ver, "Hehuan 1.0") == 0) && (empty_program == false))
                {
                    remove("_86duino.exe");
                }
                
				rename("___86tmp.$$$", "_86duino.exe");
			}
			else // BURN_OTHERFILE or BURN_TOOL_FIRMWARE
			{
                if (command == BURN_TOOL_FIRMWARE) _run_tool_fw = true;
                
                if ((fp = fopen(filename, "r")) != NULL)
                {
                    fclose(fp);
                    remove(filename);
                }
				rename("___86tmp.$$$", filename);
			}
			
		    _time = timer_NowTime(); // reset timer
		    ker_Mfree((void*)pstart_p);
			com_Write(handle, '1');
			if (old_blversion == true)
				goto END; // we don't care "isLast" flag and result package for old bootloader
			else
				bstate = SENDRESULT;
			break;
        case SENDRESULT:
        	package_p = (unsigned char*) ker_Malloc(WFDONE_BYTES);
        	packageSize = COMM_BYTES + ERRCODE_BYTES + CHSUM_BYTES;
        	for (i=0, j=0; j<PSIZE_BYTES; i++, j++)
				package_p[i] = (packageSize & (0xff << (8*j))) >> (8*j);
			for (j=0; j<COMM_BYTES; i++, j++)
				package_p[i] = (command & (0xff << (8*j))) >> (8*j);
			for (j=0; j<ERRCODE_BYTES; i++, j++)
				package_p[i] = (errorcode & (0xff << (8*j))) >> (8*j);
        	package_p[i] = Cal_checksum(package_p+PSIZE_BYTES, packageSize);
        	
        	com_Send(handle, package_p, WFDONE_BYTES);
        	
        	ker_Mfree((void*)package_p);
        	if (errorcode == 0 && isLast == NO)
        	{
        		_time = timer_NowTime();
        		bstate = GETPSZIE;
        	}
			else
				goto END;
        	break;
		case GETVERSION:
			if (command == GET_BOOTLOADER_VER)
			{
				for (i=0; i<VERBITS; i++)
				{
					if (BLD.ver_n[i] != '\0')
						version[i] = BLD.ver_n[i];
					else
						break;
				}
				version[i] = '\0';
			}
			else if (command == GET_BIOS_VER)
			{
				fp = fopen("_bver.v86", "r");
				if (fp == NULL)
				{
					for (i=0; i<VERBITS; i++)
					{
						if (bios_ver[i] != '\0')
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
            else if (command == GET_BOOTLOADER_NAME)
            {
                for (i=0; i<VERBITS; i++)
				{
					if (BLD.ver[i] != '\0')
						version[i] = BLD.ver[i];
					else
						break;
				}
				version[i] = '\0';
            }
            else if (command == GET_86ME_FW_VERSION)
            {
                fp = fopen("_86mever.v86", "r");
				if (fp == NULL)
				{
                    char* s = "no this file";
					for (i=0; i<VERBITS; i++)
					{
						if (s[i] != '\0')
							version[i] = s[i];
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
        case GETBOARDNAME:
            fp = fopen("_brdn.v86", "r");
            if (fp == NULL)
            {
                errorcode |= EOPENFILE;
                bstate = SENDRESULT;
                break;
            }
            else
            {
                fgets(version, VERSION_BYTES, fp);
                fclose(fp);
            }
            bstate = GETVERSIONDONE;
            break;
		case GETVERSIONDONE:
			package_p = (unsigned char*) ker_Malloc(VSDONE_BYTES);
        	packageSize = COMM_BYTES + VERSION_BYTES + ERRCODE_BYTES + CHSUM_BYTES;
        	for (i=0, j=0; j<PSIZE_BYTES; i++, j++)
				package_p[i] = (packageSize & (0xff << (8*j))) >> (8*j);
			for (j=0; j<COMM_BYTES; i++, j++)
				package_p[i] = (command & (0xff << (8*j))) >> (8*j);
			for (j=0; j<VERSION_BYTES; i++, j++)
				package_p[i] = version[j];
			for (j=0; j<ERRCODE_BYTES; i++, j++)
				package_p[i] = (errorcode & (0xff << (8*j))) >> (8*j);
        	package_p[i] = Cal_checksum(package_p+PSIZE_BYTES, packageSize);
        	
        	com_Send(handle, package_p, VSDONE_BYTES);
        	
        	ker_Mfree((void*)package_p);
        	if (errorcode == 0 && isLast == NO)
        	{
        		_time = timer_NowTime();
        		bstate = GETPSZIE;
        	}
			else
				goto END;
			break;
        case BOOTLOADEREND:
            ker_Mfree((void*)pstart_p);
            goto END;
            break;
		case BURN86BIOS: // bootloader: anybios.exe -> coreboot.rom -> burn it -> versoin file
			fp = fopen("anybios.exe", "r");
			if (fp == NULL)
			{
				errorcode |= EOPENFILE;
				bstate = SENDRESULT;
				break;
			}
			fclose(fp);
			fp = fopen("coreboot.rom", "r");
			if (fp == NULL)
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
	} // while((timer_NowTime() ...
	
	
END: // prepare to run user program
	timer_Delay(200);
	com_Close(handle);
	// DisableLLED();
	
	if (reboot == true)
		io_outpb(0x64, 0xfe); //reset CPU;
    else if (_run_tool_fw == true)
        return spawnl(P_OVERLAY, filename, filename, NULL);
    else
		return spawnl(P_OVERLAY, "_86duino.exe", "_86duino.exe", NULL);
}

