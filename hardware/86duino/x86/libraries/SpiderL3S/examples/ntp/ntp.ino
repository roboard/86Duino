/*****************************************************************************
*
*  ntp.ino - Simple network time protocol client example.
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

// GMT offset in program.
#define MY_GMT              8

// Update times per minute peroidic in ms.
#define UPDATE_TIME_PEROID  500

// Configure your WiFi module pin connection.
unsigned char WLAN_CS = 43;
unsigned char WLAN_EN = 44;
unsigned char WLAN_IRQ = 42;
unsigned char WLAN_IRQ_INTNUM = 0;


const int INDICATE_LED = 13;

// Don't forget set correct WiFi SSID and Password.
char AP_Ssid[] = {"WIFISSID"};
char AP_Pass[] = {"12345678"};

long ntp_socket = 0xFFFFFFFF;
sockaddr    host_addr;
sockaddr    recv_addr;
socklen_t   recv_addr_l;

int Initial_get_time(void);

void Initial_Spider(void) {          
    int ret = 0;
    /* initial uart message output interface. */
    Serial.begin(115200);

    while (!Serial) {
      ; // wait for serial port to connect. Needed for Leonardo only
    }

    Serial.println("=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+");
    Serial.println("        Spider L3 ntp client.         ");
    Serial.println("=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+");

    /* initial status LED pin */
    pinMode(INDICATE_LED, OUTPUT);
    digitalWrite(INDICATE_LED, LOW);

    /* Initial Spider L3 */
    Serial.print("Starting Spider L3...");
    ret = Spider_begin();
    if(ret != 0){
        while(1){};
    }
    Serial.println("ok");

    // Spider L3 SPI interface initial success, indicated LED signal.
    digitalWrite(INDICATE_LED, HIGH); 
    delay(100);
    digitalWrite(INDICATE_LED, LOW);

    /* Connect to WiFi AP */
    Serial.print("Connect to ");
    Serial.write((unsigned char*)AP_Ssid, strlen(AP_Ssid));
    Serial.print(" access point...");
    ret = Spider_Connect(3, (char*)AP_Ssid, (char*)AP_Pass);
    if(ret != 0){
        Serial.println("fail.");
        while(1) ;
    }
    Serial.println("ok");

    /* wait connection and Get DHCP address finished */
    Serial.print("Waiting DHCP...");
    while((Spider_CheckConnected() != 0) || (Spider_CheckDHCP() != 0)) {};
    Serial.println("ok");

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

void setup() {
    Initial_Spider();
    Initial_get_time();
}

int Initial_get_time(void){
    char ntp_server[] = "time.stdtime.gov.tw";
    unsigned int ntp_port = 123; 

    int ret = -1;
    long ntp_timeout = 0;

    union {
        unsigned long ulip;
        unsigned char ucip[4];
    }tar_ip;

    union{
        unsigned short  usport;
        unsigned char   ucport[2];
    }tar_port;

    memset(&host_addr, 0, sizeof(sockaddr));

    // Get ntp server's IP from host name.
    ret = gethostbyname((char*)ntp_server, strlen(ntp_server), &(tar_ip.ulip));
    if(ret < 0){
        return -1;
    }

    // Get socket from spider L3.
    ntp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(ntp_socket < 0){
        ntp_socket = 0xFFFFFFFF;
        return -2;
    }

    // Setting BSD socket timeout.
    ntp_timeout = 1000;
    ret = setsockopt(ntp_socket, SOL_SOCKET, SOCKOPT_RECV_TIMEOUT, &ntp_timeout, sizeof(ntp_timeout));
    if(ret < 0){
        closesocket(ntp_socket);
        ntp_socket = 0xFFFFFFFF;
        return -3;
    }

    // Setting BSD like socket setting.
    host_addr.sa_family = AF_INET;
    tar_port.usport = ntp_port;

    host_addr.sa_data[0] = tar_port.ucport[1];
    host_addr.sa_data[1] = tar_port.ucport[0];

    host_addr.sa_data[2] = tar_ip.ucip[3];
    host_addr.sa_data[3] = tar_ip.ucip[2];
    host_addr.sa_data[4] = tar_ip.ucip[1];
    host_addr.sa_data[5] = tar_ip.ucip[0];

    return 0;
}



