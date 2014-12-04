#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/select.h> 
#include <fcntl.h>					/* for O_WRONLY ... */
#include <semaphore.h>

#include "types.h"
#include "audio.h"
#include "rw.h"
#include "adpcm.h"
#include "buffer.h"

/* --------------------------------------------------------- */
struct AUDIO_CFG 
{
	int rate;
	int channels;
	int bit;
	int ispk;
};

enum TalkBufferFlag
{
	START_TALK,
	KEEP_TALK,
	END_TALK,
	STOP_TALK
};

enum TalkReceiveState
{
	TALK_INIT,
	TALK_RECEIVE,
	TALK_STOPPED
};

enum TalkPlaybackState
{
	PLAYER_INIT,
	PLAYER_PLAYBACK,
	PLAYER_STOPPED,
	PLAYER_RESET
};

#define BUFFER_NUM 3
BufferQueue g_talk_buffer_queue;
#define TALK_QUEUE (&g_talk_buffer_queue)

u8 g_decoded_buffer[2][ADPCM_MAX_READ_LEN * 4];


/*
 * clear AVdata socket buffer
 */
void clear_recv_buf(int client_fd)
{
	int recv_size = 0;
	int  nret;
	char tmp[2];
	struct timeval tm_out;
	fd_set  fds;

	tm_out.tv_sec = 0;
	tm_out.tv_usec = 0;

	FD_ZERO(&fds);
	FD_SET(client_fd, &fds);
#if 0	
	if ((recv_size = recv(client_fd, runtime_recv_buf, RUNTIME_RECV_LEN, 0)) == -1) {
		perror("recv");
		close(client_fd);
		exit(0);
	}
	printf("first * clear recv socket buffer : %d×Ö½Ú!\n", recv_size);
	recv_size = 0;
#endif
	while(1) {
		nret = select(FD_SETSIZE, &fds, NULL, NULL, &tm_out);
		if(nret == 0)
			break;
		recv(client_fd, tmp, 1, 0);
		recv_size++;
	}
	printf("   second * clear recv socket buffer : %d×Ö½Ú!\n", recv_size);
#if 0
	do {
		if ((recv_size = recv(client_fd, runtime_recv_buf, RUNTIME_RECV_LEN, 0)) == -1) {
			perror("recv");
			close(client_fd);
			exit(0);
		}
		printf("clear recv socket buffer : %d×Ö½Ú!\n", recv_size);
	} while (recv_size > 0);
#endif
}

static void InitBufferList( BufferList *list, Buffer *head, int len )
{
	Buffer *p = head;
	int i;
	for( i = 0; i < len-1; i++ )
	{
		p[i].next = &p[i+1];
	}
	if( len > 0 )
	{
		p[i].next = NULL;
		list->head = head;
		list->tail = &p[i];
	}
	else
	{
		list->head = NULL;
		list->tail = NULL;
	}
	sem_init( &list->ready, 0, 0 );
	list->wanted = 0;
	return;
}

static void ResetBufferList( BufferList *list, Buffer *head, int len )
{
	Buffer *p = head;
	int i;
	for( i = 0; i < len-1; i++ )
	{
		p[i].next = &p[i+1];
	}
	if( len > 0 )
	{
		p[i].next = NULL;
		list->head = head;
		list->tail = &p[i];
	}
	else
	{
		list->head = NULL;
		list->tail = NULL;
	}
	if( list->wanted )
	{
		sem_post( &list->ready );
		list->wanted = 0;
	}
	return;
}

static inline void InBufferList( BufferList *list, Buffer *node )
{
	if( list->head == NULL )
	{
		list->head = node;
		list->tail = list->head;
	}
	else
	{
		list->tail->next = node;
		list->tail = node;
	}
	node->next = NULL;
	return;
}

static inline Buffer *OutBufferList( BufferList *list )
{
	Buffer *node = NULL;

	if( list->head != NULL )
	{
		node = list->head;
		list->head = node->next;
		node->next = NULL;
		if( node == list->tail )
		{
			list->tail = NULL;
		}
	}
	return node;
}


Buffer *GetBuffer( BufferQueue *queue )
{
	Buffer *p = NULL;

	pthread_mutex_lock( &queue->lock );
	if( queue->out_state == QUEUE_WORKING )
	{
	p = queue->list_filled.head;
	if( p == NULL )
	{
		queue->list_filled.wanted = 1;
		pthread_mutex_unlock( &queue->lock );

		sem_wait(&queue->list_filled.ready);
		if( queue->out_state != QUEUE_WORKING )
		{
			//printf("GetBuffer Cancelled\n");
			return NULL;
		}

		pthread_mutex_lock( &queue->lock );
		p = queue->list_filled.head;
	}
	}
	pthread_mutex_unlock( &queue->lock );
	//printf("GetBuffer %x\n",p);
	return p;
}

