#include "Ethercat.h"

EthercatMaster master;
EthercatDevice_CiA402 motor;

void setup()
{
    master.begin();
    motor.attach(0, master);
    motor.setCiA402Mode(CIA402_PP_MODE);
    master.start();

    motor.enable();
    motor.pp_SetMotionProfileType(0);
    motor.pp_SetVelocity(100000);
    motor.pp_SetAcceleration(5000);
    motor.pp_SetDeceleration(5000);
}

void loop()
{
    motor.pp_Run(10000);
    while (motor.pp_IsTargetReached() == 0);
    motor.pp_Run(-10000);
    while (motor.pp_IsTargetReached() == 0);
}