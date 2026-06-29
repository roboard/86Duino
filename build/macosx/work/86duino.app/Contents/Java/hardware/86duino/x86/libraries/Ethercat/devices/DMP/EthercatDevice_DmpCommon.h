#ifndef ___WZ_ETHERCAT_DEVICE_DMP_COMMON_H
#define ___WZ_ETHERCAT_DEVICE_DMP_COMMON_H

#include "EthercatDevice.h"

class _EthercatDevice_DmpCommonDriver : public EthercatDevice {

public:
    _EthercatDevice_DmpCommonDriver(uint32_t product = 0);
    ~_EthercatDevice_DmpCommonDriver();

    virtual int attach(uint16_t slave_id, EthercatMaster *master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    virtual int attach(uint16_t slave_id, EthercatMaster &master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    virtual int detach();

    /* Order Information. */
    char *getCustomer(char *buffer = NULL);
    char *getOrderNumber(char *buffer = NULL);
    char *getInvoiceNumber(char *buffer = NULL);
    char *getDeliveryDate(char *buffer = NULL);

    /* MTBF. */
    int getWorkingHours();
    int getBootTimes();

    bool tryToGetWorkingHours();
    bool tryToGetBootTimes();

    /* System Monitoring. */
    double getSystemTemperature();
    double getSystemPowerVoltage();
    double getSystemPowerCurrent();
    double getPeripheralPowerVoltage();
    double getPeripheralPowerCurrent();

    bool tryToGetSystemTemperature();
    bool tryToGetSystemPowerVoltage();
    bool tryToGetSystemPowerCurrent();
    bool tryToGetPeripheralPowerVoltage();
    bool tryToGetPeripheralPowerCurrent();

    /*************************************************************************
     *  The following APIs are still under development and are not           *
     *  recommended for use.                                                 *
     ************************************************************************/
    #define DMP_STATUS_NORMAL_OPERATION             (0x00)
    #define DMP_STATUS_SYSTEM_V3P3_NG               (0x03)
    #define DMP_STATUS_PERIPHERAL_V3P3_NG           (0x04)
    #define DMP_STATUS_EXTERNAL_CRYSTAL_STOP        (0x05)
    #define DMP_STATUS_EXTERNAL_CRYSTAL_OVER_RANGE  (0x06)
    #define DMP_STATUS_PROGRAM_CHECKSUM_ERROR       (0x07)
    #define DMP_STATUS_POWER_WARNING                (0x10)
    #define DMP_STATUS_POWER_OVER_RANGE             (0x11)
    int getDeviceStatus();

protected:
    bool detectSystemTemperature;
    bool detectSystemPower;
    bool detectPeripheralPower;
    bool detectDeviceStatus;
    bool withOrderInformation;
    bool withMTBF;

private:
    void *private_data;
    char orderInfo_Customer[7];
    char orderInfo_OrderNumber[9];
    char orderInfo_InvoiceNumber[12];
    char orderInfo_DeliveryDate[5];

};

#endif
