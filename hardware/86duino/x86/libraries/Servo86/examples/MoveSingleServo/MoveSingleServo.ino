#include <Servo86.h> 
 
Servo myservo;
 
void setup() 
{ 
  myservo.attach(21);           // attaches the servo on pin 21
  myservo.setPosition(1500);    // set servo to HOME position
  myservo.run();                // servo runs to HOME position
  delay(1000);                  // wait 1s
} 

void loop() 
{
  myservo.setPosition(1600, 2000);    // set the first target position and moving time: 2s
  myservo.run();                      // servo runs to first target position during 2s
  while(myservo.isMoving() == true);  // wait for the servo that move to target position

  myservo.setPosition(900, 1000);    // set the second target position and moving time: 1
  myservo.run();                      // servo runs to second target position during 1s
  while(myservo.isMoving() == true);  // wait for the servo that move to target position
} 