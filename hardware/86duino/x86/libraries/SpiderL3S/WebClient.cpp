/*****************************************************************************
*
*  WebClient.c - Web client function for Spider_L3
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
#include "WebClient.h"

static const char WebGet_MsgFnt[] = {"GET "};
static const char WebGet_MsgMid[] = {" HTTP/1.1\r\nHost: "};
static const char WebGet_MsgEnd[] = {"\r\nConnection: close\r\n\r\n"};
//static const char WebGet_MsgEnd[] = {"\r\n\r\n"};


int WebClient_Begin_URL(Host_Info *info, char* url_link){
    union {
        unsigned long ulip;
        unsigned char ucip[4];
    }tar_ip;

    union{
        unsigned short  usport;
        unsigned char   ucport[2];
    }tar_port;

    char*           sp1 = 0;
    char*           sp2 = 0;
    int             Connect_stat = -1;
    long            timeout = 0;

    info->host_socket = 0xFFFFFFFF;
    memset(&info->host_addr, 0, sizeof(sockaddr));

    sp1 = strstr(url_link, "http://");
    if(sp1 == 0){
        return WEB_CLI_HOST_STR_FAIL;
    }
    sp1 += strlen("http://");
    sp2 = strstr(sp1, "/");
    if(sp2 == 0){
        return WEB_CLI_HOST_STR_FAIL;
    }

    // Get host IP from host name.
    Connect_stat = gethostbyname(sp1, (sp2-sp1), &(tar_ip.ulip));
    if(Connect_stat < 0){
        return WEB_CLI_HOST_IP_FAIL;
    }

    // Get socket from spider L3.
    info->host_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(info->host_socket < 0){
        info->host_socket = 0xFFFFFFFF;
        return WEB_CLI_GET_SOCKET_FAIL;
    }

    // Setting BSD socket timeout.
    timeout = WEB_CLIENT_RECV_TO;
    Connect_stat = setsockopt(info->host_socket, SOL_SOCKET, SOCKOPT_RECV_TIMEOUT, &timeout, sizeof(timeout));
    if(Connect_stat < 0){
        closesocket(info->host_socket);
        info->host_socket = 0xFFFFFFFF;
        return WEB_CLI_SET_SOCKET_TO_FAIL;
    }

    // Setting BSD like socket setting.
    info->host_addr.sa_family = AF_INET;
    tar_port.usport = DEFAULT_WEB_PORT;

    info->host_addr.sa_data[0] = tar_port.ucport[1];
    info->host_addr.sa_data[1] = tar_port.ucport[0];

    info->host_addr.sa_data[2] = tar_ip.ucip[3];
    info->host_addr.sa_data[3] = tar_ip.ucip[2];
    info->host_addr.sa_data[4] = tar_ip.ucip[1];
    info->host_addr.sa_data[5] = tar_ip.ucip[0];

    return WEB_CLI_OK;
}

int WebClient_Get_HostIP(char* url_link, unsigned char* ret_IP0, unsigned char* ret_IP1, unsigned char* ret_IP2, unsigned char* ret_IP3){
    union {
        unsigned long ulip;
        unsigned char ucip[4];
    }tar_ip;

    char*           sp1 = 0;
    char*           sp2 = 0;
    int             Connect_stat = -1;

    sp1 = strstr(url_link, "http://");
    if(sp1 == 0){
        return WEB_CLI_HOST_STR_FAIL;
    }
    sp1 += strlen("http://");
    sp2 = strstr(sp1, "/");
    if(sp2 == 0){
        return WEB_CLI_HOST_STR_FAIL;
    }

    // Get host IP from host name.
    Connect_stat = gethostbyname(sp1, (sp2-sp1), &(tar_ip.ulip));
    if(Connect_stat < 0){
        return WEB_CLI_HOST_IP_FAIL;
    }

    *ret_IP0 = tar_ip.ucip[3];
    *ret_IP1 = tar_ip.ucip[2];
    *ret_IP2 = tar_ip.ucip[1];
    *ret_IP3 = tar_ip.ucip[0];

    return WEB_CLI_OK;
}

int WebClient_Begin_IP(Host_Info *info, unsigned char ip1, unsigned char ip2, unsigned char ip3, unsigned char ip4, unsigned short port){
    union{
        unsigned short  usport;
        unsigned char   ucport[2];
    }tar_port;

    int             Connect_stat = -1;
    long            timeout = 0;
    tar_port.usport = 0;
    info->host_socket = 0xFFFFFFFF;
    memset(&info->host_addr, 0, sizeof(sockaddr));

    // Get socket from spider L3.
    info->host_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(info->host_socket < 0){
        info->host_socket = 0xFFFFFFFF;
        return WEB_CLI_GET_SOCKET_FAIL;
    }


    // Setting BSD socket timeout.
    timeout = WEB_CLIENT_RECV_TO;
    Connect_stat = setsockopt(info->host_socket, SOL_SOCKET, SOCKOPT_RECV_TIMEOUT, &timeout, sizeof(timeout));
    if(Connect_stat < 0){
        closesocket(info->host_socket);
        info->host_socket = 0xFFFFFFFF;
        return WEB_CLI_SET_SOCKET_TO_FAIL;
    }

    // Setting BSD like socket setting.
    info->host_addr.sa_family = AF_INET;
    tar_port.usport = port;

    info->host_addr.sa_data[0] = tar_port.ucport[1];
    info->host_addr.sa_data[1] = tar_port.ucport[0];

    info->host_addr.sa_data[2] = ip1;
    info->host_addr.sa_data[3] = ip2;
    info->host_addr.sa_data[4] = ip3;
    info->host_addr.sa_data[5] = ip4;

    return WEB_CLI_OK;
}

int WebClient_Connect(Host_Info *info){
    int             Connect_stat = -1;
    // Connect to host server.
    Connect_stat = connect(info->host_socket, &info->host_addr, sizeof(sockaddr));
    if(Connect_stat < 0){
        return WEB_CLI_SERVER_CONN_FAIL;
    }
    return Connect_stat;
}


int WebClient_SendRequest(Host_Info *info, char* url_link){
    char*           sp1 = 0;
    char*           sp2 = 0;
    int             Connect_stat = -1;

    sp1 = strstr(url_link, "http://");
    if(sp1 == 0){
        return WEB_CLI_HOST_STR_FAIL;
    }
    sp1 += strlen("http://");
    sp2 = strstr(sp1, "/");
    if(sp2 == 0){
        return WEB_CLI_HOST_STR_FAIL;
    }

    // Send http get.
    Connect_stat = send(info->host_socket, WebGet_MsgFnt, strlen(WebGet_MsgFnt), 0);
    if(Connect_stat < 0){
        return WEB_CLI_SERVER_SEND_FAIL;
    }

    Connect_stat = send(info->host_socket, sp2, strlen(sp2), 0);
    if(Connect_stat < 0){
        return WEB_CLI_SERVER_SEND_FAIL;
    }

    Connect_stat = send(info->host_socket, WebGet_MsgMid, strlen(WebGet_MsgMid), 0);
    if(Connect_stat < 0){
        return WEB_CLI_SERVER_SEND_FAIL;
    }

    Connect_stat = send(info->host_socket, sp1, (sp2-sp1), 0);
    if(Connect_stat < 0){
        return WEB_CLI_SERVER_SEND_FAIL;
    }

    Connect_stat = send(info->host_socket, WebGet_MsgEnd, strlen(WebGet_MsgEnd), 0);
    if(Connect_stat < 0){
        return WEB_CLI_SERVER_SEND_FAIL;
    }
    return 0;
}

int WebClient_DataAvailable(Host_Info *info){
    timeval timeout;
    fd_set fd_read;

    memset(&fd_read, 0, sizeof(fd_read));
    FD_SET(info->host_socket, &fd_read);

    timeout.tv_sec = 0;
    timeout.tv_usec = 5000; // 5 millisec

    return select((info->host_socket+1), &fd_read, NULL, NULL, &timeout);
}

int WebClient_RecvData(Host_Info *info, char* ret_buf, int buf_size){
    int             Connect_stat = -1;
    // Receiving http get result from host server.
    Connect_stat = recv(info->host_socket, ret_buf, buf_size, 0);
    if(Connect_stat < 0){
        return WEB_CLI_SERVER_RECV_FAIL;
    }

    return Connect_stat;
}

int WebClient_Get(Host_Info *info, char* url_link, char* ret_buf, int buf_size){
    char*           sp1 = 0;
    char*           sp2 = 0;

    int             Connect_stat = -1;

    // Copy host name from url.
    sp1 = strstr(url_link, "http://");
    if(sp1 == 0){
        return WEB_CLI_HOST_STR_FAIL;
    }
    sp1 += strlen("http://");
    sp2 = strstr(sp1, "/");
    if(sp2 == 0){
        return WEB_CLI_HOST_STR_FAIL;
    }

    char send_buffer[500];
    int  ptr = 0;

    memset(send_buffer, 0, sizeof(send_buffer));
    ptr = 0;

    strncpy(&send_buffer[ptr], WebGet_MsgFnt, (sizeof(send_buffer) - ptr));
    ptr += strlen(WebGet_MsgFnt);

    strncpy(&send_buffer[ptr], sp2, (sizeof(send_buffer) - ptr));
    ptr += strlen(sp2);

    strncpy(&send_buffer[ptr], WebGet_MsgMid, (sizeof(send_buffer) - ptr));
    ptr += strlen(WebGet_MsgMid);

    strncpy(&send_buffer[ptr], sp1, (sizeof(send_buffer) - ptr));
    ptr += (sp2-sp1);

    strncpy(&send_buffer[ptr], WebGet_MsgEnd, (sizeof(send_buffer) - ptr));
    ptr += strlen(WebGet_MsgEnd);
    
    // Send end of http get.
    Connect_stat = send(info->host_socket, send_buffer, ptr, 0);
    if(Connect_stat < 0){
        return WEB_CLI_SERVER_SEND_FAIL;
    }

    // Receiving http get result from host server.
    Connect_stat = recv(info->host_socket, ret_buf, buf_size, 0);
    if(Connect_stat < 0){
        return WEB_CLI_SERVER_RECV_FAIL;
    }

    return Connect_stat;
}

int WebClient_Close(Host_Info *info){
    long        Connect_stat = -1;

    Connect_stat = closesocket(info->host_socket);
    if(Connect_stat != 0){
        return WEB_CLI_ERROR;
    }
    return WEB_CLI_OK;
}
