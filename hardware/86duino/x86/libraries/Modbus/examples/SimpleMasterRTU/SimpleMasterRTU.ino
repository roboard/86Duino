#include <Arduino.h>
#include <Modbus.h>

ModbusMaster bus;
ModbusMasterNode node1;
ModbusMasterNode node2;

int led = 0;
uint32_t value = 0;

void setup()
{
    Serial485.begin(115200);
    
    /* Modbus RTU Mode via RS485. */
    bus.begin(MODBUS_RTU, Serial485);
    
    /* Slave node 1 with ID 11. */
    node1.begin(11, bus);
    /* Slave node 2 with ID 12. */
    node2.begin(12, bus);
}

void loop()
{
    uint16_t reg[2];
    
    /* Write 1 coil to address 0 of the slave with ID 11. */
    node1.writeSingleCoil(0, led);
    
    /* Write 2 word to holding registers address 16 of the slave with ID 11. */
    reg[0] = value & 0xFFFF;
    reg[1] = (value >> 16) & 0xFFFF;
    node1.setTransmitBuffer(0, reg[0]);
    node1.setTransmitBuffer(1, reg[1]);
    node1.writeMultipleRegisters(16, 2);

    /* Read 2 word from holding registers address 16 of the slave with ID 11. */
    node1.readHoldingRegisters(16, 2);
    Serial.print("From Node 1 Holding Register: ");
    value = node1.getResponseBuffer(0) | (node1.getResponseBuffer(1) << 16);
    Serial.println(value);

    /* Read 1 word from input registers address 2 of the slave with ID 11. */
    node1.readInputRegisters(2, 1);
    Serial.print("              Input Register: ");
    Serial.print(node1.getResponseBuffer(0));
    Serial.println();
    
    /* Write 1 coil to address 0 of the slave with ID 12. */
    node2.writeSingleCoil(0, led);
    
    /* Write 2 word to holding registers address 16 of the slave with ID 12. */
    reg[0] = value & 0xFFFF;
    reg[1] = (value >> 16) & 0xFFFF;
    node2.setTransmitBuffer(0, reg[0]);
    node2.setTransmitBuffer(1, reg[1]);
    node2.writeMultipleRegisters(16, 2);

    /* Read 2 word from holding registers address 16 of the slave with ID 12. */
    node2.readHoldingRegisters(16, 2);
    Serial.print("From Node 2 Holding Register: ");
    value = node2.getResponseBuffer(0) | (node2.getResponseBuffer(1) << 16);
    Serial.println(value);

    /* Read 1 word from input registers address 2 of the slave with ID 12. */
    node2.readInputRegisters(2, 1);
    Serial.print("              Input Register: ");
    Serial.print(node2.getResponseBuffer(0));
    Serial.println();
    
    led = !led;
    value++;
    
    delay(1000);
}
