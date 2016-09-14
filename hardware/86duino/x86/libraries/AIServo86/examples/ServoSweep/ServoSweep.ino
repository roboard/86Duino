#include <AIServo86.h> 

AIServoPort(ROBOTIS, AX12) bus; // ServoPort(Vendor, Servo), support ROBOTIS AX12, RX28, MX28 and XL320

AIServo myservo;  // create servo object to control a servo

int pos = 0;    // variable to store the servo position 
 
void setup() 
{ 
  bus.begin(Serial1, 1000000); // set used port and its baudrate
  myservo.attach(bus, 17);  // attaches the servo with ID 17
} 

void loop() 
{
  for(pos = 0; pos < 180; pos += 1)  // goes from 0 degrees to 180 degrees 
  {                                  // in steps of 1 degree 
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position 
  } 
  for(pos = 180; pos>=1; pos-=1)     // goes from 180 degrees to 0 degrees 
  {                                
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position 
  }
} 