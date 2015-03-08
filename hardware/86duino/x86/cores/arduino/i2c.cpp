/*
  i2c.cpp - DM&P Vortex86 I2C Bus library
  Copyright (c) 2009 AAA <aaa@dmp.com.tw>. All right reserved.

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

#define __I2C_LIB

#include "io.h"
#include "err.h"
#include "i2c.h"

#define I2C_TIMEOUT     (500L)  //500 ms

#define I2CACT_DISABLE          (0x00)
#define I2CACT_IDLE             (0x10)
#define I2CACT_MASTERWRITE      (0x20)
//---------------------------
#define I2CACT_MASTERREAD       (0x30)
#define I2CACT_MASTERHASREAD    (I2CACT_MASTERREAD + 1)
#define I2CACT_MASTERLASTREAD   (I2CACT_MASTERREAD + 2)
//---------------------------
#define I2CACT_SLAVE            (0x40)
#define I2CACT_SLAVEWRITEREQ    (I2CACT_SLAVE + 1)
#define I2CACT_SLAVEREADREQ     (I2CACT_SLAVE + 2)
static unsigned I2C_action[2] = {I2CACT_DISABLE, I2CACT_DISABLE};

#define I2CSW_DISABLE              (0x00)
static int I2C_swMode[2] = {I2CSW_DISABLE, I2CSW_DISABLE};

static int I2C_speedMode[2] = {I2CMODE_STANDARD, I2CMODE_STANDARD};


#define GPIO3_DATA   (0x7b)
#define GPIO3_DIR    (0x9b)

static unsigned long OLD_I2CGPIO3MASK[2] = {2L, 4L};
static unsigned long OLD_I2CGPIO3FLAG[2];
static unsigned char OLD_I2CGPIO3DIR[2];
static unsigned char OLD_I2CGPIO3VAL[2];


#define SB_IPFCTRL3_REG      (0xcc)
/*********  South Bridge Internal Peripheral Feature Control REG III: 0xCC  ******
 *    To use I2C0 interface, the user should set GS[1] (bit 1 of SB_IPFCTRL3_REG)
 *    to 1 (GS[1] = 1: I2C0; PINS0 = 0: GPIO3[5:4]). To use I2C1 interface, the
 *    should set GS[2] (bit 2 of SB_IPFCTRL3_REG) to 1 (GS[2] = 1: I2C1;
 *    PINS0 = 0: GPIO3[7:6])
 *********************************************************************************/

static int I2C_ioSection = -1;
DMPAPI(bool) i2c_InUse(void) {
    if (I2C_ioSection == -1) return false; else return true;
}


DMP_INLINE(void) set_pins(int dev, int SCL, int SDA) {
    unsigned char mask = ~(0x03 << (dev*2 + 4));
    unsigned char dir  = 0;
    unsigned char val  = (unsigned char)((SDA<<1) + SCL) << (dev*2 + 4);
    
    if (SCL == 0) dir = dir | (0x01 << (dev*2 + 4));  // set the SCL to output 0 
    if (SDA == 0) dir = dir | (0x02 << (dev*2 + 4));  // set the SDA to output 0
    
    io_outpb(GPIO3_DIR,  (io_inpb(GPIO3_DIR)  & mask) | dir); //set GPIO direction = out
    io_outpb(GPIO3_DATA, (io_inpb(GPIO3_DATA) & mask) | val);
}

