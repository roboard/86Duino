/*
FreeIMU.cpp - A libre and easy to use orientation sensing library for Arduino
Copyright (C) 2011-2012 Fabio Varesano <fabio at varesano dot net>

Development of this code has been supported by the Department of Computer Science,
Universita' degli Studi di Torino, Italy within the Piemonte Project
http://www.piemonte.di.unito.it/


This program is free software: you can redistribute it and/or modify
it under the terms of the version 3 GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Modified 15 January 2014 by DY Hung

*/

#include <inttypes.h>
#include <stdint.h>
#include <math.h>
#include "FreeIMU1.h"

#include "i2c.h"

FreeIMU1::FreeIMU1() {
  #if HAS_LSM330DLC()
    accgyro = LSM330DLC(); 
  #endif
  
  // initialize quaternion
  q0 = 1.0f;
  q1 = 0.0f;
  q2 = 0.0f;
  q3 = 0.0f;
  twoKp = twoKpDef;
  twoKi = twoKiDef;
  integralFBx = 0.0f,  integralFBy = 0.0f, integralFBz = 0.0f;
  lastUpdate = 0;
  now = 0;
  
  #ifndef CALIBRATION_H
  // initialize scale factors to neutral values
  acc_scale_x = 1;
  acc_scale_y = 1;
  acc_scale_z = 1;
  magn_scale_x = 1;
  magn_scale_y = 1;
  magn_scale_z = 1;
  #else
  // get values from global variables of same name defined in calibration.h
  acc_off_x = ::acc_off_x;
  acc_off_y = ::acc_off_y;
  acc_off_z = ::acc_off_z;
  acc_scale_x = ::acc_scale_x;
  acc_scale_y = ::acc_scale_y;
  acc_scale_z = ::acc_scale_z;
  magn_off_x = ::magn_off_x;
  magn_off_y = ::magn_off_y;
  magn_off_z = ::magn_off_z;
  magn_scale_x = ::magn_scale_x;
  magn_scale_y = ::magn_scale_y;
  magn_scale_z = ::magn_scale_z;
  #endif
}

void FreeIMU1::init() {
  #if HAS_LSM330DLC()
    init(FIMU1_ACC_ADDR, FIMU1_GYRO_ADDR, false);
  #endif
}

void FreeIMU1::init(bool fastmode) {
  #if HAS_LSM330DLC()
    init(FIMU1_ACC_ADDR, FIMU1_GYRO_ADDR, fastmode);
  #endif
}


