#define N_(x) x
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "sonix_xu_ctrls.h"

extern struct H264Format *gH264fmt;

unsigned char m_CurrentFPS = 24;
#define Default_fwLen 13
const unsigned char Default_fwData[Default_fwLen] = {0x05, 0x00, 0x02, 0xD0, 0x01,		// W=1280, H=720, NumOfFrmRate=1
										   0xFF, 0xFF, 0xFF, 0xFF,			// Frame size
										   0x07, 0xA1, 0xFF, 0xFF,			// 20
											};

#define LENGTH_OF_SONIX_XU_CTR (7)
#define LENGTH_OF_SONIX_XU_MAP (7)

static struct uvc_xu_control_info sonix_xu_ctrls[] = 
{
	{
		.entity   = UVC_GUID_SONIX_USER_HW_CONTROL,
		.selector = XU_SONIX_ASIC_READ,
		.index    = 0,
		.size     = 4,
		.flags    = UVC_CONTROL_SET_CUR | UVC_CONTROL_GET_MIN | UVC_CONTROL_GET_MAX | 
		            UVC_CONTROL_GET_DEF | UVC_CONTROL_AUTO_UPDATE | UVC_CONTROL_GET_CUR
	},
	{
		.entity   = UVC_GUID_SONIX_USER_HW_CONTROL,
		.selector = XU_SONIX_FRAME_INFO,
		.index    = 1,
		.size     = 11,
		.flags    = UVC_CONTROL_SET_CUR | UVC_CONTROL_GET_MIN | UVC_CONTROL_GET_MAX | 
		            UVC_CONTROL_GET_DEF | UVC_CONTROL_AUTO_UPDATE | UVC_CONTROL_GET_CUR
	},
	{
		.entity   = UVC_GUID_SONIX_USER_HW_CONTROL,
		.selector = XU_SONIX_H264_CTRL,
		.index    = 2,
		.size     = 11,
		.flags    = UVC_CONTROL_SET_CUR | UVC_CONTROL_GET_CUR
	},
	{
		.entity   = UVC_GUID_SONIX_USER_HW_CONTROL,
		.selector = XU_SONIX_MJPG_CTRL,
		.index    = 3,
		.size     = 11,
		.flags    = UVC_CONTROL_SET_CUR | UVC_CONTROL_GET_CUR
	},
	{
		.entity   = UVC_GUID_SONIX_USER_HW_CONTROL,
		.selector = XU_SONIX_OSD_CTRL,
		.index    = 4,
		.size     = 11,
		.flags    = UVC_CONTROL_SET_CUR | UVC_CONTROL_GET_MIN | UVC_CONTROL_GET_MAX | 
		            UVC_CONTROL_GET_DEF | UVC_CONTROL_AUTO_UPDATE | UVC_CONTROL_GET_CUR
	},
	{
		.entity   = UVC_GUID_SONIX_USER_HW_CONTROL,
		.selector = XU_SONIX_MOTION_DETECTION,
		.index    = 5,
		.size     = 11,
		.flags    = UVC_CONTROL_SET_CUR | UVC_CONTROL_GET_CUR
	},
	{
		.entity   = UVC_GUID_SONIX_USER_HW_CONTROL,
		.selector = XU_SONIX_IMG_SETTING,
		.index    = 6,
		.size     = 11,
		.flags    = UVC_CONTROL_SET_CUR | UVC_CONTROL_GET_CUR
	},
};

