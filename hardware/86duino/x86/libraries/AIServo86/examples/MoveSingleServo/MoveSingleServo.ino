#include <AIServo86.h> 

AIServoPort(ROBOTIS, MX28) bus;
AIServo myservo;
 
void setup() 
{
  bus.begin(Serial1, 3000000);
  myservo.attach(bus, 1);             // attaches the servo with ID 1
} 

void loop() 
{
  myservo.setPosition(320, 2000);     // set the first target position and moving time: 2s
  myservo.run();                      // servo runs to first target position during 2s
  while(myservo.isMoving() == true);  // wait for the servo that move to target position

  myservo.setPosition(80, 1000);      // set the second target position and moving time: 1
  myservo.run();                      // servo runs to second target position during 1s
  while(myservo.isMoving() == true);  // wait for the servo that move to target position
} 