int EmptyBuffer( BufferQueue *queue )
{
	Buffer *p;
	pthread_mutex_lock( &queue->lock );
	if( queue->out_state == QUEUE_WORKING )
	{
		p = OutBufferList( &queue->list_filled );
		//printf("EmptyBuffer %x\n",p);
		if( p != NULL )
		{
			InBufferList( &queue->list_empty, p );
			if( queue->list_empty.wanted )
			{
				sem_post(&queue->list_empty.ready);
				queue->list_empty.wanted = 0;
			}
		}
	}
	pthread_mutex_unlock( &queue->lock );
	return 1;
}

Buffer *GetEmptyBuffer( BufferQueue *queue )
{
	Buffer *p = NULL;

	pthread_mutex_lock( &queue->lock );
	if( queue->in_state == QUEUE_WORKING )
	{
		p = queue->list_empty.head;
		if( p == NULL )
		{
			queue->list_empty.wanted = 1;
			pthread_mutex_unlock( &queue->lock );
			sem_wait(&queue->list_empty.ready);
			if( queue->in_state != QUEUE_WORKING )
			{
				//printf("GetEmptyBuffer Cancelled\n");
				return NULL;
			}
			pthread_mutex_lock( &queue->lock );
			p = queue->list_empty.head;
		}
	}
	pthread_mutex_unlock( &queue->lock );
	//printf("GetEmptyBuffer %x\n",p);
	return p;
}

int FillBuffer( BufferQueue *queue )
{
	Buffer *p;
	pthread_mutex_lock( &queue->lock );
	if( queue->in_state == QUEUE_WORKING )
	{
		p = OutBufferList( &queue->list_empty );
		//printf("FillBuffer %x\n",p);
		if( p != NULL )
		{
			InBufferList( &queue->list_filled, p );
			if( queue->list_filled.wanted )
			{
				sem_post(&queue->list_filled.ready);
				queue->list_filled.wanted = 0;
			}
		}
	}
	pthread_mutex_unlock( &queue->lock );
	return 1;
}

void InitQueue( BufferQueue *queue, const char *name, int num )
{
	printf("   InitQueue(%s)\n",name);
	queue->buffer = (Buffer *)malloc(sizeof(Buffer)*num);
	queue->buffer_num = num;
	queue->name = name;

	queue->in_state = QUEUE_STOPPED;
	queue->out_state = QUEUE_STOPPED;
	InitBufferList( &queue->list_filled, NULL, 0 );
	InitBufferList( &queue->list_empty, queue->buffer, num );

	pthread_mutex_init(&queue->lock,NULL);
	sem_init(&queue->in_ready,0,0);
	sem_init(&queue->out_ready,0,0);

	return;
}

int OpenQueueIn( BufferQueue *queue )
{
	sem_wait( &queue->in_ready );
	printf("   Open Queue(%s) In\n", queue->name);
	return ( queue->in_state == QUEUE_WORKING );
}

int OpenQueueOut( BufferQueue *queue )
{
	sem_wait( &queue->out_ready );
	printf("   Open Queue(%s) Out\n", queue->name);
	return ( queue->out_state == QUEUE_WORKING );
}

void EnableBufferQueue( BufferQueue *queue )
{
	printf("   Enable queue(%s)\n", queue->name);
	queue->in_state = QUEUE_WORKING;
	queue->out_state = QUEUE_WORKING;
	sem_trywait( &queue->in_ready );
	sem_post( &queue->in_ready );
	sem_trywait( &queue->out_ready );
	sem_post( &queue->out_ready );
	return;
}

void DisableBufferQueue( BufferQueue *queue )
{
	printf("   Disable queue(%s)\n",queue->name);
	pthread_mutex_lock( &queue->lock );
	ResetBufferList( &queue->list_filled, NULL, 0 );
	ResetBufferList( &queue->list_empty, queue->buffer, queue->buffer_num );
	queue->in_state = QUEUE_STOPPED;
	queue->out_state = QUEUE_STOPPED;
	sem_trywait( &queue->in_ready );
	sem_trywait( &queue->out_ready );
	pthread_mutex_unlock( &queue->lock );
	return;
}

