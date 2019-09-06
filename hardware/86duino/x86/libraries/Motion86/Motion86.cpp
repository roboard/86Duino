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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "motion/g_code.h" // for Motion86
#include "motion/global.h" // for Motion86
#include "motion/blockqueue.h" // for Motion86
#include "motion/stepperqueue.h" // for Motion86
#include "motion/planner.h" // for Motion86
#include "motion/interpolator.h" // for Motion86
#include "motion/dda.h" // for Motion86

#include "Motion86.h"

typedef struct {
  MotSetting *setting;
  MotBlockQueue *bqueue;
  MotStepperQueue *squeue;
  MotDDA *dda;
  MotPlanner *planner;
  MotInterpolator *interpolator;
  MotGcode *g_code;
  Machine *EGear_Machine;
} MotionController_t;


static char buffer[512];
static MotionController_t MC_Group[3];
static volatile bool MC_Group_inUse[3] = {false, false, false};

static int jogTrigger[3][6];
static Encoder *mpgTrigger[3] = { NULL, NULL, NULL };

static short jogCount[3][3];
static double jogVector[3][3];

static double incJogPerMM[3] = {1, 1, 1};

static machineAxis_t mpgAxis[3];
static double mpgRate[3];
static double mpgPreLocation[3];
static double mpgDistance[3] = {0, 0 , 0};
static const int detectTimes = 3;

bool jogCallback(int axis_group)
{
  int i;
  if (MC_Group[axis_group].setting->machineMode == JogMode)
  {
    for (i = AXIS_X; i <= AXIS_Z; i++)
    {
      if (digitalRead(jogTrigger[axis_group][i * 2]) == LOW && digitalRead(jogTrigger[axis_group][i * 2 + 1]) == HIGH)
        jogCount[axis_group][i] = jogCount[axis_group][i] < detectTimes ? jogCount[axis_group][i] + 1 : detectTimes;
      else if (digitalRead(jogTrigger[axis_group][i * 2]) == HIGH && digitalRead(jogTrigger[axis_group][i * 2 + 1]) == LOW)
        jogCount[axis_group][i] = jogCount[axis_group][i] > -detectTimes ? jogCount[axis_group][i] - 1 : -detectTimes;
      else
        jogCount[axis_group][i] = 0;
      jogVector[axis_group][i] = (jogCount[axis_group][i] / detectTimes);
    }
    if (jogVector[axis_group][AXIS_X] != 0.0 || jogVector[axis_group][AXIS_Y] != 0.0 || jogVector[axis_group][AXIS_Z] != 0.0)
      return MC_Group[axis_group].g_code->prepare_jog_move(jogVector[axis_group]);
  }
  else if (MC_Group[axis_group].setting->machineMode == IncJogMode)
  {
    for (i = AXIS_X; i <= AXIS_Z; i++)
    {
      jogVector[axis_group][i] = 0;
      if (digitalRead(jogTrigger[axis_group][i * 2]) == LOW && digitalRead(jogTrigger[axis_group][i * 2 + 1]) == HIGH)
        jogCount[axis_group][i] = jogCount[axis_group][i] < detectTimes ? jogCount[axis_group][i] + 1 : detectTimes;
      else if (digitalRead(jogTrigger[axis_group][i * 2]) == HIGH && digitalRead(jogTrigger[axis_group][i * 2 + 1]) == LOW)
        jogCount[axis_group][i] = jogCount[axis_group][i] > -detectTimes ? jogCount[axis_group][i] - 1 : -detectTimes;
      else if (digitalRead(jogTrigger[axis_group][i * 2]) == HIGH && digitalRead(jogTrigger[axis_group][i * 2 + 1]) == HIGH && jogCount[axis_group][i] != 0)
        jogVector[axis_group][i] = incJogPerMM[axis_group] * (jogCount[axis_group][i] / detectTimes);
      else
        jogCount[axis_group][i] = 0;
    }
    if (jogVector[axis_group][AXIS_X] != 0.0 || jogVector[axis_group][AXIS_Y] != 0.0 || jogVector[axis_group][AXIS_Z] != 0.0)
      if (MC_Group[axis_group].g_code->prepare_jog_move(jogVector[axis_group]))
      {
        for (i = AXIS_X; i <= AXIS_Z; i++)
          if (jogVector[axis_group][i] != 0.0)
            jogCount[axis_group][i] = 0;
        return true;
      }
  }
  else if (MC_Group[axis_group].setting->machineMode == MpgMode)
  {
    unsigned long mpgRange = mpgTrigger[axis_group]->readRange();
    double location = (double)mpgTrigger[axis_group]->read();
    int dir = mpgTrigger[axis_group]->directionRead();

    if (mpgPreLocation[axis_group] != location)
    {
      for (i = AXIS_X; i <= AXIS_Z; i++)
        jogVector[axis_group][i] = 0;

      if (dir > 0 && location < mpgPreLocation[axis_group])
        jogVector[axis_group][mpgAxis[axis_group]] = mpgRate[axis_group] * (location - (mpgRange - mpgPreLocation[axis_group])) + mpgDistance[axis_group];
      else if (dir < 0 && location > mpgPreLocation[axis_group])
        jogVector[axis_group][mpgAxis[axis_group]] = mpgRate[axis_group] * ((mpgRange - location) - mpgPreLocation[axis_group]) + mpgDistance[axis_group];
      else
        jogVector[axis_group][mpgAxis[axis_group]] = mpgRate[axis_group] * (location - mpgPreLocation[axis_group]) + mpgDistance[axis_group];

      mpgPreLocation[axis_group] = location;
      if (MC_Group[axis_group].g_code->prepare_jog_move(jogVector[axis_group]))
      {
        mpgDistance[axis_group] = 0;
        return true;
      }
      else
        mpgDistance[axis_group] = jogVector[axis_group][mpgAxis[axis_group]];
    }
  }
  return false;
}

