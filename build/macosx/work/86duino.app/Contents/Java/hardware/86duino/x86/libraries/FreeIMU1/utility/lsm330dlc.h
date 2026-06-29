/*
  LSM330DLC.h - Part of DM&P Vortex86 FreeIMU1 library
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

#ifndef __LSM330DLC_H
#define __LSM330DLC_H

#ifdef __cplusplus
extern "C" {
#endif

#define LSM330DLC_A_DATARATE_1HZ      (0x10)
#define LSM330DLC_A_DATARATE_10HZ     (0x20)
#define LSM330DLC_A_DATARATE_25HZ     (0x30)
#define LSM330DLC_A_DATARATE_50HZ     (0x40)
#define LSM330DLC_A_DATARATE_100HZ    (0x50)
#define LSM330DLC_A_DATARATE_200HZ    (0x60)
#define LSM330DLC_A_DATARATE_400HZ    (0x70)

#define LSM330DLC_A_FULLSCALE_PM2G    (0x00)
#define LSM330DLC_A_FULLSCALE_PM4G    (0x01)
#define LSM330DLC_A_FULLSCALE_PM8G    (0x02)
#define LSM330DLC_A_FULLSCALE_PM16G   (0x03)

#define LSM330DLC_G_FULLSCALE_250DPS  (0x00)
#define LSM330DLC_G_FULLSCALE_500DPS  (0x01)
#define LSM330DLC_G_FULLSCALE_2KDPS1  (0x02)
#define LSM330DLC_G_FULLSCALE_2KDPS2  (0x03)

void *lsm330dlc_init(int acc_addr, int gyro_addr, int magn_addr);
bool  lsm330dlc_getraw(void *h, int *raw);
void  lsm330dlc_raw2value(void *h, int *src, double *dst);
void  lsm330dlc_close(void *h);

bool  lsm330dlc_setAccelDataRate(void *h, int rate);
bool  lsm330dlc_setFullScaleAccelRange(void *h, int scale);
bool  lsm330dlc_enableGyro(void *h);
bool  lsm330dlc_setFullScaleGyroRange(void *h, int scale);

#ifdef __cplusplus
}
#endif

#endif
