/*
FreeIMU.h - A libre and easy to use orientation sensing library for Arduino
Copyright (C) 2011 Fabio Varesano <fabio at varesano dot net>

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

#ifndef FreeIMU1_H
#define FreeIMU1_H

#include "Arduino.h"
#include "calibra.h"

#ifndef CALIBRATION_H
#include <EEPROM.h>
#endif

#define FREEIMU_EEPROM_BASE 0x0A
#define FREEIMU_EEPROM_SIGNATURE 0x19

#ifdef __86DUINO_ONE
	#define HAS_LSM330DLC()  (1)
	#define FIMU1_ACC_ADDR   (0x30 >> 1)
	#define FIMU1_GYRO_ADDR  (0xD4 >> 1)
	
	#include "utility/LSM330.h"
	
	#define IS_6DOM() (1)
	#define IS_9DOM() (!IS_6DOM())
#else
	#error FreeIMU1 library is only used on 86Duino ONE!
#endif

#define twoKpDef  (2.0f * 0.5f) // 2 * proportional gain
#define twoKiDef  (2.0f * 0.1f) // 2 * integral gain

class FreeIMU1
{
  public:
    FreeIMU1();
    void init();
    void init(bool fastmode);
	#if HAS_LSM330DLC()
    void init(int acc_addr, int gyro_addr, bool fastmode);
    #else
    void init(int accgyro_addr, bool fastmode);
    #endif
    #ifndef CALIBRATION_H
    void calLoad();
    #endif
    void zeroGyro();
	void getRawValues(int * raw_values);
	
    void getValues(float * values);
    void getValues(double * values);
    void getQ(float * q);
    void getQ(double * q);
    void getEuler(float * angles);
    void getEuler(double * angles);
    void getYawPitchRoll(float * ypr);
    void getYawPitchRoll(double * ypr);
    void getEulerRad(float * angles);
    void getEulerRad(double * angles);
    void getYawPitchRollRad(float * ypr);
    void getYawPitchRollRad(double * ypr);
	void gravityCompensateAcc(float * acc, float * q);
    void gravityCompensateAcc(double * acc, double * q);
	
    int* raw_acc, raw_gyro, raw_magn;
    // calibration parameters
    int16_t gyro_off_x, gyro_off_y, gyro_off_z;
    int16_t acc_off_x, acc_off_y, acc_off_z, magn_off_x, magn_off_y, magn_off_z;
    float acc_scale_x, acc_scale_y, acc_scale_z, magn_scale_x, magn_scale_y, magn_scale_z;
	
	#if HAS_LSM330DLC()
	  LSM330DLC accgyro;
	#endif
    
  private:
    void AHRSupdate(double gx, double gy, double gz, double ax, double ay, double az);
    
    volatile double twoKp;      // 2 * proportional gain (Kp)
    volatile double twoKi;      // 2 * integral gain (Ki)
    volatile double q0, q1, q2, q3; // quaternion of sensor frame relative to auxiliary frame
    volatile double integralFBx,  integralFBy, integralFBz;
    unsigned long lastUpdate, now; // sample period expressed in milliseconds
    double sampleFreq; // half the sample period expressed in seconds
};

void arr3_rad_to_deg(double * arr);


#endif

