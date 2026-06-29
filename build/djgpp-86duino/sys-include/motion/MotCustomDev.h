/*
  MotCustomDev.h - DM&P Vortex86 Motion86 library
  Copyright (c) 2025 RoBoardGod <roboardgod@dmp.com.tw>. All right reserved.

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

#ifndef __CUSTOMDEV_H
#define __CUSTOMDEV_H

#include "global.h"
#include "MotInterface.h"


class MotCustomDev : public MotInterface
{
  public:
    MotCustomDev(IMotCB *_machine, MotSetting *_setting, MotStepperQueue *_squeue);
    ~MotCustomDev() override;

    void st_set_position(const long &x, const long &y, const long &z) override;
    long st_get_position(int axis) override;
    void st_get_position(long *pos) override;
    void st_set_motor_dir_invert(int axis, int invert) override;
    int st_get_motor_dir_invert(int axis) override;
    
    bool Init() override;
    bool AttachIfIdle() override {
      return false;
    };
    bool DetachIfIdle() override { 
      return false; 
    };
    void Update() override;

    const std::type_info& getType() const override {
        return typeid(MotCustomDev);
    }

    MotSetting *setting;
    MotStepperQueue *squeue;
    volatile int mot_state;
    MotStepper volatile *current_stepper;
    volatile long count_position[MAX_AXIS];
    int dir_invert[MAX_AXIS];
  private:
    IMotCB *imotCB = NULL;
    long startPos[MAX_AXIS];
};

#endif
