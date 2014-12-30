/*************************************************** 
  This is an example for the Adafruit CC3000 Wifi Breakout & Shield

  Designed specifically to work with the Adafruit WiFi products:
  ----> https://www.adafruit.com/products/1469

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Kevin Townsend & Limor Fried for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

/*

This example does a full test of core connectivity:
* Initialization
* SSID Scan
* AP connection
* DHCP printout
* DNS lookup
* Ping
* Disconnect
It's a good idea to run this sketch when first setting up the
module.

*/

#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   42  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  44
#define ADAFRUIT_CC3000_CS    43
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed but DI

#define WLAN_SSID       "myNetwork"        // cannot be longer than 32 characters!
#define WLAN_PASS       "myPassword"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2



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

  displayDriverMode();
  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);
  
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
     Serial.println("Failed trying to update the MAC address");
     while(1);
   }
*/
  
  uint16_t firmware = checkFirmwareVersion();
  if (firmware < 0x113) {
    Serial.println("Wrong firmware version!");
    for(;;);
  } 
  
  displayMACAddress();
  
  /* Optional: Get the SSID list (not available in 'tiny' mode) */
#ifndef CC3000_TINY_DRIVER
  listSSIDResults();
#endif
  
  /* Delete any old connection data on the module */
  Serial.println("\nDeleting old connection profiles");
  if (!cc3000.deleteProfiles()) {
    Serial.println("Failed!");
    while(1);
  }

  /* Optional: Set a static IP address instead of using DHCP.
     Note that the setStaticIPAddress function will save its state
     in the CC3000's internal non-volatile memory and the details
     will be used the next time the CC3000 connects to a network.
     This means you only need to call the function once and the
     CC3000 will remember the connection details.  To switch back
     to using DHCP, call the setDHCP() function (again only needs
     to be called once).
  */
  /*
  uint32_t ipAddress = cc3000.IP2U32(192, 168, 1, 19);
  uint32_t netMask = cc3000.IP2U32(255, 255, 255, 0);
  uint32_t defaultGateway = cc3000.IP2U32(192, 168, 1, 1);
  uint32_t dns = cc3000.IP2U32(8, 8, 4, 4);
  if (!cc3000.setStaticIPAddress(ipAddress, netMask, defaultGateway, dns)) {
    Serial.println("Failed to set static IP!");
    while(1);
  }
  */
  /* Optional: Revert back from static IP addres to use DHCP.
     See note for setStaticIPAddress above, this only needs to be
     called once and will be remembered afterwards by the CC3000.
  */
  /*
  if (!cc3000.setDHCP()) {
    Serial.println("Failed to set DHCP!");
    while(1);
  }
  */

  /* Attempt to connect to an access point */
  char *ssid = WLAN_SSID;             /* Max 32 chars */
  Serial.print("\nAttempting to connect to "); Serial.println(ssid);
  
  /* NOTE: Secure connections are not available in 'Tiny' mode!
     By default connectToAP will retry indefinitely, however you can pass an
     optional maximum number of retries (greater than zero) as the fourth parameter.
  */
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

  /* Display the IP address DNS, Gateway, etc. */  
  while (! displayConnectionDetails()) {
    delay(1000);
  }
  
#ifndef CC3000_TINY_DRIVER
  /* Try looking up www.adafruit.com */
  uint32_t ip = 0;
  Serial.print("www.adafruit.com -> ");
  while  (ip  ==  0)  {
    if  (!  cc3000.getHostByName("www.adafruit.com", &ip))  {
      Serial.println("Couldn't resolve!");
    }
    delay(500);
  }  
  cc3000.printIPdotsRev(ip);
  
  /* Do a quick ping test on adafruit.com */  
  Serial.print("\n\rPinging "); cc3000.printIPdotsRev(ip); Serial.print("...");  
  uint8_t replies = cc3000.ping(ip, 5);
  Serial.print(replies); Serial.println(" replies");
  if (replies)
    Serial.println("Ping successful!");
#endif

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
    @brief  Displays the driver mode (tiny of normal), and the buffer
            size if tiny mode is not being used

    @note   The buffer size and driver mode are defined in cc3000_common.h
*/
/**************************************************************************/
void displayDriverMode(void)
{
  #ifdef CC3000_TINY_DRIVER
    Serial.println("CC3000 is configure in 'Tiny' mode");
  #else
    Serial.print("RX Buffer : ");
    Serial.print(CC3000_RX_BUFFER_SIZE);
    Serial.println(" bytes");
    Serial.print("TX Buffer : ");
    Serial.print(CC3000_TX_BUFFER_SIZE);
    Serial.println(" bytes");
  #endif
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

/**************************************************************************/
/*!
    @brief  Tries to read the 6-byte MAC address of the CC3000 module
*/
/**************************************************************************/
void displayMACAddress(void)
{
  uint8_t macAddress[6];
  
  if(!cc3000.getMacAddress(macAddress))
  {
    Serial.println("Unable to retrieve MAC Address!\r\n");
  }
  else
  {
    Serial.print("MAC Address : ");
    cc3000.printHex((byte*)&macAddress, 6);
  }
}


/**************************************************************************/
/*!
    @brief  Tries to read the IP address and other connection details
*/
/**************************************************************************/
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println("Unable to retrieve the IP Address!\r\n");
    return false;
  }
  else
  {
    Serial.print("\nIP Addr: "); cc3000.printIPdotsRev(ipAddress);
    Serial.print("\nNetmask: "); cc3000.printIPdotsRev(netmask);
    Serial.print("\nGateway: "); cc3000.printIPdotsRev(gateway);
    Serial.print("\nDHCPsrv: "); cc3000.printIPdotsRev(dhcpserv);
    Serial.print("\nDNSserv: "); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}

/**************************************************************************/
/*!
    @brief  Begins an SSID scan and prints out all the visible networks
*/
/**************************************************************************/

void listSSIDResults(void)
{
  uint32_t index;
  uint8_t valid, rssi, sec;
  char ssidname[33]; 

  if (!cc3000.startSSIDscan(&index)) {
    Serial.println("SSID scan failed!");
    return;
  }

  Serial.print("Networks found: "); Serial.println(index);
  Serial.println("================================================");

  while (index) {
    index--;

    valid = cc3000.getNextSSID(&rssi, &sec, ssidname);
    
    Serial.print("SSID Name    : "); Serial.print(ssidname);
    Serial.println();
    Serial.print("RSSI         : ");
    Serial.println(rssi);
    Serial.print("Security Mode: ");
    Serial.println(sec);
    Serial.println();
  }
  Serial.println("================================================");

  cc3000.stopSSIDscan();
}
