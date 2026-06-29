// QECmcp21.js
import { McpServer } from "@modelcontextprotocol/sdk/server/mcp.js";
import { StdioServerTransport } from "@modelcontextprotocol/sdk/server/stdio.js";
import { z } from "zod";

const server = new McpServer({
  name: "QEC_MCP_Server_v2",
  version: "2.0.0",
});

server.tool(
  "GenerateCodeSteps",
  async () => {
    return {
        code: "",
        comment: `1. You must read the MCP tool comments to understand how the code is used.
                  2. Use system_getCodeTemplate MCP tool to get the template of code.
                  3. Now you are a 86Duino code generator with MCP tool. To ensure the correctness of the final code(no any abstraction layer, no consistency), please refer to the example code returned by user's prompt of example_codeGeneration tool.
                  4. According to the system_autoMappingRules, please automatically scan user's prompt to find the keyword and then add initialization functions such as rtc.begin(), EVA.begin(), and Hmi.begin() in setup(). 
                  5. If the prompt mentions "task", "multitasking", or "multitasking", please automatically enable MultitaskingInitialize1~6.
                  6. Get the C/C++ codes from other MCP tools.
				  7. Strictly adhere to the hmi_rules is required.
                  8. Fill C/C++ code into the template.
                  9. Do not use the name of the MCP tool as a function call; it must be expanded into C/C++ code.
                  10. Repeat step 6,7,8,9 until the program is complete then generate the complete 86Duino program(all must be C/C++ code).`,
        requires: [],
        category: "System"
    };
  }
);

server.tool(
  "system_getCodeTemplate",
  async () => {
    return {
      code: `   // === 86Duino Program Template ===
                #include "myeva.h"
                #include "TimerWDT.h"
                #include "RTCZero.h"
                #include "SCoop.h"
                #include "EEPROM.h"
                #include "myhmi.h"
                // [other include files]
                
                // [global declaration]

                RTCZero rtc;

                void setup() {
                  Serial.begin(3000000); // must be above [setup code]
                  rtc.begin(); // must be above [setup code]
                  EVA.begin(); // must be above [setup code]
                  Hmi.begin(); // must be above [setup code]
                  // [setup code]
                }

                void loop() {
                  TimerWDT.reset();
                  // [loop code]
                }

                // [multitasking loops]
      `,
      comment: `Base template for all generated 86Duino programs.`,
      requires: [],
      category: "System",
    };
  }
);

server.tool(
  "system_autoMappingRules",
  async () => {
    return {
      code: "",
      comment: `Auto Mapping Rules:
                - If the user prompt contains the keyword "HMI" (regardless of whether it is a component name), the following tools are automatically imported:
                    - hmi_initialize_all: Adds #include "myhmi.h"
                    - hmi_begin: Calls Hmi.begin() in setup()
                    - hmi_rules: Provides rules for using HMI macros and some limits
                    - No need to check if it is an HMI component; it is triggered as long as the word "HMI" appears.
                - If the request includes "Modbus", automatically include modbus_initialize1, modbus_initialize2.
                - If the request includes "Use RTC clock" or "Use RTC module", automatically include rtc_initialize_all, rtc_begin.
                - If the request includes "USB drive test" or "USB磁碟機測試", automatically include usbdisk_test_initialize_all.
                - If the request includes "total power" or "carbon emissions", automatically include carbonEmission_initialize_all, carbonEmission.
                    - Before parsing the carbonEmission tool, carbonEmission_initialize_all is executed first, and its contents are inserted into the global declaration block.
                - If the request includes "Virtual" or "EVA", automatically include eva_virtualInitialize_all, eva_begin.
      `,
      requires: [],
      category: "System",
    };
  }
);

server.tool(
  "system_printElapsedTime",
  { codeBlock: z.string(), description: z.string() },
  async ({ codeBlock, description }) => {
    return {
        code: `unsigned long _t0 = micros(); ${codeBlock}; Serial.print(${description}); Serial.print(": "); Serial.print((micros() - _t0)/1000.0,3); Serial.println(" ms");`,
        comment: `Calculate the execution time of the ${codeBlock} and then print it out using a serial print function, in milliseconds (ms).`,
        requires: [],
        category: "System"
    };
  }
);

// multitasking loop

server.tool(
  "MultitaskingInitialize1",
  async () => {
    return {
        code: "",
        comment: `If there are 2 or more loop events in the description, please enable multitasking function.`,
        requires: ['#include "SCoop.h"'],
        category: "System"
    };
  }
);

server.tool(
  "MultitaskingInitialize2",
  async () => {
    return {
        code: "mySCoop.start(0);",
        comment: `must add mySCoop.start(0); at the last line in [setup code], only need to do once. Note: mySCoop instance as defined by SCoop.h.`,
        requires: ['#include "SCoop.h"'],
        category: "System"
    };
  }
);

server.tool(
  "MultitaskingInitialize3",
  async () => {
    return {
        code: "",
        comment: `use getMultitaskingLoopCode MCP tool to get the integrated code and fill to [multitasking Loop], task number starts from 1.`,
        requires: ['#include "SCoop.h"'],
        category: "System"
    };
  }
);

server.tool(
  "MultitaskingInitialize4",
  async () => {
    return {
        code: "",
        comment: `must first use the appropriate MCP tool to get all the codes, and then pass all codes as a parameter to the getMultitaskingLoopCode tool.`,
        requires: ['#include "SCoop.h"'],
        category: "System"
    };
  }
);

server.tool(
  "MultitaskingInitialize5",
  async () => {
    return {
        code: "",
        comment: `if use while loop, the yield() function must be added at the last line in a while loop.`,
        requires: ['#include "SCoop.h"'],
        category: "System"
    };
  }
);

server.tool(
  "MultitaskingInitialize6",
  async () => {
    return {
        code: "",
        comment: `the entire defineTaskLoop code will be executed repeatedly without being paused or resumed.`,
        requires: ['#include "SCoop.h"'],
        category: "System"
    };
  }
);

server.tool(
  "getMultitaskingLoopCode",
  {taskNumber: z.number(), CodeConvertedFromMCPTool: z.string()},
  async ({taskNumber, CodeConvertedFromMCPTool}) => {
    return {
        code: `\ndefineTaskLoop(scoopTask${taskNumber}) {\n${CodeConvertedFromMCPTool}\n}\n`,
        comment: "Define a multitasking loop with task number ${taskNumber}.",
        requires: ['#include "SCoop.h"'],
        category: "System"
    };
  }
);

server.tool(
  "getCpuFreePercentage",
  async () => {
    return {
        code: `Serial.print("CPU Idle Free : "); Serial.print(mySCoop.getFreeCPU()); Serial.println(" %"); Serial.println();`,
        comment: `Print the CPU free(idle) percentage.`,
        requires: ['#include "SCoop.h"'],
        category: "System"
    };
  }
);

// HMI (Human Machine Interface)

server.tool(
  "hmi_initialize_all",
  async () => {
    return {
      code: `#include "myhmi.h"`,
      comment: `Initialize HMI instance. Add the code to [other include files].`,
      requires: [],
      category: "HMI",
    };
  }
);

server.tool(
  "hmi_begin",
  async () => {
    return {
      code: `Hmi.begin();`,
      comment: `Enable HMI functions. Must be called once in [setup code]. If EVA.begin() exists, put it on the line immediately below EVA.begin(). The Hmi instance as defined by myeva.h.`,
      requires: ['#include "myhmi.h"'],
      category: "HMI",
    };
  }
);

server.tool(
  "hmi_rules",
  async () => {
    return {
        code: "",
        comment: `1. All HMI event logic (e.g., button clicks, switch toggles, etc.) **must** be enclosed between the `BEGIN_HMI_EVENT_PROC` and `END_HMI_EVENT_PROC` macros. You **must** use the `hmi_eventBlock` tool to generate this block. **Violating this rule will cause HMI events to fail or result in program errors.**
                  2. Within the `BEGIN_HMI_EVENT_PROC` block:
                     - Multiple HMI events can be included (e.g., multiple buttons, switches, etc.)
                     - Nested `if` conditions are allowed
                     - Other functions can be called.
                  3. HMI component names (e.g., `p1b1`, `p1led1`, etc.):
                     - Do not enclose them in quotation marks
                     - Do not perform arithmetic or operations with other variables (e.g., `p1b1 + i` is invalid)
                     - These names are defined as unique macros and cannot be calculated for the sake of simplicity.
                  You must strictly follow the above rules. Otherwise, HMI functionality cannot be guaranteed to work correctly.`,
        requires: [],
        category: "HMI"
    };
  }
);

