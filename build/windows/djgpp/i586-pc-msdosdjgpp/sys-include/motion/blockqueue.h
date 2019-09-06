/*
  blockqueue.h - DM&P Vortex86 Motion86 library
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

#ifndef __BLOCKQUEUE_H
#define __BLOCKQUEUE_H

#include "block.h"

class MotBlockQueue
{
  public:
    MotBlockQueue();
    ~MotBlockQueue();

    void clearBlockBuffer(void);

    unsigned long next_block_index(unsigned long block_index);

    unsigned long prev_block_index(unsigned long block_index);

    unsigned long get_block_num(void);

    void plan_discard_current_block(void);

    MotBlock *plan_get_current_block(void);

    MotBlock *plan_get_block(unsigned long index);

    bool plan_buffer_null(void);

    bool buffer_is_full_by_one(void);

    MotBlock *plan_get_current_head(void) {
      return &block_buffer[block_buffer_head];
    };

    void plan_update_head_index(void);

    int plan_queued_move(void) {
      return (block_buffer_head - block_buffer_tail + block_buffer_size) & (block_buffer_size - 1);
    };

    void plan_set_head_index(unsigned long index);

    unsigned long plan_get_head_index(void) {
      return block_buffer_head;
    };

    unsigned long plan_get_tail_index(void) {
      return block_buffer_tail;
    };

    unsigned long plan_get_buffer_size(void) {
      return block_buffer_size;
    };

  private:
    unsigned long block_buffer_size;
    volatile unsigned long block_buffer_head;
    volatile unsigned long block_buffer_tail;
    MotBlock *block_buffer;
};

#endif
