/*
  Audio.cpp - DM&P Vortex86 Audio library
  Copyright (c) 2013 Vic Cheng <vic@dmp.com.tw>. All right reserved.

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

#include "Audio.h"
#include "utility/WSS.h"
#include "irq.h"
#include <dpmi.h>
#include "AudioSample.h"


void AudioClass::begin(unsigned long int sampleRate, unsigned long int msPreBuffer)
{
	//Start HD audio
	if(w_sound_device_init(28, sampleRate) == false)
	{
		printf("%s\n", w_get_error_message());
		// while(1);
		return;
	}
	w_set_device_master_volume(1);
	
	latency_size = (DWORD)(w_get_nominal_sample_rate() * 0.025);
	
	w_lock_mixing_buffer(latency_size);
	w_mixing_zero();
	w_unlock_mixing_buffer();
	
	
	if (sampleRate == 44100 || sampleRate == 32000 || sampleRate == 24000 ||
		sampleRate == 22050 || sampleRate == 16000 || sampleRate == 11025 ||
		sampleRate == 9600 || sampleRate == 8000 || sampleRate == 6000)
		outSampleRate = sampleRate;
	else if(sampleRate < 0)
		outSampleRate = 22050;
	else
		outSampleRate = 48000;
	as_list = NULL;
}

void AudioClass::end()
{
	if (!w_get_device_name()) return;
	w_sound_device_exit();
}

void AudioClass::prepare(short int *buffer, int S, int _volume)
{
	if (!w_get_device_name()) return;
	volume = _volume;
	w_set_device_master_volume(_volume);
}

unsigned long int AudioClass::write(const unsigned long int *data, unsigned long int size)
{
	if (!w_get_device_name()) return 0L;
	int samples;
	do
	{
		samples = w_get_buffer_size() - w_get_latency() - latency_size;
	}while(samples < size);
	w_lock_mixing_buffer(size);
	w_mixing_stereo((short int *)data, size, volume, volume);
	w_unlock_mixing_buffer();
    
    return 0L;
}

void AudioClass::update()
{
	if (!w_get_device_name()) return;
    static unsigned long t = micros();
    if ((micros() - t) < 200) return;
	
	AudioSample *tmp, *tmp2 = NULL;
	int i;
	int samples = w_get_buffer_size() - w_get_latency() - latency_size;
	bool inuse = false;
    double a, b;
	
	for(tmp = as_list; tmp != NULL; tmp = tmp->next)
	{
		if(tmp->isPlaying())
		{
			tmp->update();
			if(tmp->available() < samples)
				samples = tmp->available();
		}
	}

	for (tmp = as_list; tmp != NULL;)
	{
		if (tmp->isPlaying())
		{
			if (tmp->state < 0 && tmp->available() == 0)
			{
				tmp2 = tmp;
				tmp = tmp->next;
				tmp2->stop();
			}
			else
			{
				for (i = 0 ; i < samples; i++)
				{
					a = tmp->pop() * tmp->volume;
					if (a > 32767) a = 32767;
					if (a < -32768) a = -32768;
					if(inuse)
					{
						a = a / 65536.0 + 0.5;
						b = data[i*2] / 65536.0 + 0.5;
						data[i*2+1] = data[i*2] = (a < 0.5 && b < 0.5) ? ((2 * (a * b) - 0.5) * 65535)
														 : ((2 * (a + b) - (2 * a * b) - 1.5) * 65535);
					}
					else
						data[i*2+1] = data[i*2] = a;
				}
				tmp = tmp->next;
				inuse = true;
			}
		}
		else
			tmp = tmp->next;
	}

    if (!inuse)
		memset(data, 0, dataLength*sizeof(short));
	w_lock_mixing_buffer(samples);
	w_mixing_stereo(data, samples, volume, volume);
	w_unlock_mixing_buffer();
	
    t = micros();
}

void AudioClass::stop()
{
	if (!w_get_device_name()) return;
	for(AudioSample* tmp = as_list; tmp != NULL; tmp = tmp->next)
		if(tmp->isPlaying()) tmp->stop();
	memset(data, 0, dataLength*sizeof(short));
	w_lock_mixing_buffer(dataLength*sizeof(short));
	w_mixing_stereo(data, dataLength*sizeof(short), volume, volume);
	w_unlock_mixing_buffer();
}
AudioClass Audio = AudioClass();