/*
  AIServo86.cpp - The part of Servo library
  
  Copyright (c) Designer 
    2016 Android Lin <acen@dmp.com.tw>. All right reserved.
    2016 AAA <aaa@dmp.com.tw>. All right reserved.
  Copyright (c) Programmer
    2016 Sayter <sayter@dmp.com.tw>. All right reserved.

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
  
  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)
*/

#include "AIServo86.h"
#include "io.h"
#include <TimerWDT.h>

#define MEGA 1000000
#define MAXSERVOS 64

enum {AISERVO_IDLE, AISERVO_MOVING, AISERVO_PAUSE};
static bool WDT_Timer_Used = false;
static bool updating = false;
static bool enableMixing = true;
AIServo nullAIServo;
uint8_t AIServoCount = 0;

static AIServo* Servoptr[MAXSERVOS] = {NULL};

typedef struct _BusList
{
	AIServoBus* bus;
	struct _BusList* next;
} BusList;

static BusList* buses = new BusList{NULL, NULL};

DMP_INLINE(long long) interpolate(long long cur, long long target, long long total, long long t, long long acc_prev, long long acc_now)
{
	long long t1 = total - t;
	long long Accfix;

	Accfix = (((t1*t1*t1) - (t1*total*total))*acc_prev) + (((t*t*t) - (t*total*total))*acc_now);
	Accfix = Accfix/(6*total);
	return (((total-t)*cur + t*target)/total) + Accfix;
}

static void timerwdt_isr_handler()
{
	if(updating == false)
	{
		updating = true;
		static BusList* tmpBusList;
		static AIServoList* tmpAIS;
		static AIServoList* cs;
		static unsigned long tt, t;
		for(tmpBusList = buses; tmpBusList->next != NULL; tmpBusList = tmpBusList->next)
		{
			if(tmpBusList->next->bus->getLength() == 0) continue;
			for(tmpAIS = tmpBusList->next->bus->aisHead; tmpAIS->next != NULL; tmpAIS = tmpAIS->next)
			{
				cs = tmpAIS->next;
				if (cs->state != AISERVO_MOVING) continue;
				t  = millis() - cs->starttime;
				tt = cs->totaltime;
				if(t >= tt || cs->curposition < 0)
				{
					cs->curposition = cs->targetposition;
					cs->state = AISERVO_IDLE;
				}
				else
				{
					cs->curposition = interpolate(cs->prevposition, cs->targetposition, tt, t, cs->acc_prev_out, cs->acc_now_in);
				}
			}
			tmpBusList->next->bus->update(enableMixing);
		}
		updating = false;
	}
}

static bool containedBus(AIServoBus& bus)
{
	static BusList* tmp;
	tmp = buses;
	while(tmp->next != NULL)
	{
		if(tmp->next->bus == &bus)
			return true;
		tmp = tmp->next;
	}
	return false;
}

AIServo::AIServo()
{
	ID = 255;
	attachedBus = NULL;
	nodePtr = NULL;
	servoIndex = AIServoCount++;
}

bool AIServo::attach(AIServoBus& bus, unsigned char id)
{
	if(!bus.isBegin()) return false;
	this->ID = id;
	long long tmpPos = (long long)(bus.readPosition(id)*bus.s2a());
	
	io_DisableINT();
	if(this->servoIndex < MAXSERVOS && !bus.contained(id))
	{
		AIServoList* node = new AIServoList;
		node->id = id;
		node->starttime = 0;
		node->endtime = 0;
		node->pausetime = 0;
		node->totaltime = 0;
		node->state = AISERVO_IDLE;
		node->speed = 0;
		node->offset = 0;
		node->mixoffset = 0;
		node->acc_prev_out = 0;
		node->acc_now_in = 0;
		node->acc_now_out = 0;
		node->prevposition = tmpPos;
		node->curposition = node->prevposition;
		node->targetposition = -1;
		node->next = NULL;
		(bus.tail())->next = node;
		bus.incLength();
		this->attachedBus = &bus;
		this->nodePtr = node;
		Servoptr[this->servoIndex] = this;
		
		if(!containedBus(bus))
		{
			BusList* busNode = new BusList;
			busNode->bus = &bus;
			busNode->next = NULL;
			BusList* tmp = buses;
			while(tmp->next != NULL)
				tmp = tmp->next;
			tmp->next = busNode;
		}
		
		io_RestoreINT();
		
		if(WDT_Timer_Used == false)
		{
			TimerWDT.initialize(10000L);
			TimerWDT.attachInterrupt(timerwdt_isr_handler);
			WDT_Timer_Used = true;
		}
		return true;
	}
	io_RestoreINT();
	return false;
}

bool AIServo::attached()
{
	return (Servoptr[this->servoIndex] != NULL);
}

void AIServo::detach()
{
	if(this->attachedBus != NULL)
	{
		io_DisableINT();
		Servoptr[this->servoIndex] = NULL;
		attachedBus->del(this->ID);
		this->attachedBus = NULL;
		this->nodePtr = NULL;
		io_RestoreINT();
	}
}

double AIServo::read()
{
	if(this->nodePtr == NULL)
		return -1;
	double tmp;
	io_DisableINT();
	tmp = (double)(((double)(this->nodePtr->curposition))/((double)MEGA));
	io_RestoreINT();
	return tmp;
}

void AIServo::setPosition(double angle, unsigned long time)
{
	if(angle < 0) return;
	if(this->nodePtr == NULL)
		return;
	io_DisableINT();
	this->nodePtr->targetposition = (long long)(angle*MEGA);
	this->nodePtr->totaltime = time;
	io_RestoreINT();
}

void AIServo::setPositionAcceleration(double _acc)
{
	setPositionAcceleration(_acc, _acc);
}

void AIServo::setPositionAcceleration(double _acc_in, double _acc_out)
{
	if(this->nodePtr == NULL)
		return;
	io_DisableINT();
	this->acc_in = (long long)(_acc_in*MEGA);
	this->acc_out = (long long)(_acc_out*MEGA);
	io_RestoreINT();
}

void AIServo::setVelocity(double velocity) // deg / s
{
	if(this->nodePtr == NULL)
		return;
	io_DisableINT();
	this->nodePtr->speed = (long long)(velocity*1000); // (MEGA * deg) / 1000millisecond
	io_RestoreINT();
}

void AIServo::setOffset(double offset)
{
	if(this->nodePtr == NULL)
		return;
	io_DisableINT();
	this->nodePtr->offset = (long long)offset*MEGA;
	io_RestoreINT();
}

void AIServo::setRealTimeMixing(double mixoffset)
{
	if(this->nodePtr == NULL)
		return;
	io_DisableINT();
	this->nodePtr->mixoffset = (long long)mixoffset*MEGA;
	io_RestoreINT();
}

void AIServo::setRealTimeMixing(double mixoffset, bool interrupt)
{
	if(this->nodePtr == NULL)
		return;
	if(interrupt) io_DisableINT();
	this->nodePtr->mixoffset = (long long)mixoffset*MEGA;
	if(interrupt) io_RestoreINT();
}

double AIServo::getPosition()
{
	return this->read();
}

void AIServo::write(double angle)
{
	this->setPosition(angle, 0);
	this->setVelocity(this->attachedBus->maxSpeed());
	this->run();
}

void AIServo::run()
{
	if(this->nodePtr == NULL || this->attachedBus == NULL) return;
	static AIServoList* cs;
	cs = this->nodePtr;
	if(cs->targetposition < 0.0) return;
	
	io_DisableINT();
	if(cs->state == AISERVO_MOVING || cs->state == AISERVO_IDLE)
	{
		cs->prevposition = cs->curposition;
		cs->targetposition = cs->targetposition + cs->offset;
		cs->acc_prev_out = cs->acc_now_out;
		cs->acc_now_in = this->acc_in;
		cs->acc_now_out = this->acc_out;
		this->acc_in = 0;
		this->acc_out = 0;
	}
	else // pause
	{
		io_RestoreINT();
		return;
	}
	
	if(cs->speed > 0 && cs->totaltime == 0L) // transform speed to time
		cs->totaltime = abs(cs->targetposition - cs->curposition) / cs->speed;
	
	cs->starttime = millis();
	cs->endtime = cs->starttime + cs->totaltime;
	cs->state = AISERVO_MOVING;
	io_RestoreINT();
}

void AIServo::pause()
{
	if(this->nodePtr == NULL || this->attachedBus == NULL) return;
	static AIServoList* cs;
	cs = this->nodePtr;

	io_DisableINT();
	if (cs->state != AISERVO_MOVING) {io_RestoreINT(); return;}
	cs->pausetime = millis();
	cs->state = AISERVO_PAUSE;
	io_RestoreINT();
}

void AIServo::resume()
{
	if(this->nodePtr == NULL || this->attachedBus == NULL) return;
	static AIServoList* cs;
	cs = this->nodePtr;

	io_DisableINT();
    if (cs->state != AISERVO_PAUSE) {io_RestoreINT(); return;}
	cs->pausetime = millis() - cs->pausetime;
	cs->starttime += cs->pausetime;
	cs->endtime += cs->pausetime;
	cs->state = AISERVO_MOVING;
	io_RestoreINT();
}

void AIServo::stop()
{
	if(this->nodePtr == NULL || this->attachedBus == NULL) return;
	static AIServoList* cs;
	cs = this->nodePtr;
	
	io_DisableINT();
	cs->prevposition = cs->targetposition = cs->curposition;
	cs->starttime = 0L;
	cs->endtime = 0L;
	cs->pausetime = 0L;
	cs->state = AISERVO_IDLE;
	cs->totaltime = 0L;
	cs->speed = 0;
	io_RestoreINT();
}

void AIServo::release()
{
	if(this->nodePtr == NULL || this->attachedBus == NULL) return;
	this->stop();
	io_DisableINT();
	this->attachedBus->release(this->ID);
	io_RestoreINT();
}

bool AIServo::isMoving()
{
    if(this->nodePtr == NULL || this->attachedBus == NULL) return false;
	static AIServoList* cs;
	cs = this->nodePtr;
	
	int tmp;
	io_DisableINT();
	tmp = cs->state;
    io_RestoreINT();
    return (tmp == AISERVO_MOVING) ? true : false;
}

void aiservoMultiRun(AIServo &s1, AIServo &s2, AIServo &s3, AIServo &s4,
                   AIServo &s5, AIServo &s6, AIServo &s7, AIServo &s8,
                   AIServo &s9, AIServo &s10, AIServo &s11, AIServo &s12,
                   AIServo &s13, AIServo &s14, AIServo &s15, AIServo &s16,
                   AIServo &s17, AIServo &s18, AIServo &s19, AIServo &s20,
                   AIServo &s21, AIServo &s22, AIServo &s23, AIServo &s24,
                   AIServo &s25, AIServo &s26, AIServo &s27, AIServo &s28,
                   AIServo &s29, AIServo &s30, AIServo &s31, AIServo &s32,
                   AIServo &s33, AIServo &s34, AIServo &s35, AIServo &s36,
                   AIServo &s37, AIServo &s38, AIServo &s39, AIServo &s40,
                   AIServo &s41, AIServo &s42, AIServo &s43, AIServo &s44,
                   AIServo &s45, AIServo &s46, AIServo &s47, AIServo &s48,
                   AIServo &s49, AIServo &s50, AIServo &s51, AIServo &s52,
                   AIServo &s53, AIServo &s54, AIServo &s55, AIServo &s56,
                   AIServo &s57, AIServo &s58, AIServo &s59, AIServo &s60,
                   AIServo &s61, AIServo &s62, AIServo &s63, AIServo &s64)
{	
	if (&s1 == &nullAIServo) // no input servo class
	{
		for (int i = 0; i < MAXSERVOS; i++) {if (Servoptr[i] != NULL) Servoptr[i]->run();}
		return;
	}
	
	 s1.run();  s2.run();  s3.run();  s4.run();  s5.run();  s6.run();  s7.run();  s8.run();
	 s9.run(); s10.run(); s11.run(); s12.run(); s13.run(); s14.run(); s15.run(); s16.run();
	s17.run(); s18.run(); s19.run(); s20.run(); s21.run(); s22.run(); s23.run(); s24.run();
	s25.run(); s26.run(); s27.run(); s28.run();	s29.run(); s30.run(); s31.run(); s32.run();
	s33.run(); s34.run(); s35.run(); s36.run(); s37.run(); s38.run(); s39.run(); s40.run();
	s41.run(); s42.run(); s43.run(); s44.run(); s45.run(); s46.run(); s47.run(); s48.run();
	s49.run(); s50.run(); s51.run(); s52.run(); s53.run(); s54.run(); s55.run(); s56.run();
	s57.run(); s58.run(); s59.run(); s60.run(); s61.run(); s62.run(); s63.run(); s64.run();
}

