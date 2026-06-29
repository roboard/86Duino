#ifndef ethernetclient_h
#define ethernetclient_h

#include "Client.h"
#include "IPAddress.h"

class EthernetServer;
class EthernetClient : public Client
{
public:
    EthernetClient();
    virtual ~EthernetClient();

#if defined(__86DUINO_DUO)
    uint8_t _id_transmit;
#endif

#if defined(ETHERNET_REMOTE)
    EthernetClient(int id);
#endif

    uint8_t status();
    virtual int connect(IPAddress ip, uint16_t port);
    virtual int connect(const char *host, uint16_t port);
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buf, size_t size);
    virtual int writeNB(uint8_t);
    virtual int writeNB(const uint8_t *buf, size_t size);
    virtual int available();
    virtual int read();
    virtual int read(uint8_t *buf, size_t size);
    virtual int peek();
    virtual void flush();
    virtual void stop();
    virtual uint8_t connected();
    virtual operator bool();
    virtual bool operator==(const bool value) { return bool() == value; }
    virtual bool operator!=(const bool value) { return bool() != value; }
    virtual bool operator==(const EthernetClient &);
    virtual bool operator!=(const EthernetClient &rhs) { return !this->operator==(rhs); }
    virtual uint16_t localPort();
    virtual IPAddress remoteIP();
    virtual uint16_t remotePort();
    virtual void setConnectionTimeout(uint16_t timeout) { _connectionTimeout = timeout; }

    friend class EthernetServer;
    using Print::write;

private:
    EthernetClient(void *data);
    EthernetServer *_server;
    int _serverIndex;
    void *_sock;
    IPAddress _remoteIP;
    uint16_t _remotePort;
    uint16_t _connectionTimeout;

};

#endif
