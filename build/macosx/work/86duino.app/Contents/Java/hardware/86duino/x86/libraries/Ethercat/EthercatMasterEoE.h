#ifndef ___WZ_ETHERCAT_MASTER_EOE_H
#define ___WZ_ETHERCAT_MASTER_EOE_H

#include "EthercatOsDefinitions.h"
#include <stdint.h>

enum EthercatEoEStatus {
    ECAT_EOE_STATUS_IDLE    = 0,
    ECAT_EOE_STATUS_SUCCESS = 1,
    ECAT_EOE_STATUS_TIMEOUT = 2,
    ECAT_EOE_STATUS_BUSY    = 3
};

enum EthercatEoEResult {

    ECAT_EOE_SUCCESS                = 0x0000,
    ECAT_EOE_UNSPECIFIED_ERROR      = 0x0001,
    ECAT_EOE_UNSUPPORTED_FRAME_TYPE = 0x0002,
    ECAT_EOE_NO_IP_SUPPORT          = 0x0201,
    ECAT_EOE_DHCP_NOT_SUPPORTED     = 0x0202,
    ECAT_EOE_NO_FILTER_SUPPORT      = 0x0401

};

typedef uint8_t MACframe[1536];
typedef uint8_t MACaddress[6];
typedef uint8_t IPaddress[4];

PACKED_BEGIN
typedef struct PACKED {

    uint32_t    MAC_included:1,
                IP_included:1,
                SubnetMask_included:1,
                DefaultGateway_included:1,
                DNSServerIP_included:1,
                DNSName_included:1,
                Reserved:26;
    MACaddress  MAC;
    IPaddress   IP;
    IPaddress   SubnetMask;
    IPaddress   DefaultGateway;
    IPaddress   DNSServerIP;
    char        DNSName[32];

} EthercatEoE_IPParameter;
PACKED_END

PACKED_BEGIN
typedef struct PACKED {

    uint16_t    MACaddressCount:4,
                MACfilterCount:2,
                Reserved:1,
                InhibitBroadcast:1,
                Reserved2:8;
    MACaddress  Address[18];

} EthercatEoE_AddressFilter;
PACKED_END

#endif
