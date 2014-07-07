#ifndef CANBUS_H
#define CANBUS_H

#include <Arduino.h>

//#ifndef __86DUINO_ONE
//#error CAN bus library is only used on 86Duino ONE!
//#endif

#define CAN_10KBPS         (CAN_BPS_10K)
#define CAN_20KBPS         (CAN_BPS_20K)
#define CAN_50KBPS         (CAN_BPS_50K)
#define CAN_83K3BPS        (CAN_BPS_83K3)
#define CAN_100KBPS        (CAN_BPS_100K)
#define CAN_125KBPS        (CAN_BPS_125K)
#define CAN_250KBPS        (CAN_BPS_250K)
#define CAN_500KBPS        (CAN_BPS_500K)
#define CAN_833KBPS        (CAN_BPS_833K)
#define CAN_1000KBPS       (CAN_BPS_1000K)

#define CAN_STDID          (STD_DATA_FRAME)
#define CAN_EXTID          (EXT_DATA_FRAME)
#define CAN_STDID_REMOTE   (STD_REMOTE_FRAME)
#define CAN_EXTID_REMOTE   (EXT_REMOTE_FRAME)

#define CAN_OK             (0)
#define CAN_MSGAVAIL       (3)
#define CAN_NOMSG          (4)
#define CAN_FAIL           (-1)

#define CAN_TX_BUFFER_MAX_SIZE (512)

class Vortex86CAN
{
  private:
	COMPort *can;
	uint32_t _remoteID;
	uint8_t _remoteExt;
	
	CANFrame TxFrame;
	CANFrame RxFrame;
	uint8_t TxBuf[CAN_TX_BUFFER_MAX_SIZE];
	int TxSize;
	int RxIndex;
	int RxSize;

  public:
	Vortex86CAN();
    void begin();
    void begin(uint32_t speed);
    
    void beginTransmission(uint32_t id, uint8_t ext = CAN_STDID);
    uint8_t endTransmission(void);
    uint8_t requestFrom(void);
    virtual size_t write(uint8_t val);
    virtual size_t write(const uint8_t *buf, size_t len);
    virtual int available(void);
    virtual int read(void);
    virtual int peek(void);
	virtual void flush(void);
	
	virtual uint8_t remoteExt() { return _remoteExt; };
	virtual uint32_t remoteID() { return _remoteID; };
	
	uint8_t init_Mask(uint8_t num, uint8_t ext, uint32_t ulData);            /* init Masks */
    uint8_t init_Filt(uint8_t num, uint8_t ext, uint32_t ulData);            /* init filters */
    uint8_t sendMsgBuf(uint32_t id, uint8_t ext, uint8_t len, uint8_t *buf); /* send buf */
    uint8_t readMsgBuf(uint8_t *len, uint8_t *buf);                          /* read buf */
    uint8_t checkReceive(void);                                              /* if something received */
    uint32_t getCanId(void);                                                 /* get can id when receive */
};

extern Vortex86CAN CAN;

#endif
