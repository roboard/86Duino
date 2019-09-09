#include <Arduino.h>
#include <Modbus.h>
#include <Ethernet.h>

ModbusMaster bus;
ModbusMasterNode node1;
ModbusMasterNode node2;

ModbusGateway gateway;

byte mac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
IPAddress localIp(192,168,1,102);

void setup()
{
    Ethernet.begin(mac, localIp);
    Serial485.begin(115200);

    gateway.begin(MODBUS_TCP);

    bus.begin(MODBUS_RTU, Serial485);
    
    node1.begin(11, bus);
    node2.begin(12, bus);
    
    gateway.connect(node1);
    gateway.connect(node2);
}

void loop()
{
    gateway.poll();
}
