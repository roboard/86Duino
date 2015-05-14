/*
  ENClient.cpp - Part of DM&P Vortex86 Ethernet library
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
#include "Arduino.h"

#include "Ethernet.h"
#include "EthernetClient.h"
#include "EthernetServer.h"
#include "Dns.h"

static int swssock_connect(SWS_SOCKET s, const SWS_sockaddr *addr, int size)
{
	int rc;
	SWS_fd_set fds;
	struct SWS_timeval seltime;
	
	SWS_connect(s, addr, size);

	SWS_FdZero(&fds);
	SWS_FdSet(s,&fds);

	seltime.tv_sec = 10;
	seltime.tv_usec = 0;

	if (SWS_select(NULL, &fds, NULL, &seltime) <= 0)
		return -1;

	return 0;
}

EthernetClient::EthernetClient()
{
	_id = -1;
	pServer = NULL;
	sws = (struct SwsSockInfo*)malloc(sizeof(struct SwsSockInfo));
	if (sws) {
		memset(sws, 0, sizeof(struct SwsSockInfo));
		sws->_sock = SWS_INVALID_SOCKET;
	}
	_RegLen = 0;
}

EthernetClient::EthernetClient(struct SwsSockInfo *info)
{
	_id = -1;
	pServer = NULL;
	sws = info;
	_RegLen = 0;
}

int EthernetClient::connect(const char* host, uint16_t port)
{
	int ret = 0;
	DNSClient dns;
	IPAddress remote_addr;
	IPAddress dns_server;

	dns.begin(Ethernet.dnsServerIP());
	ret = dns.getHostByName(host, remote_addr);
	if (ret == 1)
		return connect(remote_addr, port);
	
	return ret;
}

int EthernetClient::connect(IPAddress ip, uint16_t port)
{
	struct SWS_sockaddr_in pin;
	int yes = 1, no = 0;
	uint8_t *ipchar;
	SWS_u_long iplong, lArg = 1;
	
	if (sws == NULL) return 0;
	
	if (sws->_sock != SWS_INVALID_SOCKET)
		stop();
	
	ipchar = rawIPAddress(ip);
	memcpy(&iplong, ipchar, 4);
	
	bzero(&pin, sizeof(pin));
    pin.sin_family = SWS_AF_INET;
    pin.sin_addr.SWS_s_addr = iplong;
    pin.sin_port = SWS_htons(port);
	
	sws->_sock = SWS_socket(SWS_AF_INET, SWS_SOCK_STREAM, 0);
    if (sws->_sock == SWS_INVALID_SOCKET)
		return 0;
	
	if (SWS_setsockopt(sws->_sock, SWS_SOL_SOCKET, SWS_SO_REUSEADDR, (const char*)&yes, sizeof(int)) < 0) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		sws->_sock = SWS_INVALID_SOCKET;
		return 0;
	}
	
	if (SWS_setsockopt(sws->_sock, SWS_SOL_SOCKET, SWS_SO_DONTLINGER, (const char*)&no, sizeof(int)) < 0) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		sws->_sock = SWS_INVALID_SOCKET;
		return 0;
	}
	
	if (SWS_ioctl(sws->_sock, SWS_FIONBIO, &lArg) < 0) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		sws->_sock = SWS_INVALID_SOCKET;
		return 0;
	}

    if (swssock_connect(sws->_sock, (const SWS_sockaddr*)&pin, sizeof(pin)) == -1) {
		SWS_shutdown(sws->_sock, SWS_SD_BOTH);
		SWS_close(sws->_sock);
		sws->_sock = SWS_INVALID_SOCKET;
		return 0;
    }
	
	_id = -1;
	pServer = NULL;
	
	return 1;
}

int EthernetClient::writeNB(uint8_t b)
{
	return writeNB(&b, 1);
}

int EthernetClient::writeNB(const uint8_t *buf, size_t size)
{
	int ret = -1;
	
	if (sws == NULL) return ret;
	if (sws->_sock == SWS_INVALID_SOCKET)
		return ret;
	
	ret = SWS_send(sws->_sock, buf, size, 0);
	if (ret < 0 && SWS_GetLastError() == SWS_EWOULDBLOCK)
		ret = 0;
		
	return ret;
}

size_t EthernetClient::write(uint8_t b)
{
	return write(&b, 1);
}

size_t EthernetClient::write(const uint8_t *buf, size_t size)
{
	int ret;
	size_t n = 0, res = size;
	
	while (n < size) {
		if ((ret = writeNB(&buf[n], res)) < 0)
			break;
		n += ret;
		res -= ret;
	}
	
	return n;
}

int EthernetClient::available()
{
	int rc, cur = 0;
	uint8_t buf[BUFFER_SIZE];
	
	if (_RegLen > 0)
		cur = 1;
	
	if (sws == NULL) return 0;
	if (sws->_sock != SWS_INVALID_SOCKET) {
	
		rc = SWS_recv(sws->_sock, buf, BUFFER_SIZE-cur, SWS_MSG_PEEK);
		
		if (rc > 0)
			cur += rc;
	}
	
	return cur;
}

int EthernetClient::read()
{
	int rc;
	uint8_t val;
	
	if (read(&val, 1) == 0)
		return -1;
	
	return val;
}

int EthernetClient::read(uint8_t *buf, size_t size)
{
	int rc, cur = 0;
	
	if (_RegLen > 0) {
		buf[cur++] = _RegBuf;
		_RegLen = 0;
		size--;
	}
	
	if (sws == NULL) return 0;
	if (sws->_sock != SWS_INVALID_SOCKET && size > 0) {
		
		rc = SWS_recv(sws->_sock, &buf[cur], size, 0);
		if (rc > 0)
			cur += rc;
	}
	
	return cur;
}

int EthernetClient::peek()
{
	int rc;
	uint8_t val;
	
	if (_RegLen > 0) {
		val = _RegBuf;
		_RegLen = 0;
		return val;
	}
	
	if (sws == NULL) return -1;
	if (sws->_sock == SWS_INVALID_SOCKET)
		return -1;
	
	if (SWS_recv(sws->_sock, &val, 1, SWS_MSG_PEEK) <= 0)
		return -1;
	
	return val;
}

void EthernetClient::flush()
{
	uint8_t val;
	
	if (sws == NULL) return;
	if (sws->_sock == SWS_INVALID_SOCKET)
		return;
		
	while (SWS_recv(sws->_sock, &val, 1, 0) > 0);
}

void EthernetClient::stop()
{
	// flush();
	
	if (sws) {
		if (sws->_sock != SWS_INVALID_SOCKET) {
			SWS_shutdown(sws->_sock, SWS_SD_BOTH);
			SWS_close(sws->_sock);
			sws->_sock = SWS_INVALID_SOCKET;
		}
	}
	if (pServer != NULL) {
		pServer->closeServerSocket(this->_id);
	}
}

uint8_t EthernetClient::connected()
{
	int rc;
	uint8_t val;
	
	if (available() > 0)
		return 1;
	
	if (sws == NULL) return 0;
	if (sws->_sock == SWS_INVALID_SOCKET)
		return 0;
	
	rc = SWS_recv(sws->_sock, &val, 1, 0);
	if (rc == 0)
		return 0;
	
	if (rc > 0) {
		_RegBuf = val;
		_RegLen = 1;
	}
	
	return 1;
}

uint8_t EthernetClient::status()
{
	if (sws == NULL) return sws != NULL;
	return sws->_sock != SWS_INVALID_SOCKET;
}

// the next function allows us to use the client returned by
// EthernetServer::available() as the condition in an if-statement.

EthernetClient::operator bool()
{
	if (sws == NULL) return sws != NULL;
	return sws->_sock != SWS_INVALID_SOCKET;
}
