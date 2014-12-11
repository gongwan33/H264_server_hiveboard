/*
 *      test.c  --  USB Video Class test application
 *
 *      Copyright (C) 2005-2008
 *          Laurent Pinchart (laurent.pinchart@skynet.be)
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 */

/*
 * WARNING: This is just a test application. Don't fill bug reports, flame me,
 * curse me on 7 generations :-).
 */

#include <stdio.h>
#include <errno.h>
#include "sonix_xu_ctrls.h"
#include "v4l2uvc.h"
#include "v4l2.h"


struct H264Format *gH264fmt = NULL;


int camera_setting(int camera_fd)
{

	int dev, ret;
	unsigned int input = 0;

	struct Cur_H264Format cur_H264fmt;

	double m_BitRate = 0.0;

	dev = camera_fd;

	ret = XU_Init_Ctrl(dev);
	if(ret<0)
	{
		if(ret == -EEXIST)
			printf("SONiX_UVC_TestAP @main : Initial XU Ctrls ignored, uvc driver had already supported\n\t\t\t No need to Add Extension Unit Ctrls into Driver\n");
		else
			printf("SONiX_UVC_TestAP @main : Initial XU Ctrls Failed (%i)\n",ret);
	}


	if(!H264_GetFormat(dev))
		printf("SONiX_UVC_TestAP @main : H264 Get Format Failed\n");

	cur_H264fmt.FrameRateId = 1;
	cur_H264fmt.FmtId = 0;
	if(XU_H264_SetFormat(dev, cur_H264fmt) < 0)
		printf("SONiX_UVC_TestAP @main : H264 Set Format Failed\n");


	m_BitRate = 1600;
	if(XU_H264_Set_BitRate(dev, m_BitRate) < 0 )
		printf("SONiX_UVC_TestAP @main : XU_H264_Set_BitRate Failed\n");


    v4l2ResetControl (dev, V4L2_CID_BRIGHTNESS);
	v4l2ResetControl (dev, V4L2_CID_SATURATION);
	v4l2ResetControl (dev, V4L2_CID_CONTRAST);
	v4l2ResetControl (dev, V4L2_CID_GAIN);
	v4l2ResetControl (dev, V4L2_CID_HUE);
	v4l2ResetControl (dev, V4L2_CID_WHITE_BALANCE_TEMPERATURE);
	v4l2ResetControl (dev, V4L2_CID_GAMMA);


/*
	v4l2SetControl (dev, V4L2_CID_SATURATION, 80);
	v4l2SetControl (dev, V4L2_CID_CONTRAST, 50);
	v4l2SetControl (dev, V4L2_CID_BRIGHTNESS, 30);
	v4l2SetControl (dev, V4L2_CID_GAIN, 40);
	v4l2SetControl (dev, V4L2_CID_HUE, 40);
	v4l2SetControl (dev, V4L2_CID_WHITE_BALANCE_TEMPERATURE, 5200);
	v4l2SetControl (dev, V4L2_CID_GAMMA, 100);

	int setval = v4l2GetControl (dev, V4L2_CID_WHITE_BALANCE_TEMPERATURE);
	printf("white balance: %d\n", setval);
	
	setval = v4l2GetControl (dev, V4L2_CID_SATURATION);
	printf("saturtion: %d\n", setval);

	setval = v4l2GetControl (dev, V4L2_CID_CONTRAST);
	printf("contrast: %d\n", setval);

	setval = v4l2GetControl (dev, V4L2_CID_BRIGHTNESS);
	printf("brightness: %d\n", setval);

	setval = v4l2GetControl (dev, V4L2_CID_GAIN);
	printf("gain: %d\n", setval);

    setval = v4l2GetControl (dev, V4L2_CID_HUE);
	printf("hue: %d\n", setval);

	setval = v4l2GetControl (dev, V4L2_CID_GAMMA);
	printf("gamma: %d\n", setval);
	*/	
}

