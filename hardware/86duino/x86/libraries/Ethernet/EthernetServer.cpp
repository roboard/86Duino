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

#include "SwsSock.h"
#include "Ethernet.h"
#include "sws_sock.h"

EthernetServer::EthernetServer(uint16_t port)
{
    this->_ethernet = &Ethernet;
    this->_localPort = port;
    this->_sock = SWS_INVALID_SOCKET;
}

EthernetServer::EthernetServer(uint16_t port, EthernetClass &ethernet)
{
    this->_ethernet = &ethernet;
    this->_localPort = port;
    this->_sock = SWS_INVALID_SOCKET;
}

EthernetServer::~EthernetServer()
{
    stop();
}

void EthernetServer::begin()
{
    begin(false);
}

void EthernetServer::begin(bool bindAny)
{
    SWS_SOCKET sock;
    SWS_sockaddr_in sin;
    SWS_u_long lArg = 1;
    int yes = 1;

    if (this->_sock != SWS_INVALID_SOCKET)
        return;
    if (this->_ethernet->isInitialized() == 0)
        return;

    if ((sock = SWS_socket(SWS_AF_INET, SWS_SOCK_STREAM, 0)) == SWS_INVALID_SOCKET)
        return;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = SWS_AF_INET;
    sin.sin_addr.SWS_s_addr = (bindAny) ? SWS_INADDR_ANY : (uint32_t)this->_ethernet->localIP();
    sin.sin_port = SWS_htons(this->_localPort);
    if (SWS_bind(sock, (struct SWS_sockaddr *)&sin, sizeof(sin)) != 0)
    {
        SWS_shutdown(sock, SWS_SD_BOTH);
        SWS_close(sock);
        return;
    }

    if (SWS_listen(sock, MAX_SOCK_NUM) != 0)
    {
        SWS_shutdown(sock, SWS_SD_BOTH);
        SWS_close(sock);
        return;
    }

    if (SWS_ioctl(sock, SWS_FIONBIO, &lArg) != 0)
    {
        SWS_shutdown(sock, SWS_SD_BOTH);
        SWS_close(sock);
        return;
    }

    if (SWS_setsockopt(sock, SWS_SOL_SOCKET, SWS_SO_REUSEADDR, (const char *)&yes, sizeof(int)) < 0)
    {
        SWS_shutdown(sock, SWS_SD_BOTH);
        SWS_close(sock);
        return;
    }

    this->_sock = sock;
}

void EthernetServer::stop()
{
    SWS_SOCKET sock = (SWS_SOCKET)this->_sock;

    if (sock == SWS_INVALID_SOCKET)
        return;

    for (int i = 0; i < MAX_SOCK_NUM; i++)
    {
        if (this->_clients[i].status())
            this->_clients[i].stop();
    }
    SWS_shutdown(sock, SWS_SD_BOTH);
    SWS_close(sock);
    this->_sock = SWS_INVALID_SOCKET;
}

EthernetClient EthernetServer::accept()
{
    SWS_SOCKET sock = (SWS_SOCKET)this->_sock;
    SWS_SOCKET newsock;
    SWS_sockaddr_in sockaddr;
    SWS_socklen_t socklen = sizeof(sockaddr);

    if (sock == SWS_INVALID_SOCKET)
        return EthernetClient();

    if ((newsock = SWS_accept(sock, (SWS_sockaddr *)&sockaddr, &socklen)) != SWS_INVALID_SOCKET)
    {
        SWS_u_long non_block = 1;
        int yes = 1;

        if (SWS_ioctl(newsock, SWS_FIONBIO, &non_block) != 0 ||
            SWS_setsockopt(newsock, SWS_SOL_SOCKET, SWS_SO_REUSEADDR, (const char *)&yes, sizeof(int)) < 0)
        {
            SWS_shutdown(newsock, SWS_SD_BOTH);
            SWS_close(newsock);
            newsock = NULL;
        }
    }

    if (newsock != SWS_INVALID_SOCKET)
    {
        int idx;

        for (idx = 0; idx < MAX_SOCK_NUM; idx++)
        {
            if (this->_clients[idx]._sock == SWS_INVALID_SOCKET)
                break;
        }

        if (idx >= MAX_SOCK_NUM)
        {
            SWS_shutdown(newsock, SWS_SD_BOTH);
            SWS_close(newsock);
        }
        else
        {
            this->_clients[idx]._ethernet = this->_ethernet;
            this->_clients[idx]._serverIndex = idx;
            this->_clients[idx]._server = this;
            this->_clients[idx]._sock = newsock;
            return this->_clients[idx];
        }
    }

    return EthernetClient();
}

EthernetClient EthernetServer::available()
{
    SWS_SOCKET sock = (SWS_SOCKET)this->_sock;
    SWS_SOCKET newsock;
    SWS_sockaddr_in sockaddr;
    SWS_socklen_t socklen = sizeof(sockaddr);

    if (sock == SWS_INVALID_SOCKET)
        return EthernetClient();

    if ((newsock = SWS_accept(sock, (SWS_sockaddr *)&sockaddr, &socklen)) != SWS_INVALID_SOCKET)
    {
        SWS_u_long non_block = 1;
        int yes = 1;

        if (SWS_ioctl(newsock, SWS_FIONBIO, &non_block) != 0 ||
            SWS_setsockopt(newsock, SWS_SOL_SOCKET, SWS_SO_REUSEADDR, (const char *)&yes, sizeof(int)) < 0)
        {
            SWS_shutdown(newsock, SWS_SD_BOTH);
            SWS_close(newsock);
            newsock = NULL;
        }
    }

    if (newsock != SWS_INVALID_SOCKET)
    {
        int idx;

        for (idx = 0; idx < MAX_SOCK_NUM; idx++)
        {
            if (this->_clients[idx]._sock == SWS_INVALID_SOCKET)
                break;
        }

        if (idx >= MAX_SOCK_NUM)
        {
            SWS_shutdown(newsock, SWS_SD_BOTH);
            SWS_close(newsock);
        }
        else
        {
            this->_clients[idx]._ethernet = this->_ethernet;
            this->_clients[idx]._serverIndex = idx;
            this->_clients[idx]._server = this;
            this->_clients[idx]._sock = newsock;
            return this->_clients[idx];
        }
    }

    for (int i = 0; i < MAX_SOCK_NUM; i++)
    {
        if (this->_clients[i]._sock == SWS_INVALID_SOCKET)
            continue;
        if (this->_clients[i].available() > 0)
            return this->_clients[i];
        if (this->_clients[i].connected() == 0)
            this->_clients[i].stop();
    }

    return EthernetClient();
}

size_t EthernetServer::write(uint8_t b)
{
    return write(&b, 1);
}

size_t EthernetServer::write(const uint8_t *buffer, size_t size)
{
    size_t n = 0;

    for (int i = 0; i < MAX_SOCK_NUM; i++)
    {
        if (this->_clients[i].status())
        {
            n += this->_clients[i].write(buffer, size);
        }
    }

    return n;
}

void EthernetServer::closeServerSocket(int idx)
{
    if (idx < 0 || idx >= MAX_SOCK_NUM)
        return;
    this->_clients[idx]._sock = SWS_INVALID_SOCKET;
}

int EthernetServer::getSocketNumber(int idx)
{
    if (idx < 0 || idx >= MAX_SOCK_NUM)
        return 0;
    return (int)this->_clients[idx]._sock;
}

EthernetServer::operator bool()
{
    return this->_sock != SWS_INVALID_SOCKET;
}
