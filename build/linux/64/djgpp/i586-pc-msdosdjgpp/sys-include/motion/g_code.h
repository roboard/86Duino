/*
  g_code.h - DM&P Vortex86 Motion86 library
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

#ifndef __G_CODE_H
#define __G_CODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <Encoder.h>

#include "global.h"
#include "setting.h"
#include "blockqueue.h"
#include "stepperqueue.h"
#include "planner.h"
#include "interpolator.h"
#include "dda.h"

enum workPlane_t
{
  XY_PLANE = 0,
  XZ_PLANE = 1,
  YZ_PLANE = 2
};

class MotGcode
{
  public:
    MotGcode(MotSetting *_setting, MotBlockQueue *_bqueue, MotStepperQueue *_squeue, MotPlanner *_planner, MotInterpolator *_interpolator, MotDDA *_dda);

    void process_commands(char *cmdbuffer);

    //softlimit
    void SetAxisLimitPos(int axis, int min, int max);
    void SetSoftLimit(int Enable);
    bool GetSoftLimitMode();

    void SetAxisStepsPerMM(int axis, double steps_per_mm);
    double GetAxisStepsPerMM(int axis);
    void SetMotorAcceleration(double vel);
    double GetMotorAcceleration(void);
    void SetAxisMaxFeedRate(int axis, double max_feedrate_mps);
    double GetAxisMaxFeedRate(int axis);
    void SetMotorDirInvert(int axis, int invert);
    int GetMotorDirInvert(int axis);

    void SetEMGStatus(bool status);

    double GetCurrentPos(int axis);
    void GetCurrentPos(double *pos);

    void SetFeedrateRatio(double ratio);
    double GetFeedrateRatio();
    void SetFeedrate(double feedrate);
    bool IsFinishMove(void);

    int getBlockCount();
    int getFreeBlockCount();
    bool IsBlockFull(void);

    void helical_Theta(workPlane_t mode, double planeA, double planeB, double vertical, double theta, double feedrate);
    bool IsRelativeMode();
    void ComputeDst(double cA, double cB, double theta, double *dstA, double *dstB);

    void SetJogFeedrate(double feedrate);
    bool prepare_jog_move(double* vector);
    double GetJogPos(int axis);
    void GetJogPos(double *pos);

    void AttachEGear(MotPlanner *target);
    void DettachEGear();
    bool isEGearAttached();
  private:
    MotSetting *setting;
    MotBlockQueue *bqueue;
    MotStepperQueue *squeue;
    MotPlanner *planner;
    MotInterpolator *interpolator;
    MotDDA *dda;
    /*****CMD*****/
    char *cmdbuffer;
    char *strchr_pointer = 0;
    int max_cmd_size;
    /*****end CMD*****/
    bool relative_mode = false;  //Determines Absolute or Relative Coordinates
    bool arc_distance_mode = true;
    const char *axis_codes = "XYZ";
    double destination[3] = {0.0, 0.0, 0.0};
    double current_position[3] = {0.0, 0.0, 0.0};
    double offset[3] = {0.0, 0.0, 0.0};
    long circle = 1;
    double radius = 0;
    double feedrate = 600.0;
    double jogFeedrate = 600.0;
    char clockflag = 0;

    double code_value();
    long code_value_long();
    bool code_seen(char code);
    void get_coordinates();
    void get_arc_coordinates();
    void prepare_move(void);
    void prepare_arc_move(void);
    void state_wait_delay(void);
    void emergency_stop(void);
	void EGear_plan_buffer_line(double* dest, double feed);

    void line(double x, double y, double z, double feedrate);
    void set_arc_plane(workPlane_t mode);
    void arc(workPlane_t mode, double refA, double refB, double dstA, double dstB, double dstC, bool revDir, double feedrate);
    int arc_axis_map[MAX_AXIS] = { X_AXIS, Y_AXIS, Z_AXIS };

    MotPlanner *EGear_planner = NULL;
};

#endif