#include <Arduino.h>
#include <Modbus86.h>

ModbusSlave bus;
ModbusSlaveNode node;

uint32_t command = 0;
uint32_t lasttime = 0;

uint8_t read_holding_registers(uint8_t function, uint16_t address, uint16_t length)
{
    for (int i = address; i < address + length; i++) {
        if (i == 16)
            node.writeHoldingRegister(i, command & 0xFFFF);
        else if (i == 17)
            node.writeHoldingRegister(i, (command >> 16) & 0xFFFF);
    }
    
    return MODBUS_SUCCESS;
}

uint8_t read_input_registers(uint8_t function, uint16_t address, uint16_t length)
{
    int sensorPin = A2;

    if (address <= 2 && address + length > 2)
        node.writeInputRegister(2, analogRead(sensorPin));
    
    return MODBUS_SUCCESS;
}

uint8_t write_multiple_registers(uint8_t function, uint16_t address, uint16_t length)
{
    int i;

    for (i = address; i < address + length; i++) {
        if (i == 16)
            command = (command & 0xFF00) | node.readHoldingRegister(i);
        else if (i == 17)
            command = (command & 0x00FF) | (node.readHoldingRegister(i) << 16);
    }

    return MODBUS_SUCCESS;
}

uint8_t write_single_coil(uint8_t function, uint16_t address, uint16_t length)
{
    uint16_t value;
    
    if (address == 0) {
        node.readCoil(address, 1, &value);
        if (value)
            digitalWrite(LED_BUILTIN, HIGH);
        else
            digitalWrite(LED_BUILTIN, LOW);
    }

    return MODBUS_SUCCESS;
}

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    
    Serial485.begin(115200);
    
    /* Modbus RTU Mode via RS485. */
    bus.begin(MODBUS_RTU, Serial485);
    
    /* Slave node with ID 11. */
    node.begin(11, bus);

    /* Set the callback function of Read Holding Registers (0x03). */
    node.cbFunc[MODBUS_CB_READ_HOLDING_REGISTERS] = read_holding_registers;

    /* Set the callback function of Read Input Registers (0x04). */
    node.cbFunc[MODBUS_CB_READ_INPUT_REGISTERS] = read_input_registers;
    
    /* Set the callback function of Write Single Coil (0x05). */
    node.cbFunc[MODBUS_CB_WRITE_SINGLE_COIL] = write_single_coil;
    
    /* Set the callback function of Write Multiple Registers (0x10). */
    node.cbFunc[MODBUS_CB_WRITE_MULTIPLE_REGISTERS] = write_multiple_registers;
}

void loop()
{
    uint32_t now;
    
    node.poll();
    
    now = millis();
    if (now - lasttime > 500) {
        lasttime = now;
        Serial.print("commnad = ");
        Serial.println(command);
    }
}
