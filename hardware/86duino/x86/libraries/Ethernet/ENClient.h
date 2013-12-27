#ifndef ethernetclient_h
#define ethernetclient_h

#ifndef SWS_SOCK_COMPAT
#define SWS_SOCK_COMPAT
#endif
#include "sws_sock.h"

#include "Arduino.h"	
#include "Print.h"
#include "Client.h"
#include "IPAddres.h"

#define BUFFER_SIZE (512)

class EthernetClient : public Client {

public:
  EthernetClient();
  EthernetClient(SOCKET sock);

  uint8_t status();
  virtual int connect(IPAddress ip, uint16_t port);
  virtual int connect(const char *host, uint16_t port);
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
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
  
  SOCKET _sock;
  int _id;
  class EthernetServer *pServer;
  
  uint8_t _RegBuf;
  uint8_t _RegLen;
};

#endif