DMPAPI(bool) i2c_Init2(unsigned baseaddr, unsigned devs, int i2c0irq, int i2c1irq) {
    int  i;
    
    if (I2C_ioSection != -1)
	{
        err_print("I2C lib was already opened");
		return false;
	}
	if ((I2C_ioSection = io_Init()) == -1) return false;

	//NOTE: base address should be selected carefully to avoid conflicts with other devices!
	if (baseaddr != 0xffff)
	    i2c_SetBaseAddress(baseaddr);
	else
	{
        if (i2c_SetDefaultBaseAddress() == 0x0000) i2c_SetBaseAddress(0xfb00);
    }

    #ifdef ROBOIO
        switch (roboio_GetRBVer())
        {
            case RB_100b1:
            case RB_100b2:
            case RB_100b3:
            case RB_100:
            case RB_100RD:
            case RB_110:
            case RB_050:
                devs = devs & I2C_USEMODULE0;
                i2c1irq = I2CIRQ_DISABLE;
                break;
            default:
                devs = 0;
                i2c0irq = i2c1irq = I2CIRQ_DISABLE;
                break;
        }
    #endif
    i2c_SetIRQ(i2c0irq, i2c1irq);

    I2C_swMode[0] = I2C_swMode[1] = I2CSW_DISABLE;
    I2C_action[0] = I2C_action[1] = I2CACT_DISABLE;
    for (i=0; i<2; i++)
    {
        if ((i == 0) && ((devs & I2C_USEMODULE0) == 0)) continue;
        if ((i == 1) && ((devs & I2C_USEMODULE1) == 0)) continue;
        I2C_action[i] = I2CACT_IDLE;

        //switch GPIO/I2C pins into GPIO pins
        OLD_I2CGPIO3FLAG[i] = sb_Read(SB_IPFCTRL3_REG) & OLD_I2CGPIO3MASK[i]; //backup GPIO/I2C switch flag
        sb_Write(SB_IPFCTRL3_REG, sb_Read(SB_IPFCTRL3_REG) & (~OLD_I2CGPIO3MASK[i]));

        //send START & STOP signal to reset I2C devices
        OLD_I2CGPIO3DIR[i] = io_inpb(GPIO3_DIR)  & (0x03 << (i*2+4)); //backup GPIO3 DIR
        OLD_I2CGPIO3VAL[i] = io_inpb(GPIO3_DATA) & (0x03 << (i*2+4)); //backup GPIO3 VAL

        //set_pins(i, 1, 1); delay_ms(1); //SCL = 1, SDA = 1; START
        //set_pins(i, 1, 0); delay_ms(1); //SCL = 1, SDA = 0
        //set_pins(i, 0, 0); delay_ms(1); //SCL = 0, SDA = 0
        //Note: if we send the above START, some I2C sensors, such as ADI ADXL345, may fail to respond

        //set_pins(i, 0, 0); delay_ms(1); //SCL = 0, SDA = 0; STOP
        //Note: if we perform the line above, some I2C sensors, such as MEMSIC MXC6202, may fail to respond
        
        //set_pins(i, 1, 0); delay_ms(1); //SCL = 1, SDA = 0
        //set_pins(i, 1, 1); delay_ms(1); //SCL = 1, SDA = 1
        //Note: the above lines work for all sensors we tested, but we disable them to avoid unexpected sensor behaviors

        if (i2c_Reset(i) == false)  // assume the status of GPIO/I2C pins are GPIO "IN" or "OUT 1"
        {
            i2c_Close();
            err_print("can't reset the I2C modules");
		    return false;
        }

        i2c_DisableINT(i, I2CINT_ALL);
        i2c_ClearSTAT(i, I2CSTAT_ALL);

        //Remarks: for DX ver.2, we must disable the noise filter to ensure that 3.3Mbps works in high-speed mode
        i2c_DisableNoiseFilter(i);
        //i2c_EnableNoiseFilter(i);
        i2c_DisableStandardHSM(i); //SCL open-drain in high-speed mode
        //i2c_EnableStandardHSM(i);
    
        i2c_SetSpeed(i, I2CMODE_AUTO, 100000L); //default 100Kbps

        i2cslave_SetAddr(i, 0x7f); //set slave address 0x7F by default (change this if it collide with external I2C devices)
        i2cslave_EnableACK(i);

        //switch GPIO pins into I2C SCL,SDA pins
        //Remarks: Vortex86DX's H/W I2C has an issue here; if you call i2c_Reset() in case GPIO/SCL pin = GPIO out 0,
        //         then, whenever you switch GPIO/SCL pin to SCL pin, the SCL pin always first send the 10 reset dummy clocks
        sb_Write(SB_IPFCTRL3_REG, sb_Read(SB_IPFCTRL3_REG) | OLD_I2CGPIO3MASK[i]);
    }

    return true;
}

DMPAPI(void) i2c_Close(void) {
    int i;
    
	if (I2C_ioSection == -1) return;

    for (i=0; i<2; i++) //restore GPIO3/I2C switch setting
    {
        if (I2C_action[i] == I2CACT_DISABLE) continue;

        io_outpb(GPIO3_DIR,  (io_inpb(GPIO3_DIR)  & (~(0x03 << (i*2+4)))) | OLD_I2CGPIO3DIR[i]); //restore GPIO3 DIR
        io_outpb(GPIO3_DATA, (io_inpb(GPIO3_DATA) & (~(0x03 << (i*2+4)))) | OLD_I2CGPIO3VAL[i]); //restore GPIO3 VAL
        sb_Write(SB_IPFCTRL3_REG, (sb_Read(SB_IPFCTRL3_REG) & (~OLD_I2CGPIO3MASK[i])) | OLD_I2CGPIO3FLAG[i]); //restore GPIO/I2C switch flag
        I2C_action[i] = I2CACT_DISABLE;
	}

	//io_Close(I2C_ioSection);
  io_Close();
	I2C_ioSection = -1;
}


DMP_INLINE(unsigned long) clkerr(unsigned long bps, unsigned long div) {
    unsigned long clk;

    if (div == 0L) return 0xffffffffL;
    clk = 33000000L / div;
    if (bps > clk) return bps - clk; else return clk - bps;
}

