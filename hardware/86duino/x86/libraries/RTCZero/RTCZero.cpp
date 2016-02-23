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

#include "RTCZero.h"
#include <dos.h>
#include "io.h"
#include "irq.h"

#define EPOCH_TIME_OFF      946684800  // This is 1st January 2000, 00:00:00 in epoch time
#define EPOCH_TIME_YEAR_OFF 100

#define RTCIRQ              8

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

DMP_INLINE(uint8_t) getRTCYear() {
	uint8_t _year = inpb_cmos(9);
	return ((_year & 0xF0)>>4)*10 + (_year & 0x0F);
}

DMP_INLINE(uint8_t) getRTCMonth() {
	uint8_t _mon = inpb_cmos(8);
	return ((_mon & 0x10)>>4)*10 + (_mon & 0x0F);
}

DMP_INLINE(uint8_t) getRTCDay() {
	uint8_t _day = inpb_cmos(7);
	return ((_day & 0x30)>>4)*10 + (_day & 0x0F);
}

DMP_INLINE(uint8_t) getRTCHours() {
	uint8_t _hour = inpb_cmos(4);
	return ((_hour & 0x30)>>4)*10 + (_hour & 0x0F);
}

DMP_INLINE(uint8_t) getRTCMinutes() {
	uint8_t _min = inpb_cmos(2);
	return ((_min & 0x70)>>4)*10 + (_min & 0x0F);
}

