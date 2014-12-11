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

    while(1)
    {
        printf(">>>>>>>>>Waiting For Client<<<<<<<<<<\n");
	    network();
    }

    close_camera();
	return 0;
}
