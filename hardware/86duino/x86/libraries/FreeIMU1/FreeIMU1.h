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

Modified 23 December 2015 by Johnson Hung

*/
#ifndef FreeIMU1_h
#define FreeIMU1_h

#include "Arduino.h"
#include "calibration.h"
#include "utility/DCM.h"
#include "utility/iCompass.h"

#ifndef CALIBRATION_H
#include <EEPROM.h>
#endif

#define FREEIMU_EEPROM_BASE         (0x0A)
#define FREEIMU_EEPROM_SIGNATURE    (0x19)

#define HAS_LSM330DLC()  (0)
#define FIMU1_ACC_ADDR   (0x30 >> 1)
#define FIMU1_GYRO_ADDR  (0xD4 >> 1)

#ifdef __86DUINO_ONE
	#define HAS_LSM330DLC()  (1)
	#define FIMU1_ACC_ADDR   (0x30 >> 1)
	#define FIMU1_GYRO_ADDR  (0xD4 >> 1)
#endif

#if HAS_LSM330DLC()
	#include "utility/lsm330dlc.h"
#endif

//Magnetic declination angle for iCompass
#define MAG_DEC 0

//Number of samples to average in iCompass
#define WINDOW_SIZE 1 //Set to 1 to turn off the Running Average

// proportional gain governs rate of convergence to accelerometer/magnetometer
// integral gain governs rate of convergence of gyroscope biases
// set up defines for various boards in my inventory, DFROBOT and Freeimu have
// temperature calibration curves. (3.31.14)
// Kp and Ki are used in the MahonyAHRS and betaDef in the MadgwickAHRS
	#define twoKpDef  (2.0f * 0.5f)
	#define twoKiDef  (2.0f * 0.1f)
	#define betaDef  0.1f
	//Used for DCM filter
	const double Kp_ROLLPITCH = 1.2f;  //was .3423
	const double Ki_ROLLPITCH = 0.0234f;
	const double Kp_YAW = 1.75f;   // was 1.2 and 0.02
	const double Ki_YAW = 0.002f;

//
// Other Options
//
#define nsamples 75

// Set filter type:
//    0 -> Madgwick implementation of Mahoney DCM in Quaternion form
//    1 -> Madgwick Gradient Descent
//    3 -> Madgwick Original Paper AHRS
//    4 -> DCM Implementation
#define DEFULT_MARG (1)

// --------------------------------------------------------------------
// Define MARG = 3 factors here
// --------------------------------------------------------------------
#define gyroMeasError 3.14159265358979 * (.50f / 180.0f) 	// gyroscope measurement error in rad/s (shown as 5 deg/s)
#define gyroMeasDrift 3.14159265358979 * (0.02f / 180.0f) 	// gyroscope measurement error in rad/s/s (shown as 0.2f deg/s/s)

#define beta1 sqrt(3.0f / 4.0f) * gyroMeasError 			// compute beta
#define zeta sqrt(3.0f / 4.0f) * gyroMeasDrift 				// compute zeta

class FreeIMU1
{
public:
    FreeIMU1();

    void RESET();
    void RESET_Q();

    void init();
    void init(bool fastmode);
    void init(int acc_addr, int gyro_addr, bool fastmode);
    void init(int type);
    void init(bool fastmode, int type);
    void init(int acc_addr, int gyro_addr, bool fastmode, int type);

    #ifndef CALIBRATION_H
    void calLoad();
    #endif

    void zeroGyro();
    void initGyros();
    void getRawValues(int * raw_values);
    void getValues(float * values);
    void getValues(double * values);
    void getQ(float * q, float * val);
    void getQ(double * q, double * val);
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
    void getYawPitchRollRadAHRS(float * ypr, float * q);
    void getYawPitchRollRadAHRS(double * ypr, double * q);
    void getYawPitchRoll180(float * ypr);
    void getYawPitchRoll180(double * ypr);
    void getEuler360deg(float * angles);	
    void getEuler360deg(double * angles);
    void getEuler360(float * angles);
    void getEuler360(double * angles);
    void getEuler360degAttitude(float * angles, float * q, float * val);
    void getEuler360degAttitude(double * angles, double * q, double * val);
    double invSqrt(double x);
    double calcMagHeading(double q0, double q1, double q2, double q3, double bx, double by, double bz);
    void getQ_simple(float* q, float * val);
    void getQ_simple(double* q, double * val);
    void MotionDetect(float * val);
    void MotionDetect(double * val);
    
    bool acceDataFromExternal;
    bool gyroDataFromExternal;
    bool magnDataFromExternal;
    double externAcce[3];
    double externGyro[3];
    double externMagn[3];
    void setAcceValues(double ax, double ay, double az);    // Unit: g
    void setGyroValues(double gx, double gy, double gz);    // Unit: degree/sec
    void setMagnValues(double mx, double my, double mz);    // Unit: gauss

    bool timeSampleFirst;
    
    DCM dcm;
    iCompass maghead;
    #if HAS_LSM330DLC()
    LSM330DLC accgyro;
    #endif

    //Global Variables

    // calibration parameters
    int16_t gyro_off_x, gyro_off_y, gyro_off_z;
    int16_t acc_off_x, acc_off_y, acc_off_z, magn_off_x, magn_off_y, magn_off_z;
    double acc_scale_x, acc_scale_y, acc_scale_z, magn_scale_x, magn_scale_y, magn_scale_z;
    double sampleFreq; // half the sample period expressed in seconds

private:

    int MARG;
    double bx, by, bz;
    volatile double twoKp;      				// 2 * proportional gain (Kp)
    volatile double twoKi;      				// 2 * integral gain (Ki)
    volatile double q0, q1, q2, q3; 	// quaternion of sensor frame relative to auxiliary frame
    volatile double integralFBx,  integralFBy, integralFBz;
    unsigned long lastUpdate, now; 			// sample period expressed in milliseconds

    //Madgwick AHRS Gradient Descent 
    volatile double beta;				// algorithm gain

    //Following lines defines Madgwicks Grad Descent Algorithm from his original paper
    // Global system variables
    double SEq_1 = 1, SEq_2 = 0, SEq_3 = 0, SEq_4 = 0; 	// estimated orientation quaternion elements with initial conditions
    double b_x = 1, b_z = 0; 				// reference direction of flux in earth frame
    double w_bx = 0, w_by = 0, w_bz = 0; // estimate gyroscope biases error

    void AHRSupdate(double gx, double gy, double gz, double ax, double ay, double az, double mx, double my, double mz);
    void MadgwickAHRSupdate(double gx, double gy, double gz, double ax, double ay, double az, double mx, double my, double mz);
    void MadgwickAHRSupdateIMU(double gx, double gy, double gz, double ax, double ay, double az);
    void MARGUpdateFilter(double gx, double gy, double gz, double ax, double ay, double az, double mx, double my, double mz);
    void MARGUpdateFilterIMU(double w_x, double w_y, double w_z, double a_x, double a_y, double a_z);
};

void arr3_rad_to_deg(double * arr);
void arr3_deg_to_rad(double * arr);

#endif // FreeIMU_h

