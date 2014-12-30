#ifndef ethernetclient_h
#define ethernetclient_h

#include "Arduino.h"	
#include "Print.h"
#include "Client.h"
#include "IPAddress.h"

#define BUFFER_SIZE (512)

class EthernetClient : public Client
{
public:
  EthernetClient();

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

  friend class EthernetServer;
  
  using Print::write;
  
  class EthernetServer *pServer;
  
private:
  struct SwsSockInfo *sws;
  EthernetClient(struct SwsSockInfo *info);
  
  int _id;
  
  uint8_t _RegBuf;
  uint8_t _RegLen;
};

#endif
