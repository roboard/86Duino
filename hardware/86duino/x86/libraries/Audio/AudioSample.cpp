#include "AudioSample.h"
#include "utility/WSS.h"

AudioSample::AudioSample()
{
	dataLength = AudioClass::dataLength / 2;
	prev = NULL;
	next = NULL;
	
	playing = false;
	suspending = false;

	playCursor = 0;
	bufCursor = 0;
	state = 0;
	out_buffer_size = 0;
	audioStream = -1;
	
	data = (short *)malloc(sizeof(short) * dataLength);
	packet=(AVPacket *)av_malloc(sizeof(AVPacket));
	pFrame=av_frame_alloc();
	au_convert_ctx = swr_alloc();
	pCodecCtx = NULL;
	pFormatCtx = NULL;
	out_buffer = NULL;
}

AudioSample::~AudioSample()
{
	stop();
	if (data != NULL)
		free(data);
	if (out_buffer != NULL)
		av_freep(out_buffer);
	if (au_convert_ctx != NULL) swr_free(&au_convert_ctx);
	if (pCodecCtx != NULL) avcodec_close(pCodecCtx);
	if (pFormatCtx != NULL) avformat_close_input(&pFormatCtx);
}

void AudioSample::play()
{
	if(playing)
		return;
	if(suspending)
	{
		resume();
		return;
	}
	
	playing = true;
	suspending = false;
	playCursor = 0;
	bufCursor = 0;
	state = 0;
	out_buffer_size = 0;


	// Add to list
	next = Audio.as_list;
	if(next != NULL)
		next->prev = this;
	Audio.as_list = this;
}

void AudioSample::pause()
{
	if (!playing || suspending) return;
	playing = false;
	suspending = true;
}

void AudioSample::resume()
{
	if (playing || !suspending) return;
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
	bufCursor = 0;
	state = 0;
	out_buffer_size = 0;

	if(audioStream != -1)
	{
		avio_seek(pFormatCtx->pb, 0, SEEK_SET);		
		avformat_seek_file(pFormatCtx, audioStream, 0, 0, pFormatCtx->streams[audioStream]->duration, 0);
	}
	
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

void AudioSample::setVolume(double v)
{
	if (v < 0 || v > 100) return;
	volume = v;
}

bool AudioSample::load(char *audioFile)
{
	int ret;
	
    if (audioFile == NULL) return false;
	const char* url = audioFile;
	
	if (playing) stop();
	
	if (pCodecCtx != NULL) avcodec_close(pCodecCtx);
	if (pFormatCtx != NULL) avformat_close_input(&pFormatCtx);
    
	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();
	//Open
	if(avformat_open_input(&pFormatCtx,url,NULL,NULL)!=0){
		printf("Couldn't open input stream.\n");
		return false;
	}
	// Retrieve stream information
	if(avformat_find_stream_info(pFormatCtx,NULL)<0){
		printf("Couldn't find stream information.\n");
		return false;
	}
	// Dump valid information onto standard error
	av_dump_format(pFormatCtx, 0, url, false);

	// Find the first audio stream
	for (int i=0; i < pFormatCtx->nb_streams; i++)
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO){
			audioStream=i;
			break;
		}

	if (audioStream==-1){
		printf("Didn't find a audio stream.\n");
		return false;
	}

	// Get a pointer to the codec context for the audio stream
	pCodecCtx=pFormatCtx->streams[audioStream]->codec;

	// Find the decoder for the audio stream
	AVCodec* pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec==NULL) {
		printf("Codec not found.\n");
		return false;
	}

	// Open codec
	if (avcodec_open2(pCodecCtx, pCodec,NULL)<0) {
		printf("Could not open codec.\n");
		return false;
	}


	//Out Audio Param
	out_channel_layout=AV_CH_LAYOUT_MONO;
	out_sample_fmt=AV_SAMPLE_FMT_S16;
	out_sample_rate=Audio.outSampleRate;
	out_channels=av_get_channel_layout_nb_channels(out_channel_layout);
	int64_t in_channel_layout=av_get_default_channel_layout(pCodecCtx->channels);
	
	av_init_packet(packet);
	//Swr
	au_convert_ctx = swr_alloc_set_opts(au_convert_ctx, out_channel_layout, out_sample_fmt, out_sample_rate, 
						in_channel_layout, pCodecCtx->sample_fmt , pCodecCtx->sample_rate, 0, NULL);

	swr_init(au_convert_ctx);
		
	avio_seek(pFormatCtx->pb, 0, SEEK_SET);
	avformat_seek_file(pFormatCtx, audioStream, 0, 0, pFormatCtx->streams[audioStream]->duration, 0);
	return true;
}

extern void __yields(void);
void AudioSample::update()
{
	int i, got_picture, ret, linesize;
	if (state < 0) return;
	do
	{
		if(state == 0)
		{
			if(av_read_frame(pFormatCtx, packet) < 0)
			{
				state = -1;
				return;
			}
			if(packet->stream_index == audioStream)
			{
				ret = avcodec_decode_audio4(pCodecCtx, pFrame, &got_picture, packet);
				if ( ret < 0 ) return;

				if ( got_picture > 0 ) {
					
					dst_nb_samples = av_rescale_rnd(swr_get_delay(au_convert_ctx, pCodecCtx->sample_rate) + pFrame->nb_samples, out_sample_rate, pCodecCtx->sample_rate, AV_ROUND_UP);
					if (out_buffer == NULL)
					{
						max_dst_nb_samples = dst_nb_samples * 2;
						ret = av_samples_alloc_array_and_samples((uint8_t ***)(&out_buffer), &linesize, out_channels,
																 max_dst_nb_samples, out_sample_fmt, 0);
					}
					else if (dst_nb_samples > max_dst_nb_samples)
					{
						max_dst_nb_samples = dst_nb_samples;
						av_freep(out_buffer);
						ret = av_samples_alloc((uint8_t **)out_buffer, &linesize, out_channels, dst_nb_samples, out_sample_fmt, 1);
					}
					if (ret < 0) return;
					ret = swr_convert(au_convert_ctx, (uint8_t **)out_buffer, dst_nb_samples,(const uint8_t **)pFrame->data , pFrame->nb_samples);
					if (ret < 0) return;
					out_buffer_size = av_samples_get_buffer_size(&linesize, out_channels, ret, out_sample_fmt, 1) / 2;
					
				}
			}
			av_free_packet(packet);
			state = 1;
		}
		if (state == 1 && dataLength - available() > out_buffer_size)
		{
			for (i = 0; i < out_buffer_size; i++) push((*out_buffer)[i]);
			state = 0;
		}
        // __yields();
	} while (dataLength - available() > out_buffer_size);

}

int AudioSample::available()
{
	return bufCursor < playCursor ? dataLength - playCursor + bufCursor : bufCursor - playCursor;
}

void AudioSample::push(short d)
{
	data[bufCursor++] = d;
	if (bufCursor >= dataLength) bufCursor = 0;
}

short AudioSample::pop()
{
	short *val = &data[playCursor++];
	if (playCursor >= dataLength) playCursor = 0;
	return *val;
}
