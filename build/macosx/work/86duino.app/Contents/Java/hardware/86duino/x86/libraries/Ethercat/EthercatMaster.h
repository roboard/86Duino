#ifndef __WZ_ETHERCAT_MASTER_H
#define __WZ_ETHERCAT_MASTER_H

#include "EthercatError.h"
#include "EthercatMasterSettings.h"
#include "EthercatMasterCallback.h"
#include "EthercatMasterBenchmark.h"
#include "EthercatMasterCoE.h"
#include "EthercatMasterEoE.h"
#include <stdlib.h>
#include <stdint.h>

/* EtherCAT Master Physical Port. */
enum EthernetPort {
    ECAT_ETH_0          = 0, // Main: eth0, Redundant: none
    ECAT_ETH_1          = 1, // Main: eth1, Redundant: none
    ECAT_ETH_REDUNDANCY = 2  // Main: eth0, Redundant: eth1
};

/* EtherCAT Master Mode. */
enum EthercatMasterMode {
    ECAT_FREERUN_MANUAL = 0,
    ECAT_FREERUN        = 1,
    ECAT_SYNC           = 2,
    ECAT_FREERUN_AUTO   = 1 // Reserved for backward compatibility with older versions.
};

/* EtherCAT Master State. */
enum EthercatState {
    ECAT_STATE_NONE     = 0x00,
    ECAT_STATE_INIT     = 0x01,
    ECAT_STATE_PRE_OP   = 0x02,
    ECAT_STATE_BOOT     = 0x03,
    ECAT_STATE_SAFE_OP  = 0x04,
    ECAT_STATE_OP       = 0x08,
    ECAT_STATE_ERROR    = 0x14,
};

enum EthercatMailboxProtocol {
    ECAT_MBX_PROTO_AOE  = 0x0001,
    ECAT_MBX_PROTO_EOE  = 0x0002,
    ECAT_MBX_PROTO_COE  = 0x0004,
    ECAT_MBX_PROTO_FOE  = 0x0008,
    ECAT_MBX_PROTO_SOE  = 0x0010,
    ECAT_MBX_PROTO_VOE  = 0x0020
};

typedef volatile uint8_t *PdoBufferPtr;

class EthercatDevice_CiA402;
class EthercatMaster {

public:
    EthercatMaster();
    ~EthercatMaster();

    int readSettings(EthercatMasterSettings *settings);
    int saveSettings(EthercatMasterSettings *settings);

    /* EtherCAT Master Initialize. All slaves will enter PRE-OP state if success. */
    int begin(EthernetPort eth = ECAT_ETH_0, const char *eni_filename = NULL);
    /* EtherCAT Master shutdown. */
    void end();

    /* Check EtherCAT uses redundancy or not. */
    bool isRedundancy();
    /* The version of EtherCAT Master library. */
    char *libraryVersion();
    /* The version of EtherCAT firmware. Call this after begin(). */
    char *firmwareVersion();

    /* Get Slave Information (SlaveCount/VendorID/ProductCode/RevisionNumber/SerialNumber/AliasAddress) on EtherCAT bus. */
    uint16_t getSlaveCount();
    uint32_t getVendorID(uint16_t slave_no);
    uint32_t getProductCode(uint16_t slave_no);
    uint32_t getRevisionNumber(uint16_t slave_no);
    uint32_t getSerialNumber(uint16_t slave_no);
    int getAliasAddress(uint16_t slave_no);
    int getSlaveNo(uint16_t alias_addr);
    int getSlaveNo(uint32_t vendor, uint32_t product);
    int getSlaveNo(uint32_t vendor, uint32_t product, uint32_t revision, uint32_t serial_num);
    int getSlaveNo(uint16_t alias_addr, uint32_t vendor, uint32_t product);
    int getSlaveNo(uint16_t alias_addr, uint32_t vendor, uint32_t product, uint32_t revision, uint32_t serial_num);

    /* Register/Unregister Cyclic Callback Function. */
    int attachCyclicCallback(CyclicCallbackType callback, bool use_fpu = false);
    int detachCyclicCallback();
    /* Register/Unregister Event Callback Function. */
    void attachEventCallback(EventCallbackType callback, bool use_fpu = false);
    void detachEventCallback();
    /* Register/Unregister Error Callback Function. */
    void attachErrorCallback(ErrorCallbackType callback, bool use_fpu = false);
    void detachErrorCallback();

    /* Call in Event Callback to get event data for ECAT_EVT_STATE_CHANGED. */
    int evtGetMasterState();
    /* Call in Error Callback to get error data for ECAT_ERR_CABLE_BROKEN. */
    int errGetCableBrokenLocation1();
    int errGetCableBrokenLocation2();

