/*
  RTC library for Arduino Zero.
  Copyright (c) 2015 Arduino LLC. All right reserved.
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
  
  Modified 19 February 2016 by Android Lin for 86Duino.
*/

#define USE_DOSH
#include "Arduino.h"

#include "RTCZero.h"

#include <time.h>
#include <sys/time.h>
#include <dos.h>
#include "io.h"
#include "irq.h"

#define EPOCH_TIME_OFF      946684800  // This is 1st January 2000, 00:00:00 in epoch time
#define EPOCH_TIME_YEAR_OFF 100

#define RTCIRQ              8
#define MAGIC_M_TIME        16777218

#define DPC_MODE  (0)
#define RT_MODE   (1)
#define HT_MODE   (2)
static _HelperTask* _task = NULL;

bool init_DosTime_Slave = false;

/*// Client
#include "fifolan.h"
#define MAX_SOCKS   (4)
static int fd, rc, temp, fds[MAX_SOCKS] = {-1, -1, -1, -1};
static unsigned char rxbuf[4096];
static unsigned char txbuf[4096];
static unsigned long val, rx_timeout, tx_timeout, temp2, temp3;
static bool init_OK = false;

static void send_command_para(uint8_t c, uint8_t* p, int p_size) {
    int i = 0, cur = 0, j;
    
    // command
    txbuf[i++] = c;
    
    // other parameters
    if (p != NULL) {
        for (j=0; j<p_size; j++)
            txbuf[i++] = *(p++);
    }
    
    do {
        rc = flan_send(fd, &txbuf[cur], i - cur);
        if (rc > 0) cur += rc;
    } while (cur != i);
    
}

// -1 : continue
//  0 : complite
static int recv_command_result(uint8_t* mybuf, int num) {
    int i, cur = 0;
    
    do {
        rc = flan_recv(fd, &rxbuf[cur], num - cur);
        if (rc < 0 && flan_errno(fd) == FLAN_ECONNABORTED)
            return -1;
        cur += rc;
    } while (cur != num);
    
    for (i=0; i<num; i++)
        mybuf[i] = rxbuf[i];
    
    return 0;
}

static unsigned int fourBytes2Int(unsigned char a0, unsigned char a1, unsigned char a2, unsigned char a3) {
    unsigned int r = 0;
    r = a3;
    r = (r << 8) + a2;
    r = (r << 8) + a1;
    r = (r << 8) + a0;
    return r;
}


void RTCZero::inti_Fifolan(void) {
	val = 1;
    rx_timeout = 100;
    tx_timeout = 100;
    
    fd = flan_socket();
    flan_ioctl(fd, FLAN_FIONBIO, (unsigned long)&val);
    flan_setsockopt(fd, FLAN_SO_RCVTIMEO, (const void*)&rx_timeout, sizeof(rx_timeout));
    flan_setsockopt(fd, FLAN_SO_SNDTIMEO, (const void*)&tx_timeout, sizeof(tx_timeout));
    // flan_getsockopt(fd, FLAN_SO_RCVTIMEO, &temp2, &temp3);
    // flan_getsockopt(fd, FLAN_SO_SNDTIMEO, &temp2, &temp3);
    
    do {
        rc = flan_connect(fd, 91); // 91 : RTCZero port
        if (rc == 0) break;
        printf("begining...\r");
    } while (1);
    printf("\n begin() OK!\n");
}

unsigned int RTCZero::getDosTime(void) {
    int r;
    unsigned char _r[6];
    
    send_command_para(1, NULL, 0); // 1 : "poll_mouse" command
    r = recv_command_result(_r, 6);
    if (r == 0) {
        setTime(_r[2], _r[1], _r[0]); // hour , minute, second
        setDate(_r[3], _r[4], _r[5]); // day, month, year
        return 0;
    }
    return 0xFFFFFFFF;
}
//*/

