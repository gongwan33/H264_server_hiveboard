#ifndef _AUDIO_H
#define _AUDIO_H

#include <pthread.h>
#include "types.h"
#include  <alsa/asoundlib.h> 


/* --------------------------------------------------------- */

//#define WIFI_CAR_V3			/* TODO (FIX ME) use wificar(apk) version 3 */

//#define USE_FMT_PCM				/* wav pcm format or adpcm format choise */
#define USE_FMT_ADPCM

/* for alsa configuration */
#define AUDIO_DEV 		"plughw:0,0"
#define PLAYBACK_DEV 	"plughw:0,0"
#define RECORD_RATE 		44100
//#define RECORD_RATE 		11025
#define AUDIO_RATE 		44100
#define AUDIO_BIT		16
#define RECORD_BIT      16
#define AUDIO_CHANNELS 	1
#define RECORD_CHANNELS 1

#define ENABLE_VIDEO
#define ENABLE_AUDIO					/* enable audio */
#define ENABLE_CAPTURE_AUDIO				/* enable usb audio, must be enable ENABLE_AUDIO together */

/* TODO (FIX ME) : next tow macros can not be defined as 1 at the same time */
#define ENABLE_RUNTIME_PLAYBACK 0		/* 1 : means playback music data from cellphone (runtime); 0 : means disable */
#define ENABLE_TALK_PLAYBACK 	1		/* 1 : means playback record data from cellphone; 0 : means disable */

/* max read buf */
#define RECORD_MAX_READ_LEN     2048				/* TODO (FIX ME) 4kb */
#define RECORD_ADPCM_MAX_READ_LEN 	512		/* TODO (FIX ME) 1kb */
#define MAX_READ_LEN 	1024*4			/* TODO (FIX ME) 4kb */
#define ADPCM_MAX_READ_LEN 	1024		/* TODO (FIX ME) 1kb */
#define WAV_HEADER_SIZE 44

/* TODO (FIX ME) for runtime playback */
#define BUFSIZE 8192
#define RUNTIME_RECV_LEN (BUFSIZE * 10)
#define RUNTIME_PLAY_LEN (BUFSIZE * 11)


/* --------------------------------------------------------- */

typedef struct {
	int index;
	int locked;
	void *audio_pkt;
}audio_t;

/* --------------------------------------------------------- */

/* do nothing, just for project compiling */
int init_audio(void);

void start_capture(void);
void stop_caputure(void);

void *audio_capture_thread(void *args);
void *audio_playback_thread(void *args);
int volume_set(char );
int speak_power(char *state);
int speak_power_off();
int speak_power_on();

/* --------------------------------------------------------- */

/* do nothing, just for project compiling */
void init_receive(void);
void StartPlayer(void);
void StopPlayer(void);
void EndPlayer(void);

void init_send(void);
void StartRecorder(void);
void StopRecorder(void);
void EndRecorder(void);

/* the third thread to process file receiving from cellphone and runtime playbacking */
void *deal_FEdata_request(void *arg);

/* parse the wav file header for alsa configuration */	
int parse_wav_header(u8 *buf, u32 fd);

/* playback buffer data (double buffer, be called from ./receive.c) */
int playback_buf(int fd,u8 *play_buf, int len);

/* set oss configuration */
int set_alsa_record_config(snd_pcm_t *handle, unsigned rate, u16 channels, int bit);
int set_i2s_rate(unsigned int rate);
extern unsigned un_OSS_RATE[];

/* --------------------------------------------------------- */

#endif
