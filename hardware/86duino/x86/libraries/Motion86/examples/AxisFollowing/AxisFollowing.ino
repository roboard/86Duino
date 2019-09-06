/*
  Axis Following

  This demo shows how to implement the axis following similar to Electronic Gearing Application.
  The slave_machine will follow the other machine by the given gear ratio.

  Please make sure your machine has enough space to move.
  Please make sure the gcode file named as "auto.gcode" is in the SD card.
*/

#include "Motion86.h"
#include "SD.h"

// Create Machine object to control machine which you can use any from machine 0 to 2.
// In this case, use machine 0 and machine 1.
Machine machine(0);
Machine slave_machine(1);

// Stepper driver enable pin.
int EnablePin = 4;

// The gocde file in the SD card storage.
char *filename = "auto.gcode";
File gfile;
char buf[256];
int ptr;
bool working;

void setup() {
  while (!Serial);
  pinMode(EnablePin, OUTPUT);

  // You can also reverse the moving direction if needed.
  // In this case, this machine needs to reverse the direction of the x-axis and y-axis.
  machine.config_ReverseDirection(AXIS_X);
  machine.config_ReverseDirection(AXIS_Y);

  // PPU (pulse per unit) is a virtual length unit.
  // In this case, it is 80 pulses per millimeter for x-axis.
  machine.config_PPU(AXIS_X, 80.0);
  machine.config_PPU(AXIS_Y, 80.0);
  machine.config_PPU(AXIS_Z, 1600.0);

  // Config PosLimit to set the software limit for the machine motion.
  machine.config_PosLimit(AXIS_X, 0, 300);
  machine.config_PosLimit(AXIS_Y, 0, 200);
  machine.config_PosLimit(AXIS_Z, 0, 300);

  // Config HomePins to set the hardware limit switch pins.
  machine.config_HomePins(2, 7, 8);

  // In this case, this slave_machine needs to reverse the direction of the y-axis.
  slave_machine.config_ReverseDirection(AXIS_Y);

  // Config PosLimit to set the software limit for the slave_machine motion.
  slave_machine.config_PPU(AXIS_X, 80.0);
  slave_machine.config_PPU(AXIS_Y, 80.0);
  slave_machine.config_PPU(AXIS_Z, 3200.0);

  // Config PosLimit to set the software limit for the slave_machine motion.
  slave_machine.config_PosLimit(AXIS_X, 0, 200);
  slave_machine.config_PosLimit(AXIS_Y, 0, 200);
  slave_machine.config_PosLimit(AXIS_Z, 0, 300);

  // Config HomePins to set the hardware limit switch pins for slave_machine.
  slave_machine.config_HomePins(21, 22, 23);

  // Config slave_machine into machine's EGearSlave, it means the slave_machine will calculate gear ratio and follow the machine's contorl.
  machine.config_EGearSlave(slave_machine, 1.5, 0.5, 1.0);

  // We should turn on the machine before control it.
  machine.machineOn();

  // Enable software limit to make control safer.
  machine.enableSoftLimit();

  // Set the feedrate of homing.
  machine.setHomeSpeed(1000, 1000, 200);
  slave_machine.setHomeSpeed(1000, 1000, 100);

  // Open if there is gcode file in the SD card.
  if (SD.exists(filename)) {
    gfile = SD.open(filename);
    working = true;
  } else {
    Serial.print("File not exists: ");
    Serial.println(filename);
    while (1);
  }

  // Enable stepper driver.
  digitalWrite(EnablePin, LOW);

  // Machine homing.
  machine.home();
}

void loop() {
  // Read and process the gcode file from SD card
  // slave_machine will zoom gcode path more wider (x1.5) and shorter (x0.5).
  if (working && !machine.isCmdBufferFull()) {
    ptr = 0;
    while (gfile.available()) {
      buf[ptr] = gfile.read();
      if (buf[ptr] == '\n') {
        buf[ptr + 1] = '\0';
        machine.gcode(buf);
        break;
      } else {
        ptr ++;
      }
    }
    if (!gfile.available())
    {
      Serial.println("GCODE FINISH");
      gfile.close();
      working = false;
    }
  }
}