void aiservoMultiPause(AIServo &s1, AIServo &s2, AIServo &s3, AIServo &s4,
                     AIServo &s5, AIServo &s6, AIServo &s7, AIServo &s8,
                     AIServo &s9, AIServo &s10, AIServo &s11, AIServo &s12,
                     AIServo &s13, AIServo &s14, AIServo &s15, AIServo &s16,
                     AIServo &s17, AIServo &s18, AIServo &s19, AIServo &s20,
                     AIServo &s21, AIServo &s22, AIServo &s23, AIServo &s24,
                     AIServo &s25, AIServo &s26, AIServo &s27, AIServo &s28,
                     AIServo &s29, AIServo &s30, AIServo &s31, AIServo &s32,
                     AIServo &s33, AIServo &s34, AIServo &s35, AIServo &s36,
                     AIServo &s37, AIServo &s38, AIServo &s39, AIServo &s40,
                     AIServo &s41, AIServo &s42, AIServo &s43, AIServo &s44,
                     AIServo &s45, AIServo &s46, AIServo &s47, AIServo &s48,
                     AIServo &s49, AIServo &s50, AIServo &s51, AIServo &s52,
                     AIServo &s53, AIServo &s54, AIServo &s55, AIServo &s56,
                     AIServo &s57, AIServo &s58, AIServo &s59, AIServo &s60,
                     AIServo &s61, AIServo &s62, AIServo &s63, AIServo &s64)
{
	if (&s1 == &nullAIServo) // no input servo class
	{
		for (int i = 0; i < MAXSERVOS; i++) {if (Servoptr[i] != NULL) Servoptr[i]->pause();}
		return;
	}
	
	 s1.pause();  s2.pause();  s3.pause();  s4.pause();  s5.pause();  s6.pause();  s7.pause();  s8.pause();
	 s9.pause(); s10.pause(); s11.pause(); s12.pause(); s13.pause(); s14.pause(); s15.pause(); s16.pause();
	s17.pause(); s18.pause(); s19.pause(); s20.pause(); s21.pause(); s22.pause(); s23.pause(); s24.pause();
	s25.pause(); s26.pause(); s27.pause(); s28.pause();	s29.pause(); s30.pause(); s31.pause(); s32.pause();
	s33.pause(); s34.pause(); s35.pause(); s36.pause(); s37.pause(); s38.pause(); s39.pause(); s40.pause();
	s41.pause(); s42.pause(); s43.pause(); s44.pause(); s45.pause(); s46.pause(); s47.pause(); s48.pause();
	s49.pause(); s50.pause(); s51.pause(); s52.pause(); s53.pause(); s54.pause(); s55.pause(); s56.pause();
	s57.pause(); s58.pause(); s59.pause(); s60.pause(); s61.pause(); s62.pause(); s63.pause(); s64.pause();
}

void aiservoMultiResume(AIServo &s1, AIServo &s2, AIServo &s3, AIServo &s4,
                      AIServo &s5, AIServo &s6, AIServo &s7, AIServo &s8,
                      AIServo &s9, AIServo &s10, AIServo &s11, AIServo &s12,
                      AIServo &s13, AIServo &s14, AIServo &s15, AIServo &s16,
                      AIServo &s17, AIServo &s18, AIServo &s19, AIServo &s20,
                      AIServo &s21, AIServo &s22, AIServo &s23, AIServo &s24,
                      AIServo &s25, AIServo &s26, AIServo &s27, AIServo &s28,
                      AIServo &s29, AIServo &s30, AIServo &s31, AIServo &s32,
                      AIServo &s33, AIServo &s34, AIServo &s35, AIServo &s36,
                      AIServo &s37, AIServo &s38, AIServo &s39, AIServo &s40,
                      AIServo &s41, AIServo &s42, AIServo &s43, AIServo &s44,
                      AIServo &s45, AIServo &s46, AIServo &s47, AIServo &s48,
                      AIServo &s49, AIServo &s50, AIServo &s51, AIServo &s52,
                      AIServo &s53, AIServo &s54, AIServo &s55, AIServo &s56,
                      AIServo &s57, AIServo &s58, AIServo &s59, AIServo &s60,
                      AIServo &s61, AIServo &s62, AIServo &s63, AIServo &s64)
{
	if (&s1 == &nullAIServo) // no input servo class
	{
		for (int i = 0; i < MAXSERVOS; i++) {if (Servoptr[i] != NULL) Servoptr[i]->resume();}
		return;
	}
	
	 s1.resume();  s2.resume();  s3.resume();  s4.resume();  s5.resume();  s6.resume();  s7.resume();  s8.resume();
	 s9.resume(); s10.resume(); s11.resume(); s12.resume(); s13.resume(); s14.resume(); s15.resume(); s16.resume();
	s17.resume(); s18.resume(); s19.resume(); s20.resume(); s21.resume(); s22.resume(); s23.resume(); s24.resume();
	s25.resume(); s26.resume(); s27.resume(); s28.resume();	s29.resume(); s30.resume(); s31.resume(); s32.resume();
	s33.resume(); s34.resume(); s35.resume(); s36.resume(); s37.resume(); s38.resume(); s39.resume(); s40.resume();
	s41.resume(); s42.resume(); s43.resume(); s44.resume(); s45.resume(); s46.resume(); s47.resume(); s48.resume();
	s49.resume(); s50.resume(); s51.resume(); s52.resume(); s53.resume(); s54.resume(); s55.resume(); s56.resume();
	s57.resume(); s58.resume(); s59.resume(); s60.resume(); s61.resume(); s62.resume(); s63.resume(); s64.resume();
}

void aiservoMultiStop(AIServo &s1, AIServo &s2, AIServo &s3, AIServo &s4,
                    AIServo &s5, AIServo &s6, AIServo &s7, AIServo &s8,
                    AIServo &s9, AIServo &s10, AIServo &s11, AIServo &s12,
                    AIServo &s13, AIServo &s14, AIServo &s15, AIServo &s16,
                    AIServo &s17, AIServo &s18, AIServo &s19, AIServo &s20,
                    AIServo &s21, AIServo &s22, AIServo &s23, AIServo &s24,
                    AIServo &s25, AIServo &s26, AIServo &s27, AIServo &s28,
                    AIServo &s29, AIServo &s30, AIServo &s31, AIServo &s32,
                    AIServo &s33, AIServo &s34, AIServo &s35, AIServo &s36,
                    AIServo &s37, AIServo &s38, AIServo &s39, AIServo &s40,
                    AIServo &s41, AIServo &s42, AIServo &s43, AIServo &s44,
                    AIServo &s45, AIServo &s46, AIServo &s47, AIServo &s48,
                    AIServo &s49, AIServo &s50, AIServo &s51, AIServo &s52,
                    AIServo &s53, AIServo &s54, AIServo &s55, AIServo &s56,
                    AIServo &s57, AIServo &s58, AIServo &s59, AIServo &s60,
                    AIServo &s61, AIServo &s62, AIServo &s63, AIServo &s64)
{		
	if (&s1 == &nullAIServo) // no input servo class
	{
		for (int i = 0; i < MAXSERVOS; i++) {if (Servoptr[i] != NULL) Servoptr[i]->stop();}
		return;
	}

	 s1.stop();  s2.stop();  s3.stop();  s4.stop();  s5.stop();  s6.stop();  s7.stop();  s8.stop();
	 s9.stop(); s10.stop(); s11.stop(); s12.stop(); s13.stop(); s14.stop(); s15.stop(); s16.stop();
	s17.stop(); s18.stop(); s19.stop(); s20.stop(); s21.stop(); s22.stop(); s23.stop(); s24.stop();
	s25.stop(); s26.stop(); s27.stop(); s28.stop();	s29.stop(); s30.stop(); s31.stop(); s32.stop();
	s33.stop(); s34.stop(); s35.stop(); s36.stop(); s37.stop(); s38.stop(); s39.stop(); s40.stop();
	s41.stop(); s42.stop(); s43.stop(); s44.stop(); s45.stop(); s46.stop(); s47.stop(); s48.stop();
	s49.stop(); s50.stop(); s51.stop(); s52.stop(); s53.stop(); s54.stop(); s55.stop(); s56.stop();
	s57.stop(); s58.stop(); s59.stop(); s60.stop(); s61.stop(); s62.stop(); s63.stop(); s64.stop();
}

bool isAIServoMultiMoving(AIServo &s1, AIServo &s2, AIServo &s3, AIServo &s4,
                        AIServo &s5, AIServo &s6, AIServo &s7, AIServo &s8,
                        AIServo &s9, AIServo &s10, AIServo &s11, AIServo &s12,
                        AIServo &s13, AIServo &s14, AIServo &s15, AIServo &s16,
                        AIServo &s17, AIServo &s18, AIServo &s19, AIServo &s20,
                        AIServo &s21, AIServo &s22, AIServo &s23, AIServo &s24,
                        AIServo &s25, AIServo &s26, AIServo &s27, AIServo &s28,
                        AIServo &s29, AIServo &s30, AIServo &s31, AIServo &s32,
                        AIServo &s33, AIServo &s34, AIServo &s35, AIServo &s36,
                        AIServo &s37, AIServo &s38, AIServo &s39, AIServo &s40,
                        AIServo &s41, AIServo &s42, AIServo &s43, AIServo &s44,
                        AIServo &s45, AIServo &s46, AIServo &s47, AIServo &s48,
                        AIServo &s49, AIServo &s50, AIServo &s51, AIServo &s52,
                        AIServo &s53, AIServo &s54, AIServo &s55, AIServo &s56,
                        AIServo &s57, AIServo &s58, AIServo &s59, AIServo &s60,
                        AIServo &s61, AIServo &s62, AIServo &s63, AIServo &s64)
{
	bool tmp[MAXSERVOS] = {0};
	
	if (&s1 == &nullAIServo) // no input servo class
	{
		for (int i = 0; i < MAXSERVOS; i++)
		{
			if (Servoptr[i] != NULL)
				tmp[i] = Servoptr[i]->isMoving();
		}
		goto END;
	}
		
	tmp[0]  = s1.isMoving();   tmp[1] = s2.isMoving();   tmp[2] = s3.isMoving();  tmp[3]  = s4.isMoving();  tmp[4]  = s5.isMoving();  tmp[5]  = s6.isMoving();  tmp[6]  = s7.isMoving();
	tmp[7]  = s8.isMoving();   tmp[8] = s9.isMoving();   tmp[9] = s10.isMoving(); tmp[10] = s11.isMoving(); tmp[11] = s12.isMoving(); tmp[12] = s13.isMoving(); tmp[13] = s14.isMoving();
	tmp[14] = s15.isMoving(); tmp[15] = s16.isMoving(); tmp[16] = s17.isMoving(); tmp[17] = s18.isMoving(); tmp[18] = s19.isMoving(); tmp[19] = s20.isMoving(); tmp[20] = s21.isMoving();
	tmp[21] = s22.isMoving(); tmp[22] = s23.isMoving(); tmp[23] = s24.isMoving(); tmp[24] = s25.isMoving(); tmp[25] = s26.isMoving(); tmp[26] = s27.isMoving(); tmp[27] = s28.isMoving();
	tmp[28] = s29.isMoving(); tmp[29] = s30.isMoving(); tmp[30] = s31.isMoving(); tmp[31] = s32.isMoving(); tmp[32] = s33.isMoving(); tmp[33] = s34.isMoving(); tmp[34] = s35.isMoving();
	tmp[35] = s36.isMoving(); tmp[36] = s37.isMoving(); tmp[37] = s38.isMoving(); tmp[38] = s39.isMoving(); tmp[39] = s40.isMoving(); tmp[40] = s41.isMoving(); tmp[41] = s42.isMoving();
	tmp[42] = s43.isMoving(); tmp[43] = s44.isMoving(); tmp[44] = s45.isMoving(); tmp[45] = s46.isMoving(); tmp[46] = s47.isMoving(); tmp[47] = s48.isMoving(); tmp[48] = s49.isMoving();
	tmp[49] = s50.isMoving(); tmp[50] = s51.isMoving(); tmp[51] = s52.isMoving(); tmp[52] = s53.isMoving(); tmp[53] = s54.isMoving(); tmp[54] = s55.isMoving(); tmp[55] = s56.isMoving();
	tmp[56] = s57.isMoving(); tmp[57] = s58.isMoving(); tmp[58] = s59.isMoving(); tmp[59] = s60.isMoving(); tmp[60] = s61.isMoving(); tmp[61] = s62.isMoving(); tmp[62] = s63.isMoving();
	tmp[63] = s64.isMoving();

END:	
	for (int i = 0; i < MAXSERVOS; i++)
		if (tmp[i] == true) return true;
		
	return false;
}

