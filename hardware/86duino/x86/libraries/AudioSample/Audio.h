/*
 * Copyright (c) 2012 by Cristian Maglie <c.maglie@bug.st>
 * Audio library for Arduino Due.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef AUDIO_H
#define AUDIO_H

#include "Arduino.h"
#include "Print.h"

#if defined (__86DUINO_ZERO)
#error Audio library is only used on 86Duino ONE/EduCake/PLC!
#endif

class AudioSample;
class AudioClass : public Print {
	friend class AudioSample;
	
public:
	AudioClass(){ };
	void prepare(short  int *buffer, int S, int volume);
	void begin(unsigned long int sampleRate, unsigned long int  msPreBuffer);
	void end();
	void update();

	virtual unsigned long int write(unsigned char c)                         { /* not implemented */ };
	virtual unsigned long int write(const unsigned char *data, unsigned long int size)  { return write((const unsigned long int*)(data), size/4) * 4; };
	virtual unsigned long int write(const short int *data, unsigned long int size) { return write((const unsigned long int*)(data), size/2) * 2; };
	
	virtual unsigned long int write(const unsigned long int* data, unsigned long int size);
	
private:
	unsigned long int latency_size;
	unsigned long int volume;
	short data[2*1024*1024];
	AudioSample *as_list;
};

extern AudioClass Audio;

#endif