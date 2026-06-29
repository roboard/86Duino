#ifndef ___WZ_ETHERCAT_MASTER_SETTINGS_H
#define ___WZ_ETHERCAT_MASTER_SETTINGS_H

#include <stdint.h>

/* EtherCAT Synchronization Mode. */
enum EthercatDcSyncMode {
    ECAT_MASTER_SHIFT   = 0,
    ECAT_BUS_SHIFT      = 1
};

/* EtherCAT Cycle Mode. */
enum EthercatCycleMode {
    ECAT_RECV_PDO_FIRST = 0,
    ECAT_SEND_PDO_FIRST = 1,
};

typedef struct {

    EthercatDcSyncMode DcSyncMode;          /* DC Synchronization Mode. (Master Shift, Bus Shift) */
    uint32_t StaticDriftCompensationFrames; /* Bus Synchronization (IP). */
    uint32_t MasterShiftRefSyncDuration;    /* The duration of Reference Time Synchronization for Master Shift Mode on PS transition. Unit: milliseconds. */
    uint32_t MasterShiftRefSyncThreshold;   /* The threshold of Reference Time Synchronization for Master Shift Mode on PS transition. Unit: nanoseconds. */
    int32_t ShiftTimeCompensation;          /* Global Shift Time Compensation. Unit: nanoseconds. */

    EthercatCycleMode CycleMode;            /* Cycle Procedure Mode. (Send PDO first, Receive PDO first) */
    uint32_t WaitAckTimeout;                /* Waiting timeout for acknowledge after trigger cyclic callback. Unit: microseconds. */
    uint32_t SoonExpired;                   /* Time difference before expiration is regarded as expiring soon. Unit: microseconds. */
    uint32_t SendPdoTimePoint;              /* Send PDO time. Unit: nanoseconds. (only for Receive PDO first) */

    uint32_t StateMachineTimeoutI2P;        /* INIT to PREOP timeout. Unit: milliseconds. */
    uint32_t StateMachineTimeoutP2S;        /* PREOP to SAFEOP timeout. Unit: milliseconds. */
    uint32_t StateMachineTimeoutS2O;        /* SAFEOP to OP timeout. Unit: milliseconds. */
    uint32_t ScanNetworkTimeout;            /* Scan network timeout. Unit: milliseconds. */
    uint32_t StartMasterTimeout;            /* Start timeout for master. Unit: milliseconds. */
    uint32_t StartDeviceTimeout;            /* Start timeout for each slave device. Unit: milliseconds. */

    uint32_t FreeRunTimerMultiplier;        /* Update PDO time equals cycle time mutiples this value. (only for auto mode) */
    uint32_t DeviceRecoveryTimeout;         /* Unit: milliseconds. */
    uint32_t CableBrokenDetectionCycleTime; /* Unit: milliseconds. */
    uint32_t ErrorDetectWkcMultipleFaultsThreshold;
    uint32_t ErrorDetectMultipleLostFramesThreshold;
    uint32_t EnableErrorBusReactionSyncUnitToSafeOp:1,
             EnableErrorBusReactionSyncUnitToSafeOpAutoRestart:1,
             EnableBenchmarkMeasurement:1,
             IgnoreDeviceErrorResume:1,
             IgnoreBiosOverride:1;

} EthercatMasterSettings;

#endif
