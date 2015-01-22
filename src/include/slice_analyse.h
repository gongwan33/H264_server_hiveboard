#ifndef SLICE_ANALYSE_H
#define SLICE_ANALYSE_H

#include <stdlib.h>
#include <stdio.h>

typedef struct 
{
	int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
	unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
	unsigned max_size;            //! Nal Unit Buffer size
	int nal_unit_type;            //! NALU_TYPE_xxxx
	int nal_reference_idc;        //! NALU_PRIORITY_xxxx
	int forbidden_bit;            //! should be always FALSE
	unsigned char *buf;        //! contains the first byte followed by the EBSP
} NALU_t;

#endif
