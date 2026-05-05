/*
  twi.cpp - TWI/I2C library for 86Duino
  Copyright (c) 2016 Android Lin <acen@dmp.com.tw>. All right reserved.

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
  
  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)
*/

#include <stdio.h>
#include "io.h"
#include "irq.h"
#include "i2c.h"
#include "i2cex.h"
#include "_twi.h"

#define DEV    (2)

static volatile uint8_t twi_state[DEV];
static volatile uint8_t twi_slarw[DEV];
static volatile uint8_t twi_sendStop[DEV];			// should the transaction end with a stop
static volatile uint8_t twi_inRepStart[DEV];			// in the middle of a repeated start

static void (*twi_onSlaveTransmit[DEV])(void);
static void (*twi_onSlaveReceive[DEV])(volatile uint8_t*, int);

static volatile uint8_t twi_masterBufferTX[DEV][TWI_BUFFER_LENGTH];
static volatile uint8_t twi_masterBufferIndexTX[DEV];
static volatile uint8_t twi_masterBufferLengthTX[DEV];

static volatile uint8_t twi_masterBufferRX[DEV][TWI_BUFFER_LENGTH];
static volatile uint8_t twi_masterBufferIndexRX[DEV];
static volatile uint8_t twi_masterBufferLengthRX[DEV];

static volatile uint8_t twi_txBuffer[DEV][TWI_BUFFER_LENGTH];
static volatile uint8_t twi_txBufferIndex[DEV];
static volatile uint8_t twi_txBufferLength[DEV];

static volatile uint8_t twi_rxBuffer[DEV][TWI_BUFFER_LENGTH];
static volatile uint8_t twi_rxBufferIndex[DEV];

static volatile uint8_t twi_error[DEV];

static volatile bool twi_highSpeedMode[DEV] = {false, false};
static volatile bool setHSM[DEV] = {false, false};
static volatile bool twi_timed_out_flag[DEV] = {false, false};
static volatile bool twi_do_reset_on_timeout[DEV] = {false, false};
static volatile uint32_t twi_timeout_us[DEV] = {0L, 0L}; // always wait

static volatile bool ISR_F_SETSTOP = false;
static uint8_t w_wait = 1;
static uint8_t r_wait = 1;
static uint8_t pre_w_wait = 1;
static uint8_t pre_r_wait = 1;

extern unsigned long long _micros(); // in wiring.cpp

// #define DEBUG_MODE // enable "printf" in this .cpp
#define I2C0_IRQ         (0x0A)
unsigned short I2C_IOADDR = 0xFB00;

#if defined (__86DUINO_AI)
	#define I2C_GPIO_DATA    (0xF224)
	#define I2C_GPIO_DIR     (0xF226)
	#define CRSB_BIT_G9      (0x89)
#elif defined (__86DUINO_ZERO) || defined (__86DUINO_ONE) || defined (__86DUINO_EDUCAKE) || defined (__86DUINO_HMI043T) 
	#define I2C_GPIO_DATA    (0xF204)
	#define I2C_GPIO_DIR     (0xF206)
	#define CRSB_BIT_G0      (0x90)
	#define CRSB_BIT_G1      (0x98)
	#define CRSB_BIT_G2      (0xA0)
	#define CRSB_BIT_G3      (0xA8)
	#define I2C_PIN          (0x03) // GP10, GP11
#elif defined (__86DUINO_DUO) || defined (__86DUINO_QEC) || defined (__86DUINO_3DPCOLOR) || defined (__86DUINO_MOTION9D)
    extern int getGPIODataBase(int port); // in wiring.cpp
    extern int getGPIODirBase(int port);
    unsigned short I2C_GPIO_DATA;
    unsigned short I2C_GPIO_DIR;
	#define I2C0_GPIO_DATA    (12) // Port 12 : bit2 is SCL
	#define I2C0_GPIO_DIR     (12) // Port 12 : bit3 is SDA
    #define I2C1_GPIO_DATA    (11) // Port 11 : bit2 is SCL
	#define I2C1_GPIO_DIR     (11) // Port 11 : bit0 is SDA
#elif defined (__86DUINO_QEC_M2) || defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9) || defined (__86DUINO_QEC_15) || defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC_PPC104) || defined (__86DUINO_QEC_PPC9) || defined (__86DUINO_PPC9S) || defined (__86DUINO_PPC15S)
    extern int getGPIODataBase(int port); // in wiring.cpp
    extern int getGPIODirBase(int port);
    unsigned short I2C_GPIO_DATA;
    unsigned short I2C_GPIO_DIR;
	#define I2C0_GPIO_DATA    (3) // Port 3 : bit2 is SDA
	#define I2C0_GPIO_DIR     (3) // Port 3 : bit3 is SCL
    #define I2C1_GPIO_DATA    (8) // Port 8 : bit7 is SCL
	#define I2C1_GPIO_DIR     (8) // Port 8 : bit6 is SDA
#endif

// Internal state machine
#define TWI_READY    (0)
#define TWI_MT_RX    (1)
#define TWI_MT_TX    (2)
#define TWI_SL_RX    (3)
#define TWI_SL_TX    (4)

#define DUMMY_VAL    (0x5A)
static unsigned char int_routing_table[16] = {0xff, 0x08, 0xff, 0x02, 0x04, 0x05, 0x07, 0x06, 0xff, 0x01, 0x03, 0x09, 0x0b, 0xff, 0x0d, 0x0f};

#define NOSETWAIT    (99)

#if defined (DEBUG_MODE)
static volatile int interrupt_times = 0;
static volatile int temp_state = 0;
#endif

#if defined (__86DUINO_AI)
	#define I2CPIN_POSITION_BIT    (0xC0)
	#define I2C_SCL_PIN_BIT        (0x40)
	#define I2C_SDA_PIN_BIT        (0x80)
#elif defined (__86DUINO_ZERO) || defined (__86DUINO_ONE) || defined (__86DUINO_EDUCAKE) || defined (__86DUINO_HMI043T) 
	#define I2CPIN_POSITION_BIT    (0x03)
	#define I2C_SCL_PIN_BIT        (0x04)
	#define I2C_SDA_PIN_BIT        (0x08)
