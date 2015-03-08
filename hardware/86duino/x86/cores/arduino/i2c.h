/*
  i2c.h - DM&P Vortex86 I2C Bus library
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

#ifndef __I2C_H
#define __I2C_H

#include "dmpcfg.h"
#include "i2cex.h"


#ifdef __cplusplus
extern "C" {
#endif

DMPAPI(bool) i2c_InUse(void);

//note: I2C lib assumes that I/O pins for GPIO3/I2C have functioned as GPIO3 or I2C pins;
//      this is BIOS default setting in general.
DMPAPI(bool) i2c_Init2(unsigned baseaddr, unsigned devs, int i2c0irq, int i2c1irq);
//-- values for the "devs" argument (note that different values
//   can be added to use multiple modules)
     #define I2C_USEMODULE0         (1<<0)
     #define I2C_USEMODULE1         (1<<1)
//-- values for the "i2c0irq" and "i2c1irq" arguments (having been defined in "i2cdx.h")
//   #define I2CIRQ1                (8)
//   #define I2CIRQ3                (2)
//   ...... (refer to the "i2c0irq" argument of i2c_SetIRQ())
//-- if the above functions return false, roboio_GetErrCode() may return:
//   #define ERROR_RBVER_UNKNOWN    (ERR_NOERROR + 801) //need include <common.h>
//   #define ERROR_RBVER_UNMATCH    (ERR_NOERROR + 800) //need include <common.h>
//   #define ERROR_IOINITFAIL       (ERR_NOERROR + 100) //need include <io.h>
//   #define ERROR_IOSECTIONFULL    (ERR_NOERROR + 101) //need include <io.h>
//   #define ERROR_CPUUNSUPPORTED	(ERR_NOERROR + 102) //need include <io.h>
//   #define ERROR_I2C_INUSE		(ERR_NOERROR + 600)
//   #define ERROR_I2C_INITFAIL		(ERR_NOERROR + 602)

DMPAPI(void) i2c_Close(void);


DMPAPI(bool) i2c_InitSW2(unsigned devs, int i2c0mode, unsigned long i2c0clkdelay, int i2c1mode, unsigned long i2c1clkdelay);
//-- values for the "i2c0mode" & "i2c1mode" arguments
     #define I2CSW_NORMAL           (0x10)
     #define I2CSW_LEGO             (0x20)
//-- if the above functions return false, roboio_GetErrCode() may return:
//   #define ERROR_RBVER_UNKNOWN    (ERR_NOERROR + 801) //need include <common.h>
//   #define ERROR_RBVER_UNMATCH    (ERR_NOERROR + 800) //need include <common.h>
//   #define ERROR_IOINITFAIL       (ERR_NOERROR + 100) //need include <io.h>
//   #define ERROR_IOSECTIONFULL    (ERR_NOERROR + 101) //need include <io.h>
//   #define ERROR_CPUUNSUPPORTED	(ERR_NOERROR + 102) //need include <io.h>
//   #define ERROR_I2C_INUSE		(ERR_NOERROR + 600)

DMPAPI(void) i2c_CloseSW(void);


DMPAPI(unsigned long) i2c_SetSpeed(int dev, int mode, unsigned long bps);
//-- values for the "mode" argument
//   #define I2CMODE_STANDARD       (0)                 //need include <i2cdx.h>
//   #define I2CMODE_FAST           (1)                 //need include <i2cdx.h>
     #define I2CMODE_HIGHSPEED      (2)
     #define I2CMODE_AUTO           (3)
//-- if the above functions return 0xffffffffL, roboio_GetErrCode() may return:
//   #define ERROR_I2CWRONGUSAGE    (ERR_NOERROR + 630)



/********************  Master Functions for Individual Module *****************/
DMPAPI(bool) i2cmaster_Start(int dev, unsigned char addr, unsigned char rwbit);
//-- values for the "rwbit" argument (having been defined in "i2cdx.h")
//   #define I2C_WRITE              (0)
//   #define I2C_READ               (1)
//-- if the above functions return false, roboio_GetErrCode() may return:
//   #define ERROR_I2CFAIL		    (ERR_NOERROR + 610)
//   #define ERROR_I2CWRONGUSAGE    (ERR_NOERROR + 630)
//   #define ERROR_I2CARLOSS        (ERR_NOERROR + 621)
//   #define ERROR_I2CACKERR        (ERR_NOERROR + 622)

