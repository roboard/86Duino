/*
  Motion86.cpp - DM&P Vortex86 Motion86 library
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

#include <Arduino.h>
#include <typeinfo>
#include <stdlib.h>
#include <math.h>

#include <motion/global.h>
#include <motion/g_code.h>
#include <motion/blockqueue.h>
#include <motion/stepperqueue.h>
#include <motion/planner.h>
#include <motion/interpolator.h>
#include <motion/MotInterface.h>
#include <motion/dda.h>
#include <motion/MotCustomDev.h>

#include "Motion86.h"

typedef struct {
  Machine *machine;
  MotSetting *setting;
  MotBlockQueue *bqueue;
  MotStepperQueue *squeue;
  MotInterface *motDevice;
  MotPlanner *planner;
  MotInterpolator *interpolator;
  MotGcode *g_code;
  Machine *EGear_Machine;

  int (*HomeCommandCB)(homeCommand_t cmd) = NULL;
  int (*JogCommandCB)(jogCommand_t cmd) = NULL;
  unsigned long (*MpgCommandCB)(mpgCommand_t cmd) = NULL;
  void (*setAxisPositionCB)(int axis, long pos) = NULL;
  long (*getAxisPositionCB)(int axis) = NULL;

  double homeFeedrate[MAX_AXIS];
  int homePins[MAX_AXIS];

  short jogTriggerCounter[MAX_AXIS];
  double jogVector[MAX_AXIS];
  int localJogPins[MAX_AXIS + MAX_AXIS];
  ioPolarity_t localJogPins_polarity[MAX_AXIS + MAX_AXIS];
  double incJogPerMM;

  machineAxis_t mpgAxis;
  unsigned long mpgRange;
  double mpgPreLocation;
  double mpgTmpDistance;
  double mpgRate;
  bool mpgOn;

  Encoder *localMpg;
  unsigned long jogUpdateT;
  int MC_HomeCommand(homeCommand_t cmd)
  {
    return HomeCommandCB ? HomeCommandCB(cmd) : machine->homeIoHelper(cmd);
  }

  int MC_JogCommand(jogCommand_t cmd)
  {
    return JogCommandCB ? JogCommandCB(cmd) : machine->jogIoHelper(cmd);
  }
  
  int MC_MpgCommand(mpgCommand_t cmd)
  {
    return MpgCommandCB ? MpgCommandCB(cmd) : machine->mpgIoHelper(cmd);
  }

  bool MC_JogCallback()
  {
    const int detectTimes = 3;
    int i;
    bool triggered[MAX_AXIS + MAX_AXIS];
    double location, diff;
    bool dir;
    if (jogUpdateT + 15 < timer_NowTime()) return false;
    jogUpdateT = timer_NowTime();
    switch (setting->machineMode)
    {
    case JogMode:
      for (i = READ_JOG_X_POSITIVE; i <= READ_JOG_Z_NEGATIVE; i++)
        triggered[i] = MC_JogCommand((jogCommand_t)i) == true;
      for (i = AXIS_X; i <= AXIS_Z; i++)
      {
        if (triggered[i * 2] && !triggered[i * 2 + 1])
          jogTriggerCounter[i] = jogTriggerCounter[i] < detectTimes ? jogTriggerCounter[i] + 1 : detectTimes;
        else if (!triggered[i * 2] && triggered[i * 2 + 1])
          jogTriggerCounter[i] = jogTriggerCounter[i] > -detectTimes ? jogTriggerCounter[i] - 1 : -detectTimes;
        else
          jogTriggerCounter[i] = 0;
        jogVector[i] = jogTriggerCounter[i] / detectTimes;
      }
      if (jogVector[AXIS_X] != 0.0 || jogVector[AXIS_Y] != 0.0 || jogVector[AXIS_Z] != 0.0)
      {
        return g_code->prepare_jog_move(jogVector);
      }
      break;
    case IncJogMode:
      for (i = READ_JOG_X_POSITIVE; i <= READ_JOG_Z_NEGATIVE; i++)
        triggered[i] = MC_JogCommand((jogCommand_t)i) == true;
      for (i = AXIS_X; i <= AXIS_Z; i++)
      {
        jogVector[i] = 0;
        if (triggered[i * 2] && !triggered[i * 2 + 1])
          jogTriggerCounter[i] = jogTriggerCounter[i] < detectTimes ? jogTriggerCounter[i] + 1 : detectTimes;
        else if (!triggered[i * 2] && triggered[i * 2 + 1])
          jogTriggerCounter[i] = jogTriggerCounter[i] > -detectTimes ? jogTriggerCounter[i] - 1 : -detectTimes;
        else if (!triggered[i * 2] && !triggered[i * 2 + 1] && jogTriggerCounter[i] != 0)
          jogVector[i] = incJogPerMM * (jogTriggerCounter[i] / detectTimes);
        else
          jogTriggerCounter[i] = 0;
      }
      if (jogVector[AXIS_X] != 0.0 || jogVector[AXIS_Y] != 0.0 || jogVector[AXIS_Z] != 0.0)
      {
        if (g_code->prepare_jog_move(jogVector))
        {
          for (i = AXIS_X; i <= AXIS_Z; i++)
            if (jogVector[i] != 0.0)
              jogTriggerCounter[i] = 0;
          return true;
        }
      }
      break;
    case MpgMode:
      location = MC_MpgCommand(READ_MPG_COUNTER);
      if (!mpgOn)
        mpgPreLocation = location;
      if (mpgPreLocation != location)
      {
        for (i = AXIS_X; i <= AXIS_Z; i++)
          jogVector[i] = 0;

        diff = location - mpgPreLocation;
        if (((diff >= 0) ? diff : -diff) > (mpgRange >> 1))
          dir = mpgPreLocation > location;
        else
          dir = mpgPreLocation < location;

        if (dir && location < mpgPreLocation)
          jogVector[mpgAxis] = mpgRate * (-diff - mpgRange) + mpgTmpDistance;
        else if (!dir && location > mpgPreLocation)
          jogVector[mpgAxis] = mpgRate * (-diff + mpgRange) + mpgTmpDistance;
        else
          jogVector[mpgAxis] = mpgRate * diff + mpgTmpDistance;

        mpgPreLocation = location;
        if (g_code->prepare_jog_move(jogVector))
        {
          mpgTmpDistance = 0;
          return true;
        }
        else
          mpgTmpDistance = jogVector[mpgAxis];
      }
      break;
    }
    return false;
  }
} MotionController_t;


static _HelperTask* _jogTaskRTC = NULL;
static char buffer[512];
static MotionController_t MC_Group[__TOTAL_MACHINE_CNT];
static volatile bool MC_Group_inUse[__TOTAL_MACHINE_CNT] = {false, false, false, false, false, false};

static void jog_task_handler()
{
  int i;
  for (i = 0; i < __TOTAL_MACHINE_CNT; i++)
  {
    if (MC_Group_inUse[i])
      MC_Group[i].MC_JogCallback();
  }
}

Machine::Machine(axisGroup_t axis_group)
{
  if (typeid(*this) != typeid(Machine) && axis_group < CUSTOM_MACHINE_0)
  {
    axis_group_number = MACHINE_UNUSED;
    return;
  }
  else if (axis_group >= CUSTOM_MACHINE_0 && axis_group <= CUSTOM_MACHINE_2)
  {
    axis_group_number = axis_group - CUSTOM_MACHINE_0 + __LOCAL_MACHINE_CNT;
  }
  else if (!MotSetting::groupAvalable(axis_group))
  {
    axis_group_number = MACHINE_UNUSED;
    return;
  }
  else
  {
    axis_group_number = axis_group;
  }

  if (MC_Group[axis_group_number].machine)
  {
    axis_group_number = MACHINE_UNUSED;
    return;
  }

  // init MC_Group[axis_group_number]
  MC_Group[axis_group_number].machine = this;
  MC_Group[axis_group_number].setting = new MotSetting(axis_group_number);
  MC_Group[axis_group_number].bqueue = new MotBlockQueue;
  MC_Group[axis_group_number].squeue = new MotStepperQueue;
  
  if (axis_group < CUSTOM_MACHINE_0)
    MC_Group[axis_group_number].motDevice = new MotDDA(MC_Group[axis_group_number].setting, MC_Group[axis_group_number].squeue);
  else
    MC_Group[axis_group_number].motDevice = new MotCustomDev(MC_Group[axis_group_number].machine, MC_Group[axis_group_number].setting, MC_Group[axis_group_number].squeue);

  MC_Group[axis_group_number].planner = new MotPlanner(MC_Group[axis_group_number].setting, MC_Group[axis_group_number].bqueue, MC_Group[axis_group_number].squeue, MC_Group[axis_group_number].motDevice);
  MC_Group[axis_group_number].interpolator = new MotInterpolator(MC_Group[axis_group_number].setting, MC_Group[axis_group_number].bqueue, MC_Group[axis_group_number].squeue, MC_Group[axis_group_number].motDevice);
  MC_Group[axis_group_number].g_code = new MotGcode(MC_Group[axis_group_number].setting, MC_Group[axis_group_number].bqueue, MC_Group[axis_group_number].squeue, MC_Group[axis_group_number].planner, MC_Group[axis_group_number].interpolator);
  MC_Group[axis_group_number].incJogPerMM = 1.0;
  for (int i = AXIS_X; i <= AXIS_Z; i++)
  {
    MC_Group[axis_group_number].localJogPins[i] = -1;
    MC_Group[axis_group_number].localJogPins[MAX_AXIS + i] = -1;
    MC_Group[axis_group_number].homeFeedrate[i] = 300;
    MC_Group[axis_group_number].homePins[i] = -1;
  }
  isHomePrepReady = false;
  defaultFeedrate = 600;
  config_HomeController_Callback();
  config_JogController_Callback();
  config_MpgController_Callback();
}

Machine::~Machine()
{
  if (!isValid()) return;

  // close MC_Group[axis_group_number]
  delete MC_Group[axis_group_number].g_code;
  delete MC_Group[axis_group_number].interpolator;
  delete MC_Group[axis_group_number].planner;
  delete MC_Group[axis_group_number].motDevice;
  delete MC_Group[axis_group_number].squeue;
  delete MC_Group[axis_group_number].bqueue;
  delete MC_Group[axis_group_number].setting;

  axis_group_number = MACHINE_UNUSED;
}

bool Machine::config_MaxVelocity(machineAxis_t axis, double max_feedrate_mps)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOff) return false;
  MC_Group[axis_group_number].g_code->SetAxisMaxFeedRate(axis, max_feedrate_mps);
  return true;
}

bool Machine::config_MaxAcc(double max_acc)
{
  int i;
  if (!isValid()) return false;
  if (getMachineMode() != MachineOff) return false;
  MC_Group[axis_group_number].g_code->SetAcceleration(max_acc);
  for (i = AXIS_X; i <= AXIS_Z; i++)
    MC_Group[axis_group_number].g_code->SetMotorAcceleration(i, max_acc);
  return true;
}

bool Machine::config_PosLimit(machineAxis_t axis, double min, double max)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOff) return false;
  MC_Group[axis_group_number].g_code->SetAxisLimitPos(axis, min, max);
  return true;
}

bool Machine::config_PPU(machineAxis_t axis, double steps_per_mm)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOff) return false;
  MC_Group[axis_group_number].g_code->SetAxisStepsPerMM(axis, steps_per_mm);
  return true;
}

bool Machine::config_ReverseDirection(machineAxis_t axis)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOff) return false;
  MC_Group[axis_group_number].g_code->SetMotorDirInvert(axis, !MC_Group[axis_group_number].g_code->GetMotorDirInvert(axis));
  return true;
}

bool Machine::config_MaxPulseSpeed(int pfhz)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOff) return false;
  if (MC_Group[axis_group_number].setting->SetMcmSamplecycleByPulseSpeed(pfhz) == -1) return false;
  return true;
}

bool Machine::config_PulseMode(machineAxis_t axis, pulseMode_t mode)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOff) return false;
  MC_Group[axis_group_number].setting->SetPulseMode(axis, mode);
  return true;
}

bool Machine::config_HomePins(int xLimit, int yLimit, int zLimit)
{
  int i;
  if (!isValid()) return false;
  if (getMachineMode() != MachineOff) return false;
  if (xLimit < 0 || yLimit < 0 || zLimit < 0) return false;

  MC_Group[axis_group_number].homePins[AXIS_X] = xLimit;
  MC_Group[axis_group_number].homePins[AXIS_Y] = yLimit;
  MC_Group[axis_group_number].homePins[AXIS_Z] = zLimit;
  for (i = AXIS_X; i <= AXIS_Z; i++)
    pinMode(MC_Group[axis_group_number].homePins[i], INPUT_PULLUP);
    
  config_HomeController_Callback();
  isHomePrepReady = true;
  return true;
}

bool Machine::config_EGearSlave(Machine &target, double xRatio, double yRatio, double zRatio)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOff) return false;

  if (MC_Group[target.axis_group_number].setting->machineMode != MachineOff) return false;
  if (!target.isValid()) return false;

  if (xRatio == 0 || yRatio == 0 || zRatio == 0) return false;

  MC_Group[axis_group_number].EGear_Machine = &target;

  MC_Group[axis_group_number].setting->EGearRatio[AXIS_X] = xRatio;
  MC_Group[axis_group_number].setting->EGearRatio[AXIS_Y] = yRatio;
  MC_Group[axis_group_number].setting->EGearRatio[AXIS_Z] = zRatio;

  MC_Group[axis_group_number].g_code->AttachEGear(MC_Group[target.axis_group_number].planner);

  return true;
}

bool Machine::config_HomeController_Callback(int (*cb)(homeCommand_t))
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOff) return false;
  MC_Group[axis_group_number].HomeCommandCB = cb ? cb : NULL;
  isHomePrepReady = true;
  return true;
}

bool Machine::config_JogController_Callback(int (*cb)(jogCommand_t))
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOff) return false;
  MC_Group[axis_group_number].JogCommandCB = cb ? cb : NULL;
  return true;
}

bool Machine::config_MpgController_Callback(unsigned long (*cb)(mpgCommand_t))
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOff) return false;
  MC_Group[axis_group_number].MpgCommandCB = cb ? cb : NULL;
  return true;
}


bool Machine::config_CustomActuators_Callbacks(void (*setCB)(int, long), long (*getCB)(int))
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOff) return false;
  if (!MC_Group[axis_group_number].motDevice->isType<MotCustomDev>()) return false;
  MC_Group[axis_group_number].setAxisPositionCB = setCB;
  MC_Group[axis_group_number].getAxisPositionCB = getCB;
  return true;
}

bool Machine::config_CustomActuators_CycleTime(unsigned long period)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOff) return false;
  if (!MC_Group[axis_group_number].motDevice->isType<MotCustomDev>()) return false;
  MC_Group[axis_group_number].setting->SetMotCycle(period / 10);
  return true;
}

double Machine::getMaxVelocity(machineAxis_t axis)
{
  if (!isValid()) return -1;
  return MC_Group[axis_group_number].g_code->GetAxisMaxFeedRate(axis);
}

double Machine::getMaxAcc()
{
  if (!isValid()) return -1;
  return MC_Group[axis_group_number].g_code->GetAcceleration();
}

double Machine::getPPU(machineAxis_t axis)
{
  if (!isValid()) return -1;
  return MC_Group[axis_group_number].g_code->GetAxisStepsPerMM(axis);
}

double Machine::getActualPos(machineAxis_t axis)
{
  if (!isValid()) return -1;
  return MC_Group[axis_group_number].g_code->GetCurrentPos(axis);
}

void Machine::getActualPos(double &x, double &y, double &z)
{
  if (!isValid()) return;
  double work_pos[MAX_AXIS];
  MC_Group[axis_group_number].g_code->GetCurrentPos(work_pos);
  x = work_pos[AXIS_X];
  y = work_pos[AXIS_Y];
  z = work_pos[AXIS_Z];
}

double Machine::getFeedrateOverride()
{
  if (!isValid()) return -1;
  return MC_Group[axis_group_number].g_code->GetFeedrateRatio();
}

pulseMode_t Machine::getPulseMode(machineAxis_t axis)
{
  if (!isValid()) return MODE_NOSET;
  return MC_Group[axis_group_number].setting->GetPulseMode(axis);
}

int Machine::getCmdCount()
{
  if (!isValid()) return -1;
  return MC_Group[axis_group_number].g_code->getBlockCount();
}

int Machine::getFreeCmdCount()
{
  if (!isValid()) return -1;
  return MC_Group[axis_group_number].g_code->getFreeBlockCount();
}

void Machine::setDefaultFeedrate(double feedrate)
{
  if (!isValid()) return;
  if (feedrate > 0)
    defaultFeedrate = feedrate;
}

void Machine::setHomeSpeed(double feedrate)
{
  if (!isValid()) return;
  setHomeSpeed(feedrate, feedrate, feedrate);
}

void Machine::setHomeSpeed(double xFeedrate, double yFeedrate, double zFeedrate)
{
  if (!isValid()) return;
  MC_Group[axis_group_number].homeFeedrate[AXIS_X] = xFeedrate;
  MC_Group[axis_group_number].homeFeedrate[AXIS_Y] = yFeedrate;
  MC_Group[axis_group_number].homeFeedrate[AXIS_Z] = zFeedrate;
}

void Machine::setRelative()
{
  if (!isValid()) return;
  gcode("G91");
}

void Machine::setAbsolute()
{
  if (!isValid()) return;
  gcode("G90");
}

double Machine::computePPU_Belt(double pulses_per_revolution, double belt_pitch, double pulley_tooth_count)
{
  if (belt_pitch <= 0 || pulley_tooth_count <= 0)
    return -1;
  return pulses_per_revolution / (belt_pitch * pulley_tooth_count);
}

double Machine::computePPU_LeadScrew(double pulses_per_revolution, double leadscrew_pitch, double gear_ratio)
{
  if (leadscrew_pitch <= 0 || gear_ratio <= 0)
    return -1;
  return (pulses_per_revolution * gear_ratio) / leadscrew_pitch;
}

bool Machine::isDirectionReversed(machineAxis_t axis)
{
  if (!isValid()) return false;
  return MC_Group[axis_group_number].g_code->GetMotorDirInvert(axis) == 1;
}

bool Machine::isMoving()
{
  if (!isValid()) return false;
  return !MC_Group[axis_group_number].g_code->IsFinishMove();
}

bool Machine::isValid()
{
  return axis_group_number != MACHINE_UNUSED;
}

bool Machine::isCmdBufferFull()
{
  if (!isValid()) return false;
  return MC_Group[axis_group_number].g_code->IsBlockFull();
}

void Machine::enableSoftLimit()
{
  if (!isValid()) return;
  MC_Group[axis_group_number].g_code->SetSoftLimit(1);
}

void Machine::disableSoftLimit()
{
  if (!isValid()) return;
  MC_Group[axis_group_number].g_code->SetSoftLimit(0);
}

void Machine::machineOn()
{
  int i;
  bool isUse = false;
  if (!isValid()) return;
  if (getMachineMode() != MachineOff) return;
  for (i = 0; i < __TOTAL_MACHINE_CNT; i++)
    isUse |= MC_Group_inUse[i];
  if (!isUse)
  {
    MotInterpolator::AttachInterrupt();

    if (_jogTaskRTC == NULL)
    {
      _jogTaskRTC = myHelper.startLoop(100 * 1024);
      if (_jogTaskRTC == NULL) return;
      myHelper.start(0);
      _jogTaskRTC->registerFunc(jog_task_handler);
      _jogTaskRTC->start();
    }
  }
  MC_Group[axis_group_number].motDevice->AttachIfIdle();
  MC_Group[axis_group_number].motDevice->Init();
	  
  
  io_DisableINT();
  MC_Group_inUse[axis_group_number] = true;
  io_RestoreINT();
  if (MC_Group[axis_group_number].motDevice->isType<MotDDA>())
  {
#ifdef __86DUINO_EX
	  if (axis_group_number == 0)
	  {
		io_outpb(CROSSBARBASE + 0x90 + 16, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + 17, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + 18, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + 19, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + 20, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + 21, 0x08);
	  }
	  else if (axis_group_number == 1)
	  {
		io_outpb(CROSSBARBASE + 0x90 + 24, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + 25, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + 26, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + 27, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + 28, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + 29, 0x08);
	  }
	  else if (axis_group_number == 2)
	  {
		io_outpb(CROSSBARBASE + 0x90 + 0, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + 1, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + 2, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + 3, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + 4, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + 5, 0x08);
	  }
#elif defined __86DUINO_EX2
	  if (axis_group_number == 0)
	  {
		io_outpb(CROSSBARBASE + 0x100 + 0, 0x01);
		io_outpb(CROSSBARBASE + 0x100 + 1, 0x01);
		io_outpb(CROSSBARBASE + 0x100 + 2, 0x01);
		io_outpb(CROSSBARBASE + 0x100 + 3, 0x01);
		io_outpb(CROSSBARBASE + 0x100 + 4, 0x01);
		io_outpb(CROSSBARBASE + 0x100 + 5, 0x01);
	  }
#endif
  }
  MC_Group[axis_group_number].setting->machineMode = MachineOn;

  if (MC_Group[axis_group_number].g_code->isEGearAttached())
    MC_Group[axis_group_number].EGear_Machine->machineOn();
}

void Machine::machineOff()
{
  int i;
  bool isUse = false;
  if (!isValid()) return;
  if (getMachineMode() == MachineOff) return;
  if (!MC_Group_inUse[axis_group_number]) return;
  while (isMoving())
  {
    helperYield();
    yield();
  }
  MC_Group[axis_group_number].setting->machineMode = MachineOff;
  
  if (MC_Group[axis_group_number].motDevice->isType<MotDDA>())
  {
#ifdef __86DUINO_EX
	  if (axis_group_number == 0)
	  {
		io_outpb(CROSSBARBASE + 0x90 + 16, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + 17, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + 18, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + 19, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + 20, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + 21, 0x01);
	  }
	  else if (axis_group_number == 1)
	  {
		io_outpb(CROSSBARBASE + 0x90 + 24, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + 25, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + 26, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + 27, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + 28, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + 29, 0x01);
	  }
	  else if (axis_group_number == 2)
	  {
		io_outpb(CROSSBARBASE + 0x90 + 0, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + 1, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + 2, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + 3, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + 4, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + 5, 0x01);
	  }
#elif defined __86DUINO_EX2
	  if (axis_group_number == 0)
	  {
		io_outpb(CROSSBARBASE + 0x100 + 0, 0x08);
		io_outpb(CROSSBARBASE + 0x100 + 1, 0x08);
		io_outpb(CROSSBARBASE + 0x100 + 2, 0x08);
		io_outpb(CROSSBARBASE + 0x100 + 3, 0x08);
		io_outpb(CROSSBARBASE + 0x100 + 4, 0x08);
		io_outpb(CROSSBARBASE + 0x100 + 5, 0x08);
	  }
#endif
  }
  if (MC_Group[axis_group_number].g_code->isEGearAttached())
    MC_Group[axis_group_number].EGear_Machine->machineOff();

  io_DisableINT();
  MC_Group_inUse[axis_group_number] = false;
  io_RestoreINT();

  MC_Group[axis_group_number].motDevice->DetachIfIdle();
  
  for (i = 0; i < __TOTAL_MACHINE_CNT; i++)
    isUse |= MC_Group_inUse[i];
  if (!isUse)
  {
    MotInterpolator::DetachInterrupt();
  }
}

bool Machine::gcode(char *cmd)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOn) return false;
  MC_Group[axis_group_number].g_code->process_commands(cmd);
  return true;
}

void Machine::home()
{
  int i, j, cmd;
  bool relativeMode = MC_Group[axis_group_number].g_code->IsRelativeMode();
  bool softLimitMode = MC_Group[axis_group_number].g_code->GetSoftLimitMode();
  double move[3] = { -100000, 5, -10 };
  MotPlanner* EGear_p = NULL;

  if (!isValid()) return;
  if (getMachineMode() != MachineOn) return;

  if (MC_Group[axis_group_number].g_code->isEGearAttached())
  {
    EGear_p = MC_Group[MC_Group[axis_group_number].EGear_Machine->axis_group_number].planner;
    MC_Group[axis_group_number].g_code->DettachEGear();
  }
  while (isMoving())
  {
    helperYield();
    yield();
  }
  
  if (isHomePrepReady)
  {
    setRelative();
    disableSoftLimit();

    for (i = READ_HOME_X; i <= READ_HOME_Z; i++)
    {
      for (j = 0; j < 3; j++)
      {
        line(i == READ_HOME_X ? move[j] : 0,
             i == READ_HOME_Y ? move[j] : 0,
             i == READ_HOME_Z ? move[j] : 0, MC_Group[axis_group_number].homeFeedrate[i]);
        while (isMoving())
        {
          cmd = MC_Group[axis_group_number].MC_HomeCommand((homeCommand_t)i);
          if (cmd && j != 1)
          {
            if (cmd < 0) j = 3;
            stop();
          }
          helperYield();
          yield();
        }
      }
    }
    gcode("G92 X0 Y0 Z0");
    if (softLimitMode) enableSoftLimit();
    if (!relativeMode) setAbsolute();
  }
  else
  {
    gcode("G92 X0 Y0 Z0");
  }

  if (EGear_p != NULL)
  {
    MC_Group[axis_group_number].EGear_Machine->home();
    MC_Group[axis_group_number].g_code->AttachEGear(EGear_p);
  }
}

void Machine::stop()
{
  if (!isValid()) return;
  if (getMachineMode() == MachineOff) return;
  emgStop();
  clearEMGStop();
}

void Machine::emgStop()
{
  if (!isValid()) return;
  if (getMachineMode() == MachineOff) return;
  MC_Group[axis_group_number].g_code->SetEMGStatus(true);
  if (MC_Group[axis_group_number].g_code->isEGearAttached())
    MC_Group[MC_Group[axis_group_number].EGear_Machine->axis_group_number].g_code->SetEMGStatus(true);
}

void Machine::clearEMGStop()
{
  if (!isValid()) return;
  if (getMachineMode() == MachineOff) return;
  MC_Group[axis_group_number].g_code->SetEMGStatus(false);
  if (MC_Group[axis_group_number].g_code->isEGearAttached())
    MC_Group[MC_Group[axis_group_number].EGear_Machine->axis_group_number].g_code->SetEMGStatus(false);
}

void Machine::feedrateOverride(double ratio)
{
  if (!isValid()) return;
  if (getMachineMode() == MachineOff) return;
  MC_Group[axis_group_number].g_code->SetFeedrateRatio(ratio);
}

bool Machine::line(double x, double y, double z, double feedrate)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOn) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  sprintf(buffer, "G1 X%.5f Y%.5f Z%.5f F%.5f", x, y, z, defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::arcXY(double cX, double cY, double dstX, double dstY, bool revDir, double feedrate)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOn) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G17");
  sprintf(buffer, "G%d X%.5f Y%.5f I%.5f J%.5f F%.5f", revDir ? 2 : 3, dstX, dstY, cX, cY, defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::arcXZ(double cX, double cZ, double dstX, double dstZ, bool revDir, double feedrate)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOn) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G18");
  sprintf(buffer, "G%d X%.5f Z%.5f I%.5f K%.5f F%.5f", revDir ? 2 : 3, dstX, dstZ, cX, cZ, defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::arcYZ(double cY, double cZ, double dstY, double dstZ, bool revDir, double feedrate)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOn) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G19");
  sprintf(buffer, "G%d Y%.5f Z%.5f J%.5f K%.5f F%.5f", revDir ? 2 : 3, dstY, dstZ, cY, cZ, defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::arcXY(double r, double dstX, double dstY, bool revDir, double feedrate)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOn) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G17");
  sprintf(buffer, "G%d X%.5f Y%.5f R%.5f F%.5f", revDir ? 2 : 3, dstX, dstY, r, defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::arcXZ(double r, double dstX, double dstZ, bool revDir, double feedrate)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOn) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G18");
  sprintf(buffer, "G%d X%.5f Z%.5f R%.5f F%.5f", revDir ? 2 : 3, dstX, dstZ, r, defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::arcYZ(double r, double dstY, double dstZ, bool revDir, double feedrate)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOn) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G19");
  sprintf(buffer, "G%d Y%.5f Z%.5f R%.5f F%.5f", revDir ? 2 : 3, dstY, dstZ, r, defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::arcXY_Theta(double cX, double cY, double theta, double feedrate)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOn) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G17");
  double dstX, dstY;
  MC_Group[axis_group_number].g_code->ComputeDst(cX, cY, theta, &dstX, &dstY);
  sprintf(buffer, "G%d X%.5f Y%.5f I%.5f J%.5f P%d F%.5f", theta < 0 ? 2 : 3, dstX, dstY, cX, cY, (int)ceil(fabs(theta) / (M_PI * 2)), defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::arcXZ_Theta(double cX, double cZ, double theta, double feedrate)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOn) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G18");
  double dstX, dstZ;
  MC_Group[axis_group_number].g_code->ComputeDst(cX, cZ, theta, &dstX, &dstZ);
  sprintf(buffer, "G%d X%.5f Z%.5f I%.5f K%.5f P%d F%.5f", theta < 0 ? 2 : 3, dstX, dstZ, cX, cZ, (int)ceil(fabs(theta) / (M_PI * 2)), defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::arcYZ_Theta(double cY, double cZ, double theta, double feedrate)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOn) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G19");
  double dstY, dstZ;
  MC_Group[axis_group_number].g_code->ComputeDst(cY, cZ, theta, &dstY, &dstZ);
  sprintf(buffer, "G%d Y%.5f Z%.5f J%.5f K%.5f P%d F%.5f", theta < 0 ? 2 : 3, dstY, dstZ, cY, cZ, (int)ceil(fabs(theta) / (M_PI * 2)), defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::circleXY(double cX, double cY, bool revDir, double feedrate)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOn) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G17");
  double dstX, dstY;
  MC_Group[axis_group_number].g_code->ComputeDst(cX, cY, M_PI * 2, &dstX, &dstY);
  sprintf(buffer, "G%d X%.5f Y%.5f I%.5f J%.5f F%.5f", revDir ? 2 : 3, dstX, dstY, cX, cY, defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::circleXZ(double cX, double cZ, bool revDir, double feedrate)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOn) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G18");
  double dstX, dstZ;
  MC_Group[axis_group_number].g_code->ComputeDst(cX, cZ, M_PI * 2, &dstX, &dstZ);
  sprintf(buffer, "G%d X%.5f Z%.5f I%.5f K%.5f F%.5f", revDir ? 2 : 3, dstX, dstZ, cX, cZ, defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::circleYZ(double cY, double cZ, bool revDir, double feedrate)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOn) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G19");
  double dstY, dstZ;
  MC_Group[axis_group_number].g_code->ComputeDst(cY, cZ, M_PI * 2, &dstY, &dstZ);
  sprintf(buffer, "G%d Y%.5f Z%.5f J%.5f K%.5f F%.5f", revDir ? 2 : 3, dstY, dstZ, cY, cZ, defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::helicalXY(double cX, double cY, double dstZ, double theta, double feedrate)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOn) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G17");
  double dstX, dstY;
  MC_Group[axis_group_number].g_code->ComputeDst(cX, cY, theta, &dstX, &dstY);
  sprintf(buffer, "G%d X%.5f Y%.5f Z%.5f I%.5f J%.5f P%d F%.5f", theta < 0 ? 2 : 3, dstX, dstY, dstZ, cX, cY, (int)ceil(fabs(theta) / (M_PI * 2)), defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::helicalXZ(double cX, double cZ, double dstY, double theta, double feedrate)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOn) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G18");
  double dstX, dstZ;
  MC_Group[axis_group_number].g_code->ComputeDst(cX, cZ, theta, &dstX, &dstZ);
  sprintf(buffer, "G%d X%.5f Y%.5f Z%.5f I%.5f K%.5f P%d F%.5f", theta < 0 ? 2 : 3, dstX, dstY, dstZ, cX, cZ, (int)ceil(fabs(theta) / (M_PI * 2)), defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::helicalYZ(double cY, double cZ, double dstX, double theta, double feedrate)
{
  if (!isValid()) return false;
  if (getMachineMode() != MachineOn) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G19");
  double dstY, dstZ;
  MC_Group[axis_group_number].g_code->ComputeDst(cY, cZ, theta, &dstY, &dstZ);
  sprintf(buffer, "G%d X%.5f Y%.5f Z%.5f J%.5f K%.5f P%d F%.5f", theta < 0 ? 2 : 3, dstX, dstY, dstZ, cY, cZ, (int)ceil(fabs(theta) / (M_PI * 2)), defaultFeedrate);
  gcode(buffer);
  return true;
}

void Machine::beginJog(bool incJog)
{
  int i;
  if (!isValid()) return;
  if (getMachineMode() != MachineOn) return;
  while (isMoving())
  {
    helperYield();
    yield();
  }

  for (i = AXIS_X; i <= AXIS_Z; i++)
  {
    MC_Group[axis_group_number].jogTriggerCounter[i] = 0;
    MC_Group[axis_group_number].jogVector[i] = 0;
  }
  MC_Group[axis_group_number].jogUpdateT = timer_NowTime();
  if (incJog)
    MC_Group[axis_group_number].setting->machineMode = IncJogMode;
  else
    MC_Group[axis_group_number].setting->machineMode = JogMode;
}

void Machine::beginJog(int pX, int nX, int pY, int nY, int pZ, int nZ, bool incJog, ioPolarity_t polarity)
{
  if (!isValid()) return;
  if (getMachineMode() != MachineOn) return;
  
  setJogPins(pX, nX, pY, nY, pZ, nZ, polarity);
  config_JogController_Callback();
  beginJog(incJog);
}

void Machine::endJog()
{
  if (!isValid()) return;
  if (getMachineMode() != IncJogMode && getMachineMode() != JogMode) return;
  while (isMoving())
  {
    helperYield();
    yield();
  }
  MC_Group[axis_group_number].setting->machineMode = MachineOn;
}

void Machine::setJogSpeed(double feedrate)
{
  if (!isValid()) return;
  MC_Group[axis_group_number].g_code->SetJogFeedrate(feedrate);
}

void Machine::setJogOffset(double incPerMM)
{
  if (!isValid()) return;
  if (incPerMM != 0)
    MC_Group[axis_group_number].incJogPerMM = incPerMM;
}

void Machine::setJogPins(int pX, int nX, int pY, int nY, int pZ, int nZ, ioPolarity_t polarity)
{
  int i;
  if (!isValid()) return;
  MC_Group[axis_group_number].localJogPins[READ_JOG_X_POSITIVE] = pX & 0x00FF;
  MC_Group[axis_group_number].localJogPins[READ_JOG_X_NEGATIVE] = nX & 0x00FF;
  MC_Group[axis_group_number].localJogPins[READ_JOG_Y_POSITIVE] = pY & 0x00FF;
  MC_Group[axis_group_number].localJogPins[READ_JOG_Y_NEGATIVE] = nY & 0x00FF;
  MC_Group[axis_group_number].localJogPins[READ_JOG_Z_POSITIVE] = pZ & 0x00FF;
  MC_Group[axis_group_number].localJogPins[READ_JOG_Z_NEGATIVE] = nZ & 0x00FF;

  for(i = READ_JOG_X_POSITIVE; i <= READ_JOG_Z_NEGATIVE; i++)
  {
    MC_Group[axis_group_number].localJogPins_polarity[i] = polarity;
    if(MC_Group[axis_group_number].localJogPins[i] >= 0)
      pinMode(MC_Group[axis_group_number].localJogPins[i], INPUT_PULLUP);
  }
}

void Machine::beginMpg()
{
  if (!isValid()) return;
  if (getMachineMode() != MachineOn) return;
  while (isMoving())
  {
    helperYield();
    yield();
  }
  MC_Group[axis_group_number].mpgAxis = AXIS_X;
  MC_Group[axis_group_number].mpgRate = 1.0;
  MC_Group[axis_group_number].mpgRange = MC_Group[axis_group_number].MC_MpgCommand(READ_MPG_RANGE);
  MC_Group[axis_group_number].mpgPreLocation = MC_Group[axis_group_number].MC_MpgCommand(READ_MPG_COUNTER);
  MC_Group[axis_group_number].mpgTmpDistance = 0; 
  
  MC_Group[axis_group_number].jogUpdateT = timer_NowTime();
  MC_Group[axis_group_number].setting->machineMode = MpgMode;
  
  setMpgOn();
}

void Machine::beginMpg(Encoder &encoder)
{
  if (!isValid()) return;
  if (getMachineMode() != MachineOn) return;
  
  MC_Group[axis_group_number].localMpg = &encoder;
  config_MpgController_Callback();

  beginMpg();
}

void Machine::endMpg()
{
  if (!isValid()) return;
  if (getMachineMode() != MpgMode) return;
  while (isMoving())
  {
    helperYield();
    yield();
  }
  MC_Group[axis_group_number].localMpg = NULL;
  MC_Group[axis_group_number].setting->machineMode = MachineOn;
}

void Machine::setMpgOn()
{
  if (!isValid()) return;
  MC_Group[axis_group_number].mpgOn = true;
}

void Machine::setMpgOff()
{
  if (!isValid()) return;
  MC_Group[axis_group_number].mpgOn = false;
}

void Machine::setMpgSpeed(double feedrate)
{
  if (!isValid()) return;
  MC_Group[axis_group_number].g_code->SetJogFeedrate(feedrate);
}

void Machine::setMpgAxis(machineAxis_t axis)
{
  if (!isValid()) return;
  if (axis < AXIS_X || axis > AXIS_Z) return;
  MC_Group[axis_group_number].mpgAxis = axis;
}

void Machine::setMpgRatio(double rate)
{
  if (!isValid()) return;
  if (rate > 0)
    MC_Group[axis_group_number].mpgRate = rate;
}

double Machine::getJogPos(machineAxis_t axis)
{
  if (!isValid()) return -1;
  if (getMachineMode() != JogMode && getMachineMode() != IncJogMode && getMachineMode() != MpgMode) return -1;
  return MC_Group[axis_group_number].g_code->GetJogPos(axis);
}

void Machine::getJogPos(double &x, double &y, double &z)
{
  if (!isValid()) return;
  if (getMachineMode() != JogMode && getMachineMode() != IncJogMode && getMachineMode() != MpgMode) return;
  double jog_pos[MAX_AXIS];
  MC_Group[axis_group_number].g_code->GetJogPos(jog_pos);
  x = jog_pos[AXIS_X];
  y = jog_pos[AXIS_Y];
  z = jog_pos[AXIS_Z];
}

void Machine::updateAxes()
{
  if (!isValid()) return;
  if (getMachineMode() == MachineOff) return;
  MC_Group[axis_group_number].motDevice->Update();
}

int Machine::homeIoHelper(homeCommand_t cmd)
{
  if (cmd < READ_HOME_X || cmd > READ_HOME_Z) return -1;
  if (MC_Group[axis_group_number].homePins[cmd] < 0) return -1;
  return digitalRead(MC_Group[axis_group_number].homePins[cmd]) == 0;
}

int Machine::jogIoHelper(jogCommand_t cmd)
{
  if (cmd < READ_JOG_X_POSITIVE || cmd > READ_JOG_Z_NEGATIVE) return -1;
  if (MC_Group[axis_group_number].localJogPins[cmd] < 0) return -1;
  return digitalRead(MC_Group[axis_group_number].localJogPins[cmd]) == MC_Group[axis_group_number].localJogPins_polarity[cmd];
}

unsigned long Machine::mpgIoHelper(mpgCommand_t cmd)
{
  if (!MC_Group[axis_group_number].localMpg) return 0;
  if (cmd == READ_MPG_COUNTER)
    return MC_Group[axis_group_number].localMpg->read();
  if (cmd == READ_MPG_RANGE)
    return MC_Group[axis_group_number].localMpg->readRange();
  return 0;
}

void Machine::setAxisPosition(int axis, long pos)
{
  if (!MC_Group[axis_group_number].setAxisPositionCB) return;
  MC_Group[axis_group_number].setAxisPositionCB(axis, pos);
}

long Machine::getAxisPosition(int axis)
{
  if (!MC_Group[axis_group_number].getAxisPositionCB) return 0;
  return MC_Group[axis_group_number].getAxisPositionCB(axis);
}

machineMode_t Machine::getMachineMode()
{
  if (!isValid()) return EnvalidMachine;
  return MC_Group[axis_group_number].setting->machineMode;
}
    