void aiservoMultiRealTimeMixing(double* mixoffsets, AIServo &s1, AIServo &s2, AIServo &s3, AIServo &s4,
                              AIServo &s5, AIServo &s6, AIServo &s7, AIServo &s8,
                              AIServo &s9, AIServo &s10, AIServo &s11, AIServo &s12,
                              AIServo &s13, AIServo &s14, AIServo &s15, AIServo &s16,
                              AIServo &s17, AIServo &s18, AIServo &s19, AIServo &s20,
                              AIServo &s21, AIServo &s22, AIServo &s23, AIServo &s24,
                              AIServo &s25, AIServo &s26, AIServo &s27, AIServo &s28,
                              AIServo &s29, AIServo &s30, AIServo &s31, AIServo &s32,
                              AIServo &s33, AIServo &s34, AIServo &s35, AIServo &s36,
                              AIServo &s37, AIServo &s38, AIServo &s39, AIServo &s40,
                              AIServo &s41, AIServo &s42, AIServo &s43, AIServo &s44,
                              AIServo &s45, AIServo &s46, AIServo &s47, AIServo &s48,
                              AIServo &s49, AIServo &s50, AIServo &s51, AIServo &s52,
                              AIServo &s53, AIServo &s54, AIServo &s55, AIServo &s56,
                              AIServo &s57, AIServo &s58, AIServo &s59, AIServo &s60,
                              AIServo &s61, AIServo &s62, AIServo &s63, AIServo &s64)
{
	if (&s1 == &nullAIServo) // no input servo class
	{
		io_DisableINT();
		for (int i = 0; i < MAXSERVOS; i++) { if (Servoptr[i] != NULL) Servoptr[i]->setRealTimeMixing(mixoffsets[i], false); }
		io_RestoreINT();
		return;
	}
	
	io_DisableINT();
	s1.setRealTimeMixing(mixoffsets[s1.servoIndex], false); s2.setRealTimeMixing(mixoffsets[s2.servoIndex], false); s3.setRealTimeMixing(mixoffsets[s3.servoIndex], false);
	s4.setRealTimeMixing(mixoffsets[s4.servoIndex], false); s5.setRealTimeMixing(mixoffsets[s5.servoIndex], false); s6.setRealTimeMixing(mixoffsets[s6.servoIndex], false);
	s7.setRealTimeMixing(mixoffsets[s7.servoIndex], false); s8.setRealTimeMixing(mixoffsets[s8.servoIndex], false); s9.setRealTimeMixing(mixoffsets[s9.servoIndex], false);
	s10.setRealTimeMixing(mixoffsets[s10.servoIndex], false); s11.setRealTimeMixing(mixoffsets[s11.servoIndex], false); s12.setRealTimeMixing(mixoffsets[s12.servoIndex], false);
	s13.setRealTimeMixing(mixoffsets[s13.servoIndex], false); s14.setRealTimeMixing(mixoffsets[s14.servoIndex], false); s15.setRealTimeMixing(mixoffsets[s15.servoIndex], false);
	s16.setRealTimeMixing(mixoffsets[s16.servoIndex], false); s17.setRealTimeMixing(mixoffsets[s17.servoIndex], false); s18.setRealTimeMixing(mixoffsets[s18.servoIndex], false);
	s19.setRealTimeMixing(mixoffsets[s19.servoIndex], false); s20.setRealTimeMixing(mixoffsets[s20.servoIndex], false); s21.setRealTimeMixing(mixoffsets[s21.servoIndex], false);
	s22.setRealTimeMixing(mixoffsets[s22.servoIndex], false); s23.setRealTimeMixing(mixoffsets[s23.servoIndex], false); s24.setRealTimeMixing(mixoffsets[s24.servoIndex], false);
	s25.setRealTimeMixing(mixoffsets[s25.servoIndex], false); s26.setRealTimeMixing(mixoffsets[s26.servoIndex], false); s27.setRealTimeMixing(mixoffsets[s27.servoIndex], false);
	s28.setRealTimeMixing(mixoffsets[s28.servoIndex], false); s29.setRealTimeMixing(mixoffsets[s29.servoIndex], false); s30.setRealTimeMixing(mixoffsets[s30.servoIndex], false);
	s31.setRealTimeMixing(mixoffsets[s31.servoIndex], false); s32.setRealTimeMixing(mixoffsets[s32.servoIndex], false); s33.setRealTimeMixing(mixoffsets[s33.servoIndex], false);
	s34.setRealTimeMixing(mixoffsets[s34.servoIndex], false); s35.setRealTimeMixing(mixoffsets[s35.servoIndex], false); s36.setRealTimeMixing(mixoffsets[s36.servoIndex], false);
	s37.setRealTimeMixing(mixoffsets[s37.servoIndex], false); s38.setRealTimeMixing(mixoffsets[s38.servoIndex], false); s39.setRealTimeMixing(mixoffsets[s39.servoIndex], false);
	s40.setRealTimeMixing(mixoffsets[s40.servoIndex], false); s41.setRealTimeMixing(mixoffsets[s41.servoIndex], false); s42.setRealTimeMixing(mixoffsets[s42.servoIndex], false);
	s43.setRealTimeMixing(mixoffsets[s43.servoIndex], false); s44.setRealTimeMixing(mixoffsets[s44.servoIndex], false); s45.setRealTimeMixing(mixoffsets[s45.servoIndex], false);
	s46.setRealTimeMixing(mixoffsets[s46.servoIndex], false); s47.setRealTimeMixing(mixoffsets[s47.servoIndex], false); s48.setRealTimeMixing(mixoffsets[s48.servoIndex], false);
	s49.setRealTimeMixing(mixoffsets[s49.servoIndex], false); s50.setRealTimeMixing(mixoffsets[s50.servoIndex], false); s51.setRealTimeMixing(mixoffsets[s51.servoIndex], false);
	s52.setRealTimeMixing(mixoffsets[s52.servoIndex], false); s53.setRealTimeMixing(mixoffsets[s53.servoIndex], false); s54.setRealTimeMixing(mixoffsets[s54.servoIndex], false);
	s55.setRealTimeMixing(mixoffsets[s55.servoIndex], false); s56.setRealTimeMixing(mixoffsets[s56.servoIndex], false); s57.setRealTimeMixing(mixoffsets[s57.servoIndex], false);
	s58.setRealTimeMixing(mixoffsets[s58.servoIndex], false); s59.setRealTimeMixing(mixoffsets[s59.servoIndex], false); s60.setRealTimeMixing(mixoffsets[s60.servoIndex], false);
	s61.setRealTimeMixing(mixoffsets[s61.servoIndex], false); s62.setRealTimeMixing(mixoffsets[s62.servoIndex], false); s63.setRealTimeMixing(mixoffsets[s63.servoIndex], false);
	s64.setRealTimeMixing(mixoffsets[s64.servoIndex], false);
	io_RestoreINT();
}

/******************************** ServoFrame **********************************/

AIServoFrame::AIServoFrame()
{
	for (int i = 0; i < MAXSERVOS; i++) { positions[i] = -1; accelerations[i][0] = 0.0; accelerations[i][1] = 0.0; }
	used_servos = 0xFFFFFFFFFFFFFFFFLL;
}

AIServoFrame::AIServoFrame(const char* dir)
{
	for (int i = 0; i < MAXSERVOS; i++) { positions[i] = -1; accelerations[i][0] = 0.0; accelerations[i][1] = 0.0; }
	load(dir);
	used_servos = 0xFFFFFFFFFFFFFFFFLL;
}

void AIServoFrame::setPositions(AIServo &s1, AIServo &s2, AIServo &s3, AIServo &s4,
                                AIServo &s5, AIServo &s6, AIServo &s7, AIServo &s8,
                                AIServo &s9, AIServo &s10, AIServo &s11, AIServo &s12,
                                AIServo &s13, AIServo &s14, AIServo &s15, AIServo &s16,
                                AIServo &s17, AIServo &s18, AIServo &s19, AIServo &s20,
                                AIServo &s21, AIServo &s22, AIServo &s23, AIServo &s24,
                                AIServo &s25, AIServo &s26, AIServo &s27, AIServo &s28,
                                AIServo &s29, AIServo &s30, AIServo &s31, AIServo &s32,
                                AIServo &s33, AIServo &s34, AIServo &s35, AIServo &s36,
                                AIServo &s37, AIServo &s38, AIServo &s39, AIServo &s40,
                                AIServo &s41, AIServo &s42, AIServo &s43, AIServo &s44,
                                AIServo &s45, AIServo &s46, AIServo &s47, AIServo &s48,
                                AIServo &s49, AIServo &s50, AIServo &s51, AIServo &s52,
                                AIServo &s53, AIServo &s54, AIServo &s55, AIServo &s56,
                                AIServo &s57, AIServo &s58, AIServo &s59, AIServo &s60,
                                AIServo &s61, AIServo &s62, AIServo &s63, AIServo &s64)
{
	if(&s1 == &nullAIServo) // no input servo class
	{
		for (int i = 0; i < MAXSERVOS; i++) { if (Servoptr[i] != NULL && positions[i] < 0) Servoptr[i]->setPosition(positions[i]); }
		return;
	}

	s1.setPosition(positions[s1.servoIndex]); s2.setPosition(positions[s2.servoIndex]); s3.setPosition(positions[s3.servoIndex]);
	s4.setPosition(positions[s4.servoIndex]); s5.setPosition(positions[s5.servoIndex]); s6.setPosition(positions[s6.servoIndex]);
	s7.setPosition(positions[s7.servoIndex]); s8.setPosition(positions[s8.servoIndex]); s9.setPosition(positions[s9.servoIndex]);
	s10.setPosition(positions[s10.servoIndex]); s11.setPosition(positions[s11.servoIndex]); s12.setPosition(positions[s12.servoIndex]);
	s13.setPosition(positions[s13.servoIndex]); s14.setPosition(positions[s14.servoIndex]); s15.setPosition(positions[s15.servoIndex]);
	s16.setPosition(positions[s16.servoIndex]); s17.setPosition(positions[s17.servoIndex]); s18.setPosition(positions[s18.servoIndex]);
	s19.setPosition(positions[s19.servoIndex]); s20.setPosition(positions[s20.servoIndex]); s21.setPosition(positions[s21.servoIndex]);
	s22.setPosition(positions[s22.servoIndex]); s23.setPosition(positions[s23.servoIndex]); s24.setPosition(positions[s24.servoIndex]);
	s25.setPosition(positions[s25.servoIndex]); s26.setPosition(positions[s26.servoIndex]); s27.setPosition(positions[s27.servoIndex]);
	s28.setPosition(positions[s28.servoIndex]); s29.setPosition(positions[s29.servoIndex]); s30.setPosition(positions[s30.servoIndex]);
	s31.setPosition(positions[s31.servoIndex]); s32.setPosition(positions[s32.servoIndex]); s33.setPosition(positions[s33.servoIndex]);
	s34.setPosition(positions[s34.servoIndex]); s35.setPosition(positions[s35.servoIndex]); s36.setPosition(positions[s36.servoIndex]);
	s37.setPosition(positions[s37.servoIndex]); s38.setPosition(positions[s38.servoIndex]); s39.setPosition(positions[s39.servoIndex]);
	s40.setPosition(positions[s40.servoIndex]); s41.setPosition(positions[s41.servoIndex]); s42.setPosition(positions[s42.servoIndex]);
	s43.setPosition(positions[s43.servoIndex]); s44.setPosition(positions[s44.servoIndex]); s45.setPosition(positions[s45.servoIndex]);
	s46.setPosition(positions[s46.servoIndex]); s47.setPosition(positions[s47.servoIndex]); s48.setPosition(positions[s48.servoIndex]);
	s49.setPosition(positions[s49.servoIndex]); s50.setPosition(positions[s50.servoIndex]); s51.setPosition(positions[s51.servoIndex]);
	s52.setPosition(positions[s52.servoIndex]); s53.setPosition(positions[s53.servoIndex]); s54.setPosition(positions[s54.servoIndex]);
	s55.setPosition(positions[s55.servoIndex]); s56.setPosition(positions[s56.servoIndex]); s57.setPosition(positions[s57.servoIndex]);
	s58.setPosition(positions[s58.servoIndex]); s59.setPosition(positions[s59.servoIndex]); s60.setPosition(positions[s60.servoIndex]);
	s61.setPosition(positions[s61.servoIndex]); s62.setPosition(positions[s62.servoIndex]); s63.setPosition(positions[s63.servoIndex]);
	s64.setPosition(positions[s64.servoIndex]);
}

