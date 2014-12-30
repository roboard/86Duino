/*****************************************************************************
*
*  WebServer.c - Web server function for Spider_L3
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
#include "WebServer.h"

int WebServer_begin(unsigned short port){
    int httpd = 0;
    sockaddr_in name;

    httpd = socket(AF_INET, SOCK_STREAM, 0);
    if(httpd < 0){
        return -1;
    }

    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(0);

    if (bind(httpd, (sockaddr *)&name, sizeof(name)) < 0){
        closesocket(httpd);
        return -2;
    }

    if (listen(httpd, 0) < 0){
        closesocket(httpd);
        return -3;
    }

    return(httpd);
}

int WebServer_close(int server_socket){
    return closesocket(server_socket);
}



int WebServer_wait_connect(int socket){
    long Connect_stat = -1;
    int client_sock = -1;
    sockaddr_in client_name;
    socklen_t client_name_len = sizeof(client_name);

    client_sock = accept(socket, (sockaddr *)&client_name, &client_name_len);
    if(client_sock < 0){
        return -1;
    }

    long timeout = 10;
    Connect_stat = setsockopt(client_sock, SOL_SOCKET, SOCKOPT_RECV_TIMEOUT, &timeout, sizeof(timeout));
    if(Connect_stat < 0){
        closesocket(client_sock);
        client_sock = -1;
        return -1;
    }

    return client_sock;
}

int WebServer_request_available(int socket){
    timeval timeout;
    fd_set fd_read;
    int retval = 0;

    FD_ZERO(&fd_read);
    FD_SET(socket, &fd_read);

    timeout.tv_sec = 0;
    timeout.tv_usec = 5000; // 5 millisec

    retval = select((socket + 1), &fd_read, NULL, NULL, &timeout);

    if(retval < 0){
        return -1;
    }
    else if(retval == 0){
        return -2;
    }
    else{
        if(FD_ISSET(socket, &fd_read)){
            return 0;
        }
    }

    return -1;
}

int WebServer_get_request(int socket, char* req_buf, unsigned long buf_len){
    int             Connect_stat = -1;

    // Receiving http get result from host server.
    Connect_stat = recv(socket, req_buf, buf_len, 0);
    if(Connect_stat < 0){
        return -1;
    }

    return Connect_stat;
}

int WebServer_process_request(int socket, char* method, unsigned char method_size, char* filename, unsigned char filename_size, 
                              char* content, unsigned char content_size, char* auth, unsigned char auth_size){

    #define STATE_GET_METHOD    0
    #define STATE_GET_FILENAME  1
    #define STATE_GET_CONTENT   2

    unsigned long tmr = 0;
    char data = 0;
    char content_head[4];
    char auth_head[15];
    unsigned char state = 0;

    char data_pool[100];
    int ret = 0;
    int i;

    unsigned char method_ptr = 0;
    unsigned char filename_ptr = 0;
    unsigned char content_ptr = 0;
    unsigned char auth_ptr = 0;

    memset(content_head, 0, sizeof(content_head));
    memset(auth_head, 0, sizeof(auth_head));


    tmr = millis() + WEBSERVER_REQUEST_TIMEOUT;

    while(millis() < tmr){
        //if(WebServer_request_available(socket) == 0){
            // Get char
            //WebServer_get_request(socket, &data, 1);
            ret = WebServer_get_request(socket, data_pool, sizeof(data_pool));
            if(ret > 0){
                tmr = millis() + WEBSERVER_REQUEST_TIMEOUT;
                for(i = 0; i < ret; i++){
                    data = data_pool[i];

                    // Find Authorization.
                    if(strncmp(auth_head, "Authorization:", 14) == 0){
                        if(auth_ptr < auth_size){

                            //Check \r\n or \n\r
                            if( ((data == '\r') && (auth[(auth_ptr - 1)] == '\n')) ||
                                ((data == '\n') && (auth[(auth_ptr - 1)] == '\r')) ){
                                auth[(auth_ptr - 1)] = 0;
                                memset(auth_head, 0, sizeof(auth_head));
                            }
                            else{
                                auth[auth_ptr] = data;
                                auth_ptr++;
                            }
                        }
                        else{
                            // Set auth_head to 0 to exit if condition
                            memset(auth_head, 0, sizeof(auth_head));
                        }
                    }
                    else{
                        memcpy(auth_head, &auth_head[1], (sizeof(auth_head) - 1));
                        auth_head[14] = data;
                    }

                    // Check state
                    switch(state){

                        case STATE_GET_METHOD:
                            if(data != ' '){
                                if(method_ptr < method_size){
                                    method[method_ptr] = data;
                                    method_ptr++;
                                }
                            }
                            else{
                                // Get blank
                                state = STATE_GET_FILENAME;
                            }
                        break;

                        case STATE_GET_FILENAME:
                            if(data != ' '){
                                if(filename_ptr < filename_size){
                                    filename[filename_ptr] = data;
                                    filename_ptr++;
                                }
                            }
                            else{
                                // Get blank, change state
                                state = STATE_GET_CONTENT;
                            }
                        break;

                        case STATE_GET_CONTENT:
                            if((content_head[0] == '\r') && (content_head[1] == '\n') && (content_head[2] == '\r') && (content_head[3] == '\n')){
                                if(content_ptr < content_size){
                                    content[content_ptr] = data;
                                    content_ptr++;
                                }
                            }else{
                                memcpy(content_head, &content_head[1], (sizeof(content_head) - 1));
                                content_head[3] = data;
                            }
                        break;

                        default:
                        break;
                    }

                }
            }
        //}
    }
    return 0;
}

int WebServer_put_response(int socket, char* resp_buf, unsigned long buf_len){

    int             Connect_stat = -1;

    Connect_stat = send(socket, resp_buf, buf_len, 0);
    if(Connect_stat < 0){
        return -1;
    }
    return Connect_stat;
}

int WebServer_put_badrequest(int socket){
    char req_pool[50];

    memset(req_pool, 0, sizeof(req_pool));
    strncpy(req_pool, "HTTP/1.0 400 Bad Request\r\n", sizeof(req_pool));
    WebServer_put_response(socket, req_pool, strlen(req_pool));

    memset(req_pool, 0, sizeof(req_pool));
    strncpy(req_pool, "Content-Type: text/html\r\n", sizeof(req_pool));
    WebServer_put_response(socket, req_pool, strlen(req_pool));

    memset(req_pool, 0, sizeof(req_pool));
    strncpy(req_pool, "Connection: close\r\n\r\n", sizeof(req_pool));
    WebServer_put_response(socket, req_pool, strlen(req_pool));

    memset(req_pool, 0, sizeof(req_pool));
    strncpy(req_pool, "<P>BAD REQUEST\r\n", sizeof(req_pool));
    WebServer_put_response(socket, req_pool, strlen(req_pool));

    return 0;
}

int WebServer_put_notfound(int socket){
    char req_pool[50];

    memset(req_pool, 0, sizeof(req_pool));
    strncpy(req_pool, "HTTP/1.0 404 Not Found\r\n", sizeof(req_pool));
    WebServer_put_response(socket, req_pool, strlen(req_pool));

    memset(req_pool, 0, sizeof(req_pool));
    strncpy(req_pool, "Content-Type: text/html\r\n", sizeof(req_pool));
    WebServer_put_response(socket, req_pool, strlen(req_pool));

    memset(req_pool, 0, sizeof(req_pool));
    strncpy(req_pool, "Connection: close\r\n\r\n", sizeof(req_pool));
    WebServer_put_response(socket, req_pool, strlen(req_pool));

    return 0;
}

int WebServer_close_connect(int socket){
    return closesocket(socket);
}