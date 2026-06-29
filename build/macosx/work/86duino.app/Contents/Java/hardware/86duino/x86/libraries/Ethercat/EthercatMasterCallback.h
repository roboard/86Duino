#ifndef ___WZ_ETHERCAT_MASTER_CALLBACK_H
#define ___WZ_ETHERCAT_MASTER_CALLBACK_H

#include "EthercatOsDefinitions.h"
#include <stdint.h>

typedef void (*CyclicCallbackType)(void);
typedef void (*EventCallbackType)(uint32_t);
typedef void (*ErrorCallbackType)(uint32_t);

enum EthercatEventCallbackCode {
    ECAT_EVT_STATE_CHANGED          = 1000001,
    ECAT_EVT_CABLE_RECONNECTED      = 1000002,
    ECAT_EVT_SLAVE_RECOVERED        = 1000003,
    ECAT_EVT_SLAVE_RECONFIGURED     = 1000004
};

enum EthercatErrorCallbackCode {
    ECAT_ERR_WKC_SINGLE_FAULT       = 2000001,
    ECAT_ERR_WKC_MULTIPLE_FAULTS    = 2000002,
    ECAT_ERR_SINGLE_LOST_FRAME      = 2000003,
    ECAT_ERR_MULTIPLE_LOST_FRAMES   = 2000004,
    ECAT_ERR_LOST_SLAVE             = 2000005,
    ECAT_ERR_STATE_MISMATCH         = 2000006,
    ECAT_ERR_CABLE_BROKEN           = 2000007,
    ECAT_ERR_EMERGENCY              = 2000100,
    ECAT_ERR_WAIT_ACK_TIMEOUT       = 2001000
};

PACKED_BEGIN
/* State Changed Event Data. */
typedef struct PACKED {

    uint8_t GroupID;
    uint8_t State;
    uint8_t Reserved[2];

} EthercatEventCB_StateChanged;

/* Slave Recovered Event Data. */
typedef struct PACKED {

    uint8_t GroupID;
    uint16_t SlaveID;
    uint8_t Reserved[1];

} EthercatEventCB_SlaveRecovered;

/* Slave Reconfigured Event Data. */
typedef struct PACKED {

    uint8_t GroupID;
    uint16_t SlaveID;
    uint8_t Reserved[1];

} EthercatEventCB_SlaveReconfigured;

/* Working Counter Single Fault Data. */
typedef struct PACKED {

    uint8_t GroupID;
    uint8_t Reserved[3];

} EthercatErrorCB_WKCSingleFault;

/* Working Counter Multiple Faults Data. */
typedef struct PACKED {

    uint8_t GroupID;
    uint8_t Reserved[3];

} EthercatErrorCB_WKCMultipleFaults;

/* Single Lost Frame Data. */
typedef struct PACKED {

    uint8_t GroupID;
    uint8_t Reserved[3];

} EthercatErrorCB_SingleLostFrame;

/* Multiple Lost Frames Data. */
typedef struct PACKED {

    uint8_t GroupID;
    uint8_t Reserved[3];

} EthercatErrorCB_MultipleLostFrames;

/* Lost Slave Data. */
typedef struct PACKED {

    uint8_t GroupID;
    uint16_t SlaveID;
    uint8_t Reserved[1];

} EthercatErrorCB_LostSlave;

/* EtherCAT State Mismatch Data. */
typedef struct PACKED {

    uint8_t GroupID;
    uint16_t SlaveID;
    uint16_t ALStatus;
    uint16_t ALStatusCode;
    uint8_t Reserved[1];

} EthercatErrorCB_StateMismatch;

/* Cable Broken Data. */
typedef struct PACKED {

    int Slave1;
    int Slave2;

} EthercatErrorCB_CableBroken;

/* Emergency Message. */
typedef struct PACKED {

    uint16_t SlaveID;
    uint16_t ErrorCode;
    uint8_t ErrorRegister;
    uint8_t Data[5];
    uint8_t Reserved[2];

} EthercatErrorCB_Emergency;
PACKED_END

#endif