void AIServoFrame::setPositions(unsigned long playtime, AIServo &s1, AIServo &s2, AIServo &s3, AIServo &s4,
                                AIServo &s5, AIServo &s6, AIServo &s7, AIServo &s8,
                                AIServo &s9, AIServo &s10, AIServo &s11, AIServo &s12,
                                AIServo &s13, AIServo &s14, AIServo &s15, AIServo &s16,
                                AIServo &s17, AIServo &s18, AIServo &s19, AIServo &s20,
                                AIServo &s21, AIServo &s22, AIServo &s23, AIServo &s24,
                                AIServo &s25, AIServo &s26, AIServo &s27, AIServo &s28,
                                AIServo &s29, AIServo &s30, AIServo &s31, AIServo &s32,
                                AIServo &s33, AIServo &s34, AIServo &s35, AIServo &s36,
                                AIServo &s37, AIServo &s38, AIServo &s39, AIServo &s40,
                                AIServo &s41, AIServo &s42, AIServo &s43, AIServo &s44,
                                AIServo &s45, AIServo &s46, AIServo &s47, AIServo &s48,
                                AIServo &s49, AIServo &s50, AIServo &s51, AIServo &s52,
                                AIServo &s53, AIServo &s54, AIServo &s55, AIServo &s56,
                                AIServo &s57, AIServo &s58, AIServo &s59, AIServo &s60,
                                AIServo &s61, AIServo &s62, AIServo &s63, AIServo &s64)
{
	if(&s1 == &nullAIServo) // no input servo class
	{
		for (int i = 0; i < MAXSERVOS; i++) { if (Servoptr[i] != NULL && positions[i] >= 0.0) Servoptr[i]->setPosition(positions[i], playtime); }
		return;
	}

	s1.setPosition(positions[s1.servoIndex], playtime); s2.setPosition(positions[s2.servoIndex], playtime); s3.setPosition(positions[s3.servoIndex], playtime);
	s4.setPosition(positions[s4.servoIndex], playtime); s5.setPosition(positions[s5.servoIndex], playtime); s6.setPosition(positions[s6.servoIndex], playtime);
	s7.setPosition(positions[s7.servoIndex], playtime); s8.setPosition(positions[s8.servoIndex], playtime); s9.setPosition(positions[s9.servoIndex], playtime);
	s10.setPosition(positions[s10.servoIndex], playtime); s11.setPosition(positions[s11.servoIndex], playtime); s12.setPosition(positions[s12.servoIndex], playtime);
	s13.setPosition(positions[s13.servoIndex], playtime); s14.setPosition(positions[s14.servoIndex], playtime); s15.setPosition(positions[s15.servoIndex], playtime);
	s16.setPosition(positions[s16.servoIndex], playtime); s17.setPosition(positions[s17.servoIndex], playtime); s18.setPosition(positions[s18.servoIndex], playtime);
	s19.setPosition(positions[s19.servoIndex], playtime); s20.setPosition(positions[s20.servoIndex], playtime); s21.setPosition(positions[s21.servoIndex], playtime);
	s22.setPosition(positions[s22.servoIndex], playtime); s23.setPosition(positions[s23.servoIndex], playtime); s24.setPosition(positions[s24.servoIndex], playtime);
	s25.setPosition(positions[s25.servoIndex], playtime); s26.setPosition(positions[s26.servoIndex], playtime); s27.setPosition(positions[s27.servoIndex], playtime);
	s28.setPosition(positions[s28.servoIndex], playtime); s29.setPosition(positions[s29.servoIndex], playtime); s30.setPosition(positions[s30.servoIndex], playtime);
	s31.setPosition(positions[s31.servoIndex], playtime); s32.setPosition(positions[s32.servoIndex], playtime); s33.setPosition(positions[s33.servoIndex], playtime);
	s34.setPosition(positions[s34.servoIndex], playtime); s35.setPosition(positions[s35.servoIndex], playtime); s36.setPosition(positions[s36.servoIndex], playtime);
	s37.setPosition(positions[s37.servoIndex], playtime); s38.setPosition(positions[s38.servoIndex], playtime); s39.setPosition(positions[s39.servoIndex], playtime);
	s40.setPosition(positions[s40.servoIndex], playtime); s41.setPosition(positions[s41.servoIndex], playtime); s42.setPosition(positions[s42.servoIndex], playtime);
	s43.setPosition(positions[s43.servoIndex], playtime); s44.setPosition(positions[s44.servoIndex], playtime); s45.setPosition(positions[s45.servoIndex], playtime);
	s46.setPosition(positions[s46.servoIndex], playtime); s47.setPosition(positions[s47.servoIndex], playtime); s48.setPosition(positions[s48.servoIndex], playtime);
	s49.setPosition(positions[s49.servoIndex], playtime); s50.setPosition(positions[s50.servoIndex], playtime); s51.setPosition(positions[s51.servoIndex], playtime);
	s52.setPosition(positions[s52.servoIndex], playtime); s53.setPosition(positions[s53.servoIndex], playtime); s54.setPosition(positions[s54.servoIndex], playtime);
	s55.setPosition(positions[s55.servoIndex], playtime); s56.setPosition(positions[s56.servoIndex], playtime); s57.setPosition(positions[s57.servoIndex], playtime);
	s58.setPosition(positions[s58.servoIndex], playtime); s59.setPosition(positions[s59.servoIndex], playtime); s60.setPosition(positions[s60.servoIndex], playtime);
	s61.setPosition(positions[s61.servoIndex], playtime); s62.setPosition(positions[s62.servoIndex], playtime); s63.setPosition(positions[s63.servoIndex], playtime);
	s64.setPosition(positions[s64.servoIndex], playtime);
}

void AIServoFrame::setPositions(int playtime, AIServo &s1, AIServo &s2, AIServo &s3, AIServo &s4,
                              AIServo &s5, AIServo &s6, AIServo &s7, AIServo &s8,
                              AIServo &s9, AIServo &s10, AIServo &s11, AIServo &s12,
                              AIServo &s13, AIServo &s14, AIServo &s15, AIServo &s16,
                              AIServo &s17, AIServo &s18, AIServo &s19, AIServo &s20,
                              AIServo &s21, AIServo &s22, AIServo &s23, AIServo &s24,
                              AIServo &s25, AIServo &s26, AIServo &s27, AIServo &s28,
                              AIServo &s29, AIServo &s30, AIServo &s31, AIServo &s32,
                              AIServo &s33, AIServo &s34, AIServo &s35, AIServo &s36,
                              AIServo &s37, AIServo &s38, AIServo &s39, AIServo &s40,
                              AIServo &s41, AIServo &s42, AIServo &s43, AIServo &s44,
                              AIServo &s45, AIServo &s46, AIServo &s47, AIServo &s48,
                              AIServo &s49, AIServo &s50, AIServo &s51, AIServo &s52,
                              AIServo &s53, AIServo &s54, AIServo &s55, AIServo &s56,
                              AIServo &s57, AIServo &s58, AIServo &s59, AIServo &s60,
                              AIServo &s61, AIServo &s62, AIServo &s63, AIServo &s64)
{
	if (playtime <= 0) playtime = 0;
	setPositions((unsigned long) playtime, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20,
	              s21, s22, s23, s24, s25, s26, s27, s28, s29, s30, s31, s32, s33, s34, s35, s36, s37, s38, s39, s40,
	              s41, s42, s43, s44, s45, s46, s47, s48, s49, s50, s51, s52, s53, s54, s55, s56, s57, s58, s59, s60,
				  s61, s62, s63, s64);					  
}

static void get_real_path(const char* dir, char* path)
{
    int i;
	const char* parent = "C:\\";
	
	if (dir[0] == 'c' || dir[0] == 'C' || dir[0] == 'a' || dir[0] == 'A')
	{
		if (dir[1] == ':' && dir[2] == '\\')
		{
			for (i=0; dir[i] != '\0'; i++) path[i] = dir[i];
			return;
		}
	}

	for (i=0; i<3; i++) path[i] = parent[i];
	for (i=0; dir[i] != '\0'; i++) path[i+3] = dir[i];

	return;
}

static bool analysis(const char* line, int* channel, long long* value)
{
	int i, j = 0;
	char tmp[256] = {'\0'};
	bool negative = false;
	
	for (i=0; line[i] != '\0' && line[i] != '\n'; i++)
	{
		if (line[i] == ';') break;
		if (line[i] != ' ') tmp[j++] = line[i];
	}
	
	if (strncmp("channel", tmp, 7) != 0) return false;

	if (isdigit(tmp[7]))
	{
		if (tmp[8] == '=')
		{
			*channel = tmp[7] - '0'; i = 9;
		}			
		else if (isdigit(tmp[8]) && tmp[9] == '=')
		{
			*channel = (tmp[7] - '0')*10 + (tmp[8] - '0'); i = 10;
		}
		else
			return false;
	}
	else
		return false;
	
	*value = 0;
	if (tmp[i] == '-') {negative = true; i++;}	
	for (;isdigit(tmp[i]) != 0; i++) *value = *value*10 + (tmp[i] - '0');
	
	if (tmp[i] != '\0') return false;
	if (negative == true) *value = (-1) * (*value);
	
	return true;
}

bool AIServoFrame::load(const char* dir)
{
	int i, channel;
	FILE *fp;
	char path[256] = {'\0'};
	char _line[256] = {'\0'};
	long long value;
	long long tmp[MAXSERVOS];
	for (i=0; i<MAXSERVOS; i++) tmp[i] = -1000000;
	
	if (dir == NULL) return false;

	get_real_path(dir, path);

	if ((fp = fopen(path, "r")) == NULL) return false;

	while(fgets(_line, 256, fp))
	{
		for (i=0; _line[i] != '\0' && _line[i] != '\n'; i++)
			if (_line[i] == ' ') continue; else break;

		if (_line[i] == ';' || _line[i] == '\0' || _line[i] == '\n') continue;
		
		if (analysis(_line, &channel, &value) == true && channel < MAXSERVOS)
			tmp[channel] = value;
		else
		{
			fclose(fp);
			return false;
		}
	}
	
	fclose(fp);
	
	for (i=0; i<MAXSERVOS; i++) positions[i] = (double)(((double)tmp[i])/1000000.0);

	return true;
}

