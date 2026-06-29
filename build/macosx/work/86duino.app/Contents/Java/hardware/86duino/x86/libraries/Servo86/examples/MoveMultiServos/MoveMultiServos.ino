/*
 * Example: Move 5 servos
 * 
 * Indicate 5 servo pins on 86Duino board to control the robot, and then set each
 * servo position, moving time and then move them at same time.
 * 
 */
   
#include <Servo86.h> 
 
Servo myservo1; 
Servo myservo2;
Servo myservo3;
Servo myservo4;
Servo myservo5;
 
void setup() 
{ 
  myservo1.attach(21);  // attaches the servo on pin 21 
  myservo2.attach(22);  // attaches the servo on pin 22
  myservo3.attach(23);  // attaches the servo on pin 23
  myservo4.attach(24);  // attaches the servo on pin 24 
  myservo5.attach(25);  // attaches the servo on pin 25
  
  // set servos to HOME positions
  myservo1.setPosition(1500);
  myservo2.setPosition(1500);
  myservo3.setPosition(1500);
  myservo4.setPosition(1500);
  myservo5.setPosition(1500);
  
  servoMultiRun();      // servos run to HOME position
  delay(1000);          // wait 1s
} 

void loop() 
{
  // set the first target position and moving time: 2s
  myservo1.setPosition(1600, 2000);
  myservo2.setPosition(2200, 2000);
  myservo3.setPosition(1800, 2000);
  myservo4.setPosition(900, 2000);
  myservo5.setPosition(1200, 2000);
  
  servoMultiRun();                     // servos run to first target position during 2s
  while(isServoMultiMoving() == true); // wait for all servos that move to target position

  // set the second target position and moving time: 1s
  myservo1.setPosition(1500, 1000);
  myservo2.setPosition(1500, 1000);
  myservo3.setPosition(1500, 1000);
  myservo4.setPosition(2200, 1000);
  myservo5.setPosition(1800, 1000);
  
  servoMultiRun();                     // servos run to first target position during 1s
  while(isServoMultiMoving() == true); // wait for all servos that move to target position
} 