//  SONiX XU Ctrls Mapping
static struct uvc_xu_control_mapping sonix_xu_mappings[] = 
{
	{
		.id        = V4L2_CID_ASIC_READ_SONIX,
		.name      = "SONiX: Asic Read",
		.entity    = UVC_GUID_SONIX_USER_HW_CONTROL,
		.selector  = XU_SONIX_ASIC_READ,
		.size      = 4,
		.offset    = 0,
		.v4l2_type = V4L2_CTRL_TYPE_INTEGER,
		.data_type = UVC_CTRL_DATA_TYPE_SIGNED
	},
	{
		.id        = V4L2_CID_FRAME_INFO_SONIX,
		.name      = "SONiX: H264 Format",
		.entity    = UVC_GUID_SONIX_USER_HW_CONTROL,
		.selector  = XU_SONIX_FRAME_INFO,
		.size      = 11,
		.offset    = 0,
		.v4l2_type = V4L2_CTRL_TYPE_INTEGER,
		.data_type = UVC_CTRL_DATA_TYPE_RAW
	},
	{
		.id        = V4L2_CID_H264_CTRL_SONIX,
		.name      = "SONiX: H264 Control",
		.entity    = UVC_GUID_SONIX_USER_HW_CONTROL,
		.selector  = XU_SONIX_H264_CTRL,
		.size      = 11,
		.offset    = 0,
		.v4l2_type = V4L2_CTRL_TYPE_INTEGER,
		.data_type = UVC_CTRL_DATA_TYPE_UNSIGNED
	},
	{
		.id        = V4L2_CID_MJPG_CTRL_SONIX,
		.name      = "SONiX: MJPG Control",
		.entity    = UVC_GUID_SONIX_USER_HW_CONTROL,
		.selector  = XU_SONIX_MJPG_CTRL,
		.size      = 11,
		.offset    = 0,
		.v4l2_type = V4L2_CTRL_TYPE_INTEGER,
		.data_type = UVC_CTRL_DATA_TYPE_UNSIGNED
	},
	{
		.id        = V4L2_CID_OSD_CTRL_SONIX,
		.name      = "SONiX: OSD Control",
		.entity    = UVC_GUID_SONIX_USER_HW_CONTROL,
		.selector  = XU_SONIX_OSD_CTRL,
		.size      = 11,
		.offset    = 0,
		.v4l2_type = V4L2_CTRL_TYPE_INTEGER,
		.data_type = UVC_CTRL_DATA_TYPE_RAW
	},
	{
		.id        = V4L2_CID_MOTION_DETECTION_SONIX,
		.name      = "SONiX: Motion Detection",
		.entity    = UVC_GUID_SONIX_USER_HW_CONTROL,
		.selector  = XU_SONIX_MOTION_DETECTION,
		.size      = 11,
		.offset    = 0,
		.v4l2_type = V4L2_CTRL_TYPE_INTEGER,
		.data_type = UVC_CTRL_DATA_TYPE_UNSIGNED
	},
	{
		.id        = V4L2_CID_IMG_SETTING_SONIX,
		.name      = "SONiX: Image Setting",
		.entity    = UVC_GUID_SONIX_USER_HW_CONTROL,
		.selector  = XU_SONIX_IMG_SETTING,
		.size      = 11,
		.offset    = 0,
		.v4l2_type = V4L2_CTRL_TYPE_INTEGER,
		.data_type = UVC_CTRL_DATA_TYPE_UNSIGNED
	},

};
// Houston 2011/08/08 XU ctrls ----------------------------------------------------------

int XU_Init_Ctrl(int fd) 
{
	int i=0;
	int err=0;
	/* try to add all controls listed above */
	for ( i=0; i<LENGTH_OF_SONIX_XU_CTR; i++ ) 
	{
		printf("Adding XU Ctrls - %s\n", sonix_xu_mappings[i].name);
		if ((err=ioctl(fd, UVCIOC_CTRL_ADD, &sonix_xu_ctrls[i])) < 0 ) 
		{
			if ((errno == EEXIST) || (errno != EACCES)) 
			{	
				printf("UVCIOC_CTRL_ADD - Ignored, uvc driver had already defined\n");
				return (-EEXIST);
			}
			else if (errno == EACCES)
			{
				printf("Need admin previledges for adding extension unit(XU) controls\n");
				printf("please run 'SONiX_UVC_TestAP --add_ctrls' as root (or with sudo)\n");
				return  (-1);
			}
			else perror("Control exists");
		}
	}
	/* after adding the controls, add the mapping now */
	for ( i=0; i<LENGTH_OF_SONIX_XU_MAP; i++ ) 
	{
		printf("Mapping XU Ctrls - %s\n", sonix_xu_mappings[i].name);
		if ((err=ioctl(fd, UVCIOC_CTRL_MAP, &sonix_xu_mappings[i])) < 0) 
		{
			if ((errno!=EEXIST) || (errno != EACCES))
			{
				printf("UVCIOC_CTRL_MAP - Error");
				return (-2);
			}
			else if (errno == EACCES)
			{
				printf("Need admin previledges for adding extension unit(XU) controls\n");
				printf("please run 'SONiX_UVC_TestAP --add_ctrls' as root (or with sudo)\n");
				return  (-1);
			}
			else perror("Mapping exists");
		}
	} 
	return 0;
}

