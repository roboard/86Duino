/*
 * This sketch can receive the commands from 86ME (86Duino Motion Editor),and
 * then control max 45 servos.
 * You can download 86ME here: http://www.86duino.com/?page_id=8923
 * See the 86ME tutorial: http://www.86duino.com/index.php?p=11544
 * 
 */
      
#include <Servo86.h>
#include <Firmata.h>

Servo servos[45];
ServoFrame myframe;
ServoFrameKondo mykondo;
unsigned char pin_uesd[45] = {0};

void currentSysexCallback(byte command, byte argc, byte*argv)
{
  word value = 0;
  word _time = 0;
  word _type = 0;
  word _speed = 0;
  unsigned int i, j, pin;

  if(command == EXTENDED_ANALOG)
  {
    if(argc > 45) // for 86ME v1.0 and v1.1
    {
      for(i=0, j=0; i<(argc-2); i+=2, j++)
      {
        value = (argv[i+1] << 7) + argv[i];
        myframe.positions[j] = value;
      }
      _time = (argv[i+1] << 7) + argv[i];
      myframe.playPositions(_time);
      if(_time != 0) while(isServoMultiMoving() == true);
    }
    else // for 86ME v1.2
    {
      _type = (argv[1] << 7) + argv[0];
      if(_type == 1) // set speed
      {
        _speed = (argv[3] << 7) + argv[2];
	for(i=0; i<45; i++) servos[i].setVelocity(_speed);
      }
    }
  }
  else if(command == ANALOG_MAPPING_QUERY)
  {
    byte r[5];
    unsigned int pin = argv[0], i;
		
    for(i=0; i<45; i++) servos[i].release();
		
    r[0] = 0xF0;
    r[1] = ANALOG_MAPPING_RESPONSE;
    mykondo.positions[pin] = mykondo.capture(servos[pin]);
	if(mykondo.positions[pin] == 0L)
    {
      r[2] = 0;
      r[3] = 0;
    }
    else
    {
      r[2] = mykondo.positions[pin] & 0x7f;
      r[3] = (mykondo.positions[pin] >> 7) & 0x7f;
    }
    r[4] = 0xF7;
    Serial.write(r[0]);
    Serial.write(r[1]);
    Serial.write(r[2]);
    Serial.write(r[3]);
    Serial.write(r[4]);
  }
  else if(command == SERVO_CONFIG)
  {
    for(i=0; i<45; i++) servos[i].release();
  }
}

void setup() 
{
  byte pin;

  Firmata.setFirmwareVersion(0, 2);
  Firmata.attach(EXTENDED_ANALOG, currentSysexCallback);

  for(pin=0; pin<45; pin++) servos[pin].attach(pin);
  Firmata.begin(57600);
}

void loop() 
{
    while(Firmata.available())
      Firmata.processInput();
}
