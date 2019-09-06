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

enum machineAxis_t
{
  AXIS_UNUSED = -1,
  AXIS_X = 0,
  AXIS_Y = 1,
  AXIS_Z = 2
};

class Machine
{
  public:
    Machine(int axis_group = 0);
    ~Machine();

    bool config_MaxVelocity(machineAxis_t axis, double max_feedrate_mps);
    bool config_MaxAcc(double vel);
    bool config_PosLimit(machineAxis_t axis, int min, int max);
    bool config_PPU(machineAxis_t axis, double steps_per_mm);
    bool config_ReverseDirection(machineAxis_t axis);
    bool config_MaxPulseSpeed(int pfhz);
    bool config_PulseMode(machineAxis_t axis, pulseMode_t mode);
    bool config_HomePins(int xLimit, int yLimit, int zLimit);
    bool config_EGearSlave(Machine &target, double xRatio, double yRatio, double zRatio);

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

    void machineOn();
    void machineOff();
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

    void beginJog(int pX, int mX, int pY, int mY, int pZ, int mZ, bool incJog = false);
    void endJog();
    void setJogSpeed(double feedrate);
    void setJogOffset(double incPerMM);

    void beginMpg(Encoder &encoder);
    void endMpg();
    void setMpgSpeed(double feedrate);
    void setMpgAxis(machineAxis_t axis);
    void setMpgRatio(double rate);
    double getJogPos(machineAxis_t axis);
    void getJogPos(double &x, double &y, double &z);
  private:
    int axis_group_number;
    double defaultFeedrate;
    double homeFeedrate[3];
};

#endif