/*
struct tm {
  int    tm_sec;    // seconds after the minute [0-60] 
  int    tm_min;    // minutes after the hour [0-59] 
  int    tm_hour;   // hours since midnight [0-23] 
  int    tm_mday;   // day of the month [1-31] 
  int    tm_mon;    // months since January [0-11] 
  int    tm_year;   // years since 1900 
  int    tm_wday;   // days since Sunday [0-6] 
  int    tm_yday;   // days since January 1 [0-365] 
  int    tm_isdst;  // Daylight Savings Time flag
  long   tm_gmtoff; // offset from GMT in seconds
  char * tm_zone;   // timezone abbreviation
};
*/
struct tm tms; // Alarm time
extern void __yields();
/*
static void checkUpdatePoint(void) {
    static unsigned long _updateTime = 0L;
    static bool _checked = false;
START:
    if (!_checked) {
        io_outpb(0x70, 0x8A);
        while ((io_inpb(0x71) & 0x80) == 0) {
            __yields();
            io_outpb(0x70, 0x8A);
        }
        // Here, UIP is 1
        io_outpb(0x70, 0x8A);
        while ((io_inpb(0x71) & 0x80) > 0) { // latch the moment when 1 to 0, will elapsed 1984 us
            __yields();
            io_outpb(0x70, 0x8A);
        }
        
        _updateTime = millis();
        _checked = true;
    } else {
        if (((millis() - _updateTime) % 1000) > 997) { // 1984 us ~= 2 ms, so we wait 3ms for safe.
            _checked = false;
            goto START;
        }
    }
}
*/

DMP_INLINE(unsigned char) inpb_cmos(unsigned char reg) {
    unsigned char tmp;
    
	io_DisableINT();
    io_outpb(0x70, 0x80 | reg); // disable NMI (by setting the 0x80 bit) and assign a RTC register address
    tmp = io_inpb(0x71);
    io_RestoreINT();
    return tmp;
}

DMP_INLINE(void) outpb_cmos(unsigned char reg, unsigned char data) {
    io_DisableINT();
    io_outpb(0x70, 0x80 | reg); // disable NMI (by setting the 0x80 bit) and assign a RTC register address
    io_outpb(0x71, data);
    io_RestoreINT();
}

typedef struct {
    unsigned char year, month, day;
} RTC_TIME_1;

typedef struct {
    unsigned char second, minute, hour;
} RTC_TIME_2;

static RTC_TIME_1 gt1;
static RTC_TIME_2 gt2;
static int read1Count = 0;
static int read2Count = 0;
static RTC_TIME_1 getTimes1(void) {
    RTC_TIME_1 t1, t2;
    
    do {
        __yields();
        io_outpb(0x70, 0x8A);
        if ((io_inpb(0x71) & 0x80) > 0) continue;
        // 第一次讀取
        t1.day    = inpb_cmos(0x07);
        t1.month  = inpb_cmos(0x08);
        t1.year   = inpb_cmos(0x09);
        
        io_outpb(0x70, 0x8A);
        if ((io_inpb(0x71) & 0x80) > 0) continue;
        
        // 第二次讀取
        t2.day    = inpb_cmos(0x07);
        t2.month  = inpb_cmos(0x08);
        t2.year   = inpb_cmos(0x09);
        
        io_outpb(0x70, 0x8A);
        if ((io_inpb(0x71) & 0x80) > 0) continue;
        
    // 只要兩次讀取的內容有任何一點不同，就繼續重試
    } while (t1.day   != t2.day || 
            t1.month != t2.month || 
            t1.year  != t2.year);
    
    read1Count = 3;
    return t1;
}

static RTC_TIME_2 getTimes2(void) {
    RTC_TIME_2 t1, t2;
    
    do {
        __yields();
        io_outpb(0x70, 0x8A);
        if ((io_inpb(0x71) & 0x80) > 0) continue;
        
        t1.second = inpb_cmos(0x00);
        t1.minute = inpb_cmos(0x02);
        t1.hour   = inpb_cmos(0x04);
        
        io_outpb(0x70, 0x8A);
        if ((io_inpb(0x71) & 0x80) > 0) continue;
        
        // 第二次讀取
        t2.second = inpb_cmos(0x00);
        t2.minute = inpb_cmos(0x02);
        t2.hour   = inpb_cmos(0x04);
        
        io_outpb(0x70, 0x8A);
        if ((io_inpb(0x71) & 0x80) > 0) continue;
        
    // 只要兩次讀取的內容有任何一點不同，就繼續重試
    } while (t1.second != t2.second || 
             t1.minute != t2.minute || 
             t1.hour   != t2.hour);
    
    read2Count = 3;
    return t1;
}

DMP_INLINE(uint8_t) getRTCYear(bool now) {
    uint8_t _year;
    if (now || read1Count == 0) gt1 = getTimes1();
	_year = gt1.year;
    read1Count--;
	return ((_year & 0xF0)>>4)*10 + (_year & 0x0F);
}

DMP_INLINE(uint8_t) getRTCMonth(bool now) {
	uint8_t _mon;
    if (now || read1Count == 0) gt1 = getTimes1();
	_mon = gt1.month;
    read1Count--;
	return ((_mon & 0x10)>>4)*10 + (_mon & 0x0F);
}

