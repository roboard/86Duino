/*
  Use encoder capture mode to get the width of a pulse. 
  PIN 9 send PWM signal and use one of PIN 42 43 44 to receive it.
  In this example, connect PIN 9 to 42, you can get
  HIGH width value of PWM pulse. 
*/

#include <Encoder.h>

void setup() {                
  Serial.begin(9600);
  analogWrite(9, 64); // send 1/4 duty PWM = 250us (HIGH width)
  Enc0.begin(MODE_CAPTURE);     
}

void loop() {
  Serial.println(Enc0.pulseIn(0, HIGH));
  delay(100);
}