/*
  Basic Move by using Gcode

  This demo shows how to control the machine easily by Motion86 Library.
  The path of this example is shown below:
  Start at the '+' on the graph (0, 0), then follow the arrows to move.
                   10
            ,------>
          /        |
          |        |
      -10 <--------+--------^ 10
                   |        |
                   |       /
                   v------'
                  -10

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

  // Enable stepper driver.
  digitalWrite(EnablePin, LOW);
}

void loop() {
  // Move left to (-10, 0).
  machine.gcode("G1 X-10 Y0 F400");

  // Move clockwise to (0, 10) with a radius of 10 in a 90 degree arc.
  machine.gcode("G2 X0 Y10 R10 F400");

  // Move down to (0, -10).
  machine.gcode("G1 X0 Y-10 F400");

  // Move counterclockwise to (10, 0) with a radius of 10 in a 90 degree arc.
  machine.gcode("G3 X10 Y0 R10 F400");

  // Move left to (0, 0).
  machine.gcode("G1 X0 Y0 F400");

  // Wait until the movement is finished.
  while (machine.isMoving());
}