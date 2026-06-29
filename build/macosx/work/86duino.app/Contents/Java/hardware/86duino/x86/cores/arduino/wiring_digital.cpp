/*
  wdigital.cpp - digital input and output functions for 86Duino
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

#define TRI_STATE     (0x00)
#define PULL_UP       (0x01)
#define PULL_DOWN     (0x02)

#if !defined (__86DUINO_PLC)

void digitalWriteAll(uint32_t) { return; }
uint32_t digitalReadAll(void) { return 0L; }

DMPAPI(bool) isDigitalInit(void) {
    return true;
}

DMPAPI(bool) isRun(void) {
    return true;
}

void digitalInit(void) {
	int i;
	for (i=0; i<PINS; i++)
	{
		if (PIN86[i].gpMode != NOSTATUS) pinMode(i, PIN86[i].gpMode);
		if (PIN86[i].gpMode == OUTPUT && PIN86[i].gpOutput != NOSTATUS) digitalWrite(i, PIN86[i].gpOutput);
	}
}

#else // #if !defined (__86DUINO_PLC)

uint8_t X00 = 255;
uint8_t X01 = 255;
uint8_t X02 = 255;
uint8_t X03 = 255;
uint8_t X04 = 255;
uint8_t X05 = 255;
uint8_t X06 = 255;
uint8_t X07 = 255;
uint8_t X10 = 255;
uint8_t X11 = 255;
uint8_t X12 = 255;
uint8_t X13 = 255;
uint8_t X14 = 255;
uint8_t X15 = 255;
uint8_t X16 = 255;
uint8_t X17 = 255;
uint8_t X20 = 255;
uint8_t X21 = 255;
uint8_t X22 = 255;
uint8_t X23 = 255;
uint8_t X24 = 255;
uint8_t X25 = 255;
uint8_t X26 = 255;
uint8_t X27 = 255;
uint8_t X30 = 255;
uint8_t X31 = 255;
uint8_t X32 = 255;
uint8_t X33 = 255;
uint8_t X34 = 255;
uint8_t X35 = 255;
uint8_t X36 = 255;
uint8_t X37 = 255;

uint8_t Y00 = 255;
uint8_t Y01 = 255;
uint8_t Y02 = 255;
uint8_t Y03 = 255;
uint8_t Y04 = 255;
uint8_t Y05 = 255;
uint8_t Y06 = 255;
uint8_t Y07 = 255;
uint8_t Y10 = 255;
uint8_t Y11 = 255;
uint8_t Y12 = 255;
uint8_t Y13 = 255;
uint8_t Y14 = 255;
uint8_t Y15 = 255;
uint8_t Y16 = 255;
uint8_t Y17 = 255;
uint8_t Y20 = 255;
uint8_t Y21 = 255;
uint8_t Y22 = 255;
uint8_t Y23 = 255;
uint8_t Y24 = 255;
uint8_t Y25 = 255;
uint8_t Y26 = 255;
uint8_t Y27 = 255;
uint8_t Y30 = 255;
uint8_t Y31 = 255;
uint8_t Y32 = 255;
uint8_t Y33 = 255;
uint8_t Y34 = 255;
uint8_t Y35 = 255;
uint8_t Y36 = 255;
uint8_t Y37 = 255;

int GPIO_IN_PORT[4] = { NOUSED, NOUSED, NOUSED, NOUSED };
int GPIO_OUT_PORT[4] = { NOUSED, NOUSED, NOUSED, NOUSED };
static int inPortBaseAddr[4] = { -1, -1, -1, -1 };
static int outPortBaseAddr[4] = { -1, -1, -1, -1 };
static int ctrlPortBaseAddr[4] = { -1, -1, -1, -1 };
static uint32_t lastOutData = 0L;
static int ISAPORT = -1;
static int adcType = -1;

#define GPCS1BASE_ADDR (0x98)
#define FRAM_SIZE      (0x02L)
        
static uint16_t gpioDirBA(int port) {
    #if defined (__86DUINO_EX)
    return GPIODIRBASE + (port * 4);
    #else
    return DIR_BA[port];
    #endif
}

static uint16_t gpioDataBA(int port) {
    #if defined (__86DUINO_EX)
    return GPIODATABASE + (port * 4);
    #else
    return DATA_BA[port];
    #endif
}

static uint16_t gpioCtrlBA(int port) {
    #if defined (__86DUINO_EX)
    return GPIOCTRLBASE + (port * 4);
    #else
    return CONTROL_BA[port];
    #endif
}

static bool digitalInited = false;
DMPAPI(bool) isDigitalInit(void) {
    return digitalInited;
}

DMPAPI(bool) isRun(void) { // QECM02 Run/Stop Switch
    return (analogRead(A2) < 100) ? false : true;
}

enum {
    PLC_A_VER = 0,
    PLC_B_VER,
    PLC_NONE
};

#if defined (__86DUINO_QEC_M02)
DMPAPI(int) check_PLC_version(void) {
    void* pci_handle;
    unsigned short gpioBase;
    
    pci_handle = pci_Alloc(0x00, 0x00, 0x00); // NB configuration space
	if (pci_handle == NULL) { printf("NB device don't exist\n"); return PLC_NONE; }
	gpioBase = pci_In16(pci_handle, NB_GPIO_CBA_REG) & 0xFFFE;
	pci_Free(pci_handle);
    
    if (io_inpw(gpioBase) & (0x01 << 9))  return PLC_A_VER;
    if (io_inpw(gpioBase) & (0x01 << 14)) return PLC_B_VER;
    
    return PLC_NONE;
}
#endif

#define WD_DEBUG    (0)
DMPAPI(void) digitalInit(void) {
    
    unsigned long IO_baseAddr;
    void* io_handle = NULL;
    void* pci_p = NULL;
    int inpinStart;
    
    #if defined (__AI_PLC_8I8O)
    
    GPIO_IN_PORT[0] = 0;
    GPIO_IN_PORT[1] = -1; // 無配置
    GPIO_IN_PORT[2] = -1; // 無配置
    GPIO_IN_PORT[3] = -1; // 無配置
    X00 = 8; X01 = 9; X02 = 10; X03 = 11; X04 = 12; X05 = 13; X06 = 14; X07 = 15;
    
    GPIO_OUT_PORT[0] = 1;
    GPIO_OUT_PORT[1] = -1;
    GPIO_OUT_PORT[2] = -1;
    GPIO_OUT_PORT[3] = -1;
    Y00 = 0; Y01 = 1; Y02 = 2; Y03 = 3; Y04 = 4; Y05 = 5; Y06 = 6; Y07 = 7;
    
    inpinStart = 8;
    
    digitalInited = true;
    
    #elif defined (__86DUINO_QEC_M02)

    // 判斷 AD52 來決定 in/out port 數字
    adcType = analogRead(A2);
    if (WD_DEBUG) printf("adcType: %d\n", adcType);
    if (adcType >= 774 && adcType <= 794) { // adcType = 784，16 IN / 16 OUT
        GPIO_IN_PORT[0] = ISA;
        GPIO_IN_PORT[1] = 0;
        GPIO_IN_PORT[2] = -1; // 無配置
        GPIO_IN_PORT[3] = -1; // 無配置
        ISAPORT = 0;
        X00 = 32; X01 = 33; X02 = 34; X03 = 35; X04 = 36; X05 = 37; X06 = 38; X07 = 39;
        X10 = 40; X11 = 41; X12 = 42; X13 = 43; X14 = 44; X15 = 45; X16 = 46; X17 = 47;
        
        GPIO_OUT_PORT[0] = 6; // 要由雷雕的GPIO編號，小排到大，例如雷雕 GPIO 0~7 是 GPIO9，GPIO 10~17 是 GPIO6，那 GPIO_OUT_PORT[0] = 9，GPIO_OUT_PORT[1] = 6
        
        if (check_PLC_version() == PLC_A_VER)
            GPIO_OUT_PORT[1] = 9; // A ver.
        else
            GPIO_OUT_PORT[1] = 14; // B ver.
        
        GPIO_OUT_PORT[2] = -1; // 無配置
        GPIO_OUT_PORT[3] = -1; // 無配置
        Y00 = 0; Y01 = 1; Y02 = 2;  Y03 = 3;  Y04 = 4;  Y05 = 5;  Y06 = 6;  Y07 = 7;
        Y10 = 8; Y11 = 9; Y12 = 10; Y13 = 11; Y14 = 12; Y15 = 13; Y16 = 14; Y17 = 15;
        
        digitalInited = true;
    } else if (adcType >= 806 && adcType <= 826) { // adcType = 816，24 IN / 8 OUT
        GPIO_IN_PORT[0] = ISA;
        GPIO_IN_PORT[1] = 0;
        GPIO_IN_PORT[2] = 6;
        GPIO_IN_PORT[3] = -1; // 無配置
        ISAPORT = 0;
        X00 = 32; X01 = 33; X02 = 34; X03 = 35; X04 = 36; X05 = 37; X06 = 38; X07 = 39;
        X10 = 40; X11 = 41; X12 = 42; X13 = 43; X14 = 44; X15 = 45; X16 = 46; X17 = 47;
        X20 = 48; X21 = 49; X22 = 50; X23 = 51; X24 = 52; X25 = 53; X26 = 54; X27 = 55;
        
        GPIO_OUT_PORT[0] = 14; // 要由雷雕的GPIO編號，小排到大，例如雷雕 GPIO 0~7 是 GPIO9，GPIO 10~17 是 GPIO6，那 GPIO_OUT_PORT[0] = 9，GPIO_OUT_PORT[1] = 6
        GPIO_OUT_PORT[1] = -1; // 無配置
        GPIO_OUT_PORT[2] = -1; // 無配置
        GPIO_OUT_PORT[3] = -1; // 無配置
        Y00 = 0; Y01 = 1; Y02 = 2;  Y03 = 3;  Y04 = 4;  Y05 = 5;  Y06 = 6;  Y07 = 7;
        
        digitalInited = true;
    } else {
        printf("Program STOP. Check the RUN/STOP button. (value:%d)\n", adcType);
        digitalInited = false;
    }
    
    // Init ISA
    if ((pci_p = pci_Alloc(0x00, 0x07, 0x00)) != NULL) {
        IO_baseAddr = pci_In32(pci_p, GPCS1BASE_ADDR) & 0xFFFFFFFEL;
        if ((io_handle = io_Alloc(IO_USE_PORTIO, IO_baseAddr, FRAM_SIZE)) == NULL) {
            printf("Memory allocation fail\n");
        }
    } else {
        printf("PCI allocation fail\n");
    }
    
    inpinStart = 32;
 
    #else
    printf("Incorrect board!\n");
    return;
    #endif

    // Set GPIO Direction
    for (int i=0; GPIO_IN_PORT[i] != -1; i++) {
        if (GPIO_IN_PORT[i] == ISA) continue;
        inPortBaseAddr[i] = gpioDirBA(GPIO_IN_PORT[i]);
        io_outpb(inPortBaseAddr[i], 0x00);
    }
    
    for (int i=0; GPIO_OUT_PORT[i] != -1; i++) {
        if (GPIO_OUT_PORT[i] == ISA) continue;
        outPortBaseAddr[i] = gpioDirBA(GPIO_OUT_PORT[i]);
        io_outpb(outPortBaseAddr[i], 0xFF);
    }
    
    // Set GPIO Data
    for (int i=0; GPIO_OUT_PORT[i] != -1; i++) {
        if (GPIO_OUT_PORT[i] == ISA) continue;
        outPortBaseAddr[i] = gpioDataBA(GPIO_OUT_PORT[i]);
        io_outpb(outPortBaseAddr[i], 0x00);
        for (int j=0; j<8; j++) {
            PIN86[(i * 8) + j].gpN = GPIO_OUT_PORT[i] * 8 + j;
            PIN86[(i * 8) + j].gpMode = OUTPUT; // 硬體上已經被強制設定為 output，pulseIn 可以避免使用到此類 pin
        }
    }
    
    for (int i=0; GPIO_IN_PORT[i] != -1; i++) {
        if (GPIO_IN_PORT[i] == ISA) {
            if (io_handle != NULL) {
                inPortBaseAddr[i] = IO_baseAddr;
                for (int j=0; j<8; j++) {
                    PIN86[inpinStart + (i * 8) + j].gpN = ISA + j;
                    PIN86[inpinStart + (i * 8) + j].gpMode = INPUT;
                }
            } else {
                printf("No ISA IO Address\n");
            }
            continue;
        }
        inPortBaseAddr[i] = gpioDataBA(GPIO_IN_PORT[i]);
        for (int j=0; j<8; j++) {
            PIN86[inpinStart + (i * 8) + j].gpN = GPIO_IN_PORT[i] * 8 + j;
            PIN86[inpinStart + (i * 8) + j].gpMode = INPUT; // 硬體上已經被強制設定為 input，tone 可以避免使用到此類 pin
        }
    }
    
    lastOutData = 0L;
}

DMPAPI(void) digitalWriteAll(uint32_t val) {
    
    static bool portCombined = false;
    static bool firstCall = true;
    static uint16_t myOutBaseAddr = 0;
    
    io_DisableINT();
    
    // 有連號
    if (portCombined) {
        io_outpw(myOutBaseAddr, val & 0xFFFFL);
        io_RestoreINT();
        return;
    } else {
        if (firstCall) {
            firstCall = false;
            if (adcType >= 774 && adcType <= 794) { // adcType = 784，16 IN / 16 OUT
                if (abs(outPortBaseAddr[0] - outPortBaseAddr[1]) == 1) {
                    portCombined = true;
                    if (outPortBaseAddr[0] > outPortBaseAddr[1])
                        myOutBaseAddr = outPortBaseAddr[1];
                    else
                        myOutBaseAddr = outPortBaseAddr[0];
                    io_outpw(myOutBaseAddr, val & 0xFFFFL);
                    io_RestoreINT();
                    return;
                }
            } // other type
        }
    }
    
    // 無連號
    for (int i=0; outPortBaseAddr[i] != -1; i++) {
        io_outpb(outPortBaseAddr[i], val & 0xFFL);
        val = val >> 8;
    }
    
    io_RestoreINT();

}

DMPAPI(uint32_t) digitalReadAll(void) {
    
    static bool portCombined = false;
    static bool firstCall = true;
    static uint16_t myInBaseAddr = 0;
    
    int adcType = 0; // M02 目前強制使用分開讀
    uint32_t val = 0L;
    int i = 0;
    
    io_DisableINT();
    
    // 有連號
    if (portCombined) {
        val = io_inpw(myInBaseAddr);
        io_RestoreINT();
        return val;
    } else {
        if (firstCall) {
            firstCall = false;
            if (adcType >= 774 && adcType <= 794) { // adcType = 784，16 IN / 16 OUT
                if (abs(inPortBaseAddr[0] - inPortBaseAddr[1]) == 1) {
                    portCombined = true;
                    if (inPortBaseAddr[0] > inPortBaseAddr[1])
                        myInBaseAddr = inPortBaseAddr[1];
                    else
                        myInBaseAddr = inPortBaseAddr[0];
                    val = io_inpw(myInBaseAddr);
                    io_RestoreINT();
                    return val;
                }
            }
        }
    }
    
    // 無連號
    for (; inPortBaseAddr[i] != -1; i++);
    for (; i > 0; i--) {
        val = val << 8;
        val = val | io_inpb(inPortBaseAddr[i-1]);
    }
    
    io_RestoreINT();
    
    return val;
}

#endif

DMPAPI(void) pinMode(uint8_t pin, uint8_t mode) {
	int crossbar_bit;
    unsigned int port;
	if (pin >= PINS || PIN86[pin].gpN == NOUSED) return;

#if defined (__86DUINO_PLC)
    if (pin != POWER_LED_GREEN && pin != POWER_LED_RED) return;
#endif

	crossbar_bit = PIN86[pin].gpN;
	port = crossbar_bit/8;
    
	io_DisableINT();
#if defined (__86DUINO_EX2)
	if (mode == INPUT)
	{
	    io_outpb(CROSSBARBASE + 0x70 + crossbar_bit, TRI_STATE);
	    io_outpb(DIR_BA[port], io_inpb(DIR_BA[port])&~(1<<(crossbar_bit%8)));   
	}
	else if (mode == INPUT_PULLDOWN)
	{
		io_outpb(CROSSBARBASE + 0x70 + crossbar_bit, PULL_DOWN);
	    io_outpb(DIR_BA[port], io_inpb(DIR_BA[port])&~(1<<(crossbar_bit%8)));   
	}
	else if (mode == INPUT_PULLUP)
	{ 
	    io_outpb(CROSSBARBASE + 0x70 + crossbar_bit, PULL_UP);
	    io_outpb(DIR_BA[port], io_inpb(DIR_BA[port])&~(1<<(crossbar_bit%8)));
	}
    else if (mode == INPUT_NOCHANGE)
	{
        // do nothing
    }
	else
	    io_outpb(DIR_BA[port], io_inpb(DIR_BA[port])|(1<<(crossbar_bit%8)));      
#elif defined (__86DUINO_EX)
    if (mode == INPUT)
	{
	    io_outpb(CROSSBARBASE + 0x30 + crossbar_bit, TRI_STATE);
	    io_outpb(GPIODIRBASE + 4*(crossbar_bit/8), io_inpb(GPIODIRBASE + 4*(crossbar_bit/8))&~(1<<(crossbar_bit%8)));   
	}
	else if (mode == INPUT_PULLDOWN)
	{
		io_outpb(CROSSBARBASE + 0x30 + crossbar_bit, PULL_DOWN);
	    io_outpb(GPIODIRBASE + 4*(crossbar_bit/8), io_inpb(GPIODIRBASE + 4*(crossbar_bit/8))&~(1<<(crossbar_bit%8)));   
	}
	else if (mode == INPUT_PULLUP)
	{ 
	    io_outpb(CROSSBARBASE + 0x30 + crossbar_bit, PULL_UP);
	    io_outpb(GPIODIRBASE + 4*(crossbar_bit/8), io_inpb(GPIODIRBASE + 4*(crossbar_bit/8))&~(1<<(crossbar_bit%8)));
	}
    else if (mode == INPUT_NOCHANGE)
	{
        // do nothing
    }
	else
	    io_outpb(GPIODIRBASE + 4*(crossbar_bit/8), io_inpb(GPIODIRBASE + 4*(crossbar_bit/8))|(1<<(crossbar_bit%8)));
#endif
    io_RestoreINT();
}

DMPAPI(void) digitalWrite(uint8_t pin, uint8_t val) {
    unsigned int port;
    unsigned int value;
    int crossbar_bit;
    
	if (pin >= PINS || PIN86[pin].gpN == NOUSED) return;
#if defined (__86DUINO_EX)
    #if defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM)
		if (pin == 32) timer1_pin32_isUsed = true;
	#endif

	crossbar_bit = PIN86[pin].gpN;
    port = GPIODATABASE + 4*(crossbar_bit/8);
    value = 1<<(crossbar_bit%8);

    io_DisableINT();
    
	if (crossbar_bit > 31)
		io_outpb(CROSSBARBASE + 0x80 + (crossbar_bit/8), 0x01);
	else if (crossbar_bit <= 31 && io_inpb(CROSSBARBASE + 0x90 + crossbar_bit) != 0x01)
	{
		io_outpb(CROSSBARBASE + 0x90 + crossbar_bit, 0x01);
		Close_Pwm(pin);
    }

    if (val == LOW) 
        io_outpb(port, io_inpb(port)&(~value));
    else
        io_outpb(port, io_inpb(port)|value);

    io_RestoreINT();
#elif defined (__86DUINO_EX2)
    unsigned char tmp;
    
	crossbar_bit = PIN86[pin].gpN;
    tmp = crossbar_bit/8;
    
    port = CONTROL_BA[tmp];
    value = 1<<(crossbar_bit-(tmp*8));

    io_DisableINT();
    
	if (crossbar_bit > 47)
		io_outpb(CROSSBARBASE + 0xF0 + tmp, 0x08);
	else if (crossbar_bit <= 47 && io_inpb(CROSSBARBASE + 0x100 + crossbar_bit) != 0x08)
	{
		io_outpb(CROSSBARBASE + 0x100 + crossbar_bit, 0x08);
		Close_Pwm(pin);
    }

    if (val == LOW) 
        io_outpb(port + 0x01, value); // 0x01: GPIO PORT Data Clear Register 
    else
        io_outpb(port, value); // 0x00: GPIO PORT Data Set Register

    io_RestoreINT();
#endif
}

DMPAPI(int) digitalRead(uint8_t pin) {
	int crossbar_bit, val;

    if (pin >= PINS || PIN86[pin].gpN == NOUSED) return LOW;
    
#if defined (__86DUINO_PLC)
    if (PIN86[pin].gpN >= ISA) {
        int isapin = PIN86[pin].gpN - ISA;
        return (io_inpb(inPortBaseAddr[ISAPORT]) & (0x01 << isapin)) > 0 ? HIGH : LOW;
    }
#endif

	crossbar_bit = PIN86[pin].gpN;
	
#if defined (__86DUINO_EX)
    #if defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM)
		if (pin == 32) timer1_pin32_isUsed = true;
	#endif

    io_DisableINT();
    
	if (crossbar_bit > 31)
		io_outpb(CROSSBARBASE + 0x80 + (crossbar_bit/8), 0x01);
	else if (crossbar_bit <= 31 && io_inpb(CROSSBARBASE + 0x90 + crossbar_bit) != 0x01)
	{
		io_outpb(CROSSBARBASE + 0x90 + crossbar_bit, 0x01);
		Close_Pwm(pin);
	}  

	val = io_inpb(GPIODATABASE + 4*(crossbar_bit/8))&(1<<(crossbar_bit%8));

    io_RestoreINT();
#elif defined (__86DUINO_EX2)
    unsigned char tmp;
    
    tmp = crossbar_bit/8;
    io_DisableINT();
    
	if (crossbar_bit > 47)
		io_outpb(CROSSBARBASE + 0xF0 + tmp, 0x08);
	else if (crossbar_bit <= 47 && io_inpb(CROSSBARBASE + 0x100 + crossbar_bit) != 0x08)
	{
		io_outpb(CROSSBARBASE + 0x100 + crossbar_bit, 0x08);
		Close_Pwm(pin);
	}  
    
	val = io_inpb(DATA_BA[tmp])&(1<<(crossbar_bit-(tmp*8)));

    io_RestoreINT();
#endif

	if (val != 0) return HIGH;
	return LOW;
}

/*
DMPAPI(void) pinMode(uint8_t pin, uint8_t mode) {
}

DMPAPI(void) digitalWrite(uint8_t pin, uint8_t val) {
    uint32_t data = lastOutData;
    
    if (val > 0)
        data = data | (0x01L << pin);
    else
        data = data & ~(0x01L << pin);

    digitalWriteAll(data);
    lastOutData = data;
}

DMPAPI(int) digitalRead(uint8_t pin) {
    
	uint32_t data = 0L;
    
	data = digitalReadAll();

	if ((data & ((0x01L) << pin)) > 0)
        return HIGH;
    
	return LOW;
}
*/


int digitalPinToInterrupt(int pin) {
    for (int i=0; i<EXTERNAL_NUM_INTERRUPTS; i++)
        if (INTPINSMAP[i] == pin)
            return i;
    return -1;
}