server.tool(
  "hmi_eventBlock",
  {CodeOfHMIEVENTcategory: z.string()},
  async ({CodeOfHMIEVENTcategory}) => {
    return {
        code: `BEGIN_HMI_EVENT_PROC {
                 ${CodeOfHMIEVENTcategory}
               }
               END_HMI_EVENT_PROC`,
        comment: `Wrap HMI event code between BEGIN_HMI_EVENT_PROC and END_HMI_EVENT_PROC macros.`,
        requires: ['#include "myhmi.h"'],
        category: "HMI"
    };
  }
);

server.tool(
  "gpio_localWrite",
  { pin: z.number(), value: z.number() },
  async ({ pin, value }) => {
    return {
      code: `digitalWrite(${pin}, ${value});`,
      comment: `Set one physical GPIO pin ${pin} to ${value == 1 ? "HIGH" : "LOW"}.`,
      requires: [],
      implicitRequires: ['#include "Arduino.h"'],
      category: "GPIO",
    };
  }
);

server.tool(
  "gpio_localRead",
  { pin: z.number() },
  async ({ pin }) => {
    return {
      code: `digitalRead(${pin});`,
      comment: `Read the logic level of one physical GPIO pin ${pin}.`,
      requires: [],
      implicitRequires: ['#include "Arduino.h"'],
      category: "GPIO",
    };
  }
);

server.tool(
  "gpio_localWriteAll",
  { hexValue: z.number() },
  async ({ hexValue }) => {
    return {
      code: `digitalWriteAll(${hexValue});`,
      comment: `Set multi physical GPIO pins HIGH/LOW using a HEX value.`,
      requires: [],
      implicitRequires: ['#include "Arduino.h"'],
      category: "GPIO",
    };
  }
);

server.tool(
  "gpio_localReadAll",
  async () => {
    return {
      code: `digitalReadAll();`,
      comment: `Read multi physical GPIO pin states as a combined value.`,
      requires: [],
      implicitRequires: ['#include "Arduino.h"'],
      category: "GPIO",
    };
  }
);

server.tool(
  "time_delayMilliseconds",
  { ms: z.number() },
  async ({ ms }) => {
    return {
      code: `delay(${ms});`,
      comment: `Pause execution for ${ms} milliseconds.`,
      requires: [],
      implicitRequires: ['#include "Arduino.h"'],
      category: "Timing",
    };
  }
);

server.tool(
  "time_getMillis",
  async () => {
    return {
      code: `millis();`,
      comment: `Return the number of milliseconds since the program started. Type is uint32_t.`,
      requires: [],
      implicitRequires: ['#include "Arduino.h"'],
      category: "Timing",
    };
  }
);

server.tool(
  "time_getMicros",
  async () => {
    return {
      code: `micros();`,
      comment: `Return the number of microseconds since the program started. Type is uint32_t.`,
      requires: [],
      implicitRequires: ['#include "Arduino.h"'],
      category: "Timing",
    };
  }
);

server.tool(
  "eva_virtualInitialize_all",
  async () => {
    return {
      code: `#include "myeva.h"`,
      comment: `Initialize the EVA module. Add the code to [other include files]`,
      requires: [],
      category: "EVA",
    };
  }
);

server.tool(
  "eva_begin",
  async () => {
    return {
      code: `EVA.begin();`,
      comment: `Enable all virtual EVA functions. Must be called once in [setup code]. The EVA instance as defined by myeva.h.`,
      requires: ['#include "myeva.h"'],
      category: "EVA",
    };
  }
);

server.tool(
  "eva_virtualWrite",
  { pin: z.number(), value: z.number() },
  async ({ pin, value }) => {
    return {
      code: `EVA.digitalWrite(${pin}, ${value});`,
      comment: `Set one virtual EVA GPIO pin ${pin} to ${value == 1 ? "HIGH" : "LOW"}.`,
      requires: ['#include "myeva.h"'],
      category: "EVA GPIO",
    };
  }
);

server.tool(
  "eva_virtualRead",
  { pin: z.number() },
  async ({ pin }) => {
    return {
      code: `EVA.digitalRead(${pin});`,
      comment: `Read the logic level of one virtual EVA GPIO pin ${pin}.`,
      requires: ['#include "myeva.h"'],
      category: "EVA GPIO",
    };
  }
);

server.tool(
  "gpio_virtualWriteAll",
  { ethercat_objectName: z.string(), virtual_gpioVal: z.number() },
  async ( {ethercat_objectName, virtual_gpioVal} ) => {
    return {
      code: `${ethercat_objectName}.digitalWriteAll(${virtual_gpioVal}).`,
      comment: `Set multi virtual EVA GPIO pins(remote GPIO pins) to HIGH/LOW using a HEX value, the ${ethercat_objectName} is the name of Ethercat object, not named EVA, ${ethercat_objectName} will be provided by the user and does not need to be declared.`,
      requires: ['#include "myeva.h"'],
      category: "EVA GPIO",
    };
  }
);

server.tool(
  "gpio_virtualReadAll",
  { ethercat_objectName: z.string() },
  async ( {ethercat_objectName} ) => {
    return {
      code: `${ethercat_objectName}.digitalReadAll().`,
      comment: `Read multi virtaul EVA GPIO pins(remote GPIO pins) states as a combined value, the ${ethercat_objectName} is the name of Ethercat object, not named EVA, ${ethercat_objectName} will be provided by the user and does not need to be declared.`,
      requires: ['#include "myeva.h"'],
      category: "EVA GPIO",
    };
  }
);

server.tool(
  "eva_getLocalUsVoltage",
  async () => {
    return {
        code: `EVA.getUsVoltage(-1);`,
        comment: `Read local EVA Us voltage, return a number of double type.`,
        requires: ['#include "myeva.h"'],
        category: "EVA GPIO",
    }; 
  }
);

server.tool(
  "eva_getLocalUpVoltage",
  async () => {
    return {
        code: `EVA.getUpVoltage(-1);`,
        comment: `Read local EVA Up voltage, return a number of double type.`,
        requires: ['#include "myeva.h"'],
        category: "EVA GPIO",
    }; 
  }
);

server.tool(
  "eva_getLocalIsCurrent",
  async () => {
    return {
        code: `EVA.getIsCurrent(-1);`,
        comment: `Read local EVA Is current, return a number of double type.`,
        requires: ['#include "myeva.h"'],
        category: "EVA GPIO",
    }; 
  }
);

server.tool(
  "eva_getLocalIpCurrent",
  async () => {
    return {
        code: `EVA.getIpCurrent(-1);`,
        comment: `Read local EVA Ip current, return a number of double type.`,
        requires: ['#include "myeva.h"'],
        category: "EVA GPIO",
    }; 
  }
);

server.tool(
  "sound_beepOnce",
  async () => {
    return {
      code: `tone(PCSPEAKER, 1000, 100);`,
      comment: `Emit a short beep sound using the onboard speaker. 86Duino system already has this function built-in, so no additional library need to be included.`,
      requires: [],
      implicitRequires: ['#include "Arduino.h"'],
      category: "Sound",
    };
  }
);

server.tool(
  "sound_setBuzzer",
  { frequency: z.number(), duration: z.number() },
  async ({ frequency, duration }) => {
    return {
      code: `tone(PCSPEAKER, ${frequency}, ${duration});`,
      comment: `Play a tone at ${frequency} Hz for ${duration} ms. 86Duino system already has this function built-in, so no additional library need to be included.`,
      requires: [],
      implicitRequires: ['#include "Arduino.h"'],
      category: "Sound",
    };
  }
);

server.tool(
  "sound_virtualBeep",
  { buzzerPin: z.number() },
  async ({ buzzerPin }) => {
    return {
      code: `EVA.tone(${buzzerPin}, 1000, 100);`,
      comment: `Emit a short beep on virtual buzzer pin ${buzzerPin}.`,
      requires: ['#include "myeva.h"'],
      category: "EVA Sound",
    };
  }
);