DMP_INLINE(uint8_t) getRTCDay(bool now) {
	uint8_t _day;
    if (now || read1Count == 0) gt1 = getTimes1();
	_day = gt1.day;
    read1Count--;
	return ((_day & 0x30)>>4)*10 + (_day & 0x0F);
}

DMP_INLINE(uint8_t) getRTCHours(bool now) {
	uint8_t _hour;
    if (now || read2Count == 0) gt2 = getTimes2();
	_hour = gt2.hour;
    read2Count--;
	return ((_hour & 0x30)>>4)*10 + (_hour & 0x0F);
}

DMP_INLINE(uint8_t) getRTCMinutes(bool now) {
	uint8_t _min;
    if (now || read2Count == 0) gt2 = getTimes2();
	_min = gt2.minute;
    read2Count--;
	return ((_min & 0x70)>>4)*10 + (_min & 0x0F);
}

DMP_INLINE(uint8_t) getRTCSeconds(bool now) {
	uint8_t _sec;
    if (now || read2Count == 0) gt2 = getTimes2();
	_sec = gt2.second;
    read2Count--;
	return ((_sec & 0x70)>>4)*10 + (_sec & 0x0F);
}



DMP_INLINE(void) setRTCYear(uint8_t year) {
	outpb_cmos(0x09, ((year/10)<<4)+(year%10));
}

DMP_INLINE(void) setRTCMonth(uint8_t month) {
	outpb_cmos(0x08, ((month/10)<<4)+(month%10));
}

DMP_INLINE(void) setRTCDay(uint8_t day) {
	outpb_cmos(0x07, ((day/10)<<4)+(day%10));
}

DMP_INLINE(void) setRTCHours(uint8_t hour) {
	outpb_cmos(0x04, ((hour/10)<<4)+(hour%10));
}

DMP_INLINE(void) setRTCMinutes(uint8_t min) {
	outpb_cmos(0x02, ((min/10)<<4)+(min%10));
}

DMP_INLINE(void) setRTCSeconds(uint8_t sec) {
	outpb_cmos(0x00, ((sec/10)<<4)+(sec%10));
}

DMP_INLINE(void) _setAlarmAllSeconds(int sec, int min, int hour, int day, int month, int year) {
	
	tms.tm_isdst = -1;
	tms.tm_yday  = 0;
	tms.tm_wday  = 0;
	
	if(year < 0)
		tms.tm_year  = getRTCYear(false) + EPOCH_TIME_YEAR_OFF;
	else
		tms.tm_year  = year + EPOCH_TIME_YEAR_OFF;
	
	if(month < 0)
		tms.tm_mon   = getRTCMonth(false) - 1;
	else
		tms.tm_mon   = month - 1;
	
	if(day < 0)
		tms.tm_mday  = getRTCDay(false);
	else
		tms.tm_mday  = day;
	
	if(hour < 0)
		tms.tm_hour  = getRTCHours(false);
	else
		tms.tm_hour  = hour;
	
	if(min < 0)
		tms.tm_min   = getRTCMinutes(false);
	else
		tms.tm_min   = min;
	
	if(sec < 0)
		tms.tm_sec   = getRTCSeconds(false);
	else
		tms.tm_sec   = sec;
}

static void _setAlarm(void) {
    // printf("Alarm - hour:%d min:%d sec:%d nowtime:%d\n", tms.tm_hour, tms.tm_min, tms.tm_sec, time(0));
	io_DisableINT();
	outpb_cmos(0x05, ((tms.tm_hour/10)<<4)+(tms.tm_hour%10));
	outpb_cmos(0x03, ((tms.tm_min/10)<<4)+(tms.tm_min%10));
	outpb_cmos(0x01, ((tms.tm_sec/10)<<4)+(tms.tm_sec%10));
	io_RestoreINT();
}