/**
 * Initialize the FreeIMU1 I2C bus, sensors and performs gyro offsets calibration
*/
#if HAS_LSM330DLC()
void FreeIMU1::init(int acc_addr, int gyro_addr, bool fastmode) {
#else
void FreeIMU1::init(int accgyro_addr, bool fastmode) {
#endif
  delay(5);
  
  if(fastmode) {
    #if defined(__86DUINO_ZERO) || defined(__86DUINO_ONE) || defined(__86DUINO_EDUCAKE)
      i2c_SetSpeed(0, I2CMODE_AUTO, 400000L);
    #endif
  }
  
  #if HAS_LSM330DLC()
    accgyro.init(acc_addr, gyro_addr);
  #endif

  // zero gyro
  zeroGyro();
  
  #ifndef CALIBRATION_H
  // load calibration from eeprom
  calLoad();
  #endif
}

#ifndef CALIBRATION_H

static uint8_t location; // assuming ordered reads

void eeprom_read_var(uint8_t size, byte * var) {
  for(uint8_t i = 0; i<size; i++) {
    var[i] = EEPROM.read(location + i);
  }
  location += size;
}

void FreeIMU1::calLoad() {
  if(EEPROM.read(FREEIMU_EEPROM_BASE) == FREEIMU_EEPROM_SIGNATURE) { // check if signature is ok so we have good data
    location = FREEIMU_EEPROM_BASE + 1; // reset location
    
    eeprom_read_var(sizeof(acc_off_x), (byte *) &acc_off_x);
    eeprom_read_var(sizeof(acc_off_y), (byte *) &acc_off_y);
    eeprom_read_var(sizeof(acc_off_z), (byte *) &acc_off_z);
    
    eeprom_read_var(sizeof(magn_off_x), (byte *) &magn_off_x);
    eeprom_read_var(sizeof(magn_off_y), (byte *) &magn_off_y);
    eeprom_read_var(sizeof(magn_off_z), (byte *) &magn_off_z);
    
    eeprom_read_var(sizeof(acc_scale_x), (byte *) &acc_scale_x);
    eeprom_read_var(sizeof(acc_scale_y), (byte *) &acc_scale_y);
    eeprom_read_var(sizeof(acc_scale_z), (byte *) &acc_scale_z);
    
    eeprom_read_var(sizeof(magn_scale_x), (byte *) &magn_scale_x);
    eeprom_read_var(sizeof(magn_scale_y), (byte *) &magn_scale_y);
    eeprom_read_var(sizeof(magn_scale_z), (byte *) &magn_scale_z);
  }
  else {
    acc_off_x = 0;
    acc_off_y = 0;
    acc_off_z = 0;
    acc_scale_x = 1;
    acc_scale_y = 1;
    acc_scale_z = 1;

    magn_off_x = 0;
    magn_off_y = 0;
    magn_off_z = 0;
    magn_scale_x = 1;
    magn_scale_y = 1;
    magn_scale_z = 1;
  }
}
#endif

/**
 * Populates raw_values with the raw_values from the sensors
*/
void FreeIMU1::getRawValues(int * raw_values) {
  #if HAS_LSM330DLC()
    accgyro.getMotion6(&raw_values[0], &raw_values[1], &raw_values[2], &raw_values[3], &raw_values[4], &raw_values[5]);
  #endif
}


/**
 * Populates values with calibrated readings from the sensors
*/
void FreeIMU1::getValues(double * values) {  
  #if HAS_LSM330DLC()
    int accgyroval[6];
    accgyro.getMotion6(&accgyroval[0], &accgyroval[1], &accgyroval[2], &accgyroval[3], &accgyroval[4], &accgyroval[5]);
    
    // remove offsets from the gyroscope
    accgyroval[3] = accgyroval[3] - gyro_off_x;
    accgyroval[4] = accgyroval[4] - gyro_off_y;
    accgyroval[5] = accgyroval[5] - gyro_off_z;
	
    values[0] = ((double)accgyroval[0]*4096/65535)*9.80665*accgyro.accFactor/1000.0;
    values[1] = ((double)accgyroval[1]*4096/65535)*9.80665*accgyro.accFactor/1000.0;
    values[2] = ((double)accgyroval[2]*4096/65535)*9.80665*accgyro.accFactor/1000.0;
    values[3] = (double)accgyroval[3]*accgyro.gyroFactor/1000.0;
    values[4] = (double)accgyroval[4]*accgyro.gyroFactor/1000.0;
    values[5] = (double)accgyroval[5]*accgyro.gyroFactor/1000.0;
  #endif
  
  #warning Accelerometer calibration active: have you calibrated your device?
  // remove offsets and scale accelerometer (calibration)
  values[0] = (values[0] - acc_off_x) / acc_scale_x;
  values[1] = (values[1] - acc_off_y) / acc_scale_y;
  values[2] = (values[2] - acc_off_z) / acc_scale_z;
  
  #if 0
    magn.getValues(&values[6]);
    // calibration 
    #warning Magnetometer calibration active: have you calibrated your device?
    values[6] = (values[6] - magn_off_x) / magn_scale_x;
    values[7] = (values[7] - magn_off_y) / magn_scale_y;
    values[8] = (values[8] - magn_off_z) / magn_scale_z;
  #endif
}

void FreeIMU1::getValues(float * values) {
  int i;
  double dValues[9];
  getValues(dValues);
  
  for (i = 0; i < 9; i++)
    values[i] = (float)dValues[i];
}

/**
 * Computes gyro offsets
*/
void FreeIMU1::zeroGyro() {
  const int totSamples = 20;
  int raw[11];
  double tmpOffsets[] = {0,0,0};
  
  for (int i = 0; i < totSamples; i++){
    getRawValues(raw);
    tmpOffsets[0] += raw[3];
    tmpOffsets[1] += raw[4];
    tmpOffsets[2] += raw[5];
  }
  
  gyro_off_x = tmpOffsets[0] / totSamples;
  gyro_off_y = tmpOffsets[1] / totSamples;
  gyro_off_z = tmpOffsets[2] / totSamples;
}


/**
 * Quaternion implementation of the 'DCM filter' [Mayhony et al].  Incorporates the magnetic distortion
 * compensation algorithms from Sebastian Madgwick's filter which eliminates the need for a reference
 * direction of flux (bx bz) to be predefined and limits the effect of magnetic distortions to yaw
 * axis only.
 * 
 * @see: http://www.x-io.co.uk/node/8#open_source_ahrs_and_imu_algorithms
*/
#if IS_9DOM()
void  FreeIMU1::AHRSupdate(double gx, double gy, double gz, double ax, double ay, double az, double mx, double my, float mz) {
#elif IS_6DOM()
void  FreeIMU1::AHRSupdate(double gx, double gy, double gz, double ax, double ay, double az) {
#endif
  double recipNorm;
  double q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;
  double halfex = 0.0f, halfey = 0.0f, halfez = 0.0f;
  double qa, qb, qc;

  // Auxiliary variables to avoid repeated arithmetic
  q0q0 = q0 * q0;
  q0q1 = q0 * q1;
  q0q2 = q0 * q2;
  q0q3 = q0 * q3;
  q1q1 = q1 * q1;
  q1q2 = q1 * q2;
  q1q3 = q1 * q3;
  q2q2 = q2 * q2;
  q2q3 = q2 * q3;
  q3q3 = q3 * q3;
  
  #if IS_9DOM()
  // Use magnetometer measurement only when valid (avoids NaN in magnetometer normalisation)
  if((mx != 0.0f) && (my != 0.0f) && (mz != 0.0f)) {
    double hx, hy, bx, bz;
    double halfwx, halfwy, halfwz;
    
    // Normalise magnetometer measurement
    recipNorm = 1.0/sqrt(mx * mx + my * my + mz * mz);
    mx *= recipNorm;
    my *= recipNorm;
    mz *= recipNorm;
    
    // Reference direction of Earth's magnetic field
    hx = 2.0f * (mx * (0.5f - q2q2 - q3q3) + my * (q1q2 - q0q3) + mz * (q1q3 + q0q2));
    hy = 2.0f * (mx * (q1q2 + q0q3) + my * (0.5f - q1q1 - q3q3) + mz * (q2q3 - q0q1));
    bx = sqrt(hx * hx + hy * hy);
    bz = 2.0f * (mx * (q1q3 - q0q2) + my * (q2q3 + q0q1) + mz * (0.5f - q1q1 - q2q2));
    
    // Estimated direction of magnetic field
    halfwx = bx * (0.5f - q2q2 - q3q3) + bz * (q1q3 - q0q2);
    halfwy = bx * (q1q2 - q0q3) + bz * (q0q1 + q2q3);
    halfwz = bx * (q0q2 + q1q3) + bz * (0.5f - q1q1 - q2q2);
    
    // Error is sum of cross product between estimated direction and measured direction of field vectors
    halfex = (my * halfwz - mz * halfwy);
    halfey = (mz * halfwx - mx * halfwz);
    halfez = (mx * halfwy - my * halfwx);
  }
  #endif

  // Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
  if((ax != 0.0f) && (ay != 0.0f) && (az != 0.0f)) {
    double halfvx, halfvy, halfvz;
    
    // Normalise accelerometer measurement
    recipNorm = 1.0/sqrt(ax * ax + ay * ay + az * az);
    ax *= recipNorm;
    ay *= recipNorm;
    az *= recipNorm;
    
    // Estimated direction of gravity
    halfvx = q1q3 - q0q2;
    halfvy = q0q1 + q2q3;
    halfvz = q0q0 - 0.5f + q3q3;
  
    // Error is sum of cross product between estimated direction and measured direction of field vectors
    halfex += (ay * halfvz - az * halfvy);
    halfey += (az * halfvx - ax * halfvz);
    halfez += (ax * halfvy - ay * halfvx);
  }

  // Apply feedback only when valid data has been gathered from the accelerometer or magnetometer
  if(halfex != 0.0f && halfey != 0.0f && halfez != 0.0f) {
    // Compute and apply integral feedback if enabled
    if(twoKi > 0.0f) {
      integralFBx += twoKi * halfex * (1.0f / sampleFreq);  // integral error scaled by Ki
      integralFBy += twoKi * halfey * (1.0f / sampleFreq);
      integralFBz += twoKi * halfez * (1.0f / sampleFreq);
      gx += integralFBx;  // apply integral feedback
      gy += integralFBy;
      gz += integralFBz;
    }
    else {
      integralFBx = 0.0f; // prevent integral windup
      integralFBy = 0.0f;
      integralFBz = 0.0f;
    }

    // Apply proportional feedback
    gx += twoKp * halfex;
    gy += twoKp * halfey;
    gz += twoKp * halfez;
  }
  
  // Integrate rate of change of quaternion
  gx *= (0.5f * (1.0f / sampleFreq));   // pre-multiply common factors
  gy *= (0.5f * (1.0f / sampleFreq));
  gz *= (0.5f * (1.0f / sampleFreq));
  qa = q0;
  qb = q1;
  qc = q2;
  q0 += (-qb * gx - qc * gy - q3 * gz);
  q1 += (qa * gx + qc * gz - q3 * gy);
  q2 += (qa * gy - qb * gz + q3 * gx);
  q3 += (qa * gz + qb * gy - qc * gx);
  
  // Normalise quaternion
  recipNorm = 1.0/sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
  q0 *= recipNorm;
  q1 *= recipNorm;
  q2 *= recipNorm;
  q3 *= recipNorm;
}


/**
 * Populates array q with a quaternion representing the IMU orientation with respect to the Earth
 * 
 * @param q the quaternion to populate
*/
void FreeIMU1::getQ(double * q) {
  double val[6];
  getValues(val);
  
  now = micros();
  sampleFreq = 1.0 / ((now - lastUpdate) / 1000000.0);
  lastUpdate = now;
  // gyro values are expressed in deg/sec, the * M_PI/180 will convert it to radians/sec
  #if IS_6DOM()
    AHRSupdate(val[3] * M_PI/180, val[4] * M_PI/180, val[5] * M_PI/180, val[0], val[1], val[2]);
  #endif
  
  q[0] = q0;
  q[1] = q1;
  q[2] = q2;
  q[3] = q3;
}

void FreeIMU1::getQ(float * q) {
  int i;
  double dQ[4];
  getQ(dQ);
  
  for (i = 0; i < 4; i++)
    q[i] = (float)dQ[i];
}

/**
 * Compensates the accelerometer readings in the 3D vector acc expressed in the sensor frame for gravity
 * @param acc the accelerometer readings to compensate for gravity
 * @param q the quaternion orientation of the sensor board with respect to the world
*/
void FreeIMU1::gravityCompensateAcc(double * acc, double * q) {
  double g[3];
  
  // get expected direction of gravity in the sensor frame
  g[0] = 2 * (q[1] * q[3] - q[0] * q[2]);
  g[1] = 2 * (q[0] * q[1] + q[2] * q[3]);
  g[2] = q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3];
  
  // compensate accelerometer readings with the expected direction of gravity
  acc[0] = acc[0] - g[0];
  acc[1] = acc[1] - g[1];
  acc[2] = acc[2] - g[2];
}

void FreeIMU1::gravityCompensateAcc(float * acc, float * q) {
  int i;
  double dAcc[3], dQ[4];
  
  for (i = 0; i < 3; i++)
    dAcc[i] = (double)acc[i];
  for (i = 0; i < 4; i++)
    dQ[i] = (double)q[i];
  
  gravityCompensateAcc(dAcc, dQ);
  
  for (i = 0; i < 3; i++)
    acc[i] = (float)dAcc[i];
  for (i = 0; i < 4; i++)
    q[i] = (float)dQ[i];
}

/**
 * Returns the Euler angles in radians defined in the Aerospace sequence.
 * See Sebastian O.H. Madwick report "An efficient orientation filter for 
 * inertial and intertial/magnetic sensor arrays" Chapter 2 Quaternion representation
 * 
 * @param angles three doubles array which will be populated by the Euler angles in radians
*/
void FreeIMU1::getEulerRad(double * angles) {
  double q[4]; // quaternion
  getQ(q);
  angles[0] = atan2(2 * q[1] * q[2] - 2 * q[0] * q[3], 2 * q[0]*q[0] + 2 * q[1] * q[1] - 1); // psi
  angles[1] = -asin(2 * q[1] * q[3] + 2 * q[0] * q[2]); // theta
  angles[2] = atan2(2 * q[2] * q[3] - 2 * q[0] * q[1], 2 * q[0] * q[0] + 2 * q[3] * q[3] - 1); // phi
}

void FreeIMU1::getEulerRad(float * angles) {
  int i;
  double dAngles[3];
  
  getEulerRad(dAngles);
  for (i = 0; i < 3; i++)
    angles[i] = (float)dAngles[i];
}

/**
 * Returns the Euler angles in degrees defined with the Aerospace sequence.
 * See Sebastian O.H. Madwick report "An efficient orientation filter for 
 * inertial and intertial/magnetic sensor arrays" Chapter 2 Quaternion representation
 * 
 * @param angles three doubles array which will be populated by the Euler angles in degrees
*/
void FreeIMU1::getEuler(double * angles) {
  getEulerRad(angles);
  arr3_rad_to_deg(angles);
}

void FreeIMU1::getEuler(float * angles) {
  int i;
  double dAngles[3];
  
  getEuler(dAngles);
  for (i = 0; i < 3; i++)
    angles[i] = (float)dAngles[i];
}

/**
 * Returns the yaw pitch and roll angles, respectively defined as the angles in radians between
 * the Earth North and the IMU X axis (yaw), the Earth ground plane and the IMU X axis (pitch)
 * and the Earth ground plane and the IMU Y axis.
 * 
 * @note This is not an Euler representation: the rotations aren't consecutive rotations but only
 * angles from Earth and the IMU. For Euler representation Yaw, Pitch and Roll see FreeIMU1::getEuler
 * 
 * @param ypr three doubles array which will be populated by Yaw, Pitch and Roll angles in radians
*/
void FreeIMU1::getYawPitchRollRad(double * ypr) {
  double q[4]; // quaternion
  double gx, gy, gz; // estimated gravity direction
  getQ(q);
  
  gx = 2 * (q[1]*q[3] - q[0]*q[2]);
  gy = 2 * (q[0]*q[1] + q[2]*q[3]);
  gz = q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3];
  
  ypr[0] = atan2(2 * q[1] * q[2] - 2 * q[0] * q[3], 2 * q[0]*q[0] + 2 * q[1] * q[1] - 1);
  ypr[1] = atan(gx / sqrt(gy*gy + gz*gz));
  ypr[2] = atan(gy / sqrt(gx*gx + gz*gz));
}

