/*
 * prase-pkt.c
 *  prase packet
 *  jgfntu@163.com
 *
 */

#include "stdio.h"
#include "prase_pkt.h"
#include "wifi_debug.h"
#include "audio.h"
#include "network.h"
#include <string.h>

extern int start_measure();
extern void enable_t_rh_sent();
extern u32 audio_num;
unsigned long video_frameinterval;
int pre_sound = 0; //0:init ; 1:record ; 2:play
int cur_sound = 0; //0:init ; 1:record ; 2:play
int sound_state = 0;
int record_state = 0;
extern int verify_ok;
//int next_op=0;
/* prase opcode command text */
int prase_packet(int opcode, u8 *buf)
{
    printf("opcode = %d\n", opcode);
    if(verify_ok == 0){
        switch (opcode) {
        case 0:
            deal_login_req(buf);
            break;
        case 2:
            deal_verify_req(buf);
            break;
        }
    }
    else{
        switch (opcode) {
        case 4:
            send_video_start_resp();
            break;
        case 255:
            keep_connect();
            break;
        case 7:
            memcpy(&video_frameinterval,buf,sizeof(unsigned long));
            // printf("video_frameinterval is %ld \n",video_frameinterval);
            switch(video_frameinterval)
            {
            case  0:
                video_frameinterval = 0;
                break; //0ms，缺省值，每次开始播放时缺省为此值，即全速
            case 10:
                video_frameinterval = 50;
                break; //50ms，最大为20fps
            case 20:
                video_frameinterval = 60;
                break; //60ms
            case 30:
                video_frameinterval = 70;
                break; //70ms
            case 40:
                video_frameinterval = 80;
                break; //80ms
            case 50:
                video_frameinterval = 90;
                break; //90ms
            case 60:
                video_frameinterval = 100;
                break; //100ms，最大为10fps
            case 70:
                video_frameinterval = 110;
                break; //110ms
            case 80:
                video_frameinterval = 130;
                break; //130ms
            case 90:
                video_frameinterval = 150;
                break; //150ms
            case 100:
                video_frameinterval = 170;
                break;//170ms
            case 110:
                video_frameinterval = 200; 
                break;//200ms，最大为5fps
            case 120: 
                video_frameinterval = 250; 
                break;//250ms
            case 130: 
                video_frameinterval = 340; 
                break;//340ms
            case 140: 
                video_frameinterval = 500;
                break;//500ms，最大为2fps
            case 150:
                video_frameinterval = 1000;
                break;//1000ms，最大为1fps
            case 160:
                video_frameinterval = 1500;
                break;//1500ms
            case 170:
                video_frameinterval = 2000;
                break;//2000ms，最大为 1fp2s
            case 180:
                video_frameinterval = 2500;
                break;//2500ms
            case 190:
                video_frameinterval = 3000;
                break;//3000ms，最大为 1fp3s
            case 200:
                video_frameinterval = 3500;
                break;//3500ms
            case 210:
                video_frameinterval = 4000;
                break;//4000ms，最大为 1fp4s
            case 220: 
                video_frameinterval = 4500;
                break;//4500ms
            case 230:
                video_frameinterval = 5000; 
                break;//5000ms，最大为 1fp5s
            default:
                printf("<--Error:Unsupported video_frameinterval !! Set to Default 0 \n");
                video_frameinterval = 0;
            }
            printf("<--Video_Frameinterval is %ld \n",video_frameinterval);

            break;
        case 8:
#ifdef ENABLE_CAPTURE_AUDIO
            audio_num = 0;
            cur_sound = 1;
            if(record_state == 1){
                printf("   Recorder is Working .. Stop It!\n");
                stop_capture();
                disable_audio_send();
            }
            record_state = 1;
            start_capture();			/* !!!must be called first */
            enable_audio_send();
#else
            printf(">>>>If you want to use the capture audio, plese enable ENABLE_CAPTURE_AUDIO in the /wificar/src/include/audio.h\n");
#endif
            break;
        case 10:
#ifdef ENABLE_CAPTURE_AUDIO
            if(record_state == 0){
                printf("   Recorder has been stopped !\n");
                break;
            }
            record_state = 0;
            printf("   send audio packages num is %lu\n",audio_num);
            stop_capture();
            disable_audio_send();
#else
            printf("<<<<Disable the capture audio\n");
#endif
            break;
        default:
            //printf("opcode = %d\n", opcode);
            printf("   Unsupported opcode[ %d ] from user!\n",opcode);
            return -1;
        }

    }
	
	return 0;
}

/* prase AVdata command text */
int prase_AVpacket(int opcode, u8 *buf)
{
	int id = 0;

	if (opcode == 0) 
		if ((id = byteArrayToInt(buf, 0, 4)) == data_ID) {
			can_send = 1;
			printf("------data_ID = %d------\n", id); 
			wifi_dbg("video data can send (same data_ID)\n");
		};
	return 0;
}
