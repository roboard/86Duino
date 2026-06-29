#ifndef ___WZ_ETHERCAT_DEVICE_DMP_DIQ_H
#define ___WZ_ETHERCAT_DEVICE_DMP_DIQ_H

#include "EthercatDevice_DmpCommon.h"
#include <stdint.h>

class EthercatDevice_QECR00D0FS;    /*  0 Inputs, 16 Outputs. RJ45 power [0, 0]. */
class EthercatDevice_QECR00D0FH;    /*  0 Inputs, 16 Outputs. RJ45 power [0, 0]. High speed digital I/O. */
class EthercatDevice_QECR00D0TL;    /*  0 Inputs, 32 Outputs. RJ45 power [0, 0]. Without MCU. */
class EthercatDevice_QECR00D0TH;    /*  0 Inputs, 32 Outputs. RJ45 power [0, 0]. High speed digital I/O. */
class EthercatDevice_QECR00DF0S;    /* 16 Inputs,  0 Outputs. RJ45 power [0, 0]. */
class EthercatDevice_QECR00DF0D;    /* 16 Inputs,  0 Outputs. RJ45 power [0, 0]. With broken wire detector. */
class EthercatDevice_QECR00DF0H;    /* 16 Inputs,  0 Outputs. RJ45 power [0, 0]. High speed digital I/O. */
class EthercatDevice_QECR00DT0L;    /* 32 Inputs,  0 Outputs. RJ45 power [0, 0]. Without MCU. */
class EthercatDevice_QECR00DT0H;    /* 32 Inputs,  0 Outputs. RJ45 power [0, 0]. High speed digital I/O. */
class EthercatDevice_QECR00D88S;    /*  8 Inputs,  8 Outputs. RJ45 power [0, 0]. */
class EthercatDevice_QECR00D88D;    /*  8 Inputs,  8 Outputs. RJ45 power [0, 0]. With broken wire detector. */
class EthercatDevice_QECR00D88H;    /*  8 Inputs,  8 Outputs. RJ45 power [0, 0]. High speed digital I/O. */
class EthercatDevice_QECR00DC4D;    /* 12 Inputs,  4 Outputs. RJ45 power [0, 0]. With broken wire detector. */
class EthercatDevice_QECR00D4CD;    /*  4 Inputs, 12 Outputs. RJ45 power [0, 0]. With broken wire detector. */
class EthercatDevice_QECR00DF0K;    /* 16 Inputs,  0 Outputs. RJ45 power [0, 0]. */
class EthercatDevice_QECR00D0FK;    /*  0 Inputs, 16 Outputs. RJ45 power [0, 0]. */
class EthercatDevice_QECR00D88K;    /*  8 Inputs,  8 Outputs. RJ45 power [0, 0]. */

class EthercatDevice_QECR11D0FS;    /*  0 Inputs, 16 Outputs. RJ45 power [1, 1]. */
class EthercatDevice_QECR11D0FH;    /*  0 Inputs, 16 Outputs. RJ45 power [1, 1]. High speed digital I/O. */
class EthercatDevice_QECR11D0TL;    /*  0 Inputs, 32 Outputs. RJ45 power [1, 1]. Without MCU. */
class EthercatDevice_QECR11D0TH;    /*  0 Inputs, 32 Outputs. RJ45 power [1, 1]. High speed digital I/O. */
class EthercatDevice_QECR11DF0S;    /* 16 Inputs,  0 Outputs. RJ45 power [1, 1]. */
class EthercatDevice_QECR11DF0D;    /* 16 Inputs,  0 Outputs. RJ45 power [1, 1]. With broken wire detector. */
class EthercatDevice_QECR11DF0H;    /* 16 Inputs,  0 Outputs. RJ45 power [1, 1]. High speed digital I/O. */
class EthercatDevice_QECR11DT0L;    /* 32 Inputs,  0 Outputs. RJ45 power [1, 1]. Without MCU. */
class EthercatDevice_QECR11DT0H;    /* 32 Inputs,  0 Outputs. RJ45 power [1, 1]. High speed digital I/O. */
class EthercatDevice_QECR11D88S;    /*  8 Inputs,  8 Outputs. RJ45 power [1, 1]. */
class EthercatDevice_QECR11D88D;    /*  8 Inputs,  8 Outputs. RJ45 power [1, 1]. With broken wire detector. */
class EthercatDevice_QECR11D88H;    /*  8 Inputs,  8 Outputs. RJ45 power [1, 1]. High speed digital I/O. */
class EthercatDevice_QECR11DF0K;    /* 16 Inputs,  0 Outputs. RJ45 power [0, 0]. */
class EthercatDevice_QECR11D0FK;    /*  0 Inputs, 16 Outputs. RJ45 power [0, 0]. */
class EthercatDevice_QECR11D88K;    /*  8 Inputs,  8 Outputs. RJ45 power [0, 0]. */

