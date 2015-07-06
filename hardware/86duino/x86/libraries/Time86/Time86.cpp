/*
  time.c - low level time and date functions
  Copyright (c) Michael Margolis 2009

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  6  Jan 2010 - initial release 
  12 Feb 2010 - fixed leap year calculation error
  1  Nov 2010 - fixed setTime bug (thanks to Korman for this)
  24 Mar 2012 - many edits by Paul Stoffregen: fixed timeStatus() to update
                status, updated examples for Arduino 1.0, fixed ARM
                compatibility issues, added TimeArduinoDue and TimeTeensy3
                examples, add error checking and messages to RTC examples,
                add examples to DS1307RTC library.
  13 Jan 2014 - modify to 86Duino version by Andriod Lin.
*/

#include "Time86.h"
#include <stdio.h>
#include <dos.h>

static time_t cacheTime;   // the time the cache was updated

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
}; */
struct tm *tptr;

/*
struct dostime_t {
  unsigned char hour;     // 0-23 
  unsigned char minute;   // 0-59 
  unsigned char second;   // 0-59 
  unsigned char hsecond;  // 0-99
}; */
struct dostime_t _dos_time;

/*
struct dosdate_t {
  unsigned char  day;       // 1-31          
  unsigned char  month;     // 1-12          
  unsigned short year;      // 1980-2099     
  unsigned char  dayofweek; // 0-6, 0=Sunday 
}; */
struct dosdate_t _dos_date;


int hour() { // the hour now 
  now();
  return _dos_time.hour; 
}

int hour(time_t t) { // the hour for the given time
  now();
  return _dos_time.hour;  
}

int hourFormat12() { // the hour now in 12 hour format
  now();
  if(_dos_time.hour == 0)
    return 12; // 12 midnight
  else if(_dos_time.hour > 12)
    return _dos_time.hour - 12;
  else
    return _dos_time.hour; 
}

int hourFormat12(time_t t) { // the hour for the given time in 12 hour format
  now();
  if(_dos_time.hour == 0)
    return 12; // 12 midnight
  else if(_dos_time.hour > 12)
    return _dos_time.hour - 12;
  else
    return _dos_time.hour;
}

uint8_t isAM() { // returns true if time now is AM
  return !isPM(); 
}

uint8_t isAM(time_t t) { // returns true if given time is AM
  return !isPM();  
}

uint8_t isPM() { // returns true if PM
  now();
  return (_dos_time.hour >= 12); 
}

uint8_t isPM(time_t t) { // returns true if PM
  now();
  return (_dos_time.hour >= 12); 
}

int minute() {
  now();
  return _dos_time.minute; 
}

int minute(time_t t) { // the minute for the given time
  now();
  return _dos_time.minute;  
}

int second() {
  now();
  return _dos_time.second; 
}

int second(time_t t) {  // the second for the given time
  now();
  return _dos_time.second;
}

int day(){
  now();
  return _dos_date.day; 
}

int day(time_t t) {
  now();
  return _dos_date.day;
}

int weekday() {   // Sunday is day 1
  now();
  return  _dos_date.dayofweek + 1; // for Arduino compatibility
}

int weekday(time_t t) {
  now();
  return _dos_date.dayofweek + 1; // for Arduino compatibility
}
   
int month(){
  now();
  return _dos_date.month; 
}

int month(time_t t) {  // the month for the given time
  now();
  return _dos_date.month; // jan is month 1 for Arduino compatibility
}

int year() {  // as in Processing, the full four digit year: (2009, 2010 etc) 
  now();
  return _dos_date.year; 
}

int year(time_t t) { // the year for the given time
  now();
  return _dos_date.year;
}

/*
// leap year calulator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0
static uint32_t sysTime = 0;
static uint32_t prevMillis = 0;
static uint32_t nextSyncTime = 0;
static timeStatus_t Status = timeNotSet;

getExternalTime getTimePtr;  // pointer to external sync function
//setExternalTime setTimePtr; // not used in this version

#ifdef TIME_DRIFT_INFO   // define this to get drift data
time_t sysUnsyncedTime = 0; // the time sysTime unadjusted by sync  
#endif
*/
void breakTime(time_t time, struct tm *temp) {
    tptr = gmtime(&time);
}

time_t makeTime(struct tm *tptrs) {
    return mktime(tptrs);
}

unsigned long now() {
	struct timeval tp;
	struct timezone tz;
	
	_dos_gettime(&_dos_time);
	_dos_getdate(&_dos_date);

	gettimeofday(&tp, &tz);
	return tp.tv_sec;
}

void setTime(time_t t) { 
	tptr = gmtime(&t);
	_dos_time.second = tptr->tm_sec;
	_dos_time.minute = tptr->tm_min;
	_dos_time.hour = tptr->tm_hour;
	_dos_date.day = tptr->tm_mday;
	_dos_date.dayofweek = tptr->tm_wday;
	_dos_date.month = tptr->tm_mon + 1;
	_dos_date.year = (tptr->tm_year + 1900);
	_dos_settime(&_dos_time); // if input any unavailable data, _dos_settime() will return non-zero. Here, we ignore this condition.
	_dos_setdate(&_dos_date); // if input any unavailable data, _dos_setdate() will return non-zero. Here, we ignore this condition.
} 

void setTime(int hr,int min,int sec,int dy, int mnth, int yr) {
// year can be given as full four digit year or two digts (2010 or 10 for 2010);  
//it is converted to years since 1970
	if(yr <= 99)
	    yr += 2000;
 
	_dos_date.year   = yr;
	_dos_date.month  = mnth;
	_dos_date.day    = dy;
	
	_dos_time.hour   = hr;
	_dos_time.minute = min;
	_dos_time.second = sec;
	_dos_settime(&_dos_time); // if input any unavailable data, _dos_settime() will return non-zero. Here, we ignore this condition. 
	_dos_setdate(&_dos_date); // if input any unavailable data, _dos_setdate() will return non-zero. Here, we ignore this condition.
}

void adjustTime(long adjustment) {
  time_t t = now() + adjustment;
  setTime(t);
}

// indicates if time has been set and recently synchronized
timeStatus_t timeStatus() {
  return timeSet;
}


void setSyncProvider( getExternalTime getTimeFunction){
  // do nothing
}

void setSyncInterval(time_t interval){ // set the number of seconds between re-sync
  // do nothing
}