Machine::Machine(int axis_group)
{
  if (axis_group < AXIS_X || axis_group > AXIS_Z)
  {
    axis_group_number = AXIS_UNUSED;
    return;
  }

  if (MC_Group_inUse[axis_group] == true)
  {
    axis_group_number = AXIS_UNUSED;
    return;
  }

  defaultFeedrate = 600;
  for (int i = AXIS_X; i <= AXIS_Z; i++)
    homeFeedrate[i] = 300;

  // init MC_Group[axis_group]
  MC_Group[axis_group].setting = new MotSetting(axis_group);
  MC_Group[axis_group].bqueue = new MotBlockQueue;
  MC_Group[axis_group].squeue = new MotStepperQueue;
  MC_Group[axis_group].dda = new MotDDA(MC_Group[axis_group].setting, MC_Group[axis_group].squeue);
  MC_Group[axis_group].planner = new MotPlanner(MC_Group[axis_group].setting, MC_Group[axis_group].bqueue, MC_Group[axis_group].dda);
  MC_Group[axis_group].interpolator = new MotInterpolator(MC_Group[axis_group].setting, MC_Group[axis_group].bqueue, MC_Group[axis_group].squeue, MC_Group[axis_group].dda);
  MC_Group[axis_group].g_code = new MotGcode(MC_Group[axis_group].setting, MC_Group[axis_group].bqueue, MC_Group[axis_group].squeue, MC_Group[axis_group].planner, MC_Group[axis_group].interpolator, MC_Group[axis_group].dda);

  axis_group_number = axis_group;
}

Machine::~Machine()
{
  if (!isValid()) return;

  // close MC_Group[axis_group_number]
  delete MC_Group[axis_group_number].g_code;
  delete MC_Group[axis_group_number].interpolator;
  delete MC_Group[axis_group_number].planner;
  delete MC_Group[axis_group_number].dda;
  delete MC_Group[axis_group_number].squeue;
  delete MC_Group[axis_group_number].bqueue;
  delete MC_Group[axis_group_number].setting;

  axis_group_number = AXIS_UNUSED;
}

bool Machine::config_MaxVelocity(machineAxis_t axis, double max_feedrate_mps)
{
  if (MC_Group[axis_group_number].setting->machineMode != MachineOff) return false;
  if (!isValid()) return false;
  MC_Group[axis_group_number].g_code->SetAxisMaxFeedRate(axis, max_feedrate_mps);
  return true;
}