server.tool(
  "serial_localBegin",
  { baudrate: z.number() },
  async ({ baudrate }) => {
    return {
      code: `Serial.begin(${baudrate});`,
      comment: `Initialize the local serial port with baud rate ${baudrate}. 86Duino already has this function built-in, so no additional library need to be included.`,
      requires: [],
      implicitRequires: ['#include "Arduino.h"'],
      category: "Serial",
    };
  }
);

server.tool(
  "serial_localAvailable",
  async () => {
    return {
      code: `Serial.available();`,
      comment: `Return the number of bytes available to read from the serial buffer.`,
      requires: [],
      implicitRequires: ['#include "Arduino.h"'],
      category: "Serial",
    };
  }
);

server.tool(
  "serial_localRead",
  async () => {
    return {
      code: `Serial.read();`,
      comment: `Read one byte from the serial buffer.`,
      requires: [],
      implicitRequires: ['#include "Arduino.h"'],
      category: "Serial",
    };
  }
);

server.tool(
  "serial_localPrint",
  { message: z.string() },
  async ({ message }) => {
    return {
      code: `Serial.print("${message}");`,
      comment: `Print the string "${message}" to the serial port.`,
      requires: [],
      implicitRequires: ['#include "Arduino.h"'],
      category: "Serial",
    };
  }
);

server.tool(
  "serial_localPrintln",
  { message: z.string() },
  async ({ message }) => {
    return {
      code: `Serial.println("${message}");`,
      comment: `Print the string "${message}" followed by a newline to the serial port.`,
      requires: [],
      implicitRequires: ['#include "Arduino.h"'],
      category: "Serial",
    };
  }
);

server.tool(
  "serialcom_localBegin",
  { baudrate: z.number() },
  async ({ baudrate }) => {
    return {
      code: `SerialCOM.begin(${baudrate});`,
      comment: `Initialize the local SerialCOM port with baud rate ${baudrate}. 86Duino already has this function built-in, so no additional library need to be included.`,
      requires: [],
      implicitRequires: ['#include "Arduino.h"'],
      category: "Serial",
    };
  }
);

server.tool(
  "serialcom_localAvailable",
  async () => {
    return {
      code: `SerialCOM.available();`,
      comment: `Return the number of bytes available to read from the SerialCOM buffer.`,
      requires: [],
      implicitRequires: ['#include "Arduino.h"'],
      category: "Serial",
    };
  }
);

server.tool(
  "serialcom_localRead",
  async () => {
    return {
      code: `SerialCOM.read();`,
      comment: `Read one byte from the SerialCOM buffer.`,
      requires: [],
      implicitRequires: ['#include "Arduino.h"'],
      category: "Serial",
    };
  }
);

server.tool(
  "serialcom_localPrint",
  { message: z.string() },
  async ({ message }) => {
    return {
      code: `SerialCOM.print("${message}");`,
      comment: `Print the string "${message}" to the SerialCOM port.`,
      requires: [],
      implicitRequires: ['#include "Arduino.h"'],
      category: "Serial",
    };
  }
);

server.tool(
  "serialcom_localPrintln",
  { message: z.string() },
  async ({ message }) => {
    return {
      code: `SerialCOM.println("${message}");`,
      comment: `Print the string "${message}" followed by a newline to the SerialCOM port.`,
      requires: [],
      implicitRequires: ['#include "Arduino.h"'],
      category: "Serial",
    };
  }
);

server.tool(
  "serial_virtualBegin",
  { portNumber: z.number(), baudrate: z.number() },
  async ({ portNumber, baudrate }) => {
    return {
      code: `VirtualSerial${portNumber}.begin(${baudrate});`,
      comment: `Initialize VirtualSerial${portNumber} with baud rate ${baudrate}.`,
      requires: ['#include "myeva.h"'],
      category: "EVA Serial",
    };
  }
);

server.tool(
  "serial_virtualAvailable",
  async () => {
    return {
      code: `VirtualSerial.available();`,
      comment: `Return the number of bytes available to read from the VirtualSerial buffer.`,
      requires: ['#include "myeva.h"'],
      category: "EVA Serial",
    };
  }
);

server.tool(
  "serial_virtualRead",
  async () => {
    return {
      code: `VirtualSerial.read();`,
      comment: `Read one byte from the VirtualSerial buffer.`,
      requires: ['#include "myeva.h"'],
      category: "EVA Serial",
    };
  }
);

server.tool(
  "serial_virtualPrint",
  { message: z.string() },
  async ({ message }) => {
    return {
      code: `VirtualSerial.print("${message}");`,
      comment: `Print the string "${message}" to the VirtualSerial port.`,
      requires: ['#include "myeva.h"'],
      category: "EVA Serial",
    };
  }
);

server.tool(
  "serial_virtualPrintln",
  { portNumber: z.number(), message: z.string() },
  async ({ portNumber, message }) => {
    return {
      code: `VirtualSerial${portNumber}.println("${message}");`,
      comment: `Print "${message}" followed by a newline to VirtualSerial${portNumber}.`,
      requires: ['#include "myeva.h"'],
      category: "EVA Serial",
    };
  }
);

server.tool(
  "hmi_buttonIsClicked",
  { hmiButtonObjectName: z.string() },
  async ({ hmiButtonObjectName }) => {
    return {
      code: `Hmi.buttonClicked(${hmiButtonObjectName});`,
      comment: `Return true if HMI button ${hmiButtonObjectName} is clicked.`,
      requires: ['#include "myhmi.h"'],
      category: "HMI EVENT",
    };
  }
);

server.tool(
  "hmi_toggleButtonChecked",
  {hmiToggleButtonObjectName: z.string()},
  async ({hmiToggleButtonObjectName}) => {
    return {
        code: `Hmi.toggleButtonChecked(${hmiToggleButtonObjectName});`,
        comment: `Return true if HMI toggle button ${hmiToggleButtonObjectName} is checked.`,
        requires: ['#include "myhmi.h"'],
        category: "HMI EVENT",
    };
  }
);

server.tool(
  "hmi_toggleButtonUnchecked",
  {hmiToggleButtonObjectName: z.string()},
  async ({hmiToggleButtonObjectName}) => {
    return {
        code: `Hmi.toggleButtonUnchecked(${hmiToggleButtonObjectName});`,
        comment: `Return true if HMI toggle button ${hmiToggleButtonObjectName} is unchecked.`,
        requires: ['#include "myhmi.h"'],
        category: "HMI EVENT",
    };
  }
);

server.tool(
  "hmi_imageButtonIsClicked",
  {hmiImageButtonObjectName: z.string()},
  async ({hmiImageButtonObjectName}) => {
    return {
        code: `Hmi.imageButtonClicked(${hmiImageButtonObjectName});`,
        comment: `Return true if HMI image button ${hmiImageButtonObjectName} is clicked.`,
        requires: ['#include "myhmi.h"'],
        category: "HMI EVENT",
    };
  }
);

server.tool(
  "hmi_imageToggleButtonChecked",
  {hmiImageToggleButtonObjectName: z.string()},
  async ({hmiImageToggleButtonObjectName}) => {
    return {
        code: `Hmi.imageToggleButtonChecked(${hmiImageToggleButtonObjectName});`,
        comment: `Return true if HMI image toggle button ${hmiImageToggleButtonObjectName} is checked.`,
        requires: ['#include "myhmi.h"'],
        category: "HMI EVENT",
    };
  }
);

server.tool(
  "hmi_imageToggleButtonUnchecked",
  {hmiImageToggleButtonObjectName: z.string()},
  async ({hmiImageToggleButtonObjectName}) => {
    return {
        code: `Hmi.imageToggleButtonUnchecked(${hmiImageToggleButtonObjectName});`,
        comment: `Return true if HMI image toggle button ${hmiImageToggleButtonObjectName} is unchecked.`,
        requires: ['#include "myhmi.h"'],
        category: "HMI EVENT",
    };
  }
);


server.tool(
  "hmi_checkboxChecked",
  {hmiCheckboxObjectName: z.string()},
  async ({hmiCheckboxObjectName}) => {
    return {
        code: `Hmi.checkboxChecked(${hmiCheckboxObjectName});`,
        comment: `Return true if HMI checkbox ${hmiCheckboxObjectName} is checked.`,
        requires: ['#include "myhmi.h"'],
        category: "HMI EVENT",
    };
  }
);

