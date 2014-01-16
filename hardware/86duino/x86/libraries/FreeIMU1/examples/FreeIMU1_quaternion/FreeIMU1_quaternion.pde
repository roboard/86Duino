#include <EEPROM.h>

#include "FreeIMU1.h"
#include <Wire.h>


float q[4];

// Set the FreeIMU object
FreeIMU1 my3IMU = FreeIMU1();

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  delay(5);
  my3IMU.init();
  delay(5);
}


void loop() { 
  my3IMU.getQ(q);
  Serial.print(q[0]);
  Serial.print(", ");
  Serial.print(q[1]);
  Serial.print(", ");
  Serial.print(q[2]);
  Serial.print(", ");
  Serial.print(q[3]);
  Serial.println("");
  delay(20);
}

