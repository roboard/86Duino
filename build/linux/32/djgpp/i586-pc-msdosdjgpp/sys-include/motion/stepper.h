/*
  stepper.h - DM&P Vortex86 Motion86 library
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

#ifndef __STEPPER_H
#define __STEPPER_H

#include "global.h"

class MotStepper
{
  public:
    unsigned char is_header;
    unsigned char motor_dir[MAX_AXIS];
    unsigned long period;
    unsigned long duty;
    unsigned long sc;
    unsigned long motor_dda[MAX_AXIS];
    int count_direction[MAX_AXIS];
};

#endif