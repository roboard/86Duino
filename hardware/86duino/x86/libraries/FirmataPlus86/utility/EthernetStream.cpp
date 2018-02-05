/*
  EthernetStream.cpp - Streaming Ethernet Arduino library 
  Copyright (c) 2012 Anton Smirnov (dev@antonsmirnov.name)
  Changes
            (c) 2014 Peter Schwarz (peter.schwarz@bicycle.io)
*/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include "EthernetStream.h"


/******************************************************************************
 * Definitions
 ******************************************************************************/

EthernetUDP Udp;
unsigned int localPort = 8888;      // local port to listen on
bool ethernetActive = false;

EthernetStream::EthernetStream()
{

}

IPAddress EthernetStream::getLocalIP()
{
    return Ethernet.localIP();
}

bool EthernetStream::makeBroadcastPkg(uint8_t* pkg, int pkgSize, char* projectName)
{
    int i, j;
    IPAddress ip;
    pkg[0] = 0x86; pkg[1] = 0x88;
    pkg[2] = 0x88; pkg[3] = 0x86;
    
    ip = getLocalIP();

    pkg[4] = ip[0]; pkg[5] = ip[1];
    pkg[6] = ip[2]; pkg[7] = ip[3];

    
    if (projectName == NULL || projectName[0] == '\0') return false;
    
    for (i=0; projectName[i] != '\0'; i++);
    pkg[8] = i; // Name size
    
    for (i=9, j=0; projectName[j] != '\0'; i++, j++)
        pkg[i] = projectName[j];
    
    broadcastPkgSize = i;
    
    return true;
}

int EthernetStream::begin(char* prjname, uint8_t *mac_address, uint16_t port)
{
    int i;
    unsigned char section[4];
    
    // start DHCP and check result
    int result = Ethernet.begin(mac_address);
    if (result == 0)
        return 0;

    ethernetActive = true;
    server = new EthernetServer(port);
    server->begin();
    
    IPAddress subnet = Ethernet._dhcp->getSubnetMask();
    IPAddress dnsserver = Ethernet._dhcp->getDnsServerIp();
    IPAddress _ipb;
    for (i=0; i<4; i++) section[i] = 255 - subnet[i];
    for (i=0; i<4; i++) _ipb[i] = (section[i] == 0) ? (dnsserver[i]) : section[i];
    // Serial.print("Subnet IP: ");
    // Serial.println(subnet);
    // Serial.print("DNSserver IP: ");
    // Serial.println(dnsserver);
    // Serial.print("Broadcast IP: ");
    // Serial.println(_ipb);
    
    broadcastIP = _ipb;
    
    Udp.begin(localPort);
    broadcastInterval = 4000;
    broadcastStartTime = millis();
    projectName = prjname;
    pkgOK = makeBroadcastPkg(broadcastPkg, sizeof(broadcastPkg), projectName);
    return result;  
}

void EthernetStream::begin(char* prjname, uint8_t *mac_address, IPAddress local_ip, IPAddress _ipb, uint16_t port)
{
    Ethernet.begin(mac_address, local_ip);

    ethernetActive = true;
    server = new EthernetServer(port);
    server->begin();
    
    broadcastIP = _ipb;
    
    Udp.begin(localPort);
    broadcastInterval = 4000;
    broadcastStartTime = millis();
    projectName = prjname;
    pkgOK = makeBroadcastPkg(broadcastPkg, sizeof(broadcastPkg), projectName);
}

IPAddress EthernetStream::localIP()
{
    return Ethernet.localIP();
}

IPAddress EthernetStream::gatewayIP()
{
    return Ethernet.gatewayIP();
}

IPAddress EthernetStream::subnetMask()
{
    return Ethernet.subnetMask();
}

int EthernetStream::connect_client() 
{
    if (ethernetActive == false) return 0;
    
    EthernetClient newClient = server->available();
    if (!newClient)
        return 0;

    client = newClient;
    return 1;
}

int EthernetStream::available()
{
    if (ethernetActive == false) return 0;
    
    if (connect_client() == 0)
    {
        if (pkgOK == true && ((millis() - broadcastStartTime) > broadcastInterval))
        {
            printf("%s\n", broadcastPkg);
            Udp.beginPacket(broadcastIP, 8686);
            Udp.write(broadcastPkg, broadcastPkgSize);
            Udp.endPacket();
            broadcastStartTime = millis();
        }
        else
            pkgOK = makeBroadcastPkg(broadcastPkg, sizeof(broadcastPkg), projectName);
        return 0;
    }
    return client.available();
}

int EthernetStream::read()
{
    return client ? client.read() : 0;
}

void EthernetStream::flush() {
    if (client) client.flush();
}

int EthernetStream::peek() {
    return client ? client.peek(): 0;
}

size_t EthernetStream::write(uint8_t outcomingByte) {
    if(client) client.write(outcomingByte);
}
