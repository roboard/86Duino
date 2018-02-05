/*
  Adapted from sEthernetStreamW5100.h - Streaming Ethernet Arduino library for W5100-based ethernet shield
  Copyright (c) 2012 Anton Smirnov (dev@antonsmirnov.name)
  https://github.com/4ntoine/ArduinoCommander-ethernet/blob/master/Ethernet/EthernetStreamW5100.h
*/
#ifndef ETHERNET_STREAM_H
#define ETHERNET_STREAM_H

#include <inttypes.h>
#include <Stream.h>
#include <SPI.h>
#include <Ethernet.h>


class EthernetStream : public Stream
{
  private:
    EthernetServer *server;
    EthernetClient client;
    
    int connect_client();
    IPAddress getLocalIP();
    IPAddress broadcastIP;
    
    unsigned long broadcastStartTime;
    unsigned long broadcastInterval;
    uint8_t broadcastPkg[512];
    char* projectName;
    unsigned int broadcastPkgSize;
    bool makeBroadcastPkg(uint8_t* pkg, int pkgSize, char* projectName);
    bool pkgOK;

  public:       
    EthernetStream();
    
    // begin with dynamic IP:port (DHCP)
    // Returns 0 if the DHCP configuration failed, and 1 if it succeeded
    int begin(char* prjName, uint8_t *mac_address, uint16_t port);

    // begin with static IP:port
    void begin(char* prjName, uint8_t *mac_address, IPAddress local_ip, IPAddress broadcastIP, uint16_t port);

    // get DCHP IP
    IPAddress localIP();
    
    // get Gateway IP
    IPAddress gatewayIP();
    
    // get Subnetmask IP
    IPAddress subnetMask();

    // overriden Stream class functions
    virtual size_t write(uint8_t byte);
    virtual int read();
    virtual int available();
    virtual void flush();
    virtual int peek();   
};

#endif
