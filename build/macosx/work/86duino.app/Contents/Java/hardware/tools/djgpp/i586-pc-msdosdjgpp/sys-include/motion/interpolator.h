/*
  interpolator.h - DM&P Vortex86 Motion86 library
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

#ifndef __INTERPOLATOR_H
#define __INTERPOLATOR_H

#include <TimerWDT.h>
#include "global.h"
#include "setting.h"
#include "blockqueue.h"
#include "stepperqueue.h"
#include "MotInterface.h"

class MotInterpolator
{
  public:
    MotInterpolator(MotSetting *_setting, MotBlockQueue *_bqueue, MotStepperQueue *_squeue, MotInterface *_motDevice);
    ~MotInterpolator();

    void run_cycle(void);
    void clearInterpolatorBuffer(void);
    int getTimerPeriod();
    void st_set_motor_dir_invert(int axis, int invert);
    int st_get_motor_dir_invert(int axis);
	
	 
    static bool AttachInterrupt();
    static bool DetachInterrupt(); 
	
  private:
    MotSetting *setting;
    MotBlockQueue *bqueue;
    MotStepperQueue *squeue;
    MotInterface *motDevice;

    MotBlock *current_block;
    
    long block_steps[MAX_AXIS];
    unsigned char block_dir_bits;
    long counter[MAX_AXIS];

    volatile double acc_vel_square, dec_vel_square;

    volatile unsigned long step_events_completed;
    unsigned char motor_dir[MAX_AXIS];
    int count_dir[MAX_AXIS];
    double skip_pluse = 0.0;
    double step_nominal;
    double acceleration_time, deceleration_time, acc_step_rate;
    double tmp_period;

    unsigned long calc_vel(double acc_time, unsigned long acc_st);
    unsigned long acc_calc(unsigned long count, unsigned long acc_st);
    unsigned long dec_calc(unsigned long count, unsigned long acc_st);
    double calc_timer(double step_rate);
};

#endif
