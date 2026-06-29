/*
  MotInterface.h - DM&P Vortex86 Motion86 library
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

#ifndef __MOTINTERFACE_H
#define __MOTINTERFACE_H

#define MOT_IDLE       (0x00)
#define MOT_RUNNING   (0x01)

#include <typeinfo>

#include "global.h"
#include "setting.h"
#include "stepperqueue.h"

class IMotCB {
  protected:
    virtual void setAxisPosition(int axis, long pos) = 0;
    virtual long getAxisPosition(int axis) = 0;
    friend class MotCustomDev;
};

class MotInterface
{
  public:
    virtual ~MotInterface() { };

    virtual void st_set_position(const long &x, const long &y, const long &z) = 0;
    virtual long st_get_position(int axis) = 0;
    virtual void st_get_position(long *pos) = 0;
    virtual void st_set_motor_dir_invert(int axis, int invert) = 0;
    virtual int st_get_motor_dir_invert(int axis) = 0;
    virtual bool Init() = 0;
    virtual bool AttachIfIdle() = 0;
    virtual bool DetachIfIdle() = 0;
    virtual void Update() = 0;
	

    virtual const std::type_info& getType() const = 0;

    template<typename T>
    bool isType() const {
        return typeid(T) == getType();
    };
  private:
};

#endif