int XU_Ctrl_ReadChipID(int fd)
{
	printf("XU_Ctrl_ReadChipID ==>\n");
	int ret = 0;
	int err = 0;
	__u8 ctrldata[4];

	struct uvc_xu_control xctrl = {
		3,										/* bUnitID 				*/
		XU_SONIX_ASIC_READ,						/* function selector	*/
		4,										/* data size			*/
		&ctrldata								/* *data				*/
		};
	
	xctrl.data[0] = 0x1f;
	xctrl.data[1] = 0x10;
	xctrl.data[2] = 0x0;
	xctrl.data[3] = 0xFF;		/* Dummy Write */
	
	/* Dummy Write */
	if ((err=ioctl(fd, UVCIOC_CTRL_SET, &xctrl)) < 0) 
	{
		printf("   ioctl(UVCIOC_CTRL_SET) FAILED (%i) : can be ignored, if used SN9C291\n",err);
		if(err==EINVAL)
			printf("    Invalid arguments\n");
		
		/* Houston 2011/06/14 SN9C291 XU +++ */
		if(err<0)
		{
			printf("Try 291 XU ... \n");
			xctrl.unit = 3;
			if ((err=ioctl(fd, UVCIOC_CTRL_SET, &xctrl)) < 0)
			{
				printf("   291 - ioctl(UVCIOC_CTRL_SET) FAILED (%i) == \n",err);
				return err;			
			}
		}
		/* Houston 2011/06/14 SN9C291 XU --- */
		/* return err; */
	}
	
	/* Asic Read */
	xctrl.data[3] = 0x00;
	if ((err=ioctl(fd, UVCIOC_CTRL_GET, &xctrl)) < 0) 
	{
		printf("   ioctl(UVCIOC_CTRL_GET) FAILED (%i) == \n",err);
		if(err==EINVAL)
			printf("    Invalid arguments\n");
		return err;
	}
	
	printf("   == XU_Ctrl_ReadChipID Success == \n");
	printf("      ASIC READ data[0] : %x\n", xctrl.data[0]);
	printf("      ASIC READ data[1] : %x\n", xctrl.data[1]);
	printf("      ASIC READ data[2] : %x (Chip ID)\n", xctrl.data[2]);
	printf("      ASIC READ data[3] : %x\n", xctrl.data[3]);
	
	printf("XU_Ctrl_ReadChipID <==\n");
	return ret;
}

int H264_GetFormat(int fd)
{
	printf("H264_GetFormat ==>\n");
	int i,j;
	int iH264FormatCount = 0;
	int success = 1;
	
	unsigned char *fwData = NULL;
	unsigned short fwLen = 0;

	// Init H264 XU Ctrl Format
	if( XU_H264_InitFormat(fd) < 0 )
	{
		printf(" H264 XU Ctrl Format failed , use default Format\n");
		fwLen = Default_fwLen;
		fwData = (unsigned char *)calloc(fwLen,sizeof(unsigned char));
		memcpy(fwData, Default_fwData, fwLen);
		goto Skip_XU_GetFormat;
	}

	// Probe : Get format through XU ctrl
	success = XU_H264_GetFormatLength(fd, &fwLen);
	if( success < 0 || fwLen <= 0)
	{
		printf(" XU Get Format Length failed !\n");
	}
	printf("fwLen = 0x%x\n", fwLen);
	
	// alloc memory
	fwData = (unsigned char *)calloc(fwLen,sizeof(unsigned char));

	if( XU_H264_GetFormatData(fd, fwData,fwLen) < 0 )
	{
		printf(" XU Get Format Data failed !\n");
	}

Skip_XU_GetFormat :

	// Get H.264 format count
	iH264FormatCount = H264_CountFormat(fwData, fwLen);

	printf("H264_GetFormat ==> FormatCount : %d \n", iH264FormatCount);

	if(iH264FormatCount>0)
		gH264fmt = (struct H264Format *)malloc(sizeof(struct H264Format)*iH264FormatCount);
	else
	{
		printf("Get Resolution Data Failed\n");
	}

	// Parse & Save Size/Framerate into structure
	success = H264_ParseFormat(fwData, fwLen, gH264fmt);

	if(success)
	{
		printf("H264_GetFormat SUCCESS!\n");
	/*	for(i=0; i<iH264FormatCount; i++)
		{
			printf("Format index: %d --- (%d x %d) ---\n", i+1, gH264fmt[i].wWidth, gH264fmt[i].wHeight);
			for(j=0; j<gH264fmt[i].fpsCnt; j++)
				printf("(%d) %2d fps\n", j+1, H264_GetFPS(gH264fmt[i].FrPay[j]));
		} */
	}

	if(fwData)
	{
		free(fwData);
		fwData = NULL;
	}

	printf("H264_GetFormat <==\n");
	return success;
}