DMPAPI(bool) i2cmaster_Write(int dev, unsigned char val);
DMPAPI(bool) i2cmaster_WriteLast(int dev, unsigned char val);
//-- if the above functions return false, roboio_GetErrCode() may return:
//   #define ERROR_I2CFAIL		    (ERR_NOERROR + 610)
//   #define ERROR_I2CWRONGUSAGE    (ERR_NOERROR + 630)
//   #define ERROR_I2CARLOSS        (ERR_NOERROR + 621)
//   #define ERROR_I2CACKERR        (ERR_NOERROR + 622)

DMPAPI(unsigned) i2cmaster_Read1(int dev, bool secondlast);
DMPAPI(unsigned) i2cmaster_ReadLast(int dev);
//-- if the above functions return 0xffff, roboio_GetErrCode() may return:
//   #define ERROR_I2CFAIL		    (ERR_NOERROR + 610)
//   #define ERROR_I2CWRONGUSAGE    (ERR_NOERROR + 630)
//   #define ERROR_I2CARLOSS        (ERR_NOERROR + 621)


DMPAPI(bool) i2cmaster_StartN(int dev, unsigned char addr, unsigned char rwbit, int count);
//-- values for the "rwbit" argument (having been defined in "i2cdx.h")
//   #define I2C_WRITE              (0)
//   #define I2C_READ               (1)
//-- if the above functions return false, roboio_GetErrCode() may return:
//   #define ERROR_I2CFAIL		    (ERR_NOERROR + 610)
//   #define ERROR_I2CWRONGUSAGE    (ERR_NOERROR + 630)
//   #define ERROR_I2CARLOSS        (ERR_NOERROR + 621)
//   #define ERROR_I2CACKERR        (ERR_NOERROR + 622)

DMPAPI(bool) i2cmaster_WriteN(int dev, unsigned char val);
//-- if the above functions return false, roboio_GetErrCode() may return:
//   #define ERROR_I2CFAIL		    (ERR_NOERROR + 610)
//   #define ERROR_I2CWRONGUSAGE    (ERR_NOERROR + 630)
//   #define ERROR_I2CARLOSS        (ERR_NOERROR + 621)
//   #define ERROR_I2CACKERR        (ERR_NOERROR + 622)

DMPAPI(unsigned) i2cmaster_ReadN(int dev);
//-- if the above functions return 0xffff, roboio_GetErrCode() may return:
//   #define ERROR_I2CFAIL		    (ERR_NOERROR + 610)
//   #define ERROR_I2CWRONGUSAGE    (ERR_NOERROR + 630)
//   #define ERROR_I2CARLOSS        (ERR_NOERROR + 621)


DMPAPI(bool) i2cmaster_SetRestart(int dev, unsigned char addr, unsigned char rwbit);
DMPAPI(bool) i2cmaster_SetRestartN(int dev, unsigned char rwbit, int count);
//-- if the above functions return false, roboio_GetErrCode() may return:
//   #define ERROR_I2CWRONGUSAGE    (ERR_NOERROR + 630)
/*-----------------------  end of Master Functions  --------------------------*/



/*********************  Slave Functions for Individual Module *****************/
DMPAPI(unsigned) i2cslave_Listen(int dev);
//-- return values of the above functions
     #define I2CSLAVE_NOTHING       (0)
     #define I2CSLAVE_START         (1)
     #define I2CSLAVE_END           (2)
     #define I2CSLAVE_WAITING       (3)
     #define I2CSLAVE_WRITEREQUEST  (4)
     #define I2CSLAVE_READREQUEST   (5)