    /* Get global shift time. */
    int getShiftTime();
    /* Set global shift time. */
    int setShiftTime(int32_t nanoseconds);

    /* Start EtherCAT Master. All slaves will enter OP state if success. */
    int start(uint64_t cycletime_ns = 0, EthercatMasterMode mode = ECAT_FREERUN);
    /* Stop EtherCAT Master. */
    int stop();
    /* Update PDO. Only used for ECAT_FREERUN_MANUAL mode. */
    void update();

    /* Get system time of current cycle. */
    uint64_t getSystemTime();
    /* Get working counter of current cycle. */
    int getWorkingCounter();
    /* Get expected working counter. */
    int getExpectedWorkingCounter();

    /*************************************************************************
     *  The following APIs are still under development and are not           *
     *  recommended for use.                                                 *
     ************************************************************************/
    /* Register/Unregister Real-Time Cyclic Callback Function. */
    int attachRTCyclicCallback(CyclicCallbackType callback, bool use_fpu = false);
    int detachRTCyclicCallback();

    /* Get measured performance result. */
    int getBenchmarkResult(EthercatBenchmark *result);
    /* Get minimum cycle time which was calculated. */
    uint64_t getMinimumCycleTime();

    int getState(uint32_t timeout_us = 100000);
    int setState(EthercatState state, uint32_t timeout_us = 100000);
    int getDeviceState(uint16_t slave_no, uint16_t *pLastStatusCode = NULL, uint32_t timeout_us = 100000);
    int setDeviceState(uint16_t slave_no, EthercatState state, uint32_t timeout_us = 100000);
    int resume(uint16_t slave_no, uint32_t timeout_us = 100000);

    /************************************************************************
     *  The following APIs are only for internal use.                       *
     ************************************************************************/
    int pdoGetOutputBits(uint16_t slave_no);
    int pdoGetOutputBytes(uint16_t slave_no);
    int pdoGetInputBits(uint16_t slave_no);
    int pdoGetInputBytes(uint16_t slave_no);
    /* Write Bit of Process Data Output. */
    int pdoBitWrite(uint16_t slave_no, uint32_t bit_offset, uint8_t value);
    /* Read Bit of Process Data Input. */
    int pdoBitRead(uint16_t slave_no, uint32_t bit_offset);
    /* Get Slave Process Data Output/Input Pointer. */
    PdoBufferPtr pdoGetOutputBuffer(uint16_t slave_no);
    PdoBufferPtr pdoGetInputBuffer(uint16_t slave_no);
    /* Write Slave Process Data Output. */
    int pdoWrite(uint16_t slave_no, uint32_t offset, void *data, uint32_t size);
    int pdoWrite8(uint16_t slave_no, uint32_t offset, uint8_t value);
    int pdoWrite16(uint16_t slave_no, uint32_t offset, uint16_t value);
    int pdoWrite32(uint16_t slave_no, uint32_t offset, uint32_t value);
    int pdoWrite64(uint16_t slave_no, uint32_t offset, uint64_t value);
    /* Read Slave Process Data Input. */
    int pdoRead(uint16_t slave_no, uint32_t offset, void *data, uint32_t size);
    uint8_t pdoRead8(uint16_t slave_no, uint32_t offset);
    uint16_t pdoRead16(uint16_t slave_no, uint32_t offset);
    uint32_t pdoRead32(uint16_t slave_no, uint32_t offset);
    uint64_t pdoRead64(uint16_t slave_no, uint32_t offset);
    /* Read Slave Process Data Output. */
    int pdoBitReadOutput(uint16_t slave_no, uint32_t bit_offset);
    int pdoReadOutput(uint16_t slave_no, uint32_t offset, void *data, uint32_t size);
    uint8_t pdoReadOutput8(uint16_t slave_no, uint32_t offset);
    uint16_t pdoReadOutput16(uint16_t slave_no, uint32_t offset);
    uint32_t pdoReadOutput32(uint16_t slave_no, uint32_t offset);
    uint64_t pdoReadOutput64(uint16_t slave_no, uint32_t offset);

