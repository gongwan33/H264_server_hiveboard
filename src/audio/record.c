/*
 * record.c
 *	Audio capture
 *	jgfntu@163.com
 */

#include <linux/soundcard.h>		/* for oss  */
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include "audio.h"
#include "types.h"
#include "protocol.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "buffer.h"
#include "adpcm.h"


static void *audio_capture( void *arg );
static void *audio_send( void *arg );

snd_pcm_t *handle = NULL;
snd_pcm_hw_params_t *params;

extern int cur_sound;
extern int pre_sound;

enum RecordState
{
	ST_RECORDING,
	ST_STOPPING,
	ST_STOPPED
};

enum RecordOp
{
	OP_NONE,
	OP_RECORD,
	OP_STOP
};

struct Recorder
{
	enum RecordState state;
	enum RecordOp next_op;
	pthread_t send_thread;
	pthread_t capture_thread;
	pthread_mutex_t lock;
};
struct AUDIO_CFG 
{
	int rate;
	int channels;
	int bit;
	int ispk;
};

enum CaptureBufferFlag
{
	START_CAPTURE,
	KEEP_CAPTURE,
	END_CAPTURE,
	STOP_CAPTURE
};

enum SendState
{
	SOCKET_INIT,
	SOCKET_SEND,
	SOCKET_STOPPED
};

enum RecorderState
{
	RECORDER_INIT,
	RECORDER_CAPTURE,
	RECORDER_STOPPED,
	RECORDER_RESET
};

//pthread_t capture_td, send_td;
struct Recorder g_recorder;

#define BUFFER_NUM 3
BufferQueue g_capture_buffer_queue;
#define CAPTURE_QUEUE (&g_capture_buffer_queue)

u8 g_raw_buffer[2][RECORD_MAX_READ_LEN];

extern int send_data_fd;
int start_record = 0;
sem_t capture_is_start;

void init_send()
{
	pthread_mutex_init(&g_recorder.lock,NULL);
	InitQueue(CAPTURE_QUEUE,"audio capture",BUFFER_NUM);
	if(pthread_create(&g_recorder.capture_thread, NULL, audio_capture, NULL) != 0) {
		perror("pthread_create");
		return ;
	}
	if(pthread_create(&g_recorder.send_thread, NULL, audio_send, NULL) != 0) {
		perror("pthread_create");
		return ;
	}
}

/* ---------------------------------------------------------- */
/*
 * set alsa record configuration
 */
int set_alsa_record_config(snd_pcm_t *handle, unsigned rate, u16 channels, int bit)
{
    int status, arg, dir, rc;
	unsigned int val;
	snd_pcm_uframes_t frames; 
    /*char volume = '5';*/
    if(pre_sound == cur_sound){
        return 1;
    }
    else{
        pre_sound = cur_sound;
    }
    /*if(volume_set(volume)<0)*/
        /*printf("   volume set failed !\n");*/

	/* Allocate a hardware parameters object. */  
	snd_pcm_hw_params_alloca(&params);  
	/* Fill it in with default values. */  
	snd_pcm_hw_params_any(handle, params);  
	/* Set the desired hardware parameters. */  
	/* Interleaved mode */  
	snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);  
	/* Signed 16-bit little-endian format */  
	snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);  
	/* Two channels (stereo) */  
	snd_pcm_hw_params_set_channels(handle, params, channels);  
	/* 44100 bits/second sampling rate (CD quality) */  
	val = rate;  
	snd_pcm_hw_params_set_rate_near(handle, params,  &val, &dir);  
	/* Set period size to 32 frames. */  
	frames = 32;  
	snd_pcm_hw_params_set_period_size_near(handle,  params, &frames, &dir);  
	/* Write the parameters to the driver */  
	rc = snd_pcm_hw_params(handle, params);  
	if (rc < 0) {  
		printf( "unable to set hw parameters: %s/n",  snd_strerror(rc));  
		return -1;  
	}  

    start_record = 1;
    printf("   rate is %d,channels is %d,bit is %d\n",rate,channels,bit);
    return 1;

}

/* ------------------------------------------------------------------------------ */
/*
 * Start recording data (alsa) ...
 */
static int record_alsa_data(u8 *buffer, u32 buf_size)
{
	int result, t1, t2;
	snd_pcm_uframes_t frames; 
	unsigned int val;
	snd_pcm_format_t fmt;
	int fmtInt = 2;
	snd_pcm_hw_params_get_channels(params, &val);
	snd_pcm_hw_params_get_format(params, &fmt);
	if(fmt == SND_PCM_FORMAT_S16_LE)
		fmtInt = 2;
	else
		fmtInt = 1;
	frames = buf_size/(val*fmtInt);  
	result = snd_pcm_readi(handle, buffer, frames); 
    return result;
}

/* ---------------------------------------------------------- */
/*
 * Start capture
 */
void start_capture(void)
{	
	StartRecorder();
	printf("<--Start capture audio ...\n");
//	enable_capture_audio();
}

/*
 * stop capture
 */
void stop_capture(void)
{
	StopRecorder();
	printf("<--Stop capture audio ...\n");
}


