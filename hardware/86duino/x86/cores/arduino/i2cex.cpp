/*
  i2cex.cpp - Part of DM&P Vortex86 I2C Bus library
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

#define __I2CDX_LIB

#include "io.h"
#include "err.h"
#include "i2cex.h"

#define I2C_TIMEOUT     (500L)  //500 ms

/*
#define SB_IPFCTRL3_REG      (0xcc)
/*********  South Bridge Internal Peripheral Feature Control REG III: 0xCC  ******
 *    To use I2C0 interface, the user should set GS[1] (bit 1 of SB_IPFCTRL3_REG)
 *    to 1 (GS[1] = 1: I2C0; PINS0 = 0: GPIO3[5:4]). To use I2C1 interface, the
 *    should set GS[2] (bit 2 of SB_IPFCTRL3_REG) to 1 (GS[2] = 1: I2C1;
 *    PINS0 = 0: GPIO3[7:6])
 *********************************************************************************/

#define SB_I2CCTRL_REG   (0xd4)
/***************  South Bridge Internal I2C Control REG: 0xd4~0xd7  **************
 *    bit 31:    =1 enable I/O address decoding
 *    bit 30-24: =0
 *    bit 23-20: I2C1 IRQ routing
 *               B23 B22 B21 B20   Routing Table
 *                 0   0   0   0   Disable.
 *                 0   0   0   1   IRQ[9]
 *                 0   0   1   0   IRQ[3] 
 *                 0   0   1   1   IRQ[10]
 *                 0   1   0   0   IRQ[4]
 *                 0   1   0   1   IRQ[5]
 *                 0   1   1   0   IRQ[7]
 *                 0   1   1   1   IRQ[6]
 *                 1   0   0   0   IRQ[1]
 *                 1   0   0   1   IRQ[11]
 *                 1   0   1   0   Reserved
 *                 1   0   1   1   IRQ[12]
 *                 1   1   0   0   Reserved
 *                 1   1   0   1   IRQ[14]
 *                 1   1   1   0   Reserved
 *                 1   1   1   1   IRQ[15]
 *    bit 19-16: I2C0 IRQ routing 
 *               B19 B18 B17 B16   Routing Table
 *                 0   0   0   0   Disable.
 *                 0   0   0   1   IRQ[9]
 *                 0   0   1   0   IRQ[3] 
 *                 0   0   1   1   IRQ[10]
 *                 0   1   0   0   IRQ[4]
 *                 0   1   0   1   IRQ[5]
 *                 0   1   1   0   IRQ[7]
 *                 0   1   1   1   IRQ[6]
 *                 1   0   0   0   IRQ[1]
 *                 1   0   0   1   IRQ[11]
 *                 1   0   1   0   Reserved
 *                 1   0   1   1   IRQ[12]
 *                 1   1   0   0   Reserved
 *                 1   1   0   1   IRQ[14]
 *                 1   1   1   0   Reserved
 *                 1   1   1   1   IRQ[15]
 *    bit 15-4:  base address bits 15~4
 *    bit 3-0:   =0
 *********************************************************************************/

static unsigned BaseAddress[2] = {0xfb00, 0xfb00+8};
DMPAPI(void) i2c_SetBaseAddress(unsigned baseaddr) {
    sb_Write(SB_I2CCTRL_REG,  //set base address with enabling I/O address decoding
                 ((sb_Read(SB_I2CCTRL_REG) & 0xffff0000L) | 0x80000000L) + (unsigned long)(baseaddr & 0xfff0));
    BaseAddress[0] = baseaddr;
    BaseAddress[1] = baseaddr+8;
}

DMPAPI(unsigned) i2c_SetDefaultBaseAddress(void) {
    BaseAddress[0] = (unsigned)(sb_Read(SB_I2CCTRL_REG) & 0xfff0L);
    BaseAddress[1] = BaseAddress[0]+8;
    return BaseAddress[0];
}


DMPAPI(void) i2c_SetIRQ(int i2c0irq, int i2c1irq) {
    sb_Write(SB_I2CCTRL_REG, (sb_Read(SB_I2CCTRL_REG) & 0xff00ffffL)
                                 + ((unsigned long)(i2c0irq & 0x000f) << 16)
                                 + ((unsigned long)(i2c1irq & 0x000f) << 20));
}



