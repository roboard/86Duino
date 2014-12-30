/*
  ENServer.cpp - Part of DM&P Vortex86 Ethernet library
  Copyright (c) 2013 DY Hung <Dyhung@dmp.com.tw>. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)
*/

extern "C" {
#include "string.h"
}

#include "SwsSock.h"
#include "EthernetClient.h"
#include "EthernetServer.h"

EthernetServer::EthernetServer(uint16_t port)
{
	int i;
	
	_port = port;
	sws = (struct SwsSockInfo*)malloc(sizeof(struct SwsSockInfo));
	if (sws) {
		memset(sws, 0, sizeof(struct SwsSockInfo));
		sws->_sock = SWS_INVALID_SOCKET;
	}
	
	for (i = 0; i < MAX_SOCK_NUM; i++) {
		if (Clients[i].sws == NULL) continue;
		Clients[i].sws->_sock = SWS_INVALID_SOCKET;
		Clients[i]._id = -1;
		Clients[i].pServer = this;
	}
}

EthernetServer::~EthernetServer()
{
	for (int i = 0; i < MAX_SOCK_NUM; i++) {
		if (Clients[i].sws == NULL) continue;
		free(Clients[i].sws);
		Clients[i].sws = NULL;
	}
	if (sws) {
		free(sws);
		sws = NULL;
	}
}

void EthernetServer::begin()
{
	int idx;
	struct SWS_sockaddr_in sin;
	SWS_u_long lArg = 1;
	int yes = 1, no = 0;
	
	if (sws == NULL) return;
	sws->_sock = SWS_socket(SWS_AF_INET, SWS_SOCK_STREAM, 0);
    if (sws->_sock == SWS_INVALID_SOCKET) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		return;
    }
	
	bzero(&sin, sizeof(sin));
    sin.sin_family = SWS_AF_INET;
    sin.sin_addr.SWS_s_addr = SwsSock.getULLocalIp();
    sin.sin_port = SWS_htons(_port);
	
	if (SWS_bind(sws->_sock, (struct SWS_sockaddr *)&sin, sizeof(sin)) != 0) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		sws->_sock = SWS_INVALID_SOCKET;
		return;
    }

    if (SWS_listen(sws->_sock, MAX_SOCK_NUM) != 0) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		sws->_sock = SWS_INVALID_SOCKET;
		return;
    }
	
	if (SWS_ioctl(sws->_sock, SWS_FIONBIO, &lArg) != 0) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		sws->_sock = SWS_INVALID_SOCKET;
		return;
	}
	
	if (SWS_setsockopt(sws->_sock, SWS_SOL_SOCKET, SWS_SO_REUSEADDR, (const char*)&yes, sizeof(int)) < 0) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		sws->_sock = SWS_INVALID_SOCKET;
		return;
	}
	
	if (SWS_setsockopt(sws->_sock, SWS_SOL_SOCKET, SWS_SO_DONTLINGER, (const char*)&no, sizeof(int)) < 0) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		sws->_sock = SWS_INVALID_SOCKET;
		return;
	}
}

struct SwsSockInfo *EthernetServer::make_new_client(struct SwsSockInfo *info)
{
	SWS_SOCKET tempsock;
	int addrsize, idx;
	int yes = 1;
	SWS_u_long lArg = 1;
	struct SWS_sockaddr_in pin;
	
	for (idx = 0; idx < MAX_SOCK_NUM; idx++) {
		if (Clients[idx].sws == NULL) continue;
		if (Clients[idx].sws->_sock == SWS_INVALID_SOCKET)
			break;
	}
	
	addrsize = sizeof(pin);
	if ((tempsock = SWS_accept(sws->_sock, (struct SWS_sockaddr *)&pin, &addrsize)) < 0)
		return NULL;
	
	if (SWS_ioctl(tempsock, SWS_FIONBIO, &lArg) != 0) {
		SWS_shutdown(tempsock, SWS_SD_BOTH);
		SWS_close(tempsock);
		return NULL;
	}
	
	if (SWS_setsockopt(tempsock, SWS_SOL_SOCKET, SWS_SO_REUSEADDR, (const char*)&yes, sizeof(int)) < 0) {
		SWS_shutdown(tempsock, SWS_SD_BOTH);
		SWS_close(tempsock);
		return NULL;
	}
	
	if (SWS_setsockopt(tempsock, SWS_SOL_SOCKET, SWS_SO_DONTLINGER, (const char*)&yes, sizeof(int)) < 0) {
		SWS_shutdown(tempsock, SWS_SD_BOTH);
		SWS_close(tempsock);
		return NULL;
	}
	
	if (idx >= MAX_SOCK_NUM) {
		SWS_shutdown(tempsock, SWS_SD_BOTH);
		SWS_close(tempsock);
		return NULL;
	}
	
	Clients[idx].sws->_sock = tempsock;
	Clients[idx]._id = idx;
	
	return Clients[idx].sws;
}

EthernetClient EthernetServer::available()
{
	struct SwsSockInfo *temp;
	SWS_SOCKET tempsock;
	SWS_SOCKET socks[MAX_SOCK_NUM+1];
	int i, j;
	struct SWS_timeval seltime;
	
	if (sws == NULL) return EthernetClient(NULL);
	if (sws->_sock != SWS_INVALID_SOCKET)
	{
		socks[0] = sws->_sock;
		SWS_FdZero(&sws->rfds);
		SWS_FdSet(sws->_sock, &sws->rfds);
		for (i = 0; i < MAX_SOCK_NUM; i++) {
			if (Clients[i].sws == NULL) continue;
			socks[i+1] = Clients[i].sws->_sock;
			
			if (Clients[i].sws->_sock != SWS_INVALID_SOCKET){
				SWS_FdSet(Clients[i].sws->_sock, &sws->rfds);
				}
		}

		seltime.tv_sec = 0;
		seltime.tv_usec = 0;
		if (!SWS_select(&sws->rfds, NULL, NULL, &seltime))
			return EthernetClient(NULL);
		
		for (i = 0; i < MAX_SOCK_NUM+1; i++) {
			
			if (SWS_FdIsSet(socks[i], &sws->rfds)) {
				
				if (socks[i] == sws->_sock) {
					temp =  make_new_client(sws);
					tempsock = (temp == NULL) ? (SWS_INVALID_SOCKET) : (temp->_sock);
				} else
					tempsock = socks[i];
				
				if (tempsock != SWS_INVALID_SOCKET && socks[i] != sws->_sock) {
					for (j = 0; j < MAX_SOCK_NUM; j++) {
						if (Clients[j].sws == NULL) continue;
						if (Clients[j].sws->_sock == tempsock) {
							uint8_t val;
							
							if (SWS_recv(Clients[j].sws->_sock, &val, 1, SWS_MSG_PEEK) <= 0)
								Clients[j].stop();
							
							return Clients[j];
						}
					}
				}
			}
		}
	}
	
	return EthernetClient(NULL);
}

size_t EthernetServer::write(uint8_t b) 
{
	return write(&b, 1);
}

size_t EthernetServer::write(const uint8_t *buffer, size_t size) 
{
	int i;
	size_t n = 0;
	
	for (i = 0; i < MAX_SOCK_NUM; i++) {
		if (Clients[i].status())
			n += Clients[i].write(buffer, size);
	}
	
	return n;
}

void EthernetServer::closeServerSocket(int idx)
{
	if (idx >= 0 && idx < MAX_SOCK_NUM) {
		if (Clients[idx].sws != NULL) {
			Clients[idx].sws->_sock = SWS_INVALID_SOCKET;
		}
	}
}
	