static char* isrname_rtc = "RTCZero";
static uint8_t volatile alarmType = ALARMOFF;
static voidFuncPtr isrCallback = NULL;
static volatile bool AlarmFlag = false;
static int pre_sec = -1;
static void* DPCh = NULL; // DPC hadles
static volatile int rt = DPC_MODE;
static volatile bool UT_flag = false;
static int rtczero_isr_handler(int irq, void* data) {
	unsigned char tmp, atm, ath;
	bool match = false;
    
	io_outpb(0x70, 0x0C); // enable NMI and read RTC register C 
	tmp = io_inpb(0x71); // clear RTC interrupt state

	if((tmp & 0x40) == 0) return ISR_NONE; // only see alarm interrupt status bit
	if(alarmType == ALARMOFF) return ISR_HANDLED;
	 
	// check match condition
	int s, m, h, d, mm, y;
	s = getRTCSeconds(false);
	m = getRTCMinutes(false);
	h = getRTCHours(false);
	d = getRTCDay(false);
	mm = getRTCMonth(false);
	y = getRTCYear(false);
	
    if (pre_sec != s)
    {
        if(alarmType == ALARMSEC)
        {
            if(s == tms.tm_sec) match = true;
        }
        else if(alarmType == ALARMMINSEC)
        {
            if(s == tms.tm_sec && m == tms.tm_min) match = true;
        }
        else if(alarmType == ALARMHOURMINSEC)
        {
            if(s == tms.tm_sec && m == tms.tm_min && h == tms.tm_hour) match = true;
        }
        else if(alarmType == ALARMDAYHOURMINSEC)
        {
            if(s == tms.tm_sec && m == tms.tm_min && h == tms.tm_hour && d == tms.tm_mday) match = true;
        }
        else if(alarmType == ALARMMONDAYHOURMINSEC)
        {
            if(s == tms.tm_sec && m == tms.tm_min && h == tms.tm_hour && d == tms.tm_mday && mm == tms.tm_mon)
                match = true;
        }
        else if(alarmType == ALARMYEARMONDAYHOURMINSEC)
        {
            if(s == tms.tm_sec && m == tms.tm_min && h == tms.tm_hour && d == tms.tm_mday && mm == tms.tm_mon && y == tms.tm_year)
                match = true;
        }
    }

	if(match == true)
	{
		if (rt == RT_MODE) {
            if (isrCallback != NULL) isrCallback();
        } else if (rt == DPC_MODE){
            if (DPCh != NULL) irq_TriggerDPC(DPCh);
        } else if (rt == HT_MODE) {
            UT_flag = true;
        }
		AlarmFlag = true;
	}

    pre_sec = s;
    
    return ISR_HANDLED;
}

static void dpcCallBackFunc(void* data) {
    if (isrCallback != NULL) isrCallback();
}

static bool RTCZeroInit = false;
static bool AM12 = false;
RTCZero::RTCZero() {
    // set Alarm initial value
    tms.tm_isdst = -1;
	tms.tm_yday  = 0;
	tms.tm_wday  = 0;
    tms.tm_year  = 0;
    tms.tm_mon   = 0;
    tms.tm_mday  = 0;
    tms.tm_hour  = 0;
    tms.tm_min   = 0;
    tms.tm_sec   = 0;
}
/*
//Client
void RTCZero::begin() {
    unsigned int r;
    RTCZeroInit = true;
    outpb_cmos(0x0B, 0x02); // set bit 1 to logic 1 is 24 hours, logic 0 is 12 hours
    
    // Add new code to get dos time here.
    if (!init_DosTime_Slave) {
        inti_Fifolan();
        r = getDosTime();
        if (r == 0xFFFFFFFF)
            printf("now() get error data\n");
        init_DosTime_Slave = true;
    }
}
//*/

//Master
void RTCZero::begin() {
    RTCZeroInit = true;
	outpb_cmos(0x0B, 0x02); // set bit 1 to logic 1 is 24 hours, logic 0 is 12 hours
}
//*/

static void UT_LOOP(void) {
    if (UT_flag) {
        if (isrCallback != NULL) isrCallback();
        UT_flag = false;
        helperYield();
    }
}

void RTCZero::attachInterrupt(void (*isr)(void), int _rt) {
    io_DisableINT();
    isrCallback = isr;
    rt = _rt;
    if (_rt == DPC_MODE) {
        if (DPCh == NULL) {
            DPCh = irq_AllocDPC(dpcCallBackFunc, NULL, IRQ_DPC_USEFPU);
            if (DPCh == NULL)
                printf("We get NULL retrun from irq_AllocDPC()\n");
        } // We just alloc DPC once
    } else if (_rt == HT_MODE) {
        _task = myHelper.startLoop(10 * 1024);
        if (_task == NULL) {
            printf("WInterrupt: myHelper init fail\n");
            return;
        }
        _task->registerFunc(UT_LOOP);
        _task->start();
    }
    io_RestoreINT();
}

void RTCZero::attachInterrupt(void (*isr)(void)) {
    attachInterrupt(isr, DPC_MODE);
}

void RTCZero::attachRTInterrupt(void (*isr)(void)) {
    attachInterrupt(isr, RT_MODE);
}

void RTCZero::attachHTInterrupt(void (*isr)(void)) {
    attachInterrupt(isr, HT_MODE);
}

