/*
  Basic Move

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
  machine.setDefaultFeedrate(400);

  // Enable stepper driver.
  digitalWrite(EnablePin, LOW);
}

void loop() {
  // Move left to (-10, 0).
  machine.line(-10, 0, 0);

  // Move clockwise to (0, 10) with a radius of 10 in a 90 degree arc.
  machine.arcXY(10, 0, 10, true);

  // Move down to (0, -10).
  machine.line(0, -10, 0);

  // Move counterclockwise to (10, 0) with a radius of 10 in a 90 degree arc.
  machine.arcXY(10, 10, 0, false);

  // Move left to (0, 0).
  machine.line(0, 0, 0);

  // Wait until the movement is finished.
  while (machine.isMoving());
}