int H264_CountFormat(unsigned char *Data, int len)
{
	int fmtCnt = 0;
	int fpsCnt = 0;
	int cur_len = 0;
	int cur_fmtid = 0;
	int cur_fpsNum = 0;
	
	if( Data == NULL || len == 0)
		return 0;

	// count Format numbers
	while(cur_len < len)
	{
		cur_fpsNum = Data[cur_len+4];
		
//		printf("H264_CountFormat ==> cur_len = %d, cur_fpsNum=%d\n", cur_len , cur_fpsNum);

		cur_len += 9 + cur_fpsNum * 4;
		fmtCnt++;
	}
	
	if(cur_len != len)
	{
		printf("H264_CountFormat ==> cur_len = %d, fwLen=%d\n", cur_len , len);
		return 0;
	}
	return fmtCnt;
}
int H264_ParseFormat(unsigned char *Data, int len, struct H264Format *fmt)
{
	printf("H264_ParseFormat ==>\n");
	int fpsCnt = 0;
	int cur_len = 0;
	int cur_fmtid = 0;
	int cur_fpsNum = 0;
	int i;

	while(cur_len < len)
	{
		// Copy Size
		fmt[cur_fmtid].wWidth  = ((unsigned short)Data[cur_len]<<8)   + (unsigned short)Data[cur_len+1];
		fmt[cur_fmtid].wHeight = ((unsigned short)Data[cur_len+2]<<8) + (unsigned short)Data[cur_len+3];
		fmt[cur_fmtid].fpsCnt  = Data[cur_len+4];
		fmt[cur_fmtid].FrameSize =	((unsigned int)Data[cur_len+5] << 24) | 
						((unsigned int)Data[cur_len+6] << 16) | 
						((unsigned int)Data[cur_len+7] << 8 ) | 
						((unsigned int)Data[cur_len+8]);

//		printf("Data[5~8]: 0x%02x%02x%02x%02x \n", Data[cur_len+5],Data[cur_len+6],Data[cur_len+7],Data[cur_len+8]);
//		printf("fmt[%d].FrameSize: 0x%08x \n", cur_fmtid, fmt[cur_fmtid].FrameSize);

		// Alloc memory for Frame rate 
		cur_fpsNum = Data[cur_len+4];

		fmt[cur_fmtid].FrPay = (unsigned int *)malloc(sizeof(unsigned int)*cur_fpsNum);
		for(i=0; i<cur_fpsNum; i++)
		{
			fmt[cur_fmtid].FrPay[i] =	(unsigned int)Data[cur_len+9+i*4]   << 24 | 
							(unsigned int)Data[cur_len+9+i*4+1] << 16 |
							(unsigned int)Data[cur_len+9+i*4+2] << 8  |
							(unsigned int)Data[cur_len+9+i*4+3] ;
			
		//printf("fmt[cur_fmtid].FrPay[%d]: 0x%x \n", i, fmt[cur_fmtid].FrPay[i]);
		}
		
		// Do next format
		cur_len += 9 + cur_fpsNum * 4;
		cur_fmtid++;
	}
	if(cur_len != len)
	{
		printf("H264_ParseFormat <==\n");
		return 0;
	}

	printf("H264_ParseFormat <==\n");
	return 1;
}

int H264_GetFPS(unsigned int FrPay)
{
	int fps = 0;

	//printf("H264_GetFPS==> FrPay = 0x%04x\n", (FrPay & 0xFFFF0000)>>16);

	unsigned short frH = (FrPay & 0xFF000000)>>16;
	unsigned short frL = (FrPay & 0x00FF0000)>>16;
	unsigned short fr = (frH|frL);
	
	//printf("FrPay: 0x%x -> fr = 0x%x\n",FrPay,fr);
	
	fps = ((unsigned int)10000000/fr)>>8;

	//printf("fps : %d\n", fps);

	return fps;
}