bool AIServoFrame::load(const String &s)
{
	return load(s.c_str());
}

void combine(char* result, int channel, long long value)
{
	int i, j;
	const char* head = "channel";
	char tmp[256] = {'\0'};
	char num[16]  = {'\0'};
	
	for (i=0; i<7; i++) tmp[i] = head[i];
	itoa(channel, num, 10);
	for (j=0; num[j] != '\0'; j++) tmp[i++] = num[j];
	tmp[i++] = '=';
	for (j=0; j<16; j++) num[j] = '\0';
	ltoa(value, num, 10);
	for (j = 0; num[j] != '\0'; j++) tmp[i++] = num[j];
	tmp[i] = '\n';
	
	strncpy(result, tmp, 256);
}

bool AIServoFrame::save(const char* dir)
{
    int i;
	FILE *fp;
	char path[256] = {'\0'};
	char _line[256] = {'\0'};
	
	if (dir == NULL) return false;

	get_real_path(dir, path);
	
	if ((fp = fopen(path, "a")) == NULL) return false;
	
	for (i = 0; i < MAXSERVOS; i++)
	{
		if (positions[i] < 0.0) continue;
		combine(_line, i, (long long)(positions[i]*MEGA));
		fputs(_line, fp);
	}
	fputs("\n", fp);
	fclose(fp);
	return true;
}

bool AIServoFrame::save(const String &s)
{
	return save(s.c_str());
}

void AIServoFrame::playPositions(AIServo &s1, AIServo &s2, AIServo &s3, AIServo &s4,
                               AIServo &s5, AIServo &s6, AIServo &s7, AIServo &s8,
                               AIServo &s9, AIServo &s10, AIServo &s11, AIServo &s12,
                               AIServo &s13, AIServo &s14, AIServo &s15, AIServo &s16,
                               AIServo &s17, AIServo &s18, AIServo &s19, AIServo &s20,
                               AIServo &s21, AIServo &s22, AIServo &s23, AIServo &s24,
                               AIServo &s25, AIServo &s26, AIServo &s27, AIServo &s28,
                               AIServo &s29, AIServo &s30, AIServo &s31, AIServo &s32,
                               AIServo &s33, AIServo &s34, AIServo &s35, AIServo &s36,
                               AIServo &s37, AIServo &s38, AIServo &s39, AIServo &s40,
                               AIServo &s41, AIServo &s42, AIServo &s43, AIServo &s44,
                               AIServo &s45, AIServo &s46, AIServo &s47, AIServo &s48,
                               AIServo &s49, AIServo &s50, AIServo &s51, AIServo &s52,
                               AIServo &s53, AIServo &s54, AIServo &s55, AIServo &s56,
                               AIServo &s57, AIServo &s58, AIServo &s59, AIServo &s60,
                               AIServo &s61, AIServo &s62, AIServo &s63, AIServo &s64)
{
	if (&s1 == &nullAIServo) // no input servo class
	{  
		for (int i = 0; i < MAXSERVOS; i++) {if (Servoptr[i] != NULL && positions[i] >= 0.0 && (used_servos & (0x01LL<<i)) != 0LL) Servoptr[i]->setPosition(positions[i]);}
		for (int i = 0; i < MAXSERVOS; i++) {if (Servoptr[i] != NULL && positions[i] >= 0.0 && (used_servos & (0x01LL<<i)) != 0LL) Servoptr[i]->run();}
		return;
	}

	s1.setPosition(positions[s1.servoIndex]); s2.setPosition(positions[s2.servoIndex]); s3.setPosition(positions[s3.servoIndex]);
	s4.setPosition(positions[s4.servoIndex]); s5.setPosition(positions[s5.servoIndex]); s6.setPosition(positions[s6.servoIndex]);
	s7.setPosition(positions[s7.servoIndex]); s8.setPosition(positions[s8.servoIndex]); s9.setPosition(positions[s9.servoIndex]);
	s10.setPosition(positions[s10.servoIndex]); s11.setPosition(positions[s11.servoIndex]); s12.setPosition(positions[s12.servoIndex]);
	s13.setPosition(positions[s13.servoIndex]); s14.setPosition(positions[s14.servoIndex]); s15.setPosition(positions[s15.servoIndex]);
	s16.setPosition(positions[s16.servoIndex]); s17.setPosition(positions[s17.servoIndex]); s18.setPosition(positions[s18.servoIndex]);
	s19.setPosition(positions[s19.servoIndex]); s20.setPosition(positions[s20.servoIndex]); s21.setPosition(positions[s21.servoIndex]);
	s22.setPosition(positions[s22.servoIndex]); s23.setPosition(positions[s23.servoIndex]); s24.setPosition(positions[s24.servoIndex]);
	s25.setPosition(positions[s25.servoIndex]); s26.setPosition(positions[s26.servoIndex]); s27.setPosition(positions[s27.servoIndex]);
	s28.setPosition(positions[s28.servoIndex]); s29.setPosition(positions[s29.servoIndex]); s30.setPosition(positions[s30.servoIndex]);
	s31.setPosition(positions[s31.servoIndex]); s32.setPosition(positions[s32.servoIndex]); s33.setPosition(positions[s33.servoIndex]);
	s34.setPosition(positions[s34.servoIndex]); s35.setPosition(positions[s35.servoIndex]); s36.setPosition(positions[s36.servoIndex]);
	s37.setPosition(positions[s37.servoIndex]); s38.setPosition(positions[s38.servoIndex]); s39.setPosition(positions[s39.servoIndex]);
	s40.setPosition(positions[s40.servoIndex]); s41.setPosition(positions[s41.servoIndex]); s42.setPosition(positions[s42.servoIndex]);
	s43.setPosition(positions[s43.servoIndex]); s44.setPosition(positions[s44.servoIndex]); s45.setPosition(positions[s45.servoIndex]);
	s46.setPosition(positions[s46.servoIndex]); s47.setPosition(positions[s47.servoIndex]); s48.setPosition(positions[s48.servoIndex]);
	s49.setPosition(positions[s49.servoIndex]); s50.setPosition(positions[s50.servoIndex]); s51.setPosition(positions[s51.servoIndex]);
	s52.setPosition(positions[s52.servoIndex]); s53.setPosition(positions[s53.servoIndex]); s54.setPosition(positions[s54.servoIndex]);
	s55.setPosition(positions[s55.servoIndex]); s56.setPosition(positions[s56.servoIndex]); s57.setPosition(positions[s57.servoIndex]);
	s58.setPosition(positions[s58.servoIndex]); s59.setPosition(positions[s59.servoIndex]); s60.setPosition(positions[s60.servoIndex]);
	s61.setPosition(positions[s61.servoIndex]); s62.setPosition(positions[s62.servoIndex]); s63.setPosition(positions[s63.servoIndex]);
	s64.setPosition(positions[s64.servoIndex]);
    aiservoMultiRun(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20,
	              s21, s22, s23, s24, s25, s26, s27, s28, s29, s30, s31, s32, s33, s34, s35, s36, s37, s38, s39, s40,
	              s41, s42, s43, s44, s45, s46, s47, s48, s49, s50, s51, s52, s53, s54, s55, s56, s57, s58, s59, s60,
				  s61, s62, s63, s64);
}

void AIServoFrame::playPositions(unsigned long long enabled_servos, AIServo &s1, AIServo &s2, AIServo &s3, AIServo &s4,
                               AIServo &s5, AIServo &s6, AIServo &s7, AIServo &s8,
                               AIServo &s9, AIServo &s10, AIServo &s11, AIServo &s12,
                               AIServo &s13, AIServo &s14, AIServo &s15, AIServo &s16,
                               AIServo &s17, AIServo &s18, AIServo &s19, AIServo &s20,
                               AIServo &s21, AIServo &s22, AIServo &s23, AIServo &s24,
                               AIServo &s25, AIServo &s26, AIServo &s27, AIServo &s28,
                               AIServo &s29, AIServo &s30, AIServo &s31, AIServo &s32,
                               AIServo &s33, AIServo &s34, AIServo &s35, AIServo &s36,
                               AIServo &s37, AIServo &s38, AIServo &s39, AIServo &s40,
                               AIServo &s41, AIServo &s42, AIServo &s43, AIServo &s44,
                               AIServo &s45, AIServo &s46, AIServo &s47, AIServo &s48,
                               AIServo &s49, AIServo &s50, AIServo &s51, AIServo &s52,
                               AIServo &s53, AIServo &s54, AIServo &s55, AIServo &s56,
                               AIServo &s57, AIServo &s58, AIServo &s59, AIServo &s60,
                               AIServo &s61, AIServo &s62, AIServo &s63, AIServo &s64)
{
	used_servos = enabled_servos;
	playPositions(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20,
	              s21, s22, s23, s24, s25, s26, s27, s28, s29, s30, s31, s32, s33, s34, s35, s36, s37, s38, s39, s40,
	              s41, s42, s43, s44, s45, s46, s47, s48, s49, s50, s51, s52, s53, s54, s55, s56, s57, s58, s59, s60,
				  s61, s62, s63, s64);
	used_servos = 0xFFFFFFFFFFFFFFFFLL;
}

