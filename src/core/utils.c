/*
 * utils.c
 *  some functions about data packet
 *  chenguangming@wxseuic.com
 */
#include <stdio.h>
#include <stdlib.h>
#include <types.h>
#include <protocol.h>

#define BUFFER_SIZE	(1*1024*1024)
#define	THRESHOLD	(24*1024)

static unsigned char repo[BUFFER_SIZE];
static unsigned char *repo_ptr;

void init_mem_repo(void)
{
	repo_ptr = repo;
}

/*
 * allocate a packet, introduce the context length
 */
void *alloc_packet(void)
{	
	int space;
	
	space = BUFFER_SIZE - (repo_ptr - repo);
	
	if(space < THRESHOLD){
		repo_ptr = repo;
		return repo;
	}
	
	return repo_ptr;
}

/*
 * access the context area of packet
 */
void *access_context(void *pkt)
{
	struct command *cmd = pkt;

	if(cmd == NULL)
		return NULL;
	
	return cmd->text;
}

/*
 * access the video data area of packet
 */
void *access_video_data(void *pkt)
{
	struct command *cmd = pkt;
	struct video_data *p_video;

	if(cmd == NULL)
		return NULL;
	
	p_video = &cmd->text[0].video_data;
	
	return p_video->pic_data;
}

/*
 * set the video data size of packet
 */
int set_video_size(void *pkt, u32 size)
{
	struct command *cmd = pkt;
	struct video_data *p_video;

	if(cmd == NULL)
		return -1;
	
	p_video = &cmd->text[0].video_data;
	p_video->pic_len = size;
	
	repo_ptr += sizeof(struct command) + sizeof(struct video_data) + size;

	return 0;
}

/*
 * get the video data size of packet
 */
u32 get_video_size(void *pkt)
{
	struct command *cmd = pkt;
	struct video_data *p_video;

	if(cmd == NULL)
		return 0;
	
	p_video = &cmd->text[0].video_data;

	return p_video->pic_len;
}

/*
 * set the context size of packet
 */
int set_context_size(void *pkt, u32 size)
{
	struct command *cmd = pkt;

	if(cmd == NULL)
		return -1;

	cmd->text_len = size;

	return 0;
}

/*
 * get the context size of packet
 */
u32 get_context_size(void *pkt)
{
	struct command *cmd = pkt;

	if(cmd == NULL)
		return -1;

	return cmd->text_len;
}

