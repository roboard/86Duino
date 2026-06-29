#include "myeva.h"
#include "myhmi.h"
#include "Ethercat.h"

int distance = 1, feedrate = 400;

void incJogMove(char axis, bool positive);

void setup() {
  EVA.begin();
  Hmi.begin();

  // Set up FileBrowser filter and disk
  Hmi.setFileBrowserFilter(p2fb1, "*.gcode");
  Hmi.setFileBrowserDisk(p2fb1, USBDISK);
}

void loop() {
  BEGIN_HMI_EVENT_PROC
  {
    // Button clicks for page navigation
    if (Hmi.buttonClicked(p2b1) || Hmi.buttonClicked(p6b1))
      Hmi.gotoPage(p1);
    else if (Hmi.buttonClicked(p1b7) || Hmi.buttonClicked(p6b2))
      Hmi.gotoPage(p2);
    else if (Hmi.buttonClicked(p1b14) || Hmi.buttonClicked(p2b7))
      Hmi.gotoPage(p6);

    // IncJog movement and distance/feedrate setting
    if (Hmi.buttonClicked(p1b1))
      incJogMove('X', true);
    else if (Hmi.buttonClicked(p1b2))
      incJogMove('X', false);
    else if (Hmi.buttonClicked(p1b3))
      incJogMove('Y', true);
    else if (Hmi.buttonClicked(p1b4))
      incJogMove('Y', false);
    else if (Hmi.buttonClicked(p1b5))
      incJogMove('Z', true);
    else if (Hmi.buttonClicked(p1b6))
      incJogMove('Z', false);

    if (Hmi.dropDownListItemSelected(p1ddl1))
    {
      switch (Hmi.getDropDownListSelectedItem(p1ddl1))
      {
        case 0: distance = 1; break;
        case 1: distance = 10; break;
        case 2: distance = 100; break;
      }
    }
    else if (Hmi.dropDownListItemSelected(p1ddl2))
    {
      switch (Hmi.getDropDownListSelectedItem(p1ddl2))
      {
        case 0: feedrate = 400; break;
        case 1: feedrate = 200; break;
        case 2: feedrate = 100; break;
      }
    }

    // Start homing process
    if (Hmi.buttonClicked(p1b8))
    {
      Slave0.machineSetHomingSpeed(ECAT_MACHINE_X_AXIS, 100);
      Slave0.machineSetHomingSpeed(ECAT_MACHINE_Y_AXIS, 100);
      Slave0.machineSetHomingSpeed(ECAT_MACHINE_Z_AXIS, 100);

      Slave0.machineStartHoming();

      while (!Slave0.machineIsHomingAttained())
      {
        Hmi.showMessageBox("Please wait until machine is homing attained.");
      }
    }

    // Update emergency stop button toggle state
    if (Hmi.pageEnter(p1) || Hmi.pageEnter(p4))
    {
      Hmi.setToggleButtonState(p1tb1, !Slave0.machineIsEmergencyStopped());
      Hmi.setToggleButtonState(p4tb1, !Slave0.machineIsEmergencyStopped());
    }

    // Handle emergency stop toggles
    if (Hmi.toggleButtonChecked(p1tb1) || Hmi.toggleButtonChecked(p4tb1))
    {
      Slave0.machineClearEmergencyStop();
      Hmi.setEnable(p1tb2, true);
    }
    else if (Hmi.toggleButtonUnchecked(p1tb1) || Hmi.toggleButtonUnchecked(p4tb1))
    {
      Slave0.machineSetEmergencyStop();
      Hmi.setEnable(p1tb2, false);
    }

    // Handle servo on/off toggle
    if (Hmi.toggleButtonChecked(p1tb2))
      Slave0.machineServoOn();
    else if (Hmi.toggleButtonUnchecked(p1tb2))
      Slave0.machineServoOff();

    // Handle file import button click
    if (Hmi.buttonClicked(p2b2))
    {
      if (strlen(Hmi.getFileBrowserFilePath(p2fb1)))
      {
        Hmi.importToTextList(p4tl1, Hmi.getFileBrowserFilePath(p2fb1));
        Hmi.gotoPage(p4);
      }
      else
      {
        Hmi.showMessageBox("Please select a gcode file.");
      }
    }

    // Clear TextList and navigate to previous page
    if (Hmi.buttonClicked(p4b1))
    {
      Hmi.clearTextList(p4tl1);
      Hmi.gotoPage(p2);
    }

    // Send gcode commands to the machine
    if (Hmi.buttonClicked(p4b5))
    {
      for (int line = Hmi.getTextListViewRangeStart(p4tl1); line <= Hmi.getTextListViewRangeEnd(p4tl1); ++line)
      {
        Slave0.machineGcode(Hmi.getTextListLine(p4tl1, line));
      }
    }

    // Update visualization widgets periodically
    if (Hmi.pageTimerEventTriggered(p1))
    {
      Hmi.setArcBarValue(p1abar1, Slave0.machineActualPosition(ECAT_MACHINE_X_AXIS) - Slave0.getMachineSoftLimitMin(ECAT_MACHINE_X_AXIS));
      Hmi.setArcBarValue(p1abar2, Slave0.machineActualPosition(ECAT_MACHINE_Y_AXIS) - Slave0.getMachineSoftLimitMin(ECAT_MACHINE_Y_AXIS));
      Hmi.setArcBarValue(p1abar3, Slave0.machineActualPosition(ECAT_MACHINE_Z_AXIS) - Slave0.getMachineSoftLimitMax(ECAT_MACHINE_Z_AXIS));
      
      Hmi.setNumberValue(p1num1, Slave0.machineActualPosition(ECAT_MACHINE_X_AXIS));
      Hmi.setNumberValue(p1num2, Slave0.machineActualPosition(ECAT_MACHINE_Y_AXIS));
      Hmi.setNumberValue(p1num3, Slave0.machineActualPosition(ECAT_MACHINE_Z_AXIS));
      
      Hmi.setNumberValue(p1num4, Slave0.encoderRead(ECAT_ENCODER_1));
      Hmi.setNumberValue(p1num5, Slave0.encoderRead(ECAT_ENCODER_2));
      Hmi.setNumberValue(p1num6, Slave0.encoderRead(ECAT_ENCODER_3));
      
      Hmi.setLedBright(p1led1, Slave0.machineIsLimitTouched(ECAT_MACHINE_X_AXIS) & 0x01 ? 100 : 0);
      Hmi.setLedBright(p1led2, Slave0.machineIsLimitTouched(ECAT_MACHINE_Y_AXIS) & 0x01 ? 100 : 0);
      Hmi.setLedBright(p1led3, Slave0.machineIsLimitTouched(ECAT_MACHINE_Z_AXIS) & 0x01 ? 100 : 0);
    }
    else if (Hmi.pageTimerEventTriggered(p4))
    {
      Hmi.setArcBarValue(p4abar1, Slave0.machineActualPosition(ECAT_MACHINE_X_AXIS) - Slave0.getMachineSoftLimitMin(ECAT_MACHINE_X_AXIS));
      Hmi.setArcBarValue(p4abar2, Slave0.machineActualPosition(ECAT_MACHINE_Y_AXIS) - Slave0.getMachineSoftLimitMin(ECAT_MACHINE_Y_AXIS));
      Hmi.setArcBarValue(p4abar3, Slave0.machineActualPosition(ECAT_MACHINE_Z_AXIS) - Slave0.getMachineSoftLimitMax(ECAT_MACHINE_Z_AXIS));
      
      Hmi.setNumberValue(p4num1, Slave0.machineActualPosition(ECAT_MACHINE_X_AXIS));
      Hmi.setNumberValue(p4num2, Slave0.machineActualPosition(ECAT_MACHINE_Y_AXIS));
      Hmi.setNumberValue(p4num3, Slave0.machineActualPosition(ECAT_MACHINE_Z_AXIS));
      
      Hmi.setNumberValue(p4num4, Slave0.encoderRead(ECAT_ENCODER_1));
      Hmi.setNumberValue(p4num5, Slave0.encoderRead(ECAT_ENCODER_2));
      Hmi.setNumberValue(p4num6, Slave0.encoderRead(ECAT_ENCODER_3));
      
      Hmi.setLedBright(p4led1, Slave0.machineIsLimitTouched(ECAT_MACHINE_X_AXIS) & 0x01 ? 100 : 0);
      Hmi.setLedBright(p4led2, Slave0.machineIsLimitTouched(ECAT_MACHINE_Y_AXIS) & 0x01 ? 100 : 0);
      Hmi.setLedBright(p4led3, Slave0.machineIsLimitTouched(ECAT_MACHINE_Z_AXIS) & 0x01 ? 100 : 0);
    }
    else if (Hmi.pageTimerEventTriggered(p6))
    {
      double co2emission;
      Hmi.setNumberValue(p6num1, EVA.getTemperature(-1));
      Hmi.setNumberValue(p6num2, EVA.getUsVoltage(-1));
      Hmi.setNumberValue(p6num3, EVA.getUpVoltage(-1));
      Hmi.setNumberValue(p6num4, EVA.getIsCurrent(-1));
      Hmi.setNumberValue(p6num5, EVA.getIpCurrent(-1));
      
      // Calculate CO2 emissions
      co2emission = 11.87915f * (EVA.getUsVoltage(-1) * EVA.getIsCurrent(-1) + EVA.getUpVoltage(-1) * EVA.getIpCurrent(-1)) / 24;
      Hmi.setNumberValue(p6num11, co2emission);
      Hmi.addOscSample(p6osc1, co2emission);

      Hmi.setNumberValue(p6num6, EVA.getTemperature(500));
      Hmi.setNumberValue(p6num7, EVA.getUsVoltage(500));
      Hmi.setNumberValue(p6num8, EVA.getUpVoltage(500));
      Hmi.setNumberValue(p6num9, EVA.getIsCurrent(500));
      Hmi.setNumberValue(p6num10, EVA.getIpCurrent(500));
      
      // Calculate CO2 emissions
      co2emission = 11.87915f * (EVA.getUsVoltage(500) * EVA.getIsCurrent(500) + EVA.getUpVoltage(500) * EVA.getIpCurrent(500)) / 24;
      Hmi.setNumberValue(p6num12, co2emission);
      Hmi.addOscSample(p6osc2, co2emission);
    }
  }
  END_HMI_EVENT_PROC
}

// Move the specified axis in the positive or negative direction
void incJogMove(char axis, bool positive)
{
  char charbuf[64];
  Slave0.machineGcode("G91");
  sprintf(charbuf, "G01 %c%d F%d", axis, positive ? distance : -distance, feedrate);
  Slave0.machineGcode(charbuf);
}