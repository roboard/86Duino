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
#include "ENClient.h"
#include "ENServer.h"

EthernetServer::EthernetServer(uint16_t port)
{
	int i;
	
	_sock = INVALID_SOCKET;
	_port = port;
	
	for (i = 0; i < MAX_SOCK_NUM; i++) {
		Clients[i]._sock = INVALID_SOCKET;
		Clients[i]._id = -1;
		Clients[i].pServer = this;
	}
}

void EthernetServer::begin()
{
	int idx;
	struct sockaddr_in sin;
	u_long lArg = 1;
	int yes = 1, no = 0;
	
	_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (_sock == INVALID_SOCKET) {
		shutdown(_sock, SD_BOTH);
		closesocket(_sock);
		return;
    }
	
	bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = SwsSock.getULLocalIp();
    sin.sin_port = htons(_port);
	
	if (bind(_sock, (struct sockaddr *)&sin, sizeof(sin)) != 0) {
		shutdown(_sock, SD_BOTH);
		closesocket(_sock);
		_sock = INVALID_SOCKET;
		return;
    }

    if (listen(_sock, MAX_SOCK_NUM) != 0) {
		shutdown(_sock, SD_BOTH);
		closesocket(_sock);
		_sock = INVALID_SOCKET;
		return;
    }
	
	if (ioctlsocket(_sock, FIONBIO, &lArg) != 0) {
		shutdown(_sock, SD_BOTH);
		closesocket(_sock);
		_sock = INVALID_SOCKET;
		return;
	}
	
	if (setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(int)) < 0) {
		shutdown(_sock, SD_BOTH);
		closesocket(_sock);
		_sock = INVALID_SOCKET;
		return;
	}
	
	if (setsockopt(_sock, SOL_SOCKET, SO_DONTLINGER, (const char*)&no, sizeof(int)) < 0) {
		shutdown(_sock, SD_BOTH);
		closesocket(_sock);
		_sock = INVALID_SOCKET;
		return;
	}
}

SOCKET EthernetServer::make_new_client(SOCKET _sock)
{
	SOCKET tempsock;
	int addrsize, idx;
	int yes = 1;
	u_long lArg = 1;
	struct sockaddr_in pin;
	struct linger linger;
	
	for (idx = 0; idx < MAX_SOCK_NUM; idx++)
		if (Clients[idx]._sock == INVALID_SOCKET)
			break;
	
	addrsize = sizeof(pin);
	if ((tempsock = accept(_sock, (struct sockaddr *)&pin, &addrsize)) < 0)
		return INVALID_SOCKET;
	
	if (ioctlsocket(tempsock, FIONBIO, &lArg) != 0) {
		shutdown(tempsock, SD_BOTH);
		closesocket(tempsock);
		return INVALID_SOCKET;
	}
	
	if (setsockopt(tempsock, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(int)) < 0) {
		shutdown(tempsock, SD_BOTH);
		closesocket(tempsock);
		return INVALID_SOCKET;
	}
	
	linger.l_onoff = 1;
	linger.l_linger = 1; 
	if (setsockopt(tempsock, SOL_SOCKET, SO_LINGER, (const char*)&linger, sizeof(linger)) < 0) {
		shutdown(tempsock, SD_BOTH);
		closesocket(tempsock);
		return INVALID_SOCKET;
	}
	
	if (idx >= MAX_SOCK_NUM) {
		shutdown(tempsock, SD_BOTH);
		closesocket(tempsock);
		return INVALID_SOCKET;
	}
	
	Clients[idx]._sock = tempsock;
	Clients[idx]._id = idx;
	
	return tempsock;
}

EthernetClient EthernetServer::available()
{
	SOCKET tempsock;
	SOCKET socks[MAX_SOCK_NUM+1];
	int i, j;
	struct timeval seltime;
	
	if (_sock != INVALID_SOCKET)
	{
		socks[0] = _sock;
		FD_ZERO(&rfds);
		FD_SET(_sock, &rfds);
		for (i = 0; i < MAX_SOCK_NUM; i++) {
		
			socks[i+1] = Clients[i]._sock;
			
			if (Clients[i]._sock != INVALID_SOCKET)
				FD_SET(Clients[i]._sock, &rfds);
		}

		seltime.tv_sec = 0;
		seltime.tv_usec = 0;
		if (!select(MAX_SOCK_NUM+1, &rfds, NULL, NULL, &seltime))
			return EthernetClient(INVALID_SOCKET);
		
		for (i = 0; i < MAX_SOCK_NUM+1; i++) {
			
			if (FD_ISSET(socks[i], &rfds)) {
				
				if (socks[i] == _sock)
					tempsock = make_new_client(_sock);
				else
					tempsock = socks[i];
				
				if (tempsock != INVALID_SOCKET && socks[i] != _sock) {
					for (j = 0; j < MAX_SOCK_NUM; j++)
						if (Clients[j]._sock == tempsock) {
						
							uint8_t val;
							
							if (recv(Clients[j]._sock, &val, 1, MSG_PEEK) <= 0)
								Clients[j].stop();
							
							return Clients[j];
						}
				}
			}
		}
	}
	
	return EthernetClient(INVALID_SOCKET);
}

size_t EthernetServer::write(uint8_t b) 
{
	return write(&b, 1);
}

size_t EthernetServer::write(const uint8_t *buffer, size_t size) 
{
	int rc, i;
	size_t n = 0;
	
	for (i = 0; i < MAX_SOCK_NUM; i++) {
		if (Clients[i]._sock != INVALID_SOCKET) {
			rc = send(Clients[i]._sock, buffer, size, 0);
			if (rc > 0)
				n += rc;
		}
	}
	
	return n;
}

void EthernetServer::closeServerSocket(int idx)
{
	if (idx >= 0 && idx < MAX_SOCK_NUM)
		Clients[idx]._sock = INVALID_SOCKET;
}
	