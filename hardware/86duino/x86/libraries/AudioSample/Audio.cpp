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
		Serial.print(w_get_error_message());
		while(1);
	}
	w_set_device_master_volume(1);
	
	latency_size = (DWORD)(w_get_nominal_sample_rate() * 0.025);
	
	w_lock_mixing_buffer(latency_size);
	w_mixing_zero();
	w_unlock_mixing_buffer();
}

void AudioClass::end()
{
	w_sound_device_exit();
}

void AudioClass::prepare(short int *buffer, int S, int _volume)
{
	volume = _volume;
	w_set_device_master_volume(_volume);
}

unsigned long int AudioClass::write(const unsigned long int *data, unsigned long int size)
{
	int samples;
	do
	{
		samples = w_get_buffer_size() - w_get_latency() - latency_size;
	}while(samples < size);
	w_lock_mixing_buffer(size);
	w_mixing_stereo((short int *)data, size, volume, volume);
	w_unlock_mixing_buffer();
}

#include <conio.h>
void AudioClass::update()
{
    static unsigned long t = micros();
	AudioSample *tmp = as_list;
	int samples = w_get_buffer_size() - w_get_latency() - latency_size;
	bool rid = false;
	
    if ((micros() - t) < 200) return;
    
    if(samples > 10240)
        samples = 10240;
    if(tmp == NULL)
    {
        for (int i=0; i<10240; i++) data[i] = 0;
    }
    
	//get first data
	while(tmp != NULL)
	{
		if(tmp->isPlaying())
		{
			for(int i = 0 ; i < samples; ++i)
			{
				if(tmp->playCursor >= tmp->dataLength)
				{
					rid = true;
					
					break;
				}
				data[i*2+1] = data[i*2] = tmp->data[tmp->playCursor];
				tmp->playCursor++;
			}
			
			if(rid)
			{
				AudioSample *a;
				a = tmp;
				tmp = tmp->next;
				a->stop();
			}
			else
				tmp = tmp->next;
			
			break;
		}		
		tmp = tmp->next;
	}
	
	
	for(;tmp != NULL;)
	{
		rid = false;
		if(tmp->isPlaying())
		{
			for(int i = 0 ; i < samples; ++i)
			{
				if(tmp->playCursor >= tmp->dataLength)
				{
					rid = true;
					break;
				}
				double a = tmp->data[tmp->playCursor] / 65536.0 + 0.5;
				double b = data[i*2] / 65536.0 + 0.5;
				data[i*2+1] = data[i*2] = (a < 0.5 && b < 0.5) ? ((2 * (a * b) - 0.5) * 65535)
												 : ((2 * (a + b) - (2 * a * b) - 1.5) * 65535);
				tmp->playCursor++;
			}
		}
		if(rid)
		{
			AudioSample *a;
			a = tmp;
			tmp = tmp->next;
			a->stop();
		}
		else
			tmp = tmp->next;
	}
    
	w_lock_mixing_buffer(samples);
	w_mixing_stereo((short int *)data, samples, volume, volume);
	w_unlock_mixing_buffer();
    
    t = micros();
    //memset(data, 0, samples);
}

AudioClass Audio = AudioClass();