void RTCZero::detachInterrupt() {
	io_DisableINT();
	isrCallback = NULL;
    if (DPCh != NULL) {
        if (irq_FreeDPC(DPCh) == false)
            printf("We free the DPC handle fail\n");
        else
            DPCh = NULL;
        rt = false;
    } else if (_task != NULL) {
        _task->kill();
        _task = NULL;
    }
	io_RestoreINT();
    
}

void RTCZero::enableAlarm(uint8_t match) {
	unsigned char tmp;
	
	if(RTCZeroInit == false) return;
    
    if(RTCZeroEnable == false)
	{
		if(irq_Setting(RTCIRQ, IRQ_EDGE_TRIGGER) == false)
	    {
	        printf("MCM IRQ Setting fail\n"); return;
	    }
		if(irq_InstallISR(RTCIRQ, rtczero_isr_handler, isrname_rtc) == false)
		{
		    printf("irq_install fail\n"); return;
		}
	}
	
	io_DisableINT();
    io_outpb(0x70, 0x0C);
    io_inpb(0x71);
    
	alarmType = match;
	
	if(alarmType == ALARMSEC)
	{
		outpb_cmos(0x03, inpb_cmos(0x02));
		outpb_cmos(0x05, inpb_cmos(0x04));
	}
	else if(alarmType == ALARMMINSEC)
	{
		outpb_cmos(0x05, inpb_cmos(0x04));
	}
	
    // set frequse
    tmp = inpb_cmos(0x0A);
    outpb_cmos(0x0A, (tmp & 0xF0) | 0x0F); // frequse = 15
    
	tmp = inpb_cmos(0x0B);
    outpb_cmos(0x0B, tmp | 0x40); // Periodic interrupt(0x40)
	io_RestoreINT();
	
	RTCZeroEnable = true;
}

void RTCZero::disableAlarm() {
	unsigned char tmp;
	
	if(RTCZeroInit == false || RTCZeroEnable == false) return;
	
	io_DisableINT();
	alarmType = ALARMOFF;
	tmp = inpb_cmos(0x0B);
    outpb_cmos(0x0B, tmp & ~(0x40));
	io_RestoreINT();
	
    irq_UninstallISR(RTCIRQ, (void*)isrname_rtc);
	
	RTCZeroEnable = false;
}

void RTCZero::standbyMode() {
	unsigned char tmp;
	
	if(RTCZeroEnable == true)
	{
		// Note: in order to implement "year, month, day" alram time, we also set "dummy" alarm time to do it,
		// so don't depended on whether RTC interrupt occur or not to exit while(), otherwise exit while() will be at wrong time.
		// we should see "AlarmFlag" to determine whether exit while() or not. If "AlarmFlag" is set to "true" in ISR, then exit while() here.
		do
		{
			asm __volatile__ ("hlt\n":::"memory");
		} while(AlarmFlag == false); 
		AlarmFlag = false;
	}
	else
	{
		do
		{
			asm __volatile__ ("hlt\n":::"memory");
			io_outpb(0x70, 0x0C); // enable NMI and read RTC register C 
			tmp = io_inpb(0x71); // clear RTC interrupt state
		} while((tmp & 0x20) == 0);
	}
}

uint8_t RTCZero::getSeconds(bool now) {
	return getRTCSeconds(now);
}

uint8_t RTCZero::getMinutes(bool now) {
	return getRTCMinutes(now);
}

uint8_t RTCZero::getHours(bool now) {
	return getRTCHours(now);
}

/*
uint8_t RTCZero::getAM_PM() {
	return (inpb_cmos(4) & 0x80) == 0 ? 
}
*/

uint8_t RTCZero::getDay(bool now) {
	return getRTCDay(now);
}

uint8_t RTCZero::getMonth(bool now) {
	return getRTCMonth(now);
}

uint8_t RTCZero::getYear(bool now) {
	return getRTCYear(now);
}

uint8_t RTCZero::getAlarmSeconds() {
	return tms.tm_sec;
}

uint8_t RTCZero::getAlarmMinutes() {
	return tms.tm_min;
}

uint8_t RTCZero::getAlarmHours() {
	return tms.tm_hour;
}

/*
uint8_t RTCZero::getAlarmAM_PM() {
	
}
*/

uint8_t RTCZero::getAlarmDay() {
	return tms.tm_mday;
}

uint8_t RTCZero::getAlarmMonth() {
	return tms.tm_mon + 1;
}

uint8_t RTCZero::getAlarmYear() {
	return (tms.tm_year >= EPOCH_TIME_YEAR_OFF) ? (tms.tm_year - EPOCH_TIME_YEAR_OFF) : tms.tm_year;
}

