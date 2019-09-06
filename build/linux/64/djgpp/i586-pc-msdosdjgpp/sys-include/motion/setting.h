/*
  setting.h - DM&P Vortex86 Motion86 library
  Copyright (c) 2019 RoBoardGod <roboardgod@dmp.com.tw>. All right reserved.

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

#ifndef __SETTING_H
#define __SETTING_H

#include "global.h"

#ifndef PulseMode_t
#define PulseMode_t
enum pulseMode_t
{
  MODE_NOSET = 0xFF,
  MODE_STEP_DIR = 0,
  MODE_CWCCW = 1,
  MODE_AB_PHASE = 2,
  MODE_CAPTURE = 3,
  MODE_SSI = 4, // continue mode, no interrupt event
  MODE_STEP_DIR_x2 = 5, // another mode
  MODE_CWCCW_x2 = 6, // another mode
  MODE_AB_PHASE_x2 = 7 // another mode
};
#endif

enum machineMode_t
{
  MachineOff = 0,
  MachineOn = 1,
  JogMode = 2,
  IncJogMode = 3,
  MpgMode = 4
};

class MotSetting
{
    friend class MotPlanner;
    friend class MotInterpolator;

  public:
    MotSetting(int axis_group);

    void setEMGStatus(bool status) {
      EmergencyStop = status;
    };
    bool getEMGStatus(void) {
      return EmergencyStop;
    };

    void setUsedAxis(unsigned int axis) {
      Used_Axis = axis;
    };
    unsigned int getUsedAxis(void) {
      return Used_Axis;
    };

    void setIsSoftLimit(int limit) {
      Is_SoftLimit = limit;
    };
    int getIsSoftLimit(void) {
      return Is_SoftLimit;
    };

    void setAxisMinPos(int axis, double pos);
    double getAxisMinPos(int axis);

    void setAxisMaxPos(int axis, double pos);
    double getAxisMaxPos(int axis);

    void enableMinSLimit(void) {
      Machine.min_software_limit = 1;
    };
    void disableMinSLimit(void) {
      Machine.min_software_limit = 0;
    };
    int getMinSLimit(void) {
      return Machine.min_software_limit;
    };

    void enableMaxSLimit(void) {
      Machine.max_software_limit = 1;
    };
    void disableMaxSLimit(void) {
      Machine.max_software_limit = 0;
    };
    int getMaxSLimit(void) {
      return Machine.max_software_limit;
    };

    void setDsicardFlag(void) {
      discard_flag = 1;
    };
    void resetDiscardFlag(void) {
      discard_flag = 0;
    };
    volatile int getDiscardFlag(void) {
      return discard_flag;
    };

    int getVelcalcMode(void) {
      return Sys.vel_calc_mode;
    };
    double getPulseDutyRatio(void) {
      return Machine.step_pulse_duty_ratio;
    };

    int SetNcmSamplecycleByPulseSpeed(int pfhz);
    int GetNcmSamplecycle(void);

    void SetAxisStepsPerMM(int axis, double steps_per_mm);
    double GetAxisStepsPerMM(int axis);

    void SetMotorAcceleration(double vel);
    double GetMotorAcceleration(void);

    void SetAxisMaxFeedRate(int axis, double max_feedrate_mps);
    double GetAxisMaxFeedRate(int axis);

    double GetDDACycle();

    void SetPulseMode(int axis, pulseMode_t mode);
    pulseMode_t GetPulseMode(int axis);

    int axis_group_number;
    machineMode_t machineMode;
    int homeLimit[3];
    double EGearRatio[3] = { 1.0, 1.0, 1.0 };

  private:
    unsigned int Used_Axis;

    int Is_SoftLimit;

    double min_pos[MAX_AXIS];
    double max_pos[MAX_AXIS];

    volatile bool EmergencyStop;

    volatile int discard_flag;

    pulseMode_t pulseMode[MAX_AXIS];

    struct
    {
      double steps_per_mm[MAX_AXIS];
      int min_software_limit;
      int max_software_limit;
      double qmax_acceleration[MAX_AXIS];
      int dropsegments;
      double minimumfeedrate;
      double max_feedrate_mps[MAX_AXIS];
      double acceleration;

      double max_xy_jerk; // steps/sec
      double max_z_jerk; // steps/sec

      double minimum_planner_speed;

      int mcm_samplecycle;
      double step_pulse_duty_ratio;
    } Machine;

    struct
    {
      int vel_calc_mode;
    } Sys;
};

#endif