/******************  General Functions for Individual Module *****************/
#define I2C_CTRL_REG(dev)       (BaseAddress[dev] + 0)
#define I2C_STAT_REG(dev)       (BaseAddress[dev] + 1)
#define I2C_MYADDR_REG(dev)     (BaseAddress[dev] + 2)
#define I2C_ADDR_REG(dev)       (BaseAddress[dev] + 3)
#define I2C_DATA_REG(dev)       (BaseAddress[dev] + 4)
#define I2C_CLKCTRL1_REG(dev)   (BaseAddress[dev] + 5)
#define I2C_CLKCTRL2_REG(dev)   (BaseAddress[dev] + 6)
#define I2C_EXCTRL_REG(dev)     (BaseAddress[dev] + 7)

DMPAPI(bool) i2c_Reset(int dev) {
    unsigned long nowtime;
    
    io_outpb(I2C_EXCTRL_REG(dev), io_inpb(I2C_EXCTRL_REG(dev)) | 0x80);

    nowtime = timer_NowTime();
    while (((io_inpb(I2C_EXCTRL_REG(dev)) & 0x80) != 0) && ((timer_NowTime() - nowtime) < I2C_TIMEOUT));

    if ((io_inpb(I2C_EXCTRL_REG(dev)) & 0x80) != 0)
    {
        err_print("fail to reset I2C module %d", dev);
        return false;
    }
    
    //Remarks: due to Vortex86DX's poor I2C design, RESET bit may become 0 when dummy clocks are still being output
    timer_Delay(20L); //lazy trick to tackle the above issue
    return true;
}


DMPAPI(void) i2c_EnableNoiseFilter(int dev) {
    if (vx86_CpuID() == CPU_VORTEX86DX_A) return;
    io_outpb(I2C_EXCTRL_REG(dev), io_inpb(I2C_EXCTRL_REG(dev)) & 0xdf);
}

DMPAPI(void) i2c_DisableNoiseFilter(int dev) {
    if (vx86_CpuID() == CPU_VORTEX86DX_A) return;
    io_outpb(I2C_EXCTRL_REG(dev), io_inpb(I2C_EXCTRL_REG(dev)) | 0x20);
}


DMPAPI(void) i2c_EnableStandardHSM(int dev) { //enable the standard high-speed mode that is compatible with Philips' SPEC.
    if (vx86_CpuID() == CPU_VORTEX86DX_A) return;
    io_outpb(I2C_EXCTRL_REG(dev), io_inpb(I2C_EXCTRL_REG(dev)) & 0xef);
}

DMPAPI(void) i2c_DisableStandardHSM(int dev) { //note: this will force SCL pin to be always open drained
    if (vx86_CpuID() == CPU_VORTEX86DX_A) return;
    io_outpb(I2C_EXCTRL_REG(dev), io_inpb(I2C_EXCTRL_REG(dev)) | 0x10);
}


DMPAPI(void) i2c_EnableINT(int dev, unsigned char i2cints) {
    io_outpb(I2C_CTRL_REG(dev), io_inpb(I2C_CTRL_REG(dev)) | i2cints);
}

DMPAPI(void) i2c_DisableINT(int dev, unsigned char i2cints) {
    io_outpb(I2C_CTRL_REG(dev), io_inpb(I2C_CTRL_REG(dev)) & (~i2cints));
}


DMPAPI(void) i2c_SetCLKREG(int dev, int mode, unsigned char prescale1, unsigned char prescale2) {
    if ((prescale1 < 10) || (prescale2 > 127))
    {
        err_print("WARNING: %s() receives wrong clk divisors!\n", __FUNCTION__);
        return;
    }

    if (vx86_CpuID() == CPU_VORTEX86DX_A) prescale1 = prescale1 - 2;

    if (mode == I2CMODE_FAST) prescale2 = prescale2 | 0x80;
    io_outpb(I2C_CLKCTRL1_REG(dev), prescale1);
    io_outpb(I2C_CLKCTRL2_REG(dev), prescale2);
}


DMPAPI(void) i2c_ClearSTAT(int dev, unsigned char i2cstats) {
    io_outpb(I2C_STAT_REG(dev), i2cstats);
}