DMPAPI(unsigned long) i2c_SetSpeed(int dev, int mode, unsigned long bps) {
    unsigned long div1, div2, i, x;
	/*
    if (I2C_action[dev] != I2CACT_IDLE)
	{
        err_print(ERROR_I2CWRONGUSAGE, "can only set I2C's speed when it is idle");
		return 0xffffffffL;
	}
	*/
    if (mode == I2CMODE_AUTO) {
        if (bps <= 100000L)
            mode = I2CMODE_STANDARD;
        else
        if (bps <= 400000L)
            mode = I2CMODE_FAST;
        else
            mode = I2CMODE_HIGHSPEED;
    }
    
	if (bps != 0L)
	{   //find optimal divisors
        div1 = 10L; div2 = 1L;
        for (i=1L; i<=128L; i++)
        {
            x = 33000000L/(bps*i);
            if (x < 10L)  x = 10L;
            if (x > 255L) x = 255L;

            if ((x < 255L) && (clkerr(bps, x*i) > clkerr(bps, (x+1L)*i))) x = x + 1L;
            if (clkerr(bps, div1*div2) > clkerr(bps, x*i))
            {
                div1 = x;
                div2 = i;
            }
        } //end for (i=...
    }
    else
    {
	   div1 = 255L;
       div2 = 128L;
	} //end if (bps...

    if (mode == I2CMODE_STANDARD)
        i2c_SetCLKREG(dev, I2CMODE_STANDARD, (unsigned char)div1, (unsigned char)div2-1);
    else
        i2c_SetCLKREG(dev, I2CMODE_FAST, (unsigned char)div1, (unsigned char)div2-1);

    I2C_speedMode[dev] = mode;
    return 33000000L/(div1*div2);
}



/*********************  I2C Software-Simulated Functions  *********************/

// GPIO3[4:5] will be used to simulate I2C port 0 for I2C S/W mode;
// GPIO3[6:7] will be used to simulate I2C port 1 for I2C S/W mode;
// (GPIO34 = I2C0_SCL, GPIO35 = I2C0_SDA, GPIO36 = I2C1_SCL, GPIO37 = I2C1_SDA)

static unsigned long I2CSW_delay[2] = {0L, 0L};

DMP_INLINE(int) read_sda(int dev) {
    unsigned char val = io_inpb(GPIO3_DATA) >> (dev*2 + 4);
    
    if ((val & 0x02) == 0) return 0; else return 1;
}



DMPAPI(bool) i2c_InitSW2(unsigned devs, int i2c0mode, unsigned long i2c0clkdelay, int i2c1mode, unsigned long i2c1clkdelay) {
    int  i;
    
    if (I2C_ioSection != -1)
	{
        err_print("I2C lib was already opened");
		return false;
	}
	if ((I2C_ioSection = io_Init()) == -1) return false;

    #ifdef ROBOIO
        switch (roboio_GetRBVer())
        {
            case RB_100b1:
            case RB_100b2:
            case RB_100b3:
            case RB_100:
            case RB_100RD:
            case RB_110:
            case RB_050:
                devs = devs & I2C_USEMODULE0;
                break;
            default:
                devs = 0;
                break;
        }
    #endif

    I2C_swMode[0] = i2c0mode; I2CSW_delay[0] = i2c0clkdelay;
    I2C_swMode[1] = i2c1mode; I2CSW_delay[1] = i2c1clkdelay;

    I2C_action[0] = I2C_action[1] = I2CACT_DISABLE;
    for (i=0; i<2; i++)
    {
        if ((i == 0) && ((devs & I2C_USEMODULE0) == 0)) continue;
        if ((i == 1) && ((devs & I2C_USEMODULE1) == 0)) continue;
        I2C_action[i] = I2CACT_IDLE;

        // switch GPIO/I2C pins into GPIO pins
        OLD_I2CGPIO3FLAG[i] = sb_Read(SB_IPFCTRL3_REG) & OLD_I2CGPIO3MASK[i];  // backup GPIO/I2C switch flag
        sb_Write(SB_IPFCTRL3_REG, sb_Read(SB_IPFCTRL3_REG) & (~OLD_I2CGPIO3MASK[i]));

        OLD_I2CGPIO3DIR[i] = io_inpb(GPIO3_DIR)  & (0x03 << (i*2+4));  // backup GPIO3 DIR
        OLD_I2CGPIO3VAL[i] = io_inpb(GPIO3_DATA) & (0x03 << (i*2+4));  // backup GPIO3 VAL
        
        // set GPIO pins as INPUT state (equiv. to OUT 1 of I2C lines)
        io_outpb(GPIO3_DIR, io_inpb(GPIO3_DIR) & ~(0x03 << (i*2+4)));
    }

    return true;
}

