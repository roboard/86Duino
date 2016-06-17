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

Modified 31 May 2016 by Johnson Hung

*/

#include "Arduino.h"
#include <inttypes.h>

#include "FreeIMU1.h"

#include "AHRS.h"
#include "MadgwickAHRS.h"
#include "MARGUpdateFilter.h"
#include "utility/MovingAvarageFilter.h"
#include "utility/AP_Math_freeimu.h"

#include "i2c.h"

#include "utility/lsm330dlc.h"
#include "utility/rmg146.h"

struct _imu_sensor_ {
    void   *handler;
    int     acc;
    int     gyro;
    int     magn;
    void* (*init)       (int acc_addr, int gyro_addr, int magn_addr);
    bool  (*getraw)     (void *h, int *raw);
    void  (*raw2value)  (void *h, int *src, double *dst);
    void  (*close)      (void *h);
    int     dir[9];
};

struct _imu_sensor_ Sensors[] = {
    {NULL, LSM330DLC_ACC_ADDR, LSM330DLC_GYRO_ADDR,             0xFF, lsm330dlc_init, lsm330dlc_getraw, lsm330dlc_raw2value, lsm330dlc_close,  1, -1, -1,  1, -1, -1,  1,  1,  1},
    {NULL,    RMG146_ACC_ADDR,    RMG146_GYRO_ADDR, RMG146_MAGN_ADDR,    rmg146_init,    rmg146_getraw,    rmg146_raw2value,    rmg146_close,  1,  1,  1,  1,  1,  1,  1,  1,  1},
    {NULL,    RMG146_ACC_ADDR,    RMG146_GYRO_ADDR,             0xFF,    rmg146_init,    rmg146_getraw,    rmg146_raw2value,    rmg146_close,  1,  1,  1,  1,  1,  1,  1,  1,  1}
};

#define GRAVITY (9.80665)

//Setup Motion Detect Averages
MovingAvarageFilter accnorm_avg(5);
MovingAvarageFilter accnorm_test_avg(7);
MovingAvarageFilter accnorm_var(7);	
MovingAvarageFilter motion_detect_ma(7);

//Set-up constants for gyro calibration
uint8_t num_gyros = 1;
#define INS_MAX_INSTANCES   (2)

FreeIMU1::FreeIMU1() {
    reset();
    
    maghead = iCompass(MAG_DEC, WINDOW_SIZE, 500);
    
    MARG = DEFULT_MARG;
    
    imu = DEFAULT_IMU;
    sensor = NULL;
}

void FreeIMU1::reset() {
    // initialize quaternion
    q0 = 1.0f;
    q1 = 0.0f;
    q2 = 0.0f;
    q3 = 0.0f;
    twoKp = twoKpDef;
    twoKi = twoKiDef;
    beta = betaDef;
    integralFBx = 0.0f,  integralFBy = 0.0f, integralFBz = 0.0f;
    lastUpdate = 0;
    now = 0;


    //initialize gyro offsets
    gyro_off_x = 0.;
    gyro_off_y = 0.;  
    gyro_off_z = 0.;

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
    
    acceDataFromExternal = false;
    gyroDataFromExternal = false;
    magnDataFromExternal = false;
    
    timeSampleFirst = true;
}

void FreeIMU1::RESET() {
    RESET_Q();
}

void FreeIMU1::RESET_Q() {
    q0 = 1.0f;
    q1 = 0.0f;
    q2 = 0.0f;
    q3 = 0.0f;
    twoKp = twoKpDef;
    twoKi = twoKiDef;
    integralFBx = 0.0f,  integralFBy = 0.0f, integralFBz = 0.0f;
}

void FreeIMU1::init() {
    init(DEFULT_MARG);
}

void FreeIMU1::init(bool fastmode) {
    init(fastmode, DEFULT_MARG);
}

void FreeIMU1::init(int acc_addr, int gyro_addr, bool fastmode) {
    init(acc_addr, gyro_addr, fastmode, DEFULT_MARG);
}

void FreeIMU1::init(int type) {
    init(false, type);
}

void FreeIMU1::init(bool fastmode, int type) {
    initEX(DEFAULT_IMU, fastmode, type);
}

void FreeIMU1::init(int acc_addr, int gyro_addr, bool fastmode, int type) {
    initEX(DEFAULT_IMU, acc_addr, gyro_addr, fastmode, type);
}

int FreeIMU1::initEX(int imu) {
    return initEX(imu, DEFULT_MARG);
}

int FreeIMU1::initEX(int imu, bool fastmode) {
    return initEX(imu, false, DEFULT_MARG);
}

int FreeIMU1::initEX(int imu, int acc_addr, int gyro_addr, bool fastmode) {
    return initEX(imu, acc_addr, gyro_addr, fastmode, DEFULT_MARG);
}

int FreeIMU1::initEX(int imu, int acc_addr, int gyro_addr, int magn_addr, bool fastmode) {
    return initEX(imu, acc_addr, gyro_addr, magn_addr, fastmode, DEFULT_MARG);
}

