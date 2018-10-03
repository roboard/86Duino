#ifndef AudioSample_H
#define AudioSample_H

#include "Arduino.h"
#include "Audio.h"

#if defined (__86DUINO_ZERO)
#error Audio library is only used on 86Duino ONE/EduCake/PLC!
#endif

struct WavFileHeader;

class AudioSample{
	
public:
	AudioSample();
	~AudioSample();
	bool load(char *wavFile);
	void play();
	void suspend();
	void resume();
	void stop();
	bool isPlaying();
	bool isSuspend();
	
private:
	friend class AudioClass;

	struct WavFileHeader *wavHeader;
	int dataLength;
	short *data;
	
	AudioSample *prev;
	AudioSample *next;
	
	int  playCursor;
	bool playing;
	bool suspending;
};

#endif