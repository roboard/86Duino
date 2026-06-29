#include "SwsSock.h"
#include "Ethernet.h"
#include "Dhcp.h"
#include "io.h"

#if defined(ETHERNET_REMOTE)

#define MAX_SOCKS (4)
static int rc;
static unsigned char rxbuf[4096];
static unsigned char txbuf[4096];
static unsigned long val;
static bool init_OK = false;

static int fd;
VDPM_HANDLE_t vdppSlave;

// port 86: serial port
// port 87: SD port
// port 88: Ethernet

// Ethernet command:
// 1 : localMAC
// 2 : begin_dhcp
// 3 : begin2_fixIP1
// 4 : begin2_fixIP2
// 5 : begin2_fixIP3
// 6 : begin2_fixIP4
// 7 : localIP
// 8 : subnetMask
// 9 : gatewayIP
// 10: dnsServerIP
void send_command_para(uint8_t c, uint8_t *p, int p_size)
{
    int i = 0, cur = 0, j;

    // command
    txbuf[i++] = c;

    // other parameters
    if (p != NULL)
    {
        for (j = 0; j < p_size; j++)
            txbuf[i++] = *(p++);
    }

    do
    {
        rc = ipcnet_send(fd, &txbuf[cur], i - cur);
        if (rc > 0)
            cur += rc;
    } while (cur != i);
}

// -1 : continue
//  0 : complite
int recv_command_result(uint8_t *mybuf, int num)
{
    int i, cur = 0;

    do
    {
        rc = ipcnet_recv(fd, &rxbuf[cur], num - cur);
        if (rc < 0 && ipcnet_errno(fd) == IPCNET_ECONNABORTED)
            return -1;
        cur += rc;
    } while (cur != num);

    for (i = 0; i < num; i++)
        mybuf[i] = rxbuf[i];

    return 0;
}

int fourBytes2Int(unsigned char a0, unsigned char a1, unsigned char a2, unsigned char a3)
{
    int r = 0;
    r = a3;
    r = (r << 8) + a2;
    r = (r << 8) + a1;
    r = (r << 8) + a0;
    return r;
}

static void init_Ethernet()
{
    int i, r;
    uint8_t _r[1];
    unsigned long tmp;

    val = 1;

    fd = ipcnet_socket();
    ipcnet_ioctl(fd, IPCNET_FIONBIO, (unsigned long)&val);

    do
    {
        rc = ipcnet_connect(fd, 88); // 88 : Ethernet port
        if (rc == 0)
            break;
        printf("begining...\r");
    } while (1);
    printf("\n begin() OK!\n");

    if (vdpm_Initialize(1 * 1024) < 0)
        printf("ERROR: vdpm_Initialize() failed.\n");

    send_command_para(0xAA, NULL, 0);
    r = recv_command_result(_r, 4);

    if (r == 0)
    {
        tmp = fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
        vdppSlave = vdpm_GetMemHandle(tmp);
        printf("vdpm_GetMemHandle: %08X\n", vdppSlave);
    }
    else
        printf("vdpm_GetMemHandle() not be run.\n");
}

uint8_t EthernetClass::_macAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

uint8_t *EthernetClass::localMAC()
{
    int i, r;
    uint8_t _r[1];

    send_command_para(1, NULL, 0); // 1 : "localMAC" command
    r = recv_command_result(_r, 6);
    if (r == 0)
    {
        for (i = 0; i < 6; i++)
            _macAddress[i] = _r[i];
    }

    return &_macAddress[0];
}

int EthernetClass::begin(uint8_t *mac_address)
{
    return begin();
}

int EthernetClass::begin(void)
{
    int i, r;
    uint8_t _r[1];

    if (init_OK)
        return 0;

    init_Ethernet();
    init_OK = true;

    send_command_para(2, NULL, 0); // 2 : "begin_dhcp" command
    r = recv_command_result(_r, 1);
    if (r == 0)
    {
        return _r[0]; // _r[0] should be 1.
    }

    return 0;
}

int EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip)
{
    int i, j, r;
    uint8_t _r[1];
    uint8_t tmp[4];

    if (init_OK)
        return 1;

    init_Ethernet();
    init_OK = true;

    for (i = 0, j = 0; i < 4; i++, j++)
        tmp[i] = local_ip[j];

    send_command_para(3, tmp, 4); // 3 : "begin_fixIP1" command
    r = recv_command_result(_r, 1);
    if (r == 0)
    {
        printf("Begin local_ip OK\n");
        return 1;
    }

    printf("Begin local_ip Fail\n");
    return 0;
}

int EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server)
{
    int i, j, r;
    uint8_t _r[1];
    uint8_t tmp[8];

    if (init_OK)
        return 1;

    init_Ethernet();
    init_OK = true;

    for (i = 0, j = 0; i < 4; i++, j++)
        tmp[i] = local_ip[j];
    for (i = 4, j = 0; i < 8; i++, j++)
        tmp[i] = dns_server[j];

    send_command_para(4, tmp, 8); // 4 : "begin_fixIP2" command
    r = recv_command_result(_r, 1);
    if (r == 0)
    {
        printf("Begin local_ip dns_server OK\n");
        return 1;
    }

    printf("Begin local_ip dns_server Fail\n");
    return 0;
}

int EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server, IPAddress gateway)
{
    int i, j, r;
    uint8_t _r[1];
    uint8_t tmp[16];

    if (init_OK)
        return 1;

    init_Ethernet();
    init_OK = true;

    for (i = 0, j = 0; i < 4; i++, j++)
        tmp[i] = local_ip[j];
    for (i = 4, j = 0; i < 8; i++, j++)
        tmp[i] = dns_server[j];
    for (i = 8, j = 0; i < 12; i++, j++)
        tmp[i] = gateway[j];

    send_command_para(5, tmp, 12); // 5 : "begin_fixIP3" command
    r = recv_command_result(_r, 1);
    if (r == 0)
    {
        printf("Begin local_ip dns_server gateway OK\n");
        return 1;
    }

    return 0;
}

int EthernetClass::begin(uint8_t *mac, IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet)
{
    int i, j, r;
    uint8_t _r[1];
    uint8_t tmp[16];

    if (init_OK)
        return 1;

    init_Ethernet();
    init_OK = true;

    for (i = 0, j = 0; i < 4; i++, j++)
        tmp[i] = local_ip[j];
    for (i = 4, j = 0; i < 8; i++, j++)
        tmp[i] = dns_server[j];
    for (i = 8, j = 0; i < 12; i++, j++)
        tmp[i] = gateway[j];
    for (i = 12, j = 0; i < 16; i++, j++)
        tmp[i] = subnet[j];

    send_command_para(6, tmp, 16); // 6 : "begin_fixIP4" command
    r = recv_command_result(_r, 1);
    if (r == 0)
    {
        printf("Begin local_ip dns_server gateway subnet OK\n");
        return 1;
    }

    return 0;
}

int EthernetClass::maintain()
{
#if 0
    int rc = DHCP_CHECK_NONE;
    if(_dhcp != NULL){
        //we have a pointer to dhcp, use it
        rc = _dhcp->checkLease();
        switch ( rc ){
            case DHCP_CHECK_NONE:
                //nothing done
                break;
            case DHCP_CHECK_RENEW_OK:
            case DHCP_CHECK_REBIND_OK:
                //we might have got a new IP.
                W5100.setIPAddress(_dhcp->getLocalIp().raw_address());
                W5100.setGatewayIp(_dhcp->getGatewayIp().raw_address());
                W5100.setSubnetMask(_dhcp->getSubnetMask().raw_address());
                _dnsServerAddress = _dhcp->getDnsServerIp();
                break;
            default:
                //this is actually a error, it will retry though
            break;
        }
    }
    return rc;
#else
    return 0;
#endif
}

IPAddress EthernetClass::localIP()
{
    IPAddress ret;
    int i, r;
    uint8_t _r[4];

    send_command_para(7, NULL, 0); // 7 : "localIP" command
    r = recv_command_result(_r, 4);
    if (r == 0)
    {
        for (i = 0; i < 4; i++)
            ret[i] = _r[i];
    }

    return ret;
}