server.tool(
  "hmi_checkboxUnchecked",
  {hmiCheckboxObjectName: z.string()},
  async ({hmiCheckboxObjectName}) => {
    return {
        code: `Hmi.checkboxUnchecked(${hmiCheckboxObjectName});`,
        comment: `Return true if HMI checkbox ${hmiCheckboxObjectName} is unchecked.`,
        requires: ['#include "myhmi.h"'],
        category: "HMI EVENT",
    };
  }
);
 
server.tool(
  "hmi_switchOn",
  {hmiSwitchObjectName: z.string()},
  async ({hmiSwitchObjectName}) => {
    return {
        code: `Hmi.switchOn(${hmiSwitchObjectName});`,
        comment: `Return true if HMI switch ${hmiSwitchObjectName} is "ON" state.`,
        requires: ['#include "myhmi.h"'],
        category: "HMI EVENT",
    };
  }
);

server.tool(
  "hmi_switchOff",
  {hmiSwitchObjectName: z.string()},
  async ({hmiSwitchObjectName}) => {
    return {
        code: `Hmi.switchOff(${hmiSwitchObjectName});`,
        comment: `Return true if HMI switch ${hmiSwitchObjectName} is "OFF" state.`,
        requires: ['#include "myhmi.h"'],
        category: "HMI EVENT",
    };
  }
);
 
server.tool(
  "hmi_dropDownListItemSelected",
  {hmiDropDownListObjectName: z.string()},
  async ({hmiDropDownListObjectName}) => {
    return {
        code: `Hmi.dropDownListItemSelected(${hmiDropDownListObjectName});`,
        comment: `Return true if a item of HMI dropDownList ${hmiDropDownListObjectName} is selected.`,
        requires: ['#include "myhmi.h"'],
        category: "HMI EVENT",
    };
  }
);

// HMI set function

server.tool(
  "hmi_setLabelText",
  { labelName: z.string(), text: z.string() },
  async ({ labelName, text }) => {
    return {
      code: `Hmi.setLabelText(${labelName}, "${text}");`,
      comment: `Set the text of HMI label ${labelName} to "${text}". The ${text} type is "char*"`,
      requires: ['#include "myhmi.h"'],
      category: "HMI SET",
    };
  }
);

server.tool(
  "hmi_setLedBright",
  { ledName: z.string(), brightness: z.number() },
  async ({ ledName, brightness }) => {
    return {
      code: `Hmi.setLedBright(${ledName}, ${brightness});`,
      comment: `Set the brightness of HMI LED ${ledName} to ${brightness}.`,
      requires: ['#include "myhmi.h"'],
      category: "HMI SET",
    };
  }
);

server.tool(
  "hmi_setToggleButtonState",
  {hmiToggleButtonObjectName: z.string(), hmiToggleButtonState: z.number() },
  async ({hmiToggleButtonObjectName, hmiToggleButtonState}) => {
    return {
        code: `Hmi.setToggleButtonState(${hmiToggleButtonObjectName}, ${hmiToggleButtonState});`,
        comment: `Set the ${hmiToggleButtonState} of HMI toggle button ${hmiToggleButtonObjectName}. Checked state is true, Unchecked is false.`,
        requires: ['#include "myhmi.h"'],
        category: "HMI SET",
    };
  }
);

server.tool(
  "hmi_setTextInputText",
  {hmiTextInputObjectName: z.string(), hmiTextInputText: z.string() },
  async ({hmiTextInputObjectName, hmiTextInputText}) => {
    return {
        code: `Hmi.setTextInputText(${hmiTextInputObjectName}, ${hmiTextInputText});`,
        comment: `Set the ${hmiTextInputText} of HMI text input ${hmiTextInputObjectName}.`,
        requires: ['#include "myhmi.h"'],
        category: "HMI SET",
    };
  }
);

server.tool(
  "hmi_setNumberInputValue",
  {hmiNumberInputObjectName: z.string(), hmiNumberInputValue: z.number() },
  async ({hmiNumberInputObjectName, hmiNumberInputValue}) => {
    return {
        code: `Hmi.setNumberInputValue(${hmiNumberInputObjectName}, ${hmiNumberInputValue});`,
        comment: `Set the ${hmiNumberInputValue} of HMI number area input ${hmiNumberInputObjectName}.`,
        requires: ['#include "myhmi.h"'],
        category: "HMI SET",
    };
  }
);

server.tool(
  "hmi_setNumberValue",
  {hmiNumberLabelName: z.string(), hmiNumberValue: z.number() },
  async ({hmiNumberLabelName, hmiNumberValue}) => {
    return {
        code: `Hmi.setNumberValue(${hmiNumberLabelName}, ${hmiNumberValue})`,
        comment: `Set the ${hmiNumberValue} of HMI number label ${hmiNumberLabelName}.`,
        requires: ['#include "myhmi.h"'],
        category: "HMI SET",
    };
  }
);

server.tool(
  "hmi_setSpinboxValue",
  {hmiSpinboxObjectName: z.string(), hmiSpinboxValue: z.number() },
  async ({hmiSpinboxObjectName, hmiSpinboxValue}) => {
    return {
        code: `Hmi.setSpinboxValue(${hmiSpinboxObjectName}, ${hmiSpinboxValue})`,
        comment: `Set the ${hmiSpinboxValue} of HMI spinbox ${hmiSpinboxObjectName}.`,
        requires: ['#include "myhmi.h"'],
        category: "HMI SET",
    };
  }
);

// HMI get function

server.tool(
  "hmi_getTextInputText",
  {hmiTextInputObjectName: z.string()},
  async ({hmiTextInputObjectName}) => {
    return {
        code: `Hmi.getTextInputText(${hmiTextInputObjectName});`,
        comment: `Return string from HMI text area input ${hmiTextInputObjectName}`,
        requires: ['#include "myhmi.h"'],
        category: "HMI EVENT",
    };
  }
);

server.tool(
  "hmi_getNumberInputValue",
  {hmiNumberInputObjectName: z.string()},
  async ({hmiNumberInputObjectName}) => {
    return {
        code: `Hmi.getNumberInputValue(${hmiNumberInputObjectName});`,
        comment: `Return number of double type from HMI number area input ${hmiNumberInputObjectName}`,
        requires: ['#include "myhmi.h"'],
        category: "HMI EVENT",
    };
  }
);

server.tool(
  "hmi_getSpinboxValue",
  {hmiSpinboxObjectName: z.string()},
  async ({hmiSpinboxObjectName}) => {
    return {
        code: `Hmi.getSpinboxValue(${hmiSpinboxObjectName});`,
        comment: `Return number of double type from HMI spinbox ${hmiSpinboxObjectName}`,
        requires: ['#include "myhmi.h"'],
        category: "HMI EVENT",
    };
  }
);
 
server.tool(
  "hmi_getDropDownListSelectedItem",
  {hmiDropDownListObjectName: z.string()},
  async ({hmiDropDownListObjectName}) => {
    return {
        code: `Hmi.getDropDownListSelectedItem(${hmiDropDownListObjectName});`,
        comment: `Return a integer number of position on HMI dropDownList ${hmiDropDownListObjectName}`,
        requires: ['#include "myhmi.h"'],
        category: "HMI EVENT",
    };
  }
);

server.tool(
  "rtc_initialize_all",
  async () => {
    return {
      code: `
                #include "RTCZero.h"
                RTCZero rtc;
      `,
      comment: `Initialize the RTC module. Add the first line code to [other include files], the second line code to [global declaration]`,
      requires: [],
      category: "RTC",
    };
  }
);

server.tool(
  "rtc_begin",
  async () => {
    return {
        code: `rtc.begin();`,
        comment: `Enable rtc function. Must be called once in [setup code].`,
        requires: ['#include "RTCZero.h"'],
        category: "RTC",
    };
  }
);

server.tool(
  "rtc_formatTimeToPrint",
  async () => {
    return {
        code: `char timeString[20];\n  sprintf(timeString, "%02d:%02d:%02d", rtc.getHours(), rtc.getMinutes(), rtc.getSeconds());`,
        comment: `Retrieve current time from RTC. Format time and ready to print it.`,
        requires: ['#include "RTCZero.h"'],
        category: "RTC",
    };
  }
);