    /* Read EEPROM. */
    int readSII(uint16_t slave_no, uint32_t offset, void *data, size_t len, uint32_t timeout_ms = 500);
    uint8_t readSII8(uint16_t slave_no, uint32_t offset, uint32_t timeout_ms = 500);
    uint16_t readSII16(uint16_t slave_no, uint32_t offset, uint32_t timeout_ms = 500);
    uint32_t readSII32(uint16_t slave_no, uint32_t offset, uint32_t timeout_ms = 500);
    /* Write EEPROM. */
    int writeSII(uint16_t slave_no, uint32_t offset, void *data, size_t len, uint32_t timeout_ms = 500);
    int writeSII8(uint16_t slave_no, uint32_t offset, uint8_t value, uint32_t timeout_ms = 500);
    int writeSII16(uint16_t slave_no, uint32_t offset, uint16_t value, uint32_t timeout_ms = 500);
    int writeSII32(uint16_t slave_no, uint32_t offset, uint32_t value, uint32_t timeout_ms = 500);
    /* Read/Write EEPROM alias address. */
    uint16_t readSII_AliasAddress(uint16_t slave_no, uint32_t timeout_ms = 500);
    int writeSII_AliasAddress(uint16_t slave_no, uint16_t alias, uint32_t timeout_ms = 500);

    /* CoE SDO Download. */
    int sdoDownload(uint16_t slave_no, uint16_t od_index, uint8_t od_subindex, void *data, uint32_t size, uint32_t *abortcode = NULL, bool complete_access = false, uint32_t timeout_us = 2000000);
    int sdoDownload8(uint16_t slave_no, uint16_t od_index, uint8_t od_subindex, uint8_t value, uint32_t timeout_us = 2000000);
    int sdoDownload16(uint16_t slave_no, uint16_t od_index, uint8_t od_subindex, uint16_t value, uint32_t timeout_us = 2000000);
    int sdoDownload32(uint16_t slave_no, uint16_t od_index, uint8_t od_subindex, uint32_t value, uint32_t timeout_us = 2000000);
    int sdoDownload64(uint16_t slave_no, uint16_t od_index, uint8_t od_subindex, uint64_t value, uint32_t timeout_us = 2000000);
    /* CoE SDO Upload. */
    int sdoUpload(uint16_t slave_no, uint16_t od_index, uint8_t od_subindex, void *data, uint32_t size, uint32_t *abortcode = NULL, bool complete_access = false, uint32_t timeout_us = 2000000);
    uint8_t sdoUpload8(uint16_t slave_no, uint16_t od_index, uint8_t od_subindex, uint32_t timeout_us = 2000000);
    uint16_t sdoUpload16(uint16_t slave_no, uint16_t od_index, uint8_t od_subindex, uint32_t timeout_us = 2000000);
    uint32_t sdoUpload32(uint16_t slave_no, uint16_t od_index, uint8_t od_subindex, uint32_t timeout_us = 2000000);
    uint64_t sdoUpload64(uint16_t slave_no, uint16_t od_index, uint8_t od_subindex, uint32_t timeout_us = 2000000);
    /* CoE SDO Information. */
    int getODlist(uint16_t slave_no, uint16_t *list, uint32_t list_size, uint32_t *abortcode = NULL, uint32_t timeout_us = 2000000);
    int getObjectDescription(uint16_t slave_no, uint16_t od_index, uint16_t *datatype, uint8_t *max_od_subindex, uint8_t *objcode, char *objname, size_t objname_size, uint32_t *abortcode = NULL, uint32_t timeout_us = 2000000);
    int getObjectDescription(uint16_t slave_no, uint16_t od_index, uint16_t &datatype, uint8_t &max_od_subindex, uint8_t &objcode, char *objname, size_t objname_size, uint32_t *abortcode = NULL, uint32_t timeout_us = 2000000);
    int getEntryDescription(uint16_t slave_no, uint16_t od_index, uint8_t od_subindex, uint8_t *valueinfo, uint16_t *datatype, uint16_t *bitlength, uint16_t *objaccess, char *entryname, size_t entryname_size, uint32_t *abortcode = NULL, uint32_t timeout_us = 2000000);
    int getEntryDescription(uint16_t slave_no, uint16_t od_index, uint8_t od_subindex, uint8_t &valueinfo, uint16_t &datatype, uint16_t &bitlength, uint16_t &objaccess, char *entryname, size_t entryname_size, uint32_t *abortcode = NULL, uint32_t timeout_us = 2000000);
    /* CoE SDO Download. (Non-Blocking) */
    int sdoDownload_Begin(uint16_t slave_no, uint16_t od_index, uint8_t od_subindex, void *data, uint32_t size, bool complete_access = false, uint32_t timeout_us = 2000000);
    int sdoDownload_Status(uint16_t slave_no);
    int sdoDownload_End(uint16_t slave_no);
    uint32_t sdoDownload_ReadAbortCode(uint16_t slave_no);
    /* CoE SDO Upload. (Non-Blocking) */
    int sdoUpload_Begin(uint16_t slave_no, uint16_t od_index, uint8_t od_subindex, void *data, uint32_t size, bool complete_access = false, uint32_t timeout_us = 2000000);
    int sdoUpload_Status(uint16_t slave_no);
    int sdoUpload_End(uint16_t slave_no);
    uint32_t sdoUpload_ReadAbortCode(uint16_t slave_no);
    uint32_t sdoUpload_ReadLength(uint16_t slave_no);
    /* CoE Get OD List. (Non-Blocking) */
    int getODlist_Begin(uint16_t slave_no, uint16_t *list, uint32_t list_size, uint32_t timeout_us = 2000000);
    int getODlist_Status(uint16_t slave_no);
    int getODlist_End(uint16_t slave_no);
    uint32_t getODlist_ReadAbortCode(uint16_t slave_no);
    uint32_t getODlist_ReadListSize(uint16_t slave_no);
    /* CoE Get Object Description. (Non-Blocking) */
    int getObjectDescription_Begin(uint16_t slave_no, uint16_t od_index, uint32_t timeout_us = 2000000);
    int getObjectDescription_Status(uint16_t slave_no);
    int getObjectDescription_End(uint16_t slave_no);
    uint32_t getObjectDescription_ReadAbortCode(uint16_t slave_no);
    uint16_t getObjectDescription_ReadDataType(uint16_t slave_no);
    uint8_t getObjectDescription_ReadMaxSubindex(uint16_t slave_no);
    uint8_t getObjectDescription_ReadObjectCode(uint16_t slave_no);
    char *getObjectDescription_ReadObjectName(uint16_t slave_no, char *name, size_t size);
    /* CoE Get Entry Description. (Non-Blocking) */
    int getEntryDescription_Begin(uint16_t slave_no, uint16_t od_index, uint8_t od_subindex, uint32_t timeout_us = 2000000);
    int getEntryDescription_Status(uint16_t slave_no);
    int getEntryDescription_End(uint16_t slave_no);
    uint32_t getEntryDescription_ReadAbortCode(uint16_t slave_no);
    uint8_t getEntryDescription_ReadValueInfo(uint16_t slave_no);
    uint16_t getEntryDescription_ReadDataType(uint16_t slave_no);
    uint16_t getEntryDescription_ReadBitLength(uint16_t slave_no);
    uint16_t getEntryDescription_ReadObjectAccess(uint16_t slave_no);
    char *getEntryDescription_ReadEntryName(uint16_t slave_no, char *name, size_t size);
    /* Get Last Abort Code. */
    uint32_t getLastAbortCode(uint16_t slave_no);

