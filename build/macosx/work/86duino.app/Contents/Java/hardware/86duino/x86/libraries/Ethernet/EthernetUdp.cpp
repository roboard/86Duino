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

#if defined(ETHERNET_REMOTE)

static unsigned long id_count = 0;

extern void send_command_para(uint8_t c, uint8_t *p, int p_size);
extern int recv_command_result(uint8_t *mybuf, int num);
extern int fourBytes2Int(unsigned char a0, unsigned char a1, unsigned char a2, unsigned char a3);
extern VDPM_HANDLE_t vdppSlave;

EthernetUDP::EthernetUDP()
{
    TxSize = 0;
    RxHead = 0;
    RxTail = 0;
    RxSize = 0;
    _id_transmit = 0;
}

EthernetUDP::~EthernetUDP()
{
    _id_transmit = 0;
}

/* Start EthernetUDP socket, listening at local port PORT */
uint8_t EthernetUDP::begin(uint16_t port)
{
    int i, r;
    unsigned char tmp[2], _r[1];

    tmp[0] = port & 0xFF;
    tmp[1] = (port >> 8) & 0xFF;

    send_command_para(26, tmp, 2); // 26 : EthernetUDP.begin() command

    r = recv_command_result(_r, 2);
    if (r == 0 && _r[0] == 1)
    {
        _id_transmit = _r[1];
        return 1;
    }

    return 0;
}

/* return number of bytes available in the current packet,
will return zero if parsePacket hasn't been called yet */
int EthernetUDP::available()
{
    int i, r;
    unsigned char tmp[1], _r[4];

    // _id_transmit : 1 byte
    tmp[0] = _id_transmit;

    send_command_para(27, tmp, 1); // 27 : client.available() command

    r = recv_command_result(_r, 4);
    if (r == 0)
    {
        return fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
    }

    return 0;
}

/* Release any resources being used by this EthernetUDP instance */
void EthernetUDP::stop()
{
    int i, r;
    unsigned char tmp[1], _r[1];

    // _id_transmit : 1 byte
    tmp[0] = _id_transmit;

    send_command_para(28, tmp, 1); // 28 : EthernetUDP.stop() command

    r = recv_command_result(_r, 1);
    if (r == 0)
        printf("stop OK\n");
    else
        printf("stop Fail\n");
}

int EthernetUDP::beginPacket(const char *host, uint16_t port)
{
    int i, r;
    unsigned char tmp[1024], _r[4];

    // _id_transmit
    tmp[0] = _id_transmit;
    // port
    tmp[1] = port & 0xFF;
    tmp[2] = (port >> 8) & 0xFF;
    // host
    for (i = 0; host[i] != '\0'; i++)
        tmp[3 + i] = host[i];
    tmp[3 + i] = '\0';

    send_command_para(29, tmp, 4 + i); // 29 : EthernetUDP.beginPacket() command

    r = recv_command_result(_r, 4);
    if (r == 0)
    {
        return fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
    }

    return 0;
}

int EthernetUDP::beginPacket(IPAddress ip, uint16_t port)
{
    int r;
    unsigned char tmp[10], _r[4];

    // _id_transmit
    tmp[0] = _id_transmit;
    // IP
    tmp[1] = ip[0];
    tmp[2] = ip[1];
    tmp[3] = ip[2];
    tmp[4] = ip[3];
    // port
    tmp[4] = port & 0xFF;
    tmp[6] = (port >> 8) & 0xFF;

    send_command_para(30, tmp, 7); // 30 : EthernetUDP.beginPacket() command

    r = recv_command_result(_r, 4);
    if (r == 0)
    {
        return fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
    }

    return 0;
}

int EthernetUDP::endPacket()
{
    int i, r;
    unsigned char tmp[1], _r[4];

    // _id_transmit : 1 byte
    tmp[0] = _id_transmit;

    send_command_para(31, tmp, 1); // 31 : EthernetUDP.endPacket() command

    r = recv_command_result(_r, 4);
    if (r == 0)
    {
        return fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
    }

    return 0;
}

size_t EthernetUDP::write(uint8_t byte)
{
    return write(&byte, 1);
}

size_t EthernetUDP::write(const uint8_t *buf, size_t size)
{
    int i, j, r;
    unsigned char tmp[10], _r[4];

    tmp[0] = _id_transmit;
    for (i = 1, j = 0; j < 4; i++, j++)
        tmp[i] = (size >> (j * 8)) & 0xFF;

    // vdpm write
    vdpm_MemWrite(vdppSlave, 0, (uint8_t *)buf, size);

    // talk to master to read them and call write()
    send_command_para(32, (uint8_t *)tmp, i);

    r = recv_command_result(_r, 4);
    if (r == 0)
    {
        return fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
    }

    return 0;
}