int FreeIMU1::initEX(int imu, int type) {
    return initEX(imu, false, type);
}

int FreeIMU1::initEX(int imu, bool fastmode, int type) {
    int sensor_num = sizeof(Sensors)/sizeof(Sensors[0]);
    
    if (imu >= 0 && imu < sensor_num) {
        
        sensor = &Sensors[imu];
        return initEX(imu, sensor->acc, sensor->gyro, sensor->magn, fastmode, type);
        
    } else {
    
        sensor = NULL;
        
    }
    
    return 1;
}

int FreeIMU1::initEX(int imu, int acc_addr, int gyro_addr, bool fastmode, int type) {
    return initEX(imu, acc_addr, gyro_addr, 0xFF, fastmode, type);
}

int FreeIMU1::initEX(int imu, int acc_addr, int gyro_addr, int magn_addr, bool fastmode, int type) {
    
    int ret = 1;
    
    reset();
    
    #ifndef CALIBRATION_H
    // load calibration from eeprom
    calLoad();
    #endif
  
    RESET_Q();

    dcm = DCM();
    
    if (type == 0 || type == 1 || type == 3 || type == 4) {
        MARG = type;
    }
    
    if(fastmode) {
        i2c_SetSpeed(0, I2CMODE_AUTO, 400000L);
    }

    if (sensor) {
        
        sensor->handler = sensor->init(acc_addr, gyro_addr, magn_addr);
        if (sensor->handler) {
            
            ret = 0;
            initGyros();
            if (QuaternionInitialize() == false) {
                
                ret = 3;
                
            }
            
        } else {
            
            ret = 2;
            
        }
        
    }
    
    return ret;
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
    } else { // neutral values
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
    int i;
    
    if (sensor) {
        
        sensor->getraw(sensor->handler, raw_values);
        
    }
}

/**
 * Populates values with calibrated readings from the sensors
*/
void FreeIMU1::getValues(double * values) {
    int i;
    double values_cal[9] = {0.,0.,0.,0.,0.,0.,0.,0.,0.};

    if (acceDataFromExternal == false || gyroDataFromExternal == false) {

        if (sensor) {
            
            int accgyroval[9];
            
            getRawValues(accgyroval);
            accgyroval[3] = accgyroval[3] - gyro_off_x;
            accgyroval[4] = accgyroval[4] - gyro_off_y;
            accgyroval[5] = accgyroval[5] - gyro_off_z;
        
            for (i = 0; i < 9; i++) {
                
                accgyroval[i] = sensor->dir[i] * accgyroval[i];
                
            }
            
            sensor->raw2value(sensor->handler, accgyroval, values_cal);
        }
    }


    #warning Accelerometer calibration active: have you calibrated your device?
    // remove offsets and scale accelerometer (calibration)
    values_cal[0] = (values_cal[0] - acc_off_x) / acc_scale_x;
    values_cal[1] = (values_cal[1] - acc_off_y) / acc_scale_y;
    values_cal[2] = (values_cal[2] - acc_off_z) / acc_scale_z;

    for(i = 0; i < 9; i++) {
        values[i] = values_cal[i];
    }
    
    // acceleration data from external
    if (acceDataFromExternal == true) {
        values[0] = externAcce[0];
        values[1] = externAcce[1];
        values[2] = externAcce[2];
        acceDataFromExternal = false;
    }
    
    // gyroscope data from external
    if (gyroDataFromExternal == true) {
        values[3] = externGyro[0];
        values[4] = externGyro[1];
        values[5] = externGyro[2];
        gyroDataFromExternal = false;
    }
    
    // magnetic data from external
    if (magnDataFromExternal == true) {
        values[6] = externMagn[0];
        values[7] = externMagn[1];
        values[8] = externMagn[2];
        magnDataFromExternal = false;
    }
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
    const int totSamples = nsamples;
    int raw[11];
    double values[11]; 
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

    delay(5);
}