void FreeIMU1::getYawPitchRollRad(float * ypr) {
  int i;
  double dYPR[3];
  
  getYawPitchRollRad(dYPR);
  for (i = 0; i < 3; i++)
    ypr[i] = (float)dYPR[i];
}

/**
 * Returns the yaw pitch and roll angles, respectively defined as the angles in degrees between
 * the Earth North and the IMU X axis (yaw), the Earth ground plane and the IMU X axis (pitch)
 * and the Earth ground plane and the IMU Y axis.
 * 
 * @note This is not an Euler representation: the rotations aren't consecutive rotations but only
 * angles from Earth and the IMU. For Euler representation Yaw, Pitch and Roll see FreeIMU1::getEuler
 * 
 * @param ypr three doubles array which will be populated by Yaw, Pitch and Roll angles in degrees
*/
void FreeIMU1::getYawPitchRoll(double * ypr) {
  getYawPitchRollRad(ypr);
  arr3_rad_to_deg(ypr);
}

void FreeIMU1::getYawPitchRoll(float * ypr) {
  int i;
  double dYPR[3];
  
  getYawPitchRoll(dYPR);
  for (i = 0; i < 3; i++)
    ypr[i] = (float)dYPR[i];
}

/**
 * Converts a 3 elements array arr of angles expressed in radians into degrees
*/
void arr3_rad_to_deg(double * arr) {
  arr[0] *= 180/M_PI;
  arr[1] *= 180/M_PI;
  arr[2] *= 180/M_PI;
}
