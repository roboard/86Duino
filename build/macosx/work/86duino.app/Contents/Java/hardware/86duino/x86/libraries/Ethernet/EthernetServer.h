#ifndef ethernetserver_h
#define ethernetserver_h

#include "Server.h"
#include "EthernetClient.h"

#if defined(ETHERNET_REMOTE)
#include "vdpmem.h"
#endif

#define MAX_SOCK_NUM (128)

class EthernetServer : public Server
{
public:
    EthernetServer(uint16_t port = 80);
    virtual ~EthernetServer();
    EthernetClient available();
    EthernetClient accept();
    virtual void begin();
    virtual void begin(uint16_t port);
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buf, size_t size);
    virtual operator bool();

    friend class EthernetClient;
    using Print::write;

private:
    void stop();
    void closeServerSocket(int idx);
    EthernetClient _clients[MAX_SOCK_NUM];
    uint16_t _localPort;
    void *_sock;

};

#endif
