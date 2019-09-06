/*
  planner.h - DM&P Vortex86 Motion86 library
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

#ifndef __PLANNER_H
#define __PLANNER_H

#include "blockqueue.h"
#include "setting.h"
#include "stepperqueue.h"
#include "dda.h"

class MotPlanner
{
  public:
    MotPlanner(MotSetting *_setting, MotBlockQueue *_queue, MotDDA *_dda);
    void plan_buffer_line(double *work, double feed_rate);
    void replan_buffer_line();
    void replan_buffer_line(double ratio);
    void plan_set_position(double *work);
    double plan_get_position_mm(int axis);
    void plan_get_position_mm(double *pos);
    void plan_update_position(void);
    double GetFeedrateRatio();

  private:
    MotSetting *setting;
    MotBlockQueue *queue;
    MotDDA *dda;

    unsigned long axis_steps_per_sqr_second[MAX_AXIS];
    double cartesian_pos_mm[MAX_AXIS];
    long motor_pos_steps[MAX_AXIS];
    double previous_speed[MAX_AXIS]; // Speed of previous path line segment
    double previous_nominal_speed; // Nominal speed of previous path line segment
    double feedrate_ratio;

    void Workspace2Cartesian(double *work, double *cart);
    void Cartesian2Workspace(double *work, double *cart);

    void clamp_to_software_endstops(double *target);

    void inverse_kinematics(double *Cartesian, double *Motor);
    void forward_kinematics(double *Cartesian, double *Motor);

    void AxistoMotor(double *Axis, long *Motor);
    void MotortoAxis(double *Axis, long *Motor);

    void reset_acceleration_rates(void);

    double max_allowable_speed(double acceleration, double target_velocity, double distance);

    void calculate_trapezoid_for_block(MotBlock *block, double entry_factor, double exit_factor);

    double estimate_acceleration_distance(double initial_rate, double target_rate, double acceleration);

    double intersection_distance(double initial_rate, double final_rate, double acceleration, double distance);

    void plan_recalculate(unsigned long head, unsigned long tail);
    void planner_reverse_pass(unsigned long head, unsigned long tail);
    void planner_reverse_pass_kernel(MotBlock *previous, MotBlock *current, MotBlock *next);
    void planner_forward_pass(unsigned long head, unsigned long tail);
    void planner_forward_pass_kernel(MotBlock *previous, MotBlock *current, MotBlock *next);
    void planner_recalculate_trapezoids(unsigned long head, unsigned long tail);

    void get_axis_pos(double *pos);
    void get_cartesian_pos(double *pos);
};

#endif