DMPAPI(void) i2c_CloseSW(void) {
    i2c_Close();
}



static bool i2csw_Start(int dev, unsigned char addr, unsigned char rwbit, bool restart) {
    int i, databit;
    
    if (restart == false)
    {   // send START signal
        set_pins(dev, 1, 1); timer_DelayMicroseconds(I2CSW_delay[dev]);  // SCL = 1, SDA = 1
        set_pins(dev, 1, 0); timer_DelayMicroseconds(I2CSW_delay[dev]);  // SCL = 1, SDA = 0
        set_pins(dev, 0, 0);                              // SCL = 0, SDA = 0
    }
    else
    if (I2C_swMode[dev] == I2CSW_LEGO)
    {   // send LEGO NXT's I2C RESTART signal
        set_pins(dev, 0, 0); timer_DelayMicroseconds(I2CSW_delay[dev]);  // SCL = 0, SDA = 0
        set_pins(dev, 1, 0); timer_DelayMicroseconds(I2CSW_delay[dev]);  // SCL = 1, SDA = 0
        set_pins(dev, 1, 1); timer_DelayMicroseconds(I2CSW_delay[dev]);  // SCL = 1, SDA = 1

        set_pins(dev, 0, 1); timer_DelayMicroseconds(I2CSW_delay[dev]);  // LEGO NXT special: SCL = 0, SDA = 1
        
        set_pins(dev, 1, 1); timer_DelayMicroseconds(I2CSW_delay[dev]);  // SCL = 1, SDA = 1
        set_pins(dev, 1, 0); timer_DelayMicroseconds(I2CSW_delay[dev]);  // SCL = 1, SDA = 0
        set_pins(dev, 0, 0);                              // SCL = 0, SDA = 0
    }
    else
    {   // send normal I2C RESTART signal
        set_pins(dev, 0, 0); timer_DelayMicroseconds(I2CSW_delay[dev]);  // SCL = 0, SDA = 0
        set_pins(dev, 0, 1); timer_DelayMicroseconds(I2CSW_delay[dev]);  // SCL = 0, SDA = 1
        set_pins(dev, 1, 1); timer_DelayMicroseconds(I2CSW_delay[dev]);  // SCL = 1, SDA = 1
        set_pins(dev, 1, 0); timer_DelayMicroseconds(I2CSW_delay[dev]);  // SCL = 1, SDA = 0
        set_pins(dev, 0, 0);                              // SCL = 0, SDA = 0
    }

    // send data byte
    addr = (addr << 1) | rwbit;
    for(i=0; i<8; i++)
    {
        databit = ((addr & 0x80) == 0)? 0 : 1;
        set_pins(dev, 0, databit); timer_DelayMicroseconds(I2CSW_delay[dev]);  // set data bit
        set_pins(dev, 1, databit); timer_DelayMicroseconds(I2CSW_delay[dev]);  // send a clock
        set_pins(dev, 0, databit);
        addr = addr << 1;
    }
    
    // read ACK
    set_pins(dev, 0, 1); timer_DelayMicroseconds(I2CSW_delay[dev]);  // set SDA as input
    set_pins(dev, 1, 1); timer_DelayMicroseconds(I2CSW_delay[dev]);  // send a clock
    if (read_sda(dev) == 1)
    {
		err_print("receive no ACK after transmitting");
        return false;
    }
    set_pins(dev, 0, 1);
    
    return true;
}

static void i2csw_Stop(int dev) {
    set_pins(dev, 0, 0); timer_DelayMicroseconds(I2CSW_delay[dev]);  // SCL = 0, SDA = 0
    set_pins(dev, 1, 0); timer_DelayMicroseconds(I2CSW_delay[dev]);  // SCL = 1, SDA = 0
    set_pins(dev, 1, 1);                              // SCL = 1, SDA = 1
}



static bool i2csw_Write(int dev, unsigned char val) {
    int i, databit;

    // send data byte
    for(i=0; i<8; i++)
    {
        databit = ((val & 0x80) == 0)? 0 : 1;
        set_pins(dev, 0, databit); timer_DelayMicroseconds(I2CSW_delay[dev]);  // set data bit
        set_pins(dev, 1, databit); timer_DelayMicroseconds(I2CSW_delay[dev]);  // send a clock
        set_pins(dev, 0, databit);
        val = val << 1;
    }
    
    // read ACK
    set_pins(dev, 0, 1); timer_DelayMicroseconds(I2CSW_delay[dev]);  // set SDA as input
    set_pins(dev, 1, 1); timer_DelayMicroseconds(I2CSW_delay[dev]);  // send a clock
    if (read_sda(dev) == 1)
    {
		err_print("receive no ACK after transmitting");
        return false;
    }
    set_pins(dev, 0, 1);
    
    return true;
}



