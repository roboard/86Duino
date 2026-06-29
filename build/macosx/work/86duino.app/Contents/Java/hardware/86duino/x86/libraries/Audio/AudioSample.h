#ifndef AudioSample_H
#define AudioSample_H

#include "Arduino.h"
#include "Audio.h"
#include "utility/ffmpeg.h"

#if defined (__86DUINO_ZERO)
#error Audio library is only used on 86Duino ONE/EduCake/PLC!
#endif

struct WavFileHeader;

class AudioSample{
	
public:
	AudioSample();
	~AudioSample();
	bool load(char *audioFile);
	void play();
	void pause();
	void resume();
	void stop();
	bool isPlaying();
	bool isSuspend();
	void setVolume(double v);
	
private:
	friend class AudioClass;

	int state;
	int dataLength;
	short *data;
	
	AudioSample *prev;
	AudioSample *next;
	
	int  playCursor;
	int  bufCursor;
	bool playing;
	bool suspending;
	
    AVFormatContext	*pFormatCtx;
	AVCodecContext	*pCodecCtx;
	int				audioStream;
	AVPacket		*packet;
	AVFrame			*pFrame;
	SwrContext *au_convert_ctx;
	
	AVSampleFormat out_sample_fmt;
	uint64_t out_channel_layout;
	int out_sample_rate;
	int out_channels;
	int out_buffer_size;
	
	short			**out_buffer;
    int dst_nb_samples, max_dst_nb_samples;
	double volume = 1.0;
	
	void update();
	int available();
	void push(short data);
	short pop();
};

#endif