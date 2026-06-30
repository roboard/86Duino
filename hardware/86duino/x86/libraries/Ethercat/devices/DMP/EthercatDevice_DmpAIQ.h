#ifndef ___WZ_ETHERCAT_DEVICE_DMP_AIQ_H
#define ___WZ_ETHERCAT_DEVICE_DMP_AIQ_H

#include "EthercatDevice_DmpMDP.h"
#include <stdint.h>

class EthercatDevice_QECR11A44S;    /*  4 Inputs,  4 Outputs. RJ45 power [1, 1]. */
class EthercatDevice_QECR11A40S;    /*  4 Inputs,  0 Outputs. RJ45 power [1, 1]. */
class EthercatDevice_QECR11A04S;    /*  0 Inputs,  4 Outputs. RJ45 power [1, 1]. */
class EthercatDevice_QECR00A44S;    /*  4 Inputs,  4 Outputs. RJ45 power [0, 0]. */

class EthercatDevice_DmpAIQ_Generic : public _EthercatDevice_DmpCommonDriver {

public:
    EthercatDevice_DmpAIQ_Generic(uint32_t product = 0);
    ~EthercatDevice_DmpAIQ_Generic();

    int attach(uint16_t slave_id, EthercatMaster *master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int attach(uint16_t slave_id, EthercatMaster &master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int detach();

    /*
        This function is an analog output function. The parameter 'value'
        is an int32_t type with a range from -2^31 to 2^31, which mapping
        range is as follows:

        - Voltage Mode: Maps to a voltage range of -64V to 64V.
        - Current Mode: Maps to a current range of -16A to 16A.
    */
    int analogWrite(int ch, int32_t value);
    /*
        This function is used for voltage output mode, and the parameter
        'voltage' is a double type, representing the desired output voltage
        value in volts (V).
    */
    int voltageWrite(int ch, double voltage);
    /*
        This function is used for current output mode, and the parameter
        'current' is a double type, representing the desired output current
        value in amperes (A).
    */
    int currentWrite(int ch, double current);

    /*
        This function maps an analog voltage input ranging from -64V to 64V to
        an integer range of -2^31 to 2^31.
    */
    int analogRead(int ch);
    /*
        The function returns the voltage value as a double, eliminating the
        necessity for any mapping process.
    */
    double voltageRead(int ch);

    /************************************************************************
     *  The following APIs are only for internal use.                       *
     ************************************************************************/
    /*
        The analog input voltage ranges, while not perfectly aligned with the
        ideal values due to hardware limitations, have been calibrated to
        approximate them.
    */
    #define ECAT_AI_PIN_MODE_N22_TO_P22_V   (0)     // ±22V (default)
    #define ECAT_AI_PIN_MODE_N20_TO_P20_V   (1)     // ±20V
    #define ECAT_AI_PIN_MODE_N11_TO_P11_V   (2)     // ±11V
    #define ECAT_AI_PIN_MODE_N10_TO_P10_V   (3)     // ±10V
    #define ECAT_AI_PIN_MODE_N5_TO_P5_V     (4)     // ±5V
    #define ECAT_AI_PIN_MODE_0_TO_22_V      (5)     // 0~22V
    #define ECAT_AI_PIN_MODE_0_TO_20_V      (6)     // 0~20V
    #define ECAT_AI_PIN_MODE_0_TO_11_V      (7)     // 0~11V
    #define ECAT_AI_PIN_MODE_0_TO_10_V      (8)     // 0~10V
    int aiPinMode(int ch, uint32_t range);

    /*
        The analog output voltage ranges.
    */
    #define ECAT_AO_PIN_MODE_0_TO_5_V       (0)     // Voltage output 0 to +5 V (default)
    #define ECAT_AO_PIN_MODE_0_TO_10_V      (1)     // Voltage output 0 to +10 V
    #define ECAT_AO_PIN_MODE_N5_TO_P5_V     (2)     // Voltage output ±5 V
    #define ECAT_AO_PIN_MODE_N10_TO_P10_V   (3)     // Voltage output ±10 V
    #define ECAT_AO_PIN_MODE_3p5_TO_23p5_mA (4)     // Current output 3.5 mA to 23.5 mA
    #define ECAT_AO_PIN_MODE_0_TO_20_mA     (5)     // Current output 0 to 20 mA
    #define ECAT_AO_PIN_MODE_0_TO_24_mA     (6)     // Current output 0 to 24 mA
    #define ECAT_AO_PIN_MODE_N24_TO_P24_mA  (7)     // Current output ±24 mA
    #define ECAT_AO_PIN_MODE_0_TO_6_V       (8)     // Voltage output 0 to +6 V
    #define ECAT_AO_PIN_MODE_0_TO_12_V      (9)     // Voltage output 0 to +12 V
    #define ECAT_AO_PIN_MODE_N6_TO_P6_V     (10)    // Voltage output ±6 V
    #define ECAT_AO_PIN_MODE_N12_TO_P12_V   (11)    // Voltage output ±12 V
    #define ECAT_AO_PIN_MODE_4_TO_20_mA     (12)    // Current output 4 mA to 20 mA
    int aoPinMode(int ch, uint32_t range);

    int getAiPinMode(int ch);
    int getAoPinMode(int ch);

protected:
    int nOutputChannels;
    int nInputChannels;

private:
    EthercatDevice_DmpMDP_Generic *_internalMDP;
    void *_privateData;
    int _attach(uint16_t slave_id, EthercatMaster *master, EthercatAttachMode mode);
    int _detach();
    int _analogWrite(int ch, int32_t value);
    int _voltageWrite(int ch, double voltage);
    int _currentWrite(int ch, double current);
    int _analogRead(int ch);
    double _voltageRead(int ch);
    int _aiPinMode(int ch, uint32_t range);
    int _aoPinMode(int ch, uint32_t range);
    int _getAiPinMode(int ch);
    int _getAoPinMode(int ch);

};

class EthercatDevice_QECR11A44S : public EthercatDevice_DmpAIQ_Generic {
public:
    EthercatDevice_QECR11A44S();
};

class EthercatDevice_QECR11A40S : public EthercatDevice_DmpAIQ_Generic {
public:
    EthercatDevice_QECR11A40S();
};

class EthercatDevice_QECR11A04S : public EthercatDevice_DmpAIQ_Generic {
public:
    EthercatDevice_QECR11A04S();
};

class EthercatDevice_QECR00A44S : public EthercatDevice_DmpAIQ_Generic {
public:
    EthercatDevice_QECR00A44S();
};

#endif
