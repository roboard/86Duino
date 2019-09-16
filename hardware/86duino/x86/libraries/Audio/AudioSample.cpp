#include "AudioSample.h"

#define RIFF_CHUNK_SIZE 12
#define FMT_HEADER_SIZE 8
#define FMT_DATA_SIZE   16
#define DATA_CHUNK_SIZE 8

#define CHANNELS    1
#define SAMPLE_RATE 22050
#define BPS         16

#define MAX_DATA_SIZE 1*1024*1024

struct WavFileHeader
{
	DWORD riff;
	DWORD wav_size;
	DWORD wave_header;
	DWORD fmt;
	DWORD fmt_size;
	WORD  audio_format;
	WORD  channels;
	DWORD sample_rate;
	DWORD byte_rate;
	WORD  align;
	WORD  bps;
	DWORD data;
	DWORD data_size;
};

// test functions

static void printWavInfo(struct WavFileHeader* info)
{
	char *temp;
	
	printf("riff         = ");
	temp = (char *)(&(info->riff));
	for(int i = 0; i < sizeof(DWORD); i++)
	{
		printf("%c", *(temp+i));
	}
	
	printf("\nwav_size     = %d", info->wav_size);
	
	printf("\nwave_header  = ");
	temp = (char *)(&(info->wave_header));
	for(int i = 0; i < sizeof(DWORD); i++)
	{
		printf("%c", *(temp+i));
	}
	
	printf("\nfmt          = ");
	temp = (char *)(&(info->fmt));
	for(int i = 0; i < sizeof(DWORD); i++)
	{
		printf("%c", *(temp+i));
	}
	
	printf("\nfmt_size     = %d", info->fmt_size);
	
	printf("\naudio_format = %d", info->audio_format);
	
	printf("\nchannels     = %d", info->channels);
	
	printf("\nsample_rate  = %d", info->sample_rate);
	
	printf("\nbyte_rate    = %d", info->byte_rate);
	
	printf("\nalign        = %d", info->align);
	
	printf("\nbps          = %d", info->bps);
	
	printf("\ndata         = ");
	temp = (char *)(&(info->data));
	for(int i = 0; i < sizeof(DWORD); i++)
	{
		printf("%c", *(temp+i));
	}
	
	printf("\ndata_size    = %d", info->data_size);
}

// end of test functions

AudioSample::AudioSample()
{
	wavHeader = NULL;
	dataLength = 0;
	data = NULL;
	
	prev = NULL;
	next = NULL;
	
	playCursor = 0;
	playing = false;
	suspending = false;
}

AudioSample::~AudioSample()
{
	stop();
	if(wavHeader != NULL)
		free(wavHeader);
	if(data != NULL)
		free(data);
}

bool AudioSample::load(char *wavFile)
{
	FILE *fp = NULL;
	int result, buffer_size;
	
	wavHeader = (struct WavFileHeader *)malloc(sizeof(struct WavFileHeader));
	if(wavHeader == NULL)
	{
		printf("malloc WavFileHeader failed\n");
		return false;
	}
		
	fp = fopen(wavFile, "rb");
	if(fp == NULL)
	{
		printf("Cannot find file \"%s\"\n", wavFile);
		return false;
	}

	// Read RIFF chunk
	result = fread((void *)wavHeader, 1, RIFF_CHUNK_SIZE, fp);
	if(result != RIFF_CHUNK_SIZE)
	{
		printf("\"%s\" is not valid wav file\n", wavFile);
		
		return false;
	}
	
	//Read fmt header+data 
	result = fread((void *)&(wavHeader->fmt), 1, FMT_HEADER_SIZE + FMT_DATA_SIZE, fp);
	if(result != (FMT_HEADER_SIZE + FMT_DATA_SIZE))
	{
		printf("\"%s\" is not valid wav file\n", wavFile);
		
		return false;
	}
	
	if(wavHeader->fmt_size > 16)
	{
		// get rid of garbage
		for(int i = FMT_DATA_SIZE; i < wavHeader->fmt_size; i++)
			fgetc(fp);
	}
	
	//Read data chunk
	result = fread((void *)&(wavHeader->data), 1, DATA_CHUNK_SIZE, fp);
	if(result != DATA_CHUNK_SIZE)
	{
		printf("\"%s\" is not valid wav file\n", wavFile);
		
		return false;
	}
	
	if(wavHeader->channels != CHANNELS || wavHeader->sample_rate != SAMPLE_RATE || wavHeader->bps != BPS)
	{
		// vic TODO resampling
		return false;
	}
	else
	{
		dataLength = (wavHeader->data_size > MAX_DATA_SIZE)? MAX_DATA_SIZE:wavHeader->data_size-4;
		dataLength /= 2;
		
		data = (short *)malloc(sizeof(short) * dataLength);
		if(data == NULL)
		{
			printf("Memory space not enough.\n");
			return false;
		}

		result = fread(data, 2, dataLength, fp);
	}
	
	fclose(fp);
	
	return true;
}

void AudioSample::play()
{
	if(playing)
		return;
	
	playing = true;
	suspending = false;
	playCursor = 0;
	
	// Add to list
	next = Audio.as_list;
	if(next != NULL)
		next->prev = this;
	Audio.as_list = this;
}

void AudioSample::suspend()
{
	playing = false;
	suspending = true;
}

void AudioSample::resume()
{
	playing = true;
	suspending = false;
}

void AudioSample::stop()
{
	if(!playing)
		return;
	
	playing = false;
	suspending = false;
	playCursor = 0;
	
	// delete from list
	if(next != NULL) //not tail
	{
		if(Audio.as_list == this) //head
		{
			Audio.as_list = next;
			next->prev = NULL;
		}
		else // not head
		{
			next->prev = prev;
			prev->next = next;
		}
	}
	else if(prev != NULL) // not head but tail
	{
		prev->next = NULL;
	}
	else if(Audio.as_list == this) //head & tail
	{
		Audio.as_list = NULL;
	}
	else //not in list
		;
		
	prev = NULL;
	next = NULL;
}

bool AudioSample::isPlaying()
{
	return playing; 
}

bool AudioSample::isSuspend()
{
	return suspending;
}