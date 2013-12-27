#ifndef ethernetserver_h
#define ethernetserver_h

#ifndef SWS_SOCK_COMPAT
#define SWS_SOCK_COMPAT
#endif
#include "sws_sock.h"

#include "Server.h"
#include "Ethernet.h"

class EthernetClient;

class EthernetServer : public Server {

private:
	SOCKET _sock;
	SOCKET make_new_client(SOCKET _sock);
	
	uint16_t _port;
	fd_set rfds;
	
	EthernetClient Clients[MAX_SOCK_NUM];
	
public:
	EthernetServer(uint16_t);
	EthernetClient available();
	virtual void begin();
	virtual size_t write(uint8_t);
	virtual size_t write(const uint8_t *buf, size_t size);
	using Print::write;
	
	void closeServerSocket(int idx);
};

#endif
