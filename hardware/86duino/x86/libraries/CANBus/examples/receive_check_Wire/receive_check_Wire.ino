#include <CANBus.h>

unsigned char ch;

void setup()
{
    Serial.begin(115200);
    CAN.begin(CAN_500KBPS);
}


void loop()
{
    if (CAN.requestFrom() > 0)         // check if data coming
    {
        while (CAN.available() > 0)
        {
            ch = CAN.read();           // read data
            Serial.print(ch);          // print the data
            Serial.print("\t");
        }
        Serial.println();
    }
}
