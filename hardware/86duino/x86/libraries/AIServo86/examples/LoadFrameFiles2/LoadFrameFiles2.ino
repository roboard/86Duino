/*
 * Note: You need to insert a MicroSD card on 86Duino board for loading 86Duino frame files.
 *       How to build a 86Duino MicroSD card: http://www.86duino.com/?page_id=2844
 * 
 *       First upload the example "SaveFrameFile" to 86Duino board, and then upload this sketch.
 *       Otherwise, this sketch is not really effective.  
 */

#include <AIServo86.h> 

AIServo myservo1;
AIServo myservo2;
AIServo myservo3;
AIServo myservo4;
AIServo myservo5;
 
AIServoFrame myframe1("frame1.txt");
AIServoFrame myframe2("frame2.txt"); 

AIServoPort(ROBOTIS, AX12) bus; // ServoPort(Vendor, Servo), support ROBOTIS AX12, RX28, MX28 and XL320

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
  myframe1.playPositions(1000);
  while(isAIServoMultiMoving() == true);
  
  myframe2.playPositions(1000);
  while(isAIServoMultiMoving() == true);
} 