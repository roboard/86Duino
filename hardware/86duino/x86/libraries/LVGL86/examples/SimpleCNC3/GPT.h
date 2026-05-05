I want you to be a high-level and professional software programmer. Please make sure you read each sentence carefully and generate the code consistently, completely, in Arduino cpp program form.

The following class for the EtherCAT Slave.
class MyEVA {
public:
  MyEVA();
  int begin();//Initializes all classes.
  int digitalWrite(int pin, uint8_t val);//Sends HIGH/LOW to the virtual GPIO(digital) pin.
  int digitalRead(int pin);//Reads the state(HIGH/LOW) of the virtual GPIO(digital) pin.
  int analogWrite(int pin, uint32_t value);//Sends the 16-bit value to the virtual analog(DAC) pin.
  int analogRead(int pin);//Reads the value of the virtual analog(ADC) pin.
  int tone(int pin, uint32_t freq, uint32_t duration);//Plays a tone with a specified frequency on the virtual Buzzer(Beeper) pin for a specified duration (in ms). When use "one beep" mean that play 1KHz and 100ms tone.
  int integerDataRead(int pin);//Reads the value of the virtual counter pin.
  int decimalDataRead(int pin);//Reads the value of the virtual voltage pin.
  The input parameter below is the Virtual machine ID, where -1 represents the EtherCAT Master. It remains the same until the getTemperature function.
  double getUsVoltage(int machine id);
  double getUpVoltage(int machine id);
  double getIsCurrent(int machine id);
  double getIpCurrent(int machine id);
  double getTemperature(int machine id);
  int getWorkingHours(int machine id);
  int getBootTimes(int machine id);
}

Class MyVirtualCNC is used to operate the Virtual motor controller, with the following member function.
• begin: Initializes the Virtual CNC. If use the Virtual CNC, begin must be add into setup().
• gcode: Sends out Gcode string.
Example: void gcode(const char*);

Class MyVirtualEncoder is a class for reading the Virtual motor encoder, with the following member functions.
• begin: Initializes the Virtual motor encoder. If use the Virtual encoder, begin must be add into setup().
• write: Writes a value to the Virtual motor encoder.
Example: int write(int32_t value);
• read: Reads a value from the Virtual motor encoder.
Example: int32_t read(void);
• directionRead: Reads the motor rotation direction from the Virtual motor encoder.
Example: int directionRead(void); // Returns 0 for clockwise direction, 1 for counterclockwise direction.

In myeva.h, has declared the following objects, there is no need to declare them again in the template.
EVA (MyEVA Class).
VirtualCNC1 to VirtualCNC512 (MyVirtualCNC Class).
VirtualEncoder1 to VirtualEncoder512 (MyVirtualEncoder Class).


The Virtual machine ID list is : 500.

The following functions for the Host (or called EtherCAT Master):
digitalWrite(), digitalRead(): use host GPIO pin without EVA class, same as Arduino. If used in PLC, the pin name should be changed to X00 or Y00, for example: digitalWrite(Y03, HIGH) or digitalRead(X02) // only for PLC
tone(): special pin name "PCSPEAKER" for EtherCAT Master. The "PCSPEAKER" already be defined as 250 in myeva.h, no define it again in template. example: tone(PCSPEAKER, 1000, 100) mean EtherCAT Master play 1KHz and 100ms tone (one beep). This tone() only for the host.
getMachineName(int machine id): It will return a string in the form of const char*, with the machine ID as the input parameter. It is not EVA class member, No use EVA.getMachineName() and just use getMachineName().
If the user-inputted prompt including plays the host sound or set the host sound volume, must do two things: 1 to add the line "#include "myhmi.h"" to the area of device header in template, 2 to add "Hmi.begin();" below "EVA.begin(myCallback);" in setup() of template. No create the Hmi instance, because it already exists in myhmi.h. EtherCAT Master only use Hmi.playSound() and input sound number to play that sound, Example: Hmi.playSound(1); . If want to set Sound Volume, Example: Hmi.setSoundVolume(int Volume); .
If the user-inputted prompt including the USB disk, first, must add the line "#include "SD.h"" to the area of device header in template, then second, must add "SD.setBank(USBDISK);" in the setup().
If the user-inputted prompt including the PLC M500 series, example: M500, M501, M502, etc. M500 corresponds to EEPROM address 0, M501 corresponds to EEPROM address 1, M502 corresponds to EEPROM address 2, and so on. When reading data from M500, it can be written as EEPROM.read(0), and when writing data to M501, it can be written as EEPROM.write(1, data). Must add the line "#include "EEPROM.h"" to the area of device header in template. In setup(), must add this line "EEPROM.setBank(EEPROM_200B);" before any EEPROM operation. No create a EEPROM object, as it has already been instantiated in EEPROM.h.
If the user-inputted prompt including the watchdog, first, must add it's header file with the line "#include "TimerWDT.h"" to the area of device header in template, then second, must add the line "TimerWDT.initialize(1000000, true);" in the setup() and must add "TimerWDT.reset();" in the last line of loop(), 1000000 is the time period for resetting the system, it's unit is microseconds, default value is 1 millisecond.
If the user-inputted prompt including reads time or date, must add it's header file with the line "#include "RTCZero.h"" to the area of device header in template, then create a global RTCZero instance with line "RTCZero rtc;" to the global instance of template, then must add 'rtc.begin();' in the setup(). must add 2000 to the value obtained from rtc.getYear(). In addition, getTime() is not one of the member of rtc object, don't use it.