void FreeIMU1::initGyros() {
    //Code modified from Ardupilot library
    //
    Vector3f last_average[INS_MAX_INSTANCES], best_avg[INS_MAX_INSTANCES], gyro_offset[INS_MAX_INSTANCES];
    double best_diff[INS_MAX_INSTANCES];
    bool converged[INS_MAX_INSTANCES];

    // remove existing gyro offsets
    for (uint8_t k=0; k<num_gyros; k++) {
        gyro_offset[k] = Vector3f(0,0,0);
        best_diff[k] = 0;
        last_average[k].zero();
        converged[k] = false;
    }

    // the strategy is to average 50 points over 0.5 seconds, then do it
    // again and see if the 2nd average is within a small margin of
    // the first

    uint8_t num_converged = 0;	

    // we try to get a good calibration estimate for up to 10 seconds
    // if the gyros are stable, we should get it in 1 second
    for (int16_t j = 0; j <= 30 && num_converged < num_gyros; j++) {
        Vector3f gyro_sum[INS_MAX_INSTANCES], gyro_avg[INS_MAX_INSTANCES], gyro_diff[INS_MAX_INSTANCES];
        double diff_norm[INS_MAX_INSTANCES];

        //For FreeIMU and most boards we are using only one gyro
        //if you have more change code to match Arduimu
        zeroGyro();
        gyro_avg[0] = Vector3f(gyro_off_x, gyro_off_y,gyro_off_z) ;

        for (uint8_t k=0; k<num_gyros; k++) {
            gyro_diff[k] = last_average[k] - gyro_avg[k];
            diff_norm[k] = gyro_diff[k].length();
        }

        for (uint8_t k=0; k<num_gyros; k++) {
            if (converged[k]) continue;
            if (j == 0) {
                best_diff[k] = diff_norm[k];
                best_avg[k] = gyro_avg[k];
            } else if (gyro_diff[k].length() < ToRad(0.05f)) {
                // we want the average to be within 0.1 bit, which is 0.04 degrees/s
                last_average[k] = (gyro_avg[k] * 0.5f) + (last_average[k] * 0.5f);
                gyro_offset[k] = last_average[k];            
                converged[k] = true;
                num_converged++;
            } else if (diff_norm[k] < best_diff[k]) {
                best_diff[k] = diff_norm[k];
                best_avg[k] = (gyro_avg[k] * 0.5f) + (last_average[k] * 0.5f);
            }
            last_average[k] = gyro_avg[k];
        }
    }

    delay(5);

    if (num_converged == num_gyros) {
        // all OK
        gyro_off_x = gyro_offset[0].x;
        gyro_off_y = gyro_offset[0].y;
        gyro_off_z = gyro_offset[0].z;
        return;
    }

    // we've kept the user waiting long enough - use the best pair we
    // found so far
    for (uint8_t k=0; k<num_gyros; k++) {
        if (!converged[k]) {
            gyro_offset[k] = best_avg[k];
        }
    }

    gyro_off_x = gyro_offset[0].x;
    gyro_off_y = gyro_offset[0].y;
    gyro_off_z = gyro_offset[0].z;
}


/**
 * Populates array q with a quaternion representing the IMU orientation with respect to the Earth
 * 
 * @param q the quaternion to populate
*/
void FreeIMU1::getQ(double * q, double * val) {
    int type;
    getValues(val);

    now = micros();
    sampleFreq = 1.0 / ((now - lastUpdate) / 1000000.0);
    lastUpdate = now;
    
    val[9] = 0.0;
    if (magnDataFromExternal || sensor->magn != 0xFF) {
        val[9] = maghead.iheading(1, 0, 0, val[0], val[1], val[2], val[6], val[7], val[8]);
    }
    
    if (timeSampleFirst == true) {
        dcm.setSensorVals(val);
        dcm.DCM_init(Kp_ROLLPITCH, Ki_ROLLPITCH, Kp_YAW, Ki_YAW);
        q[0] = q0;	
        q[1] = q1;
        q[2] = q2;
        q[3] = q3;
        timeSampleFirst = false;
    } else if (MARG == 0) {
        val[0] = val[0]*GRAVITY;
        val[1] = val[1]*GRAVITY;
        val[2] = val[2]*GRAVITY;
        AHRSupdate(val[3] * M_PI/180, val[4] * M_PI/180, val[5] * M_PI/180, val[0], val[1], val[2], val[6], val[7], val[8]);
        q[0] = q0;	
        q[1] = q1;
        q[2] = q2;
        q[3] = q3;
    } else if (MARG == 1) {
        val[0] = val[0]*GRAVITY;
        val[1] = val[1]*GRAVITY;
        val[2] = val[2]*GRAVITY;
        MadgwickAHRSupdate(val[3] * M_PI/180, val[4] * M_PI/180, val[5] * M_PI/180, val[0], val[1], val[2], val[6], val[7], val[8]);
        q[0] = q0;	
        q[1] = q1;
        q[2] = q2;
        q[3] = q3;
    } else if (MARG == 3) {
        val[0] = val[0]*GRAVITY;
        val[1] = val[1]*GRAVITY;
        val[2] = val[2]*GRAVITY;
        MARGUpdateFilter(val[3] * M_PI/180, val[4] * M_PI/180, val[5] * M_PI/180, val[0], val[1], val[2], val[6], val[7], val[8]);
        q[0] = q0;	
        q[1] = q1;
        q[2] = q2;
        q[3] = q3;
    } else if (MARG == 4) {
        dcm.setSensorVals(val);
        dcm.G_Dt = 1./ sampleFreq;
        dcm.calDCM();
        dcm.getDCM2Q(q);
    }

    MotionDetect(val);
}

void FreeIMU1::getQ(float * q, float * val) {
    int i;
    double dQ[4];
    double dVal[10];
    getQ(dQ, dVal);

    for (i = 0; i < 4; i++)
        q[i] = (float)dQ[i];
    for (i = 0; i < 10; i++)
        val[i] = (float)dVal[i];
}

void FreeIMU1::getQ(double * q) {
    double val[12];
    getQ(q, val);
}

void FreeIMU1::getQ(float * q) {
    int i;
    double dQ[4];
    
    getQ(dQ);

    for (i = 0; i < 4; i++)
        q[i] = (float)dQ[i];
}

