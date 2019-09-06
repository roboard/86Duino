/*
  Feed Rate Override
  This demo shows how to implement feed rate override.
  In this example, the machine runs the gcode file from the SD card automatically,
  and we can change the feed rate by spinning the hand wheel encoder.

  Please make sure your machine has enough space to move.
  Please make sure the gcode file named as "auto.gcode" is in the SD card.
*/

#include "Motion86.h"
#include "Encoder.h"
#include "SD.h"

// Create Machine object to control machine which you can use any from machine 0 to 2.
Machine machine(0);

// Stepper driver enable pin.
int EnablePin = 4;

// The gocde file in the SD card storage.
char *filename = "auto.gcode";
File gfile;
char buf[256];
int ptr;
bool working;

// Set the initial feed rate ratio.
volatile double FeedrateRatio = 1.0;

bool enc_triggered = false;

void encoder_callback(int flag);

void setup() {
  while (!Serial);
  SD.begin();

  // In this case, Handwheel Encoder is installed on encoder 3 and encoder mode is set.
  Enc3.begin(MODE_AB_PHASE);
  Enc3.setRange(1); //trigger interrupt every time.
  Enc3.attachInterrupt(encoder_callback);

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

  // We should turn on the machine before control it.
  machine.machineOn();

  // Enable software limit to make control safer.
  machine.enableSoftLimit();

  // Set the feedrate of homing.
  machine.setHomeSpeed(1000, 1000, 200);

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
  // Read and process the gcode file from SD card.
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

  // Override Feedrate when interrupt triggered.
  if (enc_triggered) {
    if (FeedrateRatio > 2.0) FeedrateRatio = 2.0;
    if (FeedrateRatio < 0.1) FeedrateRatio = 0.1;
    machine.feedrateOverride(FeedrateRatio);
    enc_triggered = false;
    Serial.print("FeedrateRatio = ");
    Serial.println(machine.getFeedrateOverride());
  }
}

void encoder_callback(int flag) {
  // Feedrate ratio should more than 0.
  if (flag == INTR_OVERFLOW && FeedrateRatio < 2.0)
    FeedrateRatio += 0.01;
  else if (flag == INTR_UNDERFLOW && FeedrateRatio > 0.0)
    FeedrateRatio -= 0.01;
  enc_triggered = true;
}