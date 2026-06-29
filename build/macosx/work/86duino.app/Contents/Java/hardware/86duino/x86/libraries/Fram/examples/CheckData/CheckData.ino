#include "Fram.h"

void setup() {
    Serial.begin(9600);
    Serial.println("Start to test ... ");
    FRAM.begin();
    
    for (int i=0, data=0; i < 0x40000L; i++, data++) {
        if (data > 0xFF) data = 0;
        FRAM.writeByte(i, data);
    }

    for (int i=0, data=0; i < 0x40000L; i++, data++) {
        if (data > 0xFF) data = 0;
        if (FRAM.readByte(i) != data) {
          Serial.print("data =");
          Serial.print(data, HEX);
          Serial.print("  FRAM.read(");
          Serial.print(i);
          Serial.print(") = ");
          Serial.println(FRAM.readByte(i), HEX);
          while(1);
        }
    }
    Serial.println("OK");
}

void loop() {

}