#elif defined (__86DUINO_DUO) || defined (__86DUINO_QEC) || defined (__86DUINO_3DPCOLOR) || defined (__86DUINO_MOTION9D)
    unsigned char I2C_SDA_PIN_BIT;
    unsigned char I2C_SCL_PIN_BIT;
    unsigned char I2CPIN_POSITION_BIT;
	#define I2C0PIN_POSITION_BIT   (0x0C)
    #define I2C1PIN_POSITION_BIT   (0x05)
	#define I2C0_SCL_PIN_BIT       (0x04) // bit2: SCL
	#define I2C0_SDA_PIN_BIT       (0x08) // bit3: SDA
    #define I2C1_SCL_PIN_BIT       (0x04) // bit2: SCL
	#define I2C1_SDA_PIN_BIT       (0x01) // bit0: SDA
#elif defined (__86DUINO_QEC_M2) || defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9) || defined (__86DUINO_QEC_15) || defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC_PPC104) || defined (__86DUINO_QEC_PPC9) || defined (__86DUINO_PPC9S) || defined (__86DUINO_PPC15S)
    unsigned char I2C_SDA_PIN_BIT;
    unsigned char I2C_SCL_PIN_BIT;
    unsigned char I2CPIN_POSITION_BIT;
	#define I2C0PIN_POSITION_BIT   (0x0C)
    #define I2C1PIN_POSITION_BIT   (0xC0)
	#define I2C0_SCL_PIN_BIT       (0x08) // bit2: SDA
	#define I2C0_SDA_PIN_BIT       (0x04) // bit3: SCL
    #define I2C1_SCL_PIN_BIT       (0x80) // bit7: SCL
	#define I2C1_SDA_PIN_BIT       (0x40) // bit6: SDA
#endif

void twi_releaseBus(uint8_t dev);

static void set_pins(int dev, int SCL, int SDA) {
    unsigned char mask, dir, val;
#if defined (__86DUINO_EX)
    mask = ~(I2CPIN_POSITION_BIT << (dev*2));
    dir  = 0;
    val  = (unsigned char)((SDA<<1) + SCL) << (dev*2);
    
    if (SCL == 0) dir = dir | (I2C_SCL_PIN_BIT << (dev*2));  // if SCL == 0, set the SCL to output 0, if SCL == 1, remain pin as INPUT and Tri-state. 
    if (SDA == 0) dir = dir | (I2C_SDA_PIN_BIT << (dev*2));  // if SDA == 0, set the SDA to output 0, if SDA == 1, remain pin as INPUT and Tri-state.
    
    io_outpb(I2C_GPIO_DIR,  (io_inpb(I2C_GPIO_DIR)  & mask) | dir);
    io_outpb(I2C_GPIO_DATA, (io_inpb(I2C_GPIO_DATA) & mask) | val);
#elif defined (__86DUINO_EX2)
    dir  = 0;
    if (dev == 0)
    {
        mask = ~I2C0PIN_POSITION_BIT;
        val  = (unsigned char)((SDA<<1) + SCL) << 2;
        
        if (SCL == 0) dir = dir | I2C0_SCL_PIN_BIT;  // if SCL == 0, set the SCL to output 0, if SCL == 1, remain pin as INPUT and Tri-state. 
        if (SDA == 0) dir = dir | I2C0_SDA_PIN_BIT;  // if SDA == 0, set the SDA to output 0, if SDA == 1, remain pin as INPUT and Tri-state.
        
        io_outpb(I2C0_GPIO_DIR,  (io_inpb(I2C0_GPIO_DIR)  & mask) | dir);
        io_outpb(I2C0_GPIO_DATA, (io_inpb(I2C0_GPIO_DATA) & mask) | val);
    }
    else if (dev == 1)
    {
        mask = ~I2C1PIN_POSITION_BIT;
        val  = (unsigned char)((SDA<<2) + SDA);
        
        if (SCL == 0) dir = dir | I2C1_SCL_PIN_BIT;  // if SCL == 0, set the SCL to output 0, if SCL == 1, remain pin as INPUT and Tri-state. 
        if (SDA == 0) dir = dir | I2C1_SDA_PIN_BIT;  // if SDA == 0, set the SDA to output 0, if SDA == 1, remain pin as INPUT and Tri-state.
        
        io_outpb(I2C1_GPIO_DIR,  (io_inpb(I2C1_GPIO_DIR)  & mask) | dir);
        io_outpb(I2C1_GPIO_DATA, (io_inpb(I2C1_GPIO_DATA) & mask) | val);
    }
#endif
}

#if defined (__86DUINO_EX2)
#define HYB_BASEADDRESS    (0x8A00)
static unsigned BaseAddress[2];
static unsigned short i2c0_SetBaseAddress(unsigned short baseaddr) {
    io_outpdw(HYB_BASEADDRESS + 0x10, (io_inpdw(HYB_BASEADDRESS + 0x10) | 0x80000000L));
    
    if ((io_inpw(HYB_BASEADDRESS + 0x10) & 0xfff8) != 0) {
        baseaddr = io_inpw(HYB_BASEADDRESS + 0x10) & 0xfff8;
    }
    
    io_outpw(HYB_BASEADDRESS + 0x10, baseaddr | 0x01);
    BaseAddress[0] = baseaddr;
    
    return BaseAddress[0];
}

static unsigned short i2c1_SetBaseAddress(unsigned short baseaddr) {
    io_outpdw(HYB_BASEADDRESS + 0x14, (io_inpdw(HYB_BASEADDRESS + 0x14) | 0x80000000L));
    
    if ((io_inpw(HYB_BASEADDRESS + 0x14) & 0xfff8) != 0) {
        baseaddr = io_inpw(HYB_BASEADDRESS + 0x14) & 0xfff8;
    }
    
    io_outpw(HYB_BASEADDRESS + 0x14, baseaddr | 0x01);
    BaseAddress[1] = baseaddr;
    
    return BaseAddress[1];
}

static void i2c0_SetIRQ(int i2cirq) {
    io_outpdw(HYB_BASEADDRESS + 0x10L, (io_inpdw(HYB_BASEADDRESS + 0x10L) & 0xfff0ffff) | (i2cirq) << 16);
}

static void i2c1_SetIRQ(int i2cirq) {
    io_outpdw(HYB_BASEADDRESS + 0x14L, (io_inpdw(HYB_BASEADDRESS + 0x14L) & 0xfff0ffff) | (i2cirq) << 16);
}
#endif

#define TEST_T    (200)
#define READWRITE_TIMEOUT (333)
bool init_OK[DEV] = {false, false};
int _dev0 = 0;
int _dev1 = 1;
int usedirq = 0;

/* 
 * Function twi_init
 * Desc     readys twi pins and sets twi bitrate
 * Input    none
 * Output   none
 */
