/*
 * v4l2.c
 *  V4L2 video capture. 
 *		flow mothods: open -> init -> start -> ... -> stop -> uninit -> close
 *  chenguangming@wxseuic.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>
#include <semaphore.h>

#include <types.h>
#include <camera.h>
#include "v4l2.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))
#define V4L2_PIX_FMT_H264     v4l2_fourcc('H', '2', '6', '4') /* H.264 Annex-B NAL Units */

struct buffer {
    void *	start;
    size_t	length;
};

struct buffer *         buffers		= NULL;
static unsigned int     n_buffers	= 0;
//static struct v4l2_buffer v4l_buf;

enum FrameFlag
{
	FRAME_EMPTY = 0,
	FRAME_FILLING,
	FRAME_READY,
	FRAME_IN_USE
};

#define MAX_FRAME_LEN	(100*1024)
#define FRAME_NUM 40
static frame_t frame_list[FRAME_NUM];
static frame_t *ready_list_head, *ready_list_tail;
static frame_t *empty_list_head;
static sem_t pict_ready;
static pthread_mutex_t frame_list_lock;


static void errno_exit (const char *s)
{
    fprintf (stderr, "%s error %d, %s\n",s, errno, strerror (errno));
    exit (EXIT_FAILURE);
}

/*
 * store the raw image data in file
 */
void store_raw(unsigned char *buf, int len)
{
	FILE *file;
	char name[16];

	static int nn=0;
	sprintf(name, "/nfs/imgs/%d.yuv", nn++);
	file = fopen(name, "wb");
	if(file == NULL){
		printf("fail create file %s\n", name);
		return;
	}

	fwrite(buf, len, 1, file);
	fclose(file);
}

void dump_frame_list()
{
	int i;
	for( i = 0; i < FRAME_NUM; i++ )
		printf("#f%d: flag=%d\n", i, frame_list[i].flag);
	printf("#ready head = f%d\n", ready_list_head - frame_list);
	printf("#ready tail = f%d\n", ready_list_tail - frame_list);
	printf("#empty head = f%d\n", empty_list_head - frame_list);
	return;
}

int get_frame(struct v4l2_buffer *v4l_buf)
{
	frame_t *fm;
	//printf("get_frame\n");
	pthread_mutex_lock(&frame_list_lock);
	if( empty_list_head == NULL )
	{
		empty_list_head = ready_list_head;
		ready_list_head = ready_list_head->next;
		empty_list_head->next = NULL;
		ready_list_tail->next = empty_list_head;
		ready_list_tail = empty_list_head;
		/*printf("@catchup\n");*/
		sem_trywait(&pict_ready);
	}
	empty_list_head->flag = FRAME_FILLING;
	fm = empty_list_head;
	empty_list_head = empty_list_head->next;
	pthread_mutex_unlock(&frame_list_lock);
	fm->length = v4l_buf->bytesused;
    	if(fm->data) memcpy(fm->data, buffers[v4l_buf->index].start, fm->length);
//	if(fm->length > 60*1024)
//		printf("buffer overflow fm->length = %d\n",fm->length);
	pthread_mutex_lock(&frame_list_lock);
    gettimeofday(&fm->time_stamp,NULL);
	fm->flag = FRAME_READY;
	//printf("@data ready\n");
	sem_post(&pict_ready);
	pthread_mutex_unlock(&frame_list_lock);
	//printf("get_frame %x ok\n", fm-frame_list);

    return 1;
}

frame_t *use_frame(void)
{
	frame_t *fm = NULL;
	
	//printf("@use_frame\n");
	sem_wait(&pict_ready);
	pthread_mutex_lock(&frame_list_lock);
	if( ready_list_head->flag == FRAME_READY )
	{
		fm = ready_list_head;
		fm->flag = FRAME_IN_USE;
		ready_list_head = fm->next;
		fm->next = NULL;
	}
	else
	{
		printf("Err flag of frame %d!\n", ready_list_head-frame_list);
		dump_frame_list();
	}
	pthread_mutex_unlock(&frame_list_lock);
	//printf("use_frame %x ok\n", fm-frame_list);
	return fm;
}

void empty_frame(frame_t *fm)
{
	//printf("empty frame\n");
	pthread_mutex_lock(&frame_list_lock);
	fm->flag = FRAME_EMPTY;
	ready_list_tail->next = fm;
	ready_list_tail = fm;
	if( empty_list_head == NULL ) empty_list_head = fm;
	pthread_mutex_unlock(&frame_list_lock);
	//printf("empty frame %x ok\n", fm-frame_list);
	return;
}

