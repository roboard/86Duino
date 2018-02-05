/*
 * Implementation is in WiFiStream.h to avoid linker issues. Legacy WiFi and modern WiFi101 both define WiFiClass which
 * will cause linker errors whenever Firmata.h is included.
 */

#include "WiFiStream.h"

WiFiUDP wUdp;

int WiFiStream::connect_client()
{
    if( !( _client && _client.connected() ) )
    {
        WiFiClient newClient = _server.available();
        if( !newClient )
        {
            return 0;
        }

        _client = newClient;
    }
    return 1;
}

bool WiFiStream::is_ready()
{
    uint8_t status = WiFi.status();
    return !( status == WL_NO_SHIELD || status == WL_CONNECTED );
}

IPAddress WiFiStream::getLocalIP()
{
    return WiFi.localIP();
}

bool WiFiStream::makeBroadcastPkg(uint8_t* pkg, int pkgSize, char* projectName)
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

// allows another way to configure a static IP before begin is called
void WiFiStream::config(IPAddress local_ip)
{
    _local_ip = local_ip;
    WiFi.config( local_ip );
}

// get DCHP IP
IPAddress WiFiStream::localIP()
{
    return WiFi.localIP();
}

IPAddress WiFiStream::gatewayIP()
{
    return WiFi.gatewayIP();
}

IPAddress WiFiStream::subnetMask()
{
    return WiFi.subnetMask();
}

bool WiFiStream::maintain()
{
    if( connect_client() ) return true;

    stop();
    int result = 0;
    if( WiFi.status() != WL_CONNECTED )
    {
        if( _local_ip )
        {
            WiFi.config( _local_ip );
        }

        if( _passphrase )
        {
            result = WiFi.begin( _ssid, _passphrase);
        }
        else if( _key_idx && _key )
        {
            result = WiFi.begin( _ssid, _key_idx, _key );
        }
        else
        {
            result = WiFi.begin( _ssid );
        }
    }
    if( result == 0 ) return false;

    _server = WiFiServer( _port );
    _server.begin();
    return result;
}

/******************************************************************************
 *           Connection functions with DHCP
 ******************************************************************************/

//OPEN networks
int WiFiStream::begin(char* prjname, char *ssid, uint16_t port)
{
    if( !is_ready() ) return 0;

    _ssid = ssid;
    _port = port;
    int result = WiFi.begin( ssid );
    if( result == 0 ) return 0;

    _server = WiFiServer( port );
    _server.begin();

    wUdp.begin(4530); // 4530 is dummy port
    broadcastInterval = 4000;
    broadcastStartTime = millis();
    projectName = prjname;
    pkgOK = makeBroadcastPkg(broadcastPkg, sizeof(broadcastPkg), projectName);
    return result;
}

//WEP-encrypted networks
int WiFiStream::begin(char* prjname, char *ssid, uint8_t key_idx, const char *key, uint16_t port)
{
    if( !is_ready() ) return 0;

    _ssid = ssid;
    _port = port;
    _key_idx = key_idx;
    _key = key;

    int result = WiFi.begin( ssid, key_idx, key );
    if( result == 0 ) return 0;

    _server = WiFiServer( port );
    _server.begin();

    wUdp.begin(4530); // 4530 is dummy port
    broadcastInterval = 4000;
    broadcastStartTime = millis();
    projectName = prjname;
    pkgOK = makeBroadcastPkg(broadcastPkg, sizeof(broadcastPkg), projectName);
    return result;
}

//WPA-encrypted networks
int WiFiStream::begin(char* prjname, char *ssid, const char *passphrase, uint16_t port)
{
    if( !is_ready() ) return 0;

    _ssid = ssid;
    _port = port;
    _passphrase = passphrase;

    int result = WiFi.begin( ssid, passphrase);
    if( result == 0 ) return 0;

    _server = WiFiServer( port );
    _server.begin();

    wUdp.begin(4530); // 4530 is dummy port
    broadcastInterval = 4000;
    broadcastStartTime = millis();
    projectName = prjname;
    pkgOK = makeBroadcastPkg(broadcastPkg, sizeof(broadcastPkg), projectName);
    return result;
}

/******************************************************************************
 *           Connection functions without DHCP
 ******************************************************************************/

//OPEN networks with static IP
int WiFiStream::begin(char* prjname, char *ssid, IPAddress local_ip, uint16_t port)
{
    if( !is_ready() ) return 0;

    _ssid = ssid;
    _port = port;
    _local_ip = local_ip;

    WiFi.config( local_ip );
    int result = WiFi.begin( ssid );
    if( result == 0 ) return 0;

    _server = WiFiServer( port );
    _server.begin();

    wUdp.begin(4530); // 4530 is dummy port
    broadcastInterval = 4000;
    broadcastStartTime = millis();
    projectName = prjname;
    pkgOK = makeBroadcastPkg(broadcastPkg, sizeof(broadcastPkg), projectName);
    return result;
}

//WEP-encrypted networks with static IP
int WiFiStream::begin(char* prjname, char *ssid, IPAddress local_ip, uint8_t key_idx, const char *key, uint16_t port)
{
    if( !is_ready() ) return 0;

    _ssid = ssid;
    _port = port;
    _local_ip = local_ip;
    _key_idx = key_idx;
    _key = key;

    WiFi.config( local_ip );
    int result = WiFi.begin( ssid, key_idx, key );
    if( result == 0 ) return 0;

    _server = WiFiServer( port );
    _server.begin();

    wUdp.begin(4530); // 4530 is dummy port
    broadcastInterval = 4000;
    broadcastStartTime = millis();
    projectName = prjname;
    pkgOK = makeBroadcastPkg(broadcastPkg, sizeof(broadcastPkg), projectName);
    return result;
}

//WPA-encrypted networks with static IP
int WiFiStream::begin(char* prjname, char *ssid, IPAddress local_ip, const char *passphrase, uint16_t port)
{
    if( !is_ready() ) return 0;

    _ssid = ssid;
    _port = port;
    _local_ip = local_ip;
    _passphrase = passphrase;

    WiFi.config( local_ip );
    int result = WiFi.begin( ssid, passphrase);
    if( result == 0 ) return 0;

    _server = WiFiServer( port );
    _server.begin();

    wUdp.begin(4530); // 4530 is dummy port
    broadcastInterval = 4000;
    broadcastStartTime = millis();
    projectName = prjname;
    pkgOK = makeBroadcastPkg(broadcastPkg, sizeof(broadcastPkg), projectName);
    return result;
}

/******************************************************************************
 *             Stream implementations
 ******************************************************************************/

int WiFiStream::available()
{
    IPAddress broadcastIP(255, 255, 255, 255);
  
    if (connect_client() == 0)
    {
        if (pkgOK == true && ((millis() - broadcastStartTime) > broadcastInterval))
        {
            wUdp.beginPacket(broadcastIP, 8686); // broadcast form port 3000
            wUdp.write(broadcastPkg, broadcastPkgSize);
            wUdp.endPacket();
            broadcastStartTime = millis();
        }
        else
            pkgOK = makeBroadcastPkg(broadcastPkg, sizeof(broadcastPkg), projectName);
    }
    return connect_client() ? _client.available() : 0;
}

void WiFiStream::flush()
{
    if( _client ) _client.flush();
}

int WiFiStream::peek()
{
    return connect_client() ? _client.peek(): 0;
}

int WiFiStream::read()
{
    return connect_client() ? _client.read() : -1;
}

void WiFiStream::stop()
{
    _client.stop();
}

size_t WiFiStream::write(uint8_t byte)
{
    if( connect_client() ) _client.write( byte );
}
