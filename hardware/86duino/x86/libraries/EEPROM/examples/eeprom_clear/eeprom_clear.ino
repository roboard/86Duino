/*
 * EEPROM Clear
 *
 * Sets all of the bytes of the EEPROM to 0.
 * This example code is in the public domain.

 */

#include <EEPROM.h>

void setup()
{
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  
  // write a 0 to all 16384 bytes of the EEPROM by default
  for (int i = 0; i < 16384; i++)
    EEPROM.write(i, 0);
    
  // turn the LED on when we're done
  digitalWrite(13, HIGH);
}

void loop()
{
}