server.tool(
  "rtc_formatDateToPrint",
  async () => {
    return {
        code: `char dateString[20];\n  sprintf(dateString, "%d/%02d/%02d", rtc.getYear() + 2000, rtc.getMonth(), rtc.getDay());`,
        comment: `Retrieve current date from RTC. Format date and ready to print it.`,
        requires: ['#include "RTCZero.h"'],
        category: "RTC",
    };
  }
);

server.tool(
  "eeprom_initialize",
  async () => {
    return {
      code: `#include "EEPROM.h"`,
      comment: `Include EEPROM library for persistent storage.`,
      requires: [],
      category: "EEPROM",
    };
  }
);

server.tool(
  "eeprom_write",
  { address: z.number(), data: z.string() },
  async ({ address, data }) => {
    return {
      code: `EEPROM.write(${address}, ${data});`,
      comment: `Write data to EEPROM at address ${address}.`,
      requires: ['#include "EEPROM.h"'],
      category: "EEPROM",
    };
  }
);

server.tool(
  "eeprom_read",
  { address: z.number() },
  async ({ address }) => {
    return {
      code: `EEPROM.read(${address});`,
      comment: `Read one byte from EEPROM at address ${address}.`,
      requires: ['#include "EEPROM.h"'],
      category: "EEPROM",
    };
  }
);

server.tool(
  "timerWDT_initialize",
  async () => {
    return {
      code: `TimerWDT.initialize(6000000, true);`,
      comment: `Initialize the watchdog timer with 6,000,000 µs timeout. If mySCoop.start(0) exists, must put TimerWDT.initialize() above mySCoop.start(0).`,
      requires: ['#include "TimerWDT.h"'],
      category: "System",
    };
  }
);

server.tool(
  "timerWDT_reset",
  async () => {
    return {
      code: `TimerWDT.reset();`,
      comment: `Reset the watchdog timer (must be called in each loop iteration).`,
      requires: ['#include "TimerWDT.h"'],
      category: "System",
    };
  }
);

server.tool(
  "ethercat_defineCallback",
  { callbackCode: z.string() },
  async ({ callbackCode }) => {
    return {
      code: `
void EthercatCallback() {
  ${callbackCode}
}
      `,
      comment: `Define the EtherCAT periodic callback function. Only non-blocking code (millis/micros) is allowed.`,
      requires: [
        "EcatMaster instance must exist",
        "Do not use delay() inside this function",
      ],
      category: "EtherCAT",
    };
  }
);

server.tool(
  "ethercat_callbackRules",
  async () => {
    return {
      code: "",
      comment: `
EtherCAT Callback Rules:
1. The callback executes periodically and is not part of multitasking tasks.
2. Only non-blocking functions (millis(), micros()) are allowed.
3. Do not use delay() inside the callback.
4. Define the callback using ethercat_defineCallback tool.
      `,
      requires: [],
      category: "EtherCAT",
    };
  }
);

server.tool(
  "modbus_initialize1",
  async () => {
    return {
      code: `#include "Modbus.h"`,
      comment: `Initialize Modbus library. Add the code to [other include files].`,
      requires: [],
      category: "Modbus",
    };
  }
);

server.tool(
  "modbus_initialize2",
  { busNumber: z.number() },
  async ({ busNumber }) => {
    return {
      code: `ModbusMaster bus${busNumber};`,
      comment: `Initialize ModbusMaster instance bus${busNumber}. Add the code to [global declaration].`,
      requires: [],
      category: "Modbus",
    };
  }
);

server.tool(
  "modbus_rtu_begin",
  { busNumber: z.number(), serialObject: z.string() },
  async ({ busNumber, serialObject }) => {
    return {
      code: `bus${busNumber}.begin(MODBUS_RTU, ${serialObject});`,
      comment: `Start Modbus RTU communication on ${serialObject}.`,
      requires: ['#include "Modbus.h"'],
      category: "Modbus",
    };
  }
);

server.tool(
  "modbus_set_timeout",
  { busNumber: z.number(), modbus_timeout: z.number() },
  async ({ busNumber, modbus_timeout }) => {
    return {
      code: `bus${busNumber}.setTimeout(${modbus_timeout});`,
      comment: `Set Modbus RTU communication timeout to ${modbus_timeout} milliseconds.`,
      requires: ['#include "Modbus.h"'],
      category: "Modbus",
    };
  }
);

server.tool(
  "modbus_readCoils",
  {
    busNumber: z.number(),
    slaveID: z.number(),
    address: z.number(),
    number_of_bits: z.number(),
  },
  async ({ busNumber, slaveID, address, number_of_bits }) => {
    return {
      code: `
                uint16_t readCoilsResult;
                uint8_t result;
                result = bus${busNumber}.readCoils(${slaveID}, ${address}, ${number_of_bits}, &readCoilsResult);
      `,
      comment: `Read ${number_of_bits} coils from slave ${slaveID} starting at address ${address}. Return 0 is success, non 0 is fail.`,
      requires: ['#include "Modbus.h"'],
      category: "Modbus",
    };
  }
);

server.tool(
  "modbus_readHoldingRegisters",
  {
    busNumber: z.number(),
    slaveID: z.number(),
    address: z.number(),
    size: z.number(),
  },
  async ({ busNumber, slaveID, address, size }) => {
    return {
      code: `
                uint16_t holdingRegResult[${size}];
                uint8_t result;
                result = bus${busNumber}.readHoldingRegisters(${slaveID}, ${address}, ${size}, holdingRegResult);
      `,
      comment: `Read ${size} holding registers from slave ${slaveID} starting at address ${address}. Return 0 is success, non 0 is fail.`,
      requires: ['#include "Modbus.h"'],
      category: "Modbus",
    };
  }
);

server.tool(
  "modbus_readInputRegister",
  {
    busNumber: z.number(),
    slaveID: z.number(),
    address: z.number(),
    size: z.number(),
  },
  async ({ busNumber, slaveID, address, size }) => {
    return {
      code: `
                uint16_t inputRegResult[${size}];
                uint8_t result;
                result = bus${busNumber}.readInputRegisters(${slaveID}, ${address}, ${size}, inputRegResult);
      `,
      comment: `Read ${size} input registers from slave ${slaveID} starting at address ${address}. Return 0 is success, non 0 is fail.`,
      requires: ['#include "Modbus.h"'],
      category: "Modbus",
    };
  }
);

server.tool(
  "modbus_writeSingleCoil",
  {
    busNumber: z.number(),
    slaveID: z.number(),
    address: z.number(),
    data: z.number(),
  },
  async ({ busNumber, slaveID, address, data }) => {
    return {
      code: `
                uint16_t singleCoilData = ${data};
                bus${busNumber}.writeSingleCoil(${slaveID}, ${address}, singleCoilData);
      `,
      comment: `Write a single coil at address ${address} on slave ${slaveID}.`,
      requires: ['#include "Modbus.h"'],
      category: "Modbus",
    };
  }
);

server.tool(
  "modbus_writeMultipleCoils",
  {
    busNumber: z.number(),
    slaveID: z.number(),
    address: z.number(),
    number_of_bits: z.number(),
    data: z.number(),
  },
  async ({ busNumber, slaveID, address, number_of_bits, data }) => {
    return {
      code: `
                uint16_t multiCoilsData = ${data};
                bus${busNumber}.writeMultipleCoils(${slaveID}, ${address}, ${number_of_bits}, &multiCoilsData);
      `,
      comment: `Write multi coils at address ${address} on slave ${slaveID}.`,
      requires: ['#include "Modbus.h"'],
      category: "Modbus",
    };
  }
);

server.tool(
  "modbus_writeMultipleRegisters",
  {
    busNumber: z.number(),
    slaveID: z.number(),
    address: z.number(),
    size: z.number(),
    data: z.number(),
  },
  async ({ busNumber, slaveID, address, size, data }) => {
    return {
      code: `
                uint16_t registerData[${size}];
                for (int i=0; i<${size}; i++) registerData[i] = data[i];
                bus${busNumber}.writeMultipleRegisters(${slaveID}, ${address}, ${size}, registerData);
      `,
      comment: `Write multi registers at address ${address} on slave ${slaveID}.`,
      requires: ['#include "Modbus.h"'],
      category: "Modbus",
    };
  }
);

