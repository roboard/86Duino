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

#ifndef RTC_ZERO_H
#define RTC_ZERO_H

#include <inttypes.h>
#include <time.h>
#include <sys/time.h>

typedef void(*voidFuncPtr)(void);

#define ALARMOFF                     (0)
#define ALARMSEC                     (1)
#define ALARMMINSEC                  (2)
#define ALARMHOURMINSEC              (3)
#define ALARMDAYHOURMINSEC           (4)
#define ALARMMONDAYHOURMINSEC        (5)
#define ALARMYEARMONDAYHOURMINSEC    (6)
class RTCZero {
public:
	uint8_t MATCH_OFF          = ALARMOFF;                   // Never
	uint8_t MATCH_SS           = ALARMSEC;                   // Every Minute
	uint8_t MATCH_MMSS         = ALARMMINSEC;                // Every Hour
	uint8_t MATCH_HHMMSS       = ALARMHOURMINSEC;            // Every Day
	uint8_t MATCH_DHHMMSS      = ALARMDAYHOURMINSEC;         // Every Month
	uint8_t MATCH_MMDDHHMMSS   = ALARMMONDAYHOURMINSEC;      // Every Year
	uint8_t MATCH_YYMMDDHHMMSS = ALARMYEARMONDAYHOURMINSEC;  // Once, on a specific date and a specific time

	RTCZero();
	void begin();

	void enableAlarm(uint8_t match);
	void disableAlarm();

	void attachInterrupt(voidFuncPtr callback);
	void detachInterrupt();

	void standbyMode();

	/* Get Functions */

	uint8_t getSeconds();
	uint8_t getMinutes();
	uint8_t getHours();
	uint8_t getAM_PM();

	uint8_t getDay();
	uint8_t getMonth();
	uint8_t getYear();

	uint8_t getAlarmSeconds();
	uint8_t getAlarmMinutes();
	uint8_t getAlarmHours();
	uint8_t getAlarmAM_PM();

	uint8_t getAlarmDay();
	uint8_t getAlarmMonth();
	uint8_t getAlarmYear();

	/* Set Functions */

	void setSeconds(uint8_t seconds);
	void setMinutes(uint8_t minutes);
	void setHours(uint8_t hours);
	void setTime(uint8_t hours, uint8_t minutes, uint8_t seconds);

	void setDay(uint8_t day);
	void setMonth(uint8_t month);
	void setYear(uint8_t year);
	void setDate(uint8_t day, uint8_t month, uint8_t year);

	void setAlarmSeconds(uint8_t seconds);
	void setAlarmMinutes(uint8_t minutes);
	void setAlarmHours(uint8_t hours);
	void setAlarmTime(uint8_t hours, uint8_t minutes, uint8_t seconds);

	void setAlarmDay(uint8_t day);
	void setAlarmMonth(uint8_t month);
	void setAlarmYear(uint8_t year);
	void setAlarmDate(uint8_t day, uint8_t month, uint8_t year);

	/* Epoch Functions */

	uint32_t getEpoch();
	uint32_t getY2kEpoch();
	void setEpoch(uint32_t ts);
	void setY2kEpoch(uint32_t ts);
private:
	bool RTCZeroEnable;
};

#endif // RTC_ZERO_H