/*
  JOG Control
  This example implements the jog control on machine.

  Please make sure your machine has enough space to move.
*/

#include "Motion86.h"

// Create Machine object to control machine which you can use any from machine 0 to 2.
Machine machine(0);

// Stepper driver enable pin.
int EnablePin = 4;

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

  // We should turn on the machine before control it.
  machine.machineOn();

  // Machine is set to jog mode and pins are connected to the buttons to trigger jog.
  machine.beginJog(27, 28, 32, 3, 16, 17, false);

  // IF set machine to Inc Jog mode, you should config jog offset to set how many millimeters per click.
  // machine.beginJog(27, 28, 32, 3, 16, 17, true);
  // machine.setJogOffset(1.0);

  // Jog mode feedrate.
  machine.setJogSpeed(600);

  // Enable stepper driver.
  digitalWrite(EnablePin, LOW);
}

void loop() {
  Serial.print("Jog position = ");
  Serial.print(machine.getJogPos(AXIS_X));
  Serial.print(", ");
  Serial.print(machine.getJogPos(AXIS_Y));
  Serial.print(", ");
  Serial.println(machine.getJogPos(AXIS_Z));
  delay(500);
}