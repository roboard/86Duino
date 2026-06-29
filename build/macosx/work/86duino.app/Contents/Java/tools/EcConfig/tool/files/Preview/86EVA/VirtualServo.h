#ifndef __VIRTUAL_SERVO_H
#define __VIRTUAL_SERVO_H

#include "Arduino.h"
#include "Ethercat.h"

enum VirtualServoPositionTypes {
    VIRTUALSERVO_ABSOLUTE_POSITION_ANGLE = 0,
    VIRTUALSERVO_ABSOLUTE_POSITION_STEP = 1,
    VIRTUALSERVO_RELATIVE_POSITION_ANGLE = 2,
    VIRTUALSERVO_RELATIVE_POSITION_STEP = 3
};

enum VirtualServoDeviceTypes {
    UNKNOWN_VIRTUALSERVO_TYPE = -1,
    VIRTUALSERVO_ETHERCAT = 0
};

class MyVirtualServo {
private:
    void* servoDevice;
    void* servoData;
    int servoType;
    
    bool _enable, _attached;
    double _MaxAcc, _MaxDecAcc, _MaxVelocity, _homeAcc, _homeDecAcc, _homeVelocity, _offset, _velocity, _acc, _decAcc, _ppr;
    int _defaultHomeMode, _motionProfileType, _positionType, _flag1, _flag2;
    
    double _angle1, _angle2, _motorStep1, _motorStep2;
    
    int32_t _angleToMotrUnit(double angle);
    double _motrUnitToAngle(int32_t position);
    
public:
    MyVirtualServo();
    void init(VirtualServoDeviceTypes type);
    void* getServoDevice(void);
    
    int attach(void* slave);
    int attached(void);
    
    int write(double angle);
    double read(void);
    
    int setVelocity(double velocity);
    int setAcceleration(double acc);
    int setOffset(double offset);
    int setPositionType(VirtualServoPositionTypes type);
    void angleMap(double angle1, double angle2, double motorStep1, double motorStep2);
    
    int home(void);
    int pause(void);
    int resume(void);
    int stop(void);
    int release(void);
    bool isMoving(void);
    
    double getT600Voltage(void);
    double getT600Temperature(void);
    double getT600ServoCurrent(void);
};
#endif