// void twi_init(void) <= Arduino origin function type
void _twi_init(uint8_t dev, uint32_t twi_speed)
{
	int irqnum, i;
	unsigned long nowtime = 0L;
    void* I2C_p;

	#if defined (DEBUG_MODE)
	printf("I2C init start\n");
	#endif

	if (init_OK[dev] == true)
	{
		Serial.println("Init fail: I2C is working now");
		return;
	}
		
	// set I2C device	
	if (dev == 0)
	{
	#if defined (__86DUINO_EX)
        i2c_SetBaseAddress(I2C_IOADDR);
        
        if ((sb_Read8(0xD6) & 0x0F) == 0 || (sb_Read8(0xD6) & 0x0F) == 0x0F)
		{
			usedirq = I2C0_IRQ;
			i2c_SetIRQ(int_routing_table[I2C0_IRQ], I2CIRQ_DISABLE);
		}
		else
		{
			irqnum = sb_Read8(0xD6) & 0x0F;
			for (i=0; i<16; i++)
				if (int_routing_table[i] == irqnum) break;
			if (i != 16)
				usedirq = i;
			else
			{
				usedirq = I2C0_IRQ;
				i2c_SetIRQ(int_routing_table[I2C0_IRQ], I2CIRQ_DISABLE);
			}
		}
	}
	#elif defined (__86DUINO_EX2)
        if (_G_I2C[0] == NOTHING) return;
        
        I2C_IOADDR = i2c0_SetBaseAddress(I2C_IOADDR);
        i2c01_SetBaseAddress(I2C_IOADDR, 0);
        // printf("Base0:%04X\n", I2C_IOADDR);
		if ((io_inpb(HYB_BASEADDRESS + 0x12) & 0x0F) == 0 || (io_inpb(HYB_BASEADDRESS + 0x12) & 0x0F) == 0x0F)
			usedirq = I2C0_IRQ;
		else
		{
			irqnum = io_inpb(HYB_BASEADDRESS + 0x12) & 0x0f;
			for (i=0; i<16; i++)
				if (int_routing_table[i] == irqnum) break;
			if (i != 16)
				usedirq = i;
			else
				usedirq = I2C0_IRQ;
		}
        i2c0_SetIRQ(int_routing_table[usedirq]);
        
        I2C_SDA_PIN_BIT     = I2C0_SDA_PIN_BIT;
        I2C_SCL_PIN_BIT     = I2C0_SCL_PIN_BIT;
        I2CPIN_POSITION_BIT = I2C0PIN_POSITION_BIT;
        I2C_GPIO_DATA       = getGPIODataBase(I2C0_GPIO_DATA);
        I2C_GPIO_DIR        = getGPIODirBase(I2C0_GPIO_DIR);
	}
	else if (dev == 1)
	{
        if (_G_I2C[1] == NOTHING) return;
        
        I2C_IOADDR = i2c1_SetBaseAddress(I2C_IOADDR+8);
        i2c01_SetBaseAddress(0, I2C_IOADDR);
        // printf("Base1:%04X\n", I2C_IOADDR);
        usedirq = 6; // The I2C1 IRQ in BIOS is 10, but it will work fail when I plug LAN wire in RJ45, so here, I set it to 6 forcely.
        io_outpb(HYB_BASEADDRESS + 0x16, (io_inpb(HYB_BASEADDRESS + 0x16) & ~(0x0f)) | int_routing_table[usedirq]);
		if ((io_inpb(HYB_BASEADDRESS + 0x16) & 0x0F) == 0 || (io_inpb(HYB_BASEADDRESS + 0x16) & 0x0F) == 0x0F)
			usedirq = I2C0_IRQ;
		else
		{
			irqnum = io_inpb(HYB_BASEADDRESS + 0x16) & 0x0f;
			for (i=0; i<16; i++)
				if (int_routing_table[i] == irqnum) break;
			if (i != 16)
				usedirq = i;
			else
				usedirq = I2C0_IRQ;
		}
        
        i2c1_SetIRQ(int_routing_table[usedirq]);
        
        I2C_SDA_PIN_BIT     = I2C1_SDA_PIN_BIT;
        I2C_SCL_PIN_BIT     = I2C1_SCL_PIN_BIT;
        I2CPIN_POSITION_BIT = I2C1PIN_POSITION_BIT;
        I2C_GPIO_DATA       = getGPIODataBase(I2C1_GPIO_DATA);
        I2C_GPIO_DIR        = getGPIODirBase(I2C1_GPIO_DIR);
	}
	#endif
    
    #if defined (DEBUG_MODE)
	printf("Use I2C%d IRQ %d for I2C ISR.\n", dev, usedirq);
    #endif

	io_outpb(I2C_GPIO_DIR, io_inpb(I2C_GPIO_DIR) & ~(I2CPIN_POSITION_BIT)); // change GPIO(SCL & SDA) DIR to INPUT

    // change pin function to GPIO.
    #if defined (__86DUINO_AI)
		io_outpb(CROSSBARBASE + CRSB_BIT_G9, 0x01);
	#elif defined (__86DUINO_ZERO) || defined (__86DUINO_ONE) || defined (__86DUINO_EDUCAKE) || defined (__86DUINO_HMI043T) 
		io_outpw(CROSSBARBASE + CRSB_BIT_G1 + 0, 0x0101);
	#elif defined (__86DUINO_DUO) || defined (__86DUINO_QEC) || defined (__86DUINO_3DPCOLOR) || defined (__86DUINO_MOTION9D)
        if (dev == 0)
            io_outpb(CROSSBARBASE + 0xFC, 0x08);
        else if (dev == 1)
            io_outpb(CROSSBARBASE + 0xFB, 0x08);
    #elif defined (__86DUINO_QEC_M2) || defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9) || defined (__86DUINO_QEC_15) || defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC_PPC104) || defined (__86DUINO_QEC_PPC9) || defined (__86DUINO_PPC9S) || defined (__86DUINO_PPC15S)
        if (dev == 0) {
            io_outpb(CROSSBARBASE + 0x11A, 0x08);
            io_outpb(CROSSBARBASE + 0x11B, 0x08);
        } else if (dev == 1)
            io_outpb(CROSSBARBASE + 0xF8, 0x08);
    #endif

	// check SCL is HIGH or LOW (Normally, it should be HIGH)
	if ((io_inpb(I2C_GPIO_DATA) & I2C_SCL_PIN_BIT) == 0x00)
	{
		Serial.println("ERROR: I2C's SCL is LOW during initial process");
		return;
	}
    
	// check SDA is HIGH or LOW (Normally, it should be HIGH)	
	if ((io_inpb(I2C_GPIO_DATA) & I2C_SDA_PIN_BIT) == 0x00)
	{
	    Serial.println("I2C's SDA is LOW, we try to repair it");
		for (i=0; i<TEST_T; i++)
	    {
	    	set_pins(dev, 1, 1);
	    	timer_DelayMicroseconds(100);
	    	set_pins(dev, 0, 1);
	    	timer_DelayMicroseconds(100);
	    	if ((io_inpb(I2C_GPIO_DATA) & I2C_SDA_PIN_BIT) == I2C_SDA_PIN_BIT) break;
		}
	
		if (i != TEST_T)
		{
			Serial.println("Repair completed");
			// use GPIO to send STOP condition
			set_pins(dev, 0, 0);
			timer_DelayMicroseconds(100);
			set_pins(dev, 1, 0);
			timer_DelayMicroseconds(100);
			set_pins(dev, 1, 1);
			timer_DelayMicroseconds(100);
			
			// use GPIO to send START condition
			set_pins(dev, 1, 1);
			timer_DelayMicroseconds(100);
			set_pins(dev, 1, 0);
			timer_DelayMicroseconds(100);
			set_pins(dev, 0, 0);
			timer_DelayMicroseconds(100);
			
			// use GPIO to send STOP condition again
			set_pins(dev, 0, 0);
			timer_DelayMicroseconds(100);
			set_pins(dev, 1, 0);
			timer_DelayMicroseconds(100);
			set_pins(dev, 1, 1);
			timer_DelayMicroseconds(100);
		}
		else
		{
		    Serial.println("Repair fail: please try to reset 86Duino");
			return;
		}
	}

	if (i2c_Reset(dev) == false)  // assume the status of GPIO/I2C pins are GPIO "IN" or "OUT 1"
	{
	    //i2c_Close();
	    #if defined (DEBUG_MODE)
		printf("can't reset the I2C modules\n");
		#endif
		return;
	}


    io_outpb(I2C_IOADDR + 0x07, (io_inpb(I2C_IOADDR + 0x07) & 0xf0) | 0x07); // I2C1 Extra Control Register
                                                                         // DIMC:  Enaable  (0)
                                                                         // DI196: Disaable (1)
                                                                         // DIAR:  Enaable  (1)
                                                                         // DIDC:  Disaable (1)
	i2c_ClearSTAT(dev, I2CSTAT_ALL);
	i2c_DisableNoiseFilter(dev);
	i2c_DisableStandardHSM(dev);

	if (twi_speed > 400000L) twi_highSpeedMode[dev] = true; // Select high speed mode
	i2c_SetSpeed(dev, I2CMODE_AUTO, twi_speed);

	i2cslave_SetAddr(dev, 0x7f);
	i2cslave_EnableACK(dev);

    // switch form GPIO(RICH_IO) to SCL & SDA
	#if defined (__86DUINO_AI)
		io_outpb(CROSSBARBASE + CRSB_BIT_G9, 0x08);
	#elif defined (__86DUINO_ZERO) || defined (__86DUINO_ONE) || defined (__86DUINO_EDUCAKE) || defined (__86DUINO_HMI043T) 
		io_outpw(CROSSBARBASE + CRSB_BIT_G1 + 0, 0x0808);
	#elif defined (__86DUINO_DUO) || defined (__86DUINO_QEC) || defined (__86DUINO_3DPCOLOR) || defined (__86DUINO_MOTION9D)
        if (dev == 0)
            io_outpb(CROSSBARBASE + 0xFC, 0x01); // change pin function to RICH-IO G0
        else
            io_outpb(CROSSBARBASE + 0xFB, 0x01); // change pin function to RICH-IO G0
    #elif defined (__86DUINO_QEC_M2) || defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9) || defined (__86DUINO_QEC_15) || defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC_PPC104) || defined (__86DUINO_QEC_PPC9) || defined (__86DUINO_PPC9S) || defined (__86DUINO_PPC15S)
        if (dev == 0) {
            io_outpb(CROSSBARBASE + 0x11A, 0x01);
            io_outpb(CROSSBARBASE + 0x11B, 0x01);
        } else if (dev == 1)
            io_outpb(CROSSBARBASE + 0xF8, 0x01);
	#endif

    // set I2C_p pointer that can be used in ISR()
    #if defined (__86DUINO_EX2)
        if (dev == 0) I2C_p = (void*)(&_dev0);
        else I2C_p = (void*)(&_dev1);
    #else
        I2C_p = (void*)(&_dev0);
    #endif
    
    if (irq_Init() == false) 
    {
        #if defined (DEBUG_MODE)
		printf("irq_init fail\n");
		#endif
		return;
    }
   
    if (irq_Setting(usedirq, IRQ_LEVEL_TRIGGER) == false)
    {
    	#if defined (DEBUG_MODE)
        printf("%s\n", __FUNCTION__);
		#endif
		return;
    }
    
    if (irq_InstallISR(usedirq, I2C_ISR, I2C_p) == false)
    {
    	#if defined (DEBUG_MODE)
        printf("irq_install fail\n");
		#endif
		return;
    }
    
    // initialize state
	twi_state[dev] = TWI_READY;
	twi_sendStop[dev] = true;		// default value
	twi_inRepStart[dev] = false;
    
    i2c_EnableINT(dev, I2CINT_ALL);
   
    init_OK[dev] = true;
    
    /* // debug
	printf("I2C init success\n");
	for (int i=0; i<256; i++)
	{
		if (i%16 == 0) printf("\n");
		printf("%02X ", io_inpb(I2C_IOADDR + i));
	}
	*/
}

