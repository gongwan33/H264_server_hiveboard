#ifndef SONIX_XU_CTRLS_H
#define SONIX_XU_CTRLS_H

#include <linux/videodev2.h>
/*
 * Dynamic controls
 */

#define UVC_CTRL_DATA_TYPE_RAW		0
#define UVC_CTRL_DATA_TYPE_SIGNED	1
#define UVC_CTRL_DATA_TYPE_UNSIGNED	2
#define UVC_CTRL_DATA_TYPE_BOOLEAN	3
#define UVC_CTRL_DATA_TYPE_ENUM		4
#define UVC_CTRL_DATA_TYPE_BITMASK	5

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

#define V4L2_CID_BASE_EXTCTR_SONIX					0x0A0c4501
#define V4L2_CID_BASE_SONIX                       	V4L2_CID_BASE_EXTCTR_SONIX
#define V4L2_CID_ASIC_READ_SONIX                    V4L2_CID_BASE_SONIX+1
#define V4L2_CID_FRAME_INFO_SONIX                  	V4L2_CID_BASE_SONIX+2
#define V4L2_CID_H264_CTRL_SONIX                 	V4L2_CID_BASE_SONIX+3
#define V4L2_CID_MJPG_CTRL_SONIX                 	V4L2_CID_BASE_SONIX+4
#define V4L2_CID_OSD_CTRL_SONIX                  	V4L2_CID_BASE_SONIX+5
#define V4L2_CID_MOTION_DETECTION_SONIX             V4L2_CID_BASE_SONIX+6
#define V4L2_CID_IMG_SETTING_SONIX                  V4L2_CID_BASE_SONIX+7
#define V4L2_CID_LAST_EXTCTR_SONIX					V4L2_CID_IMG_SETTING_SONIX

/* ---------------------------------------------------------------------------- */

#define UVC_GUID_SONIX_USER_HW_CONTROL       	{0x70, 0x33, 0xf0, 0x28, 0x11, 0x63, 0x2e, 0x4a, 0xba, 0x2c, 0x68, 0x90, 0xeb, 0x33, 0x40, 0x16}

// ----------------------------- XU Control Selector ------------------------------------
#define XU_SONIX_ASIC_READ 				0x01
//#define XU_SONIX_H264_FMT  			0x06
//#define XU_SONIX_H264_QP   			0x07
//#define XU_SONIX_H264_BITRATE 		0x08
#define XU_SONIX_FRAME_INFO  			0x06
#define XU_SONIX_H264_CTRL   			0x07
#define XU_SONIX_MJPG_CTRL		 		0x08
#define XU_SONIX_OSD_CTRL	  			0x09
#define XU_SONIX_MOTION_DETECTION		0x0A
#define XU_SONIX_IMG_SETTING	 		0x0B

// ----------------------------- XU Control Selector ------------------------------------




#define UVC_CONTROL_SET_CUR	(1 << 0)
#define UVC_CONTROL_GET_CUR	(1 << 1)
#define UVC_CONTROL_GET_MIN	(1 << 2)
#define UVC_CONTROL_GET_MAX	(1 << 3)
#define UVC_CONTROL_GET_RES	(1 << 4)
#define UVC_CONTROL_GET_DEF	(1 << 5)
/* Control should be saved at suspend and restored at resume. */
#define UVC_CONTROL_RESTORE	(1 << 6)
/* Control can be updated by the camera. */
#define UVC_CONTROL_AUTO_UPDATE	(1 << 7)

#define UVC_CONTROL_GET_RANGE   (UVC_CONTROL_GET_CUR | UVC_CONTROL_GET_MIN | \
                                 UVC_CONTROL_GET_MAX | UVC_CONTROL_GET_RES | \
                                 UVC_CONTROL_GET_DEF)

struct uvc_xu_control_info {
	__u8 entity[16];
	__u8 index;
	__u8 selector;
	__u16 size;
	__u32 flags;
};

struct uvc_xu_control_mapping {
	__u32 id;
	__u8 name[32];
	__u8 entity[16];
	__u8 selector;

	__u8 size;
	__u8 offset;
	enum v4l2_ctrl_type v4l2_type;
	__u32 data_type;
};

struct uvc_xu_control {
	__u8 unit;
	__u8 selector;
	__u16 size;
	__u8 *data;
};

struct H264Format
{
	unsigned short  wWidth;
	unsigned short  wHeight;
	int   		fpsCnt;
	unsigned int	FrameSize;
	unsigned int	*FrPay;		// FrameInterval[0|1]Payloadsize[2|3]
};

struct Cur_H264Format
{	
	int FmtId;
	unsigned short wWidth;
	unsigned short wHeight;
	int FrameRateId;
	unsigned char framerate;
	unsigned int FrameSize;
};

#define UVCIOC_CTRL_ADD		_IOW('U', 1, struct uvc_xu_control_info)
#define UVCIOC_CTRL_MAP		_IOWR('U', 2, struct uvc_xu_control_mapping)
#define UVCIOC_CTRL_GET		_IOWR('U', 3, struct uvc_xu_control)
#define UVCIOC_CTRL_SET		_IOW('U', 4, struct uvc_xu_control)

int XU_Init_Ctrl(int fd);
int XU_Ctrl_ReadChipID(int fd);

int H264_GetFormat(int fd);
int H264_CountFormat(unsigned char *Data, int len);
int H264_ParseFormat(unsigned char *Data, int len, struct H264Format *fmt);
int H264_GetFPS(unsigned int FrPay);

// H.264 XU +++++

int XU_Set(int fd, struct uvc_xu_control xctrl);
int XU_Get(int fd, struct uvc_xu_control *xctrl);

int XU_H264_InitFormat(int fd);
int XU_H264_GetFormatLength(int fd, unsigned short *fwLen);
int XU_H264_GetFormatData(int fd, unsigned char *fwData, unsigned short fwLen);
int XU_H264_SetFormat(int fd, struct Cur_H264Format fmt);

int XU_H264_Get_QP(int fd, int *QP_Val);
int XU_H264_Set_QP(int fd, int QP_Val);

int XU_H264_Get_BitRate(int fd, double *BitRate);
int XU_H264_Set_BitRate(int fd, double BitRate);


// H.264 XU -----

#endif



