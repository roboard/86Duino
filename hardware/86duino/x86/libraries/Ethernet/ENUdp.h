#ifndef ethernetudp_h
#define ethernetudp_h

#ifndef SWS_SOCK_COMPAT
#define SWS_SOCK_COMPAT
#endif
#include "sws_sock.h"

#include <Udp.h>

#define UDP_TX_PACKET_MAX_SIZE (512)
#define UDP_RX_PACKET_MAX_SIZE (512)

class EthernetUDP : public UDP {
private:
  SOCKET _sock;
  uint16_t _port; // local port to listen on
  IPAddress _remoteIP; // remote IP address for the incoming packet whilst it's being processed
  uint16_t _remotePort; // remote port for the incoming packet whilst it's being processed
  struct sockaddr_in txaddr;
  struct sockaddr_in rxaddr;
  uint8_t TxBuffer[UDP_TX_PACKET_MAX_SIZE];
  uint8_t RxBuffer[UDP_RX_PACKET_MAX_SIZE];
  int TxSize;
  int RxSize;
  int RxHead;
  int RxTail;

public:
  EthernetUDP();  // Constructor
  virtual uint8_t begin(uint16_t);	// initialize, start listening on specified port. Returns 1 if successful, 0 if there are no sockets available to use
  virtual void stop();  // Finish with the UDP socket

  // Sending UDP packets
  
  // Start building up a packet to send to the remote host specific in ip and port
  // Returns 1 if successful, 0 if there was a problem with the supplied IP address or port
  virtual int beginPacket(IPAddress ip, uint16_t port);
  // Start building up a packet to send to the remote host specific in host and port
  // Returns 1 if successful, 0 if there was a problem resolving the hostname or port
  virtual int beginPacket(const char *host, uint16_t port);
  // Finish off this packet and send it
  // Returns 1 if the packet was sent successfully, 0 if there was an error
  virtual int endPacket();
  // Write a single byte into the packet
  virtual size_t write(uint8_t);
  // Write size bytes from buffer into the packet
  virtual size_t write(const uint8_t *buffer, size_t size);
  
  using Print::write;

  // Start processing the next available incoming packet
  // Returns the size of the packet in bytes, or 0 if no packets are available
  virtual int parsePacket();
  // Number of bytes remaining in the current packet
  virtual int available();
  // Read a single byte from the current packet
  virtual int read();
  // Read up to len bytes from the current packet and place them into buffer
  // Returns the number of bytes read, or 0 if none are available
  virtual int read(unsigned char* buffer, size_t len);
  // Read up to len characters from the current packet and place them into buffer
  // Returns the number of characters read, or 0 if none are available
  virtual int read(char* buffer, size_t len) { return read((unsigned char*)buffer, len); };
  // Return the next byte from the current packet without moving on to the next byte
  virtual int peek();
  virtual void flush();	// Finish reading the current packet

  // Return the IP address of the host who sent the current incoming packet
  virtual IPAddress remoteIP() { return _remoteIP; };
  // Return the port of the host who sent the current incoming packet
  virtual uint16_t remotePort() { return _remotePort; };
};

#endif