void twi_init(uint8_t dev) // Arduino compaliable function
{
	_twi_init(dev, 100000L);
}

void twi_init(uint8_t dev, uint32_t speed) // VertexEX's special function
{
	_twi_init(dev, speed);
}

void twi_disable(uint8_t dev)
{
    twi_releaseBus(dev);
    
    io_DisableINT();
    i2c_ClearSTAT(dev, I2CSTAT_ALL);
    i2c_DisableINT(dev, I2CINT_ALL);
    if (dev == 0)
        irq_UninstallISR(usedirq, (void *)&_dev0);
    else
        irq_UninstallISR(usedirq, (void *)&_dev1);
    io_RestoreINT();
   
    init_OK[dev] = false;
}

void twi_setFrequency(uint8_t dev, uint32_t twi_speed)
{
    if (twi_speed > 400000L) twi_highSpeedMode[dev] = true; // Select high speed mode
    i2c_SetSpeed(dev, I2CMODE_AUTO, twi_speed);
}

void twi_setTimeoutInMicros(uint8_t dev, uint32_t timeout, bool reset_with_timeout)
{
    twi_timed_out_flag[dev] = false;
    twi_timeout_us[dev] = timeout;
    twi_do_reset_on_timeout[dev] = reset_with_timeout;
}

void twi_handleTimeout(uint8_t dev, bool reset)
{
    twi_timed_out_flag[dev] = true;
    
    #if defined (DEBUG_MODE)
    printf("twi_stop: interrupt times = %d status = 0x%02X\n", interrupt_times, temp_state);
    #endif
    
    if (reset) {
        // reset the interface
        twi_disable(dev);
        timer_Delay(100);
        twi_init(dev);
    }
}