void AIServoFrame::playPositions(unsigned long playtime, AIServo &s1, AIServo &s2, AIServo &s3, AIServo &s4,
                               AIServo &s5, AIServo &s6, AIServo &s7, AIServo &s8,
                               AIServo &s9, AIServo &s10, AIServo &s11, AIServo &s12,
                               AIServo &s13, AIServo &s14, AIServo &s15, AIServo &s16,
                               AIServo &s17, AIServo &s18, AIServo &s19, AIServo &s20,
                               AIServo &s21, AIServo &s22, AIServo &s23, AIServo &s24,
                               AIServo &s25, AIServo &s26, AIServo &s27, AIServo &s28,
                               AIServo &s29, AIServo &s30, AIServo &s31, AIServo &s32,
                               AIServo &s33, AIServo &s34, AIServo &s35, AIServo &s36,
                               AIServo &s37, AIServo &s38, AIServo &s39, AIServo &s40,
                               AIServo &s41, AIServo &s42, AIServo &s43, AIServo &s44,
                               AIServo &s45, AIServo &s46, AIServo &s47, AIServo &s48,
                               AIServo &s49, AIServo &s50, AIServo &s51, AIServo &s52,
                               AIServo &s53, AIServo &s54, AIServo &s55, AIServo &s56,
                               AIServo &s57, AIServo &s58, AIServo &s59, AIServo &s60,
                               AIServo &s61, AIServo &s62, AIServo &s63, AIServo &s64)
{                               
	int i;

	if (&s1 == &nullAIServo) // no input servo class
	{
		for (i = 0; i < MAXSERVOS; i++)
		{
			if (Servoptr[i] != NULL)
			{
				Servoptr[i]->setPositionAcceleration(accelerations[i][0], accelerations[i][1]);
				if (playtime == 0L && (accelerations[i][0] != 0.0 || accelerations[i][1] != 0.0)) playtime = 20L; // If we have acceleration, the playtime is not as 0.
				accelerations[i][0] = 0.0; accelerations[i][1] = 0.0;
			}
		}
		for (i = 0; i < MAXSERVOS; i++) {if (Servoptr[i] != NULL && positions[i] >= 0.0 && (used_servos & (0x01LL<<i)) != 0LL) Servoptr[i]->setPosition(positions[i], playtime);}
		for (i = 0; i < MAXSERVOS; i++) {if (Servoptr[i] != NULL && positions[i] >= 0.0 && (used_servos & (0x01LL<<i)) != 0LL) Servoptr[i]->run();}
		return;
	}

	s1.setPosition(positions[s1.servoIndex], playtime); s2.setPosition(positions[s2.servoIndex], playtime); s3.setPosition(positions[s3.servoIndex], playtime);
	s4.setPosition(positions[s4.servoIndex], playtime); s5.setPosition(positions[s5.servoIndex], playtime); s6.setPosition(positions[s6.servoIndex], playtime);
	s7.setPosition(positions[s7.servoIndex], playtime); s8.setPosition(positions[s8.servoIndex], playtime); s9.setPosition(positions[s9.servoIndex], playtime);
	s10.setPosition(positions[s10.servoIndex], playtime); s11.setPosition(positions[s11.servoIndex], playtime); s12.setPosition(positions[s12.servoIndex], playtime);
	s13.setPosition(positions[s13.servoIndex], playtime); s14.setPosition(positions[s14.servoIndex], playtime); s15.setPosition(positions[s15.servoIndex], playtime);
	s16.setPosition(positions[s16.servoIndex], playtime); s17.setPosition(positions[s17.servoIndex], playtime); s18.setPosition(positions[s18.servoIndex], playtime);
	s19.setPosition(positions[s19.servoIndex], playtime); s20.setPosition(positions[s20.servoIndex], playtime); s21.setPosition(positions[s21.servoIndex], playtime);
	s22.setPosition(positions[s22.servoIndex], playtime); s23.setPosition(positions[s23.servoIndex], playtime); s24.setPosition(positions[s24.servoIndex], playtime);
	s25.setPosition(positions[s25.servoIndex], playtime); s26.setPosition(positions[s26.servoIndex], playtime); s27.setPosition(positions[s27.servoIndex], playtime);
	s28.setPosition(positions[s28.servoIndex], playtime); s29.setPosition(positions[s29.servoIndex], playtime); s30.setPosition(positions[s30.servoIndex], playtime);
	s31.setPosition(positions[s31.servoIndex], playtime); s32.setPosition(positions[s32.servoIndex], playtime); s33.setPosition(positions[s33.servoIndex], playtime);
	s34.setPosition(positions[s34.servoIndex], playtime); s35.setPosition(positions[s35.servoIndex], playtime); s36.setPosition(positions[s36.servoIndex], playtime);
	s37.setPosition(positions[s37.servoIndex], playtime); s38.setPosition(positions[s38.servoIndex], playtime); s39.setPosition(positions[s39.servoIndex], playtime);
	s40.setPosition(positions[s40.servoIndex], playtime); s41.setPosition(positions[s41.servoIndex], playtime); s42.setPosition(positions[s42.servoIndex], playtime);
	s43.setPosition(positions[s43.servoIndex], playtime); s44.setPosition(positions[s44.servoIndex], playtime); s45.setPosition(positions[s45.servoIndex], playtime);
	s46.setPosition(positions[s46.servoIndex], playtime); s47.setPosition(positions[s47.servoIndex], playtime); s48.setPosition(positions[s48.servoIndex], playtime);
	s49.setPosition(positions[s49.servoIndex], playtime); s50.setPosition(positions[s50.servoIndex], playtime); s51.setPosition(positions[s51.servoIndex], playtime);
	s52.setPosition(positions[s52.servoIndex], playtime); s53.setPosition(positions[s53.servoIndex], playtime); s54.setPosition(positions[s54.servoIndex], playtime);
	s55.setPosition(positions[s55.servoIndex], playtime); s56.setPosition(positions[s56.servoIndex], playtime); s57.setPosition(positions[s57.servoIndex], playtime);
	s58.setPosition(positions[s58.servoIndex], playtime); s59.setPosition(positions[s59.servoIndex], playtime); s60.setPosition(positions[s60.servoIndex], playtime);
	s61.setPosition(positions[s61.servoIndex], playtime); s62.setPosition(positions[s62.servoIndex], playtime); s63.setPosition(positions[s63.servoIndex], playtime);
	s64.setPosition(positions[s64.servoIndex], playtime);
    aiservoMultiRun(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20,
	              s21, s22, s23, s24, s25, s26, s27, s28, s29, s30, s31, s32, s33, s34, s35, s36, s37, s38, s39, s40,
	              s41, s42, s43, s44, s45, s46, s47, s48, s49, s50, s51, s52, s53, s54, s55, s56, s57, s58, s59, s60,
				  s61, s62, s63, s64);
}

void AIServoFrame::playPositions(int playtime, AIServo &s1, AIServo &s2, AIServo &s3, AIServo &s4,
                               AIServo &s5, AIServo &s6, AIServo &s7, AIServo &s8,
                               AIServo &s9, AIServo &s10, AIServo &s11, AIServo &s12,
                               AIServo &s13, AIServo &s14, AIServo &s15, AIServo &s16,
                               AIServo &s17, AIServo &s18, AIServo &s19, AIServo &s20,
                               AIServo &s21, AIServo &s22, AIServo &s23, AIServo &s24,
                               AIServo &s25, AIServo &s26, AIServo &s27, AIServo &s28,
                               AIServo &s29, AIServo &s30, AIServo &s31, AIServo &s32,
                               AIServo &s33, AIServo &s34, AIServo &s35, AIServo &s36,
                               AIServo &s37, AIServo &s38, AIServo &s39, AIServo &s40,
                               AIServo &s41, AIServo &s42, AIServo &s43, AIServo &s44,
                               AIServo &s45, AIServo &s46, AIServo &s47, AIServo &s48,
                               AIServo &s49, AIServo &s50, AIServo &s51, AIServo &s52,
                               AIServo &s53, AIServo &s54, AIServo &s55, AIServo &s56,
                               AIServo &s57, AIServo &s58, AIServo &s59, AIServo &s60,
                               AIServo &s61, AIServo &s62, AIServo &s63, AIServo &s64)
{
	if (playtime <= 0) playtime = 0;
	playPositions((unsigned long) playtime, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20,
	              s21, s22, s23, s24, s25, s26, s27, s28, s29, s30, s31, s32, s33, s34, s35, s36, s37, s38, s39, s40,
	              s41, s42, s43, s44, s45, s46, s47, s48, s49, s50, s51, s52, s53, s54, s55, s56, s57, s58, s59, s60,
				  s61, s62, s63, s64);
}

void AIServoFrame::playPositions(unsigned long playtime, unsigned long long enabled_servos, AIServo &s1, AIServo &s2, AIServo &s3, AIServo &s4,
                               AIServo &s5, AIServo &s6, AIServo &s7, AIServo &s8,
                               AIServo &s9, AIServo &s10, AIServo &s11, AIServo &s12,
                               AIServo &s13, AIServo &s14, AIServo &s15, AIServo &s16,
                               AIServo &s17, AIServo &s18, AIServo &s19, AIServo &s20,
                               AIServo &s21, AIServo &s22, AIServo &s23, AIServo &s24,
                               AIServo &s25, AIServo &s26, AIServo &s27, AIServo &s28,
                               AIServo &s29, AIServo &s30, AIServo &s31, AIServo &s32,
                               AIServo &s33, AIServo &s34, AIServo &s35, AIServo &s36,
                               AIServo &s37, AIServo &s38, AIServo &s39, AIServo &s40,
                               AIServo &s41, AIServo &s42, AIServo &s43, AIServo &s44,
                               AIServo &s45, AIServo &s46, AIServo &s47, AIServo &s48,
                               AIServo &s49, AIServo &s50, AIServo &s51, AIServo &s52,
                               AIServo &s53, AIServo &s54, AIServo &s55, AIServo &s56,
                               AIServo &s57, AIServo &s58, AIServo &s59, AIServo &s60,
                               AIServo &s61, AIServo &s62, AIServo &s63, AIServo &s64)
{
	used_servos = enabled_servos;
	playPositions(playtime, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20,
	              s21, s22, s23, s24, s25, s26, s27, s28, s29, s30, s31, s32, s33, s34, s35, s36, s37, s38, s39, s40,
	              s41, s42, s43, s44, s45, s46, s47, s48, s49, s50, s51, s52, s53, s54, s55, s56, s57, s58, s59, s60,
				  s61, s62, s63, s64);
	used_servos = 0xFFFFFFFFFFFFFFFFLL;
}

void AIServoFrame::playPositions(int playtime, unsigned long long enabled_servos, AIServo &s1, AIServo &s2, AIServo &s3, AIServo &s4,
                               AIServo &s5, AIServo &s6, AIServo &s7, AIServo &s8,
                               AIServo &s9, AIServo &s10, AIServo &s11, AIServo &s12,
                               AIServo &s13, AIServo &s14, AIServo &s15, AIServo &s16,
                               AIServo &s17, AIServo &s18, AIServo &s19, AIServo &s20,
                               AIServo &s21, AIServo &s22, AIServo &s23, AIServo &s24,
                               AIServo &s25, AIServo &s26, AIServo &s27, AIServo &s28,
                               AIServo &s29, AIServo &s30, AIServo &s31, AIServo &s32,
                               AIServo &s33, AIServo &s34, AIServo &s35, AIServo &s36,
                               AIServo &s37, AIServo &s38, AIServo &s39, AIServo &s40,
                               AIServo &s41, AIServo &s42, AIServo &s43, AIServo &s44,
                               AIServo &s45, AIServo &s46, AIServo &s47, AIServo &s48,
                               AIServo &s49, AIServo &s50, AIServo &s51, AIServo &s52,
                               AIServo &s53, AIServo &s54, AIServo &s55, AIServo &s56,
                               AIServo &s57, AIServo &s58, AIServo &s59, AIServo &s60,
                               AIServo &s61, AIServo &s62, AIServo &s63, AIServo &s64)
{
	if (playtime <= 0) playtime = 0;
	used_servos = enabled_servos;
	playPositions((unsigned long) playtime, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20,
	              s21, s22, s23, s24, s25, s26, s27, s28, s29, s30, s31, s32, s33, s34, s35, s36, s37, s38, s39, s40,
	              s41, s42, s43, s44, s45, s46, s47, s48, s49, s50, s51, s52, s53, s54, s55, s56, s57, s58, s59, s60,
				  s61, s62, s63, s64);
	used_servos = 0xFFFFFFFFFFFFFFFFLL;
}

/********************************* Servo Offset *******************************/
AIServoOffset::AIServoOffset()
{
	for(int i = 0; i < MAXSERVOS; i++)
		offsets[i] = 0.0;
}

AIServoOffset::AIServoOffset(const char* dir)
{
	for (int i = 0; i < MAXSERVOS; i++)
		offsets[i] = 0.0;
	load(dir);
}