bool Machine::config_MaxAcc(double vel)
{
  if (MC_Group[axis_group_number].setting->machineMode != MachineOff) return false;
  if (!isValid()) return false;
  MC_Group[axis_group_number].g_code->SetMotorAcceleration(vel);
  return true;
}

bool Machine::config_PosLimit(machineAxis_t axis, int min, int max)
{
  if (MC_Group[axis_group_number].setting->machineMode != MachineOff) return false;
  if (!isValid()) return false;
  MC_Group[axis_group_number].g_code->SetAxisLimitPos(axis, min, max);
  return true;
}

bool Machine::config_PPU(machineAxis_t axis, double steps_per_mm)
{
  if (MC_Group[axis_group_number].setting->machineMode != MachineOff) return false;
  if (!isValid()) return false;
  MC_Group[axis_group_number].g_code->SetAxisStepsPerMM(axis, steps_per_mm);
  return true;
}

bool Machine::config_ReverseDirection(machineAxis_t axis)
{
  if (MC_Group[axis_group_number].setting->machineMode != MachineOff) return false;
  if (!isValid()) return false;
  MC_Group[axis_group_number].g_code->SetMotorDirInvert(axis, !MC_Group[axis_group_number].g_code->GetMotorDirInvert(axis));
  return true;
}

bool Machine::config_MaxPulseSpeed(int pfhz)
{
  if (MC_Group[axis_group_number].setting->machineMode != MachineOff) return false;
  if (!isValid()) return false;
  if (MC_Group[axis_group_number].setting->SetNcmSamplecycleByPulseSpeed(pfhz) == -1) return false;
  return true;
}

bool Machine::config_PulseMode(machineAxis_t axis, pulseMode_t mode)
{
  if (MC_Group[axis_group_number].setting->machineMode != MachineOff) return false;
  if (!isValid()) return false;
  MC_Group[axis_group_number].setting->SetPulseMode(axis, mode);
}

bool Machine::config_HomePins(int xLimit, int yLimit, int zLimit)
{
  if (MC_Group[axis_group_number].setting->machineMode != MachineOff) return false;
  if (!isValid()) return false;
  MC_Group[axis_group_number].setting->homeLimit[0] = xLimit;
  MC_Group[axis_group_number].setting->homeLimit[1] = yLimit;
  MC_Group[axis_group_number].setting->homeLimit[2] = zLimit;
}

