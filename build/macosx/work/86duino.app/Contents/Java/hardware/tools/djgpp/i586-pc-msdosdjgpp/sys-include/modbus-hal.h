/*
  modbus-hal.h - Modbus HAL Library.
  Copyright (c) 2024 Johnson Hung <Dyhung@dmp.com.tw>. All right reserved.

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
  
*/

#ifndef __MODBUS_HAL_H
#define __MODBUS_HAL_H

#include <stdlib.h>

typedef struct {
    int (*connect)(void *);
    int (*disconnect)(void *);
    int (*available)(void *);
    int (*flush)(void *);
    int (*write)(void *, void *, size_t);
    int (*read)(void *, void *, size_t);
    void *user_data;
} modbus_hal_t;

#endif
