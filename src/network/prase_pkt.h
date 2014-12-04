/*
 * prase-pkt.h
 *  prase packet head file
 *  jgfntu@163.com
 *
 */

#ifndef __PRASE_PKT_H
#define __PRASE_PKT_H

#include "types.h"

extern int can_send;
extern int data_ID;

/* prase opcode command text */
int packet_packet(int opcode, u8 buf[]);

/* prase AVdata command text */
int packet_AVpacket(int opcode, u8 buf[]);

#endif