bool Machine::config_EGearSlave(Machine &target, double xRatio, double yRatio, double zRatio)
{
  if (MC_Group[axis_group_number].setting->machineMode != MachineOff) return false;
  if (!isValid()) return false;

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

double Machine::getMaxVelocity(machineAxis_t axis)
{
  if (!isValid()) return -1;
  if (!MC_Group_inUse[axis_group_number]) return -1;
  return MC_Group[axis_group_number].g_code->GetAxisMaxFeedRate(axis);
}

double Machine::getMaxAcc()
{
  if (!isValid()) return -1;
  if (!MC_Group_inUse[axis_group_number]) return -1;
  return MC_Group[axis_group_number].g_code->GetMotorAcceleration();
}

double Machine::getPPU(machineAxis_t axis)
{
  if (!isValid()) return -1;
  if (!MC_Group_inUse[axis_group_number]) return -1;
  return MC_Group[axis_group_number].g_code->GetAxisStepsPerMM(axis);
}

double Machine::getActualPos(machineAxis_t axis)
{
  if (!isValid()) return -1;
  if (!MC_Group_inUse[axis_group_number]) return -1;
  return MC_Group[axis_group_number].g_code->GetCurrentPos(axis);
}

void Machine::getActualPos(double &x, double &y, double &z)
{
  if (!isValid()) return;
  if (!MC_Group_inUse[axis_group_number]) return;
  double work_pos[MAX_AXIS];
  MC_Group[axis_group_number].g_code->GetCurrentPos(work_pos);
  x = work_pos[AXIS_X];
  y = work_pos[AXIS_Y];
  z = work_pos[AXIS_Z];
}

double Machine::getFeedrateOverride()
{
  return MC_Group[axis_group_number].g_code->GetFeedrateRatio();
}

pulseMode_t Machine::getPulseMode(machineAxis_t axis)
{
  MC_Group[axis_group_number].setting->GetPulseMode(axis);
}

int Machine::getCmdCount()
{
  return MC_Group[axis_group_number].g_code->getBlockCount();
}

int Machine::getFreeCmdCount()
{
  return MC_Group[axis_group_number].g_code->getFreeBlockCount();
}

void Machine::setDefaultFeedrate(double feedrate)
{
	if (!isValid()) return;
	if (!MC_Group_inUse[axis_group_number]) return;
	if (feedrate > 0)
		defaultFeedrate = feedrate;
}

void Machine::setHomeSpeed(double feedrate)
{
	setHomeSpeed(feedrate, feedrate, feedrate);
}

void Machine::setHomeSpeed(double xFeedrate, double yFeedrate, double zFeedrate)
{
	homeFeedrate[AXIS_X] = xFeedrate;
	homeFeedrate[AXIS_Y] = yFeedrate;
	homeFeedrate[AXIS_Z] = zFeedrate;
}

void Machine::setRelative()
{
	if (!isValid()) return;
	if (!MC_Group_inUse[axis_group_number]) return;
	gcode("G91");
}

void Machine::setAbsolute()
{
	if (!isValid()) return;
	if (!MC_Group_inUse[axis_group_number]) return;
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
  if (!MC_Group_inUse[axis_group_number]) return false;
  return MC_Group[axis_group_number].g_code->GetMotorDirInvert(axis) == 1;
}

bool Machine::isMoving()
{
  if (!isValid()) return false;
  if (!MC_Group_inUse[axis_group_number]) return false;
  return !MC_Group[axis_group_number].g_code->IsFinishMove();
}

bool Machine::isValid()
{
  return axis_group_number != AXIS_UNUSED;
}

bool Machine::isCmdBufferFull()
{
  return MC_Group[axis_group_number].g_code->IsBlockFull();
}

void Machine::enableSoftLimit()
{
  if (!isValid()) return;
  if (!MC_Group_inUse[axis_group_number]) return;
  MC_Group[axis_group_number].g_code->SetSoftLimit(1);
}

void Machine::disableSoftLimit()
{
  if (!isValid()) return;
  if (!MC_Group_inUse[axis_group_number]) return;
  MC_Group[axis_group_number].g_code->SetSoftLimit(0);
}

void Machine::machineOn()
{
	if (!isValid()) return;

	if ((MC_Group_inUse[0] == false) && (MC_Group_inUse[1] == false) && (MC_Group_inUse[2] == false))
	{
		interpolator_AttachInterrupt();
		dda_AttachInterrupt();
	}
	MC_Group[axis_group_number].dda->DDAInit();
	io_DisableINT();
	MC_Group_inUse[axis_group_number] = true;
	io_RestoreINT();

	if (axis_group_number == 0)
	{
		io_outpb(CROSSBARBASE + 0x90 + PIN86[5].gpN, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[6].gpN, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[9].gpN, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[42].gpN, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[43].gpN, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[44].gpN, 0x08);
	}
	else if (axis_group_number == 1)
	{
		io_outpb(CROSSBARBASE + 0x90 + PIN86[10].gpN, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[11].gpN, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[13].gpN, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[18].gpN, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[19].gpN, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[20].gpN, 0x08);
	}
	else if (axis_group_number == 2)
	{
		io_outpb(CROSSBARBASE + 0x90 + PIN86[29].gpN, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[30].gpN, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[31].gpN, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[33].gpN, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[34].gpN, 0x08);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[35].gpN, 0x08);
	}
	MC_Group[axis_group_number].setting->machineMode = MachineOn;
	if (MC_Group[axis_group_number].g_code->isEGearAttached())
		MC_Group[axis_group_number].EGear_Machine->machineOn();
}