int XU_H264_InitFormat(int fd)
{
	printf("XU_H264_InitFormat ==>\n");
	int i = 0;
	int ret = 0;
	int err = 0;
	__u8 ctrldata[11]={0};

	struct uvc_xu_control xctrl = {
		3,										/* bUnitID 				*/
		XU_SONIX_FRAME_INFO,					/* function selector	*/
		11,										/* data size			*/
		&ctrldata								/* *data				*/
		};

	// Switch command : FMT_NUM_INFO
	// xctrl.data[0] = 0x01;
	xctrl.data[0] = 0x9A;
	xctrl.data[1] = 0x01;
	
	if ((err=ioctl(fd, UVCIOC_CTRL_SET, &xctrl)) < 0) 
	{
		printf("   Set Switch command : FMT_NUM_INFO FAILED (%i)\n",err);
		return err;
	}
	
	//xctrl.data[0] = 0;
	memset(xctrl.data, 0, xctrl.size);
	if ((err=ioctl(fd, UVCIOC_CTRL_GET, &xctrl)) < 0)
	{	
		printf("   ioctl(UVCIOC_CTRL_GET) FAILED (%i)\n",err);
		return err;
	}
	
//	for(i=0; i<xctrl.size; i++)
//		printf(" Get Data[%d] = 0x%x\n",i, xctrl.data[i]);
//	printf(" ubH264Idx_S1 = %d\n", xctrl.data[5]);

	
	printf("XU_H264_InitFormat <== Success\n");
	return ret;
}

int XU_H264_GetFormatLength(int fd, unsigned short *fwLen)
{
	printf("XU_H264_GetFormatLength ==>\n");
	int i = 0;
	int ret = 0;
	int err = 0;
	__u8 ctrldata[11]={0};

	struct uvc_xu_control xctrl = {
		3,										/* bUnitID 				*/
		XU_SONIX_FRAME_INFO,					/* function selector	*/
		11,										/* data size			*/
		&ctrldata								/* *data				*/
		};

	// Switch command : FMT_Data Length_INFO
	//	xctrl.data[0] = 0x02;
	xctrl.data[0] = 0x9A;
	xctrl.data[1] = 0x02;

	if ((err=ioctl(fd, UVCIOC_CTRL_SET, &xctrl)) >= 0) 
	{
		//for(i=0; i<11; i++)	xctrl.data[i] = 0;		// clean data
		memset(xctrl.data, 0, xctrl.size);

		if ((err=ioctl(fd, UVCIOC_CTRL_GET, &xctrl)) >= 0)
		{
//			for (i=0 ; i<11 ; i+=2)
//				printf(" Get Data[%d] = 0x%x\n", i, (xctrl.data[i]<<8)+xctrl.data[i+1]);

			// Get H.264 format length
			*fwLen = ((unsigned short)xctrl.data[6]<<8) + xctrl.data[7];
//			printf(" H.264 format Length = 0x%x\n", *fwLen);
		}
		else
		{
			printf("   ioctl(UVCIOC_CTRL_GET) FAILED (%i)\n",err);
			return err;
		}
	}
	else
	{
		printf("   Set Switch command : FMT_Data Length_INFO FAILED (%i)\n",err);
		return err;
	}
	
	printf("XU_H264_GetFormatLength <== Success\n");
	return ret;
}

int XU_H264_GetFormatData(int fd, unsigned char *fwData, unsigned short fwLen)
{
	printf("XU_H264_GetFormatData ==>\n");
	int i = 0;
	int ret = 0;
	int err = 0;
	__u8 ctrldata[11]={0};

	int loop = 0;
	int LoopCnt  = (fwLen%11) ? (fwLen/11)+1 : (fwLen/11) ;
	int Copyleft = fwLen;

	struct uvc_xu_control xctrl = {
		3,										/* bUnitID 				*/
		XU_SONIX_FRAME_INFO,					/* function selector	*/
		11,										/* data size			*/
		&ctrldata								/* *data				*/
		};

	
	// Switch command
	xctrl.data[0] = 0x9A;		
	xctrl.data[1] = 0x03;		// FRM_DATA_INFO
	
	if ((err=ioctl(fd, UVCIOC_CTRL_SET, &xctrl)) < 0)
	{
		printf("   Set Switch command : FRM_DATA_INFO FAILED (%i)\n",err);
		return err;
	}

	// Get H.264 Format Data
	xctrl.data[0] = 0x02;		// Stream: 1
	xctrl.data[1] = 0x01;		// Format: 1 (H.264)

	if ((err=ioctl(fd, UVCIOC_CTRL_SET, &xctrl)) >= 0) 
	{
		// Read all H.264 format data
		for(loop = 0 ; loop < LoopCnt ; loop ++)
		{
//			for(i=0; i<11; i++)	xctrl.data[i] = 0;		// clean data
			
//			printf("--> Loop : %d <--\n",  loop);
			
			if ((err=ioctl(fd, UVCIOC_CTRL_GET, &xctrl)) >= 0)
			{
//				for (i=0 ; i<11 ; i++)
//					printf(" Data[%d] = 0x%x\n", i, xctrl.data[i]);

				// Copy Data
				if(Copyleft >= 11)
				{
					memcpy( &fwData[loop*11] , xctrl.data, 11);
					Copyleft -= 11;
				}
				else
				{
					memcpy( &fwData[loop*11] , xctrl.data, Copyleft);
					Copyleft = 0;
				}
			}
			else
			{
				printf("   ioctl(UVCIOC_CTRL_GET) FAILED (%i)\n",err);
				return err;
			}
		}
	}
	else
	{
		printf("   Set Switch command : FRM_DATA_INFO FAILED (%i)\n",err);
		return err;
	}
	
	printf("XU_H264_GetFormatData <== Success\n");
	return ret;
}