static unsigned i2csw_Read(int dev, bool last) {
    unsigned char val = 0;
    int i, databit;

    // read data byte
    for(i=0; i<8; i++)
    {
        set_pins(dev, 0, 1); timer_DelayMicroseconds(I2CSW_delay[dev]);  // set SDA as input
        set_pins(dev, 1, 1); timer_DelayMicroseconds(I2CSW_delay[dev]);  // send a clock
        databit = read_sda(dev);
        val = (val << 1) | (unsigned char)databit;
    }
    set_pins(dev, 0, 1);

    if (last == false)
    {  // send ACK
        set_pins(dev, 0, 0); timer_DelayMicroseconds(I2CSW_delay[dev]);  // set SDA = 0
        set_pins(dev, 1, 0); timer_DelayMicroseconds(I2CSW_delay[dev]);  // send a clock
        set_pins(dev, 0, 0);
    }
    else
    {  // send NACK
        set_pins(dev, 0, 1); timer_DelayMicroseconds(I2CSW_delay[dev]);  // set SDA = 1
        set_pins(dev, 1, 1); timer_DelayMicroseconds(I2CSW_delay[dev]);  // send a clock
        set_pins(dev, 0, 1);
    }
    
    return (unsigned)val;
}
/*----------------  end of I2C Software-Simulated Functions  -----------------*/



/*******************  Master Functions for Individual Module *****************/
static struct
{
    bool restart;
    unsigned char addr;
    unsigned char rwbit;
    int count;

} I2C_rsInfo[2] = {{false, 0x00, I2C_READ, 0}, {false, 0x00, I2C_READ, 0}};

static int I2C_count[2] = {0, 0};  // only used by i2cmaster_xxxxN() functions
static unsigned char I2C_curAddr[2] = {0, 0};  // only used by i2cmaster_xxxxN() functions

DMP_INLINE(bool) check_TX_done(int dev) {
    unsigned long nowtime;
    unsigned char statreg;

    for (nowtime=0; nowtime<100000L; nowtime++) //trick for speed; timer_NowTime() is time-consuming in some OS
        if (i2c_CheckTXDone(dev) == true) goto TX_SUCCESS;

    nowtime = timer_NowTime();
    while ((i2c_CheckTXDone(dev) == false) && ((timer_NowTime() - nowtime) < I2C_TIMEOUT));

TX_SUCCESS:
    statreg = i2c_ReadStatREG(dev); //ugly code for speed:p
    i2c_ClearTXDone(dev);

    if ((statreg & 0x08) != 0) //if (i2cmaster_CheckARLoss(dev) == true)
    {
        err_print("arbitration loss for I2C bus");
        i2cmaster_ClearARLoss(dev);
    }
    if ((statreg & 0x10) != 0) //if (i2cmaster_CheckAckErr(dev) == true)
    {
        err_print("receive no ACK after transmitting");
        i2cmaster_ClearAckErr(dev);
    }
    if ((statreg & (0x10 + 0x08)) != 0) return false;

    if ((statreg & 0x01) == 0) //if (i2c_IsMaster(dev) == false)
    {
        err_print("I2C%d module isn't in Master Mode", dev);
        return false;
    }
    if ((statreg & 0x20) == 0) //if (i2c_CheckTXDone(dev) == false)
    {
        err_print("I2C%d module doesn't respond", dev);
        return false;
    }
    
    return true;
}

DMP_INLINE(bool) check_RX_done(int dev) {
    unsigned long nowtime;
    unsigned char statreg;

    for (nowtime=0; nowtime<100000L; nowtime++) //trick for speed; timer_NowTime() is time-consuming in some OS
        if (i2c_CheckRXRdy(dev) == true) goto RX_SUCCESS;

    nowtime = timer_NowTime();
    while ((i2c_CheckRXRdy(dev) == false) && ((timer_NowTime() - nowtime) < I2C_TIMEOUT));

RX_SUCCESS:
    statreg = i2c_ReadStatREG(dev); //ugly code for speed:p
    i2c_ClearRXRdy(dev);

    if ((statreg & 0x08) != 0) //if (i2cmaster_CheckARLoss(dev) == true)
    {
        err_print("arbitration loss for I2C bus");
        i2cmaster_ClearARLoss(dev);
        return false;
    }
    if ((statreg & 0x01) == 0) //if (i2c_IsMaster(dev) == false)
    {
        err_print("I2C%d module isn't in Master Mode", dev);
        return false;
    }
    if ((statreg & 0x40) == 0) //if (i2c_CheckRXRdy(dev) == false)
    {
        err_print("I2C%d module doesn't respond", dev);
        return false;
    }

    return true;
}

