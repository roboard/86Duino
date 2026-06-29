/*
 * Note: You need to insert a MicroSD card on 86Duino board for loading 86Duino frame files.
 *       How to build a 86Duino MicroSD card: http://www.86duino.com/?page_id=2844
 * 
 *       First upload the example "SaveFrameFile" to 86Duino board, and then upload this sketch.
 *       Otherwise, this sketch is not really effective.  
 */

#include <Servo86.h> 

Servo myservo1;
Servo myservo2;
Servo myservo3;
Servo myservo4;
Servo myservo5;
 
ServoFrame myframe1("frame1.txt");
ServoFrame myframe2("frame2.txt"); 

void setup() 
{
  myservo1.attach(21);  // attaches the servo on pin 21 
  myservo2.attach(22);  // attaches the servo on pin 22
  myservo3.attach(23);  // attaches the servo on pin 23
  myservo4.attach(24);  // attaches the servo on pin 24 
  myservo5.attach(25);  // attaches the servo on pin 25
} 

void loop() 
{
  myframe1.playPositions(1000);
  while(isServoMultiMoving() == true);
  
  myframe2.playPositions(1000);
  while(isServoMultiMoving() == true);
} 