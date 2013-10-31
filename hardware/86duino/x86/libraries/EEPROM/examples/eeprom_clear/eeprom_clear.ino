/*
 * EEPROM Clear
 *
 * Sets all of the bytes of the EEPROM to 0.
 * This example code is in the public domain.

 */

#include <EEPROM.h>

void setup()
{
  // write a 0 to all 200 bytes of the EEPROM
  for (int i = 0; i < 200; i++)
    EEPROM.write(i, 0);
    
  // turn the LED on when we're done
  digitalWrite(13, HIGH);
}

void loop()
{
}