/**
 * Returns the Euler angles in radians defined in the Aerospace sequence.
 * See Sebastian O.H. Madwick report "An efficient orientation filter for 
 * inertial and intertial/magnetic sensor arrays" Chapter 2 Quaternion representation
 * 
 * @param angles three floats array which will be populated by the Euler angles in radians
*/
void FreeIMU1::getEulerRad(double * angles) {
    double q[4]; // quaternion
    double val[12];

    getQ(q, val);
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
 * @param angles three floats array which will be populated by the Euler angles in degrees
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
 * Returns the Euler angles in degrees defined with the Aerospace sequence (NED).  Conversion
 * based on MATLAB quat2angle.m for an ZXY rotation sequence. euler[0] = yaw, euler[1]=pitch and euler[2] = roll
 * Angles are lie between 0-360 degrees.  
 * 
 * @param angles three floats array which will be populated by the Euler angles in degrees
*/
void FreeIMU1::getEuler360deg(double * angles) {
    double m11, m12, m21, m31, m32;
    double gx, gy, gz; // estimated gravity direction
    double q[4]; // quaternion
    double val[12];

    getQ(q, val);

    gx = 2 * (q[1]*q[3] - q[0]*q[2]);
    gy = 2 * (q[0]*q[1] + q[2]*q[3]);
    gz = q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3];

    m11 = 2.*(q[1]*q[2] + q[0]*q[3]);
    m12 = q[0]*q[0] + q[1]*q[1] - q[2]*q[2] - q[3]*q[3];
    m21 = -2.*(q[1]*q[3] - q[0]*q[2]);               
    m31 = 2.*(q[2]*q[3] + q[0]*q[1]);              
    m32 = q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3];

    // find angles for rotations about X, Y, and Z axes
    angles[0] = -atan2( m11, m12 ) * 57.2957795;
    angles[1] = -asin( m21 ) * 57.2957795;
    angles[2] = -atan2( m31, m32 ) * 57.2957795;

    //  	Gx	gy	gz
    //0-90	"+"		"+"
    //90-180	"+"		"-"
    //180-270	"-"		"-"
    //270-360	"-"		"+"

    if(gx >= 0 && gz < 0)
        angles[1] = 180. - angles[1];
    else if(gx < 0 && gz < 0)
        angles[1] = 180. - angles[1];
    else if(gx < 0 && gz >=0)
        angles[1] = 360. + angles[1];

    if(angles[0] < 0) angles[0] = 360. + angles[0];
    if(angles[2] < 0) angles[2] = 360. + angles[2];

    angles[0] = 360 - angles[0];
  
}

void FreeIMU1::getEuler360deg(float * angles) {
    int i;
    double dAngles[3];

    getEuler360deg(dAngles);
    for (i = 0; i < 3; i++)
        angles[i] = (float)dAngles[i];
}

/**
 * Returns the Euler angles in degrees defined with the Aerospace sequence (NED).  Conversion
 * based on MATLAB quat2angle.m for an ZXY rotation sequence. euler[0] = yaw, euler[1]=pitch and euler[2] = roll
 * Angles are lie between 0-360 degrees.  Added return of q and val array.
 * 
 * @param angles three floats array which will be populated by the Euler angles in degrees
*/
void FreeIMU1::getEuler360degAttitude(double * angles, double * q, double * val) {
    double m11, m12, m21, m31, m32;
    double gx, gy, gz; // estimated gravity direction

    getQ(q, val);

    gx = 2 * (q[1]*q[3] - q[0]*q[2]);
    gy = 2 * (q[0]*q[1] + q[2]*q[3]);
    gz = q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3];

    m11 = 2.*(q[1]*q[2] + q[0]*q[3]);
    m12 = q[0]*q[0] + q[1]*q[1] - q[2]*q[2] - q[3]*q[3];
    m21 = -2.*(q[1]*q[3] - q[0]*q[2]);               
    m31 = 2.*(q[2]*q[3] + q[0]*q[1]);              
    m32 = q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3];

    // find angles for rotations about X, Y, and Z axes
    angles[0] = -atan2( m11, m12 ) * 57.2957795;
    angles[1] = -asin( m21 ) * 57.2957795;
    angles[2] = -atan2( m31, m32 ) * 57.2957795;

    //  	Gx	gy	gz
    //0-90	"+"		"+"
    //90-180	"+"		"-"
    //180-270	"-"		"-"
    //270-360	"-"		"+"

    if(gx >= 0 && gz < 0)
        angles[1] = 180. - angles[1];
    else if(gx < 0 && gz < 0)
        angles[1] = 180. - angles[1];
    else if(gx < 0 && gz >=0)
        angles[1] = 360. + angles[1];

    if(angles[0] < 0) angles[0] = 360. + angles[0];
    if(angles[2] < 0) angles[2] = 360. + angles[2];

    angles[0] = 360 - angles[0];
}

