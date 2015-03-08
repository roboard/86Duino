/*
  i2cex.h - Part of DM&P Vortex86 I2C Bus library
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

  Modified 01 November 2013 by Android Lin 
    
  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)
*/

#ifndef __I2CDX_H
#define __I2CDX_H

#include "dmpcfg.h"


#ifdef __cplusplus
extern "C" {
#endif

DMPAPI(void) i2c_SetBaseAddress(unsigned baseaddr);
DMPAPI(unsigned) i2c_SetDefaultBaseAddress(void);

DMPAPI(void) i2c_SetIRQ(int i2c0irq, int i2c1irq);
//-- values for the "i2c0irq" and "i2c1irq" argument
     #define I2CIRQ1         (8)   //don't want to use enum:p
     #define I2CIRQ3         (2)
     #define I2CIRQ4         (4)
     #define I2CIRQ5         (5)
     #define I2CIRQ6         (7)
     #define I2CIRQ7         (6)
     #define I2CIRQ9         (1)
     #define I2CIRQ10        (3)
     #define I2CIRQ11        (9)
     #define I2CIRQ12        (11)
     #define I2CIRQ14        (13)
     #define I2CIRQ15        (15)
     #define I2CIRQ_DISABLE  (0)



/******************  General Functions for Individual Module *****************/
DMPAPI(bool) i2c_Reset(int dev);
//-- if the above function return false, roboio_GetErrCode() may return:
//   #define ERROR_I2CFAIL		    (ERR_NOERROR + 620)

DMPAPI(void) i2c_EnableNoiseFilter(int dev);
DMPAPI(void) i2c_DisableNoiseFilter(int dev);

DMPAPI(void) i2c_EnableStandardHSM(int dev);
DMPAPI(void) i2c_DisableStandardHSM(int dev);

DMPAPI(void) i2c_EnableINT(int dev, unsigned char i2cints);
DMPAPI(void) i2c_DisableINT(int dev, unsigned char i2cints);
//-- values for the "i2cints" argument
     #define I2CINT_SLAVEWREQ       (0x80)
     #define I2CINT_RXRDY           (0x40)
     #define I2CINT_TXDONE          (0x20)
     #define I2CINT_ACKERR          (0x10)
     #define I2CINT_ARLOSS          (0x08)
     #define I2CINT_SLAVESTOPPED    (0x04)
     #define I2CINT_ALL             (0xfc)

DMPAPI(void) i2c_SetCLKREG(int dev, int mode, unsigned char prescale1, unsigned char prescale2);
//-- values for the "mode" argument
     #define I2CMODE_STANDARD       (0)
     #define I2CMODE_FAST           (1)

DMPAPI(void) i2c_ClearSTAT(int dev, unsigned char i2cstats);
//-- values for the "i2cstats" argument
     #define I2CSTAT_SLAVEWREQ      (0x80)
     #define I2CSTAT_RXRDY          (0x40)
     #define I2CSTAT_TXDONE         (0x20)
     #define I2CSTAT_ACKERR         (0x10)
     #define I2CSTAT_ARLOSS         (0x08)
     #define I2CSTAT_SLAVESTOPPED   (0x04)
     #define I2CSTAT_ALL            (0xfc)

DMPAPI(unsigned char) i2c_ReadStatREG(int dev);

DMPAPI(bool) i2c_IsMaster(int dev);

DMPAPI(bool) i2c_CheckBusBusy(int dev);

DMPAPI(bool) i2c_CheckTXDone(int dev);
DMPAPI(void) i2c_ClearTXDone(int dev);

DMPAPI(bool) i2c_CheckRXRdy(int dev);
DMPAPI(void) i2c_ClearRXRdy(int dev);

DMPAPI(void) i2c_WriteDataREG(int dev, unsigned char val);
DMPAPI(unsigned char) i2c_ReadDataREG(int dev);
/*---------------------  end of General Functions  --------------------------*/



/*******************  Master Functions for Individual Module *****************/
DMPAPI(void) i2cmaster_SetStopBit(int dev);
DMPAPI(bool) i2cmaster_CheckStopBit(int dev);

DMPAPI(bool) i2cmaster_CheckARLoss(int dev);
DMPAPI(void) i2cmaster_ClearARLoss(int dev);

DMPAPI(bool) i2cmaster_CheckAckErr(int dev);
DMPAPI(void) i2cmaster_ClearAckErr(int dev);

DMPAPI(void) i2cmaster_WriteAddrREG(int dev, unsigned char addr, unsigned char rwbit);
//-- values for the "rwbit" argument
     #define I2C_WRITE              (0)
     #define I2C_READ               (1)
/*----------------------  end of Master Functions  --------------------------*/



/********************  Slave Functions for Individual Module *****************/
DMPAPI(void) i2cslave_EnableACK(int dev);
DMPAPI(void) i2cslave_EnableNAK(int dev);

DMPAPI(void) i2cslave_SetAddr(int dev, unsigned char addr);

DMPAPI(bool) i2cslave_CheckSlaveWREQ(int dev);
DMPAPI(void) i2cslave_ClearSlaveWREQ(int dev);
/*-----------------------  end of Slave Functions  --------------------------*/

#ifdef __cplusplus
}
#endif



