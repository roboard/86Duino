/*
  wanalog.cpp - analog input and output for 86duino
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include "Arduino.h"
#include "io.h"
#include "dmpcfg.h"
#include "mcm.h"

#define TimeOut		(1000)
#define MCM_MC      (0)
#define MCM_MD      (1)
static int mc_md_inuse[PINS];

#if defined (__86DUINO_EX)
    #define BaseAddress (0xfe00)
    #define ADC_RESOLUTION    (11) // for 86Duino EX
#elif defined (__86DUINO_EX2)
	static unsigned short ADC_BASEADDR[2] = {0, 0};
	#define ADC_RESOLUTION    (12) // for 86Duino DUO
#endif

#define PWM_RESOLUTION    (13) // for 86duino

void analogReference(uint8_t mode) {
	return;
}

static int _readResolution = 10; // Arduino compatibility
static int _writeResolution = 8; // Arduino compatibility
void analogReadResolution(int res) {
	_readResolution = res;
}

void analogWriteResolution(int res) {
	_writeResolution = res;
}

static unsigned long mapResolution(unsigned long value, unsigned long from, unsigned long to) {
	if (from == to)
		return value;
	if (from > to)
		return value >> (from-to);
	else
		return value << (to-from);
}


void Close_Pwm(uint8_t pin) {
	int mc, md;
	if(pin >= PINS || PIN86[pin].gpN == NOUSED || mc_md_inuse[pin] == 0) return;

	mc = PIN86[pin].PWMMC;
	md = PIN86[pin].PWMMD;

    io_DisableINT();
    mcpwm_Disable(mc, md);
    mc_md_inuse[pin] = 0;
    io_RestoreINT();
}

#if defined (__86DUINO_EX2)
void init_ADC(void) {
    int i;
    void* handle;
    unsigned short ADC_Addr;
    
    handle = pci_Alloc(0, 7, 1);
	pci_Out8(handle, 0xde,0x03);  // Not Available for 8051A Access ADC0 & ADC1
	pci_Free(handle);
	
	// if (io_inpdw(HYB_BASEADDRESS + 0x18L) != 0xFFFFFFFF)
    if (_G_ADC[0] == GET)
    {
        ADC_Addr = io_inpw(HYB_BASEADDRESS + 0x18L) & 0xfff0;  // Read base address1
        if (ADC_Addr == 0 || ADC_Addr == 0xfff0)
        {
            io_outpdw(HYB_BASEADDRESS + 0x18L, 0x7010fe01);
            ADC_BASEADDR[0] = 0xfe00;
        }
        else
        {
            io_outpw(HYB_BASEADDRESS + 0x1aL, 0x7010); 
            ADC_BASEADDR[0] = ADC_Addr;
        }
    }
    
    // if (io_inpdw(HYB_BASEADDRESS + 0x1cL) != 0xFFFFFFFF)
    if (_G_ADC[1] == GET)
    {
        ADC_Addr = io_inpw(HYB_BASEADDRESS + 0x1cL) & 0xfff0;  // Read base address1
        if (ADC_Addr == 0 || ADC_Addr == 0xfff0)
        {
            io_outpdw(HYB_BASEADDRESS + 0x1cL, 0x7010fe11);
            ADC_BASEADDR[1] = 0xfe10;
        }
        else
        {
            io_outpw(HYB_BASEADDRESS + 0x1eL, 0x7010); 
            ADC_BASEADDR[1] = ADC_Addr;
        }
    }
    
    if (ADC_BASEADDR[0] != 0) for(i=0; (io_inpb(ADC_BASEADDR[0] + 0x02) & 0x01) != 0 && i<16; i++) io_inpb(ADC_BASEADDR[0] + 0x04); // clear ADC FIFO
    if (ADC_BASEADDR[1] != 0) for(i=0; (io_inpb(ADC_BASEADDR[1] + 0x02) & 0x01) != 0 && i<16; i++) io_inpb(ADC_BASEADDR[1] + 0x04); // clear ADC FIFO
    
    for (i=0; i<PINS; i++) mc_md_inuse[i] = 0;
}
#endif

int analogRead(uint8_t adpin) {
	unsigned long d, t;
	uint8_t pin;
    int ad_index = 0;
	
	if(adpin >= PINS || PIN86[adpin].AD == NOAD) return 0xffff;
	pin = PIN86[adpin].AD;

#if defined (__86DUINO_EX2)
    if (pin > 3) 
    {
        ad_index = 1;
        pin = pin - 4;
    }
    if (ADC_BASEADDR[ad_index] == 0) return 0xffff;
    
    io_DisableINT();
    
	io_outpb(CROSSBARBASE + 0x120 + (ad_index*8) + pin, 0x10);

	io_outpb(ADC_BASEADDR[ad_index] + 0, (0x01<<pin));
	io_outpb(ADC_BASEADDR[ad_index] + 1, 0x01); // enable ADC_ST
	for(t=timer_NowTime(); (io_inpb(ADC_BASEADDR[ad_index] + 2) & 0x01) == 0;)
    {
		if((timer_NowTime() - t) >= 1000L)
        {
			io_RestoreINT();
			return 0xffff;
		}
    }
    d = io_inpw(ADC_BASEADDR[ad_index] + 4);
    // printf("pin = %d\n", (d&0xE000)>>13);
    for(t=timer_NowTime(); (io_inpb(ADC_BASEADDR[ad_index] + 1) & 0x01) > 0;)
    {
		if((timer_NowTime() - t) >= 1000L)
        {
			io_RestoreINT();
			return 0xffff;
		}
    }
    io_RestoreINT();
    
	d = mapResolution((d&0x0fffL), ADC_RESOLUTION, _readResolution);
#elif defined (__86DUINO_EX)
    io_DisableINT();

	io_outpb(BaseAddress + 1, 0x08); // disable ADC
	io_outpb(BaseAddress + 0, (0x01<<pin));
	io_outpb(BaseAddress + 1, 0x01); // enable ADC_ST
	for(t=timer_NowTime();(io_inpb(BaseAddress + 2) & 0x01) == 0;)
    {
		if((timer_NowTime() - t) >= 1000L)
        {
			io_RestoreINT();
			return 0xffff;
		}
    }
    d = io_inpw(BaseAddress + 4);

    io_RestoreINT();

	d = mapResolution((d&0x7ffL), ADC_RESOLUTION, _readResolution);
#endif

	return d;
}

#define MAX_RESOLUTION    (100000L)
void analogWrite(uint8_t pin, uint32_t val) {
	float unit;
	int mc, md;

	if(pin >= PINS || PIN86[pin].gpN == NOUSED) return;
	pinMode(pin, OUTPUT);

	if (val == 0L)
		digitalWrite(pin, LOW);
	else if (val >= ((0x00000001L<<_writeResolution)-1))
		digitalWrite(pin, HIGH);
	else
	{
	    mc = PIN86[pin].PWMMC;
	    md = PIN86[pin].PWMMD;

	    if(mc == NOPWM || md == NOPWM)
		{
			if (val < (0x00000001L<<(_writeResolution-1)))
				digitalWrite(pin, LOW);
			else
				digitalWrite(pin, HIGH);
			return;
	    }

	    // Init H/W PWM
        io_DisableINT();

	    if(mc_md_inuse[pin] == 0)
		{
			mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_CANCEL);
			mcpwm_SetOutMask(mc, md, MCPWM_HMASK_NONE + MCPWM_LMASK_NONE);
			mcpwm_SetOutPolarity(mc, md, MCPWM_HPOL_NORMAL + MCPWM_LPOL_NORMAL);
			mcpwm_SetDeadband(mc, md, 0L);
			mcpwm_ReloadOUT_Unsafe(mc, md, MCPWM_RELOAD_NOW);

			mcpwm_SetWaveform(mc, md, MCPWM_EDGE_A0I1);
			mcpwm_SetSamplCycle(mc, md, 1999L);   // sample cycle: 20ms
	    }

	    val = mapResolution(val, _writeResolution, PWM_RESOLUTION);
	    unit = ((float)MAX_RESOLUTION)/((float)(0x00000001L<<PWM_RESOLUTION));

        if(mcpwm_ReadReloadPWM(mc, md) != 0) mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_CANCEL);
        mcpwm_SetWidth(mc, md, 1000L*SYSCLK, ((float)val)*unit);
	    mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_PEREND);
	    if(mc_md_inuse[pin] == 0)
		{
			mcpwm_Enable(mc, md);
#if defined (__86DUINO_EX)
			io_outpb(CROSSBARBASE + 0x90 + PIN86[pin].gpN, 0x08);
#elif defined (__86DUINO_EX2)
            io_outpb(CROSSBARBASE + 0x100 + PIN86[pin].gpN, 0x01);
#endif
			mc_md_inuse[pin] = 1;
	    }

        io_RestoreINT();
	}
}

// https://www.murata.com.cn/products/productdata/8824151932958/NTHCG578.txt?1702265412000
int temp[] = {-40, -39, -38, -37, -36, -35, -34, -33, -32, -31, -30, -29, -28, -27, -26, -25, -24, -23, -22, -21, -20, -19, -18, -17, -16, -15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150};
int adcVal[] = {1948, 1943, 1937, 1931, 1925, 1919, 1912, 1905, 1897, 1890, 1882, 1874, 1865, 1857, 1847, 1838, 1828, 1818, 1808, 1797, 1786, 1775, 1763, 1751, 1739, 1726, 1713, 1700, 1686, 1672, 1658, 1643, 1628, 1613, 1597, 1581, 1565, 1549, 1532, 1515, 1498, 1480, 1463, 1445, 1427, 1408, 1390, 1371, 1353, 1334, 1315, 1295, 1276, 1257, 1237, 1218, 1198, 1179, 1159, 1140, 1120, 1101, 1082, 1062, 1043, 1024, 1005, 986, 967, 948, 930, 911, 893, 875, 857, 840, 822, 805, 788, 771, 755, 738, 722, 706, 690, 675, 660, 645, 630, 616, 602, 588, 574, 561, 548, 535, 522, 510, 498, 486, 474, 463, 452, 441, 431, 421, 411, 401, 392, 382, 373, 364, 355, 347, 339, 331, 323, 315, 307, 300, 293, 286, 279, 272, 266, 260, 254, 248, 242, 236, 230, 225, 220, 215, 209, 205, 200, 195, 191, 186, 182, 178, 173, 169, 166, 162, 158, 155, 151, 148, 144, 141, 138, 135, 132, 129, 126, 123, 121, 118, 115, 113, 110, 108, 106, 103, 101, 99, 97, 95, 93, 91, 89, 87, 85, 83, 82, 80, 78, 77, 75, 74, 72, 71, 69, 68, 66, 65, 64, 63, 61};

int getTemperature(double data) {
    int i;
    int offset = -5;
    int _size = sizeof(temp)/sizeof(int);
    
    for (i = 0; i < _size; i++) {
        if (adcVal[i] <= data) break;
        if (i == (_size - 1)) break;
    }
    
    return temp[i] + offset;
}

double cpuTemperature(void) {
    #if defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC_M2)
    return getTemperature(analogRead(A7)); // A7 is thermistor
    #else
    return cpuTemperature(DEGREE_C);
    #endif
}

double cpuTemperature(int unit) {
	unsigned long d, TSCV;
	double temperature;

    io_DisableINT();
#if defined (__86DUINO_EX)
    sb1_Write(0xe0, sb1_Read(0xe0) | (0x01L<<21));
	io_outpb(BaseAddress + 1, 0x08); // disable ADC
	io_outpb(BaseAddress + 0, 0x80); // ex: any pin is temperature pin
	io_outpb(BaseAddress + 1, 0x01); // enable ADC_ST
	for(int i=0; i<20; i++) io_inpb(BaseAddress + 2);
	if((io_inpb(BaseAddress + 2) & 0x01) == 0)
    {
        io_RestoreINT();
        return 0xffff;
    }
    d = io_inpw(BaseAddress + 4);
	sb1_Write(0xe0, sb1_Read(0xe0) & ~(0x01L<<21));
#elif defined (__86DUINO_EX2)
    if (_G_ADC[0] == GET)
    {
        char oldData = io_inpb(HYB_BASEADDRESS + 0x1a);
        io_outpb(HYB_BASEADDRESS + 0x1a, oldData | 0x30);

        io_outpb(ADC_BASEADDR[0] + 0, 0x80); // ex: any pin is temperature pin
        io_outpb(ADC_BASEADDR[0] + 1, 0x01); // enable ADC_ST
        for(int i=0; i<20; i++) io_inpb(ADC_BASEADDR[0] + 2);
        if((io_inpb(ADC_BASEADDR[0] + 2) & 0x01) == 0)
        {
            io_RestoreINT();
            return 0xffff;
        }
        d = io_inpw(ADC_BASEADDR[0] + 4);
        for(int i=0; i<20; i++) io_inpb(ADC_BASEADDR[0] + 1);
        if((io_inpb(ADC_BASEADDR[0] + 1) & 0x01) > 0)
        {
            io_RestoreINT();
            return 0xffff;
        }
        io_outpb(HYB_BASEADDRESS + 0x1a, oldData);
    }
    else
    {
        temperature = 0.0;
        goto EXIT;
    }
#endif
	io_RestoreINT();
    
#if defined (__86DUINO_EX)
    temperature = 0.2173913*(d&0x7ffL)-66.0;
#elif defined (__86DUINO_EX2)
    TSCV = ((sb1_Read(0x50) & 0x00FF0000L) >> 16) & 0x000000FFL;
    // printf("ADC read: %ld TSCV: %ld\n", d&0xfffL, TSCV);
	temperature = 0.1372 * (d & 0xfffL) - 191.6 + TSCV;
#endif

	if(unit == DEGREE_F) temperature = (temperature * 1.8) + 32.0;
EXIT:
	return temperature;
}

