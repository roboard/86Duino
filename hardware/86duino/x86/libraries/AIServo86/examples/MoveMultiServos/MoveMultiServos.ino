/*
 * Example: Move 5 servos
 * 
 * Indicate 5 servo pins on 86Duino board to control the robot, and then set each
 * servo position, moving time and then move them at same time.
 * 
 */
   
#include <AIServo86.h> 

AIServoPort(ROBOTIS, AX12) bus; // ServoPort(Vendor, Servo), support ROBOTIS AX12, RX28, MX28 and XL320

AIServo myservo1; 
AIServo myservo2;
AIServo myservo3;
AIServo myservo4;
AIServo myservo5;
 
void setup() 
{ 
  bus.begin(Serial1, 1000000);
  myservo1.attach(bus, 17);  // attaches the servo with ID 21 
  myservo2.attach(bus, 8);   // attaches the servo with ID 22
  myservo3.attach(bus, 9);   // attaches the servo with ID 23
  myservo4.attach(bus, 14);  // attaches the servo with ID 24 
  myservo5.attach(bus, 15);  // attaches the servo with ID 25
} 

void loop() 
{
  // set the first target position and moving time: 2s
  myservo1.setPosition(100, 2000);
  myservo2.setPosition(220, 2000);
  myservo3.setPosition(180, 2000);
  myservo4.setPosition(90,  2000);
  myservo5.setPosition(120, 2000);
  
  aiservoMultiRun();                     // servos run to first target position during 2s
  while(isAIServoMultiMoving() == true); // wait for all servos that move to target position

  // set the second target position and moving time: 1s
  myservo1.setPosition(150, 1000);
  myservo2.setPosition(150, 1000);
  myservo3.setPosition(150, 1000);
  myservo4.setPosition(220, 1000);
  myservo5.setPosition(180, 1000);
  
  aiservoMultiRun();                     // servos run to first target position during 1s
  while(isAIServoMultiMoving() == true); // wait for all servos that move to target position
} 