void AIServoOffset::setOffsets(AIServo &s1, AIServo &s2, AIServo &s3, AIServo &s4,
                               AIServo &s5, AIServo &s6, AIServo &s7, AIServo &s8,
                               AIServo &s9, AIServo &s10, AIServo &s11, AIServo &s12,
                               AIServo &s13, AIServo &s14, AIServo &s15, AIServo &s16,
                               AIServo &s17, AIServo &s18, AIServo &s19, AIServo &s20,
                               AIServo &s21, AIServo &s22, AIServo &s23, AIServo &s24,
                               AIServo &s25, AIServo &s26, AIServo &s27, AIServo &s28,
                               AIServo &s29, AIServo &s30, AIServo &s31, AIServo &s32,
                               AIServo &s33, AIServo &s34, AIServo &s35, AIServo &s36,
                               AIServo &s37, AIServo &s38, AIServo &s39, AIServo &s40,
                               AIServo &s41, AIServo &s42, AIServo &s43, AIServo &s44,
                               AIServo &s45, AIServo &s46, AIServo &s47, AIServo &s48,
                               AIServo &s49, AIServo &s50, AIServo &s51, AIServo &s52,
                               AIServo &s53, AIServo &s54, AIServo &s55, AIServo &s56,
                               AIServo &s57, AIServo &s58, AIServo &s59, AIServo &s60,
                               AIServo &s61, AIServo &s62, AIServo &s63, AIServo &s64)
{
	if (&s1 == &nullAIServo) // no input servo class
	{
		for (int i = 0; i < MAXSERVOS; i++) { if (Servoptr[i] != NULL && offsets[i] != 0.0) Servoptr[i]->setOffset(offsets[i]); }
		return;
	}
		
	s1.setOffset(offsets[s1.servoIndex]); s2.setOffset(offsets[s2.servoIndex]); s3.setOffset(offsets[s3.servoIndex]);
	s4.setOffset(offsets[s4.servoIndex]); s5.setOffset(offsets[s5.servoIndex]); s6.setOffset(offsets[s6.servoIndex]);
	s7.setOffset(offsets[s7.servoIndex]); s8.setOffset(offsets[s8.servoIndex]); s9.setOffset(offsets[s9.servoIndex]);
	s10.setOffset(offsets[s10.servoIndex]); s11.setOffset(offsets[s11.servoIndex]); s12.setOffset(offsets[s12.servoIndex]);
	s13.setOffset(offsets[s13.servoIndex]); s14.setOffset(offsets[s14.servoIndex]); s15.setOffset(offsets[s15.servoIndex]);
	s16.setOffset(offsets[s16.servoIndex]); s17.setOffset(offsets[s17.servoIndex]); s18.setOffset(offsets[s18.servoIndex]);
	s19.setOffset(offsets[s19.servoIndex]); s20.setOffset(offsets[s20.servoIndex]); s21.setOffset(offsets[s21.servoIndex]);
	s22.setOffset(offsets[s22.servoIndex]); s23.setOffset(offsets[s23.servoIndex]); s24.setOffset(offsets[s24.servoIndex]);
	s25.setOffset(offsets[s25.servoIndex]); s26.setOffset(offsets[s26.servoIndex]); s27.setOffset(offsets[s27.servoIndex]);
	s28.setOffset(offsets[s28.servoIndex]); s29.setOffset(offsets[s29.servoIndex]); s30.setOffset(offsets[s30.servoIndex]);
	s31.setOffset(offsets[s31.servoIndex]); s32.setOffset(offsets[s32.servoIndex]); s33.setOffset(offsets[s33.servoIndex]);
	s34.setOffset(offsets[s34.servoIndex]); s35.setOffset(offsets[s35.servoIndex]); s36.setOffset(offsets[s36.servoIndex]);
	s37.setOffset(offsets[s37.servoIndex]); s38.setOffset(offsets[s38.servoIndex]); s39.setOffset(offsets[s39.servoIndex]);
	s40.setOffset(offsets[s40.servoIndex]); s41.setOffset(offsets[s41.servoIndex]); s42.setOffset(offsets[s42.servoIndex]);
	s43.setOffset(offsets[s43.servoIndex]); s44.setOffset(offsets[s44.servoIndex]); s45.setOffset(offsets[s45.servoIndex]);
	s46.setOffset(offsets[s46.servoIndex]); s47.setOffset(offsets[s47.servoIndex]); s48.setOffset(offsets[s48.servoIndex]);
	s49.setOffset(offsets[s49.servoIndex]); s50.setOffset(offsets[s50.servoIndex]); s51.setOffset(offsets[s51.servoIndex]);
	s52.setOffset(offsets[s52.servoIndex]); s53.setOffset(offsets[s53.servoIndex]); s54.setOffset(offsets[s54.servoIndex]);
	s55.setOffset(offsets[s55.servoIndex]); s56.setOffset(offsets[s56.servoIndex]); s57.setOffset(offsets[s57.servoIndex]);
	s58.setOffset(offsets[s58.servoIndex]); s59.setOffset(offsets[s59.servoIndex]); s60.setOffset(offsets[s60.servoIndex]);
	s61.setOffset(offsets[s61.servoIndex]); s62.setOffset(offsets[s62.servoIndex]); s63.setOffset(offsets[s63.servoIndex]);
	s64.setOffset(offsets[s64.servoIndex]);
}

bool AIServoOffset::load(const char* dir)
{
	int i, channel;
	FILE *fp;
	char path[256] = {'\0'};
	char _line[256] = {'\0'};
	long long tmp[MAXSERVOS] = {0L}, value;
	
	if (dir == NULL) return false;    

	get_real_path(dir, path);

	if ((fp = fopen(path, "r")) == NULL) return false;

	while(fgets(_line, 256, fp))
	{
		for (i=0; _line[i] != '\0' && _line[i] != '\n'; i++)
			if (_line[i] == ' ') continue; else break;

		if (_line[i] == ';' || _line[i] == '\0' || _line[i] == '\n') continue;
		
		if (analysis(_line, &channel, &value) == true && channel < MAXSERVOS)
			tmp[channel] = value;
		else
		{
			fclose(fp);
			return false;
		}
	}
	
	fclose(fp);
	
	for (i=0; i<MAXSERVOS; i++) offsets[i] = (double)(((double)tmp[i])/1000000.0);

	return true;
}

bool AIServoOffset::load(const String &s)
{
	return load(s.c_str());
}

bool AIServoOffset::save(const char* dir)
{
    int i;
	FILE *fp;
	char path[256] = {'\0'};
	char _line[256] = {'\0'};
	
	if (dir == NULL) return false;

	get_real_path(dir, path);
	
	if ((fp = fopen(path, "a")) == NULL) return false;
	
	for (i=0; i<MAXSERVOS; i++)
	{
		if (offsets[i] == 0.0) continue;
		combine(_line, i, (long long)(offsets[i]*MEGA));
		fputs(_line, fp);
	}
	fputs("\n", fp);
	fclose(fp);
	return true;
}

bool AIServoOffset::save(const String &s)
{
	return save(s.c_str());
}

AIServoFrameRobotis::AIServoFrameRobotis() : AIServoFrame()
{
}

AIServoFrameRobotis::AIServoFrameRobotis(const char* dir, const char* sname, int step) : AIServoFrame()
{
	load(dir, sname, step);
}

bool AIServoFrameRobotis::loadMtnx(const char* dir, const char* sname, int step)
{
	//TODO
	return false;
}

bool AIServoFrameRobotis::loadMtn(const char* dir, const char* sname, int step)
{
	FILE *fp;
	char path[256] = {'\0'};
	char _line[256] = {'\0'};
	int motor_type[26] = {0};
	int tmp[26];
	for(int i = 0; i < 26; i++) tmp[i] = -1;
	
	get_real_path(dir, path);
	
	if((fp = fopen(path, "r")) == NULL) return false;
	
	bool res = false;
	while(fgets(_line, 256, fp))
	{
		if (strncmp(_line, "motor_type=", 11) == 0)
		{
			char* pch;
			pch = strtok(_line + 11, " ");
			for (int j = 0; j < 26; j++)
			{
				motor_type[j] = atoi(pch);
				pch = strtok(NULL, " ");
			}
		}
		if (strncmp(_line, "name=", 5) == 0)
		{
			if (memcmp(_line + 5, sname, sizeof(char)*strlen(sname)) == 0)
			{
				if(fgets(_line, 256, fp) == NULL) { fclose(fp); return false; }
				if(fgets(_line, 256, fp) == NULL) { fclose(fp); return false; }
				for (int i = 0; i <= step; i++)
				{
					if (fgets(_line, 256, fp) == NULL)
					{
						fclose(fp);
						return false;
					}
				}
				if (strncmp(_line, "step=", 5) == 0)
				{
					res = true;
					char* pch;
					pch = strtok(_line + 5, " ");
					for (int j = 0; j < 26; j++)
					{
						tmp[j] = atoi(pch);
						pch = strtok(NULL, " ");
					}
				}
			}
		}
	}
	
	fclose(fp);
	
	if(res == false)
		return res;
	
	for (int i = 0; i < 26; i++) 
	{
		double angle = -1;
		
		if(motor_type[i] == 1)
			angle = (double)((double)tmp[i]*0.06103515625);
		else if(motor_type[i] == 2)
			angle = (double)((double)tmp[i]*0.087890625);
		else
			angle = (double)((double)tmp[i]*0.29296875);
		
		if(angle > 360) angle = 360;
		if(angle < 0) angle = 0;
		positions[i] = angle;
	}

	return true;
}

static char* extension(char* dir)
{
	char* pch = strtok(dir, ".");
	char* res = pch;
	while(pch != NULL)
	{
		res = pch;
		pch = strtok(NULL, ".");
	}
	return res;
}

bool AIServoFrameRobotis::load(const char* dir, const char* sname, int step)
{
	if (dir == NULL) return false;
	char* _dir = (char*)malloc(sizeof(char)*(strlen(dir)+1));
	strcpy(_dir, dir);
	char* ext = extension(_dir);
	if(strncmp(ext, "mtn", 3) == 0)
	{
		if(strncmp(ext, "mtnx", 4) == 0)
		{
			free(_dir);
			return loadMtnx(dir, sname, step);
		}
		else
		{
			free(_dir);
			return loadMtn(dir, sname, step);
		}
	}
	free(_dir);
	return false;
}

bool AIServoFrameRobotis::load(const String &s, const String &s1, int step)
{
	return load(s.c_str(), s1.c_str(), step);
}

void aiEnableMixing (void) { enableMixing = true; }
void aiDisableMixing (void) { enableMixing = false; }

static double** AImallocMyDoubleMatrix(int m, int n)
{
	double **Array = (double**) malloc(m*sizeof(double *));
	for (int i = 0; i < m; i++)
		Array[i] = (double*) malloc(n*sizeof(double));
	return Array;
}

static void AIfreeMyDoubleMatrix(double** array, int rows)
{
	for(int i = 0; i < rows; i++)
		free(array[i]);
	free(array);
}

static void AIservoComputeCubicSpline (double** points, double** acc, unsigned long size)
{
	int totalPoints = 0;
	int totalLines = 0;

	double* x_Array;
	double* h_Array;
	double* L_Array;
	double* U_Array;
	double* z_Array;
	double* alpha_Array;
	int i, j;

	if (points == NULL) return;
	if (size == 1) {acc[0][0] = 0.0; acc[0][1] = 0.0; return;}
	if (size == 2) {acc[0][0] = 0.0; acc[0][1] = 0.0; acc[1][0] = 0.0; acc[1][1] = 0.0; return;}
	
	totalPoints = size;
	totalLines = totalPoints - 1L;
	
	h_Array = (double*) malloc(totalPoints*sizeof(double));
	x_Array = (double*) malloc(totalPoints*sizeof(double));
	alpha_Array = (double*) malloc(totalPoints*sizeof(double));
	L_Array =  (double*) malloc(totalPoints*sizeof(double));
	U_Array =  (double*) malloc(totalPoints*sizeof(double));
	z_Array =  (double*) malloc(totalPoints*sizeof(double));
	
	for(i=0; i<totalLines; i++)
		h_Array[i] = points[i+1][0] - points[i][0];

	for (i=1; i<totalLines; i++)
		alpha_Array[i] = (3*(points[i+1][1] - points[i][1])/h_Array[i]) - (3*(points[i][1] - points[i-1][1])/h_Array[i-1]);

	for (i=0; i<totalPoints; i++)
	{
		if (i == 0 || i == totalLines)
		{
			L_Array[i] = 1.0; x_Array[i] = 0.0; z_Array[i] = 0.0; U_Array[0] = 0.0;
		}
		else
		{
			L_Array[i] = 2*(points[i+1][0] - points[i-1][0]) - h_Array[i-1]*U_Array[i-1];
			U_Array[i] = h_Array[i]/L_Array[i];
			z_Array[i] = (alpha_Array[i] - h_Array[i-1]*z_Array[i-1])/L_Array[i];
		}
	}

	for (i=totalLines-1; i>=0; i--)
	{
		x_Array[i] = z_Array[i] - U_Array[i]*x_Array[i+1];
		acc[i][0] = 2*x_Array[i];
		acc[i][1] = 2*x_Array[i];
	}
	acc[totalLines][0] = 0; // the speed of last point is 0
	acc[totalLines][1] = 0;
	
	free(h_Array);
	free(x_Array);
	free(alpha_Array);
	free(L_Array);
	free(U_Array);
	free(z_Array); 
}