IPAddress EthernetClass::subnetMask()
{
    IPAddress ret;
    int i, r;
    uint8_t _r[4];

    send_command_para(8, NULL, 0); // 8 : "subnetMask" command
    r = recv_command_result(_r, 4);
    if (r == 0)
    {
        for (i = 0; i < 4; i++)
            ret[i] = _r[i];
    }

    return ret;
}

IPAddress EthernetClass::gatewayIP()
{
    IPAddress ret;
    int i, r;
    uint8_t _r[4];

    send_command_para(9, NULL, 0); // 9 : "gatewayIP" command
    r = recv_command_result(_r, 4);
    if (r == 0)
    {
        for (i = 0; i < 4; i++)
            ret[i] = _r[i];
    }

    return ret;
}

IPAddress EthernetClass::dnsServerIP()
{
    IPAddress ret;
    int i, r;
    uint8_t _r[4];

    send_command_para(10, NULL, 0); // 10 : "dnsServerIP" command
    r = recv_command_result(_r, 4);
    if (r == 0)
    {
        for (i = 0; i < 4; i++)
            ret[i] = _r[i];
    }

    return ret;
}

#else

DhcpClass *EthernetClass::_dhcp = NULL;
uint8_t EthernetClass::_macAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

uint8_t *EthernetClass::localMAC()
{
    return &_macAddress[0];
}

void EthernetClass::MACAddress(uint8_t *mac_address)
{
    memcpy(mac_address, _macAddress, sizeof(_macAddress));
}

int EthernetClass::begin(uint8_t *mac_address)
{
    return begin();
}

int EthernetClass::begin(void)
{
    int ret = _dhcp->beginWithDHCP();
    SwsSock.getMACAddress(_macAddress);
    return ret;
}

int EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip)
{
    // Assume the DNS server will be the machine on the same network as the local IP
    // but with last octet being '1'
    IPAddress dns_server = local_ip;
    dns_server[3] = 1;
    return begin(mac_address, local_ip, dns_server);
}

int EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server)
{
    // Assume the gateway will be the machine on the same network as the local IP
    // but with last octet being '1'
    IPAddress gateway = local_ip;
    gateway[3] = 1;
    return begin(mac_address, local_ip, dns_server, gateway);
}

int EthernetClass::begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server, IPAddress gateway)
{
    IPAddress subnet(255, 255, 255, 0);
    return begin(mac_address, local_ip, dns_server, gateway, subnet);
}

int EthernetClass::begin(uint8_t *mac, IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet)
{
    int ret = SwsSock.init(local_ip, subnet, gateway, dns_server);
    SwsSock.getMACAddress(_macAddress);
    return ret;
}

int EthernetClass::maintain()
{
#if 0
    int rc = DHCP_CHECK_NONE;
    if(_dhcp != NULL){
        //we have a pointer to dhcp, use it
        rc = _dhcp->checkLease();
        switch ( rc ){
            case DHCP_CHECK_NONE:
                //nothing done
                break;
            case DHCP_CHECK_RENEW_OK:
            case DHCP_CHECK_REBIND_OK:
                //we might have got a new IP.
                W5100.setIPAddress(_dhcp->getLocalIp().raw_address());
                W5100.setGatewayIp(_dhcp->getGatewayIp().raw_address());
                W5100.setSubnetMask(_dhcp->getSubnetMask().raw_address());
                _dnsServerAddress = _dhcp->getDnsServerIp();
                break;
            default:
                //this is actually a error, it will retry though
            break;
        }
    }
    return rc;
#else
    return 0;
#endif
}

IPAddress EthernetClass::localIP()
{
    return SwsSock.getLocalIp();
}

IPAddress EthernetClass::subnetMask()
{
    return SwsSock.getSubnetMask();
}

IPAddress EthernetClass::gatewayIP()
{
    return SwsSock.getGatewayIp();
}

IPAddress EthernetClass::dnsServerIP()
{
    return SwsSock.getDnsServerIp();
}

#endif

EthernetClass Ethernet;
