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
#include "mcm.h"
#include "irq.h"
#include "stdio.h"
#include "com.h"
#include "fifolan.h"

COMPort *handle = NULL;
bool Global_irq_Init = false;
extern void setPinStatus(void);
extern bool HID_Reinitialize(struct usb_dev *usb);
extern void HID_Uninitialize(struct usb_dev *usb);

#if defined (__86DUINO_ZERO) || defined (__86DUINO_ONE) || defined (__86DUINO_EDUCAKE) || defined (__86DUINO_AI)
    #define LED_MAX_COUNT    (100) // Under FastSpeed, the SOF interrupt interval is 1ms, so LED_MAX_COUNT is 100.
    unsigned long CLOCKS_PER_MICROSEC = 300L; // The default value is 400Mhz for 86Duino slave, you should call init() to set it automatically.
	unsigned long VORTEX86EX_CLOCKS_PER_MS = 300000L; // The default value is 300000 for 86Duino, you should call init() to set it automatically.
#elif defined (__86DUINO_EX2)
	#define LED_MAX_COUNT    (800) // Under HiSpeed, the SOF interrupt interval is 125us, so LED_MAX_COUNT is 800.
    unsigned long CLOCKS_PER_MICROSEC = 400L; // The default value is 400Mhz for 86Duino slave, you should call init() to set it automatically.
	unsigned long VORTEX86EX_CLOCKS_PER_MS = 400000L; // The default value is 300000 for 86Duino, you should call init() to set it automatically.
#endif

static int tx_led_count = 0;
static int rx_led_count = 0;

#define GPIO_CONFIG_ADDR                (0xF100)
#define GPIO_BASE_ADDR                  (0xF200)
static unsigned short gpio_config_addr = 0x0000;
static unsigned short tx_led_data  = 0x0000;
static unsigned short tx_led_dir   = 0x0000;
static unsigned short rx_led_data  = 0x0000;
static unsigned short rx_led_dir   = 0x0000;
static char tx_led_pin = 0;
static char rx_led_pin = 0;

static void set_gpio_config_addr(void)
{
#if defined (__86DUINO_ZERO) || defined (__86DUINO_ONE) || defined (__86DUINO_EDUCAKE) || defined (__86DUINO_AI)
    unsigned long value;
    
    value = sb_Read16(0x62) & 0xFFFE;
    if (!value)
        gpio_config_addr = GPIO_CONFIG_ADDR;
    else
        gpio_config_addr = value;
    value = gpio_config_addr | 0x01;
    sb_Write16(0x62, value);
#elif defined (__86DUINO_EX2)
    gpio_config_addr = CONFIG_BASEADDR;
#endif
}

static void set_tx_led(char port, char pin)
{
    io_outpdw(gpio_config_addr + 0x00, io_inpdw(gpio_config_addr + 0x00) | 0x01L << port);
    
    // set data address
    if ((tx_led_data = io_inpw(gpio_config_addr + (4 + 4*port))) == 0x0000)
        io_outpw(gpio_config_addr + (4 + 4*port), (tx_led_data = (GPIO_BASE_ADDR + port)));
    // set direction address
    if ((tx_led_dir  = io_inpw(gpio_config_addr + (6 + 4*port))) == 0x0000)
        io_outpw(gpio_config_addr + (6 + 4*port), (tx_led_dir = (GPIO_BASE_ADDR + port + 2)));
    
    // set TX LED pin -> OUT
    io_outpb(tx_led_dir, io_inpb(tx_led_dir) | (1 << pin));
    tx_led_pin = pin;
}

static void set_rx_led(char port, char pin)
{
    io_outpdw(gpio_config_addr + 0x00, io_inpdw(gpio_config_addr + 0x00) | 0x01L << port);
    
    // set data address
    if ((rx_led_data = io_inpw(gpio_config_addr + (4 + 4*port))) == 0x0000)
        io_outpw(gpio_config_addr + (4 + 4*port), (rx_led_data = (GPIO_BASE_ADDR + port)));
    // set direction address
    if ((rx_led_dir  = io_inpw(gpio_config_addr + (6 + 4*port))) == 0x0000)
        io_outpw(gpio_config_addr + (6 + 4*port), (rx_led_dir = (GPIO_BASE_ADDR + port + 2)));
    
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
    io_outpb(tx_led_data, io_inpb(tx_led_data) | (1 << tx_led_pin));
}

