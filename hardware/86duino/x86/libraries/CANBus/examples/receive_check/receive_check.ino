#include <CANBus.h>

unsigned char len = 0;
unsigned char buf[8];


void setup()
{
    Serial.begin(115200);
    CAN.begin(CAN_500KBPS);
}


void loop()
{
    if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        for(int i = 0; i<len; i++)    // print the data
        {
            Serial.print(buf[i]);Serial.print("\t");
        }
        Serial.println();
    }
}