void FreeIMU1::getEuler360degAttitude(float * angles, float * q, float * val) {
    int i;
    double dAngles[3];
    double dQ[4];
    double dVal[10];

    getEuler360degAttitude(dAngles, dQ, dVal);
    for (i = 0; i < 3; i++)
        angles[i] = (float)dAngles[i];
    for (i = 0; i < 4; i++)
        q[i] = (float)dQ[i];
    for (i = 0; i < 10; i++)
        val[i] = (float)dVal[i];
}

/**
 * Returns the Euler angles in degrees defined with the Aerospace sequence (NED).  Conversion
 * based on MATLAB quat2angle.m for an ZXY rotation sequence.
 * Angles are lie between 0-360 degrees in radians.
 * 
 * @param angles three floats array which will be populated by the Euler angles in degrees
*/
void FreeIMU1::getEuler360(double * angles) {
    getEulerRad(angles);
    arr3_deg_to_rad(angles);
}

void FreeIMU1::getEuler360(float * angles) {
    int i;
    double dAngles[3];

    getEuler360(dAngles);
    for (i = 0; i < 3; i++)
        angles[i] = (float)dAngles[i];
}

/**
 * Returns the yaw pitch and roll angles, respectively defined as the angles in radians between
 * the Earth North and the IMU X axis (yaw), the Earth ground plane and the IMU X axis (pitch)
 * and the Earth ground plane and the IMU Y axis.
 * 
 * @note This is not an Euler representation: the rotations aren't consecutive rotations but only
 * angles from Earth and the IMU. For Euler representation Yaw, Pitch and Roll see FreeIMU::getEuler
 * 
 * @param ypr three floats array which will be populated by Yaw, Pitch and Roll angles in radians
*/
void FreeIMU1::getYawPitchRollRad(double * ypr) {
    double q[4]; // quaternion
    double val[12];
    double gx, gy, gz; // estimated gravity direction
    getQ(q, val);

    gx = 2 * (q[1]*q[3] - q[0]*q[2]);
    gy = 2 * (q[0]*q[1] + q[2]*q[3]);
    gz = q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3];

    ypr[0] = atan2(2 * q[1] * q[2] + 2 * q[0] * q[3], 2 * q[0]*q[0] + 2 * q[1] * q[1] - 1);
    ypr[1] = -atan2(gx, sqrt(gy*gy + gz*gz));
    ypr[2] = atan2(gy, gz);
}

void FreeIMU1::getYawPitchRollRad(float * ypr) {
    int i;
    double dYPR[3];

    getYawPitchRollRad(dYPR);
    for (i = 0; i < 3; i++)
        ypr[i] = (float)dYPR[i];
}

/**
 * Returns the yaw pitch and roll angles, respectively defined as the angles in radians between
 * the Earth North and the IMU X axis (yaw), the Earth ground plane and the IMU X axis (pitch)
 * and the Earth ground plane and the IMU Y axis.
 * 
 * @note This is not an Euler representation: the rotations aren't consecutive rotations but only
 * angles from Earth and the IMU. For Euler representation Yaw, Pitch and Roll see FreeIMU::getEuler
 * 
 * @param ypr three floats array which will be populated by Yaw, Pitch and Roll angles in radians
*/
void FreeIMU1::getYawPitchRollRadAHRS(double * ypr, double * q) {
    double gx, gy, gz; // estimated gravity direction

    gx = 2 * (q[1]*q[3] - q[0]*q[2]);
    gy = 2 * (q[0]*q[1] + q[2]*q[3]);
    gz = q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3];

    ypr[0] = atan2(2 * q[1] * q[2] + 2 * q[0] * q[3], 2 * q[0]*q[0] + 2 * q[1] * q[1] - 1);
    ypr[1] = -atan2(gx, sqrt(gy*gy + gz*gz));
    ypr[2] = atan2(gy, gz);
}

void FreeIMU1::getYawPitchRollRadAHRS(float * ypr, float * q) {
    int i;
    double dYPR[3];
    double dQ[4];

    getYawPitchRollRadAHRS(dYPR, dQ);
    for (i = 0; i < 3; i++)
        ypr[i] = (float)dYPR[i];
    for (i = 0; i < 4; i++)
        q[i] = (float)dQ[i];
}

