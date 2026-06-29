/*
  RMG146.cpp - Part of DM&P Vortex86 FreeIMU1 library
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
#include "rmg146.h"

#include "Arduino.h"
#include "Wire.h"

static int getRegVal(unsigned char addr, unsigned char offset, unsigned char *pVal) {
    int i = 0;
    
    uint32_t pretime;
    while (i < 6) {
        Wire.beginTransmission(addr);
        Wire.write(offset+i);
        Wire.endTransmission();

        Wire.requestFrom((int)addr, 1);
        if (Wire.available()) {
            pVal[i++] = (unsigned char)Wire.read();
        } else {
            break;
        }
    }
  
    return i;
}

struct rmg146 {
    
    int accAddr;
    int gyroAddr;
    int magnAddr;
    double accFactor;
    double gyroFactor;
    double magnXYFactor;
    double magnZFactor;
    
};

void *rmg146_init(int acc_addr, int gyro_addr, int magn_addr)
{
    struct rmg146 *l;
    void *h;
    
    if (acc_addr == 0xFF && gyro_addr == 0xFF && magn_addr == 0xFF) {
        return NULL;
    }
    
    h = (void *)malloc(sizeof(struct rmg146));
    if (h == NULL) {
        return NULL;
    }
    l = (struct rmg146 *)h;
    
    l->accAddr = acc_addr;
    l->gyroAddr = gyro_addr;
    l->magnAddr = magn_addr;
    l->accFactor = 1.0;
    l->gyroFactor = 131.0;
    l->magnXYFactor = 1055;
    l->magnZFactor = 950;
    
    if (l->accAddr != 0xFF && rmg146_setAccelDataRate(h, RMG146_A_DATARATE_1KHZ) == false) {
        free(h);
        return NULL;
    }
    
    if (l->accAddr != 0xFF && rmg146_setFullScaleAccelRange(h, RMG146_A_FULLSCALE_PM8G) == false) {
        free(h);
        return NULL;
    }
    
    if (l->gyroAddr != 0xFF && rmg146_setFullScaleGyroRange(h, RMG146_G_FULLSCALE_2KDPS) == false) {
        free(h);
        return NULL;
    }
    
    if (l->magnAddr != 0xFF && rmg146_setFullScaleMagnRange(h, RMG146_M_FULLSCALE_2P5_GAUSS) == false) {
        free(h);
        return NULL;
    }
    
    return h;
}

void rmg146_close(void *h)
{
    if (h) free(h);
}

bool rmg146_getraw(void *h, int *raw)
{
    struct rmg146 *l = (struct rmg146 *)h;
    unsigned char temp[18];
    short word[9];
    int error = 0;
    
    if (l->accAddr != 0xFF && getRegVal( l->accAddr, 0x28, &temp[0]) != 6) {
        error++;
    } else if (l->gyroAddr != 0xFF && getRegVal(l->gyroAddr, 0x1D, &temp[6]) != 6) {
        error++;
    } else if (l->magnAddr != 0xFF && getRegVal(l->magnAddr, 0x03, &temp[12]) != 6) {
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
            word[3] = (short)temp[7]  | ((short)temp[6]  << 8);
            word[4] = (short)temp[9]  | ((short)temp[8]  << 8);
            word[5] = (short)temp[11] | ((short)temp[10] << 8);
            raw[3] = (int)word[3];
            raw[4] = (int)word[4];
            raw[5] = (int)word[5];
        } else {
            raw[3] = 0;
            raw[4] = 0;
            raw[5] = 0;
        }
        
        if (l->magnAddr != 0xFF) {
            word[6] = (short)temp[13] | ((short)temp[12] << 8);
            word[7] = (short)temp[15] | ((short)temp[14] << 8);
            word[8] = (short)temp[17] | ((short)temp[16] << 8);
            raw[6] = (int)word[6];
            raw[7] = (int)word[7];
            raw[8] = (int)word[8];
        } else {
            raw[6] = 0;
            raw[7] = 0;
            raw[8] = 0;
        }
        
        return true;
        
    }
    
    raw[0] = 0;
    raw[1] = 0;
    raw[2] = 0;
    raw[3] = 0;
    raw[4] = 0;
    raw[5] = 0;
    raw[6] = 0;
    raw[7] = 0;
    raw[8] = 0;
    
    return false;
}

void rmg146_raw2value(void *h, int *src, double *dst)
{
    struct rmg146 *l = (struct rmg146 *)h;
    
    dst[0] = ((double)src[0]*4096/65535)*l->accFactor/1000.0;
    dst[1] = ((double)src[1]*4096/65535)*l->accFactor/1000.0;
    dst[2] = ((double)src[2]*4096/65535)*l->accFactor/1000.0;
    dst[3] = (double)src[3]/l->gyroFactor;
    dst[4] = (double)src[4]/l->gyroFactor;
    dst[5] = (double)src[5]/l->gyroFactor;
    dst[6] = (double)src[6]/l->magnXYFactor;
    dst[7] = (double)src[7]/l->magnXYFactor;
    dst[8] = (double)src[8]/l->magnZFactor;
}

bool rmg146_setAccelDataRate(void *h, int rate)
{
    struct rmg146 *l = (struct rmg146 *)h;
    unsigned char buf[2];

    switch (rate) {
    case RMG146_A_DATARATE_50HZ:
    case RMG146_A_DATARATE_100HZ:
    case RMG146_A_DATARATE_400HZ:
    case RMG146_A_DATARATE_1KHZ:
        break;
    default:
        return false;
    }
    
    buf[0] = 0x20;
    buf[1] = 0x27 | (rate << 3);
    Wire.beginTransmission(l->accAddr);
    Wire.write(buf, 2);
    if (Wire.endTransmission()) {
        return false;
    }
    
    return true;
}

bool rmg146_setFullScaleAccelRange(void *h, int scale)
{
    struct rmg146 *l = (struct rmg146 *)h;
    unsigned char buf[2];
    double factor;

    switch (scale) {
    case RMG146_A_FULLSCALE_PM2G:  factor = 1.0;  break;
    case RMG146_A_FULLSCALE_PM4G:  factor = 2.0;  break;
    case RMG146_A_FULLSCALE_PM8G:  factor = 3.9;  break;
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

bool rmg146_setFullScaleGyroRange(void *h, int scale)
{
    struct rmg146 *l = (struct rmg146 *)h;
    unsigned char buf[2];
    double factor;

    switch (scale) {
    case RMG146_G_FULLSCALE_250DPS: factor = 131.0; break;
    case RMG146_G_FULLSCALE_500DPS: factor = 65.5;  break;
    case RMG146_G_FULLSCALE_1KDPS:  factor = 32.8;  break;
    case RMG146_G_FULLSCALE_2KDPS:  factor = 16.4;  break;
    default:
        return false;
    }
  
    buf[0] = 0x16;
    buf[1] = scale << 3;
    Wire.beginTransmission(l->gyroAddr);
    Wire.write(buf, 2);
    if (Wire.endTransmission()) {
        return false;
    }
    
    l->gyroFactor = factor;
    
    return true;
}

bool rmg146_setFullScaleMagnRange(void *h, int scale)
{
    struct rmg146 *l = (struct rmg146 *)h;
    unsigned char buf[4];
    double xyfactor, zfactor;

    switch (scale) {
    case RMG146_M_FULLSCALE_1P3_GAUSS: xyfactor = 1055; zfactor = 950; break;
    case RMG146_M_FULLSCALE_1P9_GAUSS: xyfactor = 795;  zfactor = 710; break;
    case RMG146_M_FULLSCALE_2P5_GAUSS: xyfactor = 635;  zfactor = 570; break;
    case RMG146_M_FULLSCALE_4P0_GAUSS: xyfactor = 430;  zfactor = 385; break;
    case RMG146_M_FULLSCALE_4P7_GAUSS: xyfactor = 375;  zfactor = 335; break;
    case RMG146_M_FULLSCALE_5P6_GAUSS: xyfactor = 320;  zfactor = 285; break;
    case RMG146_M_FULLSCALE_8P1_GAUSS: xyfactor = 230;  zfactor = 205; break;
    default:
        return false;
    }
  
    buf[0] = 0x00;
    buf[1] = 0x18;
    buf[2] = scale << 5;
    buf[3] = 0x00;
    Wire.beginTransmission(l->magnAddr);
    Wire.write(buf, 4);
    if (Wire.endTransmission()) {
        return false;
    }
    
    l->magnXYFactor = xyfactor;
    l->magnZFactor = zfactor;
    
    return true;
}