void RTCZero::setSeconds(uint8_t seconds) {
	if(seconds > 59) return;
	setRTCSeconds(seconds);
}

void RTCZero::setMinutes(uint8_t minutes) {
	if(minutes > 59) return;
	setRTCMinutes(minutes);
}

void RTCZero::setHours(uint8_t hours) {
	if(hours > 23) return;
	setRTCHours(hours); 
}

void RTCZero::setTime(uint8_t hours, uint8_t minutes, uint8_t seconds) {
	if(seconds > 59 || minutes > 59 || hours > 23) return;
	setSeconds(seconds);
	setMinutes(minutes);
	setHours(hours);
}

void RTCZero::setDay(uint8_t day) {
	if(day > 31) return;
	setRTCDay(day);
}

void RTCZero::setMonth(uint8_t month) {
	if(month > 12) return;
	setRTCMonth(month);
}

void RTCZero::setYear(uint8_t year) {
	if(year > 99) return;
	setRTCYear(year);
}

void RTCZero::setDate(uint8_t day, uint8_t month, uint8_t year) {
	if(day > 31 || month > 12 || year > 99) return;
	setRTCDay(day);
	setRTCMonth(month);
	setRTCYear(year);
}

void RTCZero::setAlarmSeconds(uint8_t seconds) {
	if(seconds > 59) return;
	
	io_DisableINT();
	_setAlarmAllSeconds(seconds, -1, -1, -1, -1, -1);
	io_RestoreINT();
	
	_setAlarm();
}

void RTCZero::setAlarmMinutes(uint8_t minutes) {
	if(minutes > 59) return;
	if(alarmType == ALARMSEC) return;
	
	io_DisableINT();
	_setAlarmAllSeconds(-1, minutes, -1, -1, -1, -1);
	io_RestoreINT();
	
	_setAlarm();
}

void RTCZero::setAlarmHours(uint8_t hours) {
	if(hours > 23) return;
	if(alarmType == ALARMSEC || alarmType == ALARMMINSEC) return;
	
	io_DisableINT();
	_setAlarmAllSeconds(-1, -1, hours, -1, -1, -1);
	io_RestoreINT();
	
	_setAlarm();
}

void RTCZero::setAlarmTime(uint8_t hours, uint8_t minutes, uint8_t seconds) {
	if(hours > 23 || minutes > 59 || seconds > 59) return;
	
	io_DisableINT();
	if(alarmType == ALARMSEC)
		_setAlarmAllSeconds(seconds, -1, -1, -1, -1, -1);
	else if(alarmType == ALARMMINSEC)
		_setAlarmAllSeconds(seconds, minutes, -1, -1, -1, -1);
	else
		_setAlarmAllSeconds(seconds, minutes, hours, -1, -1, -1);
	io_RestoreINT();

	_setAlarm();
}

void RTCZero::setAlarmDay(uint8_t day) {
	if(day > 31) return;
	
	io_DisableINT();
	_setAlarmAllSeconds(-1, -1, -1, day, -1, -1);
	io_RestoreINT();
	
	_setAlarm();
}

void RTCZero::setAlarmMonth(uint8_t month) {
	if(month > 12) return;
	
	io_DisableINT();
	_setAlarmAllSeconds(-1, -1, -1, -1, month, -1);
	io_RestoreINT();
	
	_setAlarm();
}

void RTCZero::setAlarmYear(uint8_t year) {
	if(year > 99) return; // 2099
	
	io_DisableINT();
	_setAlarmAllSeconds(-1, -1, -1, -1, -1, year);
	io_RestoreINT();
	
	_setAlarm();
}

void RTCZero::setAlarmDate(uint8_t day, uint8_t month, uint8_t year) {
	if(day > 31 || month > 12 || year > 99) return;
	
	io_DisableINT();
	_setAlarmAllSeconds(-1, -1, -1, day, month, year);
	io_RestoreINT();
	
	_setAlarm();
}

static unsigned BCD2Hex(unsigned bcd)
{
  return (bcd >> 4) * 10 + (bcd & 0x0f);
}

