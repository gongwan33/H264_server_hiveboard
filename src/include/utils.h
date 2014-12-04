/*
 * utils.c
 *  some functions about data packet
 *  chenguangming@wxseuic.com
 */
#ifndef _UTILS_H
#define _UTILS_H

#include <types.h>

void init_mem_repo(void);
void *alloc_packet(void);
void *access_context(void *pkt);
void *access_video_data(void *pkt);
void free_packet(void *pkt);
int set_context_size(void *pkt, u32 size);
u32 get_context_size(void *pkt);
int set_video_size(void *pkt, u32 size);
u32 get_video_size(void *pkt);

#endif