void Machine::machineOff()
{
	if (!isValid()) return;
	if (!MC_Group_inUse[axis_group_number]) return;
	while (isMoving());
	MC_Group[axis_group_number].setting->machineMode = MachineOff;
	if (axis_group_number == 0)
	{
		io_outpb(CROSSBARBASE + 0x90 + PIN86[5].gpN, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[6].gpN, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[9].gpN, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[42].gpN, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[43].gpN, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[44].gpN, 0x01);
	}
	else if (axis_group_number == 1)
	{
		io_outpb(CROSSBARBASE + 0x90 + PIN86[10].gpN, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[11].gpN, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[13].gpN, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[18].gpN, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[19].gpN, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[20].gpN, 0x01);
	}
	else if (axis_group_number == 2)
	{
		io_outpb(CROSSBARBASE + 0x90 + PIN86[29].gpN, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[30].gpN, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[31].gpN, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[33].gpN, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[34].gpN, 0x01);
		io_outpb(CROSSBARBASE + 0x90 + PIN86[35].gpN, 0x01);
	}
	if (MC_Group[axis_group_number].g_code->isEGearAttached())
		MC_Group[axis_group_number].EGear_Machine->machineOff();

	io_DisableINT();
	MC_Group_inUse[axis_group_number] = false;
	io_RestoreINT();

	if ((MC_Group_inUse[0] == false) && (MC_Group_inUse[1] == false) && (MC_Group_inUse[2] == false))
	{
		dda_DetachInterrupt();
		interpolator_DetachInterrupt();
	}
}

bool Machine::gcode(char *cmd)
{
  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return false;
  if (!isValid()) return false;
  if (!MC_Group_inUse[axis_group_number]) return false;
  MC_Group[axis_group_number].g_code->process_commands(cmd);
  return true;
}

void Machine::home()
{
  int i, j;
  bool relativeMode = MC_Group[axis_group_number].g_code->IsRelativeMode();
  bool softLimitMode = MC_Group[axis_group_number].g_code->GetSoftLimitMode();
  double move[3] = { -150, 5, -10 };
  MotPlanner* EGear_p = NULL;

  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return;
  if (!isValid()) return;
  if (!MC_Group_inUse[axis_group_number]) return;

  if (MC_Group[axis_group_number].g_code->isEGearAttached())
  {
    EGear_p = MC_Group[MC_Group[axis_group_number].EGear_Machine->axis_group_number].planner;
    MC_Group[axis_group_number].g_code->DettachEGear();
  }
  while (isMoving());

  if (MC_Group[axis_group_number].setting->homeLimit[0] < 0 || MC_Group[axis_group_number].setting->homeLimit[1] < 0 || MC_Group[axis_group_number].setting->homeLimit[2] < 0)
  {
    gcode("G92 X0 Y0 Z0");
  }
  else
  {
    setRelative();
    disableSoftLimit();

    while (isMoving());
    for (i = AXIS_X; i <= AXIS_Z; i++)
    {
      pinMode(MC_Group[axis_group_number].setting->homeLimit[i], INPUT_PULLUP);
      for (j = 0; j < 3; j++)
      {
        line(i == AXIS_X ? move[j] : 0,
             i == AXIS_Y ? move[j] : 0,
             i == AXIS_Z ? move[j] : 0, homeFeedrate[i]);
        while (isMoving())
        {
          if (digitalRead(MC_Group[axis_group_number].setting->homeLimit[i]) == 0 && j != 1)
            stop();
        }
      }
    }
    gcode("G92 X0 Y0 Z0");
    if (softLimitMode) enableSoftLimit();
    if (!relativeMode) setAbsolute();
  }
  if (EGear_p != NULL)
  {
    MC_Group[axis_group_number].EGear_Machine->home();
    MC_Group[axis_group_number].g_code->AttachEGear(EGear_p);
  }
}

void Machine::stop()
{
  emgStop();
  clearEMGStop();
}

void Machine::emgStop()
{
  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return;
  if (!isValid()) return;
  if (!MC_Group_inUse[axis_group_number]) return;
  MC_Group[axis_group_number].g_code->SetEMGStatus(true);
  if (MC_Group[axis_group_number].g_code->isEGearAttached())
    MC_Group[MC_Group[axis_group_number].EGear_Machine->axis_group_number].g_code->SetEMGStatus(true);
}

void Machine::clearEMGStop()
{
  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return;
  if (!isValid()) return;
  if (!MC_Group_inUse[axis_group_number]) return;
  MC_Group[axis_group_number].g_code->SetEMGStatus(false);
  if (MC_Group[axis_group_number].g_code->isEGearAttached())
    MC_Group[MC_Group[axis_group_number].EGear_Machine->axis_group_number].g_code->SetEMGStatus(false);
}

