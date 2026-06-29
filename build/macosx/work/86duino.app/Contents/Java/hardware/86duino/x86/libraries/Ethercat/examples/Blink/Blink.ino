#include "Ethercat.h"

EthercatMaster master;
EthercatDevice_QECR11D0FS slave;

void setup(void) {
    master.begin();
    slave.attach(0, master);
    master.start();
}

void loop() {
    slave.digitalWrite(0, HIGH);
    delay(4000);
    slave.digitalWrite(0, LOW);
    delay(1000);
}