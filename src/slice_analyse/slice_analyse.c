#include <stdlib.h>
#include <stdio.h>
#include <slice_analyse.h>
#include <string.h>

static int IsFirstByteStreamNALU = 1;

static int FindStartCode (unsigned char *Buf, int zeros_in_startcode)
{
	int info;
	int i;

	info = 1;
	for (i = 0; i < zeros_in_startcode; i++)
		if(Buf[i] != 0)
			info = 0;

	if(Buf[i] != 1)
		info = 0;
	return info;
}

int GetAnnexbNALU (char *data, int datLen, NALU_t *nalu)
{
	int info2, info3, pos = 0;
	int StartCodeFound, rewind;
	char *Buf;
	int LeadingZero8BitsCount=0, TrailingZero8Bits=0;


	while(pos <= datLen - 1 && data[pos++] == 0)

	if(pos >= datLen - 1)
	{
		if(pos == 0)
			return 0;
		else
		{
			printf( "GetAnnexbNALU can't read start code\n");
			return -1;
		}
	}

	if(data[pos-1] != 1)
	{
		printf ("GetAnnexbNALU: no Start Code at the begin of the NALU, return -1\n");
		return -1;
	}

	if(pos < 3)
	{
		printf ("GetAnnexbNALU: no Start Code at the begin of the NALU, return -1\n");
		return -1;
	}
	else if(pos == 3)
	{
		nalu->startcodeprefix_len = 3;
		LeadingZero8BitsCount = 0;
	}
	else
	{
		LeadingZero8BitsCount = pos-4;
		nalu->startcodeprefix_len = 4;
	}

	//allowed to contain it since these zeros(if any) are considered trailing_zero_8bits
	//of the previous byte stream NAL unit.
	if(!IsFirstByteStreamNALU && LeadingZero8BitsCount > 0)
	{
		printf ("GetAnnexbNALU: The leading_zero_8bits syntax can only be present in the first byte stream NAL unit, return -1\n");
		return -1;
	}
	IsFirstByteStreamNALU = 0;

	StartCodeFound = 0;
	info2 = 0;
	info3 = 0;

	while (!StartCodeFound)
	{
		if (pos >= datLen - 1)
		{
			//Count the trailing_zero_8bits
			while(data[pos-2-TrailingZero8Bits] == 0)
				TrailingZero8Bits++;
			nalu->len = (pos-1)-nalu->startcodeprefix_len-LeadingZero8BitsCount-TrailingZero8Bits;
			memcpy (nalu->buf, &data[LeadingZero8BitsCount+nalu->startcodeprefix_len], nalu->len);     
			nalu->forbidden_bit = (nalu->buf[0]>>7) & 1;
			nalu->nal_reference_idc = (nalu->buf[0]>>5) & 3;
			nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;

			return 0;
			//return pos-1;
		}
		pos++;
		info3 = FindStartCode(&data[pos-4], 3);	
		if(info3 != 1)
			info2 = FindStartCode(&Buf[pos-3], 2);	
		StartCodeFound = (info2 == 1 || info3 == 1);
	}

	//Count the trailing_zero_8bits
	if(info3 == 1)	//if the detected start code is 00 00 01, trailing_zero_8bits is sure not to be present
	{				
		while(data[pos-5-TrailingZero8Bits] == 0)
			TrailingZero8Bits++;
	}
	// Here, we have found another start code (and read length of startcode bytes more than we should
	// have.  Hence, go back in the file
	rewind = 0;
	if(info3 == 1)
		rewind = -4;
	else if (info2 == 1)
		rewind = -3;
	else
		printf(" Panic: Error in next start code search \n");

	// Here the leading zeros(if any), Start code, the complete NALU, trailing zeros(if any)
	// and the next start code is in the Buf.
	// The size of Buf is pos, pos+rewind are the number of bytes excluding the next
	// start code, and (pos+rewind)-startcodeprefix_len-LeadingZero8BitsCount-TrailingZero8Bits
	// is the size of the NALU.

	nalu->len = (pos+rewind)-nalu->startcodeprefix_len-LeadingZero8BitsCount-TrailingZero8Bits;
	memcpy (nalu->buf, &data[LeadingZero8BitsCount+nalu->startcodeprefix_len], nalu->len);
	nalu->forbidden_bit = (nalu->buf[0]>>7) & 1;		
	nalu->nal_reference_idc = (nalu->buf[0]>>5) & 3;	
	nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;		

	return (pos+rewind);
}

