#include "stdarg.h"
#include "VirtualServo.h"

#define VS_ANGLE_MODE        (0)
#define VS_MOTOR_UNIT_MODE   (1)

#define DEBUG    (0)

MyVirtualServo::MyVirtualServo()
{
    servoType = UNKNOWN_VIRTUALSERVO_TYPE;
    servoDevice = NULL;
    _enable = _attached = false;
    _MaxAcc = _MaxDecAcc = _MaxVelocity = 0.0;
    _defaultHomeMode = -1;
    _homeAcc = _homeVelocity = 0.0;
    // _motorUnit = 0;
    _offset = 0;
    _ppr = 0.0;
    _acc = _decAcc = _velocity = 0.0;
    _motionProfileType = -1;
    _flag1 = CIA402_PP_ABSOLUTE;
    _flag2 = VS_ANGLE_MODE;
}

void MyVirtualServo::init(VirtualServoDeviceTypes type)
{
    servoType = type;
}

int MyVirtualServo::attach(void* slave) {
    int rc;
    
    if (slave == NULL) return -1;
    
    servoDevice = slave;
    
    if (servoType == VIRTUALSERVO_ETHERCAT) {
        _defaultHomeMode = ((EthercatDevice_CiA402*)servoDevice)->driveGetHomingMethod();
        _homeVelocity = ((EthercatDevice_CiA402*)servoDevice)->driveGetHomingSpeedDuringSearchForSwitch();
        _homeAcc = ((EthercatDevice_CiA402*)servoDevice)->driveGetHomingAcceleration();
        // rc = ((EthercatDevice_CiA402*)servoDevice)->driveSetMaxProfileVelocity(200.0);
        // if (rc < 0) printf("1Error: %d\n", rc);
        //_MaxVelocity = ((EthercatDevice_CiA402*)servoDevice)->driveGetMaxMotorSpeed();
        // printf("MaxVelocity: %f\n", _MaxVelocity);
        // rc = ((EthercatDevice_CiA402*)servoDevice)->driveSetMaxAcceleration(200.0);
        // if (rc < 0) printf("2Error: %d\n", rc);
        //_MaxAcc = ((EthercatDevice_CiA402*)servoDevice)->driveGetMaxAcceleration();
        // printf("MaxAcc: %f\n", _MaxAcc);
        // rc = ((EthercatDevice_CiA402*)servoDevice)->driveSetMaxDeceleration(200.0);
        // if (rc < 0) printf("3Error: %d\n", rc);
        //_MaxDecAcc = ((EthercatDevice_CiA402*)servoDevice)->driveGetMaxDeceleration();
        // printf("MaxDec: %f\n", _MaxDecAcc);

        _ppr = ((EthercatDevice_CiA402*)servoDevice)->driveGetMotorResolution(); // T120/T600/Panasonic will get 0
        if (DEBUG) printf("ppr getting from machine: %f\n", _ppr);
        angleMap(0.0, 360.0, 0.0, _ppr);
        
        _acc = _decAcc = _MaxAcc;
        _velocity = _MaxVelocity;
        
        _motionProfileType = 0; // T Acc/Dec
        _enable = (((EthercatDevice_CiA402*)servoDevice)->driveGetState() == CIA402_OPERATION_ENABLED) ? true : false;
        
        ((EthercatDevice_CiA402*)servoDevice)->setCiA402Mode(CIA402_PP_MODE);
        ((EthercatDevice_CiA402*)servoDevice)->setSoftwarePositionLimit(0x80000000L, 0x7FFFFFFFL);

        ((EthercatDevice_CiA402*)servoDevice)->pp_SetMotionProfileType(_motionProfileType);
    }
    
    _attached = true;
    
    return 0;
}

