#include <AIServo86.h> 

AIServoPort(ROBOTIS, AX12) bus; // ServoPort(Vendor, Servo), support ROBOTIS AX12, RX28, MX28 and XL320
AIServo myservo;
 
void setup() 
{
  bus.begin(Serial1, 1000000);  // set used port and its baudrate
  myservo.attach(bus, 17);      // attaches the servo with ID 17
  myservo.setOffset(-10);       // set servo offset: -10 degree
  
  myservo.setVelocity(100);     // 100 degree/s
} 

void loop() 
{
  myservo.setPosition(220);          // set the first target position
  myservo.run();                     // servo runs to first target position
  while(myservo.isMoving() == true); // wait for the servo that move to target position

  myservo.setPosition(70);        // set the second target position
  myservo.run();                     // servo runs to second target position
  while(myservo.isMoving() == true); // wait for the servo that move to target position
}