//-- if the aboves functions return 0xffff, roboio_GetErrCode() may return:
//   #define ERROR_I2CFAIL		    (ERR_NOERROR + 610)
//   #define ERROR_I2CWRONGUSAGE    (ERR_NOERROR + 630)

DMPAPI(bool) i2cslave_Write(int dev, unsigned char val);
//-- if the above functions return false, roboio_GetErrCode() may return:
//   #define ERROR_I2CFAIL		    (ERR_NOERROR + 610)
//   #define ERROR_I2CWRONGUSAGE    (ERR_NOERROR + 630)

DMPAPI(unsigned) i2cslave_Read(int dev);
//-- if the above function return 0xffff, roboio_GetErrCode() may return:
//   #define ERROR_I2CWRONGUSAGE    (ERR_NOERROR + 630)
/*------------------------  end of Slave Functions  --------------------------*/



DMPAPI(void) i2c_SetResetPin(void);
DMPAPI(void) i2c_ClearResetPin(void);

#ifdef __cplusplus
}
#endif



/****************************  Inline Functions  **************************/
#ifdef ROBOIO_DLL //use no inline functions for DLL
#ifdef __cplusplus
extern "C" {
#endif
    DMPAPI(unsigned long) i2c0_SetSpeed(int mode, unsigned long bps);
    DMPAPI(unsigned long) i2c1_SetSpeed(int mode, unsigned long bps);

    DMPAPI(bool) i2c0master_Start(unsigned char addr, unsigned char rwbit);
    DMPAPI(bool) i2c1master_Start(unsigned char addr, unsigned char rwbit);

    DMPAPI(bool) i2c0master_Write(unsigned char val);
    DMPAPI(bool) i2c1master_Write(unsigned char val);
    DMPAPI(bool) i2c0master_WriteLast(unsigned char val);
    DMPAPI(bool) i2c1master_WriteLast(unsigned char val);
    
    DMPAPI(unsigned) i2c0master_Read(void);
    DMPAPI(unsigned) i2c1master_Read(void);
    DMPAPI(unsigned) i2c0master_ReadSecondLast(void);
    DMPAPI(unsigned) i2c1master_ReadSecondLast(void);
    DMPAPI(unsigned) i2c0master_ReadLast(void);
    DMPAPI(unsigned) i2c1master_ReadLast(void);
    
    DMPAPI(bool) i2c0master_StartN(unsigned char addr, unsigned char rwbit, int count);
    DMPAPI(bool) i2c1master_StartN(unsigned char addr, unsigned char rwbit, int count);

    DMPAPI(bool) i2c0master_WriteN(unsigned char val);
    DMPAPI(bool) i2c1master_WriteN(unsigned char val);

    DMPAPI(unsigned) i2c0master_ReadN(void);
    DMPAPI(unsigned) i2c1master_ReadN(void);
    
    DMPAPI(bool) i2c0master_SetRestart(unsigned char addr, unsigned char rwbit);
    DMPAPI(bool) i2c1master_SetRestart(unsigned char addr, unsigned char rwbit);
    DMPAPI(bool) i2c0master_SetRestartN(unsigned char rwbit, int count);
    DMPAPI(bool) i2c1master_SetRestartN(unsigned char rwbit, int count);

    DMPAPI(unsigned) i2c0slave_Listen(void);
    DMPAPI(unsigned) i2c1slave_Listen(void);

    DMPAPI(bool) i2c0slave_Write(unsigned char val);
    DMPAPI(bool) i2c1slave_Write(unsigned char val);

    DMPAPI(unsigned) i2c0slave_Read(void);
    DMPAPI(unsigned) i2c1slave_Read(void);

    #ifdef ROBOIO
        DMPAPI(bool) i2c_Initialize(int i2cirq);  // for compatibility to RoBoIO 1.61

        DMPAPI(bool) i2c_Init(int spdmode, unsigned long bps);
        DMPAPI(bool) i2c_InitSW(int i2cmode, unsigned long clkdelay);

        DMPAPI(bool) i2c_Send(unsigned char addr, unsigned char* data, int datasize);
        DMPAPI(bool) i2c_Receive(unsigned char addr, unsigned char* buf, int bufsize);
        DMPAPI(bool) i2c_SensorRead(unsigned char addr, unsigned char cmd, unsigned char* buf, int bufsize);
        DMPAPI(bool) i2c_SensorReadEX(unsigned char addr, unsigned char* cmds, int cmdsize, unsigned char* buf, int bufsize);
    #endif

#ifdef __cplusplus
}
#endif
#endif

