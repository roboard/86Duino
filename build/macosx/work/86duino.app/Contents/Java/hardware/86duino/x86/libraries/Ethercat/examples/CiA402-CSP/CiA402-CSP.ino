#include "Ethercat.h"

EthercatMaster master;
EthercatDevice_CiA402 motor;

int32_t position = 0;

void MyCyclicCallback()
{
    if (motor.getCiA402State() == CIA402_OPERATION_ENABLED)
        motor.setTargetPosition(position += 1000);
}

void setup()
{
    master.begin();

    motor.attach(0, master);
    motor.setDc(1000000);
    motor.setCiA402Mode(CIA402_CSP_MODE);

    master.attachCyclicCallback(MyCyclicCallback);
    master.start(1000000, ECAT_SYNC);

    motor.setTargetPosition(position = motor.getPositionActualValue());
    motor.enable();
}

void loop()
{
}