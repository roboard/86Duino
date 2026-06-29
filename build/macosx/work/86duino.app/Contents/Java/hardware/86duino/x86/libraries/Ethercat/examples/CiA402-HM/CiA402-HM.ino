#include "Ethercat.h"

EthercatMaster master;
EthercatDevice_CiA402 motor;

void setup()
{
    master.begin();
    motor.attach(0, master);
    motor.setCiA402Mode(CIA402_HOMING_MODE);
    master.start();

    motor.enable();

    motor.hm_SetHomingMethod(CIA402_HM19);
    motor.hm_SetHomeOffset(0);
    motor.hm_SetHomingSpeeds(100, 20);
    motor.hm_SetHomingAcceleration(100);
    motor.hm_Run();
    while (motor.hm_IsAttained() == CIA402_HM_RUNNING);
}

void loop()
{
}