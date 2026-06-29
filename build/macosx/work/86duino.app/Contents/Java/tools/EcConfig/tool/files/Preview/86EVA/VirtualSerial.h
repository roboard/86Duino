#ifndef __VIRTUAL_SERIAL_H
#define __VIRTUAL_SERIAL_H

#include "Arduino.h"
#include "Ethercat.h"
#include "SerialInterface.h"

typedef struct privateSerialFunction {
    int (*update)(void* slave);
    int (*uartSetBaud)(void* slave, int dev, int baudrate);
    int (*uartSetFormat)(void* slave, int dev, int config);
    size_t (*uartWrite)(void* slave, int dev, uint8_t value);
    size_t (*uartSend)(void* slave, int dev, uint8_t* buf, size_t size);
    int (*uartRead)(void* slave, int dev);
    int (*uartQueryRxQueue)(void* slave, int dev);
    int (*uartClearTxQueue)(void* slave, int dev);
    int (*uartClearRxQueue)(void* slave, int dev);
} PSF;

class MyVirtualSerial : public SerialInterface {
private:
    void* _slave;
    int dev, peek_val;
    unsigned int updateCount;
    bool peek_stored;
    PSF* priv;
    
public:
    MyVirtualSerial();
    void init(void* slave, int device, PSF* psf = NULL);
    int begin(int baudrate, int config=SERIAL_8N1);
    size_t write(uint8_t value);
    size_t write(const uint8_t *buf, size_t size);
    int available();
    int peek(void);
    int read(void);
    void flush(int w=0);
    inline size_t write(unsigned short n) { return write((uint8_t)n); }
    inline size_t write(short n) { return write((uint8_t)n); }
    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
};

// BarCode Scanner
enum BarcodeDevice {
    HONEYWELL_N4680 = 0x1001,
    HONEYWELL_HF521 = 0x1002
};

enum BarcodeState {
    IDLE,
    SCANNING,
    INIT
};

#define DEFAULT_BAUD    (0)
#define USE_MYVS_BAUD   (1)

class BarcodeScanner
{
    private:
        SerialInterface* serialport;
        int   _Mode;
        char* _buf;
        BarcodeState _state;
        unsigned long _triIv, _tst, _trt;
        void sendCommand(char* command);
        
    public:
        BarcodeScanner();
        BarcodeScanner(BarcodeDevice dev, SerialInterface &serialport);
        bool  begin(BarcodeDevice dev, SerialInterface &serialport, unsigned long triggerInterval=0, unsigned int new_baud = USE_MYVS_BAUD); // for ArduBlock
        bool  begin(unsigned long triggerInterval=0);
        void  triggerScan(); // not include in ArduBlock
        void  stopScan(); // not include in ArduBlock
        void  enableBeep(); // not include in ArduBlock
        void  disableBeep(); // not include in ArduBlock
        void  command(char* command); // not include in ArduBlock
        int   available();
        int   read();
        int   readLine(char* buf, int size); // not include in ArduBlock
        char* readLine();
};

// ForceSensor
enum ForceSensorDevice {
    DM320 = 0x1001
};

enum ForceSensorState {
    FSIDLE,
    FSREADY1,
    FSREADY2,
    FSRECV1,
    FSRECV2,
};

enum ForceSensorCommand {
    FCMD_RESET = 0,
    FCMD_SET_ID,
    FCMD_SET_FOOTSIZE,
    FCMD_GET_FOOTSIZE,
    FCMD_WRITE_0Para,
    FCMD_READ_0Para,
    FCMD_WRITE_1Para,
    FCMD_READ_1Para,
    FCMD_WRITE_2Para,
    FCMD_READ_2Para,
    FCMD_SET_BAUD,
    FCMD_WRITE_3Para,
    FCMD_READ_3Para,
    
    FCMD_GET_RAWDATA = 0xFE,
    FCMD_GET_FORCEDATA = 0xFF,
    NO_FCMD
};

union float2byte {
    unsigned char bytes[4];
    float value;
};

enum ForceOperationMode {
    F_ETHERCAT = 0,
    F_HWSERIAL,
    F_NOSETMODE
};

#define LITTLE_END      (1)
#define BIG_END         (2)

#define DEFAULT_BAUD    (0)
#define USE_MYVS_BAUD   (1)

#define MASTER_MODE    (0)
#define SLAVE_MODE     (1)

class _ForceSensor
{
    private:
        SerialInterface* serialport;
        ForceOperationMode  _FMode;
        unsigned char* _commandBuf;
        unsigned char* _recvBuf;
        int _commLen;
        int _recvlen;
        
        ForceSensorState _state;
        
        int _newid;
        int _Mode;
        int _id;
        int _F0;
        int _F1;
        int _F2;
        int _Fz;
        int _Mx;
        int _My;
        float _footLength;
        float _footWidth;
        unsigned long _t;
        unsigned long _timeout;
        int _baudIdx;
        int _refreshRate;
        bool _result;
        
        ForceSensorCommand pre_command;
        void _handleData();
        bool _si_update();
        bool update();
        
        bool _begined;
        
    public:
        _ForceSensor(unsigned long timeout = 1000);
        _ForceSensor(ForceSensorDevice dev, SerialInterface &serialport, unsigned long timeout = 1000);
        bool begin(ForceSensorDevice dev, SerialInterface &serialport, int id = 0, unsigned int new_baud = USE_MYVS_BAUD); // for ArduBlock
        bool begin(int id = 1);
        // void setRefreshRate(unsigned char t);
        // void setMasterMode(void);
        bool reset();
        bool setNewID(int newid);
        bool setFootSize(unsigned long length, unsigned long width);
        bool getFootSize(float* length, float* width);
        bool setBaudrate(int newBaud);
        // void autoCalibration(void);
        void refresh(bool block = false);
        double getMx(bool block = false);
        double getMy(bool block = false);
        double getFz(bool block = false);
        double getF0Raw(bool block = false);
        double getF1Raw(bool block = false);
        double getF2Raw(bool block = false);
        // bool btnPressed(bool block = false);
};

