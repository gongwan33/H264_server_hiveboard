/*
 * yuv2jpg.c
 *
 * Copyright (C) 2012, cgm.
 * This file is part of WXSEUI's software.
 */
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include "jpeglib.h"
#include "yuv2jpg.h"

#define WIDTH 640
#define HEIGHT	480
#define WH	(WIDTH*HEIGHT)
#define SIZE WH*2

static jpg_info *jpg;

int init_jpg_codec(int image_width, int image_height, int quality)
{
	jpg = malloc(sizeof(jpg_info));
	
	if(jpg == NULL){
		printf("failed to malloc jpg info\n");
		return -1;
	}
	
	jpg->width = image_width;
	jpg->height = image_height;
	jpg->quality = quality;
	
	return 0;
}

int uinit_jpg_codec(void)
{	
	if(jpg)
		free(jpg);
	
	jpg = NULL;
}

#if 1
int yuv422_to_jpeg(unsigned char *yuv_data, unsigned char *jpg_data)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPIMAGE  buffer;
    int band, i, buf_width[3], buf_height[3], max_line, counter;
    
    unsigned long *yuyv = (unsigned long *)yuv_data;
	unsigned char *y_buf;
	unsigned char *u_buf;
	unsigned char *v_buf;
	unsigned long temp;
	unsigned long line;
	unsigned long block_words;
	int jpg_size = 0;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_mem_dest(&cinfo, jpg_data, &jpg_size);

    cinfo.image_width = jpg->width; 
    cinfo.image_height = jpg->height;
    cinfo.input_components = 3;   
    cinfo.in_color_space = JCS_RGB; 

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, jpg->quality, TRUE);

    cinfo.raw_data_in = TRUE;
    cinfo.jpeg_color_space = JCS_YCbCr;
    cinfo.comp_info[0].h_samp_factor = 2;
    cinfo.comp_info[0].v_samp_factor = 1;

    jpeg_start_compress(&cinfo, TRUE);

    buffer = (JSAMPIMAGE) (*cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_IMAGE, 3 * sizeof(JSAMPARRAY));
    for(band=0; band <3; band++)
    {
        buf_width[band] = cinfo.comp_info[band].width_in_blocks * DCTSIZE;
        buf_height[band] = cinfo.comp_info[band].v_samp_factor * DCTSIZE;
        buffer[band] = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, buf_width[band], buf_height[band]);
    }

    max_line = cinfo.max_v_samp_factor*DCTSIZE;
	block_words = (buf_height[0]*jpg->width)>>1;

    for(counter=0; cinfo.next_scanline < cinfo.image_height; counter++)
    {
		y_buf = (unsigned char *)buffer[0][0];
		u_buf = (unsigned char *)buffer[1][0];
		v_buf = (unsigned char *)buffer[2][0];
		for(i=0; i<block_words; i++){
	    	temp = *yuyv++;
	    	*y_buf++ = temp & 0xFF;
	    	*u_buf++ = (temp >> 8) & 0xFF;
	    	*y_buf++ = (temp >> 16) & 0xFF;
	    	*v_buf++ = (temp >> 24) & 0xFF;
        }

        jpeg_write_raw_data(&cinfo, buffer, max_line);
    }  
	
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    return jpg_size;
}

#else
int yuv422_to_jpeg(unsigned char *data, unsigned char *jpg_data, int image_width, int image_height, int quality)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1]; 
    int row_stride;   
    JSAMPIMAGE  buffer;
    int band,i,buf_width[3],buf_height[3], mem_size, max_line, counter;
    unsigned char *yuv[3];
    unsigned char *pSrc, *pDst;
	int y_n,u_n,v_n;
	int u,v;
	unsigned char Ysub[WH];
	unsigned char Usub[WH];
	unsigned char Vsub[WH];
	int jpg_size;

	y_n=0;
	u_n=0;
	v_n=0;
	u=1;
	v=3;
	i=0;
	
	while(i<image_height*image_width*2){
		if(i%2 == 0){
			Ysub[y_n]= *(data+i);
			y_n++;		
		}
		else if(i == u){
			Usub[u_n]= *(data+i);
			u += 4;	
			u_n++;	
		}
		else if(i == v){
			Vsub[v_n] = *(data+i);
			v += 4;
			v_n++;
		}
		i++;
	}

	yuv[0] = Ysub;
	yuv[1] = Usub;
	yuv[2] = Vsub;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
 //   jpeg_stdio_dest(&cinfo, fp);
 	jpeg_mem_dest(&cinfo, jpg_data, &jpg_size);

    cinfo.image_width = image_width; 
    cinfo.image_height = image_height;
    cinfo.input_components = 3;   
    cinfo.in_color_space = JCS_RGB; 

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE );

    cinfo.raw_data_in = TRUE;
    cinfo.jpeg_color_space = JCS_YCbCr;
    cinfo.comp_info[0].h_samp_factor = 2;
    cinfo.comp_info[0].v_samp_factor = 1;

    jpeg_start_compress(&cinfo, TRUE);

    buffer = (JSAMPIMAGE) (*cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_IMAGE, 3 * sizeof(JSAMPARRAY));
    for(band=0; band <3; band++)
    {
        buf_width[band] = cinfo.comp_info[band].width_in_blocks * DCTSIZE;
        buf_height[band] = cinfo.comp_info[band].v_samp_factor * DCTSIZE;
        buffer[band] = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, buf_width[band], buf_height[band]);
    }
    max_line = cinfo.max_v_samp_factor*DCTSIZE;
    for(counter=0; cinfo.next_scanline < cinfo.image_height; counter++)
    {
        //buffer image copy.
        for(band=0; band <3; band++)
        {
            mem_size = buf_width[band];
            pDst = (unsigned char *) buffer[band][0];
            pSrc = (unsigned char *) yuv[band] + counter*buf_height[band] * buf_width[band];
            
            for(i=0; i <buf_height[band]; i++)
            {
                memcpy(pDst, pSrc, mem_size);
                pSrc += buf_width[band];
                pDst += buf_width[band];
            }
        }   

        jpeg_write_raw_data(&cinfo, buffer, max_line);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    return 0;
}
#endif

#ifdef _TEST
int main()
{
	long HighStart,LowStart,HighEnd,LowEnd;
	long numhigh,numlow;

	FILE *tar_jpeg;
	FILE *rfp;
	unsigned char *rData = malloc(WIDTH*HEIGHT*2);
	unsigned char *jpg = malloc(WIDTH*HEIGHT*2);
	if ((tar_jpeg = fopen("test.jpg", "wb")) == NULL) 
	{  
		return FALSE;
	}  
	rfp=fopen("p_00001.yuv", "rb");
	if(rfp == NULL){
		printf("err open yuv\n");
	}
	fread(rData, WIDTH*HEIGHT*2, 1, rfp);
    int size = yuv422_to_jpeg(rData, jpg, WIDTH, HEIGHT, 80);

	if(size > 0){
		printf("compress jpg in %d Bytes\n", size);
		fwrite(jpg, size, 1, tar_jpeg);
	}

	fclose(tar_jpeg);
	fclose(rfp);
    
    free(rData);     
    free(jpg);     
    
	return 0;
}

#endif
