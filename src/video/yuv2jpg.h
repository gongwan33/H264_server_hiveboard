#ifndef _YUV2JPG_H_
#define _YUV2JPG_H_

typedef struct {
	int size;
	int width;
	int height;
	int quality;
}jpg_info;

int init_jpg_codec(int image_width, int image_height, int quality);
int yuv422_to_jpeg(unsigned char *yuv_data, unsigned char *jpg_data);
int uinit_jpg_codec(void);

#endif