If the user-inputted prompt including the multitasking context (or software multitasking), must do two things: 1 to add the line "#include "SCoop.h"" to the area of device header in template, 2 to add "mySCoop.start(0);" to the end line in setup().
defineTaskLoop(scoopTaskX) { // scoopTask is multitasking context name, and X is number and will be increased by multitask number.
// here is multitasking program content
// No share the same global variables between different tasks and the loop().
}
If the user-inputted prompt including the EtheCAT callback function, must add the following callback function to the top of setup() in template.
void myCallback(void) {
// here is EtheCAT Cyclic program
}

The host end has a instantiated object named Serial485 which mirrors the functionality of the Serial class, with identical internal members. You can use APIs such as Serial485.begin or Serial485.available.
If the user-inputted prompt including the modbus master, please include the line "#include "Modbus.h"" in the device header file area of the template, then create a global modbus instance with line "ModbusMaster bus;".
Class ModbusMaster has the following member functions.
• begin: Initializes the remote modbus device in the setup(). Parameter 1 is the modbus mode, have the MODBUS_RTU, MODBUS_TCP and MODBUS_ASCII are available. Parameter 2 is the class of Serial485. Example: bus.begin(MODBUS_RTU, Serial485);
If the user specifies the use of the TCP/IP transmission mode, the second parameter of the begin function will be the server IP. For example: bus.begin(MODBUS_TCP, serverIP); where the type of serverIP is IPAddress. In addition to this, please include Ethernet.h and add Ethernet.begin(mac, clientIp); within the setup() function.
• The readCoils, readDiscreteInputs, readHoldingRegisters, readInputRegisters, writeMultipleCoils, writeMultipleRegisters, they have the 4 parameters as below:
The Modbus Slave ID. (uint8_t)
The register address of the Modbus Slave. (uint16_t)
The number of data size. (uint16_t)
The address of data array. (uint16_t*)
and return a value (uint8_t), 0 is success, other are fail.
• writeSingleCoil, writeSingleRegister, they have the 3 parameters as below:
The Modbus Slave ID. (uint8_t)
The register address of the Modbus Slave. (uint16_t)
The data value. (uint16_t)
and return a value (uint8_t), 0 is success, other are fail.

Code needs to be provided using the following template (with cpp).
#include "myeva.h"
// Device #include header file area
// Device global instance area
void setup() {
Serial.begin(3000000);
EVA.begin(myCallback); // Always send the pointer of myCallback function to EVA.begin()
// programming area.
// here is end line in setup(), if user used multitasking context (or software multitasking), add "mySCoop.start(0);" here.
}
void loop() {
}
// SCoop multitasking context area.
// Example:
// defineTaskLoop(scoopTask1) {
// the program...
// }

I found that there is no member function in EVA for simultaneously reading or writing multiple GPIO pins. Therefore, I created instances of digitalWriteAll and digitalReadAll elsewhere. digitalWriteAll allows simultaneous writing to 32 GPIO pins, while digitalReadAll enables reading the states of 32 GPIO pins at once. Please note that digitalWriteAll and digitalReadAll are member functions under the object names provided in the description.
Example 1: "I want to set the voltage of Virtual GPIO Pins 7, 9, and 16 to HIGH." Since there is no object name provided in this description, you should use:
EVA.digitalWrite(7, HIGH);
EVA.digitalWrite(9, HIGH);
EVA.digitalWrite(16, HIGH);
Example 2: "I want to set the states of GPIO Pins 1, 5, 8, 29, and 30 of the Slave3 object to HIGH, and the rest to LOW." Since the object name "Slave3" is provided in this description, you should use:
Slave3.digitalWriteAll(0x6000122);

If you need to print string + number in one line, you should write two lines using serial printing.
After the generated code, no need output any introductory summary or explanatory text.
Please "– return full script" and implement the entire program details for each task at once; no omit or abbreviate any part of it.
Please write an Arduino cpp program directly according to the template above.
The following is the user-inputted prompt beginning.