static void get_rtc_time(unsigned *py, unsigned *pmo, unsigned *pd, unsigned *ph, unsigned *pm, unsigned *ps)
{
    union REGS regs_1, regs_2;
    // read CMOS time via BIOS call 2 times, make sure the timer is not in update,
    // and 2 time value are the same.
    while(1) {
        regs_1.h.ah = 4;
        int86(0x1a, &regs_1, &regs_1);
        *py = BCD2Hex(regs_1.h.ch) * 100 + BCD2Hex(regs_1.h.cl);
        *pmo = BCD2Hex(regs_1.h.dh);
        *pd = BCD2Hex(regs_1.h.dl);
        regs_1.h.ah = 2;
        regs_2.h.ah = 2;
        int86(0x1a, &regs_1, &regs_1);
        int86(0x1a, &regs_2, &regs_2);
        if(regs_1.x.cflag || regs_2.x.cflag)
            continue;  // in update, call again
        if(regs_1.h.ch != regs_2.h.ch ||
           regs_1.h.cl != regs_2.h.cl ||
           regs_1.h.dh != regs_2.h.dh)
            continue;  // time changed, call again
        *ph = BCD2Hex(regs_1.h.ch);
        *pm = BCD2Hex(regs_1.h.cl);
        *ps = BCD2Hex(regs_1.h.dh);
        break;
    }
}

