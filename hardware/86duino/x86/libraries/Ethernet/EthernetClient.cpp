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
#include "sws_sock.h"

EthernetClient::EthernetClient(const EthernetClient &other)
{
    this->_ethernet = other._ethernet;
    this->_connectionTimeout = other._connectionTimeout;
    this->_sock = other._sock;
    this->_remoteIP = other._remoteIP;
    this->_remotePort = other._remotePort;
    this->_server = other._server;
    this->_serverIndex = other._serverIndex;
    this->_isCopy = true;
}

EthernetClient::EthernetClient()
{
    this->_ethernet = &Ethernet;
    this->_connectionTimeout = 5000;
    this->_sock = SWS_INVALID_SOCKET;
    this->_remoteIP = (uint32_t)0;
    this->_remotePort = 0;
    this->_server = NULL;
    this->_serverIndex = 0;
    this->_isCopy = false;
}

EthernetClient::EthernetClient(EthernetClass &ethernet)
{
    this->_ethernet = &ethernet;
    this->_connectionTimeout = 5000;
    this->_sock = SWS_INVALID_SOCKET;
    this->_remoteIP = (uint32_t)0;
    this->_remotePort = 0;
    this->_server = NULL;
    this->_serverIndex = 0;
    this->_isCopy = false;
}

EthernetClient::~EthernetClient()
{
    if (this->_isCopy == false)
        stop();
    this->_isCopy = false;
}

int EthernetClient::connect(const char *host, uint16_t port)
{
    DNSClient dns;
    IPAddress remote_addr;
    int rc;

    dns.begin(this->_ethernet->dnsServerIP());
    if ((rc = dns.getHostByName(host, remote_addr)) != 1)
        return rc;

    return connect(remote_addr, port);
}

int EthernetClient::connect(IPAddress ip, uint16_t port)
{
    SWS_SOCKET sock;
    SWS_sockaddr_in sin;
    SWS_sockaddr_in pin;
    SWS_timeval seltime;
    SWS_fd_set fds;
    SWS_u_long lArg = 1;
    int yes = 1;

    if (this->_sock != SWS_INVALID_SOCKET)
        stop();
    if (this->_ethernet->isInitialized() == 0)
        return 0;

    if ((sock = SWS_socket(SWS_AF_INET, SWS_SOCK_STREAM, 0)) == SWS_INVALID_SOCKET)
        return 0;

    if (SWS_setsockopt(sock, SWS_SOL_SOCKET, SWS_SO_REUSEADDR, (const char *)&yes, sizeof(int)) < 0)
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

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = SWS_AF_INET;
    sin.sin_addr.SWS_s_addr = (uint32_t)this->_ethernet->localIP();
    sin.sin_port = 0;
    if (SWS_bind(sock, (struct SWS_sockaddr *)&sin, sizeof(sin)) != 0)
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
    seltime.tv_sec = this->_connectionTimeout / 1000;
    seltime.tv_usec = (this->_connectionTimeout % 1000) * 1000;
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
    int nRead;

    if (sock == SWS_INVALID_SOCKET)
        return 0;

    nRead = SWS_recv(sock, NULL, 0, SWS_MSG_PEEK);

    return nRead * !!(nRead > 0);
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

    SWS_shutdown(sock, SWS_SD_BOTH);
    SWS_close(sock);
    this->_sock = SWS_INVALID_SOCKET;
    this->_remoteIP = (uint32_t)0;
    this->_remotePort = 0;

    if (this->_server)
        this->_server->closeServerSocket(this->_serverIndex);
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

int EthernetClient::getSocketNumber()
{
    return (this->_server == NULL) ? ((int)this->_sock) : ((int)this->_server->getSocketNumber(this->_serverIndex));
}