#define TIME_DIFF(t1,t2) (((t1).tv_sec-(t2).tv_sec)*1000+((t1).tv_usec-(t2).tv_usec)/1000)
void *audio_capture( void *arg )
{
	int state = RECORDER_INIT, i=0, length;
	struct AUDIO_CFG cfg = {RECORD_RATE,RECORD_CHANNELS,RECORD_BIT,0};
	Buffer *buffer;
	int rc;

	pthread_detach(pthread_self());
	
	do
	{
		switch( state )
		{
			case RECORDER_INIT:
				printf("   Recorder Init\n");
				OpenQueueIn(CAPTURE_QUEUE);
				rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_CAPTURE, 0);   
                if (rc < 0) {  
                    printf("unable to open pcm device: %s/n",  snd_strerror(rc));  
					break;  
				}  
                set_alsa_record_config(handle,cfg.rate,cfg.channels,cfg.bit);
				printf("   Recorder Start\n");
				state = RECORDER_CAPTURE;
				break;
			case RECORDER_RESET:
				printf("   Recorder Reset\n");
                pre_sound = 0;
                start_record = 0;
                do{
					rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_CAPTURE, 0);	
                    if(rc < 0)
                    {
                        printf("   Err(audio_capture): Open audio(alsas) [%d]device failed! Sleep 100ms And Try Again !\n",rc);
                        usleep(100000);
                    }
                }while(handle == NULL);
				if( set_alsa_record_config(handle,cfg.rate,cfg.channels,cfg.bit) < 0 )
				{
					printf("   Err(audio_capture): set oss play config failed!\n");
					snd_pcm_drain(handle);  
                    snd_pcm_close(handle);
					handle = NULL;
					break;
				}
				state = RECORDER_CAPTURE;
				break;
			case RECORDER_CAPTURE:
				//printf("capture\n");
				buffer = GetEmptyBuffer( CAPTURE_QUEUE );
				if( buffer == NULL )
				{
					state = RECORDER_STOPPED;
					break;
				}
                length =  record_alsa_data( buffer->data, RECORD_MAX_READ_LEN);
                if (rc == -EPIPE) {  
					/* EPIPE means overrun */  
					printf("overrun occurred/n");  
					snd_pcm_prepare(handle);  
					}
				else if (rc < 0) {  
					printf("error from read: %s/n",snd_strerror(rc));  
				}
				
				gettimeofday(&buffer->time_stamp,NULL);
                if( length <= 0 )
				{
					snd_pcm_drain(handle);  
                    snd_pcm_close(handle);
					printf("   Record Ret Error ! length is %d\n",length);
					state = RECORDER_RESET;
				}
				else
				{
					buffer->len = RECORD_MAX_READ_LEN;
					FillBuffer( CAPTURE_QUEUE );
				}
				break;
			case RECORDER_STOPPED:
				printf("   Recorder Stop\n");				
				//snd_pcm_drain(handle);	
				//snd_pcm_close(handle);
				//handle = NULL;
				state = RECORDER_INIT;
				break;
		}
	}while(1);

	pthread_exit(NULL);
}

//#define CAPTURE_PROFILE
void *audio_send( void *arg )
{
	int state = SOCKET_INIT;
	Buffer *buffer;
	int error_flag;
	adpcm_state_t adpcm_state = {0, 0};
#ifdef CAPTURE_PROFILE
	struct timeval t1, t2, t3;
	int total_cap_time = 0, cap_cnt = 0, cap_time, max_cap_time = 0;
	int total_send_time = 0, send_cnt = 0, send_time, max_send_time = 0;
#endif

	pthread_detach(pthread_self());
	do
	{
		switch( state )
		{
			case SOCKET_INIT:
				printf("   Sender Init\n");
				OpenQueueOut(CAPTURE_QUEUE);
				adpcm_state.valprev = 0;
				adpcm_state.index = 0;
				state = SOCKET_SEND;
				break;
			case SOCKET_SEND:
				//printf("send audio\n");
#ifdef CAPTURE_PROFILE
				gettimeofday(&t1,NULL);
#endif
				buffer = GetBuffer(CAPTURE_QUEUE);
				if( buffer == NULL )
				{
					state = SOCKET_STOPPED;
					break;
				}
#ifdef CAPTURE_PROFILE
				gettimeofday(&t2,NULL);
#endif
                memcpy((u8 *)&g_raw_buffer[0][RECORD_ADPCM_MAX_READ_LEN], (u8 *)(&adpcm_state), 3);
                adpcm_coder( (short *)buffer->data, g_raw_buffer[0],RECORD_MAX_READ_LEN,&adpcm_state);
				error_flag = send_audio_data(g_raw_buffer[0],RECORD_ADPCM_MAX_READ_LEN,buffer->time_stamp);
#ifdef CAPTURE_PROFILE
				gettimeofday(&t3,NULL);
				cap_time = TIME_DIFF(t2,t1);
				if( cap_time > max_cap_time ) max_cap_time = cap_time;
				total_cap_time += cap_time;
				cap_cnt++;
				if( cap_cnt == 10 )
				{
					printf("   ave_cap=%d,max_cap=%d\n",total_cap_time/cap_cnt,max_cap_time);
					total_cap_time = 0;
					cap_cnt = 0;
				}
				send_time = TIME_DIFF(t3,t2);
				if( send_time > max_send_time ) max_send_time = send_time;
				total_send_time += send_time;
				send_cnt++;
				if( send_cnt == 10 )
				{
					printf("   ave_send=%d,max_send=%d\n",total_send_time/send_cnt,max_send_time);
					total_send_time = 0;
					send_cnt = 0;
				}
#endif
				if (error_flag < 0)
				{
					state = SOCKET_STOPPED;
				}
				EmptyBuffer(CAPTURE_QUEUE);
				break;
			case SOCKET_STOPPED:
				printf("   Send Stop\n");
				state = SOCKET_INIT;
				break;
		}
	}while(1);

	pthread_exit(NULL);
}

void StartRecorder()
{
	printf("<--Start Recorder\n");
	EnableBufferQueue(CAPTURE_QUEUE);
	return;
}

void StopRecorder()
{
	printf("<--Stop Recorder\n");
	DisableBufferQueue(CAPTURE_QUEUE);
	return;
}
/* ---------------------------------------------------------- */