static uint32_t get_rtc_seconds() {
    unsigned y, m, d, hour, min, sec;
    unsigned yoe, doy, doe;
    int era;
    get_rtc_time(&y, &m, &d, &hour, &min, &sec);

    y -= m <= 2;
    era = (y >= 0 ? y : y - 399) / 400;
    yoe = y - era * 400;                                // [0, 399]
    doy = (153*(m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1; // [0, 365]
    doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;        // [0, 146096]

    return (era * 146097 + doe - 719468) * 86400 + hour * 3600 + min * 60 + sec;
}

uint32_t RTCZero::getEpoch() {
	// struct tm tms;
	// tms.tm_isdst = -1;
	// tms.tm_yday = 0;
	// tms.tm_wday = 0;
	// tms.tm_year = getYear() + EPOCH_TIME_YEAR_OFF;
	// tms.tm_mon = getMonth() - 1;
	// tms.tm_mday = getDay();
	// tms.tm_hour = getHours();
	// tms.tm_min = getMinutes();
	// tms.tm_sec = getSeconds();
	// return mktime(&tms); <= have a bug!
    return get_rtc_seconds(); // use new method!
}

uint32_t RTCZero::getY2kEpoch() {
	return (getEpoch() - EPOCH_TIME_OFF);
}

void RTCZero::setEpoch(uint32_t ts) {
	if (ts < EPOCH_TIME_OFF) {
		ts = EPOCH_TIME_OFF;
	}

	time_t t = ts;
	struct tm* tmes = gmtime(&t);

	setRTCSeconds(tmes->tm_sec);
	setRTCMinutes(tmes->tm_min);
	setRTCHours(tmes->tm_hour);
	setRTCDay(tmes->tm_mday);
	setRTCMonth(tmes->tm_mon + 1);
	setRTCYear(tmes->tm_year  - EPOCH_TIME_YEAR_OFF);
}

void RTCZero::setY2kEpoch(uint32_t ts) {
	setEpoch(ts + EPOCH_TIME_OFF);
}

/*
     Reg#			  Description
	 00  RTC seconds
	 01  RTC seconds alarm
	 02  RTC minutes
	 03  RTC minutes alarm
	 04  RTC hours
	 05  RTC hours alarm
	 06  RTC day of week
	 07  RTC day of month
	 08  RTC month
	 09  RTC year
	 0A  RTC Status register A:
	    |7|6|5|4|3|2|1|0|  RTC Status Register A
	     | | | | `---------- rate selection Bits for divider output
	     | | | |		 frequency (set to 0110 = 1.024kHz, 976.562æs)
	     | `-------------- 22 stage divider, time base being used;
	     |			  (initialized to 010 = 32.768kHz)
	     `-------------- 1=time update in progress, 0=time/date available
	 
	 0B  RTC Status register B:
	    |7|6|5|4|3|2|1|0|  RTC Status Register B
	     | | | | | | | `---- 1=enable daylight savings, 0=disable (default)
	     | | | | | | `----- 1=24 hour mode, 0=12 hour mode (24 default)
	     | | | | | `------ 1=time/date in binary, 0=BCD (BCD default)
	     | | | | `------- 1=enable square wave frequency, 0=disable
	     | | | `-------- 1=enable update ended interrupt, 0=disable
	     | | `--------- 1=enable alarm interrupt, 0=disable
	     | `---------- 1=enable periodic interrupt, 0=disable
	     `----------- 1=disable clock update, 0=update count normally
	 0C  RTC Status register C (read only):
	    |7|6|5|4|3|2|1|0|  RTC Status Register C  (read only)
	     | | | | `---------- reserved (set to 0)
	     | | | `---------- update ended interrupt enabled
	     | | `---------- alarm interrupt enabled
	     | `---------- periodic interrupt enabled
	     `---------- IRQF flag
	 0D  RTC Status register D (read only):
	    |7|6-0|  RTC Status Register D  (read only)
	     |	`----- reserved (set to 0)
	     `------ 1=CMOS RAM has power, 0=CMOS RAM has lost power
	 0E  Diagnostic status byte:
	    |7|6|5|4|3|2|1|0|  Diagnostic Status Byte
	     | | | | | | `------ reserved
	     | | | | | `------- 1=time is invalid, 0=ok (POST validity check)
	     | | | | `-------- 1=fixed disk 0 failed initialization, 0=ok
	     | | | `--------- 1=memory size doesn't match config info, 0=ok
	     | | `---------- 1=invalid config info found, 0=ok (see below)
	     | `----------- 1=config record checksum is bad, 0=ok
	     `------------ 1=RTC lost power, 0=power state stable
	     
	 0F  Shutdown status byte:
	       0  soft reset or unexpected shutdown
	       1  shut down after memory size determination
	       2  shut down after memory test
	       3  shut down with memory error
	       4  shut down with boot loader request
	       5  JMP DWORD request with INT init
	       6  protected mode test 7 passed
	       7  protected mode test 7 failed
	       8  protected mode test1 failed
	       9  block move shutdown request
	       A  JMP DWORD request without INT init
	 10  Diskette drive type for A: and B:
	     
	    |7|6|5|4|3|2|1|0|  Diskette drive type for A: and B:
	     | | | | `---------- second diskette type
	     `----------------- first diskette type
		0000  no drive installed
		0001  DSDD 48 TPI drive
		0010  DSQD 96 TPI drive
		other values are reserved
	 11  Reserved
	 12  Fixed disk drive type for drive 0 and drive 1
	    |7|6|5|4|3|2|1|0|  Diskette drive type for A: and B:
	     | | | | `---------- second hard disk drive code (0000=no disk)
	     `----------------- first hard disk drive code (0000=no disk)
	 13  Reserved
	 14  Equipment byte
	    |7|6|5|4|3|2|1|0|  Equipment byte
	     | | | | | | | `---- 1=diskette drives installed, 0=none
	     | | | | | | `----- 1=math coprocessor installed, 0=none
	     | | | | `-------- unused
	     | | `--------- primary display
	     `------------ number of diskette drives installed
	    Bits		       Bits
	     54  Primary Display	76  Number of Drives
	     00  reserved		00  1 diskette drive
	     01  40 column color	01  2 diskette drives
	     10  80 column color	10  reserved
	     11  monochrome		11  reserved
	 15  LSB of system base memory in 1k blocks
	 16  MSB of system base memory in 1k blocks
	 17  LSB of total extended memory in 1k blocks
	 18  MSB of total extended memory in 1k blocks
	 19  Drive C extension byte (reserved AT)
	 1A  Drive D extension byte (reserved AT)
	 1B  13 bytes reserved
	 2E  CMOS checksum of bytes 10h-20h (MSB)
	 2F  CMOS checksum of bytes 10h-20h (LSB)
	 30  LSB of extended memory size found above 1 megabyte during POST
	 31  MSB of extended memory size found above 1 megabyte during POST
	 32  Date century byte in BCD ( BIOS interface to read and set)
	 33  Information flags (set during power-on)
	    |7|6|5-0|  Information Flags
	     | |  `----- reserved
	     | `------- initial setup message flag
	     `-------- 1=IBM 128k expansion installed, 0=none
	 34  12 bytes reserved
	Programming Considerations:
	Write CMOS address to read or write to port 70h
	Read/write port 71h to get/set data
	- the information here is only applicable to AT and PS/2 systems
	- INT 1A is used to read/set the Time of Day and Alarm.  To use the
	  alarm, INT 4A must be a valid interrupt service routine.
	- configuration settings are maintained using the Motorola MC146818
	  Real Time Clock.  Each of this chips 64 memory registers is used
	  for storage (0-3F).
	- Bit 5 of the diagnostic (0Eh) status byte is set during a power
	  on test.  This Bit is set if no floppy disks are found or the
	  display doesn't match the system display switch setting.
	- all addresses sent to port 70h have Bits 7&6 clear since Bit 7
	  of port 70h is used to enable/disable NMI.  Setting this Bit 7
	  enables NMI, clearing this Bit disables NMI.
	- when masking the NMI through using port 70H, port 71H should be
	  read immediately after or the RTC may be left in an unknown state.
	  This wont affect the PS/2 watchdog timer or system channel timeout.
	- see	INT 1A
*/