/**
 * Returns the yaw pitch and roll angles, respectively defined as the angles in degrees between
 * the Earth North and the IMU X axis (yaw), the Earth ground plane and the IMU X axis (pitch)
 * and the Earth ground plane and the IMU Y axis. 
 * 
 * Roll and Pitch angle ranges 0-360 degrees instead of 0-90 degrees
 * 
 * @note This is not an Euler representation: the rotations aren't consecutive rotations but only
 * angles from Earth and the IMU. For Euler representation Yaw, Pitch and Roll see FreeIMU::getEuler
 * 
 * @param ypr three floats array which will be populated by Yaw, Pitch and Roll angles in degrees
*/
void FreeIMU1::getYawPitchRoll180(double * ypr) {
    double q[4];				// quaternion
    double val[12];
    double gx, gy, gz;		// estimated gravity direction

    getQ(q, val);

    gx = 2 * (q[1]*q[3] - q[0]*q[2]);
    gy = 2 * (q[0]*q[1] + q[2]*q[3]);
    gz = q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3];

    //calculating yaw
    //ypr[0] = atan2(2 * q[1] * q[2] - 2 * q[0] * q[3], 2 * q[0]*q[0] + 2 * q[1] * q[1] - 1);	
    ypr[0] = val[9];
    if(ypr[0] > 180.) ypr[0] = ypr[0] - 360.;
    ypr[0] = ypr[0] * 0.0174532925;

    //calculating Pitch1
    //Serial.print(gx); Serial.print("       "); Serial.print(gz); Serial.print("       ");
    if(gx > 0 && gz > 0) {
        ypr[1] = atan(gx / sqrt(gy*gy + gz*gz));
    } else if(gx > 0 && gz <= 0) {
        ypr[1] = PI - atan(gx / sqrt(gy*gy + gz*gz));
    } else if(gx < 0 && gz < 0) {
        ypr[1] = (-PI - atan(gx / sqrt(gy*gy + gz*gz)));
    } else  {
        ypr[1] =  atan(gx / sqrt(gy*gy + gz*gz));
    }

    //Calculating Roll1
    ypr[2] = atan(gy / sqrt(gx*gx + gz*gz));
}

void FreeIMU1::getYawPitchRoll180(float * ypr) {
    int i;
    double dYPR[3];

    getYawPitchRoll180(dYPR);
    for (i = 0; i < 3; i++)
        ypr[i] = (float)dYPR[i];
}

/**
 * Returns the yaw pitch and roll angles, respectively defined as the angles in degrees between
 * the Earth North and the IMU X axis (yaw), the Earth ground plane and the IMU X axis (pitch)
 * and the Earth ground plane and the IMU Y axis.
 * 
 * @note This is not an Euler representation: the rotations aren't consecutive rotations but only
 * angles from Earth and the IMU. For Euler representation Yaw, Pitch and Roll see FreeIMU::getEuler
 * 
 * @param ypr three floats array which will be populated by Yaw, Pitch and Roll angles in degrees
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
 * Get heading from magnetometer if LSM303 not available
 * code extracted from rob42/FreeIMU-20121106_1323 Github library
 * (https://github.com/rob42/FreeIMU-20121106_1323.git)
 * which is based on 
 *
**/
double FreeIMU1::calcMagHeading(double q0, double q1, double q2, double q3, double bx, double by, double bz) {
    double Head_X, Head_Y;
    double cos_roll, sin_roll, cos_pitch, sin_pitch;
    double gx, gy, gz; // estimated gravity direction
    double ypr[3];

    gx = 2 * (q1*q3 - q0*q2);
    gy = 2 * (q0*q1 + q2*q3);
    gz = q0*q0 - q1*q1 - q2*q2 + q3*q3;

    ypr[0] = atan2(2 * q1 * q2 - 2 * q0 * q3, 2 * q0*q0 + 2 * q1 * q1 - 1);
    ypr[1] = atan(gx / sqrt(gy*gy + gz*gz));
    ypr[2] = atan(gy / sqrt(gx*gx + gz*gz)); 

    cos_roll = cos(-ypr[2]);
    sin_roll = sin(-ypr[2]);
    cos_pitch = cos(ypr[1]);
    sin_pitch = sin(ypr[1]);

    //Example calc
    //Xh = bx * cos(theta) + by * sin(phi) * sin(theta) + bz * cos(phi) * sin(theta)
    //Yh = by * cos(phi) - bz * sin(phi)
    //return wrap((atan2(-Yh, Xh) + variation))

    // Tilt compensated Magnetic field X component:
    Head_X = bx*cos_pitch + by*sin_roll*sin_pitch + bz*cos_roll*sin_pitch;
    // Tilt compensated Magnetic field Y component:
    Head_Y = by*cos_roll - bz*sin_roll;
    // Magnetic Heading
    return (atan2(-Head_Y,Head_X)*180./M_PI) + MAG_DEC;
}

void FreeIMU1::getQ_simple(double * q, double * val) {
    double yaw;
    double pitch = atan2(val[0], sqrt(val[1]*val[1]+val[2]*val[2]));
    double roll = -atan2(val[1], sqrt(val[0]*val[0]+val[2]*val[2]));

    yaw = val[9] - MAG_DEC;

    if(val[9] > 180.) {
        yaw = (yaw - 360.) * M_PI/180;
    } else {
        yaw = yaw * M_PI/180;
    }

    double rollOver2 = roll * 0.5f;
    double sinRollOver2 = (double)sin(rollOver2);
    double cosRollOver2 = (double)cos(rollOver2);
    double pitchOver2 = pitch * 0.5f;
    double sinPitchOver2 = (double)sin(pitchOver2);
    double cosPitchOver2 = (double)cos(pitchOver2);
    double yawOver2 = yaw * 0.5f;
    double sinYawOver2 = (double)sin(yawOver2);
    double cosYawOver2 = (double)cos(yawOver2);

    q1 = cosYawOver2 * cosPitchOver2 * sinRollOver2 - sinYawOver2 * sinPitchOver2 * cosRollOver2;
    q0 = cosYawOver2 * cosPitchOver2 * cosRollOver2 + sinYawOver2 * sinPitchOver2 * sinRollOver2;
    q2 = sinYawOver2 * cosPitchOver2 * cosRollOver2 - cosYawOver2 * sinPitchOver2 * sinRollOver2;
    q3 = cosYawOver2 * sinPitchOver2 * cosRollOver2 + sinYawOver2 * cosPitchOver2 * sinRollOver2;

    if (q!=NULL) {
        q[0] = q0;
        q[1] = q1;
        q[2] = q2;
        q[3] = q3;
    }
}

