/*
  RMG146.h - Part of DM&P Vortex86 FreeIMU1 library
  Copyright (c) 2016 DY Huang <Dyhung@dmp.com.tw>. All right reserved.

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

#ifndef __RMG146_H
#define __RMG146_H

#ifdef __cplusplus
extern "C" {
#endif

#define RMG146_A_DATARATE_50HZ          (0x00)
#define RMG146_A_DATARATE_100HZ         (0x01)
#define RMG146_A_DATARATE_400HZ         (0x02)
#define RMG146_A_DATARATE_1KHZ          (0x03)

#define RMG146_A_FULLSCALE_PM2G         (0x00)
#define RMG146_A_FULLSCALE_PM4G         (0x01)
#define RMG146_A_FULLSCALE_PM8G         (0x03)

#define RMG146_G_FULLSCALE_250DPS       (0x00)
#define RMG146_G_FULLSCALE_500DPS       (0x01)
#define RMG146_G_FULLSCALE_1KDPS        (0x02)
#define RMG146_G_FULLSCALE_2KDPS        (0x03)

#define RMG146_M_FULLSCALE_1P3_GAUSS    (0x01)
#define RMG146_M_FULLSCALE_1P9_GAUSS    (0x02)
#define RMG146_M_FULLSCALE_2P5_GAUSS    (0x03)
#define RMG146_M_FULLSCALE_4P0_GAUSS    (0x04)
#define RMG146_M_FULLSCALE_4P7_GAUSS    (0x05)
#define RMG146_M_FULLSCALE_5P6_GAUSS    (0x06)
#define RMG146_M_FULLSCALE_8P1_GAUSS    (0x07)

void *rmg146_init(int acc_addr, int gyro_addr, int magn_addr);
bool  rmg146_getraw(void *h, int *raw);
void  rmg146_raw2value(void *h, int *src, double *dst);
void  rmg146_close(void *h);

bool  rmg146_setAccelDataRate(void *h, int rate);
bool  rmg146_setFullScaleAccelRange(void *h, int scale);
bool  rmg146_setFullScaleGyroRange(void *h, int scale);
bool  rmg146_setFullScaleMagnRange(void *h, int scale);

#ifdef __cplusplus
}
#endif

#endif
