// Controlling a servo position using a potentiometer (variable resistor) 
// by Michal Rinott <http://people.interaction-ivrea.it/m.rinott> 

#include <Servo.h> 
 
Servo S1;
Servo S2;
Servo S3;
Servo S4; 
 
int potpin = 0;  // analog pin used to connect the potentiometer
int val;    // variable to read the value from the analog pin 
 
void setup() 
{ 
  S1.attach(21);  // attaches the servo on pin 9 to the servo object 
  S2.attach(22);
  S3.attach(23);
  S4.attach(24);
} 
 
void loop() 
{ 
  S1.write(1000);
  S2.write(1900);
  S3.write(1600);
  S4.write(2000); 
} 
