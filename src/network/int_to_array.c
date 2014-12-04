/*
 * int_to_array.c
 *  int to array and array to int
 *  jgfntu@163.com
 *
 */

#include "int_to_array.h"

int true = 1;

u32 byteArrayToInt(u8 byte[], int i, int j)
{
	int k = 0;
	int l = 0;
	do
	{
		if(l >= j)
			return k;
		int i1 = (j + -1 + i) - l;
		int j1 = byte[i1] & 0xff;
		k |= j1;
		int k1 = j + -1;
		if(l < k1)
			k <<= 8;
		l++;
	} while(true);
}

u8* int16ToByteArray(int i)
{
	u8 byte0 = 2;
	u8 byte[byte0];
	int j = 0;
	do
	{
		if(j >= byte0)
			return byte;
		int k = j * 8;
		u8 byte1 = (u8)(i >> k & 0xff);
		byte[j] = byte1;
		j++;
	} while(true);
}

u8* int32ToByteArray(int i)
{
	u8 byte0 = 4;
	u8 byte[byte0];
	int j = 0;
	do
	{
		if(j >= byte0)
			return byte;
		int k = j * 8;
		u8 byte1 = (u8)(i >> k & 0xff);
		byte[j] = byte1;
		j++;
	} while(true);
}

u8* int8ToByteArray(int i)
{
	int j = 1;
	u8 byte[j];
	int k = 0;
	do
	{
		if(k >= j)
			return byte;
		int l = k * 8;
		u8 byte0 = (u8)(i >> l & 0xff);
		byte[k] = byte0;
		k++;
	} while(true);
}
