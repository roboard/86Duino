/*
  ESP8266Stream.h
 */

#ifndef ESP8266STREAM_H
#define ESP8266STREAM_H

#include <inttypes.h>
#include <stdio.h>
#include <Stream.h>
#include "ESP82664fp.h"

class ESP8266Stream : public Stream
{
public:
  std::queue<unsigned char> wifi_data;
  ESP8266Stream();
  void init(char* prjname, uint16_t port, HardwareSerial &uart, uint32_t baud, uint8_t ch_pd, char *ssid = NULL, char *key = NULL);
  int available();
  int read();
  int peek();
  size_t write(uint8_t);
  size_t write(const uint8_t* c, size_t size);

private:
  ESP82664fp wifi;
  HardwareSerial *puart;
  uint8_t recvBuf[128];
  uint8_t used_mux_id;
  int localIP[4];
  uint8_t broadcastPkg[512];
  int broadcastPkgSize;
  char* projectName;
  unsigned long broadcastStartTime;
  unsigned long broadcastInterval;
  bool makeBroadcastPkg(uint8_t* pkg, int pkgSize, char* projectName);
  bool pkgOK;
};

#endif