    /* FoE Read. */
    int readFoE(uint16_t slave_no, char *filename, uint32_t password, void *data, uint32_t size, uint32_t timeout_ms = 30000);
    /* FoE Write. */
    int writeFoE(uint16_t slave_no, char *filename, uint32_t password, void *data, uint32_t size, uint32_t timeout_ms = 30000);

    /* EoE related. (Blocking). */
    int eoeRead(uint16_t slave_no, uint8_t port, MACframe frame, uint32_t timeout_us = 2000000);
    int eoeWrite(uint16_t slave_no, uint8_t port, MACframe frame, uint32_t size, uint32_t timeout_us = 2000000);
    int eoeGetIP(uint16_t slave_no, uint8_t port, EthercatEoE_IPParameter *param, uint32_t timeout_us = 2000000);
    int eoeSetIP(uint16_t slave_no, uint8_t port, EthercatEoE_IPParameter *param, uint16_t *result, uint32_t timeout_us = 2000000);
    int eoeGetAddressFilter(uint16_t slave_no, uint8_t port, EthercatEoE_AddressFilter *filter, uint32_t timeout_us = 2000000);
    int eoeSetAddressFilter(uint16_t slave_no, uint8_t port, EthercatEoE_AddressFilter *filter, uint16_t *result, uint32_t timeout_us = 2000000);
    /* EoE: Read. (Non-Blocking). */
    int eoeRead_Begin(uint16_t slave_no, uint8_t port, MACframe frame, uint32_t timeout_us = 2000000);
    int eoeRead_Status(uint16_t slave_no);
    int eoeRead_End(uint16_t slave_no);
    uint32_t eoeRead_ReadLength(uint16_t slave_no);
    /* EoE: Write. (Non-Blocking). */
    int eoeWrite_Begin(uint16_t slave_no, uint8_t port, MACframe frame, uint32_t size, uint32_t timeout_us = 2000000);
    int eoeWrite_Status(uint16_t slave_no);
    int eoeWrite_End(uint16_t slave_no);
    /* EoE: Get IP parameters. (Non-Blocking). */
    int eoeGetIP_Begin(uint16_t slave_no, uint8_t port, EthercatEoE_IPParameter *param, uint32_t timeout_us = 2000000);
    int eoeGetIP_Status(uint16_t slave_no);
    int eoeGetIP_End(uint16_t slave_no);
    /* EoE: Set IP parameters. (Non-Blocking). */
    int eoeSetIP_Begin(uint16_t slave_no, uint8_t port, EthercatEoE_IPParameter *param, uint32_t timeout_us = 2000000);
    int eoeSetIP_Status(uint16_t slave_no);
    int eoeSetIP_End(uint16_t slave_no);
    uint16_t eoeSetIP_ReadResult(uint16_t slave_no);
    /* EoE: Get Address Filter. (Non-Blocking). */
    int eoeGetAddressFilter_Begin(uint16_t slave_no, uint8_t port, EthercatEoE_AddressFilter *filter, uint32_t timeout_us = 2000000);
    int eoeGetAddressFilter_Status(uint16_t slave_no);
    int eoeGetAddressFilter_End(uint16_t slave_no);
    /* EoE: Set Address Filter. (Non-Blocking). */
    int eoeSetAddressFilter_Begin(uint16_t slave_no, uint8_t port, EthercatEoE_AddressFilter *filter, uint32_t timeout_us = 2000000);
    int eoeSetAddressFilter_Status(uint16_t slave_no);
    int eoeSetAddressFilter_End(uint16_t slave_no);
    uint16_t eoeSetAddressFilter_ReadResult(uint16_t slave_no);