#if !defined(ROBOIO_DLL) || defined(__I2C_LIB)
    DMP_INLINE(unsigned long) i2c0_SetSpeed(int mode, unsigned long bps) {
        return i2c_SetSpeed(0, mode, bps);
    }
    DMP_INLINE(unsigned long) i2c1_SetSpeed(int mode, unsigned long bps) {
        return i2c_SetSpeed(1, mode, bps);
    }

    DMP_INLINE(bool) i2c0master_Start(unsigned char addr, unsigned char rwbit) {
        return i2cmaster_Start(0, addr, rwbit);
    }
    DMP_INLINE(bool) i2c1master_Start(unsigned char addr, unsigned char rwbit) {
        return i2cmaster_Start(1, addr, rwbit);
    }

    DMP_INLINE(bool) i2c0master_Write(unsigned char val) {
        return i2cmaster_Write(0, val);
    }
    DMP_INLINE(bool) i2c1master_Write(unsigned char val) {
        return i2cmaster_Write(1, val);
    }
    DMP_INLINE(bool) i2c0master_WriteLast(unsigned char val) {
        return i2cmaster_WriteLast(0, val);
    }
    DMP_INLINE(bool) i2c1master_WriteLast(unsigned char val) {
        return i2cmaster_WriteLast(1, val);
    }

    DMP_INLINE(unsigned) i2c0master_Read(void) {
        return i2cmaster_Read1(0, false);
    }
    DMP_INLINE(unsigned) i2c1master_Read(void) {
        return i2cmaster_Read1(1, false);
    }
    DMP_INLINE(unsigned) i2c0master_ReadSecondLast(void) {
        return i2cmaster_Read1(0, true);
    }
    DMP_INLINE(unsigned) i2c1master_ReadSecondLast(void) {
        return i2cmaster_Read1(1, true);
    }
    DMP_INLINE(unsigned) i2c0master_ReadLast(void) {
        return i2cmaster_ReadLast(0);
    }
    DMP_INLINE(unsigned) i2c1master_ReadLast(void) {
        return i2cmaster_ReadLast(1);
    }

    DMP_INLINE(bool) i2c0master_StartN(unsigned char addr, unsigned char rwbit, int count) {
        return i2cmaster_StartN(0, addr, rwbit, count);
    }
    DMP_INLINE(bool) i2c1master_StartN(unsigned char addr, unsigned char rwbit, int count) {
        return i2cmaster_StartN(1, addr, rwbit, count);
    }

    DMP_INLINE(bool) i2c0master_WriteN(unsigned char val) {
        return i2cmaster_WriteN(0, val);
    }
    DMP_INLINE(bool) i2c1master_WriteN(unsigned char val) {
        return i2cmaster_WriteN(1, val);
    }

    DMP_INLINE(unsigned) i2c0master_ReadN(void) {
        return i2cmaster_ReadN(0);
    }
    DMP_INLINE(unsigned) i2c1master_ReadN(void) {
        return i2cmaster_ReadN(1);
    }

    DMP_INLINE(bool) i2c0master_SetRestart(unsigned char addr, unsigned char rwbit) {
        return i2cmaster_SetRestart(0, addr, rwbit);
    }
    DMP_INLINE(bool) i2c1master_SetRestart(unsigned char addr, unsigned char rwbit) {
        return i2cmaster_SetRestart(1, addr, rwbit);
    }
    DMP_INLINE(bool) i2c0master_SetRestartN(unsigned char rwbit, int count) {
        return i2cmaster_SetRestartN(0, rwbit, count);
    }
    DMP_INLINE(bool) i2c1master_SetRestartN(unsigned char rwbit, int count) {
        return i2cmaster_SetRestartN(1, rwbit, count);
    }

    DMP_INLINE(unsigned) i2c0slave_Listen(void) {
        return i2cslave_Listen(0);
    }
    DMP_INLINE(unsigned) i2c1slave_Listen(void) {
        return i2cslave_Listen(1);
    }

    DMP_INLINE(bool) i2c0slave_Write(unsigned char val) {
        return i2cslave_Write(0, val);
    }
    DMP_INLINE(bool) i2c1slave_Write(unsigned char val) {
        return i2cslave_Write(1, val);
    }

    DMP_INLINE(unsigned) i2c0slave_Read(void) {
        return i2cslave_Read(0);
    }
    DMP_INLINE(unsigned) i2c1slave_Read(void) {
        return i2cslave_Read(1);
    }

    #ifdef ROBOIO
        DMP_INLINE(bool) i2c_Initialize(int i2cirq) {  // for compatibility to RoBoIO 1.61
            return i2c_Init2(0xffff, I2C_USEMODULE0, i2cirq, I2CIRQ_DISABLE);
        }

        DMP_INLINE(bool) i2c_Init(int spdmode, unsigned long bps) {
            if (i2c_Initialize(I2CIRQ_DISABLE) == false) return false;
            if (i2c0_SetSpeed(spdmode, bps) == 0xffffffffL) return false;

            return true;
        }

        DMP_INLINE(bool) i2c_InitSW(int i2cmode, unsigned long clkdelay) {
            return i2c_InitSW2(I2C_USEMODULE0, i2cmode, clkdelay, i2cmode, 0L);
        }


        DMP_INLINE(bool) i2c_Send(unsigned char addr, unsigned char* data, int datasize) {
            int i;
            
            if (i2c0master_StartN(addr, I2C_WRITE, datasize) == false) return false;
            for (i=0; i<datasize; i++)
                if (i2c0master_WriteN(data[i]) == false) return false;
                
            return true;
        }

        DMP_INLINE(bool) i2c_Receive(unsigned char addr, unsigned char* buf, int bufsize) {
            int i; unsigned int val;
            
            if (i2c0master_StartN(addr, I2C_READ, bufsize) == false) return false;
            for (i=0; i<bufsize; i++)
            {
                if ((val = i2c0master_ReadN()) == 0xffff) return false;
                buf[i] = (unsigned char)(val & 0xff);
            }

            return true;
        }

        DMP_INLINE(bool) i2c_SensorReadEX(unsigned char addr, unsigned char* cmds, int cmdsize, unsigned char* buf, int bufsize) {
            int i; unsigned int val;
            
            if (i2c0master_StartN(addr, I2C_WRITE, cmdsize) == false) return false;
            if (i2c0master_SetRestartN(I2C_READ, bufsize)   == false) return false;

            for (i=0; i<cmdsize; i++)
                if (i2c0master_WriteN(cmds[i]) == false) return false;

            for (i=0; i<bufsize; i++)
            {
                if ((val = i2c0master_ReadN()) == 0xffff) return false;
                buf[i] = (unsigned char)(val & 0xff);
            }

            return true;
        }

        DMP_INLINE(bool) i2c_SensorRead(unsigned char addr, unsigned char cmd, unsigned char* buf, int bufsize) {
            return i2c_SensorReadEX(addr, &cmd, 1, buf, bufsize);
        }
    #endif

#endif
/*-----------------------  end of Inline Functions  ----------------------*/

#endif