bool twi_manageTimeoutFlag(uint8_t dev, bool clear_flag)
{
    bool flag = twi_timed_out_flag[dev];
    if (clear_flag)
        twi_timed_out_flag[dev] = false;
    return flag;
}

void twi_wait_mode(uint8_t write_wait, uint8_t read_wait) {
  if (write_wait != 0) w_wait = true; else w_wait = false;
  if (read_wait  != 0) r_wait = true; else r_wait = false;
}

void twi_write_wait(uint8_t wait) {
  pre_w_wait = w_wait;
  if (wait != NOSETWAIT) w_wait = wait;
}

/* 
 * Function twi_slaveInit
 * Desc     sets slave address and enables interrupt
 * Input    none
 * Output   none
 */
void twi_setAddress(uint8_t dev, uint8_t address)
{
  // set twi slave address (skip over TWGCE bit)
  i2cslave_SetAddr(dev, address);
}

// Vortex86EX I2C status Register
#define MT_RXRDY             (I2CSTAT_RXRDY + 0x03)
#define MT_RXRDY_NOBUSY      (I2CSTAT_RXRDY + 0x01)
#define MT_TX_DONE  	     (I2CSTAT_TXDONE + 0x03)
#define MT_TX_DONE_NOBUSY  	 (I2CSTAT_TXDONE + 0x01)
#define MT_TXDONE_NACK 	     (I2CSTAT_TXDONE + I2CSTAT_ACKERR +0x03)
#define MT_ARB_LOST 	     (I2CSTAT_ARLOSS + 0x03)
#define SL_NEEDTOW  	     (I2CSTAT_SLAVEWREQ + 0x02)
#define SL_RXRDY    	     (I2CSTAT_RXRDY + 0x02)
#define SL_TX_DONE  	     (I2CSTAT_TXDONE + 0x02)
#define SL_NACK_ERR 	     (I2CSTAT_ACKERR + 0x02)
#define SL_ARB_LOST 	     (I2CSTAT_ARLOSS + 0x02)
#define SL_GET_STOP 	     (I2CSTAT_SLAVESTOPPED + 0x01)

// error code
#define MT_ADDR_NACK    (0)
#define MT_DATA_NACK    (1)
#define MT_ARB_LT       (2)
#define MT_TX_NOBUSY    (3)

static bool send_twi_addr = false;
static uint8_t* _user_data = NULL;

/* 
 * Function twi_readFrom
 * Desc     attempts to become twi bus master and read a
 *          series of bytes from a device on the bus
 * Input    address: 7bit i2c device address
 *          data: pointer to byte array
 *          length: number of bytes to read into array
 *          sendStop: Boolean indicating whether to send a stop at the end
 * Output   number of bytes read
 */
uint8_t twi_readFrom(uint8_t dev, uint8_t address, uint8_t* data, uint8_t length, uint8_t sendStop, uint8_t wait)
{
  int i; 
  unsigned int start_t;
  
  #if defined (DEBUG_MODE)
  printf("I2C readfrom start\n");
  #endif
  
  if (init_OK[dev] == false) return 0;

  // ensure data will fit into buffer
  if (TWI_BUFFER_LENGTH < length) {
    return 0;
  }
  
  // wait until twi is ready, become master receiver
  start_t = _micros();
  while(TWI_READY != twi_state[dev]) {
    if ((twi_timeout_us[dev] > 0L) && ((_micros() - start_t) > twi_timeout_us[dev])) {
      twi_handleTimeout(dev, twi_do_reset_on_timeout[dev]);
      printf("\nRead START: twi_state[dev]=%X\n", twi_state[dev]);
      return 0;
    }
    
    if (ISR_F_SETSTOP) {
      while (i2cmaster_CheckStopBit(dev) == true);
      ISR_F_SETSTOP = false;
      twi_state[dev] = TWI_READY;
      setHSM[dev] = false;
    }
  }

  pre_r_wait = r_wait;
  if (wait != NOSETWAIT) r_wait = wait;
  
  twi_state[dev] = TWI_MT_RX;
  twi_sendStop[dev] = sendStop;
  // reset error state (0xFF.. no error occured)
  twi_error[dev] = 0xFF;

  // initialize buffer iteration vars
  twi_masterBufferIndexRX[dev] = 0;
  twi_masterBufferLengthRX[dev] = length;  // This is not intuitive, read on...
  // On receive, the previously configured ACK/NACK setting is transmitted in
  // response to the received byte before the interrupt is signalled. 
  // Therefor we must actually set NACK when the _next_ to last byte is
  // received, causing that NACK to be sent in response to receiving the last
  // expected byte of data.

  // build sla+w, slave device address + w bit
  twi_slarw[dev] = address;
  
  if (true == twi_inRepStart[dev]) {
    // if we're in the repeated start state, then we've already sent the start,
    // (@@@ we hope), and the TWI statemachine is just waiting for the address byte.
    // We need to remove ourselves from the repeated start state before we enable interrupts,
    // since the ISR is ASYNC, and we could get confused if we hit the ISR before cleaning
    // up. Also, don't enable the START interrupt. There may be one pending from the 
    // repeated start that we sent outselves, and that would really confuse things.
    twi_inRepStart[dev] = false;			// remember, we're dealing with an ASYNC ISR
    i2c_EnableINT(dev, I2CINT_TXDONE | I2CINT_ACKERR | I2CINT_ARLOSS);
  }
  else
  {
    // send start condition
    if (twi_highSpeedMode[dev] == true && setHSM[dev] == false)
	{
		i2cmaster_WriteAddrREG(dev, 0x07, I2C_WRITE);
		while(setHSM[dev] == false);
		#if defined (DEBUG_MODE)
		printf("MT_READ: set high speed mode.\n");
		#endif
    }
    send_twi_addr = true;
	i2cmaster_WriteAddrREG(dev, twi_slarw[dev], I2C_READ);
  }
  
  // wait for read operation to complete
  start_t = _micros();
  while (r_wait && (TWI_MT_RX == twi_state[dev])) {
    if ((twi_timeout_us[dev] > 0L) && ((_micros() - start_t) > twi_timeout_us[dev])) {
      twi_handleTimeout(dev, twi_do_reset_on_timeout[dev]);
      printf("\nRead END: twi_state[dev]=%X\n", twi_state[dev]);
      return 0;
    }
    
    if (ISR_F_SETSTOP) {
      while (i2cmaster_CheckStopBit(dev) == true);
      ISR_F_SETSTOP = false;
      twi_state[dev] = TWI_READY;
    }
  }

  if (r_wait == false) {
    _user_data = data;
    return length;
  }
  
  if (twi_masterBufferIndexRX[dev] < length)
    length = twi_masterBufferIndexRX[dev];

  // copy twi buffer to data
  for (i = 0; i < length; ++i)
    data[i] = twi_masterBufferRX[dev][i];

  #if defined (DEBUG_MODE)
  printf("I2C readfrom success\n");
  #endif	
  return length;
}


