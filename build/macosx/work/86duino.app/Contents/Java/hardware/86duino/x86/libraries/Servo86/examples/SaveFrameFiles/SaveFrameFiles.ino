/*
 * Note: You need to insert a MicroSD card on 86Duino board for saving 86Duino frame files.
 *       How to build a 86Duino MicroSD card: http://www.86duino.com/?page_id=2844
 */
   
#include <Servo86.h> 
 
ServoFrame myframe1;
ServoFrame myframe2; 

void setup() 
{ 
  myframe1.positions[0] = 1500;
  myframe1.positions[1] = 1350;
  myframe1.positions[2] = 900;
  myframe1.positions[3] = 1600;
  myframe1.positions[4] = 2000;
  
  myframe1.save("frame1.txt");
  
  myframe2.positions[0] = 2000;
  myframe2.positions[1] = 1600;
  myframe2.positions[2] = 1350;
  myframe2.positions[3] = 900;
  myframe2.positions[4] = 1500;
  
  myframe2.save("frame2.txt");
} 

void loop() 
{
} 