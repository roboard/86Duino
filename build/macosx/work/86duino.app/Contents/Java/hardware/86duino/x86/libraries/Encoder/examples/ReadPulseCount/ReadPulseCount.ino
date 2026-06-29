/*
  Continue to read A/B pulse by using encoder 0 (PIN 42 43 44). 
*/

#include "Encoder.h"

void setup() {
  Serial.begin(9600);
  Enc0.begin(MODE_AB_PHASE);
  Enc0.write(1000L); // set counter value as 1000   
}

void loop() {
  Serial.println(Enc0.read()); // read A/B pulse counter
  delay(100);
}