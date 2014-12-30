/*****************************************************************************
*
*  WebServer.h - Web server function for Spider_L3
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
#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__
/*****************************************************************************
*
*  WebServer.h.
*  
*****************************************************************************/
#include "Spider_L3.h"

#define WEBSERVER_FAIL                  -1

#define WEBSERVER_METHOD_GET            1
#define WEBSERVER_METHOD_POST           2

// how long break receiving loop
#define WEBSERVER_REQUEST_TIMEOUT       100

#ifdef  __cplusplus
extern "C" {
#endif

extern int WebServer_begin(unsigned short port);

extern int WebServer_close(int server_socket);

extern int WebServer_wait_connect(int socket);

extern int WebServer_request_available(int socket);

extern int WebServer_get_request(int socket, char* req_buf, unsigned long buf_len);

extern int WebServer_process_request(int socket, char* method, unsigned char method_size, char* filename, unsigned char filename_size, 
	                                 char* content, unsigned char content_size, char* auth, unsigned char auth_size);

extern int WebServer_put_response(int socket, char* resp_buf, unsigned long buf_len);

extern int WebServer_put_badrequest(int socket);

extern int WebServer_put_notfound(int socket);

extern int WebServer_close_connect(int socket);


#ifdef  __cplusplus
}
#endif

#endif //__WEBSERVER_H__