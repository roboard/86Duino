#ifndef ___WZ_ETHERCAT_DEVICE_DMP_ACPWR_H
#define ___WZ_ETHERCAT_DEVICE_DMP_ACPWR_H

#include "EthercatDevice_DmpCommon.h"
#include <stdint.h>

class EthercatDevice_QECR11ACPS;
class EthercatDevice_QECR11ACSS;

class EthercatDevice_DmpACPWR_Generic : public _EthercatDevice_DmpCommonDriver {

public:
    EthercatDevice_DmpACPWR_Generic(uint32_t product = 0);
    ~EthercatDevice_DmpACPWR_Generic();

    int attach(uint16_t slave_id, EthercatMaster *master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int attach(uint16_t slave_id, EthercatMaster &master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int detach();

    int isValid();
    double getMinVoltage(bool clear_record = false);
    double getMaxCurrent(bool clear_record = false);

    int configVoltageSetting(double voltage);
    double getVoltageSetting();

private:
    void *private_data;

};

class EthercatDevice_QECR11ACPS : public EthercatDevice_DmpACPWR_Generic {
public:
    EthercatDevice_QECR11ACPS();
};

class EthercatDevice_QECR11ACSS : public EthercatDevice_DmpACPWR_Generic {
public:
    EthercatDevice_QECR11ACSS();
};

#endif
