/*
  stepperqueue.h - DM&P Vortex86 Motion86 library
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

#ifndef __STEPPERQUEUE_H
#define __STEPPERQUEUE_H

#include "stepper.h"

class MotStepperQueue
{
  public:
    MotStepperQueue();
    ~MotStepperQueue();

    bool st_buffer_null();

    unsigned long next_stepper_index(unsigned long stepper_index);

    unsigned long prev_stepper_index(unsigned long stepper_index);

    unsigned long get_stepper_num(void);

    bool st_discard_current_block(void);

    MotStepper volatile * plan_get_current_stepper(void);

    void plan_discard_current_stepper(void);

    bool buffer_is_full_by_one(void);

    MotStepper *st_get_current_head(void) {
      return &stepper_buffer[stepper_buffer_head];
    };

    void st_update_head_index(void);

    void clearStepperBuffer(void);

    unsigned long st_get_buffer_size(void);
  private:
    unsigned long stepper_buffer_size;
    unsigned long stepper_buffer_bound;
    volatile unsigned long stepper_buffer_head, stepper_buffer_tail;
    MotStepper *stepper_buffer;
};

#endif

