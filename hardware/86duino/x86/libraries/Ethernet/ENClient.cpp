extern "C" {
  #include "string.h"
}

#include "Arduino.h"

#include "Ethernet.h"
#include "ENClient.h"
#include "ENServer.h"
#include "Dns.h"

static int swssock_connect(SOCKET s, const sockaddr *addr, int size)
{
	int rc;
	fd_set fds;
	struct timeval seltime;
	
	connect(s, addr, size);

	FD_ZERO(&fds);
	FD_SET(s,&fds);

	seltime.tv_sec = 10;
	seltime.tv_usec = 0;

	if (select(0, NULL, &fds, NULL, &seltime) <= 0)
		return -1;

	return 0;
}

EthernetClient::EthernetClient() : _sock(INVALID_SOCKET)
{
	_id = -1;
	pServer = NULL;
}

EthernetClient::EthernetClient(SOCKET sock) : _sock(sock)
{
	_id = -1;
	pServer = NULL;
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
	struct sockaddr_in pin;
	int yes = 1;
	uint8_t *ipchar;
	u_long iplong, lArg = 1;
	struct linger linger;
	
	if (_sock != INVALID_SOCKET)
		stop();
	
	ipchar = rawIPAddress(ip);
	memcpy(&iplong, ipchar, 4);
	
	bzero(&pin, sizeof(pin));
    pin.sin_family = AF_INET;
    pin.sin_addr.s_addr = iplong;
    pin.sin_port = htons(port);
	
	_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (_sock == INVALID_SOCKET)
		return 0;
	
	if (setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(int)) < 0) {
		shutdown(_sock, SD_BOTH);
		closesocket(_sock);
		_sock = INVALID_SOCKET;
		return 0;
	}
	
	linger.l_onoff = 1;
	linger.l_linger = 1; 
	if (setsockopt(_sock, SOL_SOCKET, SO_LINGER, (const char*)&linger, sizeof(linger)) < 0) {
		shutdown(_sock, SD_BOTH);
		closesocket(_sock);
		return INVALID_SOCKET;
	}
	
	if (ioctlsocket(_sock, FIONBIO, &lArg) < 0) {
		shutdown(_sock, SD_BOTH);
		closesocket(_sock);
		_sock = INVALID_SOCKET;
		return 0;
	}

    if (swssock_connect(_sock, (const sockaddr*)&pin, sizeof(pin)) == -1) {
		shutdown(_sock, SD_BOTH);
		closesocket(_sock);
		_sock = INVALID_SOCKET;
		return 0;
    }
	
	_id = -1;
	pServer = NULL;
	
	return 1;
}

size_t EthernetClient::write(uint8_t b)
{
	return write(&b, 1);
}

size_t EthernetClient::write(const uint8_t *buf, size_t size)
{
	int rc;
	size_t n = 0;
	
	if (_sock == INVALID_SOCKET)
		return 0;
	
	rc = send(_sock, buf, size, 0);
	if (rc > 0) {
		n = rc;
	}
	else if (rc < 0)
		stop();
		
	return n;
}

int EthernetClient::available()
{
	int rc, cur = 0;
	uint8_t buf[BUFFER_SIZE];
	
	if (_RegLen > 0)
		cur = 1;
	
	if (_sock != INVALID_SOCKET) {
	
		rc = recv(_sock, buf, BUFFER_SIZE-cur, MSG_PEEK);
		
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
	
	if (_sock != INVALID_SOCKET && size > 0) {
		
		rc = recv(_sock, &buf[cur], size, 0);
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
	
	if (_sock == INVALID_SOCKET)
		return -1;
	
	if (recv(_sock, &val, 1, MSG_PEEK) <= 0)
		return -1;
	
	return val;
}

void EthernetClient::flush()
{
	uint8_t val;
	
	if (_sock == INVALID_SOCKET)
		return;
		
	while (recv(_sock, &val, 1, 0) > 0);
}

void EthernetClient::stop()
{
	flush();
	
	if (_sock != INVALID_SOCKET) {
		shutdown(_sock, SD_BOTH);
		closesocket(_sock);
		_sock = INVALID_SOCKET;
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
		
	if (_sock == INVALID_SOCKET)
		return 0;
	
	rc = recv(_sock, &val, 1, 0);
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
	return _sock != INVALID_SOCKET;
}

// the next function allows us to use the client returned by
// EthernetServer::available() as the condition in an if-statement.

EthernetClient::operator bool()
{
	return _sock != INVALID_SOCKET;
}
