/*
  Motion86_Ethercat.h - DM&P Vortex86 Motion86 library
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

#ifndef __MOTION86_ETHERCAT_H
#define __MOTION86_ETHERCAT_H

#include "Arduino.h"
#include "Motion86.h"
#include "Ethercat.h"
#include "VirtualServo.h"
#include "VirtualMpg.h"
#include "VirtualEncoder.h"

extern MyEVA EVA;
extern EthercatMaster EcatMaster;
extern bool isRun();
extern void _CyclicCallback(void);
static Machine *_myEcatMachine[__CUSTOM_MACHINE_CNT] = {NULL, NULL, NULL};

void _EcatMachineCallback(void)
{
  int i;
  if (!isRun()) return;
  for (i = 0; i < __CUSTOM_MACHINE_CNT; i++)
    if (_myEcatMachine[i])
      _myEcatMachine[i]->updateAxes();

  _CyclicCallback();
}

class EcatMachine : virtual public Machine
{
  public:
    EcatMachine(axisGroup_t axis_group) : Machine(axis_group) 
    {
      int i;
      if (!isValid()) return;
      if (axis_group >= CUSTOM_MACHINE_0 && axis_group <= CUSTOM_MACHINE_2)
        _myEcatMachine[axis_group - CUSTOM_MACHINE_0] = this;
      vMpg = NULL;
      vEnc = NULL;
      for (i = AXIS_X; i <= AXIS_Z; i++)
      {
        vServo[i] = NULL;
        homePins[i] = -1;
      }
      for (i = READ_JOG_X_POSITIVE; i <= READ_JOG_Z_NEGATIVE; i++)
        jogPins[i] = -1;
    }

    bool attach_EcatServo_X(MyVirtualServo &_vServo) { return attach_EcatServo_X(&_vServo); }
    bool attach_EcatServo_X(MyVirtualServo *_vServo)
    {
      if (!isValid()) return false;
      if (getMachineMode() != MachineOff) return false;
      vServo[AXIS_X] = _vServo;
      servo[AXIS_X] = NULL;
      return true;
    }
    
    bool attach_EcatServo_X(EthercatDevice_CiA402 &_ecatDev) { return attach_EcatServo_X(&_ecatDev); }
    bool attach_EcatServo_X(EthercatDevice_CiA402 *_ecatDev)
    {
      if (!isValid()) return false;
      if (getMachineMode() != MachineOff) return false;
      vServo[AXIS_X] = NULL;
      servo[AXIS_X] = _ecatDev;
      return true;
    }

    bool attach_EcatServo_Y(MyVirtualServo &_vServo) { return attach_EcatServo_Y(&_vServo); }
    bool attach_EcatServo_Y(MyVirtualServo *_vServo)
    {
      if (!isValid()) return false;
      if (getMachineMode() != MachineOff) return false;
      vServo[AXIS_Y] = _vServo;
      servo[AXIS_Y] = NULL;
      return true;
    }

    bool attach_EcatServo_Y(EthercatDevice_CiA402 &_ecatDev) { return attach_EcatServo_Y(&_ecatDev); }
    bool attach_EcatServo_Y(EthercatDevice_CiA402 *_ecatDev)
    {
      if (!isValid()) return false;
      if (getMachineMode() != MachineOff) return false;
      vServo[AXIS_Y] = NULL;
      servo[AXIS_Y] = _ecatDev;
      return true;
    }
    
    bool attach_EcatServo_Z(MyVirtualServo &_vServo) { return attach_EcatServo_Z(&_vServo); }
    bool attach_EcatServo_Z(MyVirtualServo *_vServo)
    {
      if (!isValid()) return false;
      if (getMachineMode() != MachineOff) return false;
      vServo[AXIS_Z] = _vServo;
      servo[AXIS_Z] = NULL;
      return true;
    }
    
    bool attach_EcatServo_Z(EthercatDevice_CiA402 &_ecatDev) { return attach_EcatServo_Z(&_ecatDev); }
    bool attach_EcatServo_Z(EthercatDevice_CiA402 *_ecatDev)
    {
      if (!isValid()) return false;
      if (getMachineMode() != MachineOff) return false;
      vServo[AXIS_Z] = NULL;
      servo[AXIS_Z] = _ecatDev;
      return true;
    }

    bool attach_EcatHomePin_X(int limit)
    {
      if (!isValid()) return false;
      if (getMachineMode() != MachineOff) return false;
      if (limit < 0) return false;
      homePins[AXIS_X] = limit;
      isHomePrepReady = true;
      return true;
    }

    bool attach_EcatHomePin_Y(int limit)
    {
      if (!isValid()) return false;
      if (getMachineMode() != MachineOff) return false;
      if (limit < 0) return false;
      homePins[AXIS_Y] = limit;
      isHomePrepReady = true;
      return true;
    }
    
    bool attach_EcatHomePin_Z(int limit)
    {
      if (!isValid()) return false;
      if (getMachineMode() != MachineOff) return false;
      if (limit < 0) return false;
      homePins[AXIS_Z] = limit;
      isHomePrepReady = true;
      return true;
    }

    bool attach_EcatJogPins_X(int positivePin, int negativePin, ioPolarity_t positivePolarity = ACTIVE_HIGH, ioPolarity_t negativePolarity = ACTIVE_HIGH)
    {
      if (!isValid()) return false;
      if (getMachineMode() != MachineOff) return false;
      jogPins[READ_JOG_X_POSITIVE] = positivePin;
      jogPins[READ_JOG_X_NEGATIVE] = negativePin;
      JogPins_polarity[READ_JOG_X_POSITIVE] = positivePolarity;
      JogPins_polarity[READ_JOG_X_NEGATIVE] = negativePolarity;
      return true;
    }

    bool attach_EcatJogPins_Y(int positivePin, int negativePin, ioPolarity_t positivePolarity = ACTIVE_HIGH, ioPolarity_t negativePolarity = ACTIVE_HIGH)
    {
      if (!isValid()) return false;
      if (getMachineMode() != MachineOff) return false;
      jogPins[READ_JOG_Y_POSITIVE] = positivePin;
      jogPins[READ_JOG_Y_NEGATIVE] = negativePin;
      JogPins_polarity[READ_JOG_Y_POSITIVE] = positivePolarity;
      JogPins_polarity[READ_JOG_Y_NEGATIVE] = negativePolarity;
      return true;
    }
    
    bool attach_EcatJogPins_Z(int positivePin, int negativePin, ioPolarity_t positivePolarity = ACTIVE_HIGH, ioPolarity_t negativePolarity = ACTIVE_HIGH)
    {
      if (!isValid()) return false;
      if (getMachineMode() != MachineOff) return false;
      jogPins[READ_JOG_Z_POSITIVE] = positivePin;
      jogPins[READ_JOG_Z_NEGATIVE] = negativePin;
      JogPins_polarity[READ_JOG_Z_POSITIVE] = positivePolarity;
      JogPins_polarity[READ_JOG_Z_NEGATIVE] = negativePolarity;
      return true;
    }

    bool attach_EcatMpg(MyVirtualMpg &_vMpg) { return attach_EcatMpg(&_vMpg); }
    bool attach_EcatMpg(MyVirtualMpg *_vMpg)
    {
      if (!isValid()) return false;
      if (getMachineMode() != MachineOff) return false;
      vMpg = _vMpg;
      vEnc = NULL;
      return true;
    }

    bool attach_EcatMpg(MyVirtualEncoder &_vEnc) { return attach_EcatMpg(&_vEnc); }
    bool attach_EcatMpg(MyVirtualEncoder *_vEnc)
    {
      if (!isValid()) return false;
      if (getMachineMode() != MachineOff) return false;
      vMpg = NULL;
      vEnc = _vEnc;
      return true;
    }

    void machineOn() override
    {
      int i, cT;
      if (!isValid()) return;
      if (getMachineMode() != MachineOff) return;

      cT = EcatMaster.__getCycleTime__();
      config_CustomActuators_CycleTime(cT);
      EcatMaster.stop();
      for (i = AXIS_X; i <= AXIS_Z; i++)
      {
        if (vServo[i])
          servo[i] = (EthercatDevice_CiA402*)vServo[i]->getServoDevice();
        if (!isValidAxis(i)) continue;
        servo[i]->setDc(cT);
      }
      
      EcatMaster.detachCyclicCallback();
      EcatMaster.attachCyclicCallback(_EcatMachineCallback);
      EcatMaster.start(cT);
      for (i = AXIS_X; i <= AXIS_Z; i++)
      {
        if (!isValidAxis(i)) continue;
        servo[i]->setCiA402Mode(CIA402_CSP_MODE);
        servo[i]->enable();
      }
      Machine::machineOn();
    }
    
    void machineOff() override
    {
      int i;
      if (!isValid()) return;
      if (getMachineMode() == MachineOff) return;
      Machine::machineOff();
      for (i = AXIS_X; i <= AXIS_Z; i++)
      {
        if (!isValidAxis(i)) continue;
        servo[i]->disable();
      }
    }

    int homeIoHelper(homeCommand_t cmd) override
    {
      if (cmd < READ_HOME_X || cmd > READ_HOME_Z) return -1;
      if (!isValidAxis(cmd)) return -1;
      if (homePins[cmd] >= 0)
        return EVA.digitalRead(homePins[cmd]);
      else
        return Machine::homeIoHelper(cmd);
    }

    int jogIoHelper(jogCommand_t cmd) override
    {
      if (cmd < READ_JOG_X_POSITIVE || cmd > READ_JOG_Z_NEGATIVE) return -1;
      if (jogPins[cmd] >= 0)
        return EVA.digitalRead(jogPins[cmd]) == JogPins_polarity[cmd];
      else
        return Machine::jogIoHelper(cmd);
    }

    unsigned long mpgIoHelper(mpgCommand_t cmd) override
    {
      switch (cmd)
      {
        case READ_MPG_COUNTER:
          if (vMpg)
            return (unsigned long)vMpg->readRaw();
          else if (vEnc)
            return vEnc->read();
        case READ_MPG_RANGE:
          if (vMpg)
            return UINT32_MAX;
          else if (vEnc)
            return vEnc->getRange();
      }
      return Machine::mpgIoHelper(cmd);
    }

  protected:

    void setAxisPosition(int axis, long pos) override
    {
      if (isValidAxis(axis))
        servo[axis]->setTargetPosition(pos);
      else
        Machine::setAxisPosition(axis, pos);
    }

    long getAxisPosition(int axis) override
    {
      if (isValidAxis(axis))
        return servo[axis]->getPositionActualValue();
      else
        return Machine::getAxisPosition(axis);
    }

    bool isValidAxis(int axis)
    {
      return servo[axis] != NULL;
    }
    
  private:
    EthercatDevice_CiA402 *servo[__CUSTOM_MACHINE_CNT];
    MyVirtualServo *vServo[__CUSTOM_MACHINE_CNT];
    MyVirtualMpg *vMpg;
    MyVirtualEncoder *vEnc;

    int jogPins[MAX_AXIS + MAX_AXIS];
    bool jogPolarity;
    ioPolarity_t JogPins_polarity[MAX_AXIS + MAX_AXIS];
    
    int homePins[MAX_AXIS];
    unsigned long preMpgVal;
};

#endif