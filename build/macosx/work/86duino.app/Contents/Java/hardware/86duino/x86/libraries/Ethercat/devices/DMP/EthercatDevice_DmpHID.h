#ifndef ___WZ_ETHERCAT_DEVICE_DMP_HID_H
#define ___WZ_ETHERCAT_DEVICE_DMP_HID_H

#include "EthercatDevice_DmpCommon.h"
#include <stdint.h>

class EthercatDevice_QECR11HU1S;
class EthercatDevice_QECR00HU1S;
class EthercatDevice_QECR11HU5S;
class EthercatDevice_QECR00HU5S;
class EthercatDevice_QECR11HU8S;
class EthercatDevice_QECR00HU8S;
class EthercatDevice_QECR11HU9S;
class EthercatDevice_QECR00HU9S;

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

#define ECAT_MPG_AXIS_CHANGE            (1)
#define ECAT_MPG_RATIO_CHANGE           (2)
#define ECAT_MPG_EMERGENCY_STOP_CHANGE  (3)
#define ECAT_MPG_ENABLE_SWITCH_CHANGE   (4)
#define ECAT_MPG_ENCODER_CHANGE         (5)

class EthercatDevice_DmpHID_Generic : public _EthercatDevice_DmpCommonDriver {

public:
    EthercatDevice_DmpHID_Generic(uint32_t product = 0);
    ~EthercatDevice_DmpHID_Generic();

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
     *                    LCM                    *
     *********************************************/
    int lcmHeight();
    int lcmWidth();
    int lcmWordWrap(bool wrap);
    int lcmGotoXY(int x, int y);
    int lcmClear();
    int lcmPrint(const char *fmt, ...);
    int lcmWrite(char c);

    /*********************************************
     *                  Keypad                   *
     *********************************************/
    int keypadSetTimeout(uint32_t timeout_ms);
    int keypadClear();
    int keypadRead();

    /*********************************************
     *             Handwheel Counter             *
     *********************************************/
    int mpgSetCallback(void (*callback)(int));
    int mpgSetNoiseFilter(uint32_t time_us);
    int mpgInvertEncoderDirection(bool invert);
    int mpgWriteEncoderRaw(int32_t value);
    int mpgWriteEncoder(int32_t value);
    int32_t mpgReadEncoderRaw();
    int32_t mpgReadEncoder();
    int mpgReadEmergencyStop();
    int mpgReadEnableSwitch();
    int mpgReadAxis();
    int mpgReadRatio();

    /*********************************************
     *                  Buzzer                   *
     *********************************************/
    int buzzer(uint32_t hz, uint32_t duration_ms = 0);

protected:
    bool mpgEnable;
    bool lcmEnable;
    bool uartEnable;
    bool keypadEnable;

private:
    void *private_data;

};

class EthercatDevice_QECR11HU1S : public EthercatDevice_DmpHID_Generic {
public:
    EthercatDevice_QECR11HU1S();
};

class EthercatDevice_QECR00HU1S : public EthercatDevice_DmpHID_Generic {
public:
    EthercatDevice_QECR00HU1S();
};

class EthercatDevice_QECR11HU5S : public EthercatDevice_DmpHID_Generic {
public:
    EthercatDevice_QECR11HU5S();
};

class EthercatDevice_QECR00HU5S : public EthercatDevice_DmpHID_Generic {
public:
    EthercatDevice_QECR00HU5S();
};

class EthercatDevice_QECR11HU8S : public EthercatDevice_DmpHID_Generic {
public:
    EthercatDevice_QECR11HU8S();
};

class EthercatDevice_QECR00HU8S : public EthercatDevice_DmpHID_Generic {
public:
    EthercatDevice_QECR00HU8S();
};

class EthercatDevice_QECR11HU9S : public EthercatDevice_DmpHID_Generic {
public:
    EthercatDevice_QECR11HU9S();
};

class EthercatDevice_QECR00HU9S : public EthercatDevice_DmpHID_Generic {
public:
    EthercatDevice_QECR00HU9S();
};

#endif