int XU_H264_SetFormat(int fd, struct Cur_H264Format fmt)
{
	// Need to get H264 format first
	if(gH264fmt==NULL)
	{
		printf("SONiX_UVC_TestAP @XU_H264_SetFormat : Do XU_H264_GetFormat before setting H264 format\n");
		return -EINVAL;
	}
	
	printf("XU_H264_SetFormat ==> %d - (%d x %d):%d fps\n", 
			fmt.FmtId+1, 
			gH264fmt[fmt.FmtId].wWidth, 
			gH264fmt[fmt.FmtId].wHeight, 
			H264_GetFPS(gH264fmt[fmt.FmtId].FrPay[fmt.FrameRateId]));
	
	int ret = 0;
	int err = 0;
	__u8 ctrldata[11]={0};

	struct uvc_xu_control xctrl = {
			3,										/* bUnitID 				*/
			XU_SONIX_FRAME_INFO,					/* function selector	*/
			11,										/* data size			*/
			&ctrldata								/* *data				*/
		};

	// Switch command
	xctrl.data[0] = 0x9A;				// Tag
	xctrl.data[1] = 0x21;				// Commit_INFO
	
	if ((err=ioctl(fd, UVCIOC_CTRL_SET, &xctrl)) < 0) 
	{
		printf("XU_H264_Set_FMT ==> Switch cmd : ioctl(UVCIOC_CTRL_SET) FAILED (%i) == \n",err);
		if(err==EINVAL)
			printf("Invalid arguments\n");
		return err;
	}

	// Set H.264 format setting data
	xctrl.data[0] = 0x02;				// Stream : 1
	xctrl.data[1] = 0x01;				// Format index : 1 (H.264) 
	xctrl.data[2] = fmt.FmtId + 1;		// Frame index (Resolution index), firmware index starts from 1
	xctrl.data[3] = ( gH264fmt[fmt.FmtId].FrPay[fmt.FrameRateId] & 0xFF000000 ) >> 24;	// Frame interval
	xctrl.data[4] = ( gH264fmt[fmt.FmtId].FrPay[fmt.FrameRateId] & 0x00FF0000 ) >> 16;
	xctrl.data[5] = ( gH264fmt[fmt.FmtId].FrameSize & 0x00FF0000) >> 16;
	xctrl.data[6] = ( gH264fmt[fmt.FmtId].FrameSize & 0x0000FF00) >> 8;
	xctrl.data[7] = ( gH264fmt[fmt.FmtId].FrameSize & 0x000000FF);
	xctrl.data[8] = ( gH264fmt[fmt.FmtId].FrPay[fmt.FrameRateId] & 0x0000FF00 ) >> 8;
	xctrl.data[9] = ( gH264fmt[fmt.FmtId].FrPay[fmt.FrameRateId] & 0x000000FF ) ;
	

	if ((err=ioctl(fd, UVCIOC_CTRL_SET, &xctrl)) < 0) 
	{
		printf("XU_H264_Set_FMT ==> ioctl(UVCIOC_CTRL_SET) FAILED (%i) == \n",err);
		if(err==EINVAL)
			printf("Invalid arguments\n");
		return err;
	}
	
	printf("XU_H264_SetFormat <== Success \n");
	return ret;

}