int MyVirtualServo::attached(void) {
    if (servoDevice == NULL) return -1;
    
    if (DEBUG) printf("ppr: %f maxAcc: %f maxDecAcc: %f maxVelocity: %f defaultHomeMode: %d, homeAcc: %f homeVelocity: %f\n", _ppr, _MaxAcc, _MaxDecAcc, _MaxVelocity, _defaultHomeMode, _homeAcc, _homeVelocity);
    //while(1);

    // printf("%f\n", ((EthercatDevice_CiA402*)servoDevice)->driveGetHomingMethod());
    // printf("%f\n", ((EthercatDevice_CiA402*)servoDevice)->driveGetHomingSpeedDuringSearchForSwitch());
    // printf("%f\n", ((EthercatDevice_CiA402*)servoDevice)->driveGetHomingAcceleration());
    // printf("%f\n", ((EthercatDevice_CiA402*)servoDevice)->driveGetMaxMotorSpeed());
    // printf("%f\n", ((EthercatDevice_CiA402*)servoDevice)->driveGetMaxAcceleration());
    // printf("%f\n", ((EthercatDevice_CiA402*)servoDevice)->driveGetMaxDeceleration());
    // printf("%f\n", ((EthercatDevice_CiA402*)servoDevice)->driveGetProfileVelocity());
    // printf("%f\n", ((EthercatDevice_CiA402*)servoDevice)->driveGetProfileAcceleration());

    return _attached;
}

int MyVirtualServo::setVelocity(double velocity) {
    
    if (servoDevice == NULL) return -1;
    
    // if (_MaxVelocity > 0 && _velocity > _MaxVelocity) _velocity = _MaxVelocity;
    
    _velocity = velocity;
    
    return 0;
}

int MyVirtualServo::setAcceleration(double acc) {
    
    if (servoDevice == NULL) return -1;
    
    // if (_MaxAcc > 0 && _acc > _MaxAcc) _acc = _MaxAcc;
    
    _acc = acc;
    _decAcc = acc;
    
    return 0;
}

int MyVirtualServo::setPositionType(VirtualServoPositionTypes type) {
    
    if (servoDevice == NULL) return -1;
    
    switch (type) {
    case VIRTUALSERVO_ABSOLUTE_POSITION_ANGLE:
        _flag1 = CIA402_PP_ABSOLUTE;
        _flag2 = VS_ANGLE_MODE;
        break;
    case VIRTUALSERVO_ABSOLUTE_POSITION_STEP:
        _flag1 = CIA402_PP_ABSOLUTE;
        _flag2 = VS_MOTOR_UNIT_MODE;
        break;
    case VIRTUALSERVO_RELATIVE_POSITION_ANGLE:
        _flag1 = CIA402_PP_RELATIVE;
        _flag2 = VS_ANGLE_MODE;
        break;
    case VIRTUALSERVO_RELATIVE_POSITION_STEP:
        _flag1 = CIA402_PP_RELATIVE;
        _flag2 = VS_MOTOR_UNIT_MODE;
        break;
    }
    
    _positionType = type;
    
    return 0;
}

int32_t MyVirtualServo::_angleToMotrUnit(double angle) {
       
    if (_flag1 == CIA402_PP_ABSOLUTE) {
        double x, in_min, in_max, out_min, out_max;
        
        x = angle;
        in_min = _angle1;
        in_max = _angle2;
        out_min = _motorStep1;
        out_max = _motorStep2;
        
        return ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);   
    
    } else if (_flag1 == CIA402_PP_RELATIVE) {
    
        if (_ppr == 0.0) {
            if (DEBUG) printf("_ppr is zero\n");
            return 0;
        }
    
        return round(angle / _ppr);
    }
    
    return -1;
}

double MyVirtualServo::_motrUnitToAngle(int32_t position) {
    
    if (_flag1 == CIA402_PP_ABSOLUTE) {
        double x, in_min, in_max, out_min, out_max;
        
        x = position;
        in_min = _motorStep1;
        in_max = _motorStep2;
        out_min = _angle1;
        out_max = _angle2;
        
        return ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);   
    
    } else if (_flag1 == CIA402_PP_RELATIVE) {
    
        // if (_ppr == 0.0) {
            // if (DEBUG) printf("_ppr is zero\n");
            // return 0;
        // }
    
        // return (double)position * _ppr;
        return 0;
    }
    
    return -1;
}

/*
double MyVirtualServo::_calVelocity(int32_t distance, double acc, double duration) {
    
    double _a = 1.0/acc;
    double _b = duration; // b
    double _c = 4.0 * (1.0/acc) * distance; // 4ac
    double _v = 0.0;
    // double _vmax = _MaxVelocity;
    
    // _v = 2 * distance / duration;
    
    // if (_v > _vmax) {
    if (_b*_b > _c) { // b^2 > 4ac
        _v = (_b - sqrt(_b*_b - _c)) / (2*_a);
        if (_v < 0.0)
            _v = (_b + sqrt(_b*_b - _c)) / (2*_a);
    } else if (_b*_b == _c) {
        _v = _b/(2*_a);
    } else // no solution
        _v = _MaxVelocity;
    // }

    return _v;
}
*/

