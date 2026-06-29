#include "Fram.h"

void setup() {
    Serial.begin(9600);
    
    FRAM.begin();
    
    FRAM.writeDecimal(101, 101.534);
    FRAM.writeDecimal(1025.43);
    FRAM.writeDecimal(10.3543);
    FRAM.writeDecimal(1045.443);
    FRAM.writeDecimal(1.05534);
    Serial.print("FRAM read:");
    Serial.println(FRAM.readDecimal(101), 3);
    Serial.println(FRAM.readDecimal());
    Serial.println(FRAM.readDecimal(), 4);
    Serial.println(FRAM.readDecimal(), 3);
    Serial.println(FRAM.readDecimal(), 5);
}

void loop() {

}