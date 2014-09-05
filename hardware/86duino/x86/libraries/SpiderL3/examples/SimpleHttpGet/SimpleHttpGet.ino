/*****************************************************************************
*
*  SimpleHttpGet.ino - Simple Yahoo weather RSS information get 
*  example through Yahoo API interface.
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

#include <SPI.h>
#include <stdio.h>
#include <string.h>

#include "Spider_L3.h"
#include "WebClient.h"

// Configure your WiFi module pin connection.
unsigned char WLAN_CS = 43;
unsigned char WLAN_EN = 44;
unsigned char WLAN_IRQ = 42;
unsigned char WLAN_IRQ_INTNUM = 0;

// Don't forget set correct WiFi SSID and Password.
char AP_Ssid[] = {"WIFISSID"};
char AP_Pass[] = {"12345678"};

const int INDICATE_LED = 13;

void setup() {          
    int ret = 0;
    /* initial uart message output interface. */
    Serial.begin(115200);

    while (!Serial) {
      ; // wait for serial port to connect. Needed for Leonardo only
    }

    /* initial status LED pin */
    pinMode(INDICATE_LED, OUTPUT);
    digitalWrite(INDICATE_LED, LOW);

    /* Initial Spider L3 */
    ret = Spider_begin();
    if(ret != 0){
        while(1){};
    }
    
    // Spider L3 SPI interface initial success, indicated LED signal.
    digitalWrite(INDICATE_LED, HIGH); 
    delay(100);
    digitalWrite(INDICATE_LED, LOW);

    /* Connect to WiFi AP */
    ret = Spider_Connect(3, AP_Ssid, AP_Pass);
    if(ret != 0){
        while(1){};
    }

    /* wait connection and Get DHCP address finished */
    while((Spider_CheckConnected() != 0) || (Spider_CheckDHCP() != 0)) {};

    // Spider L3 connect success, indicated LED signal.
    digitalWrite(INDICATE_LED, HIGH); 
    delay(100);
    digitalWrite(INDICATE_LED, LOW);

    /* Print device's IP address */
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

/* For contex switch use */
#define CONNECT_BEGIN       0x01
#define CONNECT_SERVER      0x02
unsigned char app_st = CONNECT_BEGIN;

/* Target URL link */
char api_url_link[] = {"http://weather.yahooapis.com/forecastrss?w=2306179&u=c"};

/* Target server's information */
Host_Info host_inf;
unsigned char ip[4];

/* Buffer for storage information from webpage */
char ret_data[100];
int ret = 0;

/* Timer register */
unsigned long task_timer = 0;
unsigned long break_tmr = 0;

/* Counting send http request times */
int get_count = 0;


void loop() {
    if(millis() > task_timer){
        switch(app_st){

            case CONNECT_BEGIN:

                /* Use gethostbyname api to get target server's IP address. */
                Serial.print("Get target server IP...");
                memset(ip, 0, sizeof(ip));
                ret = WebClient_Get_HostIP(api_url_link, &ip[0], &ip[1], &ip[2], &ip[3]);

                if(ret != 0){
                    task_timer = millis() + 1000;
                    Serial.println("failed");
                    break;
                }
                Serial.println("ok");

                Serial.print("Target server IP address:");
                Serial.print(ip[0] ,DEC);
                Serial.print(".");
                Serial.print(ip[1] ,DEC);
                Serial.print(".");
                Serial.print(ip[2] ,DEC);
                Serial.print(".");
                Serial.print(ip[3] ,DEC);
                Serial.print("\r\n");

                /* Get ip address, switch to CONNECT_SERVER status*/
                get_count = 0;
                app_st = CONNECT_SERVER;
                task_timer = millis() + 100;
                break;

            case CONNECT_SERVER:

                /* Initial host_inf socket option */
                Serial.print("Set target server's socket...");
                ret = WebClient_Begin_IP(&host_inf, ip[0],  ip[1],  ip[2],  ip[3], 80);
                if(ret != 0){
                    Serial.println("failed");
                    WebClient_Close(&host_inf);
                    task_timer = millis() + 1000;
                    break;
                }
                Serial.println("ok");

                /* Connect to WebServer */
                Serial.print("Connect to target server...");
                ret = WebClient_Connect(&host_inf);
                if(ret != 0){
                    Serial.println("failed");
                    WebClient_Close(&host_inf);
                    task_timer = millis() + 1000;
                    break;
                }
                Serial.println("ok");

                /* Send http request to WebServer */
                Serial.print("Send request to target server...");
                ret = WebClient_SendRequest(&host_inf, api_url_link);
                if(ret != 0){
                    Serial.println("failed");
                    /* Send request error, close connection. */
                    WebClient_Close(&host_inf);
                    task_timer = millis() + 1000;
                    break;
                }
                Serial.println("ok");

                /* Print request success count. */
                get_count++;
                Serial.print("No.");
                Serial.print(get_count, DEC);
                Serial.println(" http request send.");

                /* Get information, timeout is 1500ms */
                break_tmr = millis() + 1500;
                while(1){
                    /* Check data available*/
                    if(WebClient_DataAvailable(&host_inf) > 0){
                        /* Receiving data... */
                        memset(ret_data, 0, sizeof(ret_data));
                        ret = WebClient_RecvData(&host_inf, ret_data, sizeof(ret_data));
                        if(ret > 0){
                            /* Received success, refresh timeout timer */
                            break_tmr = millis() + 1500;

                            /* Do information parsing */
                            char* str1;
                            str1 = strstr(ret_data, "temp=\"");
                            if(str1 != 0){
                                digitalWrite(INDICATE_LED, HIGH);
                                
                                char* str2 = 0;
                                int temp = 0;
 
                                str1 += strlen("temp=\"");
                                str2 = strstr(str1, "\"");
                                *str2 = 0;
                                sscanf(str1, "%d", &temp);

                                /* Print tempecture */
                                Serial.print("Tempecture of Taipei city:");
                                Serial.print(temp, DEC);
                                Serial.println();

                                digitalWrite(INDICATE_LED, LOW);
                            }

                        }
                    }

                    /* Check timeout */
                    if(millis() > break_tmr){
                        Serial.println("Timeout, breaking recive loop.");
                        break;
                    } 
                }

                /* http get complete, close connection */
                WebClient_Close(&host_inf);
                Serial.println("Connection closed.");

                /* Wait 1000ms and do http get again */
                task_timer = millis() + 100;
                break;
        }
    }
}