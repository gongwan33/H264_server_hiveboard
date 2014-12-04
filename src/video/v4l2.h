/*
 * v4l2.h
 *  V4L2 video capture. 
 *		flow mothods: open -> init -> start -> ... -> stop -> uninit -> close
 *  chenguangming@wxseuic.com
 */
#ifndef _V4L2_h
#define _V4L2_h

#include <linux/videodev2.h>
#include <camera.h>

typedef struct frame{
	struct frame *next;
	int flag;
	char *data;
	struct timeval time_stamp;
	unsigned long length;
} frame_t;

int open_device (char *dev_name);
int init_device (int fd, int width, int heigth, int fps);
void start_capturing (int fd);
void stop_capturing (int fd);
void uninit_device (void);
void close_device (int fd);
int get_buffer(int fd, struct v4l2_buffer *v4l_buf);
int put_buffer(int fd, struct v4l2_buffer *v4l_buf);
int get_frame(struct v4l2_buffer *v4l_buf);
frame_t *use_frame(void);
void empty_frame(frame_t *fm);
int init_frame_list(void);

#endif
