#include "Ethercat.h"

EthercatMaster master;

void setup(void) {
    uint16_t slavecount, i;
    
    Serial.begin(115200);
    while (!Serial);
    
    master.begin();
    
    slavecount = master.getSlaveCount();
    for (i = 0; i < slavecount; i++) {
        Serial.print("Slave");
        Serial.print(i);
        Serial.print(" => Vendor ID: 0x");
        Serial.print(master.getVendorID(i), HEX);
        Serial.print(", Product Code: 0x");
        Serial.println(master.getProductCode(i), HEX);
    }
}

void loop() {
}