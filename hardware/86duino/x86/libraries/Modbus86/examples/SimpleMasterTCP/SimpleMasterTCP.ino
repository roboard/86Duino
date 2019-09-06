#include <Arduino.h>
#include <Ethernet.h>
#include <Modbus86.h>

ModbusMaster bus;
ModbusMasterNode node;

byte mac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
IPAddress localIp(192,168,1,101);
IPAddress serverIp(192,168,1,102);

int led = 0;
uint32_t value = 0;

void setup()
{
    Ethernet.begin(mac, localIp);
    
    /* Modbus TCP Mode via Ethernet. */
    bus.begin(MODBUS_TCP, serverIp);
    
    /* Slave node initialize. */
    node.begin(11, bus);
}

void loop()
{
    uint16_t reg[2];
    
    /* Write 1 coil to address 0 of the slave with ID 11. */
    node.writeSingleCoil(0, led);
    
    /* Write 2 word to holding registers address 16 of the slave with ID 11. */
    reg[0] = value & 0xFFFF;
    reg[1] = (value >> 16) & 0xFFFF;
    node.setTransmitBuffer(0, reg[0]);
    node.setTransmitBuffer(1, reg[1]);
    node.writeMultipleRegisters(16, 2);

    /* Read 2 word from holding registers address 16 of the slave with ID 11. */
    node.readHoldingRegisters(16, 2);
    Serial.print("From Node 1 Holding Register: ");
    value = node.getResponseBuffer(0) | (node.getResponseBuffer(1) << 16);
    Serial.println(value);

    /* Read 1 word from input registers address 2 of the slave with ID 11. */
    node.readInputRegisters(2, 1);
    Serial.print("              Input Register: ");
    Serial.print(node.getResponseBuffer(0));
    Serial.println();
    
    led = !led;
    value++;
    
    delay(1000);
}
