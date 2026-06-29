/*
 * Note: You need to insert a MicroSD card on 86Duino board for saving 86Duino frame files.
 *       How to build a 86Duino MicroSD card: http://www.86duino.com/?page_id=2844
 */
   
#include <AIServo86.h> 
 
AIServoFrame myframe1;
AIServoFrame myframe2; 

void setup() 
{ 
  myframe1.positions[0] = 100;
  myframe1.positions[1] = 110;
  myframe1.positions[2] = 120;
  myframe1.positions[3] = 130;
  myframe1.positions[4] = 140;
  
  myframe1.save("frame1.txt");
  
  myframe2.positions[0] = 50;
  myframe2.positions[1] = 200;
  myframe2.positions[2] = 250;
  myframe2.positions[3] = 230;
  myframe2.positions[4] = 40;
  
  myframe2.save("frame2.txt");
} 

void loop() 
{
} 