DMP_INLINE(bool) check_STOP_done(int dev) {
    unsigned long nowtime;

    for (nowtime=0; nowtime<100000L; nowtime++) //trick for speed; timer_NowTime() is time-consuming in some OS
        if (i2cmaster_CheckStopBit(dev) == false) goto STOP_SUCCESS;

    nowtime = timer_NowTime();
    while ((i2cmaster_CheckStopBit(dev) == true) && ((timer_NowTime() - nowtime) < I2C_TIMEOUT));

    if (i2cmaster_CheckStopBit(dev) == true)
    {
        err_print("fail to stop the transaction");
        return false;
    }

STOP_SUCCESS:
    return true;
}



DMP_INLINE(bool) i2cmaster_Start1(int dev, unsigned char addr, unsigned char rwbit) {
    if (i2c_IsMaster(dev) == false)
    {
        err_print("can't start a transaction in Slave state");
        return false;
    }

    i2cmaster_WriteAddrREG(dev, addr, rwbit);

    //Remarks: if I2C master sends "Start" twice sequently, ARLoss will occur according to H/W design
    return check_TX_done(dev);
}

DMPAPI(bool) i2cmaster_Start(int dev, unsigned char addr, unsigned char rwbit) {
    bool result = true;
    
    if (I2C_action[dev] != I2CACT_IDLE)
    {
        err_print("can't start a transaction because I2C%d isn't idle", dev);
        return false;
    }

    I2C_rsInfo[dev].restart = false;

    if (I2C_swMode[dev] != I2CSW_DISABLE)
        result = i2csw_Start(dev, addr, rwbit, false);
    else
    {

        if (I2C_speedMode[dev] == I2CMODE_HIGHSPEED) 
            result = i2cmaster_Start1(dev, 0x07, I2C_WRITE); //send high-speed master code

        if (result == true) result = i2cmaster_Start1(dev, addr, rwbit);
    }
    
    if (result == true)
    {
        I2C_action[dev]  = (rwbit == I2C_WRITE)? I2CACT_MASTERWRITE : I2CACT_MASTERREAD;
        I2C_curAddr[dev] = addr;
    }
    return result;
}



DMPAPI(bool) i2cmaster_Write(int dev, unsigned char val) {
    bool result;

    if (I2C_action[dev] != I2CACT_MASTERWRITE)
    {
        err_print("must start a correct transaction before writing data");
        return false;
    }

    if (I2C_swMode[dev] != I2CSW_DISABLE)
        result = i2csw_Write(dev, val);
    else
    {
        i2c_WriteDataREG(dev, val);
        result = check_TX_done(dev);
    }

    if (result == false) I2C_action[dev] = I2CACT_IDLE;
    return result;
}

DMPAPI(bool) i2cmaster_WriteLast(int dev, unsigned char val) {
    bool result = true;

    if (i2cmaster_Write(dev, val) == false) return false; else I2C_action[dev] = I2CACT_IDLE;

    // case of I2C RESTART after write
    if (I2C_rsInfo[dev].restart == true)
    {
        I2C_rsInfo[dev].restart = false;

        if (I2C_swMode[dev] != I2CSW_DISABLE)
            result = i2csw_Start(dev, I2C_rsInfo[dev].addr, I2C_rsInfo[dev].rwbit, true);
        else
            result = i2cmaster_Start1(dev, I2C_rsInfo[dev].addr, I2C_rsInfo[dev].rwbit);

        if (result == true)
        {
            I2C_action[dev] = (I2C_rsInfo[dev].rwbit == I2C_WRITE)? I2CACT_MASTERWRITE : I2CACT_MASTERREAD;
            I2C_count[dev]  = I2C_rsInfo[dev].count;
        }
        return result;
    }

    // case of I2C STOP after write
    if (I2C_swMode[dev] != I2CSW_DISABLE)
        i2csw_Stop(dev);
    else
    {
        i2cmaster_SetStopBit(dev);
        result = check_STOP_done(dev);
    }
    return result;
}



//Remarks: the following "Read" functions are odd due to Vortex86DX's poor I2C design
DMPAPI(unsigned) i2cmaster_Read1(int dev, bool secondlast) {
    switch (I2C_action[dev])
    {
        case I2CACT_MASTERREAD:
            if (I2C_swMode[dev] == I2CSW_DISABLE) i2c_ReadDataREG(dev);  // dummy read to trigger the I2C module for reading a byte
            I2C_action[dev] = I2CACT_MASTERHASREAD;
            break;
        case I2CACT_MASTERHASREAD:
            break;
        case I2CACT_MASTERLASTREAD:
            err_print("call i2cmaster_ReadLast() to read the last byte");
            return 0xffff;
        default:
            err_print("must start a correct transaction before reading data");
            return 0xffff;
    }
    
    if (I2C_swMode[dev] == I2CSW_DISABLE)
    {
        if (check_RX_done(dev) == false)
        {
            I2C_action[dev] = I2CACT_IDLE;
            return 0xffff;
        }

        if (secondlast == true)
        {
            if (I2C_rsInfo[dev].restart == true)
            {
                i2cmaster_WriteAddrREG(dev, I2C_rsInfo[dev].addr, I2C_rsInfo[dev].rwbit);
                timer_DelayMicroseconds(150L); //Remarks: must delay here due to DX's I2C H/W bug
            }
            else
                i2cmaster_SetStopBit(dev);
        }
    } //end if (I2C_swMode[dev] ...

    if (secondlast == true) I2C_action[dev] = I2CACT_MASTERLASTREAD;

    if (I2C_swMode[dev] != I2CSW_DISABLE)
        return i2csw_Read(dev, false);
    else
        return (unsigned)i2c_ReadDataREG(dev); //Remarks: this read will trigger I2C module to read the next byte
}