    /* DC configuration. */
    int setDc(uint16_t slave_no, uint32_t cycletime0_ns, int32_t shifttime_ns = 0, uint32_t cycletime1_ns = 0);

    int readESC(uint16_t slave_no, uint16_t ado, void *data, size_t len, uint32_t timeout_ms = 1000);
    uint8_t readESC8(uint16_t slave_no, uint16_t ado, uint32_t timeout_ms = 1000);
    uint16_t readESC16(uint16_t slave_no, uint16_t ado, uint32_t timeout_ms = 1000);
    uint32_t readESC32(uint16_t slave_no, uint16_t ado, uint32_t timeout_ms = 1000);
    uint64_t readESC64(uint16_t slave_no, uint16_t ado, uint32_t timeout_ms = 1000);
    int writeESC(uint16_t slave_no, uint16_t ado, void *data, size_t len, uint32_t timeout_ms = 1000);
    int writeESC8(uint16_t slave_no, uint16_t ado, uint8_t value, uint32_t timeout_ms = 1000);
    int writeESC16(uint16_t slave_no, uint16_t ado, uint16_t value, uint32_t timeout_ms = 1000);
    int writeESC32(uint16_t slave_no, uint16_t ado, uint32_t value, uint32_t timeout_ms = 1000);
    int writeESC64(uint16_t slave_no, uint16_t ado, uint64_t value, uint32_t timeout_ms = 1000);

    int enterBootState(uint16_t slave_no, uint32_t timeout_ms = 3000);
    int exitBootState(uint16_t slave_no, uint32_t timeout_ms = 3000);

    char *getDeviceName(uint16_t slave_no, char *name, size_t len);
    int getMailboxProtocol(uint16_t slave_no);
    int getCoEDetails(uint16_t slave_no);
    int getFoEDetails(uint16_t slave_no);
    int getEoEDetails(uint16_t slave_no);
    int getSoEChannels(uint16_t slave_no);
    int isSupportDC(uint16_t slave_no);

    int __addCiA402__(EthercatDevice_CiA402 *device, uint16_t slave_no, uint32_t drive_id);
    int __delCiA402__(size_t id);
    int __inInterrupt__();
    uint64_t __getCycleTime__();

    /* For internal use. */
    int getErrorCallbackData(void *data, size_t size);
    int getEventCallbackData(void *data, size_t size);
    char *getErrorMessage(int error, char *message, size_t size);
    char *getEventMessage(int event, char *message, size_t size);
    uint32_t getFreeSystemMemory();
    uint32_t getFreeSharedMemory();
    int __begin1__(EthernetPort eth = ECAT_ETH_0, const char *eni_filename = NULL);
    int __begin2__();

private:
    void *private_data;
    char libVersion[128];
    EthercatMasterSettings settings;
    ErrorCallbackType UserErrorCallback;
    EventCallbackType UserEventCallback;
    bool UserErrorCallbackUseFPU;
    bool UserEventCallbackUseFPU;

};

#endif
