/*
  block.h - DM&P Vortex86 Motion86 library
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

#ifndef __BLOCK_H
#define __BLOCK_H

#include "global.h"

class MotBlock
{
  public:
    long steps[MAX_AXIS];
    unsigned long step_event_count;
    long accelerate_until;
    long decelerate_after;
    unsigned char direction_bits;

    double nominal_speed;
    double entry_speed;
    double max_entry_speed;
    double millimeters;
    double acceleration;
    unsigned char recalculate_flag;
    unsigned char nominal_length_flag;

    unsigned long nominal_rate;
    unsigned long initial_rate;
    unsigned long final_rate;
    unsigned long acceleration_st;

    double work[MAX_AXIS];
    double feed_rate;
    double current_speed[MAX_AXIS]; // Speed of path line segment
};

#endif