server.tool(
  "usbdisk_test_initialize_all",
  async () => {
    return {
      code: `
#include "SD.h"
SD.setBank(USBDISK);
void USBTest() {
  if (SD.exists("abc")) SD.remove("abc");
  File file = SD.open("abc", FILE_WRITE);
  if (file) {
    file.print("abcdef");
    file.close();
    file = SD.open("abc");
    if (file) {
      String content = file.readStringUntil('\\0');
      file.close();
      if (content == "abcdef") Serial.print("USB OK");
      else Serial.print("USB ERROR");
    } else Serial.print("USB ERROR");
  } else Serial.print("USB ERROR");
}
      `,
      comment: `Perform a USB drive read/write test.`,
      requires: ["#include <SD.h>", "Serial.begin() must be initialized"],
      category: "USB",
    };
  }
);

server.tool(
  "power_setPinVoltage",
  { pin: z.number(), voltage: z.number() },
  async ({ pin, voltage }) => {
    return {
      code: `EVA.decimalDataWrite(${pin}, ${voltage});`,
      comment: `Set virtual power pin ${pin} to ${voltage} volts.`,
      requires: ['#include "myeva.h"'],
      category: "Power",
    };
  }
);

server.tool(
  "carbonEmission_initialize_all",
  async () => {
    return {
      code: `
// [global declaration]
double totalPower = 0.0;
double workingPower = 0.0;
double carbonEmission = 0.0;
long ce_count = 0;
long ce_lasttime = 0;
      `,
      comment: `Add the code to [global declaration].`,
      requires: ['Declare the global variables that will be used to save data'],
      category: "Energy",
    };
  }
);

server.tool(
  "carbonEmission",
  async () => {
    return {
      code: `   if (millis() - ce_lasttime > 1000) {
                    ce_count++;
                    ce_lasttime = millis();
                    double powerLocal = EVA.getUsVoltage(-1)*EVA.getIsCurrent(-1) + EVA.getUpVoltage(-1)*EVA.getIpCurrent(-1);
                    double powerRemote = 0.0;
                    int slaveCount = EcatMaster.getSlaveCount();
                    for(int i=0;i<slaveCount;i++){
                      powerRemote += EVA.getUsVoltage(EcatMaster.getAliasAddress(i))*EVA.getIsCurrent(EcatMaster.getAliasAddress(i))
                                    + EVA.getUpVoltage(EcatMaster.getAliasAddress(i))*EVA.getIpCurrent(EcatMaster.getAliasAddress(i));
                    }
                    totalPower += powerLocal + powerRemote;
                    workingPower = powerLocal + powerRemote;
                    carbonEmission = ((totalPower / 1000) / 3600) / 0.4997; // kWh to CO2 emission
                }
      `,
      comment: `Calculate total power, working power, and carbon emission every second.`,
      requires: ['#include "myeva.h"',"EcatMaster instance must exist"],
      category: "Energy",
    };
  }
);

server.tool(
  "servo_virtualSetVelocity",
  { servoNumber: z.number(), velocity: z.number() },
  async ({ servoNumber, velocity }) => {
    return {
      code: `VirtualServo${servoNumber}.setVelocity(${velocity});`,
      comment: `Set the velocity of VirtualServo${servoNumber} to ${velocity}. The VirtualServo${servoNumber} instance as defined by myeva.h.`,
      requires: ['#include "myeva.h"'],
      category: "EVA Servo",
    };
  }
);

server.tool(
  "servo_virtualSetAcceleration",
  { servoNumber: z.number(), acceleration: z.number() },
  async ({ servoNumber, acceleration }) => {
    return {
      code: `VirtualServo${servoNumber}.setAcceleration(${acceleration});`,
      comment: `Set the acceleration of VirtualServo${servoNumber} to ${acceleration}.`,
      requires: ['#include "myeva.h"'],
      category: "EVA Servo",
    };
  }
);

server.tool(
  "servo_virtualWrite",
  { servoNumber: z.number(), angle: z.number() },
  async ({ servoNumber, angle }) => {
    return {
      code: `VirtualServo${servoNumber}.write(${angle});`,
      comment: `Move VirtualServo${servoNumber} to angle ${angle} degrees.`,
      requires: ['#include "myeva.h"'],
      category: "EVA Servo",
    };
  }
);

server.tool(
  "servo_virtualRead",
  { servoNumber: z.number(), angle: z.number() },
  async ({ servoNumber, angle }) => {
    return {
      code: `VirtualServo${servoNumber}.read();`,
      comment: `Returen the angle degrees by reading from VirtualServo${servoNumber}.`,
      requires: ['#include "myeva.h"'],
      category: "EVA Servo",
    };
  }
);

server.tool(
  "servo_virtualIsMoving",
  { servoNumber: z.number() },
  async ({ servoNumber }) => {
    return {
      code: `VirtualServo${servoNumber}.isMoving();`,
      comment: `Return true if VirtualServo${servoNumber} is currently moving.`,
      requires: ['#include "myeva.h"'],
      category: "EVA Servo",
    };
  }
);

server.tool(
  "encoder_virtualWrite",
  { encoderNumber: z.number(), value: z.number() },
  async ({ encoderNumber, value }) => {
    return {
      code: `VirtualEncoder${encoderNumber}.write(${value});`,
      comment: `Set VirtualEncoder${encoderNumber} to value ${value}.`,
      requires: ['#include "myeva.h"'],
      category: "Encoder",
    };
  }
);

server.tool(
  "encoder_virtualRead",
  { encoderNumber: z.number() },
  async ({ encoderNumber }) => {
    return {
      code: `VirtualEncoder${encoderNumber}.read();`,
      comment: `Read the current value of VirtualEncoder${encoderNumber}.`,
      requires: ['#include "myeva.h"'],
      category: "Encoder",
    };
  }
);

server.tool(
  "encoder_virtualGetDirection",
  { encoderNumber: z.number() },
  async ({ encoderNumber }) => {
    return {
      code: `VirtualEncoder${encoderNumber}.directionRead();`,
      comment: `Return the rotation direction of VirtualEncoder${encoderNumber}.`,
      requires: ['#include "myeva.h"'],
      category: "Encoder",
    };
  }
);

server.tool(
  "lcd_virtualBegin",
  { lcdNumber: z.number(), lcdID: z.number() },
  async ({ lcdNumber, lcdID }) => {
    return {
      code: `VirtualLcd${lcdNumber}.begin(${lcdID});`,
      comment: `Initialize VirtualLcd${lcdNumber} with LCD ID ${lcdID} (0=2.4", 1=3.5").`,
      requires: ['#include "myeva.h"'],
      category: "LCD",
    };
  }
);

server.tool(
  "lcd_virtualFillScreen",
  { lcdNumber: z.number(), color: z.string() },
  async ({ lcdNumber, color }) => {
    return {
      code: `VirtualLcd${lcdNumber}.fillScreen(${color});`,
      comment: `Fill the entire screen of VirtualLcd${lcdNumber} with color ${color}.`,
      requires: ['#include "myeva.h"'],
      category: "LCD",
    };
  }
);

server.tool(
  "lcd_virtualSetTextColor",
  { lcdNumber: z.number(), color: z.string() },
  async ({ lcdNumber, color }) => {
    return {
      code: `VirtualLcd${lcdNumber}.setTextColor(${color});`,
      comment: `Set text color for VirtualLcd${lcdNumber} to ${color}.`,
      requires: ['#include "myeva.h"'],
      category: "LCD",
    };
  }
);

server.tool(
  "lcd_virtualPrintString",
  { lcdNumber: z.number(), message: z.string() },
  async ({ lcdNumber, message }) => {
    return {
      code: `VirtualLcd${lcdNumber}.print("${message}");`,
      comment: `Print string "${message}" on VirtualLcd${lcdNumber}.`,
      requires: ['#include "myeva.h"'],
      category: "LCD",
    };
  }
);

server.tool(
  "lcd_virtualTouchCalibration",
  { lcdNumber: z.number() },
  async ({ lcdNumber }) => {
    return {
      code: `while (VirtualLcd${lcdNumber}.TouchCalibration() == 0);`,
      comment: `Perform touch calibration for VirtualLcd${lcdNumber}.`,
      requires: ['#include "myeva.h"'],
      category: "LCD",
    };
  }
);

