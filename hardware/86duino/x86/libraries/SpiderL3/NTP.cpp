/*****************************************************************************
*
*  NTP.c - Network time protocol library for Spider L3
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
#include <Arduino.h>
#include "NTP.h"

static long ntp_socket = 0xFFFFFFFF;
static sockaddr    host_addr;
static sockaddr    recv_addr;
static socklen_t   recv_addr_l;
static int GMT = 0;

int NTP_begin(int Loc_GMT, char* ServerAddr){
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

    GMT = Loc_GMT;
    // Get ntp server's IP from host name.
    ret = gethostbyname((char*)ServerAddr, strlen(ServerAddr), &(tar_ip.ulip));
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

int NTP_get_time(unsigned int *year, unsigned char *month, unsigned int *day, 
                 unsigned char *hour, unsigned char *minute, unsigned char *second){

    // Reference table and parameter.
    unsigned char m_d_table_normal[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    unsigned char m_d_table_leap[]   = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    int           ret = -1;
    unsigned char msg[48];
    unsigned long timestamp_now = 0;
    long          gmt_offset = 0;
    unsigned long counter = 0;
    
    unsigned char *m_d_ptr = 0;

    memset(msg, 0, sizeof(msg));

    // NTP time stamp request header, version 3, mode : 3 as client
    // Reference => http://tools.ietf.org/html/rfc2030
    msg[0] = 0b00100011;
    sendto(ntp_socket, msg, sizeof(msg), 0, &host_addr, sizeof(sockaddr));

    memset(&recv_addr, 0, sizeof(sockaddr));
    recv_addr_l = 0;
    recvfrom(ntp_socket, msg, sizeof(msg), 0, &recv_addr, &recv_addr_l);

    gmt_offset = (GMT) * 60 * 60;
    // Caculate timestamp rightnow
    timestamp_now = ((unsigned long)msg[40] << 24) | ((unsigned long)msg[41] << 16) | ((unsigned long)msg[42] << 8) | ((unsigned long)msg[43]);
    // Dec 70 years in second.
    timestamp_now -= 2208988800UL;
    timestamp_now += gmt_offset;

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