// duration 單位是 ms
int MyVirtualServo::write(double angle) {
    
    int32_t _position;
    int rc = 0;
    
    if (servoDevice == NULL) return -1;
    
    _position = (_flag2 == VS_ANGLE_MODE) ? _angleToMotrUnit(angle + _offset) : angle + _offset;
    
    /*
    if (duration > 0) {
        if (_flag1 == CIA402_PP_ABSOLUTE)
            _velocity = _calVelocity(abs(_position - read() + _offset), _acc, duration/1000.0);
        else if (_flag1 == CIA402_PP_RELATIVE)
            _velocity = _calVelocity(abs(_position + _offset), _acc, duration/1000.0);
    }
    */
    
    if (DEBUG) printf("_ppr: %f _velocity: %f\n", _ppr, _velocity);
    
    if (servoType == VIRTUALSERVO_ETHERCAT) {
        ((EthercatDevice_CiA402*)servoDevice)->setCiA402Mode(CIA402_PP_MODE); // call setCiA402Mode() here for NPM Motor
        ((EthercatDevice_CiA402*)servoDevice)->setSoftwarePositionLimit(0x80000000L, 0x7FFFFFFFL); // call setSoftwarePositionLimit() here for NPM Motor
        ((EthercatDevice_CiA402*)servoDevice)->pp_SetMotionProfileType(_motionProfileType); // call pp_SetMotionProfileType() here for NPM Motor
        
        if (!_enable) { ((EthercatDevice_CiA402*)servoDevice)->enable(); _enable = true; }
        
        if (DEBUG) printf("_position: %d acc: %f dec: %f vel: %f\n", _position, _acc, _decAcc, _velocity);
        
        ((EthercatDevice_CiA402*)servoDevice)->pp_SetAcceleration(_acc);
        ((EthercatDevice_CiA402*)servoDevice)->pp_SetDeceleration(_decAcc);
        ((EthercatDevice_CiA402*)servoDevice)->pp_SetVelocity(_velocity);
        
        ((EthercatDevice_CiA402*)servoDevice)->pp_Run(_position, _flag1, true);
    }
    
    return rc;
}

double MyVirtualServo::read() {
    
    if (servoDevice == NULL) return -1;
    
    if (servoType == VIRTUALSERVO_ETHERCAT) {
        if (_flag2 == VS_ANGLE_MODE)
            return _motrUnitToAngle(((EthercatDevice_CiA402*)servoDevice)->getPositionActualValue()) - _offset;
        else
            return ((EthercatDevice_CiA402*)servoDevice)->getPositionActualValue() - _offset;
    }
    
    if (DEBUG) {
        Serial.println("Error: No Set Servo Type");
        printf("Error: No Set Servo Type\n");
    }
    
    return -1;
}

int MyVirtualServo::setOffset(double offset) {
    
    if (servoDevice == NULL) return -1;
    
    // It would be "Angle" or "Step" nuit
    _offset = offset;
    
    return 0;
}

/*
int MyVirtualServo::setMotorUnitPerRevolution(double ppr) {
    
    if (servoDevice == NULL) return -1;
    
    if (DEBUG) printf("ppr: %f\n", ppr);
    
    _ppr = 360.0 / ppr;
    
    return 0;
}
*/

int MyVirtualServo::pause(void) { // blocking function
    
    if (servoDevice == NULL) return -1;

    if (servoType == VIRTUALSERVO_ETHERCAT) {
        return ((EthercatDevice_CiA402*)servoDevice)->pp_Halt(10000);
    }
    
    if (DEBUG) {
        Serial.println("Error: No Set Servo Type");
        printf("Error: No Set Servo Type\n");
    }
    
    return -1;
}

int MyVirtualServo::resume(void) {
    
    if (servoDevice == NULL) return -1;
    
    if (servoType == VIRTUALSERVO_ETHERCAT) {
        return ((EthercatDevice_CiA402*)servoDevice)->pp_Resume();
    }
    
    if (DEBUG) {
        Serial.println("Error: No Set Servo Type");
        printf("Error: No Set Servo Type\n");
    }
    
    return -1;
}

