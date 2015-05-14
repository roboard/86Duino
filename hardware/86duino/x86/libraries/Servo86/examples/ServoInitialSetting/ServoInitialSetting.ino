#include <Servo86.h> 
 
Servo myservo;
 
void setup() 
{ 
  myservo.attach(21, 544, 2400);// attaches the servo on pin 21 and set min duty: 544us, max duty: 2400us
  myservo.setOffset(-10);       // set servo offset: -10us
  myservo.setPosition(1500);    // set servo to HOME position (90 degree)
  myservo.run();                // servo runs to HOME position
  delay(1000);                  // wait 1s
  
  myservo.setVelocity(500);     // 500 us/s
} 

void loop() 
{
  myservo.setPosition(900);          // set the first target position
  myservo.run();                     // servo runs to first target position
  while(myservo.isMoving() == true); // wait for the servo that move to target position

  myservo.setPosition(2000);        // set the second target position
  myservo.run();                     // servo runs to second target position
  while(myservo.isMoving() == true); // wait for the servo that move to target position
}