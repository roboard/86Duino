/*
  LSM330DLC.cpp - Part of DM&P Vortex86 FreeIMU1 library
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
#include "lsm330dlc.h"

#include "Arduino.h"
#include "Wire.h"

static int getRegVal(unsigned char addr, unsigned char offset, unsigned char *pVal) {
    int i;

    Wire.beginTransmission(addr);
    Wire.write(offset);
    Wire.endTransmission();

    Wire.requestFrom((int)addr, 6);
    i = 0;
    while (Wire.available() > 0 && i < 6) {
        pVal[i++] = (unsigned char)Wire.read();
    }

    return i;
}

struct lsm330dlc {
    
    int accAddr;
    int gyroAddr;
    double accFactor;
    double gyroFactor;
    
};

void *lsm330dlc_init(int acc_addr, int gyro_addr, int magn_addr)
{
    struct lsm330dlc *l;
    void *h;
    
    if (acc_addr == 0xFF && gyro_addr == 0xFF) {
        return NULL;
    }
    
    h = (void *)malloc(sizeof(struct lsm330dlc));
    if (h == NULL) {
        return NULL;
    }
    l = (struct lsm330dlc *)h;
    
    l->accAddr = acc_addr;
    l->gyroAddr = gyro_addr;
    l->accFactor = 1.0;
    l->gyroFactor = 8.75;
    
    if (l->accAddr != 0xFF && lsm330dlc_setAccelDataRate(h, LSM330DLC_A_DATARATE_400HZ) == false) {
        free(h);
        return NULL;
    }
    
    if (l->accAddr != 0xFF && lsm330dlc_setFullScaleAccelRange(h, LSM330DLC_A_FULLSCALE_PM16G) == false) {
        free(h);
        return NULL;
    }

    if (l->gyroAddr != 0xFF && lsm330dlc_enableGyro(h) == false) {
        free(h);
        return NULL;
    }
    
    if (l->gyroAddr != 0xFF && lsm330dlc_setFullScaleGyroRange(h, LSM330DLC_G_FULLSCALE_2KDPS1) == false) {
        free(h);
        return NULL;
    }
    
    return h;
}

void  lsm330dlc_close(void *h)
{
    if (h) free(h);
}

bool lsm330dlc_getraw(void *h, int *raw)
{
    struct lsm330dlc *l = (struct lsm330dlc *)h;
    unsigned char temp[12];
    short word[6];
    int error = 0;
    
    if (l->accAddr != 0xFF && getRegVal( l->accAddr, 0xA8, &temp[0]) != 6) {
        error++;
    } else if (l->gyroAddr != 0xFF && getRegVal(l->gyroAddr, 0xA8, &temp[6]) != 6) {
        error++;
    }
    
    if (error == 0) {
        
        if (l->accAddr != 0xFF) {
            word[0] = (short)temp[0]  | ((short)temp[1]  << 8);
            word[1] = (short)temp[2]  | ((short)temp[3]  << 8);
            word[2] = (short)temp[4]  | ((short)temp[5]  << 8);
            raw[0] = (int)word[0];
            raw[1] = (int)word[1];
            raw[2] = (int)word[2];
        } else {
            raw[0] = 0;
            raw[1] = 0;
            raw[2] = 0;
        }
        
        if (l->gyroAddr != 0xFF) {
            word[3] = (short)temp[6]  | ((short)temp[7]  << 8);
            word[4] = (short)temp[8]  | ((short)temp[9]  << 8);
            word[5] = (short)temp[10] | ((short)temp[11] << 8);
            raw[3] = (int)word[3];
            raw[4] = (int)word[4];
            raw[5] = (int)word[5];
        } else {
            raw[3] = 0;
            raw[4] = 0;
            raw[5] = 0;
        }
        
        return true;
        
    }
    
    raw[0] = 0;
    raw[1] = 0;
    raw[2] = 0;
    raw[3] = 0;
    raw[4] = 0;
    raw[5] = 0;
    
    return false;
}

void lsm330dlc_raw2value(void *h, int *src, double *dst)
{
    struct lsm330dlc *l = (struct lsm330dlc *)h;
    
    dst[0] = ((double)src[0]*4096/65535)*l->accFactor/1000.0;
    dst[1] = ((double)src[1]*4096/65535)*l->accFactor/1000.0;
    dst[2] = ((double)src[2]*4096/65535)*l->accFactor/1000.0;
    dst[3] = (double)src[3]*l->gyroFactor/1000.0;
    dst[4] = (double)src[4]*l->gyroFactor/1000.0;
    dst[5] = (double)src[5]*l->gyroFactor/1000.0;
}

bool lsm330dlc_setAccelDataRate(void *h, int rate)
{
    struct lsm330dlc *l = (struct lsm330dlc *)h;
    unsigned char buf[2];

    switch (rate) {
    case LSM330DLC_A_DATARATE_1HZ:
    case LSM330DLC_A_DATARATE_10HZ:
    case LSM330DLC_A_DATARATE_25HZ:
    case LSM330DLC_A_DATARATE_50HZ:
    case LSM330DLC_A_DATARATE_100HZ:
    case LSM330DLC_A_DATARATE_200HZ:
    case LSM330DLC_A_DATARATE_400HZ:
        break;
    default:
        return false;
    }
    
    buf[0] = 0x20;
    buf[1] = rate | 0x07;
    Wire.beginTransmission(l->accAddr);
    Wire.write(buf, 2);
    if (Wire.endTransmission()) {
        return false;
    }
    
    return true;
}

bool lsm330dlc_setFullScaleAccelRange(void *h, int scale)
{
    struct lsm330dlc *l = (struct lsm330dlc *)h;
    unsigned char buf[2];
    double factor;

    switch (scale) {
    case LSM330DLC_A_FULLSCALE_PM2G:  factor = 1.0;  break;
    case LSM330DLC_A_FULLSCALE_PM4G:  factor = 2.0;  break;
    case LSM330DLC_A_FULLSCALE_PM8G:  factor = 4.0;  break;
    case LSM330DLC_A_FULLSCALE_PM16G: factor = 12.0; break;
    default:
        return false;
    }
    
    buf[0] = 0x23;
    buf[1] = scale << 4;
    Wire.beginTransmission(l->accAddr);
    Wire.write(buf, 2);
    if (Wire.endTransmission()) {
        return false;
    }

    l->accFactor = factor;
    
    return true;
}

bool lsm330dlc_enableGyro(void *h)
{
    struct lsm330dlc *l = (struct lsm330dlc *)h;
    unsigned char buf[2];

    buf[0] = 0x20;
    buf[1] = 0x0F;
    Wire.beginTransmission(l->gyroAddr);
    Wire.write(buf, 2);
    if (Wire.endTransmission()) {
        return false;
    }
    
    return true;
}

bool lsm330dlc_setFullScaleGyroRange(void *h, int scale)
{
    struct lsm330dlc *l = (struct lsm330dlc *)h;
    unsigned char buf[2];
    double factor;

    switch (scale) {
    case LSM330DLC_G_FULLSCALE_250DPS: factor = 8.75;  break;
    case LSM330DLC_G_FULLSCALE_500DPS: factor = 17.5;  break;
    case LSM330DLC_G_FULLSCALE_2KDPS1: factor = 70.0;  break;
    case LSM330DLC_G_FULLSCALE_2KDPS2: factor = 70.0; break;
    default:
        return false;
    }
  
    buf[0] = 0x23;
    buf[1] = scale << 4;
    Wire.beginTransmission(l->gyroAddr);
    Wire.write(buf, 2);
    if (Wire.endTransmission()) {
        return false;
    }
    
    l->gyroFactor = factor;
    
    return true;
}