/* 
 * Function twi_writeTo
 * Desc     attempts to become twi bus master and write a
 *          series of bytes to a device on the bus
 * Input    address: 7bit i2c device address
 *          data: pointer to byte array
 *          length: number of bytes in array
 *          wait: boolean indicating to wait for write or not
 *          sendStop: boolean indicating whether or not to send a stop at the end
 * Output   0 .. success
 *          1 .. length to long for buffer
 *          2 .. address send, NACK received
 *          3 .. data send, NACK received
 *          4 .. other twi error (lost bus arbitration, bus error, ..)
 *          5 .. timeout
 *          6 .. nonblock, user need to call twi_isBusy() to know whether the process is complete or not
 */
uint8_t twi_writeTo(uint8_t dev, uint8_t address, uint8_t* data, uint8_t length, uint8_t wait, uint8_t sendStop)
{
  uint8_t i;
  unsigned int start_t;
  
  #if defined (DEBUG_MODE)
  printf("I2C writeto start\n");
  #endif
  
  if (init_OK[dev] == false) return 4;

  // ensure data will fit into buffer
  if (TWI_BUFFER_LENGTH < length) {
    return 1;
  }

  // wait until twi is ready, become master transmitter
  start_t = _micros();
  while (TWI_READY != twi_state[dev]) {
    if ((twi_timeout_us[dev] > 0L) && ((_micros() - start_t) > twi_timeout_us[dev])) {
      twi_handleTimeout(dev, twi_do_reset_on_timeout[dev]);
      printf("\nWrite START: twi_state[dev]=%X\n", twi_state[dev]);
      return 5;
    }
    
    if (ISR_F_SETSTOP) {
      while (i2cmaster_CheckStopBit(dev) == true);
      ISR_F_SETSTOP = false;
      twi_state[dev] = TWI_READY;
      setHSM[dev] = false;
    }
  }

  twi_state[dev] = TWI_MT_TX;
  twi_sendStop[dev] = sendStop;
  // reset error state (0xFF.. no error occured)
  twi_error[dev] = 0xFF;

  // initialize buffer iteration vars
  twi_masterBufferIndexTX[dev] = 0;
  twi_masterBufferLengthTX[dev] = length;

  // copy data to twi buffer
  for (i = 0; i < length; ++i){
    twi_masterBufferTX[dev][i] = data[i];
  }

  // build sla+w, slave device address + w bit
  twi_slarw[dev] = address;

  // if we're in a repeated start, then we've already sent the START
  // in the ISR. Don't do it again.
  //
  if (true == twi_inRepStart[dev]) {
    // if we're in the repeated start state, then we've already sent the start,
    // (@@@ we hope), and the TWI statemachine is just waiting for the address byte.
    // We need to remove ourselves from the repeated start state before we enable interrupts,
    // since the ISR is ASYNC, and we could get confused if we hit the ISR before cleaning
    // up. Also, don't enable the START interrupt. There may be one pending from the 
    // repeated start that we sent outselves, and that would really confuse things.
    twi_inRepStart[dev] = false;			// remember, we're dealing with an ASYNC ISR
    i2c_EnableINT(dev, I2CINT_TXDONE | I2CINT_ACKERR | I2CINT_ARLOSS);
  }
  else
  {
  	if (twi_highSpeedMode[dev] == true && setHSM[dev] == false)
	{
	  i2cmaster_WriteAddrREG(dev, 0x07, I2C_WRITE);
	  while(setHSM[dev] == false);
	  #if defined (DEBUG_MODE)
	  printf("MT_WRITE: set high speed mode.\n");
	  #endif
    }

	send_twi_addr = true;
    // send start condition
    i2cmaster_WriteAddrREG(dev, twi_slarw[dev], I2C_WRITE);
  }

  // wait for write operation to complete
  start_t = _micros();
  while (w_wait && (TWI_MT_TX == twi_state[dev])) {
    if ((twi_timeout_us[dev] > 0L) && ((_micros() - start_t) > twi_timeout_us[dev])) {
      twi_handleTimeout(dev, twi_do_reset_on_timeout[dev]);
      printf("\nWrite END: twi_state[dev]=%X\n", twi_state[dev]);
      return 5;
    }

    if (ISR_F_SETSTOP) {
      while (i2cmaster_CheckStopBit(dev) == true);
      ISR_F_SETSTOP = false;
      twi_state[dev] = TWI_READY;
    }
  }

  if (w_wait == false) return 6;
  
  #if defined (DEBUG_MODE)
  printf("I2C writeto success\n");
  printf("Interrupt times = %d status = 0x%02X\n", interrupt_times, temp_state);
  #endif
  
  if (twi_error[dev] == 0xFF)
    return 0;	// success
  else if (twi_error[dev] == MT_ADDR_NACK)
    return 2;	// error: address send, nack received
  else if (twi_error[dev] == MT_DATA_NACK)
    return 3;	// error: data send, nack received
  else {
    // printf("twi_error[dev]=%X\n", twi_error[dev]);
    return 4;	// other twi error
  }
}