DMPAPI(unsigned) i2cmaster_ReadLast(int dev) {
    unsigned val;
    
    switch (I2C_action[dev])
    {
        case I2CACT_MASTERREAD:
            if (I2C_swMode[dev] == I2CSW_DISABLE)
            {
                if (I2C_rsInfo[dev].restart == true)
                {
                    i2cmaster_WriteAddrREG(dev, I2C_rsInfo[dev].addr, I2C_rsInfo[dev].rwbit);
                    timer_DelayMicroseconds(150L); //Remarks: must delay here due to DX's I2C H/W bug
                }
                else
                    i2cmaster_SetStopBit(dev);
            }
            break;
        case I2CACT_MASTERHASREAD:
            err_print("set Second-Last-Read before calling %s()", __FUNCTION__);
            return 0xffff;
        case I2CACT_MASTERLASTREAD:
            I2C_action[dev] = I2CACT_MASTERHASREAD;  // for calling i2cmaster_Read1() again to get the last byte
            break;
    }

    val = (I2C_swMode[dev] != I2CSW_DISABLE)? i2csw_Read(dev, true) : i2cmaster_Read1(dev, false);
    I2C_action[dev] = I2CACT_IDLE;
    if (val == 0xffff) return 0xffff;

    if (I2C_rsInfo[dev].restart == true)
    {
        I2C_rsInfo[dev].restart = false;
        
        if (I2C_swMode[dev] != I2CSW_DISABLE)
        {
            if (i2csw_Start(dev, I2C_rsInfo[dev].addr, I2C_rsInfo[dev].rwbit, true) == false) return 0xffff;
        }
        else
        {
            if (check_TX_done(dev) == false) return 0xffff;
        }

        I2C_action[dev] = (I2C_rsInfo[dev].rwbit == I2C_WRITE)? I2CACT_MASTERWRITE : I2CACT_MASTERREAD;
        I2C_count[dev]  = I2C_rsInfo[dev].count;
    }
    else
    {
        if (I2C_swMode[dev] != I2CSW_DISABLE)
            i2csw_Stop(dev);
        else
        {
            if (check_STOP_done(dev) == false) return 0xffff;
        }
    }//end if I2C_restart...

    return val;
}



DMPAPI(bool) i2cmaster_SetRestart(int dev, unsigned char addr, unsigned char rwbit) {
    if ((I2C_action[dev] != I2CACT_MASTERWRITE) && (I2C_action[dev] != I2CACT_MASTERREAD))
    {
        err_print("wrong time to call %s()", __FUNCTION__);
        return false;
    }

    I2C_rsInfo[dev].restart = true;
    I2C_rsInfo[dev].addr    = addr;
    I2C_rsInfo[dev].rwbit   = rwbit;
    I2C_rsInfo[dev].count   = 0;
    return true;
}



// ^^^^^^^^^^^^^^^^  Alternative I2C API for More Simple Usages ^^^^^^^^^^^^^^^^
DMPAPI(bool) i2cmaster_StartN(int dev, unsigned char addr, unsigned char rwbit, int count) {
    if (count <= 0)
    {
        err_print("the number of bytes to read/write must > 0");
        return false;
    }

    if (i2cmaster_Start(dev, addr, rwbit) == false) return false;

    I2C_count[dev] = count;
    return true;
}

DMPAPI(bool) i2cmaster_WriteN(int dev, unsigned char val) {
    if (I2C_count[dev] == 0) return true; else I2C_count[dev]--; //lazy code:p (should return false with Err_Msg)

    if (I2C_count[dev] == 0)
        return i2cmaster_WriteLast(dev, val);
    else
    {
        if (i2cmaster_Write(dev, val) == true) return true;
    }

    I2C_count[dev] = 0;
    return false;
}

