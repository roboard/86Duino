/*
  Multiple Machines

  This demo demonstrates how to config machine from serial monitor command line.
  The instruction and the example of command format are shown as below.

  Instruction:
    1. open serial monitor.
    2. First parameter should be machine group id.
    3. Second parameter can be Init, Close, Home, Pos, Pin, Gfile or Gcode command.
    4. If you use Gfile to read file, you should add third parameter as gcode filename in SD card.

  Example:
    0 INIT
    1 INIT
    0 HOME
    1 HOME
    0 GFILE auto.gcode
    1 GFILE auto2.gcode

  Please make sure your machine has enough space to move.
  Please make sure the gcode file named as "auto.gcode" is in the SD card.
*/

#include "Motion86.h"
#include "SD.h"

// Array of pointer to Machine Objects and you can have up to 3 machines.
Machine *machine[3];

// Stepper driver enable pin.
int EnablePin = 4;

// The specific gocde files for each machine in the SD card storage.
File gfile[3];
bool fileopened[3] = {false, false, false};
char *filename;
char buf[256];
int ptr;

double ActualPos[3];

// PPU (pulse per unit) is a virtual length unit.
// Set the pulses per millimeter for each axis of each machine.
double AxisPPU[3][3] = {{80.0, 80.0, 1600.0},
                        {80.0, 80.0, 3200.0},
                        {80.0, 80.0, 3200.0}};

// You can also reverse the moving direction if needed.
// Set 1 to reverse and 0 for not to reverse for each axis of each machine.
int MotorDirInvert[3][3] = {{1, 1, 0},
                            {0, 1, 0},
                            {0, 0, 1}};

// Set the minimum software position limit for each axis of each machine.
int MotorPosLimitMin[3][3] = {{0, 0, 0},
                              {0, 0, 0},
                              {0, 0, 0}};

// Set the maximum software position limit for each axis of each machine.
int MotorPosLimitMax[3][3] = {{300, 200, 300},
                              {200, 200, 300},
                              {200, 200, 300}};

// Set the hardware limit switch pins for each axis of each machine.
int Limit_PinMap[3][3] = {{ 2,  7,  8},
                          {21, 22, 23},
                          {24, 25, 26}};

void processingCMD(int group, char *command);

void setup() {
  while (!Serial);
  SD.begin();
  pinMode(EnablePin, OUTPUT);
  // Enable stepper driver.
  digitalWrite(EnablePin, LOW);
}

void loop() {
  ptr = 0;
  // Read and process if receive command from serial.
  while (Serial.available() > 0) {
    buf[ptr] = (char)Serial.read();
    if (buf[ptr] == '\n') {
      buf[ptr] = '\0';
      Serial.println(buf);
      processingCMD(atoi(buf), &buf[2]);
      break;
    } else {
      ptr ++;
    }
  }

  // Check every machine if they are valid or not.
  for (int i = 0; i < 3; i++) {
    if (machine[i] == NULL) continue;
    if (!machine[i]->isValid()) continue;
    // Read and process the gcode file from SD card
    if (fileopened[i] && !machine[i]->isCmdBufferFull())
    {
      ptr = 0;
      while (gfile[i].available()) {
        buf[ptr] = gfile[i].read();
        if (buf[ptr] == '\n') {
          buf[ptr + 1] = '\0';
          machine[i]->gcode(buf);
          break;
        } else {
          ptr ++;
        }
      }
      if (!gfile[i].available()) {
        Serial.println("GCODE FINISH");
        gfile[i].close();
        fileopened[i] = false;
      }
    }
  }
}

void processingCMD(int group, char *command) {
  if (group < 0 || group > 2) return;
  if (stricmp(command, "INIT") == 0) {
    if (machine[group] != NULL) return;
    // Construct a new machine.
    machine[group] = new Machine(group);
    // Reverse the moving direction for each axis of each machine if needed.
    if (MotorDirInvert[group][AXIS_X]) machine[group]->config_ReverseDirection(AXIS_X);
    if (MotorDirInvert[group][AXIS_Y]) machine[group]->config_ReverseDirection(AXIS_Y);
    if (MotorDirInvert[group][AXIS_Z]) machine[group]->config_ReverseDirection(AXIS_Z);
    // Set the pulses per millimeter for each axis of each machine.
    machine[group]->config_PPU(AXIS_X, AxisPPU[group][AXIS_X]);
    machine[group]->config_PPU(AXIS_Y, AxisPPU[group][AXIS_Y]);
    machine[group]->config_PPU(AXIS_Z, AxisPPU[group][AXIS_Z]);
    // Set the software position limit for each axis of each machine.
    machine[group]->config_PosLimit(AXIS_X, MotorPosLimitMin[group][AXIS_X], MotorPosLimitMax[group][AXIS_X]);
    machine[group]->config_PosLimit(AXIS_Y, MotorPosLimitMin[group][AXIS_Y], MotorPosLimitMax[group][AXIS_Y]);
    machine[group]->config_PosLimit(AXIS_Z, MotorPosLimitMin[group][AXIS_Z], MotorPosLimitMax[group][AXIS_Z]);
    // Set the hardware limit switch pins for each axis of each machine.
    machine[group]->config_HomePins(Limit_PinMap[group][AXIS_X], Limit_PinMap[group][AXIS_Y], Limit_PinMap[group][AXIS_Z]);
    // We should turn on the machine before control it.
    machine[group]->machineOn();
    return;
  }

  if (machine[group] == NULL) return;
  if (!machine[group]->isValid()) return;

  if (stricmp(command, "CLOSE") == 0) {
    // Deconstructor.
    delete machine[group];
    machine[group] = NULL;
  } else if (stricmp(command, "HOME") == 0) {
    // Set the feedrate of homing.
    machine[group]->setHomeSpeed(1000, 1000, 200);
    // Machine homing.
    machine[group]->home();
  } else if (stricmp(command, "EMG") == 0) {
    // Emergency stop.
    machine[group]->emgStop();
    if (fileopened[group]) {
      gfile[group].close();
      fileopened[group] = false;
    }
  } else if (stricmp(command, "~EMG") == 0) {
    // Clear emergency stop status.
    machine[group]->clearEMGStop();
  } else if (stricmp(command, "POS") == 0) {
    // Get actual position.
    machine[group]->getActualPos(ActualPos[AXIS_X], ActualPos[AXIS_Y], ActualPos[AXIS_Z]);
    Serial.print("X:");
    Serial.print(ActualPos[AXIS_X]);
    Serial.print(", Y:");
    Serial.print(ActualPos[AXIS_Y]);
    Serial.print(", Z:");
    Serial.println(ActualPos[AXIS_Z]);
  } else if (stricmp(command, "PIN") == 0) {
    // Get status of hardware limit switches.
    Serial.println(digitalRead(Limit_PinMap[group][AXIS_X]));
    Serial.println(digitalRead(Limit_PinMap[group][AXIS_Y]));
    Serial.println(digitalRead(Limit_PinMap[group][AXIS_Z]));
  } else if (stricmp(command, "GFILE") == 32) {
    // Decode parameters to open the gcode file.
    filename = &command[6];
    if (fileopened[group]) {
      Serial.println("Already running.");
    } else if (SD.exists(filename)) {
      gfile[group] = SD.open(filename);
      fileopened[group] = true;
    } else {
      Serial.print("File not exists: ");
      Serial.println(filename);
    }
  } else {
    // Other commands are treated as gcode.
    machine[group]->gcode(command);
  }
}