#define LOW     (0x00)
#define HIGH    (0x01)

class EthercatDevice_DmpDIQ_Generic : public _EthercatDevice_DmpCommonDriver {

public:
    EthercatDevice_DmpDIQ_Generic(uint32_t product = 0);
    ~EthercatDevice_DmpDIQ_Generic();

    int attach(uint16_t slave_id, EthercatMaster *master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int attach(uint16_t slave_id, EthercatMaster &master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int detach();

    int digitalWrite(int pin, uint8_t value);
    int digitalRead(int pin);
    int digitalWriteAll(uint32_t value);
    uint32_t digitalReadAll();
    uint16_t startBrokenWireTest(uint16_t bitMask, uint32_t timeout_ms = 1000);

protected:
    bool brokenWireDetect;

};

class EthercatDevice_QECR00DC4D : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR00DC4D();
};

class EthercatDevice_QECR00D4CD : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR00D4CD();
};

class EthercatDevice_QECR11D0FS : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR11D0FS();
};

class EthercatDevice_QECR00D0FS : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR00D0FS();
};

class EthercatDevice_QECR11DF0D : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR11DF0D();
};

class EthercatDevice_QECR00DF0D : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR00DF0D();
};

class EthercatDevice_QECR11D88S : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR11D88S();
};

class EthercatDevice_QECR00D88D : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR00D88D();
};

class EthercatDevice_QECR11D88D : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR11D88D();
};

class EthercatDevice_QECR11D88H : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR11D88H();
};

class EthercatDevice_QECR11D0FH : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR11D0FH();
};

class EthercatDevice_QECR11DF0H : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR11DF0H();
};

class EthercatDevice_QECR00D0FH : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR00D0FH();
};

class EthercatDevice_QECR00DF0H : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR00DF0H();
};

class EthercatDevice_QECR11DT0L : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR11DT0L();
};

class EthercatDevice_QECR11D0TL : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR11D0TL();
};

class EthercatDevice_QECR11DT0H : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR11DT0H();
};

class EthercatDevice_QECR11D0TH : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR11D0TH();
};

class EthercatDevice_QECR00D88S : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR00D88S();
};

class EthercatDevice_QECR00DT0L : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR00DT0L();
};

class EthercatDevice_QECR00D0TL : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR00D0TL();
};

class EthercatDevice_QECR00DT0H : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR00DT0H();
};

class EthercatDevice_QECR00D0TH : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR00D0TH();
};

class EthercatDevice_QECR00D88H : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR00D88H();
};

class EthercatDevice_QECR00DF0S : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR00DF0S();
};

class EthercatDevice_QECR11DF0S : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR11DF0S();
};

class EthercatDevice_QECR00DF0K : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR00DF0K();
};

class EthercatDevice_QECR00D0FK : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR00D0FK();
};

class EthercatDevice_QECR00D88K : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR00D88K();
};

class EthercatDevice_QECR11DF0K : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR11DF0K();
};

class EthercatDevice_QECR11D0FK : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR11D0FK();
};

class EthercatDevice_QECR11D88K : public EthercatDevice_DmpDIQ_Generic {
public:
    EthercatDevice_QECR11D88K();
};

#endif
