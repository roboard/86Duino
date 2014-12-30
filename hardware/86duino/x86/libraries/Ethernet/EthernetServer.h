#ifndef ethernetserver_h
#define ethernetserver_h

#include "Server.h"
#include "Ethernet.h"

class EthernetClient;

class EthernetServer : public Server {

private:
	struct SwsSockInfo *sws;
	struct SwsSockInfo *make_new_client(struct SwsSockInfo *info);
	
	uint16_t _port;
	
	EthernetClient Clients[MAX_SOCK_NUM];
	
public:
	EthernetServer(uint16_t);
	~EthernetServer();
	EthernetClient available();
	virtual void begin();
	virtual size_t write(uint8_t);
	virtual size_t write(const uint8_t *buf, size_t size);
	using Print::write;
	
	void closeServerSocket(int idx);
};

#endif