int init_frame_list()
{
	int i;

	ready_list_head = &frame_list[0];
	ready_list_tail = ready_list_head;
	ready_list_head->flag = FRAME_EMPTY;
	ready_list_head->data = malloc(200*1024);
	for( i = 1; i < FRAME_NUM; i++ )
	{
		frame_t *fm;
		fm = &frame_list[i];
		fm->flag = FRAME_EMPTY;
		fm->data = malloc(MAX_FRAME_LEN);
		ready_list_tail->next = fm;
		ready_list_tail = fm;
	} 
	ready_list_tail->next = NULL;
	empty_list_head = ready_list_head;
	sem_init(&pict_ready, 0, 0);
	pthread_mutex_init(&frame_list_lock);

	return 1;
}

/*
 * read a frame of image from video device
 */
int get_buffer(int fd, struct v4l2_buffer *v4l_buf)
{
    if (-1 == ioctl (fd, VIDIOC_DQBUF, v4l_buf))
		return 0;
    
    return 1;
}

/*
 * enqueue the frame again
 */
int put_buffer(int fd, struct v4l2_buffer *v4l_buf)
{
	return ioctl(fd, VIDIOC_QBUF, v4l_buf);
}

/*
 * stop video device capturing
 */
void stop_capturing (int fd)
{
    enum v4l2_buf_type type;

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

   	if (-1 == ioctl (fd, VIDIOC_STREAMOFF, &type))
		errno_exit ("VIDIOC_STREAMOFF");
}

/*
 * start video device capturing
 */
void start_capturing (int fd)
{
    unsigned int i;
    enum v4l2_buf_type type;
    
    printf("start_capturing...\n");

    for (i = 0; i < n_buffers; ++i) {
        struct v4l2_buffer buf;

        CLEAR (buf);

        buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory	= V4L2_MEMORY_MMAP;
        buf.index	= i;

        if (-1 == ioctl (fd, VIDIOC_QBUF, &buf))
  			errno_exit ("VIDIOC_QBUF");
    }
    
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 == ioctl (fd, VIDIOC_STREAMON, &type))
		errno_exit ("VIDIOC_STREAMON");
}

/*
 * uninit the video device
 */
void uninit_device (void)
{
    unsigned int i;

	for (i = 0; i < n_buffers; ++i)
   		if (-1 == munmap (buffers[i].start, buffers[i].length))
  			errno_exit ("munmap");
  			
    free (buffers);
}

/*
 * init the video device
 */
int init_device (int fd, int width, int height, int fps)
{
    struct v4l2_requestbuffers req;
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;
	int ret;
	struct v4l2_fmtdesc fmtdesc;
	int camera_setting(int);

	
	camera_setting(fd);
 
    CLEAR (req);
    req.count	= 16;
    req.type 	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory	= V4L2_MEMORY_MMAP;

    if (-1 == ioctl (fd, VIDIOC_REQBUFS, &req)) {
        fprintf (stderr, "VIDIOC_QUERYCAP fail\n");
        goto err;
    }

    if (req.count < 2) {
        fprintf (stderr, "Insufficient buffer memory\n");
        return -1;
    }
    
    buffers = calloc (req.count, sizeof (*buffers));
    if (!buffers) {
        fprintf (stderr, "Out of memory\n");
        return -1;
    }

    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        struct v4l2_buffer buf;   

        CLEAR (buf);

        buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory	= V4L2_MEMORY_MMAP;
        buf.index	= n_buffers;

        if (-1 == ioctl (fd, VIDIOC_QUERYBUF, &buf)){
		    fprintf (stderr, "VIDIOC_QUERYCAP fail\n");
		    goto err;
		}

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start = mmap (NULL /* start anywhere */,
				buf.length,
				PROT_READ | PROT_WRITE /* required */,
				MAP_SHARED /* recommended */,
				fd, buf.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start){
		    fprintf (stderr, "mmap fail\n");
		    goto err;
		}
    }

	init_frame_list();

    return 0;

err:
	return -1;
}

/*
 * close the video device
 */
void close_device (int fd)
{
    if (-1 == close (fd))
    	errno_exit ("close");
}

/*
 * open the video device
 */
int open_device (char *dev_name)
{
    struct stat st; 
    int fd = -1;

    if (-1 == stat (dev_name, &st)) {
		fprintf (stderr, "Cannot identify '%s': %d, %s\n",
                     dev_name, errno, strerror (errno));
       	goto err;
    }

    if (!S_ISCHR (st.st_mode)) {
        fprintf (stderr, "%s is no device\n", dev_name);
       	goto err;
    }

    fd = open (dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
    if (-1 == fd) {
        fprintf (stderr, "Cannot open '%s': %d, %s\n",
                 dev_name, errno, strerror (errno));
       	goto err;
    }

err:
	return fd;
}