void FreeIMU1::getQ_simple(float * q, float * val) {
    int i;
    double dQ[4];
    double dVal[10];

    getQ_simple(dQ, dVal);
    for (i = 0; i < 4; i++)
        q[i] = (float)dQ[i];
    for (i = 0; i < 10; i++)
        val[i] = (float)dVal[i];
}

void FreeIMU1::MotionDetect(double * val) {
    double gyro[3];
    double accnorm;
    int accnorm_test, accnorm_var_test, omegax, omegay, omegaz, omega_test, motionDetect;

    gyro[0] = val[3] * M_PI/180;
    gyro[1] = val[4] * M_PI/180;
    gyro[2] = val[5] * M_PI/180;

    /*###################################################################
    #
    #   acceleration squared euclidean norm analysis
    #   
    #   some test values previously used:
    #       if((accnorm >=0.96) && (accnorm <= 0.99)){
    #										<= 0.995
    #
    ################################################################### */
    accnorm = (val[0]*val[0]+val[1]*val[1]+val[2]*val[2]);
    if((accnorm >=0.94) && (accnorm <= 1.03)){  
        accnorm_test = 0;
    } else {
        accnorm_test = 1;
    }

    /** take average of 5 to 10 points  **/
    double accnormavg = accnorm_avg.process(accnorm);
    double accnormtestavg = accnorm_test_avg.process(accnorm_test);

    /*####################################################################
    #
    #   squared norm analysis to determine suddenly changes signal
    #
    ##################################################################### */
    //accnorm_var.process(sq(accnorm-accnorm_avg.getAvg()));
    // was 0.0005
    if(accnorm_var.process(sq(accnorm-accnormavg)) < 0.0005) {
        accnorm_var_test = 0;
    } else {
        accnorm_var_test = 1;
    }

    /*###################################################################
    #
    #   angular rate analysis in order to disregard linear acceleration
    #
    #   other test values used: 0, 0.00215, 0.00215
    ################################################################### */
    if ((gyro[0] >=-0.005) && (gyro[0] <= 0.005)) {
        omegax = 0;
    } else {
        omegax = 1;
    }

    if((gyro[1] >= -0.005) && (gyro[1] <= 0.005)) {
        omegay = 0;
    } else {
        omegay = 1;
    }

    if((gyro[2] >= -0.005) && (gyro[2] <= 0.005)) {
        omegaz = 0;
    } else {
        omegaz = 1;
    }

    if ((omegax+omegay+omegaz) > 0) {
        omega_test = 1;
    } else {
        omega_test = 0;
    }


    /* 
    ###################################################################
    #
    # combined movement detector
    #
    #################################################################### 
    */
    if ((accnormtestavg + omega_test + accnorm_var_test) > 0) {
        motionDetect = 1;
    } else {
        motionDetect = 0;
    }

    /* 
    ################################################################## 
    */   

    //motion_detect_ma.process(motionDetect);

    if(motion_detect_ma.process(motionDetect) > 0.5) {
        val[11] = 1.0f;
    } else {
        val[11] = 0.0f;
    }
}

void FreeIMU1::MotionDetect(float * val) {
    int i;
    double dVal[12];

    MotionDetect(dVal);
    for (i = 0; i < 12; i++)
        val[i] = (float)dVal[i];
}

/**                           END OF FREEIMU                           **/
/************************************************************************/
/**                           HELP FUNCTIONS                           **/