static void RX_LED_OFF(void)
{
    io_outpb(rx_led_data, io_inpb(rx_led_data) | (1 << rx_led_pin));
}

void _86Duino_reset_callback(void) {
    if (handle == NULL) return;
    // the below behavier is only for compatible of Arduino Leonado (windows)
    if(com_GetBaud(handle) == 1200 && com_GetDTR(handle) == 0)
    {   
        io_DisableINT();
        com_Disconnect(handle); // io_outpb(usb_on_off_data, io_inpb(usb_on_off_data) | (1 << usb_on_off_pin));
        io_outpb(0xf21A, 0x5a); // write soft reset key
        io_outpb(0x64, 0xfe); // reboot
    }
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

#if defined (__86DUINO_EX2)
unsigned short DIR_BA[16] = {0x0DB0, 0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5, 0x0DB6, 0x0DB7, 
							 0x0DB8, 0x0DB9, 0x0DBA, 0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0DBF};
unsigned short DATA_BA[16] = {0x0DA0, 0x0DA1, 0x0DA2, 0x0DA3, 0x0DA4, 0x0DA5, 0x0DA6, 0x0DA7, 
							  0x0DA8, 0x0DA9, 0x0DAA, 0x0DAB, 0x0DAC, 0x0DAD, 0x0DAE, 0x0DAF};
unsigned short CONTROL_BA[16] = {0x0DC0, 0x0DC4, 0x0DC8, 0x0DCC, 0x0DD0, 0x0DD4, 0x0DD8, 0x0DDC, 
							     0x0DE0, 0x0DE4, 0x0DE8, 0x0DEC, 0x0DF0, 0x0DF4, 0x0DF8, 0x0DFC};
#endif

void setFIFOCOM_BaseAddr(void) {
    // Set FIFO COM Base Address to 0xD8000
    nb_Write(0xBC, 0xD8000 | 0x01);
}

void setFIFOCOM_IRQ(void) {
    int irq;
    const unsigned int route_irq[16] = {
        0, 8, 0, 2, 4, 5, 7, 6, 0, 1, 3, 9, 11, 0, 13, 15
    };
    
    irq = 10;
    if (irq > 7)
        io_outpb(0x4d1, io_inpb(0x4d1) | (1 << (irq - 8)));
    else
        io_outpb(0x4d0, io_inpb(0x4d0) | (1 << irq));
    // Set FIFO COM IRQ to 10
    sb1_Write(0xB4, sb1_Read(0xB4) & ~(0x0F << 16) | (route_irq[irq] << 16));
}

bool init() {
#if defined (__86DUINO_ZERO) || defined (__86DUINO_ONE) || defined (__86DUINO_EDUCAKE) || defined (__86DUINO_AI)
    int i, crossbarBase, gpioBase;
#elif defined (__86DUINO_EX2)
	int i, j;
    unsigned long data;
    unsigned char mcm_irq;
    void* pci_handle;
#endif
    
	if(io_Init() == false) return false;
    timer_NowTime(); // initialize timer
    CLOCKS_PER_MICROSEC = vx86_CpuCLK();
    VORTEX86EX_CLOCKS_PER_MS = CLOCKS_PER_MICROSEC*1000UL;
    
    // Set IRQ4 as level-trigger
    io_outpb(0x4D0, io_inpb(0x4D0) | 0x10);
#if defined (__86DUINO_ZERO) || defined (__86DUINO_ONE) || defined (__86DUINO_EDUCAKE) || defined (__86DUINO_AI)
    crossbarBase = sb_Read16(SB_CROSSBASE) & 0xfffe;
	if(crossbarBase == 0 || crossbarBase == 0xfffe)
	{
		sb_Write16(SB_CROSSBASE, CROSSBARBASE | 0x01);
		crossbarBase = CROSSBARBASE;
	}
#elif defined (__86DUINO_EX2)
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
	for(i=0; i<CRBTABLE_SIZE; i++) io_outpb(CROSSBARBASE+i, CROSSBARTABLE[i]);
	#endif

	// Force set HIGH speed ISA on SB
	sb_Write(SB_FCREG, sb_Read(SB_FCREG) | 0x8000C000L);
#if defined (__86DUINO_ZERO) || defined (__86DUINO_ONE) || defined (__86DUINO_EDUCAKE) || defined (__86DUINO_AI)
    //set SB GPIO Base Address
	gpioBase = sb_Read16(SB_GPIOBASE) & 0xfffe;
	if(gpioBase == 0 || gpioBase == 0xfffe)
	{
		sb_Write16(SB_GPIOBASE, GPIOCTRLBASE | 0x01);
		gpioBase = GPIOCTRLBASE;
	}

	// Enable GPIO 0 ~ 9
	io_outpdw(gpioBase, 0x03ff);

	// set GPIO Port 0~7 dircetion & data Address
	for(i=0;i<10;i++)
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
#elif defined (__86DUINO_EX2)
	// set SB GPIO Base Address
    pci_handle = pci_Alloc(0x00, 0x00, 0x00); // NB configuration space
	if(pci_handle == NULL) {printf("NB device don't exist\n"); return false;}
	pci_Out32(pci_handle, NB_GPIO_CBA_REG, (unsigned long)(CONFIG_BASEADDR | 0x01));
	pci_Free(pci_handle);

    io_outpdw(CONFIG_BASEADDR, 0xFFFF); // Enable GPIO 0 ~ 15
    
    // set GPIO Port 0 ~ 15 dircetion & data address
	for(i = 0; i < 16; i++) {
		data = ((unsigned long)DIR_BA[i] << 16) + DATA_BA[i];
		io_outpdw(CONFIG_BASEADDR + 0x04 + 0x04 * i, data); // dir+data
		io_outpdw(CONFIG_BASEADDR + 0x44 + 0x04 * i, (unsigned long)CONTROL_BA[i]); // control
	}

	setPinStatus();
	
	// set ADC
    init_ADC();
    
	// set MCM Base Address
	set_MMIO();
	mcmInit();
	for(i=0; i<3; i++)
        if (isAllocThisMCMGroup(i))
            for (j=4*i; j<(4+4*i); j++)
                mc_SetMode(j, MCMODE_PWM_SIFB);
#endif
	// init wdt1
	wdt_init();

	if(Global_irq_Init == false)
	{
		// set MCM IRQ
		if(irq_Init() == false) 
	    {
	        printf("MCM IRQ init fail\n"); return false;
	    }
#if defined (__86DUINO_ZERO) || defined (__86DUINO_ONE) || defined (__86DUINO_EDUCAKE) || defined (__86DUINO_AI)
        if(irq_Setting(GetMCIRQ(), IRQ_LEVEL_TRIGGER + IRQ_DISABLE_INTR) == false)
	    {
	        printf("MCM IRQ Setting fail\n"); return false;
	    }
	    Set_MCIRQ(GetMCIRQ());
#elif defined (__86DUINO_EX2)
        for (i=0; i<3; i++)
        {
            if (isAllocThisMCMGroup(i))
            {
                mcm_irq = GetMCIRQ(i);
                if(irq_Setting(mcm_irq, IRQ_LEVEL_TRIGGER + IRQ_DISABLE_INTR) == false)
                {
                    printf("MCM IRQ Setting fail\n"); return false;
                }
                Set_MCIRQ(mcm_irq, i);
            }
        }
#endif
	    Global_irq_Init = true;
	}

    // Set TX/RX LED pins and OFF them.
    set_gpio_config_addr();
#if defined (__86DUINO_ZERO) || defined (__86DUINO_ONE) || defined (__86DUINO_EDUCAKE) || defined (__86DUINO_AI)
    set_tx_led(7, 2);
	set_rx_led(7, 3);
#elif defined (__86DUINO_EX2)
    set_tx_led(14, 6);
	set_rx_led(14, 7);
#endif
	TX_LED_OFF();
	RX_LED_OFF();
    
    // Setup USBCDC & USBHID
    com_SetUSBCallback(HID_Reinitialize, HID_Uninitialize, _86Duino_suspend_callback);
    handle = com_Init(USB_COM);
    if(handle == NULL)
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

EXITUSBSETTING:    
    //DM&P FIFOLAN intialize
    setFIFOCOM_BaseAddr();
    printf("FIFOCOM base: 0x%08X\n", nb_Read(0xBC));
    setFIFOCOM_IRQ();
    printf("FIFOCOM IRQ: 0x%02X\n", sb1_Read8(0xB6));
    
    if (flan_initDriver() != 0) {
        printf("ERROR: flan_initDriver() failed\n");
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