bool twi_isBusy(uint8_t dev) {
  bool ret = true;
  uint8_t length;
  int i;
  
  if (twi_state[dev] == TWI_READY) {
    ret = false;
    w_wait = pre_w_wait;
    r_wait = pre_r_wait;
  } else if (twi_state[dev] == TWI_MT_TX) {
    if (ISR_F_SETSTOP) {
      if (i2cmaster_CheckStopBit(dev) == false) {
        ISR_F_SETSTOP = false;
        twi_state[dev] = TWI_READY;
        ret = false;
        w_wait = pre_w_wait;
      }
    }
  } else if (twi_state[dev] == TWI_MT_RX) {
    if (ISR_F_SETSTOP) {
      if (i2cmaster_CheckStopBit(dev) == false) {
        if (twi_masterBufferIndexRX[dev] < twi_masterBufferLengthRX[dev])
          length = twi_masterBufferIndexRX[dev];
        else
          length = twi_masterBufferLengthRX[dev];
  
        // copy twi buffer to data
        if (_user_data != NULL) {
          for (i = 0; i < length; ++i)
            _user_data[i] = twi_masterBufferRX[dev][i];
        }
        
        ISR_F_SETSTOP = false;
        twi_state[dev] = TWI_READY;
        ret = false;
        r_wait = pre_r_wait;
      }
    }
  }
  
  return ret;
}

/* 
 * Function twi_transmit
 * Desc     fills slave tx buffer with data
 *          must be called in slave tx event callback
 * Input    data: pointer to byte array
 *          length: number of bytes in array
 * Output   1 length too long for buffer
 *          2 not slave transmitter
 *          0 ok
 */
uint8_t twi_transmit(uint8_t dev, const uint8_t* data, uint8_t length)
{
  uint8_t i;
  #if defined (DEBUG_MODE)
  printf("I2C transmit start\n");
  #endif

  if (init_OK[dev] == false) return 1;
  
  // ensure data will fit into buffer
  if (TWI_BUFFER_LENGTH < length){
    return 1;
  }
  
  // ensure we are currently a slave transmitter
  if (TWI_SL_TX != twi_state[dev]){
    return 2;
  }
  
  // set length and copy data into tx buffer
  twi_txBufferLength[dev] = length;
  for (i = 0; i < length; ++i){
    twi_txBuffer[dev][i] = data[i];
  }

  #if defined (DEBUG_MODE)
  printf("I2C transmit success\n");
  #endif  

  return 0;
}

/* 
 * Function twi_attachSlaveRxEvent
 * Desc     sets function called before a slave read operation
 * Input    function: callback function to use
 * Output   none
 */
void twi_attachSlaveRxEvent(uint8_t dev, void (*function)(volatile uint8_t*, int) )
{
  twi_onSlaveReceive[dev] = function;
}

/* 
 * Function twi_attachSlaveTxEvent
 * Desc     sets function called before a slave write operation
 * Input    function: callback function to use
 * Output   none
 */
void twi_attachSlaveTxEvent(uint8_t dev, void (*function)(void) )
{
  twi_onSlaveTransmit[dev] = function;
}

/* 
 * Function twi_stop (it only for i2c write operation, not read)
 * Desc     relinquishes bus master status
 * Input    none
 * Output   none
 */
void twi_stop(uint8_t dev)
{
  #if defined (DEBUG_MODE)
  printf("I2C send stop start\n");
  #endif

  if (init_OK[dev] == false) return;
  
  // send stop condition
  i2cmaster_SetStopBit(dev);

  const uint8_t us_per_loop = 8;
  uint32_t counter = (twi_timeout_us[dev] + us_per_loop - 1)/us_per_loop;
  while(i2cmaster_CheckStopBit(dev) == true){
    if(twi_timeout_us[dev] > 0L){
      if (counter > 0L){
        timer_DelayMicroseconds(us_per_loop);
        counter--;
      } else {
        twi_handleTimeout(dev, twi_do_reset_on_timeout[dev]);
        return;
      }
    }
  }

  // update twi state
  twi_state[dev] = TWI_READY;
  setHSM[dev] = false;

  #if defined (DEBUG_MODE)
  printf("I2C send stop sucess\n");
  #endif
}

/* 
 * Function twi_releaseBus
 * Desc     releases bus control
 * Input    none
 * Output   none
 */
void twi_releaseBus(uint8_t dev)
{
  if (init_OK[dev] == false) return;
  
  // release bus
  if (i2c_Reset(dev) == false)
  {
    #if defined (DEBUG_MODE)
    printf("i2c_Reset fail\n");
    #endif
    return;
  }
  #if defined (DEBUG_MODE)
  printf("i2c_Reset success\n");
  #endif
  // update twi state
  twi_state[dev] = TWI_READY;
}

