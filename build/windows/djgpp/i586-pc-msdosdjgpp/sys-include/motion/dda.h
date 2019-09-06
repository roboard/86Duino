/*
  dda.h - DM&P Vortex86 Motion86 library
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

#ifndef __DDA_H
#define __DDA_H

#include "mcm.h"
#include "io.h"
#include "irq.h"
#include "Arduino.h"
#include "global.h"
#include "setting.h"
#include "stepperqueue.h"

typedef struct {
  int motor_mc;
  int motor_md;
  unsigned long lock_val;
  int dir_invert;
} Axis_t;

class MotDDA
{
  public:
    MotDDA(MotSetting *_setting, MotStepperQueue *_squeue);
    ~MotDDA();

    void st_set_position(const long &x, const long &y, const long &z);
    long st_get_position(int axis);
    void st_get_position(long *pos);
    void st_set_motor_dir_invert(int axis, int invert);
    int st_get_motor_dir_invert(int axis);
    void set_reg(unsigned long sc, unsigned long *dda, unsigned long period, unsigned long duty);
    void set_idle(void);
	bool DDAInit();

    MotSetting *setting;
    MotStepperQueue *squeue;
    Axis_t AXIS[MAX_AXIS];
    volatile int isr_state;
    MotStepper volatile *current_stepper;
    volatile long count_position[MAX_AXIS];

  private:
    void stepper_map();
    void servo_configuration(void);
    bool MCMInit();
    bool MCMColse();
    bool DDAClose();
};

bool dda_AttachInterrupt();
bool dda_DetachInterrupt();

#endif
