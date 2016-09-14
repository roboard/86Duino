#include <AIServo86.h> 
 
AIServo myservo;
AIServoPort(ROBOTIS, AX12) bus; // ServoPort(Vendor, Servo), support ROBOTIS AX12, RX28, MX28 and XL320

unsigned char ServoID = 17;

void setup() 
{
  bus.begin(Serial1, 1000000);  // set used port and its baudrate
  myservo.attach(bus, ServoID); // attaches the servo on pin 21
  myservo.setPosition(150);     // set servo to HOME position
  myservo.run();                // servo runs to HOME position
  delay(1000);                  // wait 1s
} 

void loop() 
{
  myservo.setPosition(300, 2000);  // set the first target position and moving time: 2s
  myservo.run();                   // servo runs to first target position during 2s
  delay(1000);                     // move 1s
  myservo.pause();                 // pause servo
  delay(1000);                     // pause for 1s
  myservo.resume();                // resume servo
  delay(2000);                     // move 2s, but it will arrive target position and stop when 1s

  myservo.setPosition(100, 5000);  // set the second target position and moving time: 5s
  myservo.run();                   // servo runs to second target position during 5s
  delay(3000);                     // move 3s
  myservo.stop();                  // stop servo
  delay(2000);                     // stop for 2s
  myservo.release();               // free servo
  
  while(1);
}