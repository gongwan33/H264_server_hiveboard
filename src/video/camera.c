/*
 * camera.c
 *  get pictures from the usb camera module PAP7501
 *  chenguangming@wxseuic.com
 */

#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>

#include "camera.h"
#include "v4l2.h"
#include "types.h"
#ifdef PRINTFPS
#include <signal.h>
#include <sys/time.h>
#endif

#define CAMERA_DEV "/dev/video1"

static pthread_t camera_ntid;
static pthread_t send_ntid;
static int camera_fd;
static int camera_stop = 0;
extern unsigned long video_frameinterval = 0;
sem_t start_camera;

sem_t new_pict;
static int pict_num = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static struct v4l2_buffer cur_buf;

#undef PRINTFPS

#ifdef PRINTFPS
#define TIME_DIFF(t1,t2) (((t1).tv_sec-(t2).tv_sec)*1000+((t1).tv_usec-(t2).tv_usec)/1000)
int send_cnt=0;
int pic_cnt=0;
int total_len=0;
int max_send_time=0;
int total_send_time=0;

void sigalrm_handler(int sig)
{
	printf("fps=%d/%d,Bps=%dK\n", send_cnt, pic_cnt, total_len/1024);
	printf("ave=%dms,max=%dms\n", total_send_time/send_cnt, max_send_time);
	pic_cnt = 0;
	send_cnt = 0;
	total_len = 0;
	total_send_time = 0;
}


int init_timer(void)
{
	struct itimerval itv;
	int ret;

	signal(SIGALRM, sigalrm_handler);
	itv.it_interval.tv_sec = 1;
	itv.it_interval.tv_usec = 0;
	itv.it_value.tv_sec = 1;
	itv.it_value.tv_usec = 0;
	
	ret = setitimer(ITIMER_REAL, &itv, NULL);
	if ( ret != 0){
		printf("Set timer error. %s \n", strerror(errno) );
		return -1;
	}
	printf("add timer\n");
}
#endif

static void *send_picture_thread(void *args)
{
	int fd = camera_fd, r;
	frame_t *fm;

	while(!camera_stop)
	{
#ifdef PRINTFPS
		struct timeval t1, t2;
		int send_time;
#endif
		fm = use_frame();
		if( fm == NULL )
		{
			printf("no frame, exit!\n");
			break;
		}
#ifdef PRINTFPS
		gettimeofday(&t1,NULL);
#endif
		if( fm->data ) send_picture(fm->data, fm->length,fm->time_stamp);
        if(video_frameinterval)
            usleep(video_frameinterval*1000);
		empty_frame( fm );
#ifdef PRINTFPS
		gettimeofday(&t2,NULL);
		send_time = TIME_DIFF(t2,t1);
		if( send_time > max_send_time ) max_send_time = send_time;
		total_send_time += send_time;
		send_cnt++;
		total_len += fm->length;
#endif
	}

	pthread_exit(0);
}

/*
 * the camera thread entry
 */
static void *camera_thread(void *args)
{
	int fd = camera_fd;
    struct timeval tv;
    fd_set fds;
    int r, i = 0, skip = 0;
    struct v4l2_buffer buf = {0};
	
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	sem_wait(&start_camera);

    start_capturing (fd);
//	sleep(1);
    r = pthread_create(&send_ntid, NULL, send_picture_thread, NULL);
    if(r < 0){
    	printf("create send picture thread failed\n");
    }
#ifdef PRINTFPS
	init_timer();
#endif    
    for (;;) {
        FD_ZERO (&fds);
        FD_SET (fd, &fds);

        /* Timeout. */
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        r = select (fd + 1, &fds, NULL, NULL, &tv);

        if (-1 == r) {
            if (EINTR == errno)
                continue;
            return NULL;
        }

        if (0 == r) {
            fprintf (stderr, "select timeout,camera can be dead,system will reboot!!!\n");
            system("reboot");
            return NULL;
        }


	r = get_buffer(fd, &buf);
	if( r != 0 )
	{
		get_frame(&buf);
#ifdef PRINTFPS
		pic_cnt++;
#endif
		r = put_buffer(fd, &buf);
		if(r<0)
			printf("skip buffer err = %d\n", r);
	}
	else
	{
		usleep(30000);
		//printf("get_buffer err = %d\n", r);
	}

#if 0       
		if(r = get_frame(fd, &pict)){
#ifdef PRINTFPS
            int t1 = times(NULL), t2, send_time;
#endif
            if(!skip) send_picture(pict.data, pict.length);

#ifdef PRINTFPS
            t2 = times(NULL);
            send_time = t2 - t1;
            if( send_time > max_send_time ) max_send_time = send_time;
            total_send_time += send_time;
            if( skip_cnt > 0 && ++i >= skip_cnt ) 
            {
                i = 0;
                skip = 1;
            }
            else
            {
                skip = 0;
            }
#endif
			r = put_frame(fd);
			if(r<0)
				printf("put_frame err = %d\n", r);
		}
		else
			printf("get_frame err = %d\n", r);
#ifdef PRINTFPS        
        pic_cnt++;
        total_len += pict.length;
#endif        
#endif
		if(camera_stop)
			pthread_exit(0);
    }
    
    stop_capturing (fd);

	return 0;
}

/*
 * init the camera module, called by the main thread
 */
int init_camera(void)
{
	int fd;
	int ret;


	
	fd = open_device (CAMERA_DEV);
    
    if(fd < 0){
    	printf("open camera failed\n");
    	goto fail1;
    }
    
    camera_fd = fd;

    init_device (camera_fd, 320, 240, 30);
    sem_init(&start_camera, 0, 0);
    ret = pthread_create(&camera_ntid, NULL, camera_thread, NULL);
    if(ret < 0){
    	printf("create camera thread failed\n");
    	goto fail1;
    }
    
    return 0;
	
fail2:
	uninit_device();
fail1:
	close_device(fd);
	return -1;
}

/*
 * close the camera, called by the other thread, don't call it in camera thread
 */
int close_camera(void)
{
	camera_stop = 1;
	pthread_join(camera_ntid, NULL);
	pthread_join(send_ntid, NULL);
	uninit_device();
	close_device(camera_fd);
    sem_destroy(&start_camera);
}
