/*
  Continue to read A/B pulse by using encoder 0 (PIN 42 43 44). 
  You can also use attachInterrupt() to run ISR when occur certain condition
  (ex. index signal or internal counter is equal to setComparator())
*/

#include <Encoder.h>
 
volatile unsigned long num1 = 0L;
volatile unsigned long num2 = 0L;
void test(int flag) { // ISR
    if(flag == INTR_COMPARE)
      num1++; // if counter value is equal to setComparator(), num1 add one
    else if(flag == INTR_INDEX)
      num2++; // if motor rotate one circle, num2 add one
}
 
void setup() {
  Serial.begin(9600);
  Enc0.begin(MODE_AB_PHASE);
  Enc0.setIndexReset();    // if occur a index signal, ISR will be run
  Enc0.setComparator(20L); // set compatre value as 20, if internal counter value is equal this, ISR will be run
  Enc0.attachInterrupt(test);
}
 
void loop() {
  Serial.print("Pulse number: ");
  Serial.print(Enc0.read()); // read pulse counter
  Serial.print("    ");
  Serial.print("index: ");
  Serial.print(num1); // print the circle numbers
  Serial.print("    ");
  Serial.print("trigger: ");
  Serial.println(num2);
  delay(100);
}