int EthernetUDP::parsePacket()
{
    int i, r;
    unsigned char tmp[1], _r[4];

    // _id_transmit : 1 byte
    tmp[0] = _id_transmit;

    send_command_para(33, tmp, 1); // 33 : EthernetUDP.parsePacket() command

    r = recv_command_result(_r, 10);
    _remoteIP[0] = _r[4];
    _remoteIP[1] = _r[5];
    _remoteIP[2] = _r[6];
    _remoteIP[3] = _r[7];
    _remotePort = (uint16_t)_r[8] + ((uint16_t)_r[9] << 8);
    if (r == 0)
    {
        return fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
    }

    return 0;
}

int EthernetUDP::read()
{
    int r;
    unsigned char tmp[1], _r[4];

    tmp[0] = _id_transmit;

    send_command_para(34, (uint8_t *)tmp, 1); // 34 : EthernetUDP.read() command

    r = recv_command_result(_r, 4);
    if (r == 0)
    {
        return fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
    }

    return 0;
}

int EthernetUDP::read(unsigned char *buf, size_t size)
{
    int i, j, r;
    unsigned char tmp[10], _r[4];

    tmp[0] = _id_transmit;
    for (i = 1, j = 0; j < 4; i++, j++)
        tmp[i] = (size >> (j * 8)) & 0xFF;

    // talk to master start to read data and write into RAM
    send_command_para(35, (uint8_t *)tmp, i);
    r = recv_command_result(_r, 10);
    _remoteIP[0] = _r[4];
    _remoteIP[1] = _r[5];
    _remoteIP[2] = _r[6];
    _remoteIP[3] = _r[7];
    _remotePort = (uint16_t)_r[8] + ((uint16_t)_r[9] << 8);
    if (r == 0)
    {
        r = fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
        vdpm_MemRead(vdppSlave, 0, (uint8_t *)buf, r);
        return r;
    }

    return 0;
}

int EthernetUDP::peek()
{
    int i, r;
    unsigned char tmp[1], _r[4];

    // _id_transmit : 1 byte
    tmp[0] = _id_transmit;

    send_command_para(36, tmp, 1); // 36 : EthernetUDP.peek() command

    r = recv_command_result(_r, 4);
    if (r == 0)
    {
        return fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
    }

    return 0;
}

void EthernetUDP::flush()
{
    int i, r;
    unsigned char tmp[1], _r[1];

    // _id_transmit : 1 byte
    tmp[0] = _id_transmit;

    send_command_para(37, tmp, 1); // 37 : EthernetUDP.flush() command

    r = recv_command_result(_r, 1);
    if (r == 0)
        printf("flush OK");
    else
        printf("flush Fail");
}

#else

#define UDP_PACKET_MAX_SIZE (65507)

/* Constructor */
EthernetUDP::EthernetUDP()
{
    this->_sock = SWS_INVALID_SOCKET;
    this->_multicast = false;
    this->_localPort = 0;
    this->_txRemotePort = 0;
    this->_rxRemotePort = 0;
    this->_txRemoteIP = (uint32_t)0;
    this->_rxRemoteIP = (uint32_t)0;
    this->_txBuffer = NULL;
    this->_rxBuffer = NULL;
    this->_txSize = 0;
    this->_rxSize = 0;
    this->_rxHead = 0;
#if defined(__86DUINO_DUO)
    // _id_transmit = id_count++;
#endif
}

EthernetUDP::~EthernetUDP()
{
    stop();
}

