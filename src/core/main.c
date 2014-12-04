/*
 * main.c
 *  the wifi car main program.
 *  chenguangming@wxseuic.com
 *  zhangjunwei166@163.com
 *  jgfntu@163.com
 *  2012-01-20 v1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "camera.h"
#include "audio.h"
#include "utils.h"
#include <pthread.h>
extern char str_camVS[];
extern char str_SSID[];
pthread_t led_flash_id;

int main()
{
    int ret;

    printf("\n===========H.264 monitoring===========\n");

#if 1
#ifdef ENABLE_VIDEO	
	if (init_camera() < 0)
    {
        close_camera();
		return 0;
	}

#endif
		
#endif
	init_send();

#if 0
	int i=0;
	picture_t *pic;
	
	while(1){
		if(get_picture(&pic) >= 0){
			//store_jpg(pic);
			put_picture(&pic);
			//printf("%d\n", pic->index);
			if(i++ > 8000)
				return 0;
		}
	}
#endif
    while(1)
    {
        printf(">>>>>>>>>Waiting For Client<<<<<<<<<<\n");
	    network();
    }

    close_camera();
	return 0;
}
