#include <CANBus.h>

unsigned char buf[8] = {0, 1, 2, 3, 4, 5, 6, 7};

void setup()
{
    Serial.begin(115200);
    CAN.begin(CAN_500KBPS);
}

void loop()
{
    CAN.sendMsgBuf(0x00, CAN_STDID, 8, buf);   // send data:  id = 0x00, 
                                               // standrad frame,
                                               // data len = 8, 
                                               // buf: data buffer
    delay(10);
}
