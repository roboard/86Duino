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

#include "SwsSock.h"
#include "Ethernet.h"
#include "EthernetClient.h"
#include "EthernetServer.h"
#include "Dns.h"

#if defined(ETHERNET_REMOTE)

static unsigned char id_count = 0;
extern void send_command_para(uint8_t c, uint8_t *p, int p_size);
extern int recv_command_result(uint8_t *mybuf, int num);
extern int fourBytes2Int(unsigned char a0, unsigned char a1, unsigned char a2, unsigned char a3);
extern VDPM_HANDLE_t vdppSlave;

EthernetClient::EthernetClient()
{
    _id = -1;
    pServer = NULL;
    _id_transmit = id_count++;
    _RegLen = 0;
}

EthernetClient::EthernetClient(int id)
{
    _id = -1;
    pServer = NULL;
    _id_transmit = id_count = id;
    id_count++;
    _RegLen = 0;
}

EthernetClient::EthernetClient(struct SwsSockInfo *info)
{
    _id = -1;
    pServer = NULL;
    sws = info;
    _id_transmit = id_count++;
    _RegLen = 0;
}

int EthernetClient::connect(const char *host, uint16_t port)
{
    int i, r;
    unsigned char tmp[1024], _r[1];

    for (i = 0; i < 1024; i++)
        tmp[i] = '\0';

    // _id_transmit : 1 byte
    tmp[0] = _id_transmit;
    // port : 1 byte
    tmp[1] = port & 0xFF;
    tmp[2] = (port >> 8) & 0xFF;
    // host : 1 byte
    for (i = 0; host[i] != '\0'; i++)
        tmp[3 + i] = host[i];
    tmp[3 + i] = '\0';

    send_command_para(11, tmp, 4 + i); // 11 : client.connect() command

    r = recv_command_result(_r, 1);
    if (r == 0)
    {
        return _r[0];
    }

    return 0;
}

int EthernetClient::connect(IPAddress ip, uint16_t port)
{
    int i, r;
    unsigned char tmp[1024], _r[1];

    for (i = 0; i < 1024; i++)
        tmp[i] = '\0';
    // _id_transmit : 1 byte
    tmp[0] = _id_transmit;
    // ip : 4 byte
    for (i = 0; i < 4; i++)
        tmp[1 + i] = ip[i];
    // port : 2 byte
    tmp[5] = port & 0xFF;
    tmp[6] = ((port >> 8) & 0xFF);

    send_command_para(12, tmp, 7); // 12 : client.connect() command

    r = recv_command_result(_r, 1);
    if (r == 0)
    {
        if (_r[0] == 0) // open "OK"
            return 1;
        else
            return 0;
    }

    return 0;
}

int EthernetClient::writeNB(uint8_t b)
{
    return writeNB(&b, 1);
}

int EthernetClient::writeNB(const uint8_t *buf, size_t size)
{
    int i, j, r;
    unsigned char tmp[1024], _r[4];

    for (i = 0; i < 1024; i++)
        tmp[i] = '\0';
    tmp[0] = _id_transmit;
    for (i = 1, j = 0; j < 4; i++, j++)
        tmp[i] = (size >> (j * 8)) & 0xFF;

    /*
    // vdpm write
    vdpm_MemWrite(vdppSlave, 0, (uint8_t*)buf, size);

    // talk to master to read them and call write()
    send_command_para(13, (uint8_t*)tmp, i);
    */
    r = recv_command_result(_r, 4);
    if (r == 0)
    {
        return fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
    }

    return 0;
}

size_t EthernetClient::write(uint8_t b)
{
    return write(&b, 1);
}

size_t EthernetClient::write(const uint8_t *buf, size_t size)
{
    int i, j, r;
    unsigned char tmp[1024], _r[4];

    for (i = 0; i < 1024; i++)
        tmp[i] = '\0';
    tmp[0] = _id_transmit;
    for (i = 1, j = 0; j < 4; i++, j++)
        tmp[i] = (size >> (j * 8)) & 0xFF;

    // vdpm write
    vdpm_MemWrite(vdppSlave, 0, (uint8_t *)buf, size);

    // talk to master to read them and call write()
    send_command_para(21, (uint8_t *)tmp, i);

    r = recv_command_result(_r, 4);
    if (r == 0)
    {
        return fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
    }

    return 0;
}

int EthernetClient::available()
{
    int i, r;
    unsigned char tmp[1], _r[4];

    // _id_transmit : 1 byte
    tmp[0] = _id_transmit;

    send_command_para(13, tmp, 1); // 13 : client.available() command

    r = recv_command_result(_r, 4);
    if (r == 0)
    {
        return fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
    }

    return 0;
}

int EthernetClient::read()
{
    int i, r;
    unsigned char tmp[1], _r[4];

    // _id_transmit : 1 byte
    tmp[0] = _id_transmit;

    send_command_para(14, tmp, 1); // 14 : client.read() command

    r = recv_command_result(_r, 4);
    if (r == 0)
    {
        return fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
    }

    return -1;
}

