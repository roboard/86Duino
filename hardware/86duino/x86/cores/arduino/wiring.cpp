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
#include "ipcnet.h"

COMPort *handle = NULL;
bool Global_irq_Init = false;
bool DEBUG_MSG = false;
unsigned char usbdev_irq = 0xFF;
extern void digitalInit(void);
extern bool HID_Reinitialize(struct usb_dev *usb);
extern void HID_Uninitialize(struct usb_dev *usb);

#if defined (__86DUINO_EX)
    #define LED_MAX_COUNT    (100) // Under FastSpeed, the SOF interrupt interval is 1ms, so LED_MAX_COUNT is 100.
    unsigned long CLOCKS_PER_MICROSEC = 300L; // The default value is 400Mhz for 86Duino slave, you should call init() to set it automatically.
	unsigned long VORTEX86EX_CLOCKS_PER_MS = 300000L; // The default value is 300000 for 86Duino, you should call init() to set it automatically.
#elif defined (__86DUINO_EX2)
	#define LED_MAX_COUNT    (800) // Under HiSpeed, the SOF interrupt interval is 125us, so LED_MAX_COUNT is 800.
    unsigned long CLOCKS_PER_MICROSEC = 400L; // The default value is 400Mhz for 86Duino slave, you should call init() to set it automatically.
	unsigned long VORTEX86EX_CLOCKS_PER_MS = 400000L; // The default value is 300000 for 86Duino, you should call init() to set it automatically.
    extern void init_ADC(void);
#endif

static int tx_led_count = 0;
static int rx_led_count = 0;

static unsigned short gpio_config_addr = 0x0000;
static unsigned short tx_led_data     = 0x0000;
static unsigned short tx_led_dir      = 0x0000;
static unsigned short rx_led_data     = 0x0000;
static unsigned short rx_led_dir      = 0x0000;
static unsigned short usb_onoff_data  = 0x0000;
static unsigned short usb_onoff_dir   = 0x0000;
static char tx_led_pin = 0;
static char rx_led_pin = 0;
static char usb_onoff_pin = 0;

static void set_gpio_config_addr(void)
{
#if defined (__86DUINO_EX)
    unsigned long value;
    
    value = sb_Read16(0x62) & 0xFFFE;
    if (!value)
        gpio_config_addr = GPIOCTRLBASE;
    else
        gpio_config_addr = value;
    value = gpio_config_addr | 0x01;
    sb_Write16(0x62, value);
#elif defined (__86DUINO_EX2)
    gpio_config_addr = CONFIG_BASEADDR;
#endif
}

static void setGPIOFunction(char port, char pin, unsigned short* dataPortBase, unsigned short* dirPortBase, char* controllPin)
{
    io_outpdw(gpio_config_addr + 0x00, io_inpdw(gpio_config_addr + 0x00) | 0x01L << port);
    
    // set data address
    if ((*dataPortBase = io_inpw(gpio_config_addr + (4 + 4*port))) == 0x0000)
    #if defined (__86DUINO_EX)
        io_outpw(gpio_config_addr + (4 + 4*port), (*dataPortBase = (GPIODATABASE + port*4)));
    #else
        io_outpw(gpio_config_addr + (4 + 4*port), (*dataPortBase = DATA_BA[port]));
    #endif
    // set direction address
    if ((*dirPortBase  = io_inpw(gpio_config_addr + (6 + 4*port))) == 0x0000)
    #if defined (__86DUINO_EX)
        io_outpw(gpio_config_addr + (6 + 4*port), (*dirPortBase = (GPIODIRBASE + port*4)));
    #else
        io_outpw(gpio_config_addr + (4 + 4*port), (*dirPortBase = DIR_BA[port]));
    #endif
    // set TX LED pin -> OUT
    io_outpb(*dirPortBase, io_inpb(*dirPortBase) | (1 << pin));
    *controllPin = pin;
}

static void TX_LED_ON(void)
{
    if (tx_led_data == 0) return;
    io_outpb(tx_led_data, io_inpb(tx_led_data) & ~(1 << tx_led_pin));
    tx_led_count = LED_MAX_COUNT;
}

