#include "Ethercat.h"

EthercatMaster master;
EthercatDevice_CiA402 motor;

int32_t velocity = 0;
int toggle = 0;

void MyCyclicCallback()
{
    if (motor.getCiA402State() != CIA402_OPERATION_ENABLED)
        return;
    if (abs(velocity) >= 3000)
        toggle = !toggle;
    velocity = velocity +  toggle - !toggle;
    motor.setTargetVelocity(velocity);
}

void setup()
{
    master.begin();

    motor.attach(0, master);
    motor.setDc(1000000);
    motor.setCiA402Mode(CIA402_CSV_MODE);

    /* RxPDO mapping configuration. */
    motor.sdoDownload8(0x1C12, 0x00, 0);
    motor.sdoDownload8(0x1601, 0x00, 0);
    motor.sdoDownload32(0x1601, 0x01, 0x60400010);
    motor.sdoDownload32(0x1601, 0x02, 0x60FF0020);
    motor.sdoDownload8(0x1601, 0x00, 2);
    motor.sdoDownload8(0x1C12, 0x00, 1);
    /* TxPDO mapping configuration. */
    motor.sdoDownload8(0x1C13, 0x00, 0);
    motor.sdoDownload8(0x1A01, 0x00, 0);
    motor.sdoDownload32(0x1A01, 0x01, 0x60410010);
    motor.sdoDownload32(0x1A01, 0x02, 0x606C0020);
    motor.sdoDownload8(0x1A01, 0x00, 2);
    motor.sdoDownload8(0x1C13, 0x00, 1);

    master.attachCyclicCallback(MyCyclicCallback);
    master.start(1000000, ECAT_SYNC);

    motor.setTargetVelocity(0);
    motor.enable();
}

void loop()
{
}