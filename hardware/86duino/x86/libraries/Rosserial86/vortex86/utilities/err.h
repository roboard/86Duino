/*******************************************************************************

  err.h - DM&P Error Logging Library
  Copyright (c) 2014 AAA <aaa@dmp.com.tw>. All right reserved.

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

*******************************************************************************/


#ifndef __ERR_H
#define __ERR_H

// #define DEBUG_WITH_UART

#include <dmpcfg.h>


#ifdef __cplusplus
extern "C" {
#endif

DMPAPI_C(void) err_print(const char* fmt, ...);
DMPAPI_C(void) err_printk(const char* fmt, ...);

DMPAPI(bool) err_Dump2Buf(char* buf);  // sizeof(buf) must >= 256
DMPAPI(bool) err_Dump(void);
DMPAPI(void) err_DumpAll(void);
DMPAPI(bool) err_AutoDump(void);

DMPAPI(bool) err_Init(const char* logfile);
DMPAPI(bool) err_Close(void);

#ifdef DEBUG_WITH_UART
    DMPAPI(unsigned int) err_getch(void);
#endif

#ifdef __cplusplus
}
#endif
#endif