server.tool(
  "lcm_virtualClear",
  { lcmNumber: z.number() },
  async ({ lcmNumber }) => {
    return {
      code: `VirtualLcm${lcmNumber}.clear();`,
      comment: `Clear the display of VirtualLcm${lcmNumber}.`,
      requires: ['#include "myeva.h"'],
      category: "LCM",
    };
  }
);

server.tool(
  "lcm_virtualPrintString",
  { lcmNumber: z.number(), message: z.string() },
  async ({ lcmNumber, message }) => {
    return {
      code: `VirtualLcm${lcmNumber}.print("${message}");`,
      comment: `Print string "${message}" on VirtualLcm${lcmNumber}.`,
      requires: ['#include "myeva.h"'],
      category: "LCM",
    };
  }
);

server.tool(
  "cnc_virtualGcode",
  { cncNumber: z.number(), gcode: z.string() },
  async ({ cncNumber, gcode }) => {
    return {
      code: `VirtualCNC${cncNumber}.gcode("${gcode}");`,
      comment: `Send G-code "${gcode}" to VirtualCNC${cncNumber}.`,
      requires: ['#include "myeva.h"'],
      category: "CNC",
    };
  }
);

server.tool(
  "cnc_virtualSetPlaneXY",
  { cncNumber: z.number() },
  async ({ cncNumber }) => {
    return {
      code: `VirtualCNC${cncNumber}.gcode("G17");`,
      comment: `Set VirtualCNC${cncNumber} to XY plane (G17).`,
      requires: ['#include "myeva.h"'],
      category: "CNC",
    };
  }
);

server.tool(
  "cnc_virtualMotorIsMoving",
  { cncNumber: z.number() },
  async ({ cncNumber }) => {
    return {
      code: `VirtualCNC${cncNumber}.motorIsMoving();`,
      comment: `Return true if any motor in VirtualCNC${cncNumber} is moving.`,
      requires: ['#include "myeva.h"'],
      category: "CNC",
    };
  }
);

server.tool(
  "cnc_virtualSetDACVoltage",
  { cncNumber: z.number(), voltage: z.number() },
  async ({ cncNumber, voltage }) => {
    return {
      code: `VirtualCNC${cncNumber}.setDACVoltage(${voltage});`,
      comment: `Set DAC output voltage of VirtualCNC${cncNumber} to ${voltage} volts.`,
      requires: ['#include "myeva.h"'],
      category: "CNC",
    };
  }
);

server.tool(
  "dac_virtualWrite",
  { pin: z.number(), voltage: z.number() },
  async ({ pin, voltage }) => {
    return {
      code: `EVA.voltageWrite(${pin}, ${voltage});`,
      comment: `Set virtual DAC pin ${pin} to ${voltage} volts.`,
      requires: ['#include "myeva.h"'],
      category: "DAC",
    };
  }
);

server.tool(
  "dac_virtualRead",
  { pin: z.number() },
  async ({ pin }) => {
    return {
      code: `EVA.voltageRead(${pin});`,
      comment: `Read voltage from virtual DAC pin ${pin}.`,
      requires: ['#include "myeva.h"'],
      category: "DAC",
    };
  }
);

server.tool(
  "power_virtualGetUsVoltage",
  { number: z.number() },
  async ({ number }) => {
    return {
      code: `EVA.getUsVoltage(${number});`,
      comment: `Return the secondary voltage (Us) from virtual EVA ${number}.`,
      requires: ['#include "myeva.h"'],
      category: "Power",
    };
  }
);

server.tool(
  "power_virtualGetUpVoltage",
  { number: z.number() },
  async ({ number }) => {
    return {
      code: `EVA.getUpVoltage(${number});`,
      comment: `Return the primary voltage (Up) from virtual EVA ${number}.`,
      requires: ['#include "myeva.h"'],
      category: "Power",
    };
  }
);

server.tool(
  "power_virtualGetIsCurrent",
  { number: z.number() },
  async ({ number }) => {
    return {
      code: `EVA.getIsCurrent(${number});`,
      comment: `Return the secondary current (Is) from virtual EVA ${number}.`,
      requires: ['#include "myeva.h"'],
      category: "Power",
    };
  }
);

server.tool(
  "power_virtualGetIpCurrent",
  { number: z.number() },
  async ({ number }) => {
    return {
      code: `EVA.getIpCurrent(${number});`,
      comment: `Return the primary current (Ip) from virtual EVA ${number}.`,
      requires: ['#include "myeva.h"'],
      category: "Power",
    };
  }
);

server.tool(
  "lcd_virtualIsTouched",
  { lcdNumber: z.number() },
  async ({ lcdNumber }) => {
    return {
      code: `VirtualLcd${lcdNumber}.isTouched();`,
      comment: `Return true if VirtualLcd${lcdNumber} is touched.`,
      requires: ['#include "myeva.h"'],
      category: "LCD",
    };
  }
);

server.tool(
  "lcd_virtualGetTouchX",
  { lcdNumber: z.number() },
  async ({ lcdNumber }) => {
    return {
      code: `VirtualLcd${lcdNumber}.touchX();`,
      comment: `Return X coordinate of touch point on VirtualLcd${lcdNumber}.`,
      requires: ['#include "myeva.h"'],
      category: "LCD",
    };
  }
);

server.tool(
  "lcd_virtualGetTouchY",
  { lcdNumber: z.number() },
  async ({ lcdNumber }) => {
    return {
      code: `VirtualLcd${lcdNumber}.touchY();`,
      comment: `Return Y coordinate of touch point on VirtualLcd${lcdNumber}.`,
      requires: ['#include "myeva.h"'],
      category: "LCD",
    };
  }
);

server.tool(
  "hmi_setSoundVolume",
  { volume: z.number() },
  async ({ volume }) => {
    return {
      code: `Hmi.setSoundVolume(${volume});`,
      comment: `Set HMI sound volume to ${volume}.`,
      requires: ['#include "myhmi.h"'],
      category: "HMI",
    };
  }
);

server.tool(
  "hmi_playSound",
  { soundNumber: z.number() },
  async ({ soundNumber }) => {
    return {
      code: `Hmi.playSound(${soundNumber});`,
      comment: `Play sound file number ${soundNumber} on HMI.`,
      requires: ['#include "myhmi.h"'],
      category: "HMI",
    };
  }
);

server.tool(
  "buzzer_virtualTone",
  { buzzerPin: z.number(), frequency: z.number(), duration: z.number() },
  async ({ buzzerPin, frequency, duration }) => {
    return {
      code: `EVA.tone(${buzzerPin}, ${frequency}, ${duration});`,
      comment: `Play tone at ${frequency} Hz for ${duration} ms on virtual buzzer pin ${buzzerPin}.`,
      requires: ['#include "myeva.h"'],
      category: "Sound",
    };
  }
);

server.tool(
  "buzzer_virtualBeepOnce",
  { buzzerPin: z.number() },
  async ({ buzzerPin }) => {
    return {
      code: `EVA.tone(${buzzerPin}, 1000, 100);`,
      comment: `Emit a short beep on virtual buzzer pin ${buzzerPin}.`,
      requires: ['#include "myeva.h"'],
      category: "Sound",
    };
  }
);

server.tool(
  "eva_virtualGetTemperature",
  { number: z.number() },
  async ({ number }) => {
    return {
      code: `EVA.getTemperature(${number});`,
      comment: `Return temperature from virtual EVA ${number}.`,
      requires: ['#include "myeva.h"'],
      category: "EVA",
    };
  }
);

server.tool(
  "eva_virtualGetWorkingHours",
  { number: z.number() },
  async ({ number }) => {
    return {
      code: `EVA.getWorkingHours(${number});`,
      comment: `Return working hours from virtual EVA ${number}.`,
      requires: ['#include "myeva.h"'],
      category: "EVA",
    };
  }
);

server.tool(
  "eva_virtualGetBootTimes",
  { number: z.number() },
  async ({ number }) => {
    return {
      code: `EVA.getBootTimes(${number});`,
      comment: `Return boot times from virtual EVA ${number}.`,
      requires: ['#include "myeva.h"'],
      category: "EVA",
    };
  }
);

