#ifndef ___WZ_ETHERCAT_DEVICE_DMP_CIQ_H
#define ___WZ_ETHERCAT_DEVICE_DMP_CIQ_H

#include "EthercatDevice_DmpCommon.h"
#include <stdint.h>

class EthercatDevice_QECR00CFFG;
class EthercatDevice_QECR11CFFG;
class EthercatDevice_QECR00CFFU;
class EthercatDevice_QECR11CFFU;
class EthercatDevice_QECR00CFFL;
class EthercatDevice_QECR11CFFL;
class EthercatDevice_QECR00CT0U;
class EthercatDevice_QECR11CT0U;
class EthercatDevice_QECR00C0TU;
class EthercatDevice_QECR11C0TU;

#define ECAT_UART_BYTESIZE5             (0x00)
#define ECAT_UART_BYTESIZE6             (0x01)
#define ECAT_UART_BYTESIZE7             (0x02)
#define ECAT_UART_BYTESIZE8             (0x03)
#define ECAT_UART_STOPBIT1              (0x00)
#define ECAT_UART_STOPBIT2              (0x04)
#define ECAT_UART_NOPARITY              (0x00)
#define ECAT_UART_ODDPARITY             (0x08)
#define ECAT_UART_EVENPARITY            (0x18)
#define ECAT_UART_MARKPARITY            (0x28)
#define ECAT_UART_SPACEPARITY           (0x38)
#define ECAT_UART_NO_CONTROL            (0)
#define ECAT_UART_RTS_CTS               (1)

#define LOW     (0x00)
#define HIGH    (0x01)

class EthercatDevice_DmpCIQ_Generic : public _EthercatDevice_DmpCommonDriver {

public:
    EthercatDevice_DmpCIQ_Generic(uint32_t product = 0);
    ~EthercatDevice_DmpCIQ_Generic();

    int attach(uint16_t slave_id, EthercatMaster *master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int attach(uint16_t slave_id, EthercatMaster &master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int detach();
    int update();

    /*********************************************
     *                   UART                    *
     *********************************************/
    int uartIs485(int dev);
    int uartSetBaud(int dev, uint32_t baud);
    int uartSetFormat(int dev, uint8_t format);
    int uartSetFlowControl(int dev, int control);
    int uartGetRTS(int dev);
    int uartGetCTS(int dev);
    int uartGetDTR(int dev);
    int uartGetDSR(int dev);
    int uartSetRTS(int dev, uint8_t value);
    int uartSetDTR(int dev, uint8_t value);
    int uartClearFIFO(int dev);
    int uartClearTxQueue(int dev);
    int uartClearRxQueue(int dev);
    int uartQueryTxQueue(int dev);
    int uartQueryRxQueue(int dev);
    int uartTxQueueEmpty(int dev);
    int uartRxQueueEmpty(int dev);
    int uartTxQueueFull(int dev);
    int uartRxQueueFull(int dev);
    int uartSend(int dev, void *buf, size_t size);
    int uartWrite(int dev, uint8_t value);
    int uartReceive(int dev, void *buf, size_t size);
    int uartRead(int dev);

    /*********************************************
     *                    DIO                    *
     *********************************************/
    int digitalWrite(int pin, uint8_t value);
    int digitalRead(int pin);
    int digitalWriteAll(uint32_t value);
    uint32_t digitalReadAll();

    /*********************************************
     *                    AIO                    *
     *********************************************/
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
    int nDigitalOutputChannels;
    int nDigitalInputChannels;
    int nAnalogOutputChannels;
    int nAnalogInputChannels;
    int nUART;

private:
    void *private_data;

};

class EthercatDevice_QECR00CFFG : public EthercatDevice_DmpCIQ_Generic {
public:
    EthercatDevice_QECR00CFFG();
};

class EthercatDevice_QECR11CFFG : public EthercatDevice_DmpCIQ_Generic {
public:
    EthercatDevice_QECR11CFFG();
};

class EthercatDevice_QECR00CFFU : public EthercatDevice_DmpCIQ_Generic {
public:
    EthercatDevice_QECR00CFFU();
};

class EthercatDevice_QECR11CFFU : public EthercatDevice_DmpCIQ_Generic {
public:
    EthercatDevice_QECR11CFFU();
};

class EthercatDevice_QECR00CFFL : public EthercatDevice_DmpCIQ_Generic {
public:
    EthercatDevice_QECR00CFFL();
};

class EthercatDevice_QECR11CFFL : public EthercatDevice_DmpCIQ_Generic {
public:
    EthercatDevice_QECR11CFFL();
};

class EthercatDevice_QECR00CT0U : public EthercatDevice_DmpCIQ_Generic {
public:
    EthercatDevice_QECR00CT0U();
};

class EthercatDevice_QECR11CT0U : public EthercatDevice_DmpCIQ_Generic {
public:
    EthercatDevice_QECR11CT0U();
};

class EthercatDevice_QECR00C0TU : public EthercatDevice_DmpCIQ_Generic {
public:
    EthercatDevice_QECR00C0TU();
};

class EthercatDevice_QECR11C0TU : public EthercatDevice_DmpCIQ_Generic {
public:
    EthercatDevice_QECR11C0TU();
};

/*
    The class name EthercatDevice_QECR11DFFG is obsolete but retained for
    compatibility.
*/
typedef class EthercatDevice_QECR11CFFG EthercatDevice_QECR11DFFG;

#endif