int get_time(unsigned int *year, unsigned char *month, unsigned int *day, unsigned char *hour, unsigned char *minute, unsigned char *second){

    // Reference table and parameter.
    unsigned char m_d_table_normal[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    unsigned char m_d_table_leap[]   = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    int           ret = -1;
    unsigned char msg[48];
    unsigned long timestamp_now = 0;
    long          gmt_offset = 0;
    unsigned long counter = 0;
    
    unsigned char *m_d_ptr = 0;

    Serial.print("Starting get time...");
    memset(msg, 0, sizeof(msg));

    // NTP time stamp request header, version 3, mode : 3 as client
    // Reference => http://tools.ietf.org/html/rfc2030
    msg[0] = 0b00100011;
    sendto(ntp_socket, msg, sizeof(msg), 0, &host_addr, sizeof(sockaddr));

    memset(&recv_addr, 0, sizeof(sockaddr));
    recv_addr_l = 0;
    recvfrom(ntp_socket, msg, sizeof(msg), 0, &recv_addr, &recv_addr_l);

    gmt_offset = (MY_GMT) * 60 * 60;
    // Caculate timestamp rightnow
    timestamp_now = ((unsigned long)msg[40] << 24) | ((unsigned long)msg[41] << 16) | ((unsigned long)msg[42] << 8) | ((unsigned long)msg[43]);
    // Dec 70 years in second.
    timestamp_now -= 2208988800UL;
    timestamp_now += gmt_offset;

    Serial.print("time stamp:");
    Serial.println(timestamp_now, DEC);

    /* counting reference : http://maumaubug.blogspot.tw/2013/05/sntp-ntp-02-epoch-time.html */

    *second = timestamp_now % 60;

    *minute = (timestamp_now / 60) % 60;

    *hour =   (timestamp_now / 3600) % 24;

    // Counting total days from 1970/01/01
    counter =  (timestamp_now / 86400);

    // Add 1 days.
    counter += 1; 

    // Count years.
    *year = 1970;
    while(counter > 365){
        if(((*year % 4 == 0) && (*year % 100 != 0)) || (*year % 400 == 0)){
            counter-= 366;
        }
        else{
            counter -=365;
        }
        *year = *year + 1;
    }

    // Count month and days.
    if(((*year % 4 == 0) && (*year % 100 != 0)) || (*year % 400 == 0)){
        m_d_ptr = m_d_table_leap;
    }
    else{
        m_d_ptr = m_d_table_normal;
    }

    *month = 0;
    for(ret = 0; ret < 12; ret++){
        *month = *month + 1;
        if(counter > m_d_ptr[ret]){
            counter -= m_d_ptr[ret];
        }
        else{
            *day = counter;
            break;
        }
    }

    return ret;
}


void Find_Me(void){
    /* Timer register */
    static unsigned long tmr = 0;

    /* Simple mdnsAdvisor function */
    char mdns_msg[10];
    memset(mdns_msg, 0, sizeof(mdns_msg));

    if(millis() > tmr){
        strncpy(mdns_msg, "NTP", sizeof(mdns_msg));
        mdnsAdvertiser(1, (char*)mdns_msg, strlen(mdns_msg));
        tmr = millis() + 1000;
    }
}





void loop() {
    Find_Me();
    static unsigned long update_tmr = 0;
    if(millis() > update_tmr){
        unsigned int  n_year = 0;
        unsigned char n_month = 0;
        unsigned int  n_day = 0;
        unsigned char n_hour = 0;
        unsigned char n_minute = 0;
        unsigned char n_second = 0;

        get_time(&n_year, &n_month, &n_day, &n_hour, &n_minute, &n_second);

        Serial.print("Time :");
        Serial.print(n_year, DEC);
        Serial.print("-");
        Serial.print(n_month, DEC);
        Serial.print("-");
        Serial.print(n_day, DEC);
        Serial.print(" ");
        Serial.print(n_hour, DEC);
        Serial.print(":");
        Serial.print(n_minute, DEC);
        Serial.print(":");
        Serial.print(n_second, DEC);
        Serial.println();

        update_tmr = millis() + UPDATE_TIME_PEROID;
    }
}