int EthernetClient::read(uint8_t *buf, size_t size)
{
    int i, j, r;
    unsigned char tmp[10], _r[4];

    tmp[0] = _id_transmit;
    for (i = 1, j = 0; j < 4; i++, j++)
        tmp[i] = (size >> (j * 8)) & 0xFF;

    // talk to master start to read data and write into RAM
    send_command_para(36, (uint8_t *)tmp, i);
    r = recv_command_result(_r, 4);
    if (r == 0)
    {
        r = fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
        vdpm_MemRead(vdppSlave, 0, (uint8_t *)buf, r);
        return r;
    }

    return 0;
}

int EthernetClient::peek()
{
    int i, r;
    unsigned char tmp[1], _r[1];

    // _id_transmit : 1 byte
    tmp[0] = _id_transmit;

    send_command_para(15, tmp, 1); // 15 : client.available() command

    r = recv_command_result(_r, 4);
    if (r == 0)
    {
        return fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
    }

    return -1;
}

void EthernetClient::flush()
{
    int i, r;
    unsigned char tmp[1], _r[1];

    // _id_transmit : 1 byte
    tmp[0] = _id_transmit;

    send_command_para(16, tmp, 1); // 16 : client.flush() command

    r = recv_command_result(_r, 1);
    if (r == 0)
        printf("flush OK\n");
    else
        printf("flush Fail\n");
}

void EthernetClient::stop()
{
    int i, r;
    unsigned char tmp[1], _r[1];

    // _id_transmit : 1 byte
    tmp[0] = _id_transmit;

    send_command_para(17, tmp, 1); // 17 : client.stop() command

    r = recv_command_result(_r, 1);
    if (r == 0)
        printf("stop OK\n");
    else
        printf("stop Fail\n");
}

uint8_t EthernetClient::connected()
{
    int r;
    unsigned char tmp[1], _r[1];

    // _id_transmit : 1 byte
    tmp[0] = _id_transmit;

    send_command_para(18, tmp, 1); // 18 : client.connected() command

    r = recv_command_result(_r, 1);

    if (r == 0)
    {
        return _r[0];
    }

    return 0;
}

uint8_t EthernetClient::status()
{
    int r;
    unsigned char tmp[1], _r[1];

    // _id_transmit : 1 byte
    tmp[0] = _id_transmit;

    send_command_para(19, tmp, 1); // 19 : client.status() command

    r = recv_command_result(_r, 1);

    if (r == 0)
    {
        return _r[0];
    }

    return 0;
}

// the next function allows us to use the client returned by
// EthernetServer::available() as the condition in an if-statement.

EthernetClient::operator bool()
{
    int r;
    unsigned char tmp[1], _r[1];

    // _id_transmit : 1 byte
    tmp[0] = _id_transmit;

    send_command_para(20, tmp, 1); // 20 : client.status() command

    r = recv_command_result(_r, 1);

    if (r == 0)
    {
        return _r[0];
    }

    return false;
}

#else

EthernetClient::EthernetClient(void *data)
{
    this->_sock = (SWS_SOCKET)data;
    this->_remoteIP = (uint32_t)0;
    this->_remotePort = 0;
    this->_connectionTimeout = 1000;
    this->_server = NULL;
    this->_serverIndex = 0;

#if defined(__86DUINO_DUO)
    // _id_transmit = id_count++;
#endif
}

EthernetClient::EthernetClient()
{
    this->_sock = SWS_INVALID_SOCKET;
    this->_remoteIP = (uint32_t)0;
    this->_remotePort = 0;
    this->_connectionTimeout = 1000;
    this->_server = NULL;
    this->_serverIndex = 0;

#if defined(__86DUINO_DUO)
    // _id_transmit = id_count++;
#endif
}

EthernetClient::~EthernetClient()
{
    this->_sock = SWS_INVALID_SOCKET;
    this->_remoteIP = (uint32_t)0;
    this->_remotePort = 0;
    this->_connectionTimeout = 1000;
    this->_server = NULL;
    this->_serverIndex = 0;
}

int EthernetClient::connect(const char *host, uint16_t port)
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
    SWS_SOCKET sock;
    SWS_sockaddr_in pin;
    SWS_timeval seltime;
    SWS_fd_set fds;
    SWS_u_long lArg = 1;
    int yes = 1, no = 0;

    if (this->_sock != SWS_INVALID_SOCKET)
        stop();
    if (SwsSock.isInitialized() == 0)
        return 0;

    if ((sock = SWS_socket(SWS_AF_INET, SWS_SOCK_STREAM, 0)) == SWS_INVALID_SOCKET)
        return 0;

    if (SWS_setsockopt(sock, SWS_SOL_SOCKET, SWS_SO_REUSEADDR, (const char *)&yes, sizeof(int)) < 0)
    {
        SWS_shutdown(sock, SWS_SD_BOTH);
        SWS_close(sock);
        return 0;
    }

    if (SWS_setsockopt(sock, SWS_SOL_SOCKET, SWS_SO_DONTLINGER, (const char *)&no, sizeof(int)) < 0)
    {
        SWS_shutdown(sock, SWS_SD_BOTH);
        SWS_close(sock);
        return 0;
    }

    if (SWS_ioctl(sock, SWS_FIONBIO, &lArg) < 0)
    {
        SWS_shutdown(sock, SWS_SD_BOTH);
        SWS_close(sock);
        return 0;
    }

    memset(&pin, 0, sizeof(pin));
    pin.sin_family = SWS_AF_INET;
    pin.sin_addr.SWS_s_addr = (uint32_t)ip;
    pin.sin_port = SWS_htons(port);
    SWS_connect(sock, (const SWS_sockaddr *)&pin, sizeof(pin));
    SWS_FdZero(&fds);
    SWS_FdSet(sock, &fds);
    seltime.tv_sec = _timeout / 1000;
    seltime.tv_usec = (_timeout % 1000) * 1000;
    if (SWS_select(NULL, &fds, NULL, &seltime) <= 0) {
        SWS_shutdown(sock, SWS_SD_BOTH);
        SWS_close(sock);
        return 0;
    }

    this->_sock = sock;
    this->_remoteIP = ip;
    this->_remotePort = port;

    return 1;
}