/****************************  Inline Functions  **************************/
#ifdef ROBOIO_DLL //use no inline functions for DLL
#ifdef __cplusplus
extern "C" {
#endif
    DMPAPI(bool) i2c0_Reset(void);
    DMPAPI(bool) i2c1_Reset(void);
    
    DMPAPI(void) i2c0_EnableNoiseFilter(void);
    DMPAPI(void) i2c1_EnableNoiseFilter(void);
    DMPAPI(void) i2c0_DisableNoiseFilter(void);
    DMPAPI(void) i2c1_DisableNoiseFilter(void);
    
    DMPAPI(void) i2c0_EnableStandardHSM(void);
    DMPAPI(void) i2c1_EnableStandardHSM(void);
    DMPAPI(void) i2c0_DisableStandardHSM(void);
    DMPAPI(void) i2c1_DisableStandardHSM(void);

    DMPAPI(void) i2c0_EnableINT(unsigned char i2cints);
    DMPAPI(void) i2c1_EnableINT(unsigned char i2cints);
    DMPAPI(void) i2c0_DisableINT(unsigned char i2cints);
    DMPAPI(void) i2c1_DisableINT(unsigned char i2cints);

    DMPAPI(void) i2c0_SetCLKREG(int mode, unsigned char prescale1, unsigned char prescale2);
    DMPAPI(void) i2c1_SetCLKREG(int mode, unsigned char prescale1, unsigned char prescale2);

    DMPAPI(void) i2c0_ClearSTAT(unsigned char i2cstats);
    DMPAPI(void) i2c1_ClearSTAT(unsigned char i2cstats);

    DMPAPI(unsigned char) i2c0_ReadStatREG(void);
    DMPAPI(unsigned char) i2c1_ReadStatREG(void);

    DMPAPI(bool) i2c0_IsMaster(void);
    DMPAPI(bool) i2c1_IsMaster(void);

    DMPAPI(bool) i2c0_CheckBusBusy(void);
    DMPAPI(bool) i2c1_CheckBusBusy(void);

    DMPAPI(bool) i2c0_CheckTXDone(void);
    DMPAPI(bool) i2c1_CheckTXDone(void);
    DMPAPI(void) i2c0_ClearTXDone(void);
    DMPAPI(void) i2c1_ClearTXDone(void);

    DMPAPI(bool) i2c0_CheckRXRdy(void);
    DMPAPI(bool) i2c1_CheckRXRdy(void);
    DMPAPI(void) i2c0_ClearRXRdy(void);
    DMPAPI(void) i2c1_ClearRXRdy(void);

    DMPAPI(void) i2c0_WriteDataREG(unsigned char val);
    DMPAPI(void) i2c1_WriteDataREG(unsigned char val);
    DMPAPI(unsigned char) i2c0_ReadDataREG(void);
    DMPAPI(unsigned char) i2c1_ReadDataREG(void);

    DMPAPI(void) i2c0master_SetStopBit(void);
    DMPAPI(void) i2c1master_SetStopBit(void);
    DMPAPI(bool) i2c0master_CheckStopBit(void);
    DMPAPI(bool) i2c1master_CheckStopBit(void);

    DMPAPI(bool) i2c0master_CheckARLoss(void);
    DMPAPI(bool) i2c1master_CheckARLoss(void);
    DMPAPI(void) i2c0master_ClearARLoss(void);
    DMPAPI(void) i2c1master_ClearARLoss(void);

    DMPAPI(bool) i2c0master_CheckAckErr(void);
    DMPAPI(bool) i2c1master_CheckAckErr(void);
    DMPAPI(void) i2c0master_ClearAckErr(void);
    DMPAPI(void) i2c1master_ClearAckErr(void);

    DMPAPI(void) i2c0master_WriteAddrREG(unsigned char addr, unsigned char rwbit);
    DMPAPI(void) i2c1master_WriteAddrREG(unsigned char addr, unsigned char rwbit);

    DMPAPI(void) i2c0slave_EnableACK(void);
    DMPAPI(void) i2c1slave_EnableACK(void);
    DMPAPI(void) i2c0slave_EnableNAK(void);
    DMPAPI(void) i2c1slave_EnableNAK(void);

    DMPAPI(void) i2c0slave_SetAddr(unsigned char addr);
    DMPAPI(void) i2c1slave_SetAddr(unsigned char addr);

    DMPAPI(bool) i2c0slave_CheckSlaveWREQ(void);
    DMPAPI(bool) i2c1slave_CheckSlaveWREQ(void);
    DMPAPI(void) i2c0slave_ClearSlaveWREQ(void);
    DMPAPI(void) i2c1slave_ClearSlaveWREQ(void);
#ifdef __cplusplus
}
#endif
#endif