extern _ForceSensor ForceSensor;

// JoyStick
enum JoystickDevice {
    SMC30A1 = 0x1001,
    SMC30B1 = 0x1002,
    SMC30C1 = 0x1003,
    SMC30D1 = 0x1004
};

enum JoystickState {
    JSIDLE,
    JSRECV
};

enum JoystickCommand {
    NO_JCMD,
    JCMD_SET_ADDR,
    JCMD_REFIX,
    JCMD_RESET,
    JCMD_SEL_PRTCL,
    JCMD_REF_RATE,
    JCMD_SET_MODE,
    JCMD_GET_INFO,
    JCMD_SET_BAUD,
    JCMD_GET_STAT
};

enum JoystickOperationMode {
    J_ETHERCAT = 0,
    J_HWSERIAL,
    J_NOSETMODE
};

#define DEFAULT_BAUD    (0)
#define USE_MYVS_BAUD   (1)

class _3DJoystick
{
    private:
        SerialInterface* serialport;
        JoystickOperationMode _Mode;
        unsigned char* _buf;
        
        JoystickState _state;
        int _JMode;
        int _addr;
        int _X;
        int _Y;
        int _Z;
        bool _btnPressed;
        unsigned long _t;
        unsigned long _timeout;
        int _baudIdx;
        int _refreshRate;
        
        JoystickCommand pre_command;
        bool _si_update(JoystickCommand cmd);
        bool update(JoystickCommand cmd);
        
        bool _begined;
        
    public:
        _3DJoystick(unsigned long timeout = 1000);
        _3DJoystick(JoystickDevice dev, SerialInterface &serialport, unsigned long timeout = 1000);
        bool begin(JoystickDevice dev, SerialInterface &serialport, int addr = 1, unsigned int new_baud = USE_MYVS_BAUD); // for ArduBlock
        bool begin(int adde = 1);
        void setRefreshRate(unsigned char t);
        void setMasterMode(void);
        void setBaudrate(int newBaud);
        void autoCalibration(void);
        int  getX(bool block = false);
        int  getY(bool block = false);
        int  getZ(bool block = false);
        bool btnPressed(bool block = false);
        
};

extern _3DJoystick Joystick;

// RFID
enum RFIDDevice {
    RC522 = 0x1001,
    PN532 = 0x1002
};

enum RFIDOperationMode {
    R_ETHERCAT = 0x01,
    R_HWSERIAL,
    R_NOSETMODE
};

enum RfidState {
    RSIDLE,
    RSREADY1,
    RSREADY2,
    RSRECV1,
    RSRECV2,
};

enum CARDID {
    MIFARE_CLASSIC,
    NTAG213,
    MIFARE_Ultralight_C
};

enum CARDTYPE {
    PICC_TYPE_MIFARE_MINI = 3,
    PICC_TYPE_MIFARE_1K,
    PICC_TYPE_MIFARE_4K,
    PICC_TYPE_MIFARE_ULC,
    PICC_TYPE_MIFARE_UL,
    PICC_TYPE_NTAG203,
    PICC_TYPE_NTAG213,
    PICC_TYPE_NTAG215,
    PICC_TYPE_NTAG216,
    UNKNOWN_TYPE = 0xFF
};

enum CARDCOMMAND {
    RCMD_SCAN_CARD,
    RCMD_SELECT_CARD,
    RCMD_READ_CARD,
    RCMD_COMPATIBILITY_WRITE,
    RCMD_WRITE_CARD,
    RCMD_MIFR_CLASSIC_AUTHE,
    RCMD_MIFR_UC_AUTHE,
    RCMD_MIFR_NTAG21X_AUTHE,
    RCMD_BEEP = 0xF0,
    RCMD_RFLED,
    NO_RCMD
};

struct _RFIDinfo {
    int type;
    int memsize;
    int uid_size;
    unsigned char uid[7];
    int bank;
    int section;
};

class ECRFID
{
    private:
        SerialInterface* serialport;
        RFIDOperationMode  _RMode;
        unsigned char* _commandBuf;
        unsigned char* _recvBuf;
        int _commLen;
        int _recvlen;
        int _blockingLevel;
        
        RfidState _state;
        
        struct _RFIDinfo myRFID;
        unsigned long _t;
        unsigned long _timeout;
        char _readbuf[8192];
        int _readLen;
        bool _result;
        int _id;
        
        int _available();
        void _handleData();
        bool _si_update();
        bool update();
        bool write(CARDCOMMAND cmd, char address, char* userData, int userDataSize);
        
        bool _begined;
        
    public:
        ECRFID(int blockingLevel = 2, unsigned long timeout = 1000);
        bool begin(RFIDDevice dev, SerialInterface &serialport, int id = -1); // for ArduBlock
        
        bool scan();
        // void update();
        int getType();
        int getMemSize();
        int getUIDSize();
        unsigned char getUIDByte(int index);
        int getDataLen();
        bool read(char address);
        bool selectCard(char uidSize, char* uid);
        bool write4B(char address, char* userData);
        bool write16B(char address, char* userData);
        bool MIFR_CA(char sector, char keyNum, char* userData);
        bool MIFR_UCA(char* userData);
        bool MIFR_21X_AUTHE(char* userData);
        bool Beep(char t);
        bool Led(char which, bool onoff);
        bool available();
        void _printResult();
        int readData(char* data, unsigned int len=16);
};
#endif