int XU_H264_Get_QP(int fd, int *QP_Val)
{	
	printf("XU_H264_Get_QP ==>\n");
	int i = 0;
	int ret = 0;
	int err = 0;
	__u8 ctrldata[11]={0};
	*QP_Val = -1;

	struct uvc_xu_control xctrl = {
			3,										/* bUnitID 				*/
			XU_SONIX_H264_CTRL,						/* function selector	*/
			11,										/* data size			*/
			&ctrldata								/* *data				*/
		};

	// Switch command
	xctrl.data[0] = 0x9A;				// Tag
	xctrl.data[1] = 0x05;				// H264_VBR_QP
	
	if ((err=ioctl(fd, UVCIOC_CTRL_SET, &xctrl)) < 0) 
	{
		printf("XU_H264_Get_QP ==> Switch cmd : ioctl(UVCIOC_CTRL_SET) FAILED (%i) == \n",err);
		if(err==EINVAL)
			printf("Invalid arguments\n");
		return err;
	}
	
	// Get QP value
	if ((err=ioctl(fd, UVCIOC_CTRL_GET, &xctrl)) < 0) 
	{
		printf("XU_H264_Get_QP ==> ioctl(UVCIOC_CTRL_GET) FAILED (%i) == \n",err);
		if(err==EINVAL)
			printf("Invalid arguments\n");
		return err;
	}
	
	printf("   == XU_H264_Get_QP Success == \n");
	for(i=0; i<3; i++)
			printf("      Get data[%d] : 0x%x\n", i, xctrl.data[i]);

	*QP_Val = xctrl.data[0];
	
	printf("XU_H264_Get_QP (0x%x)<==\n", *QP_Val);
	return ret;

}

int XU_H264_Set_QP(int fd, int QP_Val)
{	
	printf("XU_H264_Set_QP (0x%x) ==>\n", QP_Val);
	int ret = 0;
	int err = 0;
	__u8 ctrldata[11]={0};

	struct uvc_xu_control xctrl = {
			3,										/* bUnitID 				*/
			XU_SONIX_H264_CTRL,						/* function selector	*/
			11,										/* data size			*/
			&ctrldata								/* *data				*/
		};

	// Switch command
	xctrl.data[0] = 0x9A;				// Tag
	xctrl.data[1] = 0x05;				// H264_VBR_QP
	
	if ((err=ioctl(fd, UVCIOC_CTRL_SET, &xctrl)) < 0) 
	{
		printf("XU_H264_Get_QP ==> Switch cmd : ioctl(UVCIOC_CTRL_SET) FAILED (%i) == \n",err);
		if(err==EINVAL)
			printf("Invalid arguments\n");
		return err;
	}
	

	// Set QP value
	memset(xctrl.data, 0, xctrl.size);
	xctrl.data[0] = QP_Val;
	if ((err=ioctl(fd, UVCIOC_CTRL_SET, &xctrl)) < 0) 
	{
		printf("XU_H264_Set_QP ==> ioctl(UVCIOC_CTRL_SET) FAILED (%i) == \n",err);
		if(err==EINVAL)
			printf("Invalid arguments\n");
		return err;
	}
	
	printf("XU_H264_Set_QP <== Success \n");
	return ret;

}

int XU_H264_Get_BitRate(int fd, double *BitRate)
{
	printf("XU_H264_Get_BitRate ==>\n");
	int i = 0;
	int ret = 0;
	int err = 0;
	__u8 ctrldata[11]={0};
	int BitRate_CtrlNum = 0;
	*BitRate = -1.0;

	struct uvc_xu_control xctrl = {
			3,										/* bUnitID 				*/
			XU_SONIX_H264_CTRL,					/* function selector	*/
			11,										/* data size			*/
			&ctrldata								/* *data				*/
		};
	
	// Switch command
	xctrl.data[0] = 0x9A;				// Tag
	xctrl.data[1] = 0x03;				// H264_BitRate
	
	if ((err=ioctl(fd, UVCIOC_CTRL_SET, &xctrl)) < 0) 
	{
		printf("XU_H264_Get_BitRate ==> Switch cmd : ioctl(UVCIOC_CTRL_SET) FAILED (%i) == \n",err);
		if(err==EINVAL)
			printf("Invalid arguments\n");
		return err;
	}
	
	// Get Bit rate ctrl number
	memset(xctrl.data, 0, xctrl.size);
	if ((err=ioctl(fd, UVCIOC_CTRL_GET, &xctrl)) < 0) 
	{
		printf("XU_H264_Get_BitRate ==> ioctl(UVCIOC_CTRL_GET) FAILED (%i) == \n",err);
		if(err==EINVAL)
			printf("Invalid arguments\n");
		return err;
	}
	
	printf("   == XU_H264_Get_BitRate Success == \n");
	for(i=0; i<2; i++)
			printf("      Get data[%d] : 0x%x\n", i, xctrl.data[i]);

	BitRate_CtrlNum = ( xctrl.data[0]<<8 )| (xctrl.data[1]) ;

	// Bit Rate = BitRate_Ctrl_Num*512*fps*8 /1000(Kbps)
	*BitRate = (double)(BitRate_CtrlNum*512.0*m_CurrentFPS*8)/1000.0;
	
	printf("XU_H264_Get_BitRate (%.2f)<==\n", *BitRate);
	return ret;

	
}

