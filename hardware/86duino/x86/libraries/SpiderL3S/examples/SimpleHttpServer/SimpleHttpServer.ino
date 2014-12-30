/*****************************************************************************
*
*  SimpleHttpServer.ino - Simple HTTP web server, user can change 
*  GPIO status from web page.
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
#include "WebServer.h"

// Configure your WiFi module pin connection.
unsigned char WLAN_CS = 43;
unsigned char WLAN_EN = 44;
unsigned char WLAN_IRQ = 42;
unsigned char WLAN_IRQ_INTNUM = 0;

// Don't forget set correct WiFi SSID and Password.
char AP_Ssid[] = {"WIFISSID"};
char AP_Pass[] = {"12345678"};

const int INDICATE_LED = 13;

const int CONTROL_GPIO = 8;

unsigned char CONTROL_STATE = LOW;


void setup() {          

    int ret = 0;

    /* initial uart debug output interface. */
    Serial.begin(115200);

    while (!Serial) {
      ; // wait for serial port to connect. Needed for Leonardo only
    }

    Serial.println("=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+");
    Serial.println("     Spider L3 simple http server.    ");
    Serial.println("=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+");

    /* initial status LED pin */
    pinMode(INDICATE_LED, OUTPUT);
    digitalWrite(INDICATE_LED, LOW);

    CONTROL_STATE = LOW;
    digitalWrite(INDICATE_LED, CONTROL_STATE);

    /* Initial monitor IO pin */
    pinMode(8, INPUT);

    /* Initial Spider L3 */
    Serial.print("Starting Spider L3...");
    ret = Spider_begin();
    if(ret != 0){
        Serial.println("fail, please check connection pin.");
        while(1) ;
    }
    Serial.println("ok");

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
    while((Spider_CheckConnected() != 0) || (Spider_CheckDHCP() != 0)) ;
    Serial.println("ok");

    tNetappIpconfigRetArgs inf;
    netapp_ipconfig(&inf);

    Serial.print("Device IP address:");
    Serial.print(inf.aucIP[3] ,DEC);
    Serial.print(".");
    Serial.print(inf.aucIP[2] ,DEC);
    Serial.print(".");
    Serial.print(inf.aucIP[1] ,DEC);
    Serial.print(".");
    Serial.print(inf.aucIP[0] ,DEC);
    Serial.print("\r\n");

}

#define INIT_SERVER             0x01
#define WAIT_CONNECT            0x02
#define HANDLE_REQUEST          0x03
#define HANDLE_GET_RESPONSE     0x04
#define HANDLE_POST_RESPONSE    0x05
#define HANDLE_BAD_RESPONSE     0x06
#define CLOSE_CLIENT_CONN       0x07


void WebService(void){
    static unsigned char app_st = INIT_SERVER;
    static int http_server_socket = -1;
    static int client_socket = -1;
    static unsigned long task_timer = 0;

    char data_pool[100];
    int ret = 0;

    static char Method[5];
    static char file[30];
    static char content[15];
    static char auth_dummy;

    if(millis() > task_timer){
        switch(app_st){
            case INIT_SERVER:
                http_server_socket = WebServer_begin(80);
                if(http_server_socket >= 0){
                    app_st = WAIT_CONNECT;
                }
            break;
            case WAIT_CONNECT:
                client_socket = WebServer_wait_connect(http_server_socket);
                if(client_socket >= 0){
                    Serial.println("Incomming connection.");
                    app_st = HANDLE_REQUEST;
                }
            break;
            case HANDLE_REQUEST:
                Serial.println("Handle incomming request.");

                memset(Method, 0, sizeof(Method));
                memset(file, 0, sizeof(file));
                memset(content, 0, sizeof(content));

                WebServer_process_request(client_socket, Method, sizeof(Method), file, sizeof(file), content, sizeof(content), &auth_dummy, 0);

                Serial.print("Method : ");
                Serial.write((unsigned char*)Method, strlen(Method));
                Serial.println();
                Serial.print("filename : ");
                Serial.write((unsigned char*)file, strlen(file));
                Serial.println();
                Serial.print("CGI parameter : ");
                Serial.write((unsigned char*)content, strlen(content));
                Serial.println();
                app_st = HANDLE_GET_RESPONSE;
                break;

            case HANDLE_GET_RESPONSE:
            case HANDLE_POST_RESPONSE:

                if(strstr(file, ("favicon")) != 0){
                    WebServer_put_notfound(client_socket);
                    task_timer = millis() + 100;
                    app_st = CLOSE_CLIENT_CONN;
                    break;
                }

                if(strstr(content, ("HIGH")) != 0){
                    CONTROL_STATE = HIGH;
                }

                if(strstr(content, ("LOW")) != 0){
                    CONTROL_STATE = LOW;
                }
                digitalWrite(INDICATE_LED, CONTROL_STATE);

                Serial.println("Acknowledge webpage.");

                /* Put HTML header */
                memset(data_pool, 0, sizeof(data_pool));
                strncpy(data_pool, ("HTTP/1.1 200 OK\r\n"), sizeof(data_pool));
                WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                memset(data_pool, 0, sizeof(data_pool));
                strncpy(data_pool, ("Content-Type: text/html\r\n"), sizeof(data_pool));
                WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                memset(data_pool, 0, sizeof(data_pool));
                strncpy(data_pool, ("Connection: close\r\n\r\n"), sizeof(data_pool));
                WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                /* PUT Message */
                memset(data_pool, 0, sizeof(data_pool));
                strncpy(data_pool, ("<html>\r\n<body>\r\n\r\n"), sizeof(data_pool));
                WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                ret = sprintf(data_pool, "Input variable :%s\r\n", ((CONTROL_STATE == HIGH) ? "HIGH" : "LOW"));
                WebServer_put_response(client_socket, data_pool, ret);

                memset(data_pool, 0, sizeof(data_pool));
                strncpy(data_pool, ("<form method=\"post\">"), sizeof(data_pool));
                WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                memset(data_pool, 0, sizeof(data_pool));
                strncpy(data_pool, ("<input type=\"hidden\" name=\"var\" value=\"HIGH\">"), sizeof(data_pool));
                WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                memset(data_pool, 0, sizeof(data_pool));
                strncpy(data_pool, ("<input type=\"submit\" style=\"height:30px; width:100px\" value=\"HIGH\">\r\n</form>\r\n"), sizeof(data_pool));
                WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                memset(data_pool, 0, sizeof(data_pool));
                strncpy(data_pool, ("<form method=\"post\">"), sizeof(data_pool));
                WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                memset(data_pool, 0, sizeof(data_pool));
                strncpy(data_pool, ("<input type=\"hidden\" name=\"var\" value=\"LOW\">"), sizeof(data_pool));
                WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                memset(data_pool, 0, sizeof(data_pool));
                strncpy(data_pool, ("<input type=\"submit\" style=\"height:30px; width:100px\" value=\"LOW\">\r\n</form>\r\n"), sizeof(data_pool));
                WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                memset(data_pool, 0, sizeof(data_pool));
                strncpy(data_pool, ("</html>\r\n</body>\r\n\r\n"), sizeof(data_pool));
                WebServer_put_response(client_socket, data_pool, strlen(data_pool));

                task_timer = millis() + 100;
                app_st = CLOSE_CLIENT_CONN;
            break;

            case CLOSE_CLIENT_CONN:
                Serial.println("Close connection.");
                WebServer_close_connect(client_socket);
                client_socket = -1;
                app_st = WAIT_CONNECT;
            break;

        }
        task_timer = millis() + 100;
    }
}

void loop() {
    WebService();
}