int EthernetClient::writeNB(uint8_t b)
{
    return writeNB(&b, 1);
}

int EthernetClient::writeNB(const uint8_t *buf, size_t size)
{
    SWS_SOCKET sock = (SWS_SOCKET)this->_sock;
    int ret;

    if (sock == SWS_INVALID_SOCKET)
        return -1;

    ret = SWS_send(sock, buf, size, 0);
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

    while (n < size)
    {
        if ((ret = writeNB(&buf[n], res)) < 0)
            break;
        n += ret;
        res -= ret;
    }

    return n;
}

int EthernetClient::available()
{
    SWS_SOCKET sock = (SWS_SOCKET)this->_sock;
    SWS_u_long nRead;

    if (sock == SWS_INVALID_SOCKET)
        return 0;
    if (SWS_ioctl(sock, SWS_FIONREAD, &nRead) < 0)
        return 0;

    return nRead;
}

int EthernetClient::read()
{
    uint8_t val;

    if (read(&val, 1) == 0)
        return -1;

    return val;
}

int EthernetClient::read(uint8_t *buf, size_t size)
{
    SWS_SOCKET sock = (SWS_SOCKET)this->_sock;
    int rc;

    if (sock == SWS_INVALID_SOCKET)
        return 0;
    if (size == 0)
        return 0;

    rc = SWS_recv(sock, buf, size, 0);

    return !!(rc > 0) * (rc);
}

int EthernetClient::peek()
{
    SWS_SOCKET sock = (SWS_SOCKET)this->_sock;
    uint8_t val;

    if (sock == SWS_INVALID_SOCKET)
        return -1;
    if (SWS_recv(sock, &val, sizeof(val), SWS_MSG_PEEK) <= 0)
        return -1;

    return val;
}

void EthernetClient::flush()
{
    SWS_SOCKET sock = (SWS_SOCKET)this->_sock;
    uint8_t val;

    if (sock == SWS_INVALID_SOCKET)
        return;

    while (SWS_recv(sock, &val, 1, 0) > 0);
}

void EthernetClient::stop()
{
    SWS_SOCKET sock = (SWS_SOCKET)this->_sock;

    if (sock == SWS_INVALID_SOCKET)
        return;

    // flush();

    SWS_shutdown(sock, SWS_SD_BOTH);
    SWS_close(sock);
    this->_sock = SWS_INVALID_SOCKET;

    if (_server)
        _server->closeServerSocket(_serverIndex);
}

uint8_t EthernetClient::connected()
{
    SWS_SOCKET sock = (SWS_SOCKET)this->_sock;

    if (sock == SWS_INVALID_SOCKET)
        return 0;

    return !!(SWS_recv(sock, NULL, 0, 0) || available());
}

uint8_t EthernetClient::status()
{
    return !!(this->_sock != SWS_INVALID_SOCKET);
}

// the next function allows us to use the client returned by
// EthernetServer::available() as the condition in an if-statement.

EthernetClient::operator bool()
{
    return (status() != 0);
}

bool EthernetClient::operator==(const EthernetClient &rhs)
{
    SWS_SOCKET sock1 = (SWS_SOCKET)this->_sock;
    SWS_SOCKET sock2 = (SWS_SOCKET)rhs._sock;

    if (sock1 != sock2)
        return false;
    if (sock1 == SWS_INVALID_SOCKET)
        return false;
    if (sock2 == SWS_INVALID_SOCKET)
        return false;

    return true;
}

IPAddress EthernetClient::remoteIP()
{
    return this->_remoteIP;
}

uint16_t EthernetClient::remotePort()
{
    return this->_remotePort;
}

uint16_t EthernetClient::localPort()
{
    SWS_SOCKET sock = (SWS_SOCKET)this->_sock;
    SWS_sockaddr_in sin;
    SWS_socklen_t len = sizeof(sin);

    if (sock == SWS_INVALID_SOCKET)
        return 0;
    if (SWS_getsockname(sock, (SWS_sockaddr *)&sin, &len) < 0)
        return 0;

    return SWS_ntohs(sin.sin_port);
}

#endif
