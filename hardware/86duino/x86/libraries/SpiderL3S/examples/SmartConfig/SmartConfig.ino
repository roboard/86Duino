/*****************************************************************************
*
*  SmartCongig.ino - Application shows how to use smartconfig function
*  connecting a WiFi access point.
*
*  Copyright (c) 2014, FunMaker Ltd.
*
*
*  This library porting from CC3000 host driver, which works with 
*  Spider_L3S WiFi module.
*
*  Spider_L3S wifi module is developed by Funmaker, we are actively 
*  involved in Taiwan maker community, and we aims to support makers 
*  to make more creative projects. 
*
*  You can support us, by buying this wifi module, and we are looking
*  forward to see your awesome projects!
*
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <SPI.h>

#include "Spider_L3.h"

// Configure your WiFi module pin connection.
unsigned char WLAN_CS = 43;
unsigned char WLAN_EN = 44;
unsigned char WLAN_IRQ = 42;
unsigned char WLAN_IRQ_INTNUM = 0;


const int INDICATE_LED = 13;
const int SMART_CONFIG_PIN = 8;

void setup() {          
    int ret = 0;
    /* initial uart debug output interface. */
    Serial.begin(115200);

    while (!Serial) {
      ; // wait for serial port to connect. Needed for Leonardo only
    }

    Serial.println("=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+");
    Serial.println("     Spider L3 smart config demo.     ");
    Serial.println("=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+");

    /* initial status LED pin */
    pinMode(INDICATE_LED, OUTPUT);
    digitalWrite(INDICATE_LED, LOW);

    /* initial smart config enable pin */
    pinMode(SMART_CONFIG_PIN, INPUT);

    /* Initial Spider L3 spi interface*/
    Serial.print("Starting Spider L3...");
    ret = Spider_begin();
    if(ret != 0){
        while(1);
    }
    Serial.println("ok");

    // Spider L3 SPI interface initial success, indicated LED signal.
    digitalWrite(INDICATE_LED, HIGH); 
    delay(100);
    digitalWrite(INDICATE_LED, LOW);

    /* Checking GPIO pin selecting use smart config setting connect profile or connect to network */ 
    Serial.print("Check select pin:");
    Serial.print(SMART_CONFIG_PIN, DEC);
    Serial.print(",level...");

    if(digitalRead(SMART_CONFIG_PIN) == HIGH){
        Serial.println("HIGH");
        Serial.print("Starting smart config...");
        Spider_SmartConfig();
    }
    else{
        Serial.println("LOW");
        Serial.print("Starting auto connect...");
        Spider_AutoConnect();
    }
    Serial.println("ok");

    Serial.print("Waiting DHCP...");
    /* wait connection and Get DHCP address finished */
    while((Spider_CheckConnected() != 0) || (Spider_CheckDHCP() != 0));
    Serial.println("ok");

    // Spider L3 connect success, indicated LED signal.
    digitalWrite(INDICATE_LED, HIGH); 
    delay(100);
    digitalWrite(INDICATE_LED, LOW);

    /* Do smart config ack */
    Spider_SmartConfig_AckFinish("SpiderL3");

    /* show system IP address */
    tNetappIpconfigRetArgs inf;
    netapp_ipconfig(&inf);

    Serial.print("Device's IP address:");
    Serial.print(inf.aucIP[3] ,DEC);
    Serial.print(".");
    Serial.print(inf.aucIP[2] ,DEC);
    Serial.print(".");
    Serial.print(inf.aucIP[1] ,DEC);
    Serial.print(".");
    Serial.print(inf.aucIP[0] ,DEC);
    Serial.print("\r\n");
}

void loop() {
}