int MyVirtualServo::stop(void) {
    
    if (servoDevice == NULL) return -1;
    
    if (isMoving())
        pause();
    
    if (servoType == VIRTUALSERVO_ETHERCAT) {
        if (_flag1 == CIA402_PP_ABSOLUTE)
            return ((EthercatDevice_CiA402*)servoDevice)->pp_Run(read(), CIA402_PP_ABSOLUTE, true);
        else if (_flag1 == CIA402_PP_RELATIVE)
            return ((EthercatDevice_CiA402*)servoDevice)->pp_Run(0, CIA402_PP_RELATIVE, true);
    }
    
    if (DEBUG) {
        Serial.println("Error: No Set Servo Type");
        printf("Error: No Set Servo Type\n");
    }
    
    return -1;
}

int MyVirtualServo::release(void) {
    
    if (servoDevice == NULL) return -1;
    
    _enable = false;
    stop();
    
    if (servoType == VIRTUALSERVO_ETHERCAT) {
        return ((EthercatDevice_CiA402*)servoDevice)->disable();
    }
    
    if (DEBUG) {
        Serial.println("Error: No Set Servo Type");
        printf("Error: No Set Servo Type\n");
    }
    
    return -1;
}
    
bool MyVirtualServo::isMoving(void) {
    
    if (servoDevice == NULL) return false;
    
    if (servoType == VIRTUALSERVO_ETHERCAT) {
        return !((EthercatDevice_CiA402*)servoDevice)->pp_IsTargetReached();
    }
    
    if (DEBUG) {
        Serial.println("Error: No Set Servo Type");
        printf("Error: No Set Servo Type\n");
    }
    
    return false;
}

int MyVirtualServo::home(void) {
    
    if (servoDevice == NULL) return 0;
    
    if (servoType == VIRTUALSERVO_ETHERCAT) {
        
        if (!_enable) { ((EthercatDevice_CiA402*)servoDevice)->enable(); _enable = true; }
        
        ((EthercatDevice_CiA402*)servoDevice)->setCiA402Mode(CIA402_HOMING_MODE);
        delay(100);
        
        ((EthercatDevice_CiA402*)servoDevice)->hm_SetHomeOffset(0);
        ((EthercatDevice_CiA402*)servoDevice)->hm_SetHomingMethod(_defaultHomeMode);
        ((EthercatDevice_CiA402*)servoDevice)->hm_SetHomingSpeeds(_homeVelocity, 20);
        ((EthercatDevice_CiA402*)servoDevice)->hm_SetHomingAcceleration(_homeAcc);
        ((EthercatDevice_CiA402*)servoDevice)->hm_Run();
        
        // #define CIA402_HM_RUNNING   (0)
        // #define CIA402_HM_ATTAINED  (1)
        // #define CIA402_HM_ERROR     (2)
        while (((EthercatDevice_CiA402*)servoDevice)->hm_IsAttained() == CIA402_HM_RUNNING);
        
        ((EthercatDevice_CiA402*)servoDevice)->setCiA402Mode(CIA402_PP_MODE);
        delay(100);
    } else {
        Serial.println("Error: No Set Servo Type");
        printf("Error: No Set Servo Type\n");
        return -1;
    }

    return 0;
}

void MyVirtualServo::angleMap(double angle1, double angle2, double motorStep1, double motorStep2) {
    
    if (servoDevice == NULL) return;
    
    _angle1 = angle1;
    _angle2 = angle2;
    _motorStep1 = motorStep1;
    _motorStep2 = motorStep2;
    if ((motorStep2 > 0.0 || motorStep1 > 0.0) && (motorStep2 - motorStep1) > 0.0) {
        _ppr = (angle2 - angle1) / (motorStep2 - motorStep1);
    }
}

void* MyVirtualServo::getServoDevice(void) {
	return servoDevice;
}

double MyVirtualServo::getT600Voltage(void) {
    
    if (servoDevice == NULL) return 0.0;
    
    return (((EthercatDevice_CiA402*)servoDevice)->sdoUpload16(0x5003, 0, 100000))/100.0; // psVoltage
}

double MyVirtualServo::getT600Temperature(void) {
    
    if (servoDevice == NULL) return 0.0;
    
    return (((EthercatDevice_CiA402*)servoDevice)->sdoUpload16(0x5001, 2, 100000))/100.0; // motorTemperature
}

double MyVirtualServo::getT600ServoCurrent(void) {
    
    if (servoDevice == NULL) return 0.0;
    
    return (((EthercatDevice_CiA402*)servoDevice)->sdoUpload16(0x5004, 0, 100000))/100.0; // servoCurrent
}