#if !defined(ROBOIO_DLL) || defined(__I2CDX_LIB)
    DMP_INLINE(bool) i2c0_Reset(void) {
        return i2c_Reset(0);
    }
    DMP_INLINE(bool) i2c1_Reset(void) {
        return i2c_Reset(1);
    }

    DMP_INLINE(void) i2c0_EnableNoiseFilter(void) {
        i2c_EnableNoiseFilter(0);
    }
    DMP_INLINE(void) i2c1_EnableNoiseFilter(void) {
        i2c_EnableNoiseFilter(1);
    }
    DMP_INLINE(void) i2c0_DisableNoiseFilter(void) {
        i2c_DisableNoiseFilter(0);
    }
    DMP_INLINE(void) i2c1_DisableNoiseFilter(void) {
        i2c_DisableNoiseFilter(1);
    }
    
    DMP_INLINE(void) i2c0_EnableStandardHSM(void) {
        i2c_EnableStandardHSM(0);
    }
    DMP_INLINE(void) i2c1_EnableStandardHSM(void) {
        i2c_EnableStandardHSM(1);
    }
    DMP_INLINE(void) i2c0_DisableStandardHSM(void) {
        i2c_DisableStandardHSM(0);
    }
    DMP_INLINE(void) i2c1_DisableStandardHSM(void) {
        i2c_DisableStandardHSM(1);
    }

    DMP_INLINE(void) i2c0_EnableINT(unsigned char i2cints) {
        i2c_EnableINT(0, i2cints);
    }
    DMP_INLINE(void) i2c1_EnableINT(unsigned char i2cints) {
        i2c_EnableINT(1, i2cints);
    }
    DMP_INLINE(void) i2c0_DisableINT(unsigned char i2cints) {
        i2c_DisableINT(0, i2cints);
    }
    DMP_INLINE(void) i2c1_DisableINT(unsigned char i2cints) {
        i2c_DisableINT(1, i2cints);
    }

    DMP_INLINE(void) i2c0_SetCLKREG(int mode, unsigned char prescale1, unsigned char prescale2) {
        i2c_SetCLKREG(0, mode, prescale1, prescale2);
    }
    DMP_INLINE(void) i2c1_SetCLKREG(int mode, unsigned char prescale1, unsigned char prescale2) {
        i2c_SetCLKREG(1, mode, prescale1, prescale2);
    }

    DMP_INLINE(void) i2c0_ClearSTAT(unsigned char i2cstats) {
        i2c_ClearSTAT(0, i2cstats);
    }
    DMP_INLINE(void) i2c1_ClearSTAT(unsigned char i2cstats) {
        i2c_ClearSTAT(1, i2cstats);
    }

    DMP_INLINE(unsigned char) i2c0_ReadStatREG(void) {
        return i2c_ReadStatREG(0);
    }
    DMP_INLINE(unsigned char) i2c1_ReadStatREG(void) {
        return i2c_ReadStatREG(1);
    }

    DMP_INLINE(bool) i2c0_IsMaster(void) {
        return i2c_IsMaster(0);
    }
    DMP_INLINE(bool) i2c1_IsMaster(void) {
        return i2c_IsMaster(1);
    }

    DMP_INLINE(bool) i2c0_CheckBusBusy(void) {
        return i2c_CheckBusBusy(0);
    }
    DMP_INLINE(bool) i2c1_CheckBusBusy(void) {
        return i2c_CheckBusBusy(1);
    }

    DMP_INLINE(bool) i2c0_CheckTXDone(void) {
        return i2c_CheckTXDone(0);
    }
    DMP_INLINE(bool) i2c1_CheckTXDone(void) {
        return i2c_CheckTXDone(1);
    }
    DMP_INLINE(void) i2c0_ClearTXDone(void) {
        i2c_ClearTXDone(0);
    }
    DMP_INLINE(void) i2c1_ClearTXDone(void) {
        i2c_ClearTXDone(1);
    }

    DMP_INLINE(bool) i2c0_CheckRXRdy(void) {
        return i2c_CheckRXRdy(0);
    }
    DMP_INLINE(bool) i2c1_CheckRXRdy(void) {
        return i2c_CheckRXRdy(1);
    }
    DMP_INLINE(void) i2c0_ClearRXRdy(void) {
        i2c_ClearRXRdy(0);
    }
    DMP_INLINE(void) i2c1_ClearRXRdy(void) {
        i2c_ClearRXRdy(1);
    }

    DMP_INLINE(void) i2c0_WriteDataREG(unsigned char val) {
        i2c_WriteDataREG(0, val);
    }
    DMP_INLINE(void) i2c1_WriteDataREG(unsigned char val) {
        i2c_WriteDataREG(1, val);
    }
    DMP_INLINE(unsigned char) i2c0_ReadDataREG(void) {
        return i2c_ReadDataREG(0);
    }
    DMP_INLINE(unsigned char) i2c1_ReadDataREG(void) {
        return i2c_ReadDataREG(1);
    }

    DMP_INLINE(void) i2c0master_SetStopBit(void) {
        i2cmaster_SetStopBit(0);
    }
    DMP_INLINE(void) i2c1master_SetStopBit(void) {
        i2cmaster_SetStopBit(1);
    }
    DMP_INLINE(bool) i2c0master_CheckStopBit(void) {
        return i2cmaster_CheckStopBit(0);
    }
    DMP_INLINE(bool) i2c1master_CheckStopBit(void) {
        return i2cmaster_CheckStopBit(1);
    }

    DMP_INLINE(bool) i2c0master_CheckARLoss(void) {
        return i2cmaster_CheckARLoss(0);
    }
    DMP_INLINE(bool) i2c1master_CheckARLoss(void) {
        return i2cmaster_CheckARLoss(1);
    }
    DMP_INLINE(void) i2c0master_ClearARLoss(void) {
        i2cmaster_ClearARLoss(0);
    }
    DMP_INLINE(void) i2c1master_ClearARLoss(void) {
        i2cmaster_ClearARLoss(1);
    }

    DMP_INLINE(bool) i2c0master_CheckAckErr(void) {
        return i2cmaster_CheckAckErr(0);
    }
    DMP_INLINE(bool) i2c1master_CheckAckErr(void) {
        return i2cmaster_CheckAckErr(1);
    }
    DMP_INLINE(void) i2c0master_ClearAckErr(void) {
        i2cmaster_ClearAckErr(0);
    }
    DMP_INLINE(void) i2c1master_ClearAckErr(void) {
        i2cmaster_ClearAckErr(1);
    }

    DMP_INLINE(void) i2c0master_WriteAddrREG(unsigned char addr, unsigned char rwbit) {
        i2cmaster_WriteAddrREG(0, addr, rwbit);
    }
    DMP_INLINE(void) i2c1master_WriteAddrREG(unsigned char addr, unsigned char rwbit) {
        i2cmaster_WriteAddrREG(1, addr, rwbit);
    }

    DMP_INLINE(void) i2c0slave_EnableACK(void) {
        i2cslave_EnableACK(0);
    }
    DMP_INLINE(void) i2c1slave_EnableACK(void) {
        i2cslave_EnableACK(1);
    }
    DMP_INLINE(void) i2c0slave_EnableNAK(void) {
        i2cslave_EnableNAK(0);
    }
    DMP_INLINE(void) i2c1slave_EnableNAK(void) {
        i2cslave_EnableNAK(1);
    }

    DMP_INLINE(void) i2c0slave_SetAddr(unsigned char addr) {
        i2cslave_SetAddr(0, addr);
    }
    DMP_INLINE(void) i2c1slave_SetAddr(unsigned char addr) {
        i2cslave_SetAddr(1, addr);
    }

    DMP_INLINE(bool) i2c0slave_CheckSlaveWREQ(void) {
        return i2cslave_CheckSlaveWREQ(0);
    }
    DMP_INLINE(bool) i2c1slave_CheckSlaveWREQ(void) {
        return i2cslave_CheckSlaveWREQ(1);
    }
    DMP_INLINE(void) i2c0slave_ClearSlaveWREQ(void) {
        i2cslave_ClearSlaveWREQ(0);
    }
    DMP_INLINE(void) i2c1slave_ClearSlaveWREQ(void) {
        i2cslave_ClearSlaveWREQ(1);
    }
#endif
/*-----------------------  end of Inline Functions  ----------------------*/

#endif

