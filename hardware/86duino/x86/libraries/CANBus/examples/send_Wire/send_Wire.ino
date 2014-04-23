#include <CANBus.h>

unsigned char buf[8] = {0, 1, 2, 3, 4, 5, 6, 7};

void setup()
{
    Serial.begin(115200);
    CAN.begin(CAN_500KBPS);
}

void loop()
{
    CAN.beginTransmission(0x00, CAN_STDID);    // id = 0x00, standrad frame
    CAN.write(buf, 8);                         // buf: data buffer, data len = 8, 
    CAN.endTransmission();                     // send data
    delay(10);
}