DMPAPI(unsigned) i2cmaster_ReadN(int dev) {
    unsigned val;

    if (I2C_count[dev] == 0) return 0x0000; else I2C_count[dev]--; //lazy code:p (should return 0xffff with Err_Msg)

    if (I2C_count[dev] == 0)
        return i2cmaster_ReadLast(dev);
    else
    if (I2C_count[dev] == 1)
        val = i2cmaster_Read1(dev, true);
    else
        val = i2cmaster_Read1(dev, false);

    if (val == 0xffff) I2C_count[dev] = 0;
    return val;
}

DMPAPI(bool) i2cmaster_SetRestartN(int dev, unsigned char rwbit, int count) {
    if (i2cmaster_SetRestart(dev, I2C_curAddr[dev], rwbit) == false) return false;

    I2C_rsInfo[dev].count = count;
    return true;
}
/*----------------------  end of Master Functions  --------------------------*/



/********************  Slave Functions for Individual Module *****************/
DMPAPI(unsigned) i2cslave_Listen(int dev) {
    unsigned char i2cstat;

    switch (I2C_action[dev])
    {
        case I2CACT_IDLE:
            if (i2c_IsMaster(dev) == true) return I2CSLAVE_NOTHING;
            I2C_action[dev] = I2CACT_SLAVE;
            return I2CSLAVE_START;
        case I2CACT_SLAVE:
            i2cstat = i2c_ReadStatREG(dev);

            if ((i2cstat & I2CSTAT_RXRDY) != 0)
            {
                I2C_action[dev] = I2CACT_SLAVEREADREQ;
                return I2CSLAVE_READREQUEST;
            }
            if ((i2cstat & I2CSTAT_SLAVESTOPPED) != 0)
            {
                i2c_ClearSTAT(dev, I2CSTAT_ALL);
                I2C_action[dev] = I2CACT_IDLE;
                return I2CSLAVE_END;
            }
            if ((i2cstat & I2CSTAT_SLAVEWREQ) != 0)
            {
                I2C_action[dev] = I2CACT_SLAVEWRITEREQ;
                return I2CSLAVE_WRITEREQUEST;
            }
            
            return I2CSLAVE_WAITING;
        case I2CACT_SLAVEWRITEREQ:
        case I2CACT_SLAVEREADREQ:
            err_print("must deal with the salve read/write request");
            break;
        case I2CACT_DISABLE:
            err_print("must enable the I2C module first");
            break;
        default:
            err_print("can't use %s() when I2C master is working", __FUNCTION__);
            break;
    }

    return 0xffff;
}


DMPAPI(bool) i2cslave_Write(int dev, unsigned char val) {
    unsigned long nowtime;

    if (I2C_action[dev] != I2CACT_SLAVEWRITEREQ)
    {
        err_print("slave must write data only when being requested");
        return false;
    }
    
    i2cslave_ClearSlaveWREQ(dev);
    i2c_WriteDataREG(dev, val);

    for (nowtime=0; nowtime<100000L; nowtime++) //trick for performance; timer_NowTime() is time-consuming in some OS
        if (i2c_CheckTXDone(dev) == true) goto TX_SUCCESS;

    nowtime = timer_NowTime();
    while ((i2c_CheckTXDone(dev) == false) && ((timer_NowTime() - nowtime) < I2C_TIMEOUT));

    if (i2c_CheckTXDone(dev) == false)
    {
        err_print("the I2C module doesn't respond");
        I2C_action[dev] = I2CACT_IDLE;
        return false;
    }

TX_SUCCESS:
    i2c_ClearTXDone(dev);
    I2C_action[dev] = I2CACT_SLAVE;
    return true;
}


DMPAPI(unsigned) i2cslave_Read(int dev) {
    if (I2C_action[dev] != I2CACT_SLAVEREADREQ)
    {
        err_print("slave must read data only when data is ready");
        return 0xffff;
    }

    i2c_ClearRXRdy(dev);
    I2C_action[dev] = I2CACT_SLAVE;
    return i2c_ReadDataREG(dev); //Remarks: slave will hold clock (to avoid slave's RX buffer ) until it read the data
}
/*-----------------------  end of Slave Functions  --------------------------*/



#define GPIO2_DATA   (0x7a)
#define GPIO2_DIR    (0x9a)

DMPAPI(void) i2c_SetResetPin(void) {  // set the Reset pin on I2C connector of RB-110 & RB-050 to 1
	io_outpb(GPIO2_DIR, io_inpb(GPIO2_DIR) | 0x80);
    io_outpb(GPIO2_DATA, io_inpb(GPIO2_DATA) | 0x80);
}

DMPAPI(void) i2c_ClearResetPin(void) {  // clear the Reset pin on I2C connector of RB-110 & RB-050 to 0
	io_outpb(GPIO2_DIR, io_inpb(GPIO2_DIR) | 0x80);
    io_outpb(GPIO2_DATA, io_inpb(GPIO2_DATA) & 0x7f);
}

