#ifndef __SERVOBUS_h
#define __SERVOBUS_h

#include "Arduino.h"

typedef struct _AIServoList {
	volatile int state;
	volatile int speed;
	unsigned char id;
	volatile unsigned long starttime;
	volatile unsigned long endtime;
	volatile unsigned long pausetime;
	volatile unsigned long totaltime;
	volatile long long offset;
	volatile long long mixoffset;
	volatile long long acc_prev_out;
	volatile long long acc_now_in;
	volatile long long acc_now_out;
	volatile long long prevposition;
	volatile long long curposition;
	volatile long long targetposition;
	struct _AIServoList* next;
} AIServoList;

class AIServoBus {
public:
	AIServoBus()
	{
		aisHead = new AIServoList;
		aisHead->next = NULL;
		length = 0;
		uart = NULL;
	}
	virtual void update(bool enableMixing) = 0;
	virtual long long s2a() = 0;
	virtual int maxSpeed() = 0;
	virtual int setMaxSpeed(unsigned char id) = 0;
	virtual int readPosition(unsigned char id) { return -1; }
	virtual int ping(unsigned char id) { return -1; }
	virtual int release(unsigned char id) { return -1; }
	bool isBegin() { return (uart != NULL); }
	bool contained(unsigned char id);
	void del(unsigned char id);
	int getLength() { return length; }
	void incLength() { length++; }
	void show();
	AIServoList* tail();
	AIServoList* aisHead;
protected:
	int length;
	HardwareSerial *uart;
	unsigned long baudrate;
	unsigned long TX_DELAY;
};

#endif