DMPAPI(unsigned char) i2c_ReadStatREG(int dev) {
    return io_inpb(I2C_STAT_REG(dev)); 
}

DMPAPI(bool) i2c_IsMaster(int dev) {
    if ((io_inpb(I2C_STAT_REG(dev)) & 0x01) == 0) return false; else return true;
}

//Remarks: for DX ver.2, the BusBusy bit will automatically be cleared to 0 if the I2C module detects no signal over 1.96ms (stupid design!)
DMPAPI(bool) i2c_CheckBusBusy(int dev) {
    if ((io_inpb(I2C_STAT_REG(dev)) & 0x02) == 0) return false; else return true;
}

DMPAPI(bool) i2c_CheckTXDone(int dev) {
    if ((io_inpb(I2C_STAT_REG(dev)) & 0x20) == 0) return false; else return true;
}

DMPAPI(void) i2c_ClearTXDone(int dev) {
    io_outpb(I2C_STAT_REG(dev), 0x20);
}

DMPAPI(bool) i2c_CheckRXRdy(int dev) {
    if ((io_inpb(I2C_STAT_REG(dev)) & 0x40) == 0) return false; else return true;
}

DMPAPI(void) i2c_ClearRXRdy(int dev) {
    io_outpb(I2C_STAT_REG(dev), 0x40);
}


DMPAPI(void) i2c_WriteDataREG(int dev, unsigned char val) {
    io_outpb(I2C_DATA_REG(dev), val);
}

DMPAPI(unsigned char) i2c_ReadDataREG(int dev) {
    return io_inpb(I2C_DATA_REG(dev));
}
/*---------------------  end of General Functions  --------------------------*/



/*******************  Master Functions for Individual Module *****************/
DMPAPI(void) i2cmaster_SetStopBit(int dev) {
    io_outpb(I2C_CTRL_REG(dev), io_inpb(I2C_CTRL_REG(dev)) | 0x02);
}

DMPAPI(bool) i2cmaster_CheckStopBit(int dev) {
    if ((io_inpb(I2C_CTRL_REG(dev)) & 0x02) != 0) return true; else return false;
}


DMPAPI(bool) i2cmaster_CheckARLoss(int dev) {
    if ((io_inpb(I2C_STAT_REG(dev)) & 0x08) == 0) return false; else return true;
}

DMPAPI(void) i2cmaster_ClearARLoss(int dev) {
    io_outpb(I2C_STAT_REG(dev), 0x08);
}

DMPAPI(bool) i2cmaster_CheckAckErr(int dev) {
    if ((io_inpb(I2C_STAT_REG(dev)) & 0x10) == 0) return false; else return true;
}

DMPAPI(void) i2cmaster_ClearAckErr(int dev) {
    io_outpb(I2C_STAT_REG(dev), 0x10);
}


DMPAPI(void) i2cmaster_WriteAddrREG(int dev, unsigned char addr, unsigned char rwbit) {
    io_outpb(I2C_ADDR_REG(dev), (addr << 1) + rwbit);
}
/*----------------------  end of Master Functions  --------------------------*/



/********************  Slave Functions for Individual Module *****************/
DMPAPI(void) i2cslave_EnableACK(int dev) {
    io_outpb(I2C_CTRL_REG(dev), io_inpb(I2C_CTRL_REG(dev)) & 0xfe);
}

DMPAPI(void) i2cslave_EnableNAK(int dev) {
    io_outpb(I2C_CTRL_REG(dev), io_inpb(I2C_CTRL_REG(dev)) | 0x01);
}

DMPAPI(void) i2cslave_SetAddr(int dev, unsigned char addr) {
    io_outpb(I2C_MYADDR_REG(dev), (addr << 1));// | 0x01); // Default: Enable TCE bit
}


DMPAPI(bool) i2cslave_CheckSlaveWREQ(int dev) {
    if ((io_inpb(I2C_STAT_REG(dev)) & 0x80) != 0) return true; else return false;
}

DMPAPI(void) i2cslave_ClearSlaveWREQ(int dev) {
    io_outpb(I2C_STAT_REG(dev), 0x80);
}
/*-----------------------  end of Slave Functions  --------------------------*/

