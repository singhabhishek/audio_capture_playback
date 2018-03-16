#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <alsa/asoundlib.h>
#include <speex/speex_echo.h>
      
#define BUF_BYTES 2048

typedef struct audio_config_params
{
	uint32_t rate;
	uint32_t channels;
    	snd_pcm_format_t format;
	char *audio_device;
}audio_config_params;

int setup_capture(snd_pcm_t **capture_handle, audio_config_params *config_params)
{
	int ret;
    	if ((ret = snd_pcm_open(capture_handle, config_params->audio_device, SND_PCM_STREAM_CAPTURE, 0)) < 0)
    	{
        	printf("Error: cannot open audio device %s (%s)\n", config_params->audio_device, snd_strerror (ret));
		return ret;
    	}
    
    	if ((ret = snd_pcm_set_params(*capture_handle, config_params->format, SND_PCM_ACCESS_RW_INTERLEAVED, config_params->channels, config_params->rate, 1, 1000000)) < 0)
    	{
        	printf("Error: Capture open error: %s\n", snd_strerror(ret));
		snd_pcm_close(*capture_handle);
		*capture_handle = NULL;
		return ret;
    	}

	return 0;
}

int setup_playback(snd_pcm_t **playback_handle, audio_config_params *config_params)
{
	int ret;
    	if ((ret = snd_pcm_open(playback_handle, config_params->audio_device, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    	{
        	printf("Error: cannot open audio device %s (%s)\n", config_params->audio_device, snd_strerror (ret));
		return ret;
    	}
    
    	if ((ret = snd_pcm_set_params(*playback_handle, config_params->format,  SND_PCM_ACCESS_RW_INTERLEAVED, config_params->channels, config_params->rate, 1, 1000000)) < 0)
    	{
        	printf("Error: Playback open error: %s\n", snd_strerror(ret)); 
		snd_pcm_close(*playback_handle);
		*playback_handle = NULL;
		return ret;
    	}

	return 0;
}

void close_capture(snd_pcm_t *capture_handle)
{
    	snd_pcm_drop(capture_handle);
    	snd_pcm_hw_free(capture_handle);
    	snd_pcm_close (capture_handle);
}

void close_playback(snd_pcm_t *playback_handle)
{
    	snd_pcm_drop(playback_handle);
    	snd_pcm_hw_free(playback_handle);
    	snd_pcm_close (playback_handle);
}

int capture_playback_audio(snd_pcm_t *capture_handle, snd_pcm_t *playback_handle, audio_config_params *config_params)
{
    	int err;
	short echo_frame[BUF_BYTES] = {0};
	short input_frame[BUF_BYTES] = {0};
	short output_frame[BUF_BYTES] = {0};
	uint32_t buf_frames = (BUF_BYTES / (config_params->channels * 2));
	SpeexEchoState *echo_state = speex_echo_state_init(buf_frames, buf_frames);
    	
	while(1)
    	{
		err = snd_pcm_writei (playback_handle, echo_frame, buf_frames);
        	if (err != buf_frames)
		{
			fprintf (stderr, "write to audio interface failed (%s)\n", snd_strerror (err));
			snd_pcm_prepare(playback_handle);
		}

		err = snd_pcm_readi (capture_handle, input_frame, buf_frames);
        	if (err != buf_frames)
		{
			fprintf (stderr, "read from audio interface failed (%s)\n", snd_strerror (err));
			snd_pcm_prepare(capture_handle);
		}
	
		speex_echo_cancellation(echo_state, input_frame, echo_frame, output_frame);
		memcpy(echo_frame, output_frame, (buf_frames)*sizeof(short));
    	}
	
	return 0;
}

int main (int argc, char *argv[])
{
    	int err;
    
    	char* device = "default";
    	if (argc > 1) device = argv[1];
	
	// Playback and capture handles
    	snd_pcm_t *playback_handle;
    	snd_pcm_t *capture_handle;

	// For debugging
    	snd_output_t *output = NULL;
    	snd_output_stdio_attach(&output, stdout, 0);

	// Allocate memory for config structure and set config parameters
	audio_config_params *config_params = (audio_config_params*)malloc(sizeof(audio_config_params));
	if(config_params == NULL)
	{
		printf("Error in allocating memory for config params\n");
		return -1;
	}
    
	config_params->rate = 44100;
	config_params->channels = 1;
	config_params->format = SND_PCM_FORMAT_S16_LE;
	config_params->audio_device = device;

	// Setup capture device for MIC
   	if(setup_capture(&capture_handle, config_params) < 0)
	{
   		close_playback(playback_handle);
		exit(1);
	}

	// Setup playback device for Speaker
	if(setup_playback(&playback_handle, config_params) < 0)
	{
   		close_playback(playback_handle);
		close_capture(capture_handle);
		exit(1);
	}
    	
	// For debugging
	snd_pcm_dump(playback_handle, output);

	// Capture data from MIC and playback on Speaker
	if(capture_playback_audio(capture_handle, playback_handle, config_params) < 0) exit(1); 

	// Close capture device handle
   	close_playback(playback_handle);

	// Close playback device handle
	close_capture(capture_handle);
 
    	return 0;
}
