/*
 *  ENUdp.cpp: Library to send/receive UDP packets with the Arduino ethernet shield.
 *  This version only offers minimal wrapping of socket.c/socket.h
 *  Drop ENUdp.h/.cpp into the Ethernet library directory at hardware/libraries/Ethernet/ 
 *
 * MIT License:
 * Copyright (c) 2008 Bjoern Hartmann
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * bjoern@cs.stanford.edu 12/30/2008
 * 
 * Modified 12 December 2013 by DY Hung   
 */
 
#include "Ethernet.h"
#include "Udp.h"
#include "Dns.h"
#include "SwsSock.h"

/* Constructor */
EthernetUDP::EthernetUDP() : _sock(INVALID_SOCKET)
{
	TxSize = 0;
	RxHead = 0;
	RxTail = 0;
	RxSize = 0;
}

/* Start EthernetUDP socket, listening at local port PORT */
uint8_t EthernetUDP::begin(uint16_t port) {
	struct sockaddr_in sin;
	u_long lArg = 1;
	int yes = 1, no = 0;
	
	if (_sock != INVALID_SOCKET)
		return 0;
	
	_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (_sock == INVALID_SOCKET)
		return 0;
	
	bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = SwsSock.getULLocalIp();
    sin.sin_port = htons(port);
	
	if (bind(_sock, (struct sockaddr *)&sin, sizeof(sin)) != 0) {
		shutdown(_sock, SD_BOTH);
		closesocket(_sock);
		_sock = INVALID_SOCKET;
		return 0;
    }
	
	if (ioctlsocket(_sock, FIONBIO, &lArg) < 0) {
		shutdown(_sock, SD_BOTH);
		closesocket(_sock);
		_sock = INVALID_SOCKET;
		return 0;
	}
	
	if (setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(int)) < 0) {
		shutdown(_sock, SD_BOTH);
		closesocket(_sock);
		_sock = INVALID_SOCKET;
		return 0;
	}
	
	_port = port;
	TxSize = 0;
	RxHead = 0;
	RxTail = 0;
	RxSize = 0;
	
	return 1;
}

/* return number of bytes available in the current packet,
   will return zero if parsePacket hasn't been called yet */
int EthernetUDP::available()
{
	return RxSize;
}

/* Release any resources being used by this EthernetUDP instance */
void EthernetUDP::stop()
{
	if (_sock == INVALID_SOCKET)
		return;
	
	flush();
	shutdown(_sock, SD_BOTH);
	closesocket(_sock);

	TxSize = 0;
	_sock = INVALID_SOCKET;
}

int EthernetUDP::beginPacket(const char *host, uint16_t port)
{
	// Look up the host first
	int ret = 0;
	DNSClient dns;
	IPAddress remote_addr;

	dns.begin(Ethernet.dnsServerIP());
	ret = dns.getHostByName(host, remote_addr);
	if (ret == 1)
		return beginPacket(remote_addr, port);
	else
		return ret;
}

int EthernetUDP::beginPacket(IPAddress ip, uint16_t port)
{
	uint8_t *ipchar;
	u_long iplong;
	
	if (_sock == INVALID_SOCKET)
		return 0;
	
	ipchar = rawIPAddress(ip);
	memcpy(&iplong, ipchar, 4);
	
	bzero(&txaddr, sizeof(txaddr));
    txaddr.sin_family = AF_INET;
    txaddr.sin_addr.s_addr = iplong;
    txaddr.sin_port = htons(port);
	
	TxSize = 0;
	
	return 1;
}

int EthernetUDP::endPacket()
{
	int rc, curByte;
	
	if (_sock == INVALID_SOCKET)
		return 0;
	
	curByte = 0;
	while (TxSize > 0) {
		rc = sendto(_sock, &TxBuffer[curByte], TxSize, 0, (struct sockaddr *)&txaddr, sizeof(txaddr));
		
		if (rc > 0) {
			curByte += rc;
			TxSize -= rc;
		}
		else {
			stop();
			return 0;
		}
	}
	
	return 1;
}

size_t EthernetUDP::write(uint8_t byte)
{
	return write(&byte, 1);
}

size_t EthernetUDP::write(const uint8_t *buffer, size_t size)
{
	size_t bytes_written = 0;
	
	if (_sock == INVALID_SOCKET)
		return 0;
	
	while (TxSize < UDP_TX_PACKET_MAX_SIZE && size > 0) {
		TxBuffer[TxSize++] = buffer[bytes_written++];
		size--;
	}
	
	return bytes_written;
}

int EthernetUDP::parsePacket()
{
	int i, rc, len;
	uint8_t buf[UDP_RX_PACKET_MAX_SIZE];
	
	if (RxSize > 0)
		return RxSize;
	
	if (_sock == INVALID_SOCKET)
		return 0;
	
	len = sizeof(rxaddr);
	rc = recvfrom(_sock, buf, UDP_RX_PACKET_MAX_SIZE, 0, (struct sockaddr *)&rxaddr, &len);
	
	if (rc > 0) {
	
		uint8_t tmpBuf[4];
		
		memcpy(tmpBuf, &rxaddr.sin_addr.s_addr, 4);
		_remoteIP = tmpBuf;
		_remotePort = ntohs(rxaddr.sin_port);
		
		i = 0;
		while (rc > 0) {
			RxBuffer[RxTail++] = buf[i++];
			RxSize++;
			if (RxTail >= UDP_RX_PACKET_MAX_SIZE)
				RxTail = 0;
				
			rc--;
		}
	}
	
	return RxSize;
}

int EthernetUDP::read()
{
	uint8_t val;
	
	if (read(&val, 1) > 0)
		return val;
		
	return -1;
}

int EthernetUDP::read(unsigned char* buf, size_t size)
{
	int rc, cur, len;
	uint8_t val;
		
	cur = 0;
	
	if (RxSize > 0) {
	
		while (RxSize > 0 && size > 0) {
			buf[cur++] = RxBuffer[RxHead++];
			RxSize--;
			size--;
			if (RxHead >= UDP_RX_PACKET_MAX_SIZE)
				RxHead = 0;
		}
	}
	else if (_sock != INVALID_SOCKET && size > 0) {
	
		len = sizeof(rxaddr);
		rc = recvfrom(_sock, &buf[cur], size, 0, (struct sockaddr *)&rxaddr, &len);
		
		if (rc > 0) {
		
			uint8_t tmpBuf[4];
			
			memcpy(tmpBuf, &rxaddr.sin_addr.s_addr, 4);
			_remoteIP = tmpBuf;
			_remotePort = ntohs(rxaddr.sin_port);
			
			size -= rc;
			cur += rc;
		}
		else
			return -1;
	}
	else
		return -1;
	
	return cur;
}

int EthernetUDP::peek()
{
	if (_sock == INVALID_SOCKET)
		return -1;
		
	if (!available())
		return -1;
		
	return RxBuffer[RxHead];
}

void EthernetUDP::flush()
{
	int rc, len;
	uint8_t val;
	
	RxSize = 0;
	RxHead = 0;
	RxTail = 0;
	
	if (_sock == INVALID_SOCKET)
		return;
	
	len = sizeof(rxaddr);
	while ((rc = recvfrom(_sock, &val, 1, 0, (struct sockaddr *)&rxaddr, &len)) > 0);
}