static void AIservoComputeConstrainedCubicSpline (double** points, double** acc, unsigned long size)
{
	int totalPoints = 0;
	int totalLines = 0;

	double* dx;
	double* dy;
	double* pointSpeed;
	double* myc;
	double* myd;
	int i, j;

	if (points == NULL) return;
	if (size == 1) {acc[0][0] = 0.0; acc[0][1] = 0.0; return;}
	if (size == 2) {acc[0][0] = 0.0; acc[0][1] = 0.0; acc[1][0] = 0.0; acc[1][1] = 0.0; return;}
	
	totalPoints = size;
	totalLines = totalPoints - 1L;
	
	dx = (double*) malloc(totalLines*sizeof(double));
	dy = (double*) malloc(totalLines*sizeof(double));
	pointSpeed = (double*) malloc(totalPoints*sizeof(double));
	myc = (double*) malloc(totalLines*sizeof(double));
	myd = (double*) malloc(totalLines*sizeof(double));
	
	for (i=0; i<totalLines; i++)
	{
		dx[i] = points[i+1][0] - points[i][0];
		if (dx[i] <= 20.0) dx[i] = 20.0;
		dy[i] = points[i+1][1] - points[i][1];
	}
	
	for (i=1; i<totalLines; i++)
	{
		long S = (long) (dy[i-1]*dy[i]);
		if (S > 0L)
			pointSpeed[i] = 2.0/((dx[i]/dy[i]) + (dx[i-1]/dy[i-1]));
		else
			pointSpeed[i] = 0.0;
	}
	
	pointSpeed[0] = ((3.0*dy[0])/(2.0*dx[0])) - (pointSpeed[1]/2.0);
	pointSpeed[totalLines] = ((3.0*dy[totalLines-1])/(2.0*dx[totalLines-1])) - (pointSpeed[totalLines-1]/2.0);
	
	for (i=1; i<=totalLines; i++)
	{
		double lacc = ((6.0*dy[i-1])/(dx[i-1]*dx[i-1])) - ((2.0*(pointSpeed[i] + 2.0*pointSpeed[i-1]))/dx[i-1]);
		double racc = ((2.0*(2.0*pointSpeed[i] + pointSpeed[i-1]))/dx[i-1]) - ((6.0*dy[i-1])/(dx[i-1]*dx[i-1]));
		
		myd[i-1] = (racc - lacc)/(6.0*dx[i-1]);
		myc[i-1] = ((points[i][0]*lacc) - (points[i-1][0]*racc))/(2.0*dx[i-1]);
		
		acc[i-1][1] = 2.0*myc[i-1] + 6.0*myd[i-1]*points[i-1][0];
		acc[i][0] = 2.0*myc[i-1] + 6.0*myd[i-1]*points[i][0];
	}

	acc[0][0] = 0.0;
	acc[i-1][1] = 0.0;

	free(dx);
	free(dy);
	free(pointSpeed);
	free(myc);
	free(myd);
}

static void AIservoComputeCatmullRomSpline (double** points, double** acc, unsigned long size)
{
	if (points == NULL || acc == NULL) return;
	
	for (int i = 0; i < size; i++)
	{			
		acc[i][0] = 0.0;
		acc[i][1] = 0.0;
	}
	
	if (size < 2) return;
	
	int totalPoints = size;
	int totalLines = totalPoints - 1L;
	
	for (int i = 0; i < totalLines; i++)
	{
		double period = points[i+1][0] - points[i][0];
		if (period < 20) period = 20;
		
		double a0, a1, a2, a3, b0, b1, b2, b3, c2, c3;
		
		if (i == 0 && i == totalLines - 1)
		{
			a0 = 2.0*CATMULL_ROM_TOU*(2*points[i][1]-points[i+1][1]);
			a1 = (CATMULL_ROM_TOU-3.0)*points[i][1];
			a2 = (3.0-2.0*CATMULL_ROM_TOU)*points[i+1][1];
			a3 = -1.0*CATMULL_ROM_TOU*(2*points[i+1][1]-points[i][1]);
			b0 = -1.0*CATMULL_ROM_TOU*(2*points[i][1]-points[i+1][1]);
			b1 = (2.0-CATMULL_ROM_TOU)*points[i][1];
			b2 = (CATMULL_ROM_TOU-2.0)*points[i+1][1];
			b3 = CATMULL_ROM_TOU*(2*points[i+1][1]-points[i][1]);
		}
		else if (i == 0)
		{
			a0 = 2.0*CATMULL_ROM_TOU*(2*points[i][1]-points[i+1][1]);
			a1 = (CATMULL_ROM_TOU-3.0)*points[i][1];
			a2 = (3.0-2.0*CATMULL_ROM_TOU)*points[i+1][1];
			a3 = -1.0*CATMULL_ROM_TOU*points[i+2][1];
			b0 = -1.0*CATMULL_ROM_TOU*(2*points[i][1]-points[i+1][1]);
			b1 = (2.0-CATMULL_ROM_TOU)*points[i][1];
			b2 = (CATMULL_ROM_TOU-2.0)*points[i+1][1];
			b3 = CATMULL_ROM_TOU*points[i+2][1];
		}
		else if (i == totalLines - 1)
		{
			a0 = 2.0*CATMULL_ROM_TOU*points[i-1][1];
			a1 = (CATMULL_ROM_TOU-3.0)*points[i][1];
			a2 = (3.0-2.0*CATMULL_ROM_TOU)*points[i+1][1];
			a3 = -1.0*CATMULL_ROM_TOU*(2*points[i+1][1]-points[i][1]);
			b0 = -1.0*CATMULL_ROM_TOU*points[i-1][1];
			b1 = (2.0-CATMULL_ROM_TOU)*points[i][1];
			b2 = (CATMULL_ROM_TOU-2.0)*points[i+1][1];
			b3 = CATMULL_ROM_TOU*(2*points[i+1][1]-points[i][1]);
		}
		else
		{
			a0 = 2.0*CATMULL_ROM_TOU*points[i-1][1];
			a1 = (CATMULL_ROM_TOU-3.0)*points[i][1];
			a2 = (3.0-2.0*CATMULL_ROM_TOU)*points[i+1][1];
			a3 = -1.0*CATMULL_ROM_TOU*points[i+2][1];
			b0 = -1.0*CATMULL_ROM_TOU*points[i-1][1];
			b1 = (2.0-CATMULL_ROM_TOU)*points[i][1];
			b2 = (CATMULL_ROM_TOU-2.0)*points[i+1][1];
			b3 = CATMULL_ROM_TOU*points[i+2][1];
		}
		
		c2 = (a0 + a1 + a2 + a3)/(period*period);
		c3 = (b0 + b1 + b2 + b3)/(period*period);
		acc[i][1] = 2.0*c2;
		acc[i+1][0] = 2.0*c2 + 6.0*c3;
	}
}

void aiservoBeginSplineMotion(int mode, AIServoFrame *Frames, unsigned long *frameTime, int numFrames)
{
	int i, j;
	double ***acc;
	double ***pp;
	unsigned int servoNum = 0;
	
	if ((mode != CONSTRAINED_CUBIC && mode != NATURAL_CUBIC && mode != CATMULL_ROM) || Frames == NULL || frameTime == NULL || numFrames <= 1)
		return;

	// Caculate moter number, suppose that user don't add motor quantity during playing frames
	for (i = 0; i < MAXSERVOS; i++)
		if (Frames[0].positions[i] >= 0.0) servoNum++;

	pp = (double***) malloc(servoNum*sizeof(double**)); // we have "servoNum" cubic splines
	for (i = 0; i < servoNum; i++)
		pp[i] = AImallocMyDoubleMatrix(numFrames, 2); // {{x0, y0}, {x1, y1}, {x2, y2}, ...}

	for (i = 0; i < servoNum; i++)
	{
		for (j = 0; j < numFrames; j++)
		{
			if (mode == NATURAL_CUBIC && frameTime[j] < 20L) frameTime[j] = 20L; // if frameTime < 20, frameTime = 20
			
			if (j == 0)
				pp[i][j][0] = frameTime[j]; // set X-axis value
			else
				pp[i][j][0] = (double) (frameTime[j] + pp[i][j-1][0]); // the frameTime need to be accumulated

			pp[i][j][1] = (double) Frames[j].positions[i]; // set Y-axis value
		}
	}

	acc = (double***) malloc(servoNum*sizeof(double**));
	for (i=0; i<servoNum; i++)
		acc[i] = AImallocMyDoubleMatrix(numFrames, 2);
	
	if (mode == NATURAL_CUBIC)
	{
		for (i = 0; i < servoNum; i++)
			AIservoComputeCubicSpline(pp[i], acc[i], numFrames); // Caculate the cubic splines for all servos
	}
	else if (mode == CONSTRAINED_CUBIC)
	{
		for (i = 0; i < servoNum; i++)
			AIservoComputeConstrainedCubicSpline(pp[i], acc[i], numFrames); // Caculate the cubic splines for all servos
	}
	else if (mode == CATMULL_ROM)
	{
		for (i = 0; i < servoNum; i++)
			AIservoComputeCatmullRomSpline(pp[i], acc[i], numFrames); // Caculate the cubic splines for all servos
	}

	for (i = 0; i < numFrames; i++)
	{
		for (j = 0; j < servoNum; j++)
		{
			Frames[i].accelerations[j][0] = acc[j][i][0];
			Frames[i].accelerations[j][1] = acc[j][i][1];
		}
	}
	
	for (i = 0; i < servoNum; i++)
	{
		AIfreeMyDoubleMatrix(acc[i], numFrames);
		AIfreeMyDoubleMatrix(pp[i], numFrames);
	}
	free(acc);
	free(pp);
}

void aiservoBeginSplineMotion(int mode, AIServoFrame **Frames, unsigned long *frameTime, int numFrames)
{
	int i, j;
	double ***acc;
	double ***pp;
	unsigned int servoNum = 0;
	
	if ((mode != CONSTRAINED_CUBIC && mode != NATURAL_CUBIC && mode != CATMULL_ROM) || Frames == NULL || frameTime == NULL || numFrames <= 1)
		return;
	for (i=0; i<numFrames; i++) if (Frames[i] == NULL) return;

	// Caculate moter number, suppose that user don't add motor quantity during playing frames
	for (i = 0; i < MAXSERVOS; i++)
		if (Frames[0]->positions[i] >= 0.0) servoNum++;

	pp = (double***) malloc(servoNum*sizeof(double**)); // we have "servoNum" cubic splines
	for (i = 0; i < servoNum; i++)
		pp[i] = AImallocMyDoubleMatrix(numFrames, 2); // {{x0, y0}, {x1, y1}, {x2, y2}, ...}

	for (i = 0; i < servoNum; i++)
	{
		for (j = 0; j < numFrames; j++)
		{
			if (mode == NATURAL_CUBIC && frameTime[j] < 20L) frameTime[j] = 20L; // if frameTime < 20, frameTime = 20
			
			if (j == 0)
				pp[i][j][0] = frameTime[j]; // set X-axis value
			else
				pp[i][j][0] = (double) (frameTime[j] + pp[i][j-1][0]); // the frameTime need to be accumulated

			pp[i][j][1] = (double) Frames[j]->positions[i]; // set Y-axis value
		}
	}

	acc = (double***) malloc(servoNum*sizeof(double**));
	for (i = 0; i < servoNum; i++)
		acc[i] = AImallocMyDoubleMatrix(numFrames, 2);

	if (mode == NATURAL_CUBIC)
	{
		for (i = 0; i < servoNum; i++)
			AIservoComputeCubicSpline(pp[i], acc[i], numFrames); // Caculate the cubic splines for all servos
	}
	else if (mode == CONSTRAINED_CUBIC)
	{
		for (i = 0; i < servoNum; i++)
			AIservoComputeConstrainedCubicSpline(pp[i], acc[i], numFrames); // Caculate the cubic splines for all servos
	}
	else if (mode == CATMULL_ROM)
	{
		for (i = 0; i < servoNum; i++)
			AIservoComputeCatmullRomSpline(pp[i], acc[i], numFrames); // Caculate the cubic splines for all servos
	}

	for (i = 0; i < numFrames; i++)
	{
		for (j = 0; j < servoNum; j++)
		{
			Frames[i]->accelerations[j][0] = acc[j][i][0];
			Frames[i]->accelerations[j][1] = acc[j][i][1];
		}
	}
	
	for (i = 0; i < servoNum; i++)
	{
		AIfreeMyDoubleMatrix(acc[i], numFrames);
		AIfreeMyDoubleMatrix(pp[i], numFrames);
	}
	free(acc);
	free(pp);
}

void aiservoEndSplineMotion()
{
}