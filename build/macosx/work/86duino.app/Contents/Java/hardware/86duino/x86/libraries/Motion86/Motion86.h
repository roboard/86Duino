/*
  Motion86.h - DM&P Vortex86 Motion86 library
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

#ifndef __MOTION86_H
#define __MOTION86_H

#include <Encoder.h>
#include "motion/MotInterface.h"

enum machineAxis_t
{
  AXIS_X = 0,
  AXIS_Y = 1,
  AXIS_Z = 2
};

enum axisGroup_t
{
  MACHINE_UNUSED = -1,
  MACHINE_0 = 0,
  MACHINE_1 = 1,
  MACHINE_2 = 2,
  CUSTOM_MACHINE_0 = 100,
  CUSTOM_MACHINE_1 = 101,
  CUSTOM_MACHINE_2 = 102,
};

enum ioPolarity_t
{
  ACTIVE_LOW = 0,
  ACTIVE_HIGH = 1,
};

enum homeCommand_t
{
  READ_HOME_X = 0,
  READ_HOME_Y = 1,
  READ_HOME_Z = 2,
};

enum jogCommand_t
{
  READ_JOG_X_POSITIVE = 0,
  READ_JOG_X_NEGATIVE = 1,
  READ_JOG_Y_POSITIVE = 2,
  READ_JOG_Y_NEGATIVE = 3,
  READ_JOG_Z_POSITIVE = 4,
  READ_JOG_Z_NEGATIVE = 5,
};

enum mpgCommand_t
{
  READ_MPG_COUNTER = 0,
  READ_MPG_RANGE = 1
};

class Machine : public IMotCB
{
  public:
    Machine(axisGroup_t axis_group = MACHINE_0);
    Machine(int axis_group) : Machine((axisGroup_t)axis_group) {}

    ~Machine();

    bool config_MaxVelocity(machineAxis_t axis, double max_feedrate_mps);
    bool config_MaxAcc(double max_acc);
    bool config_PosLimit(machineAxis_t axis, double min, double max);
    bool config_PPU(machineAxis_t axis, double steps_per_mm);
    bool config_ReverseDirection(machineAxis_t axis);
    bool config_MaxPulseSpeed(int pfhz);
    bool config_PulseMode(machineAxis_t axis, pulseMode_t mode);
    bool config_HomePins(int xLimit, int yLimit, int zLimit);
    bool config_EGearSlave(Machine &target, double xRatio, double yRatio, double zRatio);
    bool config_HomeController_Callback(int (*cb)(homeCommand_t) = NULL);
    bool config_JogController_Callback(int (*cb)(jogCommand_t) = NULL);
    bool config_MpgController_Callback(unsigned long (*cb)(mpgCommand_t) = NULL);
    
    bool config_CustomActuators_Callbacks(void (*setCB)(int, long), long (*getCB)(int));
    bool config_CustomActuators_CycleTime(unsigned long period);

    double getMaxVelocity(machineAxis_t axis);
    double getMaxAcc();
    double getPPU(machineAxis_t axis);
    double getActualPos(machineAxis_t axis);
    void getActualPos(double &x, double &y, double &z);
    double getFeedrateOverride();
    pulseMode_t getPulseMode(machineAxis_t axis);
    int getCmdCount();
    int getFreeCmdCount();

    void setDefaultFeedrate(double feedrate);
    void setHomeSpeed(double feedrate);
    void setHomeSpeed(double xFeedrate, double yFeedrate, double zFeedrate);
    void setRelative();
    void setAbsolute();

    double computePPU_Belt(double pulses_per_revolution, double belt_pitch, double pulley_tooth_count);
    double computePPU_LeadScrew(double pulses_per_revolution, double leadscrew_pitch, double gear_ratio);

    bool isDirectionReversed(machineAxis_t axis);
    bool isMoving();
    bool isValid();
    bool isCmdBufferFull();

    void enableSoftLimit();
    void disableSoftLimit();

    virtual void machineOn();
    virtual void machineOff();
    bool gcode(char *cmd);
    void home();
    void stop();
    void emgStop();
    void clearEMGStop();
    void feedrateOverride(double ratio);

    bool line(double x, double y, double z, double feedrate = -1.0);
    bool arcXY(double cX, double cY, double dstX, double dstY, bool revDir = false, double feedrate = -1.0);
    bool arcXZ(double cX, double cZ, double dstX, double dstZ, bool revDir = false, double feedrate = -1.0);
    bool arcYZ(double cY, double cZ, double dstY, double dstZ, bool revDir = false, double feedrate = -1.0);
    bool arcXY(double r, double dstX, double dstY, bool revDir = false, double feedrate = -1.0);
    bool arcXZ(double r, double dstX, double dstZ, bool revDir = false, double feedrate = -1.0);
    bool arcYZ(double r, double dstY, double dstZ, bool revDir = false, double feedrate = -1.0);
    bool arcXY_Theta(double cX, double cY, double theta, double feedrate = -1.0);
    bool arcXZ_Theta(double cX, double cZ, double theta, double feedrate = -1.0);
    bool arcYZ_Theta(double cY, double cZ, double theta, double feedrate = -1.0);
    bool circleXY(double cX, double cY, bool revDir = false, double feedrate = -1.0);
    bool circleXZ(double cX, double cZ, bool revDir = false, double feedrate = -1.0);
    bool circleYZ(double cY, double cZ, bool revDir = false, double feedrate = -1.0);
    bool helicalXY(double cX, double cY, double dstZ, double theta, double feedrate = -1.0);
    bool helicalXZ(double cX, double cZ, double dstY, double theta, double feedrate = -1.0);
    bool helicalYZ(double cY, double cZ, double dstX, double theta, double feedrate = -1.0);

    void beginJog(bool incJog = false);
    void beginJog(int pX, int nX, int pY, int nY, int pZ, int nZ, bool incJog = false, ioPolarity_t polarity = ACTIVE_HIGH);
    void endJog();
    void setJogSpeed(double feedrate);
    void setJogOffset(double incPerMM);
    void setJogPins(int pX, int nX, int pY, int nY, int pZ, int nZ, ioPolarity_t polarity = ACTIVE_HIGH);

    void beginMpg();
    void beginMpg(Encoder &encoder);
    void endMpg();
    void setMpgOn();
    void setMpgOff();
    void setMpgSpeed(double feedrate);
    void setMpgAxis(machineAxis_t axis);
    void setMpgRatio(double rate);

    double getJogPos(machineAxis_t axis);
    void getJogPos(double &x, double &y, double &z);

    void updateAxes();
    

    virtual int homeIoHelper(homeCommand_t cmd);
    virtual int jogIoHelper(jogCommand_t cmd);
    virtual unsigned long mpgIoHelper(mpgCommand_t cmd);
    
  protected:
    void setAxisPosition(int axis, long pos) override;
    long getAxisPosition(int axis) override;
    machineMode_t getMachineMode();
    bool isHomePrepReady;
    
  private:
    int axis_group_number;
    double defaultFeedrate;
};

#endif