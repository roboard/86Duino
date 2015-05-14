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
  myservo.setPosition(900, 4000);  // set the first target position and moving time: 4s
  myservo.run();                   // servo runs to first target position during 4s
  delay(2000);                     // move 2s
  myservo.pause();                 // pause servo
  delay(2000);                     // pause for 2s
  myservo.resume();                // resume servo
  delay(3000);                     // move 3s, but it will arrive target position and stop when 2s

  myservo.setPosition(2000, 5000);// set the second target position and moving time: 5s
  myservo.run();                   // servo runs to second target position during 5s
  delay(3000);                     // move 3s
  myservo.stop();                  // stop servo
  delay(2000);                     // stop for 2s
  myservo.release();               // free servo
  
  while(1);
}