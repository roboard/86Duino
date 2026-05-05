#ifndef ethernetserver_h
#define ethernetserver_h

#include "Server.h"

#define MAX_SOCK_NUM (128)

class EthernetClass;
class EthernetServer : public Server
{
public:
    EthernetServer(uint16_t port);
    EthernetServer(uint16_t port, EthernetClass &ethernet);
    virtual ~EthernetServer();
    EthernetClient available();
    EthernetClient accept();
    virtual void begin();
    virtual void begin(bool bindAny);
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buf, size_t size);
    virtual operator bool();
    void stop();

    friend class EthernetClient;
    using Print::write;

private:
    void closeServerSocket(int idx);
    int getSocketNumber(int idx);
    EthernetClass *_ethernet;
    EthernetClient _clients[MAX_SOCK_NUM];
    uint16_t _localPort;
    void *_sock;

};

#endif
