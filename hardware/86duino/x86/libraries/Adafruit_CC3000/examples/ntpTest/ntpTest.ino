/*************************************************** 
  This is an example for the Adafruit CC3000 Wifi Breakout & Shield

  Designed specifically to work with the Adafruit WiFi products:
  ----> https://www.adafruit.com/products/1469

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Kevin Townsend & Limor Fried & Rick Lesniak for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

/*

This example does a test of the SNTP (Simple Network Time Protocol) client:
* Initialization
* SSID Scan
* AP connection
* DHCP printout
* SNTP time synchronization
* Extract and print current time and date information
 
*/

#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
//#include "utility/NetTime.h"
#include <string.h>
#include "utility/debug.h"
#include "utility/sntp.h"

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   42  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  44
#define ADAFRUIT_CC3000_CS    43
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed but DI

#define WLAN_SSID       "myNetwork"   // cannot be longer than 32 characters!
#define WLAN_PASS       "myPassword"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

//Arguments to SNTP client constructor:
//	1 - Primary Network Time Server URL (can be NULL)
//	2 - Secondary Network Time Server URL (also can be NULL)
//	3 - Local UTC offset in minutes (US Eastern Time is UTC - 5:00
//	4 - Local UTC offset in minutes for Daylight Savings Time (US Eastern DST is UTC - 4:00
//	5 - Enable Daylight Savings Time adjustment (not implemented yet)
//
sntp mysntp = sntp(NULL, "time.nist.gov", (short)(-5 * 60), (short)(-4 * 60), true);

// Type SNTP_Timestamp is 64-bit NTP time. High-order 32-bits is seconds since 1/1/1900
//   Low order 32-bits is fractional seconds
SNTP_Timestamp_t now;

// Type NetTime_t contains NTP time broken out to human-oriented values:
//	uint16_t millis; ///< Milliseconds after the second (0..999)
//	uint8_t	 sec;    ///< Seconds after the minute (0..59)
//	uint8_t	 min;    ///< Minutes after the hour (0..59)
//	uint8_t	 hour;   ///< Hours since midnight (0..23)
//	uint8_t	 mday;   ///< Day of the month (1..31)
//	uint8_t	 mon;    ///< Months since January (0..11)
//	uint16_t year;   ///< Year.
//	uint8_t	 wday;	 ///< Days since Sunday (0..6)
//	uint8_t	 yday;   ///< Days since January 1 (0..365)
//	bool	 isdst;  ///< Daylight savings time flag, currently not supported	
NetTime_t timeExtract;

const char   janStr[]  = "January";
const char   febStr[]  = "February";
const char   marStr[]  = "March";
const char   aprStr[]  = "April";
const char   mayStr[]  = "May";
const char   junStr[]  = "June";
const char   julStr[]  = "July";
const char   augStr[]  = "August";
const char   sepStr[]  = "September";
const char   octStr[]  = "October";
const char   novStr[]  = "November";
const char   decStr[]  = "December"; 

const char* monthStrs[] = { janStr, febStr, marStr, aprStr, mayStr, junStr,
                            julStr, augStr, sepStr, octStr, novStr, decStr}; 

const char   sunStr[]  = "Sunday";
const char   monStr[]  = "Monday";
const char   tueStr[]  = "Tuesday";
const char   wedStr[]  = "Wednesday";
const char   thuStr[]  = "Thursday";
const char   friStr[]  = "Friday";
const char   satStr[]  = "Saturday"; 

const char* const dayStrs[] = { sunStr, monStr, tueStr,  wedStr,
                                        thuStr, friStr, satStr};


/**************************************************************************/
/*!
    @brief  Sets up the HW and the CC3000 module (called automatically
            on startup)
*/
/**************************************************************************/
void setup(void)
{
  Serial.begin(115200);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  Serial.println("Hello, CC3000!\n"); 

  //Serial.print("Free RAM: "); //Serial.println(getFreeRam(), DEC);

 
  /* Initialise the module */
  Serial.println("\nInitialising the CC3000 ...");
  if (!cc3000.begin())
  {
    Serial.println("Unable to initialise the CC3000! Check your wiring?");
    while(1);
  }

  /* Optional: Update the Mac Address to a known value */
/*
  uint8_t macAddress[6] = { 0x08, 0x00, 0x28, 0x01, 0x79, 0xB7 };
   if (!cc3000.setMacAddress(macAddress))
   {
     Serial.println(F("Failed trying to update the MAC address"));
     while(1);
   }
*/
  
  uint16_t firmware = checkFirmwareVersion();
  if (firmware < 0x113) {
    Serial.println("Wrong firmware version!");
    for(;;);
  } 
  
  /* Delete any old connection data on the module */
  Serial.println("\nDeleting old connection profiles");
  if (!cc3000.deleteProfiles()) {
    Serial.println("Failed!");
    while(1);
  }

  /* Attempt to connect to an access point */
  char *ssid = WLAN_SSID;             /* Max 32 chars */
  Serial.print("\nAttempting to connect to "); Serial.println(ssid);
  
  /* NOTE: Secure connections are not available in 'Tiny' mode! */
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println("Failed!");
    while(1);
  }
   
  Serial.println("Connected!");
  
  /* Wait for DHCP to complete */
  Serial.println("Request DHCP");
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }  

  Serial.println("UpdateNTPTime");
  mysntp.UpdateNTPTime();

  Serial.println("Current local time is:");
  mysntp.ExtractNTPTime(mysntp.NTPGetTime(&now, true), &timeExtract);

  Serial.print(timeExtract.hour); Serial.print(":"); Serial.print(timeExtract.min); Serial.print(":");Serial.print(timeExtract.sec); Serial.print(".");Serial.println(timeExtract.millis);
  Serial.print(dayStrs[timeExtract.wday]); Serial.print(", "); Serial.print(monthStrs[timeExtract.mon]); Serial.print(" "); Serial.print(timeExtract.mday); Serial.print(", ");Serial.println(timeExtract.year);
  Serial.print("Day of year: "); Serial.println(timeExtract.yday + 1); 

  /* You need to make sure to clean up after yourself or the CC3000 can freak out */
  /* the next time you try to connect ... */
  Serial.println("\n\nClosing the connection");
  cc3000.disconnect();
}

void loop(void)
{
    delay(1000);
}


/**************************************************************************/
/*!
    @brief  Tries to read the CC3000's internal firmware patch ID
*/
/**************************************************************************/
uint16_t checkFirmwareVersion(void)
{
  uint8_t major, minor;
  uint16_t version;
  
#ifndef CC3000_TINY_DRIVER  
  if(!cc3000.getFirmwareVersion(&major, &minor))
  {
    Serial.println("Unable to retrieve the firmware version!\r\n");
    version = 0;
  }
  else
  {
    Serial.print("Firmware V. : ");
    Serial.print(major); Serial.print("."); Serial.println(minor);
    version = major; version <<= 8; version |= minor;
  }
#endif
  return version;
}