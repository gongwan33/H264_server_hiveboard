#ifndef _BUFFER_H
#define _BUFFER_H

enum QueueState
{
	QUEUE_INIT = 0,
	QUEUE_WORKING,
	QUEUE_STOPPED
};

#define MAX_BUF_LEN	(MAX_READ_LEN)
typedef struct s_Buffer
{
	struct s_Buffer *next;
	u8 data[MAX_BUF_LEN];
    struct timeval time_stamp;
	int len;
	int flag;
}Buffer;

typedef struct
{
	Buffer *head;
	Buffer *tail;
	sem_t ready;
	int wanted;
}BufferList;

#define IN_PORT		1
#define OUT_PORT	2

typedef struct
{
	Buffer *buffer;
	BufferList list_filled;
	BufferList list_empty;
	int buffer_num;
	int in_state;
	int out_state;
	sem_t in_ready;
	sem_t out_ready;
	pthread_mutex_t lock;
	const char *name;
}BufferQueue;

Buffer *GetBuffer( BufferQueue *queue );
int EmptyBuffer( BufferQueue *queue );
Buffer *GetEmptyBuffer( BufferQueue *queue );
int FillBuffer( BufferQueue *queue );
void InitQueue( BufferQueue *queue, const char *name, int num );
int OpenQueueIn( BufferQueue *queue );
int OpenQueueOut( BufferQueue *queue );
void EnableBufferQueue( BufferQueue *queue );
void DisableBufferQueue( BufferQueue *queue );

#endif