server.tool(
  "lcm_virtualSetCursorHome",
  { lcmNumber: z.number() },
  async ({ lcmNumber }) => {
    return {
      code: `VirtualLcm${lcmNumber}.home();`,
      comment: `Set cursor of VirtualLcm${lcmNumber} to home position.`,
      requires: ['#include "myeva.h"'],
      category: "LCM",
    };
  }
);

server.tool(
  "lcm_virtualSetCursorPosition",
  { lcmNumber: z.number(), x: z.number(), y: z.number() },
  async ({ lcmNumber, x, y }) => {
    return {
      code: `VirtualLcm${lcmNumber}.setCursor(${x}, ${y});`,
      comment: `Set cursor position of VirtualLcm${lcmNumber} to (${x}, ${y}).`,
      requires: ['#include "myeva.h"'],
      category: "LCM",
    };
  }
);

server.tool(
  "lcm_virtualWriteData",
  { lcmNumber: z.number(), data: z.number() },
  async ({ lcmNumber, data }) => {
    return {
      code: `VirtualLcm${lcmNumber}.write(${data});`,
      comment: `Write data ${data} to VirtualLcm${lcmNumber}.`,
      requires: ['#include "myeva.h"'],
      category: "LCM",
    };
  }
);

server.tool(
  "record_libraryHeaderFile",
  async () => {
    return {
      code: `#include "recordData.h"`,
      comment: `This library will record data into internal flash. Add the code to [other include files].`,
      requires: [],
      category: "RECORD",
    };
  }
);

server.tool(
  "record_useUSBDiskAndStaticIP",
  async () => {
    return {
      code: `recordInitUSBDiskStaticIP();`,
      comment: `Initialize the USB disk for recording data and ethernet with static IP. Add the code to [setup code].`,
      requires: ['#include "recordData.h"'],
      category: "RECORD",
    };
  }
);

server.tool(
  "record_useUSBDiskAndDHCP",
  async () => {
    return {
      code: `recordInitUSBDiskDHCP();`,
      comment: `Initialize the USB disk for recording data and ethernet with DHCP. Add the code to [setup code].`,
      requires: ['#include "recordData.h"'],
      category: "RECORD",
    };
  }
);

server.tool(
  "record_getCurrentIP",
  async () => {
    return {
      code: `getCurrentIP();`,
      comment: `Return the IP with "char*" type, not "const char*".`,
      requires: ['#include "recordData.h"'],
      category: "RECORD",
    };
  }
);

server.tool(
  "record_InISR",
  async () => {
    return {
      code: `recordDataInISR();`,
      comment: `It is a recording data function called in EtherCAT callback function.`,
      requires: ['#include "recordData.h"'],
      category: "RECORD",
    };
  }
);

server.tool(
  "record_RoutineLoop",
  async () => {
    return {
      code: `recordRoutineData();`,
      comment: `It is a recording data function called in a task or loop function.`,
      requires: ['#include "recordData.h"'],
      category: "RECORD",
    };
  }
);

server.tool(
  "record_ElapsedTimeBySetupFunction",
  { codeBlock: z.string() },
  async ({ codeBlock }) => {
    return {
      code: `unsigned long _t0 = micros(); ${codeBlock}; recordSetupElapsedTime((micros() - _t0)/1000.0);`,
      comment: `Calculate the elapsed time of the ${codeBlock} and record the elapsed time of setup function by milliseconds.`,
      requires: ['#include "recordData.h"'],
      category: "RECORD",
    };
  }
);

server.tool(
  "record_ElapsedTimeByLoopFunction",
  { codeBlock: z.string() },
  async ({ codeBlock }) => {
    return {
      code: `unsigned long _t0 = micros(); ${codeBlock}; recordLoopElapsedTime((micros() - _t0)/1000.0);`,
      comment: `Calculate the elapsed time of the ${codeBlock} and record the elapsed time of loop function by milliseconds.`,
      requires: ['#include "recordData.h"'],
      category: "RECORD",
    };
  }
);

server.tool(
  "record_ElapsedTimeByOneTask",
  { codeBlock: z.string(), task_number: z.number() },
  async ({ codeBlock, task_number }) => {
    return {
      code: `unsigned long _t0 = micros(); ${codeBlock}; recordTaskElapsedTime(${task_number}, (micros() - _t0)/1000.0);`,
      comment: `Calculate the elapsed time of the ${codeBlock} and record the elapsed time in one task by milliseconds. The ${task_number} from 1 to start.`,
      requires: ['#include "recordData.h"'],
      category: "RECORD",
    };
  }
);

server.tool(
  "record_ModbusWrittenData",
  { 
    slaveID: z.number(),
    address: z.number(),
    bitSize: z.number(),
    data: z.number()
  },
  async ({ slaveID, address, bitSize, data }) => {
    return {
      code: `uint16_t writtenData = ${data};
             recordModbusWriteData(${slaveID}, ${address}, ${bitSize}, writtenData);`,
      comment: `It is a function to record the written data of Modbus.`,
      requires: ['#include "recordData.h"'],
      category: "RECORD",
    };
  }
);

server.tool(
  "record_ModbusReadData",
  { 
    slaveID: z.number(),
    address: z.number(),
    bitSize: z.number(),
    data: z.number()
  },
  async ({ slaveID, address, bitSize, data }) => {
    return {
      code: `uint16_t readData = ${data};
             recordModbusReadData(${slaveID}, ${address}, ${bitSize}, readData);`,
      comment: `It is a function to record the read data of Modbus.`,
      requires: ['#include "recordData.h"'],
      category: "RECORD",
    };
  }
);

server.tool(
  "example_codeGeneration",
  async () => {
    return {
      code: `   #include "myeva.h"
                #include "TimerWDT.h"
                #include "RTCZero.h"
                #include "EEPROM.h"
                #include "SCoop.h"
                #include "myhmi.h"
                
                RTCZero rtc;
                
                void setup() {
                  Serial.begin(3000000);
                  rtc.begin();
                  EVA.begin();
                  Hmi.begin();
                  tone(PCSPEAKER, 1000, 100);
                  Hmi.setSoundVolume(100);
                  Hmi.playSound(1);
                  VirtualSerial1.begin(115200);
                  for (int i=0; i<8; i++) EVA.digitalWrite(i, HIGH); // Set Virtual EVA GPIO pin 0~7 to HIGH
                  TimerWDT.initialize(6000000,true);
                  mySCoop.start(0);
                }
                
                void loop() {
                  TimerWDT.reset();
                  BEGIN_HMI_EVENT_PROC { // put BEGIN_HMI_EVENT_PROC before the start of HMI events.
                  if (Hmi.buttonClicked(p1b1)) { Serial.pritnln("Click"); }
                  }
                  END_HMI_EVENT_PROC // put END_HMI_EVENT_PROC after the end of HMI events.
                  if (EVA.digitalRead(0) == LOW) { Hmi.setLedBright(p1led1, 0); }
                }
                
                defineTaskLoop(scoopTask1){
                  Serial.println("Hello1");
                  delay(500);
                }
                
                defineTaskLoop(scoopTask2){
                  VirtualSerial1.println("Hello2");
                  delay(1000);
                }
      `,
      comment: `User's prompts:
                1. In the setup()
                The host emits a short beep sound.
                The host sets the HMI volume to 100 and then plays the first voice file.
                Virtual Serial1 is initialized and set to a baud rate of 115200.
                Set Virtual EVA GPIO pin 0~7 to HIGH.
                Before setup() completes, the watchdog timer is started using TimerWDT.initialize(). Finally, the multitasking scheduler is started with mySCoop.start(0) to ensure the cooperative scheduler is launched in the correct environment.
                2. In the loop(), if the HMI button p1b1 is pressed, use Serial.print to print "Click" followed by a newline. If virtual EVA GPIO pin0 is LOW, set HMI LDE p1led1 to 0.
                3. Use defineTaskLoop(), //to handle periodic tasks (all delay() calls are centralized here)// 
                Task: Every 500 ms, use Serial.print to print "Hello1" followed by a newline.
                Task: Every 1000 ms, use Virtual Serial1 to print "Hello2" followed by a newline.`,
      requires: [],
      category: "Example",
    };
  }
);

async function main() {
  const transport = new StdioServerTransport();
  await server.connect(transport);
  console.error("QEC MCP v2.0 Running");
}

main().catch((error) => {
  console.error("Fatal error in main():", error);
  process.exit(1);
});
