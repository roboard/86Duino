#ifndef ___WZ_ETHERCAT_DEVICE_H
#define ___WZ_ETHERCAT_DEVICE_H

#include "EthercatMaster.h"

enum EthercatAttachMode {
    ECAT_SLAVE_NO,
    ECAT_ALIAS_ADDRESS
};

class EthercatDevice_Generic;
typedef class EthercatDevice {
public:
    EthercatDevice();
    virtual ~EthercatDevice();

    virtual int attach(uint16_t slave_id, EthercatMaster *master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    virtual int attach(uint16_t slave_id, EthercatMaster &master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    virtual int detach();

    uint32_t getVendorID();
    uint32_t getProductCode();
    uint32_t getRevisionNumber();
    uint32_t getSerialNumber();
    int getAliasAddress();
    int getSlaveNo();

    char *getDeviceName(char *name, size_t len);
    int getMailboxProtocol();
    int getCoEDetails();
    int getFoEDetails();
    int getEoEDetails();
    int getSoEChannels();
    int isSupportDC();

    /* Read EEPROM. */
    int readSII(uint32_t offset, void *data, size_t len, uint32_t timeout_ms = 500);
    uint8_t readSII8(uint32_t offset, uint32_t timeout_ms = 500);
    uint16_t readSII16(uint32_t offset, uint32_t timeout_ms = 500);
    uint32_t readSII32(uint32_t offset, uint32_t timeout_ms = 500);
    /* Write EEPROM. */
    int writeSII(uint32_t offset, void *data, size_t len, uint32_t timeout_ms = 500);
    int writeSII8(uint32_t offset, uint8_t value, uint32_t timeout_ms = 500);
    int writeSII16(uint32_t offset, uint16_t value, uint32_t timeout_ms = 500);
    int writeSII32(uint32_t offset, uint32_t value, uint32_t timeout_ms = 500);

    /* Write Bit of Process Data Output. */
    int pdoBitWrite(uint32_t bit_offset, uint8_t value);
    /* Read Bit of Process Data Input. */
    int pdoBitRead(uint32_t bit_offset);
    /* Get Slave Process Data Output/Input Pointer. */
    PdoBufferPtr pdoGetOutputBuffer();
    PdoBufferPtr pdoGetInputBuffer();
    /* Write Slave Process Data Output. */
    int pdoWrite(uint32_t offset, void *data, uint32_t size);
    int pdoWrite8(uint32_t offset, uint8_t value);
    int pdoWrite16(uint32_t offset, uint16_t value);
    int pdoWrite32(uint32_t offset, uint32_t value);
    int pdoWrite64(uint32_t offset, uint64_t value);
    /* Read Slave Process Data Input. */
    int pdoRead(uint32_t offset, void *data, uint32_t size);
    uint8_t pdoRead8(uint32_t offset);
    uint16_t pdoRead16(uint32_t offset);
    uint32_t pdoRead32(uint32_t offset);
    uint64_t pdoRead64(uint32_t offset);

    /* CoE related. (Blocking). */
    int sdoDownload(uint16_t od_index, uint8_t od_subindex, void *data, uint32_t size, uint32_t *abortcode = NULL, bool complete_access = false, uint32_t timeout_us = 2000000);
    int sdoDownload8(uint16_t od_index, uint8_t od_subindex, uint8_t value, uint32_t timeout_us = 2000000);
    int sdoDownload16(uint16_t od_index, uint8_t od_subindex, uint16_t value, uint32_t timeout_us = 2000000);
    int sdoDownload32(uint16_t od_index, uint8_t od_subindex, uint32_t value, uint32_t timeout_us = 2000000);
    int sdoDownload64(uint16_t od_index, uint8_t od_subindex, uint64_t value, uint32_t timeout_us = 2000000);
    int sdoUpload(uint16_t od_index, uint8_t od_subindex, void *data, uint32_t size, uint32_t *abortcode = NULL, bool complete_access = false, uint32_t timeout_us = 2000000);
    uint8_t sdoUpload8(uint16_t od_index, uint8_t od_subindex, uint32_t timeout_us = 2000000);
    uint16_t sdoUpload16(uint16_t od_index, uint8_t od_subindex, uint32_t timeout_us = 2000000);
    uint32_t sdoUpload32(uint16_t od_index, uint8_t od_subindex, uint32_t timeout_us = 2000000);
    uint64_t sdoUpload64(uint16_t od_index, uint8_t od_subindex, uint32_t timeout_us = 2000000);
    int getODlist(uint16_t *list, uint32_t list_size, uint32_t *abortcode = NULL, uint32_t timeout_us = 2000000);
    int getObjectDescription(uint16_t od_index, uint16_t *datatype, uint8_t *max_od_subindex, uint8_t *objcode, char *objname, size_t objname_size, uint32_t *abortcode = NULL, uint32_t timeout_us = 2000000);
    int getObjectDescription(uint16_t od_index, uint16_t &datatype, uint8_t &max_od_subindex, uint8_t &objcode, char *objname, size_t objname_size, uint32_t *abortcode = NULL, uint32_t timeout_us = 2000000);
    int getEntryDescription(uint16_t od_index, uint8_t od_subindex, uint8_t *valueinfo, uint16_t *datatype, uint16_t *bitlength, uint16_t *objaccess, char *entryname, size_t entryname_size, uint32_t *abortcode = NULL, uint32_t timeout_us = 2000000);
    int getEntryDescription(uint16_t od_index, uint8_t od_subindex, uint8_t &valueinfo, uint16_t &datatype, uint16_t &bitlength, uint16_t &objaccess, char *entryname, size_t entryname_size, uint32_t *abortcode = NULL, uint32_t timeout_us = 2000000);

    /* FoE Read. */
    int readFoE(char *filename, uint32_t password, void *data, uint32_t size, uint32_t timeout_ms = 30000);
    /* FoE Write. */
    int writeFoE(char *filename, uint32_t password, void *data, uint32_t size, uint32_t timeout_ms = 30000);

    /* DC configuration. */
    int setDc(uint32_t cycletime0_ns, int32_t shifttime_ns = 0, uint32_t cycletime1_ns = 0);

    /************************************************************************
     *  The following APIs are only for internal use.                       *
     ************************************************************************/
    /* Get the internal EthercatMaster pointer. */
    EthercatMaster *getMaster();

    /* Read/Write EEPROM alias address. */
    uint16_t readSII_AliasAddress(uint32_t timeout_ms = 500);
    int writeSII_AliasAddress(uint16_t alias, uint32_t timeout_ms = 500);

    /* CoE SDO Download. (Non-Blocking) */
    int sdoDownload_Begin(uint16_t od_index, uint8_t od_subindex, void *data, uint32_t size, bool complete_access = false, uint32_t timeout_us = 2000000);
    int sdoDownload_Status();
    int sdoDownload_End();
    uint32_t sdoDownload_ReadAbortCode();
    /* CoE SDO Upload. (Non-Blocking) */
    int sdoUpload_Begin(uint16_t od_index, uint8_t od_subindex, void *data, uint32_t size, bool complete_access = false, uint32_t timeout_us = 2000000);
    int sdoUpload_Status();
    int sdoUpload_End();
    uint32_t sdoUpload_ReadAbortCode();
    uint32_t sdoUpload_ReadLength();
    /* CoE Get OD List. (Non-Blocking) */
    int getODlist_Begin(uint16_t *list, uint32_t list_size, uint32_t timeout_us = 2000000);
    int getODlist_Status();
    int getODlist_End();
    uint32_t getODlist_ReadAbortCode();
    uint32_t getODlist_ReadListSize();
    /* CoE Get Object Description. (Non-Blocking) */
    int getObjectDescription_Begin(uint16_t od_index, uint32_t timeout_us = 2000000);
    int getObjectDescription_Status();
    int getObjectDescription_End();
    uint32_t getObjectDescription_ReadAbortCode();
    uint16_t getObjectDescription_ReadDataType();
    uint8_t getObjectDescription_ReadMaxSubindex();
    uint8_t getObjectDescription_ReadObjectCode();
    char *getObjectDescription_ReadObjectName(char *name, size_t size);
    /* CoE Get Entry Description. (Non-Blocking) */
    int getEntryDescription_Begin(uint16_t od_index, uint8_t od_subindex, uint32_t timeout_us = 2000000);
    int getEntryDescription_Status();
    int getEntryDescription_End();
    uint32_t getEntryDescription_ReadAbortCode();
    uint8_t getEntryDescription_ReadValueInfo();
    uint16_t getEntryDescription_ReadDataType();
    uint16_t getEntryDescription_ReadBitLength();
    uint16_t getEntryDescription_ReadObjectAccess();
    char *getEntryDescription_ReadEntryName(char *name, size_t size);
    /* Get Last Abort Code. */
    uint32_t getLastAbortCode();

    /* EoE related. (Blocking). */
    int eoeRead(uint8_t port, MACframe frame, uint32_t timeout_us = 2000000);
    int eoeWrite(uint8_t port, MACframe frame, uint32_t size, uint32_t timeout_us = 2000000);
    int eoeGetIP(uint8_t port, EthercatEoE_IPParameter *param, uint32_t timeout_us = 2000000);
    int eoeSetIP(uint8_t port, EthercatEoE_IPParameter *param, uint16_t *result, uint32_t timeout_us = 2000000);
    int eoeGetAddressFilter(uint8_t port, EthercatEoE_AddressFilter *filter, uint32_t timeout_us = 2000000);
    int eoeSetAddressFilter(uint8_t port, EthercatEoE_AddressFilter *filter, uint16_t *result, uint32_t timeout_us = 2000000);

    /* EoE: Read. (Non-Blocking). */
    int eoeRead_Begin(uint8_t port, MACframe frame, uint32_t timeout_us = 2000000);
    int eoeRead_Status();
    int eoeRead_End();
    uint32_t eoeRead_ReadLength();
    /* EoE: Write. (Non-Blocking). */
    int eoeWrite_Begin(uint8_t port, MACframe frame, uint32_t size, uint32_t timeout_us = 2000000);
    int eoeWrite_Status();
    int eoeWrite_End();
    /* EoE: Get IP parameters. (Non-Blocking). */
    int eoeGetIP_Begin(uint8_t port, EthercatEoE_IPParameter *param, uint32_t timeout_us = 2000000);
    int eoeGetIP_Status();
    int eoeGetIP_End();
    /* EoE: Set IP parameters. (Non-Blocking). */
    int eoeSetIP_Begin(uint8_t port, EthercatEoE_IPParameter *param, uint32_t timeout_us = 2000000);
    int eoeSetIP_Status();
    int eoeSetIP_End();
    uint16_t eoeSetIP_ReadResult();
    /* EoE: Get Address Filter. (Non-Blocking). */
    int eoeGetAddressFilter_Begin(uint8_t port, EthercatEoE_AddressFilter *filter, uint32_t timeout_us = 2000000);
    int eoeGetAddressFilter_Status();
    int eoeGetAddressFilter_End();
    /* EoE: Set Address Filter. (Non-Blocking). */
    int eoeSetAddressFilter_Begin(uint8_t port, EthercatEoE_AddressFilter *filter, uint32_t timeout_us = 2000000);
    int eoeSetAddressFilter_Status();
    int eoeSetAddressFilter_End();
    uint16_t eoeSetAddressFilter_ReadResult();

    /* Output/Input Process Data Size. */
    int pdoGetOutputBits();
    int pdoGetOutputBytes();
    int pdoGetInputBits();
    int pdoGetInputBytes();
    /* Read Slave Process Data Output. */
    int pdoBitReadOutput(uint32_t bit_offset);
    int pdoReadOutput(uint32_t offset, void *data, uint32_t size);
    uint8_t pdoReadOutput8(uint32_t offset);
    uint16_t pdoReadOutput16(uint32_t offset);
    uint32_t pdoReadOutput32(uint32_t offset);
    uint64_t pdoReadOutput64(uint32_t offset);

    int readESC(uint16_t ado, void *data, size_t len, uint32_t timeout_ms = 1000);
    uint8_t readESC8(uint16_t ado, uint32_t timeout_ms = 1000);
    uint16_t readESC16(uint16_t ado, uint32_t timeout_ms = 1000);
    uint32_t readESC32(uint16_t ado, uint32_t timeout_ms = 1000);
    uint64_t readESC64(uint16_t ado, uint32_t timeout_ms = 1000);
    int writeESC(uint16_t ado, void *data, size_t len, uint32_t timeout_ms = 1000);
    int writeESC8(uint16_t ado, uint8_t value, uint32_t timeout_ms = 1000);
    int writeESC16(uint16_t ado, uint16_t value, uint32_t timeout_ms = 1000);
    int writeESC32(uint16_t ado, uint32_t value, uint32_t timeout_ms = 1000);
    int writeESC64(uint16_t ado, uint64_t value, uint32_t timeout_ms = 1000);

    int enterBootState(uint32_t timeout_ms = 3000);
    int exitBootState(uint32_t timeout_ms = 3000);
    int getState(uint16_t *pLastStatusCode = NULL, uint32_t timeout_us = 100000);
    int setState(EthercatState state, uint32_t timeout_us = 100000);
    int resume(uint32_t timeout_us = 100000);

    virtual int update();
    virtual int remapping() { return 0; }
    virtual int unmapping() { return 0; }

protected:
    EthercatMaster::Passkey getPasskey() {
        return EthercatMaster::Passkey();
    }
    EthercatMaster *master;
    uint16_t slave_no;
    uint32_t vendorID;
    uint32_t productCode;

} _EthercatDevice_CommonDriver;

class EthercatDevice_Generic : public EthercatDevice {
public:
    EthercatDevice_Generic(uint32_t vendor = 0, uint32_t product = 0);
};

#endif