static void RX_LED_ON(void)
{
    if (rx_led_data == 0) return;
    io_outpb(rx_led_data, io_inpb(rx_led_data) & ~(1 << rx_led_pin));
    rx_led_count = LED_MAX_COUNT;
}

static void USB_DEVICE_ON(void)
{
    if (usb_onoff_data == 0) return;
    io_outpb(usb_onoff_data, io_inpb(usb_onoff_data) & ~(1 << usb_onoff_pin));
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

static void USB_DEVICE_OFF(void)
{
    if (usb_onoff_data == 0) return;
    io_outpb(usb_onoff_data, io_inpb(usb_onoff_data) | (1 << usb_onoff_pin));
}

void _86Duino_reset_callback(void) {
    
    if (handle == NULL) return;
    
    // the below behavier is only for compatible of Arduino Leonado (windows)
    if((com_GetBaud(handle) == 1200) && (com_GetDTR(handle) == 0))
    {   
        io_DisableINT();
        com_Disconnect(handle); // io_outpb(usb_on_off_data, io_inpb(usb_on_off_data) | (1 << usb_on_off_pin));
        com_Close(handle);
    #if defined (__86DUINO_EX)
        io_outpb(0xf21A, 0x5A); // write soft reset key
    #elif defined (__86DUINO_EX2)
        io_outpb(0x3FF, 0x5A); // write soft reset key
    #endif
        io_outpb(0x64, 0xfe); // reboot
    }
}

void _86Duino_RXLED_ON_callback(void) {
    RX_LED_ON();
}

void _86Duino_TXLED_ON_callback(void) {
    TX_LED_ON();
}

void _86Duino_USB_ON_PIN_callback(void) {
    USB_DEVICE_ON();
}

void _86Duino_USB_OFF_PIN_callback(void) {
    USB_DEVICE_OFF();
}

void _86Duino_LED_OFF_callback(void) {
    if(tx_led_count && !(--tx_led_count)) TX_LED_OFF();
    if(rx_led_count && !(--rx_led_count)) RX_LED_OFF();
}

void _86Duino_suspend_callback(void) {
    TX_LED_OFF();
    RX_LED_OFF();
}

unsigned int coroutineState = IN_MAIN_LOOP;
void __yields(void) {
    switch(coroutineState) {
    case IN_MAIN_LOOP:
        stopMainLoopTimerInSCoop();
        helperYield();
        yield();
    break;
    case IN_HELPER:
        helperYield();
    break;
    case IN_SCOOP:
        yield();
    break;
    }
}

unsigned long TIMER_shiftTime = 0L;
unsigned long long int MICRO_shiftTime = 0LL;
static unsigned long long int timer_NowTime64() { 
    return (timer_GetClocks64()/VORTEX86EX_CLOCKS_PER_MS) - TIMER_shiftTime;
}

static unsigned long long int micro_NowTime64() { 
    return (timer_GetClocks64()/CLOCKS_PER_MICROSEC) - MICRO_shiftTime;
}

void setClockShiftMs(unsigned long shift) {
    TIMER_shiftTime += shift;
    MICRO_shiftTime += (shift * 1000);
}

unsigned long millis() {
    static unsigned long long int _endTime = timer_NowTime64() + 1LL;
    unsigned long long int tmp = timer_NowTime64(); // no static
    if (tmp >= _endTime) {
        if (irq_InInterrupt() == false) {
            __yields();
        }
        tmp = timer_NowTime64();
        _endTime = tmp + 1LL;
    }
	return tmp;
}

extern int _86DuinoState;
void delay(unsigned long ms) {
    unsigned long long int end = timer_NowTime64() + ms;
    while (timer_NowTime64() < end) {
        if (_86DuinoState == IN_MAIN) helperYield();
        __yields();
    }
}

// For 86Duino IDE, only use micros() after calling init()
unsigned long micros() {
    static unsigned long long int _endTime = micro_NowTime64() + 1000LL;
    unsigned long long int tmp = micro_NowTime64(); // no static
    if (tmp >= _endTime) {
        if (irq_InInterrupt() == false) {
            __yields();
        }
        
        tmp = micro_NowTime64();
        _endTime = tmp + 1000LL;
    }
	return (unsigned long) (tmp);
}

unsigned long long _millis() {
    return (timer_GetClocks64()/VORTEX86EX_CLOCKS_PER_MS);
}

unsigned long long _micros() {
    return (timer_GetClocks64()/CLOCKS_PER_MICROSEC);
}

void delayMicroseconds(unsigned long us) {
    unsigned long long end;
    if (us <= 50) timer_DelayMicroseconds(us);
    else {
        end = _micros() + us;
        while (_micros() < end) {
            if (_86DuinoState == IN_MAIN) helperYield();
            __yields();
        }
    }
}

#if defined (__86DUINO_EX2)
unsigned short DIR_BA[16] = {0x0DB0, 0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5, 0x0DB6, 0x0DB7, 
							 0x0DB8, 0x0DB9, 0x0DBA, 0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0DBF};
unsigned short DATA_BA[16] = {0x0DA0, 0x0DA1, 0x0DA2, 0x0DA3, 0x0DA4, 0x0DA5, 0x0DA6, 0x0DA7, 
							  0x0DA8, 0x0DA9, 0x0DAA, 0x0DAB, 0x0DAC, 0x0DAD, 0x0DAE, 0x0DAF};
unsigned short CONTROL_BA[16] = {0x0DC0, 0x0DC4, 0x0DC8, 0x0DCC, 0x0DD0, 0x0DD4, 0x0DD8, 0x0DDC, 
							     0x0DE0, 0x0DE4, 0x0DE8, 0x0DEC, 0x0DF0, 0x0DF4, 0x0DF8, 0x0DFC};

int getGPIODataBase(int port) {
    return DATA_BA[port];
}

int getGPIODirBase(int port) {
    return DIR_BA[port];
}

/*
#define FIFOCOM_BA    (0xD8000)
void setFIFOCOM_BaseAddr(void) {
    nb_Write(0xBC, FIFOCOM_BA | 0x01);
}

#define FIFOCOM_IRQ   (10)
void setFIFOCOM_IRQ(void) {
    int irq = FIFOCOM_IRQ;
    const unsigned int route_irq[16] = {
        0, 8, 0, 2, 4, 5, 7, 6, 0, 1, 3, 9, 11, 0, 13, 15
    };
    
    if (irq > 7)
        io_outpb(0x4d1, io_inpb(0x4d1) | (1 << (irq - 8)));
    else
        io_outpb(0x4d0, io_inpb(0x4d0) | (1 << irq));
    // Set FIFO COM IRQ to 10
    sb1_Write(0xB4, sb1_Read(0xB4) & ~(0x0F << 16) | (route_irq[irq] << 16));
}
*/
#endif

bool init() {
    unsigned char mcm_irq;
    void *_usbdev_EX;
    void *_usbdev_EX2;
#if defined (__86DUINO_EX)
    int i, crossbarBase, gpioBase;
    void *pciDev = NULL;
#elif defined (__86DUINO_EX2)
	int i, j;
    unsigned long data;
    void* pci_handle;
    unsigned short gpioBase;
#endif
    
	if (io_Init() == false) return false;
    timer_NowTime(); // initialize timer
    CLOCKS_PER_MICROSEC = vx86_CpuCLK();
    VORTEX86EX_CLOCKS_PER_MS = CLOCKS_PER_MICROSEC*1000UL;
    
    // Set IRQ4 as level-trigger
    io_outpb(0x4D0, io_inpb(0x4D0) | 0x10);
#if defined (__86DUINO_EX)
    crossbarBase = sb_Read16(SB_CROSSBASE) & 0xfffe;
	if (crossbarBase == 0 || crossbarBase == 0xfffe)
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

    getResource();
    
#if defined CRB_DEBUGMODE
	for (i=0; i<CRBTABLE_SIZE; i++) io_outpb(CROSSBARBASE+i, CROSSBARTABLE[i]);
#endif

	// Force set HIGH speed ISA on SB
	sb_Write(SB_FCREG, sb_Read(SB_FCREG) | 0x8000C000L);
#if defined (__86DUINO_EX)
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

	digitalInit();

	// set ADC Base Address
	sb_Write(0xbc, sb_Read(0xbc) & (~(1L<<28)));  // active adc
	sb1_Write16(0xde, sb1_Read16(0xde) | 0x02);   // not Available for 8051A Access ADC
	sb1_Write(0xe0, 0x0050fe00L); // baseaddr = 0xfe00, disable irq
	io_outpb(0xfe01, 0x00);
    for (i=0; (io_inpb(0xfe02) & 0x01) != 0 && i<16; i++) io_inpb(0xfe04); // clear ADC FIFO

	// set MCM Base Address
    set_MMIO();
    mcmInit();
    for(i=0; i<4; i++)
        mc_SetMode(i, MCMODE_PWM_SIFB);
    
    // USB Device Setting
    pciDev = pci_Alloc(0x00, 0x0F, 0x00);
	if (pciDev != NULL) {
        pci_Out8(pciDev, 0x04, 0x05);
        pci_Out8(pciDev, 0x05, 0x01);
        pci_Free(pciDev);
    }
    
#elif defined (__86DUINO_EX2)

    // set ADC
    init_ADC();
    
	// set SB GPIO Base Address
    pci_handle = pci_Alloc(0x00, 0x00, 0x00); // NB configuration space
	if (pci_handle == NULL) {printf("NB device don't exist\n"); return false;}
	gpioBase = pci_In16(pci_handle, NB_GPIO_CBA_REG) & 0xFFFE;
    if (gpioBase == 0) {pci_Out16(pci_handle, NB_GPIO_CBA_REG, CONFIG_BASEADDR | 0x01); gpioBase = CONFIG_BASEADDR;}
	pci_Free(pci_handle);

    if (DEBUG_MSG) printf("GPIO Base = 0x%X\n", gpioBase);
    // io_outpdw(gpioBase, 0xFFFF); // Enable GPIO 0 ~ 15
    
    // set GPIO Port 0 ~ 15 dircetion & data address
	for (i = 0; i < 16; i++) {
		data = io_inpdw(gpioBase + 0x04 + 0x04 * i); // dir+data
        if (data == 0L) {
            data = ((unsigned long)DIR_BA[i] << 16) + DATA_BA[i];
            io_outpdw(gpioBase + 0x04 + 0x04 * i, data); // dir+data
        } else {
            DATA_BA[i] = data & 0xFFFFL;
            DIR_BA[i] = (data >> 16) & 0xFFFFL;
        }
        
        data = io_inpdw(gpioBase + 0x44 + 0x04 * i);
        if (data == 0L) {
            io_outpdw(gpioBase + 0x44 + 0x04 * i, (unsigned long)CONTROL_BA[i]); // control
        } else {
            CONTROL_BA[i] = (unsigned short) data;
        }
	}

    digitalInit();
    
	// set MCM Base Address
	set_MMIO();
	mcmInit();
    #if defined (__86DUINO_MOTION9D)
        if (_G_MCM[0] == GET)
        {
            mc_SetMode(0, MCMODE_SERVO);
            mc_SetMode(1, MCMODE_SERVO);
            mc_SetMode(2, MCMODE_SERVO);
            mc_SetMode(3, MCMODE_PWM_SIFB);
        }
        
        if (_G_MCM[1] == GET)
        {
            mc_SetMode(4, MCMODE_SIF);
            mc_SetMode(5, MCMODE_SIF);
            mc_SetMode(6, MCMODE_SIF);
            mc_SetMode(7, MCMODE_SIF);
        }
        
        if (_G_MCM[2] == GET)
        {
            mc_SetMode(8, MCMODE_SIF);
            mc_SetMode(9, MCMODE_PWM_SIFB);
            mc_SetMode(10, MCMODE_SERVO);
            mc_SetMode(11, MCMODE_PWM_SIFB);
        }
    #else
        for(i=0; i<3; i++)
            if (_G_MCM[i] == GET)
                for (j=4*i; j<(4+4*i); j++)
                    mc_SetMode(j, MCMODE_PWM_SIFB);
    #endif
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

        for (i=0; i<3; i++)
        {
            if (_G_MCM[i] == GET)
            {
                mcm_irq = GetMCIRQ(i);
                if (mcm_irq == 0xff) continue;
                if(irq_Setting(mcm_irq, IRQ_LEVEL_TRIGGER + IRQ_DISABLE_INTR) == false)
                {
                    printf("MCM IRQ Setting fail\n"); return false;
                }
                Set_MCIRQ(mcm_irq, i);
            }
        }

	    Global_irq_Init = true;
	}

    // Set TX/RX LED pins and OFF them.
    set_gpio_config_addr();
#if defined (__86DUINO_EX)
    setGPIOFunction(7, 2, &tx_led_data, &tx_led_dir, &tx_led_pin);
	setGPIOFunction(7, 3, &rx_led_data, &rx_led_dir, &rx_led_pin);
    setGPIOFunction(vusbd_onoff_port, vusbd_onoff_pin, &usb_onoff_data, &usb_onoff_dir, &usb_onoff_pin);
#elif defined (__86DUINO_DUO)
    setGPIOFunction(14, 6, &tx_led_data, &tx_led_dir, &tx_led_pin);
	setGPIOFunction(14, 7, &rx_led_data, &rx_led_dir, &rx_led_pin);
#endif
	TX_LED_OFF();
	RX_LED_OFF();
    
#if defined (__86DUINO_DUO)
    delay(200);
#endif
   
    // Setup USBCDC & USBHID
    if (_G_USBD == NOTHING) goto EXITUSBSETTING;

    _usbdev_EX = pci_Find(0x17F3, 0x1060);
    if (_usbdev_EX) {
        usbdev_irq = pci_In8(_usbdev_EX, 0x3C);
        pci_Free(_usbdev_EX);
    } else {
        _usbdev_EX2 = pci_Find(0x17F3, 0x1061);
        if (_usbdev_EX2) {
            usbdev_irq = pci_In8(_usbdev_EX2, 0x3C);
            pci_Free(_usbdev_EX2);
        }
    }
    if (usbdev_irq == 0) usbdev_irq = 0xFF;

    if (usbdev_irq != 0xFF) {
        com_SetUSBCallback(HID_Reinitialize, HID_Uninitialize, _86Duino_suspend_callback, _86Duino_USB_ON_PIN_callback, _86Duino_USB_OFF_PIN_callback);
        handle = com_Init(USB_COM);
        if(handle == NULL)
        {
            printf("USB Device on this CPU but init fail.\n");
            goto EXITUSBSETTING;
        }
        com_SetTimeOut(handle, 0L, 500L); // USB RX timerout is 0ms and TX timeout is 500ms
        
        com_SetControlLineStateCallback(handle, _86Duino_reset_callback); // Add the unique "Reset" function for 86Duino ver.
        com_SetDataInCallback(handle, _86Duino_TXLED_ON_callback); // light the TX LED when DataIn
        com_SetDataOutCallback(handle, _86Duino_RXLED_ON_callback); // light the RX LED when DataOut    
        com_SetSOFCallback(handle, _86Duino_LED_OFF_callback); // Examine the count of TX/RX LED in the SOF signal of ISR.
        
        // Add the unique "TX/RX LED Blink" function for 86Duino ver.
        com_EnableSOFInterrupt(handle); // Enable SOF interrupt to remain 100ms LED Blink.
        if (DEBUG_MSG) printf("USB Device init OK.\n");
    }

EXITUSBSETTING:
#if defined (__86DUINO_EX2)
    // DM&P FIFOLAN initialize
    // setFIFOCOM_BaseAddr();
    if (DEBUG_MSG) printf("FIFOCOM base: 0x%08X\n", nb_Read(0xBC));
    // setFIFOCOM_IRQ();
    if (DEBUG_MSG) printf("FIFOCOM IRQ: 0x%02X\n", sb1_Read8(0xB6));
    
    if (ipcnet_initialize() != 0) {
        printf("ERROR: ipcnet_initialize() failed\n");
        return false;
    }
#endif
    // printf("Exit init()\n");
	// io_Close();
	return true;
}

/*
// only for 86Duino Motion9D board
unsigned short getDeviceID(void) {
	unsigned char mask = 0xC3;
	int port = 7;
	unsigned char data;
	data = io_inpb(GPIODIRBASE + port*4) & mask;
	io_outpb(GPIODIRBASE + port*4, data); // switch GP72, GP73, GP74, GP75 to input direction
	return (io_inpb(GPIODATABASE + port*4) >> 2) & 0x0F;
}
*/