/* Start EthernetUDP socket, listening at local port PORT */
uint8_t EthernetUDP::begin(uint16_t port)
{
    SWS_SOCKET sock;
    SWS_sockaddr_in sin;
    SWS_u_long lArg = 1;
    int yes = 1, no = 0;
    uint8_t *buffer;

    if (this->_sock != SWS_INVALID_SOCKET)
        return 0;
    if (SwsSock.isInitialized() == 0)
        return 0;

    if ((buffer = (uint8_t *)malloc(UDP_PACKET_MAX_SIZE << 1)) == NULL)
        return 0;

    if ((sock = SWS_socket(SWS_AF_INET, SWS_SOCK_DGRAM, 0)) == SWS_INVALID_SOCKET)
    {
        free(buffer);
        return 0;
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = SWS_AF_INET;
    sin.sin_addr.SWS_s_addr = SwsSock.getLocalIp();
    sin.sin_port = SWS_htons(port);
    if (SWS_bind(sock, (struct SWS_sockaddr *)&sin, sizeof(sin)) != 0)
    {
        SWS_shutdown(sock, SWS_SD_BOTH);
        SWS_close(sock);
        free(buffer);
        return 0;
    }

    if (SWS_ioctl(sock, SWS_FIONBIO, &lArg) < 0)
    {
        SWS_shutdown(sock, SWS_SD_BOTH);
        SWS_close(sock);
        free(buffer);
        return 0;
    }

    if (SWS_setsockopt(sock, SWS_SOL_SOCKET, SWS_SO_REUSEADDR, (const char *)&yes, sizeof(int)) < 0)
    {
        SWS_shutdown(sock, SWS_SD_BOTH);
        SWS_close(sock);
        free(buffer);
        return 0;
    }

    if (SWS_setsockopt(sock, SWS_SOL_SOCKET, SWS_SO_DONTLINGER, (const char *)&no, sizeof(int)) < 0)
    {
        SWS_shutdown(sock, SWS_SD_BOTH);
        SWS_close(sock);
        free(buffer);
        return 0;
    }

    this->_txBuffer = buffer;
    this->_rxBuffer = buffer + UDP_PACKET_MAX_SIZE;
    this->_txSize = 0;
    this->_rxSize = 0;
    this->_localPort = port;
    this->_sock = sock;

    return 1;
}

uint8_t EthernetUDP::beginMulticast(IPAddress ip, uint16_t port)
{
    SWS_SOCKET sock;
    SWS_sockaddr_in sin;
    SWS_ip_mreq mreq;
    SWS_u_long lArg = 1;
    int yes = 1, no = 0;
    uint8_t *buffer;

    if (this->_sock != SWS_INVALID_SOCKET)
        return 0;
    if (SwsSock.isInitialized() == 0)
        return 0;

    if ((buffer = (uint8_t *)malloc(UDP_PACKET_MAX_SIZE << 1)) == NULL)
        return 0;

    if ((sock = SWS_socket(SWS_AF_INET, SWS_SOCK_DGRAM, 0)) == SWS_INVALID_SOCKET)
    {
        free(buffer);
        return 0;
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = SWS_AF_INET;
    sin.sin_addr.SWS_s_addr = SWS_INADDR_ANY;
    sin.sin_port = SWS_htons(port);
    if (SWS_bind(sock, (struct SWS_sockaddr *)&sin, sizeof(sin)) != 0)
    {
        SWS_shutdown(sock, SWS_SD_BOTH);
        SWS_close(sock);
        free(buffer);
        return 0;
    }

    mreq.imr_multiaddr.S_un.S_addr = (uint32_t)ip;
    mreq.imr_interface.S_un.S_addr = SWS_INADDR_ANY;
    if (SWS_setsockopt(sock, SWS_IPPROTO_IP, SWS_IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
    {
        SWS_shutdown(sock, SWS_SD_BOTH);
        SWS_close(sock);
        free(buffer);
        return 0;
    }

    if (SWS_ioctl(sock, SWS_FIONBIO, &lArg) < 0)
    {
        SWS_shutdown(sock, SWS_SD_BOTH);
        SWS_close(sock);
        free(buffer);
        return 0;
    }

    if (SWS_setsockopt(sock, SWS_SOL_SOCKET, SWS_SO_REUSEADDR, (const char *)&yes, sizeof(int)) < 0)
    {
        SWS_shutdown(sock, SWS_SD_BOTH);
        SWS_close(sock);
        free(buffer);
        return 0;
    }

    if (SWS_setsockopt(sock, SWS_SOL_SOCKET, SWS_SO_DONTLINGER, (const char *)&no, sizeof(int)) < 0)
    {
        SWS_shutdown(sock, SWS_SD_BOTH);
        SWS_close(sock);
        free(buffer);
        return 0;
    }

    this->_txBuffer = buffer;
    this->_rxBuffer = buffer + UDP_PACKET_MAX_SIZE;
    this->_txSize = 0;
    this->_rxSize = 0;
    this->_localPort = port;
    this->_sock = sock;
    this->_multicast = true;
    SwsSock.setPromiscuousMode(true);

    return 1;
}

/* return number of bytes available in the current packet,
will return zero if parsePacket hasn't been called yet */
int EthernetUDP::available()
{
    SWS_SOCKET sock = (SWS_SOCKET)this->_sock;

    if (sock == SWS_INVALID_SOCKET)
        return 0;

    return this->_rxSize;
}

/* Release any resources being used by this EthernetUDP instance */
void EthernetUDP::stop()
{
    SWS_SOCKET sock = (SWS_SOCKET)this->_sock;

    if (sock == SWS_INVALID_SOCKET)
        return;

    flush();

    if (this->_multicast == true)
    {
        this->_multicast = false;
        SwsSock.setPromiscuousMode(false);
    }
    SWS_shutdown(sock, SWS_SD_BOTH);
    SWS_close(sock);
    free(this->_txBuffer);
    this->_sock = SWS_INVALID_SOCKET;
    this->_localPort = 0;
    this->_txRemotePort = 0;
    this->_rxRemotePort = 0;
    this->_txRemoteIP = (uint32_t)0;
    this->_rxRemoteIP = (uint32_t)0;
    this->_txBuffer = NULL;
    this->_rxBuffer = NULL;
    this->_txSize = 0;
    this->_rxSize = 0;
    this->_rxHead = 0;
}

int EthernetUDP::beginPacket(const char *host, uint16_t port)
{
    DNSClient dns;
    IPAddress remote_addr;
    int ret;

    dns.begin(Ethernet.dnsServerIP());
    if ((ret = dns.getHostByName(host, remote_addr)) == 1)
        return beginPacket(remote_addr, port);

    return ret;
}

int EthernetUDP::beginPacket(IPAddress ip, uint16_t port)
{
    if (this->_sock == SWS_INVALID_SOCKET)
        return 0;

    this->_txRemoteIP = ip;
    this->_txRemotePort = port;
    this->_txSize = 0;

    return 1;
}

int EthernetUDP::endPacket()
{
    SWS_SOCKET sock = (SWS_SOCKET)this->_sock;
    SWS_sockaddr_in tx_address;
    uint32_t sent = 0;

    if (sock == SWS_INVALID_SOCKET)
        return 0;
    if (this->_txSize == 0)
        return 0;

    memset(&tx_address, 0, sizeof(tx_address));
    tx_address.sin_family = SWS_AF_INET;
    tx_address.sin_addr.SWS_s_addr = (uint32_t)this->_txRemoteIP;
    tx_address.sin_port = SWS_htons(this->_txRemotePort);

    while (sent < this->_txSize)
    {
        int rc = SWS_sendto(sock, &this->_txBuffer[sent], this->_txSize - sent, 0, (SWS_sockaddr *)&tx_address, sizeof(tx_address));
        if (rc <= 0)
            break;
        sent += rc;
    }
    this->_txSize = 0;

    return !!(sent > 0);
}

size_t EthernetUDP::write(uint8_t byte)
{
    return write(&byte, 1);
}

size_t EthernetUDP::write(const uint8_t *buffer, size_t size)
{
    SWS_SOCKET sock = (SWS_SOCKET)this->_sock;

    if (sock == SWS_INVALID_SOCKET)
        return 0;

    if (size > UDP_PACKET_MAX_SIZE - this->_txSize)
        size = UDP_PACKET_MAX_SIZE - this->_txSize;
    if (size == 0)
        return 0;

    memcpy(&this->_txBuffer[this->_txSize], buffer, size);
    this->_txSize += size;

    return size;
}

int EthernetUDP::parsePacket()
{
    SWS_SOCKET sock = (SWS_SOCKET)this->_sock;
    SWS_sockaddr_in rx_address;
    SWS_socklen_t socklen = sizeof(rx_address);
    int rc;

    if (sock == SWS_INVALID_SOCKET)
        return 0;

    if ((rc = SWS_recvfrom(sock, this->_rxBuffer, UDP_PACKET_MAX_SIZE, 0, (struct SWS_sockaddr *)&rx_address, &socklen)) <= 0)
        return 0;

    this->_rxRemoteIP = (uint32_t)rx_address.sin_addr.SWS_s_addr;
    this->_rxRemotePort = SWS_ntohs(rx_address.sin_port);
    this->_rxSize = rc;
    this->_rxHead = 0;

    return this->_rxSize;
}

int EthernetUDP::read()
{
    uint8_t val;

    if (read(&val, 1) > 0)
        return val;

    return -1;
}

int EthernetUDP::read(unsigned char *buffer, size_t len)
{
    SWS_SOCKET sock = (SWS_SOCKET)this->_sock;

    if (sock == SWS_INVALID_SOCKET)
        return -1;

    if (len > this->_rxSize)
        len = this->_rxSize;
    if (len == 0)
        return -1;

    memcpy(buffer, &this->_rxBuffer[this->_rxHead], len);
    this->_rxSize -= len;
    this->_rxHead += len;

    return len;
}

int EthernetUDP::peek()
{
    if (available() == 0)
        return -1;
    return this->_rxBuffer[this->_rxHead];
}

void EthernetUDP::flush()
{
    SWS_SOCKET sock = (SWS_SOCKET)this->_sock;
    SWS_sockaddr_in rx_address;
    SWS_socklen_t socklen = sizeof(rx_address);

    if (sock == SWS_INVALID_SOCKET)
        return;

    this->_rxSize = 0;
    this->_rxHead = 0;
    while (SWS_recvfrom(sock, this->_rxBuffer, UDP_PACKET_MAX_SIZE, 0, (struct SWS_sockaddr *)&rx_address, &socklen) > 0);
}

#endif
