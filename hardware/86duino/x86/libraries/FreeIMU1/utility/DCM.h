#ifndef DCM_H
#define DCM_H

/***************************************************************************************************************
* Razor AHRS Firmware v1.4.2
* 9 Degree of Measurement Attitude and Heading Reference System
* for Sparkfun "9DOF Razor IMU" (SEN-10125 and SEN-10736)
* and "9DOF Sensor Stick" (SEN-10183, 10321 and SEN-10724)
*
* Released under GNU GPL (General Public License) v3.0
* Copyright (C) 2013 Peter Bartz [http://ptrbrtz.net]
* Copyright (C) 2011-2012 Quality & Usability Lab, Deutsche Telekom Laboratories, TU Berlin
*
* Infos, updates, bug reports, contributions and feedback:
*     https://github.com/ptrbrtz/razor-9dof-ahrs
*
*
* History:
*   * Original code (http://code.google.com/p/sf9domahrs/) by Doug Weibel and Jose Julio,
*     based on ArduIMU v1.5 by Jordi Munoz and William Premerlani, Jose Julio 
*/

/*
  Axis definition (differs from definition printed on the board!):
    X axis pointing forward (towards the short edge with the connector holes)
    Y axis pointing to the right
    and Z axis pointing down.
    
  Positive yaw   : clockwise
  Positive roll  : right wing down
  Positive pitch : nose up
  
  Transformation order: first yaw then pitch then roll.
*/


// DEBUG OPTIONS
/*****************************************************************/
// When set to true, gyro drift correction will not be applied

/*****************************************************************/

class DCM
{
	public:
		DCM();
		
		// DCM parameters
		//#define Kp_ROLLPITCH 0.02f
		//#define Ki_ROLLPITCH 0.00002f
		//#define Kp_YAW 1.2f
		//#define Ki_YAW 0.00002f
		
		//#define Kp_ROLLPITCH 1.2f  //was .3423
		//#define Ki_ROLLPITCH 0.0234f
		//#define Kp_YAW 1.75f   // was 1.2 and 0.02
		//#define Ki_YAW 0.002f		
		
		#define DEBUG__NO_DRIFT_CORRECTION false
		
		// Stuff
		//#define GRAVITY 256.0f 				   // "1G reference" used for DCM filter and accelerometer calibration
		#define GRAVITY 1.0f //everything coming accross normalized from FreeIMU library.
		#define TO_RAD(x) (x * 0.01745329252)  // *pi/180
		#define TO_DEG(x) (x * 57.2957795131)  // *180/pi			
		
		// Sensor variables
		double accel[3] = {0, 0, 0};  
		double magnetom[3] = {0, 0, 0};
		double gyro[3] = {0, 0, 0};
		
		// DCM variables
		double MAG_Heading = 0.0f;
		
		// Euler angles
		double yaw = 0.0f;
		double pitch = 0.0f;
		double roll = 0.0f;
		double q0, q1, q2, q3;
		
		// DCM timing in the main loop
		unsigned long timestamp = 0;
		unsigned long timestamp_old = 0;
		double G_Dt = 0.0f; // Integration time for DCM algorithm
		
		void DCM_init(double Kp_RP, double Ki_RP, double Kp_Y, double Ki_Y);
		void setSensorVals(double * cal_val);
		void Matrix_update(void);
		void Drift_correction(void);
		void Normalize(void);
		void reset_sensor_fusion();
		void getEulerRad(double * angles);
		void getEulerDeg(double * angles);
		void calDCM();
		void getDCM2Q(double * q);
	
	  private:
		// Sensor variables
		double magnetom_tmp[3] = {0, 0, 0};

		// DCM variables
		double Kp_ROLLPITCH, Ki_ROLLPITCH, Kp_YAW, Ki_YAW;
		double Accel_Vector[3]= {0, 0, 0}; // Store the acceleration in a vector
		double Gyro_Vector[3]= {0, 0, 0}; // Store the gyros turn rate in a vector
		double Omega_Vector[3]= {0, 0, 0}; // Corrected Gyro_Vector data
		double Omega_P[3]= {0, 0, 0}; // Omega Proportional correction
		double Omega_I[3]= {0, 0, 0}; // Omega Integrator
		double Omega[3]= {0, 0, 0};
		double errorRollPitch[3] = {0, 0, 0};
		double errorYaw[3] = {0, 0, 0};
		double DCM_Matrix[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
		double Update_Matrix[3][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}};
		double Temporary_Matrix[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

		// More output-state variables
		int num_accel_errors = 0;
		int num_magn_errors = 0;
		int num_gyro_errors = 0;
	
		void init_rotation_matrix(double m[3][3], double yaw, double pitch, double roll);
		void Matrix_Vector_Multiply( double a[3][3],  double b[3], double out[3]);
		void Matrix_Multiply( double a[3][3],  double b[3][3], double out[3][3]);
		void Vector_Add(double out[3],  double v1[3],  double v2[3]);
		void Vector_Scale(double out[3],  double v[3], double scale);
		void Vector_Cross_Product(double *out,  double *v1,  double *v2);
		double Vector_Dot_Product(double *v1, double *v2);

};		
		

#endif