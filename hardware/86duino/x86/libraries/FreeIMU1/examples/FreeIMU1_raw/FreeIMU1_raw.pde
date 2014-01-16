#include <EEPROM.h>

#include "FreeIMU1.h"
#include <Wire.h>

int raw_values[11];
char str[512];
float val[9];


// Set the default object
FreeIMU1 my3IMU = FreeIMU1();

void setup() { 
  Serial.begin(115200);
  Wire.begin();
  
  delay(500);
  my3IMU.init(true); // the parameter enable or disable fast mode
  delay(500);
}

void loop() {
  my3IMU.getRawValues(raw_values);
  sprintf(str, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t", raw_values[0], raw_values[1], raw_values[2], raw_values[3], raw_values[4], raw_values[5], raw_values[6], raw_values[7], raw_values[8]);
  Serial.print(str);
  Serial.print('\n');
  delay(20);
}