/**
 * Compensates the accelerometer readings in the 3D vector acc expressed in the sensor frame for gravity
 * @param acc the accelerometer readings to compensate for gravity
 * @param q the quaternion orientation of the sensor board with respect to the world
*/
void gravityCompensateAcc(double * acc, double * q) {
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

/**
 * Sets the Quaternion to be equal to the product of quaternions {@code q1} and {@code q2}.
 * 
 * @param q1
 *          the first Quaternion
 * @param q2
 *          the second Quaternion
 */
void Qmultiply(double *  q, double *  q1, double * q2) {
    q[0] = q1[0] * q2[0] - q1[1] * q2[1] - q1[2] * q2[2] - q1[3] * q2[3];
    q[1] = q1[0] * q2[1] + q2[0] * q1[1] + q1[2] * q2[3] - q1[3] * q2[2];
    q[2] = q1[0] * q2[2] + q2[0] * q1[2] - q1[1] * q2[3] + q1[3] * q2[1];
    q[3] = q1[0] * q2[3] + q2[0] * q1[3] + q1[1] * q2[2] - q1[2] * q2[1];
}

/**
 * Converts a 3 elements array arr of angles expressed in radians into degrees
*/
void arr3_rad_to_deg(double * arr) {
    arr[0] *= 180/M_PI;
    arr[1] *= 180/M_PI;
    arr[2] *= 180/M_PI;
}

/**
 * Converts a 3 elements array arr of angles expressed in degrees into radians
*/
void arr3_deg_to_rad(double * arr) {
    arr[0] /= 180/M_PI;
    arr[1] /= 180/M_PI;
    arr[2] /= 180/M_PI;
}

double FreeIMU1::invSqrt(double x) {
    return 1.0f / sqrt(x);
}

void FreeIMU1::setAcceValues(double ax, double ay, double az) {
    acceDataFromExternal = true;
    externAcce[0] = ax;
    externAcce[1] = ay;
    externAcce[2] = az;
}

void FreeIMU1::setGyroValues(double gx, double gy, double gz) {
    gyroDataFromExternal = true;
    externGyro[0] = gx;
    externGyro[1] = gy;
    externGyro[2] = gz;
}

void FreeIMU1::setMagnValues(double mx, double my, double mz) {
    magnDataFromExternal = true;
    externMagn[0] = mx;
    externMagn[1] = my;
    externMagn[2] = mz;
}

bool FreeIMU1::QuaternionInitialize() {
    int i, n, cnt;
    int a2qMethod;
    double x, y, z;
    double cx, cy, cz, sx, sy, sz;
    double m00, m01, m02, m10, m11, m12, m20, m21, m22;
    double hx, hy;
    double s;
    double val[9], sum[9];
    double recipNorm;
    
    cnt = 0;
    do {
        
        if (cnt >= 3) {
            
            return false;
            
        }
        
        for (i = 0; i < 9; i++) {
            
            sum[i] = 0.0;
            
        }
        
        for (n = 0; n < nsamples; n++) {
        
            getValues(val);
            
            for (i = 0; i < 9; i++) {
                
                sum[i] += val[i];
                
            }
        
        }
        
        for (i = 0; i < 9; i++) {
            
            val[i] = sum[i]/nsamples;
            
        }
        
        x = atan2(val[1], val[2]);
        y = -atan2(val[0], sqrt(val[1]*val[1] + val[2]*val[2]));
        z = 0;

        sx = sin(x);
        sy = sin(y);
        cx = cos(x);
        cy = cos(y);
        sz = sin(z);
        cz = cos(z);
        
        if (val[6] != 0.0 || val[7] != 0.0 || val[8] != 0.0) {
            
            hx = val[6]*cy + val[7]*sy*sx + val[8]*cx*sy;
            hy = val[7]*cx - val[8]*sx;
            
            z = atan2(hx, hy) - M_PI/2;
            sz = sin(z);
            cz = cos(z);
            
        }
        
        m00 = cy*cz;
        m10 = cy*sz;
        m20 = -sy;
        m01 = cz*sy*sx - cx*sz;
        m11 = cx*cz + sy*sx*sz;
        m21 = cy*sx;
        m02 = cx*cz*sy + sx*sz;
        m12 = cx*sy*sz - cz*sx;
        m22 = cy*cx;
        
        if ((m00 + m11 + m22) > 0) {
            
            s = sqrt(1 + m00 + m11 + m22)*2.0;
            a2qMethod = 0;
            
        } else if (m00 > m11 && m00 > m22) {
            
            s = sqrt(1 + m00 - m11 - m22)*2.0;
            a2qMethod = 1;
            
        } else if (m11 > m22) {
            
            s = sqrt(1 + m11 - m00 - m22)*2.0;
            a2qMethod = 2;
            
        } else {
            
            s = sqrt(1 + m22 - m00 - m11)*2.0;
            a2qMethod = 3;
            
        }
        
        cnt++;
        
    } while (fabs(s) < 0.000001);
    
    if (a2qMethod == 0) {
        
        q0 = 0.25*s;
        q1 = (m21 - m12)/s;
        q2 = (m02 - m20)/s;
        q3 = (m10 - m01)/s;
        
    } else if (a2qMethod == 1) {
        
        q0 = (m21 - m12)/s;
        q1 = 0.25*s;
        q2 = (m01 + m10)/s;
        q3 = (m02 + m20)/s;
        
    } else if (a2qMethod == 2) {
        
        q0 = (m02 - m20)/s;
        q1 = (m01 + m10)/s;
        q2 = 0.25*s;
        q3 = (m12 + m21)/s;
        
    } else {
        
        q0 = (m10 - m01)/s;
        q1 = (m02 + m20)/s;
        q2 = (m12 + m21)/s;
        q3 = 0.25*s;
        
    }
    
	recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 *= recipNorm;
	q1 *= recipNorm;
	q2 *= recipNorm;
	q3 *= recipNorm;

    SEq_1 = q0;
    SEq_2 = q1;
    SEq_3 = q2;
    SEq_4 = q3;
    
    return true;
}
