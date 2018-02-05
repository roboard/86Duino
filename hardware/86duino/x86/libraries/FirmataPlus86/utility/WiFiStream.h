/*
  WiFiStream.h
  An Arduino Stream that wraps an instance of a WiFi server. For use
  with legacy Arduino WiFi shield and other boards and sheilds that
  are compatible with the Arduino WiFi library.

  Copyright (C) 2015-2016 Jesse Frush. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.
 */

#ifndef WIFI_STREAM_H
#define WIFI_STREAM_H

#include <inttypes.h>
#include <Stream.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#define nullptr NULL

class WiFiStream : public Stream
{
private:
  WiFiServer _server = WiFiServer(23);
  WiFiClient _client;

  //configuration members
  IPAddress _local_ip;
  uint16_t _port = 0;
  uint8_t _key_idx = 0;               //WEP
  const char *_key = nullptr;         //WEP
  const char *_passphrase = nullptr;  //WPA
  char *_ssid = nullptr;

  int connect_client();
  bool is_ready();
  IPAddress getLocalIP();

  unsigned long broadcastStartTime;
  unsigned long broadcastInterval;
  uint8_t broadcastPkg[512];
  char* projectName;
  unsigned int broadcastPkgSize;
  bool makeBroadcastPkg(uint8_t* pkg, int pkgSize, char* projectName);
  bool pkgOK;
  
public:
  WiFiStream() {};

  // allows another way to configure a static IP before begin is called
  void config(IPAddress local_ip);

  // get DCHP IP
  IPAddress localIP();
  
  // get Gateway IP
  IPAddress gatewayIP();
  
  // get DCHP IP
  IPAddress subnetMask();


  bool maintain();

/******************************************************************************
 *           Connection functions with DHCP
 ******************************************************************************/

  //OPEN networks
  int begin(char* prjname, char *ssid, uint16_t port);

  //WEP-encrypted networks
  int begin(char* prjname, char *ssid, uint8_t key_idx, const char *key, uint16_t port);

  //WPA-encrypted networks
  int begin(char* prjname, char *ssid, const char *passphrase, uint16_t port);

/******************************************************************************
 *           Connection functions without DHCP
 ******************************************************************************/

  //OPEN networks with static IP
  int begin(char* prjname, char *ssid, IPAddress local_ip, uint16_t port);

  //WEP-encrypted networks with static IP
  int begin(char* prjname, char *ssid, IPAddress local_ip, uint8_t key_idx, const char *key, uint16_t port);

  //WPA-encrypted networks with static IP
  int begin(char* prjname, char *ssid, IPAddress local_ip, const char *passphrase, uint16_t port);
  

/******************************************************************************
 *             Stream implementations
 ******************************************************************************/
  int available();
  void flush();
  int peek();
  int read();
  void stop();
  size_t write(uint8_t byte);
  
};

#endif //WIFI_STREAM_H
