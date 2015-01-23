#include <stdlib.h>
#include <stdio.h>
#include <slice_analyse.h>
#include <string.h>

int GetAnnexbNALU (char *data, int datLen, NALU_t *nalu)
{
	int info2, info3, pos = 0;
	int StartCodeFound, rewind;
	int LeadingZero8BitsCount = 0, TrailingZero8Bits = 0;

	while(pos <= datLen - 1 && data[pos++] == 0);

	if(pos >= datLen - 1)
	{
		if(pos == 0)
			return -2;
		else
		{
			printf( "GetAnnexbNALU can't read start code\n");
			return -1;
		}
	}

	if(data[pos-1] != 1)
	{
		printf ("GetAnnexbNALU: pos-1 != 1, no Start Code at the begin of the NALU, return -1\n");
		return -1;
	}

	if(pos < 3)
	{
		printf ("GetAnnexbNALU: pos < 3, no Start Code at the begin of the NALU, return -1\n");
		return -1;
	}
	else if(pos == 3)
	{
		nalu->startcodeprefix_len = 3;
		LeadingZero8BitsCount = 0;
	}
	else
	{
		LeadingZero8BitsCount = pos - 4;
		nalu->startcodeprefix_len = 4;
	}

	nalu->nal_unit_type = data[pos] & 0x1f;
//	printf("type %d pos %d L %d pre %d\n", nalu->nal_unit_type, pos, LeadingZero8BitsCount, nalu->startcodeprefix_len);
//	printf("%x %x %x %x\n", data[pos - 1], data[pos], data[pos + 1], data[pos + 2]);
	return nalu->nal_unit_type;
}