void Machine::feedrateOverride(double ratio)
{
  if (!isValid()) return;
  if (!MC_Group_inUse[axis_group_number]) return;
  MC_Group[axis_group_number].g_code->SetFeedrateRatio(ratio);
}

bool Machine::line(double x, double y, double z, double feedrate)
{
  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return false;
  if (!isValid()) return false;
  if (!MC_Group_inUse[axis_group_number]) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  sprintf(buffer, "G1 X%.5f Y%.5f Z%.5f F%.5f", x, y, z, defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::arcXY(double cX, double cY, double dstX, double dstY, bool revDir, double feedrate)
{
  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return false;
  if (!isValid()) return false;
  if (!MC_Group_inUse[axis_group_number]) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G17");
  sprintf(buffer, "G%d X%.5f Y%.5f I%.5f J%.5f F%.5f", revDir ? 2 : 3, dstX, dstY, cX, cY, defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::arcXZ(double cX, double cZ, double dstX, double dstZ, bool revDir, double feedrate)
{
  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return false;
  if (!isValid()) return false;
  if (!MC_Group_inUse[axis_group_number]) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G18");
  sprintf(buffer, "G%d X%.5f Z%.5f I%.5f K%.5f F%.5f", revDir ? 2 : 3, dstX, dstZ, cX, cZ, defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::arcYZ(double cY, double cZ, double dstY, double dstZ, bool revDir, double feedrate)
{
  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return false;
  if (!isValid()) return false;
  if (!MC_Group_inUse[axis_group_number]) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G19");
  sprintf(buffer, "G%d Y%.5f Z%.5f J%.5f K%.5f F%.5f", revDir ? 2 : 3, dstY, dstZ, cY, cZ, defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::arcXY(double r, double dstX, double dstY, bool revDir, double feedrate)
{
  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return false;
  if (!isValid()) return false;
  if (!MC_Group_inUse[axis_group_number]) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G17");
  sprintf(buffer, "G%d X%.5f Y%.5f R%.5f F%.5f", revDir ? 2 : 3, dstX, dstY, r, defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::arcXZ(double r, double dstX, double dstZ, bool revDir, double feedrate)
{
  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return false;
  if (!isValid()) return false;
  if (!MC_Group_inUse[axis_group_number]) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G18");
  sprintf(buffer, "G%d X%.5f Z%.5f R%.5f F%.5f", revDir ? 2 : 3, dstX, dstZ, r, defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::arcYZ(double r, double dstY, double dstZ, bool revDir, double feedrate)
{
  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return false;
  if (!isValid()) return false;
  if (!MC_Group_inUse[axis_group_number]) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G19");
  sprintf(buffer, "G%d Y%.5f Z%.5f R%.5f F%.5f", revDir ? 2 : 3, dstY, dstZ, r, defaultFeedrate);
  gcode(buffer);
  return true;
}

bool Machine::arcXY_Theta(double cX, double cY, double theta, double feedrate)
{
  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return false;
  if (!isValid()) return false;
  if (!MC_Group_inUse[axis_group_number]) return false;
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
  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return false;
  if (!isValid()) return false;
  if (!MC_Group_inUse[axis_group_number]) return false;
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
  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return false;
  if (!isValid()) return false;
  if (!MC_Group_inUse[axis_group_number]) return false;
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
  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return false;
  if (!isValid()) return false;
  if (!MC_Group_inUse[axis_group_number]) return false;
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
  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return false;
  if (!isValid()) return false;
  if (!MC_Group_inUse[axis_group_number]) return false;
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
  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return false;
  if (!isValid()) return false;
  if (!MC_Group_inUse[axis_group_number]) return false;
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
  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return false;
  if (!isValid()) return false;
  if (!MC_Group_inUse[axis_group_number]) return false;
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
  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return false;
  if (!isValid()) return false;
  if (!MC_Group_inUse[axis_group_number]) return false;
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
  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return false;
  if (!isValid()) return false;
  if (!MC_Group_inUse[axis_group_number]) return false;
  if (feedrate > 0)
    setDefaultFeedrate(feedrate);
  gcode("G19");
  double dstY, dstZ;
  MC_Group[axis_group_number].g_code->ComputeDst(cY, cZ, theta, &dstY, &dstZ);
  sprintf(buffer, "G%d X%.5f Y%.5f Z%.5f J%.5f K%.5f P%d F%.5f", theta < 0 ? 2 : 3, dstX, dstY, dstZ, cY, cZ, (int)ceil(fabs(theta) / (M_PI * 2)), defaultFeedrate);
  gcode(buffer);
  return true;
}

void Machine::beginJog(int pX, int mX, int pY, int mY, int pZ, int mZ, bool incJog)
{

  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return;
  if (!isValid()) return;
  if (!MC_Group_inUse[axis_group_number]) return;
  while (isMoving());

  pinMode(pX, INPUT);
  pinMode(mX, INPUT);
  pinMode(pY, INPUT);
  pinMode(mY, INPUT);
  pinMode(pZ, INPUT);
  pinMode(mZ, INPUT);
  jogTrigger[axis_group_number][0] = pX;
  jogTrigger[axis_group_number][1] = mX;
  jogTrigger[axis_group_number][2] = pY;
  jogTrigger[axis_group_number][3] = mY;
  jogTrigger[axis_group_number][4] = pZ;
  jogTrigger[axis_group_number][5] = mZ;

  jogCount[axis_group_number][AXIS_X] = 0;
  jogCount[axis_group_number][AXIS_Y] = 0;
  jogCount[axis_group_number][AXIS_Z] = 0;
  jogVector[axis_group_number][AXIS_X] = 0;
  jogVector[axis_group_number][AXIS_Y] = 0;
  jogVector[axis_group_number][AXIS_Z] = 0;
  MC_Group[axis_group_number].interpolator->InitJog(jogCallback);
  if (incJog)
    MC_Group[axis_group_number].setting->machineMode = IncJogMode;
  else
    MC_Group[axis_group_number].setting->machineMode = JogMode;
}

void Machine::endJog()
{
  MC_Group[axis_group_number].setting->machineMode = MachineOn;
  MC_Group[axis_group_number].interpolator->CloseJog();
}
void Machine::setJogSpeed(double feedrate)
{
  MC_Group[axis_group_number].g_code->SetJogFeedrate(feedrate);
}

void Machine::setJogOffset(double incPerMM)
{
  if (incPerMM != 0)
    incJogPerMM[axis_group_number] = incPerMM;
}

void Machine::beginMpg(Encoder &encoder)
{
  if (MC_Group[axis_group_number].setting->machineMode != MachineOn) return;
  if (!isValid()) return;
  if (!MC_Group_inUse[axis_group_number]) return;
  while (isMoving());

  mpgTrigger[axis_group_number] = &encoder;
  mpgAxis[axis_group_number] = AXIS_X;
  mpgRate[axis_group_number] = 1.0;
  mpgPreLocation[axis_group_number] = (double)encoder.read();
  MC_Group[axis_group_number].interpolator->InitJog(jogCallback);
  MC_Group[axis_group_number].setting->machineMode = MpgMode;
}

void Machine::endMpg()
{
  mpgTrigger[axis_group_number] = NULL;
  MC_Group[axis_group_number].setting->machineMode = MachineOn;
  MC_Group[axis_group_number].interpolator->CloseJog();
}

void Machine::setMpgSpeed(double feedrate)
{
  MC_Group[axis_group_number].g_code->SetJogFeedrate(feedrate);
}

void Machine::setMpgAxis(machineAxis_t axis)
{
  mpgAxis[axis_group_number] = axis;
}

void Machine::setMpgRatio(double rate)
{
  if (rate > 0)
    mpgRate[axis_group_number] = rate;
}

double Machine::getJogPos(machineAxis_t axis)
{
  if (!isValid()) return -1;
  if (!MC_Group_inUse[axis_group_number]) return -1;
  return MC_Group[axis_group_number].g_code->GetJogPos(axis);
}

void Machine::getJogPos(double &x, double &y, double &z)
{
  if (!isValid()) return;
  if (!MC_Group_inUse[axis_group_number]) return;
  double jog_pos[MAX_AXIS];
  MC_Group[axis_group_number].g_code->GetJogPos(jog_pos);
  x = jog_pos[AXIS_X];
  y = jog_pos[AXIS_Y];
  z = jog_pos[AXIS_Z];
}