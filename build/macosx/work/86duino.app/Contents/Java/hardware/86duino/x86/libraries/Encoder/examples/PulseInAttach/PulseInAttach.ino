/*
  Use encoder capture mode to get the width of a pulse. 
  PIN 9 send PWM signal and use one of PIN 42 43 44 to receive it.
  In this example, connect PIN 9 and PIN 42, you can get
  HIGH & LOW width value of PWM pulse by using attachInterrupt(). 
*/

#include <Encoder.h>

volatile unsigned long num1 = 0L;
volatile unsigned long num2 = 0L;
void myISR(int flag) {
    if(flag == INTR_A_PULSE_LOW)       // If it is LOW pulse width
      num1 = Enc0.read();              // read value and assign to num1
    else if(flag == INTR_A_PULSE_HIGH) // If it is HIGH pulse width
      num2 = Enc0.read();              // read value and assign to num2
}

void setup() {                
  Serial.begin(9600);
  analogWrite(9, 64); // send 1/4 duty PWM = 250us (HIGH width)
  Enc0.begin(MODE_CAPTURE);     
  Enc0.attachInterrupt(myISR);
}

void loop() {
  Serial.print("LOW:");
  Serial.print(num1); // print LOW width (unit is microsecond)
  Serial.print("    ");
  Serial.print("HIGH:");
  Serial.println(num2); // print HIGH width (unit is microsecond)
  delay(100);
}