int I2C_ISR(int irq, void* data)
{
  unsigned char status, clearBit = 0;
  int dev = *(int*)data;
  status = i2c_ReadStatREG(dev);
  if((status & 0xfc) == 0) return ISR_NONE;

  clearBit = i2c_ReadCtrlREG(dev) & 0xfc;
  clearBit &= (status & 0xfc);
  
  i2c_ClearSTAT(dev, clearBit);
  
  // I2C Master Handler
  if ((status & 0x01) != 0) {
    
    // I2CSTAT_ACKERR
    if ((clearBit & 0x10) != 0) {
      if (send_twi_addr == true) twi_error[dev] = MT_ADDR_NACK; else twi_error[dev] = MT_DATA_NACK;
      send_twi_addr = false;
	  i2cmaster_SetStopBit(dev);
      ISR_F_SETSTOP = true;
    }
    
    // I2CSTAT_ARLOSS
    if ((clearBit & 0x08) != 0) {
      twi_error[dev] = MT_ARB_LT;
      twi_releaseBus(dev);
    }
    
    if ((clearBit & (0x10 + 0x08)) != 0) return ISR_HANDLED;
    
    // I2C RXRDY
    if ((clearBit & 0x40) != 0) {
      
      // put byte into buffer
      twi_masterBufferRX[dev][twi_masterBufferIndexRX[dev]++] = i2c_ReadDataREG(dev);
        
      // Is the last data? if it is, set stopbit then do dummywrite
      if (twi_masterBufferIndexRX[dev] == (twi_masterBufferLengthRX[dev] - 1))
      {
        if (twi_sendStop[dev]) {
            i2cmaster_SetStopBit(dev);
            ISR_F_SETSTOP = true;
        }
        else
        {
          twi_inRepStart[dev] = true;	// we're gonna send the START
          // don't enable the interrupt. We'll generate the start, but we 
          // avoid handling the interrupt until we're in the next transaction,
          // at the point where we would normally issue the start.
          i2cmaster_WriteAddrREG(dev, twi_slarw[dev], I2C_WRITE); // WRITE??
          send_twi_addr = true;
          i2c_DisableINT(dev, I2CINT_TXDONE | I2CINT_ACKERR | I2CINT_ARLOSS);
        }
        
        i2c_WriteDataREG(dev, DUMMY_VAL);
        
      }
      else if (twi_masterBufferIndexRX[dev] < twi_masterBufferLengthRX[dev])
      {
        i2c_WriteDataREG(dev, DUMMY_VAL);
      }
      else // last round
      {
        if (ISR_F_SETSTOP == false) // restart only
          twi_state[dev] = TWI_READY;
      }
    }
    
    // I2C TXDONE
    if ((clearBit & 0x20) != 0) {
      
      // READ -> TXDONE (send address)
      if (twi_state[dev] == TWI_MT_RX)
	  {
        if (send_twi_addr == true) send_twi_addr = false;
        
	  	if (twi_masterBufferLengthRX[dev] > 1)
	  	  i2c_WriteDataREG(dev, DUMMY_VAL);
		else
		{
          if (twi_sendStop[dev]) {
            i2cmaster_SetStopBit(dev);
            ISR_F_SETSTOP = true;
          }
		  else
		  {
		    twi_inRepStart[dev] = true;	// we're gonna send the START
		    // don't enable the interrupt. We'll generate the start, but we 
		    // avoid handling the interrupt until we're in the next transaction,
		    // at the point where we would normally issue the start.
		    i2cmaster_WriteAddrREG(dev, twi_slarw[dev], I2C_WRITE); // WRITE??
		    send_twi_addr = true;
            i2c_DisableINT(dev, I2CINT_TXDONE | I2CINT_ACKERR | I2CINT_ARLOSS);
		  }

	      i2c_WriteDataREG(dev, DUMMY_VAL);

		}
	  }
      // WRITE -> TXDONE (send address/data)
      else if (twi_state[dev] == TWI_MT_TX)
      {
        if (twi_highSpeedMode[dev] == true && setHSM[dev] == false)
	    {
	      setHSM[dev] = true;
	    }
        else
        {
	      if (send_twi_addr == true) send_twi_addr = false;
	      
	      // if there is data to send, send it, otherwise stop 
          if (twi_masterBufferIndexTX[dev] < twi_masterBufferLengthTX[dev])
	      {
            // copy data to output register and ack
            i2c_WriteDataREG(dev, twi_masterBufferTX[dev][twi_masterBufferIndexTX[dev]++]);
          }
	      else
	      {
	        if (twi_sendStop[dev]) {
                i2cmaster_SetStopBit(dev);
                ISR_F_SETSTOP = true;
            }
	        else
	        {
	          twi_inRepStart[dev] = true;	// we're gonna send the START
	          // don't enable the interrupt. We'll generate the start, but we 
	          // avoid handling the interrupt until we're in the next transaction,
	          // at the point where we would normally issue the start.
	          i2cmaster_WriteAddrREG(dev, twi_slarw[dev], I2C_READ); // I2C_READ??
	          twi_state[dev] = TWI_READY; // Enter next "READ" opration as soon as possible
              send_twi_addr = true;
              i2c_DisableINT(dev, I2CINT_TXDONE | I2CINT_ACKERR | I2CINT_ARLOSS);
	        }
          }
        }
      }
    }
  } 
  else // I2C Slave Handler
  {
    if ((clearBit & 0x10) != 0) {
    }
    if ((clearBit & 0x08) != 0) {
    }
    
    if ((clearBit & (0x10 + 0x08)) != 0) return ISR_HANDLED;
    
    if ((clearBit & 0x80) != 0) {
      if (twi_state[dev] != TWI_SL_TX) {
        // enter slave transmitter mode
        twi_state[dev] = TWI_SL_TX;
        // ready the tx buffer index for iteration
        twi_txBufferIndex[dev] = 0;
        // set tx buffer length to be zero, to verify if user changes it
        twi_txBufferLength[dev] = 0;
        // request for txBuffer to be filled and length to be set
        // note: user must call twi_transmit(bytes, length) to do this
        twi_onSlaveTransmit[dev]();
        // if they didn't change buffer & length, initialize it
        if (0 == twi_txBufferLength[dev]) {
          twi_txBufferLength[dev] = 1;
          twi_txBuffer[dev][0] = 0x00;
        }
      }
      // transmit first byte from buffer, fall
      i2c_WriteDataREG(dev, twi_txBuffer[dev][twi_txBufferIndex[dev]++]);
    }
    
    // Slave Receiver
    if ((clearBit & 0x40) != 0) {
      if (twi_state[dev] != TWI_SL_RX) {
        // enter slave receiver mode
        twi_state[dev] = TWI_SL_RX;
        // indicate that rx buffer can be overwritten and ack
        twi_rxBufferIndex[dev] = 0;
      }
      // if there is still room in the rx buffer
      if (twi_rxBufferIndex[dev] < TWI_BUFFER_LENGTH) {
        // put byte in buffer and ack
        twi_rxBuffer[dev][twi_rxBufferIndex[dev]++] = i2c_ReadDataREG(dev);
      }
      //printf("SL_RXRDY ISR: interrupt times = %d status = 0x%02X\n", interrupt_times, temp_state);
    }
    
    if ((clearBit & 0x20) != 0) { // byte sent, ack returned
      // if there is more to send, ack, otherwise nack
      if (twi_txBufferIndex[dev] >= twi_txBufferLength[dev]) {
        twi_txBufferIndex[dev] = twi_txBufferLength[dev] - 1; // if too many SL_NEEDTOW command, send the last byte to master
      }
    }
    
    // Slave I2CSTAT_SLAVESTOPPED
    if ((clearBit & 0x04) != 0) {
      // put a null char after data if there's room
      if (twi_rxBufferIndex[dev] < TWI_BUFFER_LENGTH) {
        twi_rxBuffer[dev][twi_rxBufferIndex[dev]] = '\0';
      }
      // callback to user defined callback
      twi_onSlaveReceive[dev](twi_rxBuffer[dev], twi_rxBufferIndex[dev]);
      // since we submit rx buffer to "wire" library, we can reset it
      twi_rxBufferIndex[dev] = 0;
      twi_state[dev] = TWI_READY;
      //printf("SL_GET_STOP ISR: interrupt times = %d status = 0x%02X\n", interrupt_times, temp_state);
    }
  }

#if defined (DEBUG_MODE)
  interrupt_times++; temp_state = status;
#endif

  return ISR_HANDLED;
}
