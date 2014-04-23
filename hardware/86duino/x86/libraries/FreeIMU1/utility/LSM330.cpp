/*
  LSM330.cpp - Part of DM&P Vortex86 FreeIMU1 library
  Copyright (c) 2013 DY Huang <Dyhung@dmp.com.tw>. All right reserved.

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

#define __LSM330DLC_LIB

#include "LSM330.h"

LSM330DLC::LSM330DLC() {
  accAddr = -1;
  gyroAddr = -1;
  
  accFactor = 0;
  gyroFactor = 0;
}

void LSM330DLC::init(int acc_addr, int gyro_addr) {
  accAddr = acc_addr;
  gyroAddr = gyro_addr;
  
  setAccelDataRate(LSM330DLC_A_DATARATE_25HZ);
  setFullScaleAccelRange(LSM330DLC_A_FULLSCALE_PM2G);
  
  enableGyro();
  setFullScaleGyroRange(LSM330DLC_G_FULLSCALE_250DPS);
}

void LSM330DLC::getMotion6(int* ax, int* ay, int* az, int* gx, int* gy, int* gz) {
  uint8_t temp[12];
  uint8_t error = 0;
  if (getRegVal( accAddr, 0xA8, &temp[0]) != 6) error++;
  if (getRegVal(gyroAddr, 0xA8, &temp[6]) != 6) error++;
  
  if (error == 0) {
    ax[0] = (int16_t)temp[0]  | ((int16_t)temp[1]  << 8);
    ay[0] = (int16_t)temp[2]  | ((int16_t)temp[3]  << 8);
    az[0] = (int16_t)temp[4]  | ((int16_t)temp[5]  << 8);
    gx[0] = (int16_t)temp[6]  | ((int16_t)temp[7]  << 8);
    gy[0] = (int16_t)temp[8]  | ((int16_t)temp[9]  << 8);
    gz[0] = (int16_t)temp[10] | ((int16_t)temp[11] << 8);
	ax[0] = -(int16_t)ax[0];
	ay[0] = (int16_t)ay[0];
	az[0] = -(int16_t)az[0];
	gx[0] = -(int16_t)gx[0];
	gy[0] = (int16_t)gy[0];
	gz[0] = -(int16_t)gz[0];
  }
  else {
    ax[0] = 0;
    ay[0] = 0;
    az[0] = 0;
    gx[0] = 0;
    gy[0] = 0;
    gz[0] = 0;
  }
}

int8_t LSM330DLC::getRegVal(uint8_t addr, uint8_t offset, uint8_t *pVal) {
  int i = 0;
  uint32_t pretime;
  
  Wire.beginTransmission(addr);
  Wire.write(offset);
  Wire.endTransmission();
  
  Wire.requestFrom((int)addr, 6);
  pretime = millis();
  while ((millis() - pretime) < 1000UL) {
    if (Wire.available()) {
	  pVal[i++] = (uint8_t)Wire.read();
	  if (i == 6) break;
	  pretime = millis();
	}
  }
  
  return i;
}

void LSM330DLC::setAccelDataRate(uint8_t rate) {
  uint8_t buf[2];
  
  buf[0] = 0x20;
  buf[1] = rate | 0x07;
  Wire.beginTransmission(accAddr);
  Wire.write(buf, 2);
  Wire.endTransmission();
}

void LSM330DLC::setFullScaleAccelRange(uint8_t scale) {
  uint8_t buf[2];
  
  buf[0] = 0x23;
  buf[1] = scale << 4;
  Wire.beginTransmission(accAddr);
  Wire.write(buf, 2);
  Wire.endTransmission();
  
  switch (scale) {
    case LSM330DLC_A_FULLSCALE_PM2G:
	  accFactor = 1;
	  break;
	case LSM330DLC_A_FULLSCALE_PM4G:
	  accFactor = 2;
	  break;
	case LSM330DLC_A_FULLSCALE_PM8G:
	  accFactor = 4;
	  break;
	case LSM330DLC_A_FULLSCALE_PM16G:
	  accFactor = 12;
	  break;
  }
}

void LSM330DLC::enableGyro(void) {
  uint8_t buf[2];
  
  buf[0] = 0x20;
  buf[1] = 0x0F;
  Wire.beginTransmission(gyroAddr);
  Wire.write(buf, 2);
  Wire.endTransmission();
}

void LSM330DLC::setFullScaleGyroRange(uint8_t scale) {
  uint8_t buf[2];
  
  buf[0] = 0x23;
  buf[1] = scale << 4;
  Wire.beginTransmission(gyroAddr);
  Wire.write(buf, 2);
  Wire.endTransmission();
  
  switch (scale) {
    case LSM330DLC_G_FULLSCALE_250DPS:
	  gyroFactor = 8.75;
	  break;
	case LSM330DLC_G_FULLSCALE_500DPS:
	  gyroFactor = 17.5;
	  break;
	case LSM330DLC_G_FULLSCALE_2KDPS1:
	  gyroFactor = 70.0;
	  break;
	case LSM330DLC_G_FULLSCALE_2KDPS2:
	  gyroFactor = 70.0;
	  break;
  }
}
