/*
  ESP8266Stream.cpp
 */
 
#include "ESP8266Stream.h"
#include <Arduino.h>

//#define SERIAL_DEBUG
#include "firmataDebug.h"

#define MILLIS_RECONNECT 10000

ESP8266Stream::ESP8266Stream()
: wifi()
{
  used_mux_id = 0;
}

void
ESP8266Stream::init(char* prjname, uint16_t port, HardwareSerial &uart, uint32_t baud, uint8_t ch_pd, char *ssid, char *key)
{
  char str_mode[10];
  wifi.init(uart, baud, ch_pd);
  wifi.setOprToStation(str_mode, 10);
  wifi.enableMUX();
  if (ssid != NULL && key != NULL)
    wifi.joinAP(ssid, key);
  wifi.startTCPServer(2000);
  wifi.disableEcho();
  timer_Delay(100);
  wifi.getGatewayAndSubnetMask(_gatewayIP, 4, _subnetMaskIP, 4);
  puart = &uart;
  wifi.registerUDP(used_mux_id+1, "255.255.255.255", 8686); // broadcast
  broadcastInterval = 4000;
  broadcastStartTime = millis();
  projectName = prjname;
  pkgOK = makeBroadcastPkg(broadcastPkg, sizeof(broadcastPkg), projectName);
}

void
ESP8266Stream::initAP(char* prjname, uint16_t port, HardwareSerial &uart, uint32_t baud, uint8_t ch_pd, char *ssid, char *key, uint8_t chl, uint8_t ecn)
{
  char str_mode[10];
  wifi.init(uart, baud, ch_pd);
  wifi.setOprToSoftAP(str_mode, 10);
  if (ssid != NULL && key != NULL)
    wifi.setSoftAPParam(ssid, key, chl, ecn);
  wifi.enableMUX();
  wifi.startTCPServer(2000);
  wifi.disableEcho();
  timer_Delay(100);
  wifi.getGatewayAndSubnetMask(_gatewayIP, 4, _subnetMaskIP, 4);
  puart = &uart;
  wifi.registerUDP(used_mux_id+1, "255.255.255.255", 8686); // broadcast
  broadcastInterval = 4000;
  broadcastStartTime = millis();
  projectName = prjname;
  pkgOK = makeBroadcastPkg(broadcastPkg, sizeof(broadcastPkg), projectName);
}

bool
ESP8266Stream::makeBroadcastPkg(uint8_t* pkg, int pkgSize, char* projectName)
{
    int i, j;
    
    pkg[0] = 0x86; pkg[1] = 0x88;
    pkg[2] = 0x88; pkg[3] = 0x86;
    
    if (wifi.getLocalIP(_localIP, 4) == true)
    {
        pkg[4] = _localIP[0]; pkg[5] = _localIP[1];
        pkg[6] = _localIP[2]; pkg[7] = _localIP[3];
    }
    else
        return false;
    
    if (projectName == NULL || projectName[0] == '\0') return false;
    
    for (i=0; projectName[i] != '\0'; i++);
    pkg[8] = i; // Name size
    
    for (i=9, j=0; projectName[j] != '\0'; i++, j++)
        pkg[i] = projectName[j];
    
    broadcastPkgSize = i;
    
    return true;
}

int
ESP8266Stream::available()
{
  int len = wifi.recv_nb(&used_mux_id, recvBuf, (uint32_t)128);
  for(int i = 0; i < len; i++) {
    //printf("%d ", recvBuf[i]);
    wifi.wifi_data.push(recvBuf[i]);
  }
  
  if (wifi.clientConnected == false)
  {
    if (pkgOK == true)
    {
      if ((millis() - broadcastStartTime) > broadcastInterval)
      {
        wifi.send(used_mux_id+1, (const uint8_t*)broadcastPkg, (uint32_t)broadcastPkgSize);
        broadcastStartTime = millis();
      }
    }
    else
      pkgOK = makeBroadcastPkg(broadcastPkg, sizeof(broadcastPkg), projectName);
  }
  
  return wifi.wifi_data.size();
}

int
ESP8266Stream::read()
{
  int len = wifi.recv_nb(&used_mux_id, recvBuf, (uint32_t)128);
  for(int i = 0; i < len; i++) {
    //printf("%d ", recvBuf[i]);
    wifi.wifi_data.push(recvBuf[i]);
  }
  if( !wifi.wifi_data.empty() )
  {
    int ch = wifi.wifi_data.front();
    wifi.wifi_data.pop();
    return ch;
  }
  return -1;
}

int
ESP8266Stream::peek()
{
  int len = wifi.recv_nb(&used_mux_id, recvBuf, (uint32_t)128);
  for(int i = 0; i < len; i++) {
    //printf("%d ", recvBuf[i]);
    wifi.wifi_data.push(recvBuf[i]);
  }
  if( !wifi.wifi_data.empty() )
    return wifi.wifi_data.front();
  return -1;
}

size_t
ESP8266Stream::write(uint8_t c)
{
  int retry = 3; 
  bool result = false;
  
  for (;retry > 0; retry--)
  {
    result = wifi.send(used_mux_id, &c, 1);
    if (result == true)
      return 1;
  }
  return 0;
}

size_t
ESP8266Stream::write(const uint8_t* c, size_t size)
{
  if (wifi.send(used_mux_id, c, (int)size) == true)
    return size;
  return 0;
}

int*
ESP8266Stream::localIP()
{
    return _localIP;
}

int*
ESP8266Stream::gatewayIP()
{
    return _gatewayIP;
}

int*
ESP8266Stream::subnetMask()
{
    return _subnetMaskIP;
}
