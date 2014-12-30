/*****************************************************************************
*
*  WebClient.h - Web client function for Spider_L3
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
#ifndef __WEBCLIENT_H__
#define __WEBCLIENT_H__
/*****************************************************************************
*
*  WebClient.h.
*  
*****************************************************************************/
#include "Spider_L3.h"

//
// Web server port
//
#define DEFAULT_WEB_PORT    80

//
// Web server reciving timeout
//
#define WEB_CLIENT_RECV_TO    2000

//
// Web client debug print
//
#define WEB_CLIENT_DBG_EN    0

#define WEB_CLI_OK                  0
#define WEB_CLI_ERROR               -1
#define WEB_CLI_HOST_STR_FAIL       -2
#define WEB_CLI_HOST_IP_FAIL        -3
#define WEB_CLI_GET_SOCKET_FAIL     -4
#define WEB_CLI_SET_SOCKET_TO_FAIL  -5
#define WEB_CLI_SERVER_CONN_FAIL    -6
#define WEB_CLI_SERVER_SEND_FAIL    -7
#define WEB_CLI_SERVER_RECV_FAIL    -8


typedef struct{
    long        host_socket;
    sockaddr    host_addr;
}Host_Info;

#ifdef  __cplusplus
extern "C" {
#endif
extern int WebClient_Get_HostIP(char* url_link, unsigned char* ret_IP0, unsigned char* ret_IP1, unsigned char* ret_IP2, unsigned char* ret_IP3);

extern int WebClient_Begin_URL(Host_Info *info, char* url_link);

extern int WebClient_Begin_IP(Host_Info *info, unsigned char ip1, unsigned char ip2, unsigned char ip3, unsigned char ip4, unsigned short port);

extern int WebClient_Connect(Host_Info *info);
extern int WebClient_SendRequest(Host_Info *info, char* url_link);
extern int WebClient_DataAvailable(Host_Info *info);
extern int WebClient_RecvData(Host_Info *info, char* ret_buf, int buf_size);
extern int WebClient_Get(Host_Info *info, char* url_link, char* ret_buf, int buf_size);
extern int WebClient_Close(Host_Info *info);

#ifdef  __cplusplus
}
#endif

#endif //__WEBCLIENT_H__