int XU_H264_Set_BitRate(int fd, double BitRate)
{	
	printf("XU_H264_Set_BitRate (%.2f) ==>\n",BitRate);
	int ret = 0;
	int err = 0;
	__u8 ctrldata[11]={0};
	int BitRate_CtrlNum = 0;

	struct uvc_xu_control xctrl = {
			3,										/* bUnitID 				*/
			XU_SONIX_H264_CTRL,						/* function selector	*/
			11,										/* data size			*/
			&ctrldata								/* *data				*/
		};

	// Switch command
	xctrl.data[0] = 0x9A;				// Tag
	xctrl.data[1] = 0x03;				// H264_BitRate
	
	if ((err=ioctl(fd, UVCIOC_CTRL_SET, &xctrl)) < 0) 
	{
		printf("XU_H264_Get_BitRate ==> Switch cmd : ioctl(UVCIOC_CTRL_SET) FAILED (%i) == \n",err);
		if(err==EINVAL)
			printf("Invalid arguments\n");
		return err;
	}

	// Set Bit Rate Ctrl Number
	
	// Bit Rate = BitRate_Ctrl_Num*512*fps*8/1000 (Kbps)
	BitRate_CtrlNum = (int)((BitRate*1000)/(512*m_CurrentFPS*8));
	xctrl.data[0] = (BitRate_CtrlNum & 0xFF00)>>8;	// BitRate ctrl Num
	xctrl.data[1] = (BitRate_CtrlNum & 0x00FF);
	
	if ((err=ioctl(fd, UVCIOC_CTRL_SET, &xctrl)) < 0) 
	{
		printf("XU_H264_Set_BitRate ==> ioctl(UVCIOC_CTRL_SET) FAILED (%i) == \n",err);
		if(err==EINVAL)
			printf("Invalid arguments\n");
		return err;
	}
	
	printf("XU_H264_Set_BitRate <== Success \n");
	return ret;

}

int XU_Get(int fd, struct uvc_xu_control *xctrl)
{
	printf("XU Get ==>\n");
	int i = 0;
	int ret = 0;
	int err = 0;

	// XU Get
	if ((err=ioctl(fd, UVCIOC_CTRL_GET, xctrl)) < 0) 
	{
		printf("XU Get ==> ioctl(UVCIOC_CTRL_GET) FAILED (%i) == \n",err);
		if(err==EINVAL)
			printf("Invalid arguments\n");
		return err;
	}
	
	printf("   == XU Get Success == \n");
	for(i=0; i<xctrl->size; i++)
			printf("      Get data[%d] : 0x%x\n", i, xctrl->data[i]);
	return ret;
}

int XU_Set(int fd, struct uvc_xu_control xctrl)
{
	printf("XU Set ==>\n");
	int i = 0;
	int ret = 0;
	int err = 0;

	// XU Set
	for(i=0; i<xctrl.size; i++)
			printf("      Set data[%d] : 0x%x\n", i, xctrl.data[i]);
	
	if ((err=ioctl(fd, UVCIOC_CTRL_SET, &xctrl)) < 0) 
	{
		printf("XU Set ==> ioctl(UVCIOC_CTRL_SET) FAILED (%i) == \n",err);
		if(err==EINVAL)
			printf("Invalid arguments\n");
		return err;
	}
	printf("   == XU Set Success == \n");
	return ret;
}


