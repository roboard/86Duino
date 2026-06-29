/*
  Continue to read A/B pulse by using encoder 0 (PIN 42 43 44). 
*/

#include "EncoderEX.h"

void setup() {
  Serial.begin(9600);
  Enc0A.begin(MODE_AB_PHASE);
  Enc0A.write(1000L); // set counter value as 1000   
}

void loop() {
  Serial.println(Enc0A.read()); // read A/B pulse counter
  delay(100);
}