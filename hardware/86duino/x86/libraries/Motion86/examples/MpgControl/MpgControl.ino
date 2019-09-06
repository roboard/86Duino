/*
  MPG Handwheel control
  This example implements the MPG control on machine.

  Please make sure your machine has enough space to move.
*/

#include "Motion86.h"
#include "Encoder.h"

// Create Machine object to control machine which you can use any from machine 0 to 2.
Machine machine(0);

// Stepper driver enable pin.
int EnablePin = 4;

void setup() {

  while (!Serial);
  // In this case, Handwheel Encoder is installed on encoder 3 and encoder mode is set.
  Enc3.begin(MODE_AB_PHASE);
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

  // Use encoder 3 to trigger Mpg control.
  machine.beginMpg(Enc3);

  // Choose which axis you want to control by Mpg Hand wheel.
  machine.setMpgAxis(AXIS_X);
  // machine.setMpgAxis(AXIS_Y);
  // machine.setMpgAxis(AXIS_Z);

  // Mpg mode feedrate.
  machine.setMpgSpeed(600);

  // Mpg ratio is the ratio between the hand wheel and the moving distance of machine, it should more than 0.
  machine.setMpgRatio(1);

  // Enable stepper driver.
  digitalWrite(EnablePin, LOW);
}

void loop() {
  Serial.print("Mpg position = ");
  Serial.print(machine.getJogPos(AXIS_X));
  Serial.print(", ");
  Serial.print(machine.getJogPos(AXIS_Y));
  Serial.print(", ");
  Serial.println(machine.getJogPos(AXIS_Z));
  delay(500);
}