DMP_INLINE(uint8_t) getRTCSeconds() {
	uint8_t _sec = inpb_cmos(0);
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

DMP_INLINE(unsigned long) _setAlarmAllSeconds(int sec, int min, int hour, int day, int month, int year) {
	struct tm tms;
	
	tms.tm_isdst = -1;
	tms.tm_yday  = 0;
	tms.tm_wday  = 0;
	
	if(year < 0)
		tms.tm_year  = getRTCYear() + EPOCH_TIME_YEAR_OFF;
	else
		tms.tm_year  = year + EPOCH_TIME_YEAR_OFF;
	
	if(month < 0)
		tms.tm_mon   = getRTCMonth() - 1;
	else
		tms.tm_mon   = month - 1;
	
	if(day < 0)
		tms.tm_mday  = getRTCDay();
	else
		tms.tm_mday  = day;
	
	if(hour < 0)
		tms.tm_hour  = getRTCHours();
	else
		tms.tm_hour  = hour;
	
	if(min < 0)
		tms.tm_min   = getRTCMinutes();
	else
		tms.tm_min   = min;
	
	if(sec < 0)
		tms.tm_sec   = getRTCSeconds();
	else
		tms.tm_sec   = sec;
	
	return mktime(&tms);
}

static uint32_t _getNowAllSeconds() {
	return _setAlarmAllSeconds(-1, -1, -1, -1, -1, -1);
}

static void _setAlarm(uint32_t at) {
	time_t t = at;
	struct tm* tms = gmtime(&t);

	io_DisableINT();
	outpb_cmos(0x05, ((tms->tm_hour/10)<<4)+(tms->tm_hour%10));
	outpb_cmos(0x03, ((tms->tm_min/10)<<4)+(tms->tm_min%10));
	outpb_cmos(0x01, ((tms->tm_sec/10)<<4)+(tms->tm_sec%10));
	io_RestoreINT();
}

static char* isrname_rtc = "RTCZero";
static uint8_t volatile alarmType = ALARMOFF;
static voidFuncPtr isrCallback = NULL;
static volatile bool AlarmFlag = false;
static volatile unsigned long AlarmTime = 0L;
static int rtczero_isr_handler(int irq, void* data) {
	unsigned char tmp, atm, ath;
	unsigned long nowtimes;
	bool match = false;
	time_t tnow, talarm;
	struct tm *tnow_m, *talarm_m;
	
	io_outpb(0x70, 0x0C); // enable NMI and read RTC register C 
	tmp = io_inpb(0x71); // clear RTC interrupt state

	if((tmp & 0x20) == 0) return ISR_NONE; // only see alarm interrupt status bit
	if(alarmType == ALARMOFF) return ISR_HANDLED;
	
	nowtimes = _getNowAllSeconds();
	tnow = nowtimes; talarm = AlarmTime;
	 
	// check match condition
	int s, m, h, d, mm, y;
	tnow_m = gmtime(&tnow);
	s = tnow_m->tm_sec;
	m = tnow_m->tm_min;
	h = tnow_m->tm_hour;
	d = tnow_m->tm_mday;
	mm = tnow_m->tm_mon;
	y = tnow_m->tm_year;
	talarm_m = gmtime(&talarm);
	
	if(alarmType == ALARMSEC)
	{
		if(s >= talarm_m->tm_sec) match = true;
	}
	else if(alarmType == ALARMMINSEC)
	{
		if(s >= talarm_m->tm_sec && m == talarm_m->tm_min) match = true;
	}
	else if(alarmType == ALARMHOURMINSEC)
	{
		if(s >= talarm_m->tm_sec && m == talarm_m->tm_min && h == talarm_m->tm_hour) match = true;
	}
	else if(alarmType == ALARMDAYHOURMINSEC)
	{
		if(s >= talarm_m->tm_sec && m == talarm_m->tm_min && h == talarm_m->tm_hour && d == talarm_m->tm_mday) match = true;
	}
	else if(alarmType == ALARMMONDAYHOURMINSEC)
	{
		if(s >= talarm_m->tm_sec && m == talarm_m->tm_min && h == talarm_m->tm_hour && d == talarm_m->tm_mday && mm == talarm_m->tm_mon)
			match = true;
	}
	else if(alarmType == ALARMYEARMONDAYHOURMINSEC)
	{
		if(s >= talarm_m->tm_sec && m == talarm_m->tm_min && h == talarm_m->tm_hour && d == talarm_m->tm_mday && mm == talarm_m->tm_mon && y == talarm_m->tm_year)
			match = true;
	}
	else
		match = false;

	if(match == true)
	{
		if(isrCallback != NULL) isrCallback();
		AlarmFlag = true;
	}

	_setAlarm(nowtimes + 60); // trigger next alarm signal after 60 sec

    return ISR_HANDLED;
}


static bool RTCZeroInit = false;
static bool AM12 = false;
RTCZero::RTCZero() {}

void RTCZero::begin() {
    RTCZeroInit = true;
	outpb_cmos(0x0B, 0x02); // set bit 1 to logic 1 is 24 hours, logic 0 is 12 hours
}

void RTCZero::attachInterrupt(void (*isr)(void)) {
    io_DisableINT();
    isrCallback = isr;
    io_RestoreINT();
}

void RTCZero::detachInterrupt() {
	io_DisableINT();
	isrCallback = NULL;
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
	
	tmp = inpb_cmos(0x0B);
    outpb_cmos(0x0B, tmp | 0x20); // Alarm interrupt
	io_RestoreINT();
	
	RTCZeroEnable = true;
}

void RTCZero::disableAlarm() {
	unsigned char tmp;
	
	if(RTCZeroInit == false || RTCZeroEnable == false) return;
	
	io_DisableINT();
	alarmType = ALARMOFF;
	tmp = inpb_cmos(0x0B);
    outpb_cmos(0x0B, tmp & ~(0x20));
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

uint8_t RTCZero::getSeconds() {
	return getRTCSeconds();
}

uint8_t RTCZero::getMinutes() {
	return getRTCMinutes();
}

uint8_t RTCZero::getHours() {
	return getRTCHours();
}

/*
uint8_t RTCZero::getAM_PM() {
	return (inpb_cmos(4) & 0x80) == 0 ? 
}
*/

uint8_t RTCZero::getDay() {
	return getRTCDay();
}

uint8_t RTCZero::getMonth() {
	return getRTCMonth();
}

uint8_t RTCZero::getYear() {
	return getRTCYear();
}

uint8_t RTCZero::getAlarmSeconds() {
	if(AlarmTime == 0L) return 0;
	
	time_t t = AlarmTime;
	struct tm* tms = gmtime(&t);
	return tms->tm_sec;
}

uint8_t RTCZero::getAlarmMinutes() {
	if(AlarmTime == 0L) return 0;
	
	time_t t = AlarmTime;
	struct tm* tms = gmtime(&t);
	return tms->tm_min;
}

uint8_t RTCZero::getAlarmHours() {
	if(AlarmTime == 0L) return 0;
	
	time_t t = AlarmTime;
	struct tm* tms = gmtime(&t);
	return tms->tm_hour;
}

/*
uint8_t RTCZero::getAlarmAM_PM() {
	
}
*/

uint8_t RTCZero::getAlarmDay() {
	if(AlarmTime == 0L) return 0;

	time_t t = AlarmTime;
	struct tm* tms = gmtime(&t);
	return tms->tm_mday;
}

uint8_t RTCZero::getAlarmMonth() {
	if(AlarmTime == 0L) return 0;

	time_t t = AlarmTime;
	struct tm* tms = gmtime(&t);
	return tms->tm_mon + 1;
}

uint8_t RTCZero::getAlarmYear() {
	if(AlarmTime == 0L) return 0;

	time_t t = AlarmTime;
	struct tm* tms = gmtime(&t);
	return (tms->tm_year >= EPOCH_TIME_YEAR_OFF) ? (tms->tm_year - EPOCH_TIME_YEAR_OFF) : tms->tm_year;
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
	AlarmTime = _setAlarmAllSeconds(seconds, -1, -1, -1, -1, -1);
	io_RestoreINT();
	
	_setAlarm(AlarmTime);
}

void RTCZero::setAlarmMinutes(uint8_t minutes) {
	if(minutes > 59) return;
	if(alarmType == ALARMSEC) return;
	
	io_DisableINT();
	AlarmTime = _setAlarmAllSeconds(-1, minutes, -1, -1, -1, -1);
	io_RestoreINT();
	
	_setAlarm(AlarmTime);
}

void RTCZero::setAlarmHours(uint8_t hours) {
	if(hours > 23) return;
	if(alarmType == ALARMSEC || alarmType == ALARMMINSEC) return;
	
	io_DisableINT();
	AlarmTime = _setAlarmAllSeconds(-1, -1, hours, -1, -1, -1);
	io_RestoreINT();
	
	_setAlarm(AlarmTime);
}

void RTCZero::setAlarmTime(uint8_t hours, uint8_t minutes, uint8_t seconds) {
	if(hours > 23 || minutes > 59 || seconds > 59) return;
	
	io_DisableINT();
	if(alarmType == ALARMSEC)
		AlarmTime = _setAlarmAllSeconds(seconds, -1, -1, -1, -1, -1);
	else if(alarmType == ALARMMINSEC)
		AlarmTime = _setAlarmAllSeconds(seconds, minutes, -1, -1, -1, -1);
	else
		AlarmTime = _setAlarmAllSeconds(seconds, minutes, hours, -1, -1, -1);
	io_RestoreINT();
	
	_setAlarm(AlarmTime);
}

void RTCZero::setAlarmDay(uint8_t day) {
	if(day > 31) return;
	
	io_DisableINT();
	AlarmTime = _setAlarmAllSeconds(-1, -1, -1, day, -1, -1);
	io_RestoreINT();
	
	_setAlarm(AlarmTime);
}

void RTCZero::setAlarmMonth(uint8_t month) {
	if(month > 12) return;
	
	io_DisableINT();
	AlarmTime = _setAlarmAllSeconds(-1, -1, -1, -1, month, -1);
	io_RestoreINT();
	
	_setAlarm(AlarmTime);
}

void RTCZero::setAlarmYear(uint8_t year) {
	if(year > 99) return; // 2099
	
	io_DisableINT();
	AlarmTime = _setAlarmAllSeconds(-1, -1, -1, -1, -1, year);
	io_RestoreINT();
	
	_setAlarm(AlarmTime);
}

void RTCZero::setAlarmDate(uint8_t day, uint8_t month, uint8_t year) {
	if(day > 31 || month > 12 || year > 99) return;
	
	io_DisableINT();
	AlarmTime = _setAlarmAllSeconds(-1, -1, -1, day, month, year);
	io_RestoreINT();
	
	_setAlarm(AlarmTime);
}

uint32_t RTCZero::getEpoch() {
	struct tm tms;
	tms.tm_isdst = -1;
	tms.tm_yday = 0;
	tms.tm_wday = 0;
	tms.tm_year = getYear() + EPOCH_TIME_YEAR_OFF;
	tms.tm_mon = getMonth() - 1;
	tms.tm_mday = getDay();
	tms.tm_hour = getHours();
	tms.tm_min = getMinutes();
	tms.tm_sec = getSeconds();
	return mktime(&tms);
}

uint32_t RTCZero::getY2kEpoch() {
	return (getEpoch() - EPOCH_TIME_OFF);
}

void RTCZero::setEpoch(uint32_t ts) {
	if (ts < EPOCH_TIME_OFF) {
		ts = EPOCH_TIME_OFF;
	}

	time_t t = ts;
	struct tm* tms = gmtime(&t);

	setRTCSeconds(tms->tm_sec);
	setRTCMinutes(tms->tm_min);
	setRTCHours(tms->tm_hour);
	setRTCDay(tms->tm_mday);
	setRTCMonth(tms->tm_mon + 1);
	setRTCYear(tms->tm_year  - EPOCH_TIME_YEAR_OFF);
}

void RTCZero::setY2kEpoch(uint32_t ts) {
	setEpoch(ts + EPOCH_TIME_OFF);
}
