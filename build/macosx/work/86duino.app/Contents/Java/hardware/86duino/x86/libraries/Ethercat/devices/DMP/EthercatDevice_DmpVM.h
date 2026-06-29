#ifndef ___WZ_ETHERCAT_DEVICE_DMP_VM_H
#define ___WZ_ETHERCAT_DEVICE_DMP_VM_H

#include "EthercatDevice_DmpCommon.h"
#include <stdlib.h>
#include <stdint.h>

class EthercatDevice_QECR11VM8S;    /*  8-channel voltage inputs. RJ45 power [1, 1]. */
class EthercatDevice_QECR11VM6S;    /*  6-channel voltage inputs. RJ45 power [1, 1]. */
class EthercatDevice_QECR11VM4S;    /*  4-channel voltage inputs. RJ45 power [1, 1]. */
class EthercatDevice_QECR11VM2S;    /*  2-channel voltage inputs. RJ45 power [1, 1]. */
class EthercatDevice_QECR00VM8S;    /*  8-channel voltage inputs. RJ45 power [0, 0]. */
class EthercatDevice_QECR00VM6S;    /*  6-channel voltage inputs. RJ45 power [0, 0]. */
class EthercatDevice_QECR00VM4S;    /*  4-channel voltage inputs. RJ45 power [0, 0]. */
class EthercatDevice_QECR00VM2S;    /*  2-channel voltage inputs. RJ45 power [0, 0]. */

class EthercatDevice_DmpVM_Generic : public _EthercatDevice_DmpCommonDriver {

public:
    EthercatDevice_DmpVM_Generic(uint32_t product = 0);
    ~EthercatDevice_DmpVM_Generic();

    int attach(uint16_t slave_id, EthercatMaster *master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int attach(uint16_t slave_id, EthercatMaster &master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int detach();

    double voltageRead(size_t channel);

protected:
    size_t vmChannels;

};

class EthercatDevice_QECR11VM8S : public EthercatDevice_DmpVM_Generic {
public:
    EthercatDevice_QECR11VM8S();
};

class EthercatDevice_QECR11VM6S : public EthercatDevice_DmpVM_Generic {
public:
    EthercatDevice_QECR11VM6S();
};

class EthercatDevice_QECR11VM4S : public EthercatDevice_DmpVM_Generic {
public:
    EthercatDevice_QECR11VM4S();
};

class EthercatDevice_QECR11VM2S : public EthercatDevice_DmpVM_Generic {
public:
    EthercatDevice_QECR11VM2S();
};

class EthercatDevice_QECR00VM8S : public EthercatDevice_DmpVM_Generic {
public:
    EthercatDevice_QECR00VM8S();
};

class EthercatDevice_QECR00VM6S : public EthercatDevice_DmpVM_Generic {
public:
    EthercatDevice_QECR00VM6S();
};

class EthercatDevice_QECR00VM4S : public EthercatDevice_DmpVM_Generic {
public:
    EthercatDevice_QECR00VM4S();
};

class EthercatDevice_QECR00VM2S : public EthercatDevice_DmpVM_Generic {
public:
    EthercatDevice_QECR00VM2S();
};

#endif
