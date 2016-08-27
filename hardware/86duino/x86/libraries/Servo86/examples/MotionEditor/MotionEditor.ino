/*
 * This sketch can receive the commands from 86ME (86Duino Motion Editor),and
 * then control max 45 servos.
 * You can download 86ME here: http://www.86duino.com/?page_id=8923
 * See the 86ME tutorial: http://www.86duino.com/index.php?p=11544
 */

#include <Servo86.h>
#include <Firmata.h>
#include <EEPROM.h>
#include "FreeIMU1.h"
#include <Wire.h>
#include <time.h>

Servo servos[45];
ServoFrame myframe;
ServoFrameKondo mykondo;
unsigned char pin_uesd[45] = {0};
double val[9];
double q[4];
FreeIMU1 my3IMU = FreeIMU1();
bool init_imu = false;
bool intr = false;
unsigned long gyro_update_time = millis();

void currentSysexCallback(byte command, byte argc, byte* argv)
{
  word value = 0;
  word _time = 0;
  word _type = 0;
  word _speed = 0;
  unsigned int i, j, pin;
  unsigned long long used_servos = 0;

  if(command == EXTENDED_ANALOG)
  {
    if(argc == 92) // for 86ME v1.0 and v1.1
    {
      for(i=0, j=0; i<(argc-2); i+=2, j++)
      {
        value = argv[i+1] & 0x7F;
        value = (value << 7) | (argv[i] & 0x7F);
        myframe.positions[j] = value;
      }
      _time = argv[i+1] & 0x7F;
      _time = (_time << 7) | (argv[i] & 0x7F);
      myframe.playPositions((unsigned long)_time);
      if(_time != 0) while(isServoMultiMoving() == true);
    }
    else if(argc == 99)
    {
      for(i=0, j=0; i<90; i+=2, j++)
      {
        value = argv[i+1] & 0x7F;
        value = (value << 7) | (argv[i] & 0x7F);
        myframe.positions[j] = value;
      }
      _time = argv[i+1] & 0x7F;
      _time = (_time << 7) | (argv[i] & 0x7F);
      used_servos = 0;
      for (i=92; i<99; i++)
      {
        used_servos = used_servos << 7;
        used_servos |= (argv[i] & 0x7F);
      }
      myframe.playPositions((unsigned long)_time, used_servos);
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
    unsigned int pin = argv[0];
		
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
    for(i=0; i<5; i++) Serial.write(r[i]);
  }
  else if(command == SERVO_CONFIG)
  {
    if(argc == 1)
      servos[argv[0]].release();
    else
      for(i=0; i<45; i++) servos[i].release();
  }
  else if(command == PIN_STATE_QUERY)
  {
    byte r[9];
    unsigned int pin = argv[0];
    r[0] = 0xF0;
    r[1] = PIN_STATE_RESPONSE;
    r[2] = pin & 0x7F;
    if (pin == 0)
    {
      unsigned long data = millis();
      memcpy(&(r[3]), (unsigned char*)(&data), 4);
      r[7] = 0;
      for(i=3; i<7; i++)
      {
        r[7] = r[7] << 1;
        r[7] |= ((r[i] & 0x80) >> 7);
        r[i] &= 0x7F;
      }
      r[8] = 0xF7;
      for(i=0; i<9; i++) Serial.write(r[i]);
    }
    else if (pin == 1)
    {
      int data = rand();
      memcpy(&(r[3]), (unsigned char*)(&data), 4);
      r[7] = 0;
      for(i=3; i<7; i++)
      {
        r[7] = r[7] << 1;
        r[7] |= ((r[i] & 0x80) >> 7);
        r[i] &= 0x7F;
      }
      r[8] = 0xF7;
      for(i=0; i<9; i++) Serial.write(r[i]);
    }
    else if (pin < 8 && pin >= 2)
    {
      int sensorValue = analogRead(pin + 43);
      r[3] = sensorValue & 0x7F;
      r[4] = (sensorValue >> 7) & 0x7F;
      r[5] = 0xF7;
      for(i=0; i<6; i++) Serial.write(r[i]);
    }
    else if (pin < 13 && pin >= 8)
    {
      float data = (float)(val[pin - 8]);
      memcpy(&(r[3]), (unsigned char*)(&data), 4);
      r[7] = 0;
      for(i=3; i<7; i++)
      {
        r[7] = r[7] << 1;
        r[7] |= ((r[i] & 0x80) >> 7);
        r[i] &= 0x7F;
      }
      r[8] = 0xF7;
      for(i=0; i<9; i++) Serial.write(r[i]);
    }
  }
  else if(command == I2C_CONFIG)
  {
    unsigned int module = argv[0];
    int ret = 0;
    delay(5);
    if(module == 1) // LSM330DLC
    {
      ret = my3IMU.initEX(0);
      if(ret == 0)
        init_imu = true;
      else
        init_imu = false;
    }
    else if(module == 2) // RM-G146
    {
      ret = my3IMU.initEX(2);
      if(ret == 0)
        init_imu = true;
      else
        init_imu = false;
    }
    delay(5);
    byte r[5];
    r[0] = 0xF0;
    r[1] = 0x77;
    r[2] = 0x01;
    r[3] = ret & 0x7F;
    r[4] = 0xF7;
    for(i=0; i<5; i++) Serial.write(r[i]);
  }
  else if(command == I2C_REQUEST)
  {
    byte r[24];
    r[0] = 0xF0;
    r[1] = 0x77;
    r[2] = 0x00;
    if(argv[0] == 0)
    {
      for(i=0; i<4; i++)
      {
        float data = (float)(q[i]);
        memcpy(&(r[i*4 + 3]), (unsigned char*)(&data), 4);
        r[19 + i] = 0;
        for(j=i*4+3; j<i*4+7; j++)
        {
          r[19 + i] = r[19 + i] << 1;
          r[19 + i] |= ((r[j] & 0x80) >> 7);
          r[j] &= 0x7F;
        }
      }
      r[23] = 0xF7;
      for(i=0; i<24; i++) Serial.write(r[i]);
    }
  }
}

void setup()
{
  byte pin;
  Firmata.setFirmwareVersion(0, 2);
  Firmata.attach(EXTENDED_ANALOG, currentSysexCallback);
  Wire.begin();
  for(pin=0; pin<45; pin++) servos[pin].attach(pin);
  Firmata.begin(57600);
  srand(time(NULL));
}

void loop()
{
  if(init_imu && (millis() - gyro_update_time > 28))
  {
    my3IMU.getQ(q, val);
    gyro_update_time = millis();
  }
  if(Firmata.available())
    Firmata.processInput();
}