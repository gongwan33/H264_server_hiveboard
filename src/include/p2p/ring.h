#ifndef RING_H
#define RING_H

#define RING_LEN 32
#define RING_WAIT_TIMEOUT 30 //seconds
#define RTT_A 7/8
#define RTT_B 2

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <p2p/JEAN_P2P.h>
#include <sys/time.h>
#include <signal.h>  
#include <p2p/JEANP2PPRO.h>
#include <p2p/DSet.h>

#define READY 0x00
#define SEND  0x01
#define RESEND 0x02

void initRing();
int reg_buff(unsigned int index, char *pointer, unsigned char priority, int len);
int unreg_buff(unsigned int index);
void emptyRing();
void printRingStatus();
char *getPointerByIndex(unsigned int index, int *len, int *Prio);

unsigned long lastRegTime = 0;
int rto = 50000;
unsigned long rtt = 0;

struct buf_node
{
    u_int32_t index;
	char priority;
    char *pointer;
	u_int64_t length;
    struct timeval tv;
	unsigned char status;
};

static struct buf_node buf_list[RING_LEN];
static char empty_list[RING_LEN];
static unsigned int seqCount = 0;
static unsigned int lastSeqCount = 0;
static unsigned int seqIndex = -1;
static pthread_mutex_t buf_lock;
static char checkRingRunning = 0;
static char checkRingSign = 0;
unsigned int page = 0;
static pthread_t checkRing_t;

extern int sockfd;
extern struct sockaddr_in slave_sin, turnaddr;
extern unsigned char connectionStatus;

void printRingStatus()
{
    int i = 0;
	printf("ring status:");
	for(i = 0; i < RING_LEN; i++)
		printf("%d ", empty_list[i]);
	printf("\n");

	printf("ring data:");
	for(i = 0; i < RING_LEN; i++)
		printf("%d ", buf_list[i].index);
	printf("\n");

}

void* checkRing(void *argc)
{
	checkRingRunning = 1;

	pthread_detach(pthread_self());

	while(checkRingSign == 1)
	{
		int i = 0;
		int resendDelay = rto;
		int sendLen = 0;
		struct timeval cur_tv;

		for(i = 0; i < RING_LEN; i++)
		{
			sendLen = 0;
			pthread_mutex_lock(&buf_lock);
			if(empty_list[i] != -1)
			{
#if TEST_LOST
				int rnd = 0;
				int lost_emt = LOST_PERCENT;
				rnd = rand()%100;
#ifdef LOST_PRINT
				if(rnd <= lost_emt)
					printf("Lost!!: rnd = %d, lost_emt = %d, lost_posibility = %d percent\n", rnd, lost_emt, lost_emt);
#endif
				if(rnd > lost_emt)
				{
#endif
					if(seqIndex >= 0 && buf_list[i].index/RING_LEN == page && buf_list[i].status != SEND && buf_list[i].pointer != NULL)
					{
						sendLen = 0;
						struct load_head head;

						*(buf_list[i].pointer + ((void *)&(head.priority) - (void *)&head)) = buf_list[i].priority;
						gettimeofday(&(buf_list[i].tv), NULL);
						if(connectionStatus == P2P)
						{
							sendLen = sendto(sockfd, buf_list[i].pointer, buf_list[i].length, 0, (struct sockaddr *)&slave_sin, sizeof(struct sockaddr_in));
						}
						else if(connectionStatus == TURN)
						{
							sendLen = sendto(sockfd, buf_list[i].pointer, buf_list[i].length, 0, (struct sockaddr *)&turnaddr, sizeof(turnaddr));
						}
						else 
						{
							pthread_mutex_unlock(&buf_lock);
							return; 
						}

						if(sendLen >= buf_list[i].length)
						{
							buf_list[i].status = SEND;
							buf_list[i].priority--;
						}
					}
#if TEST_LOST
				}
#endif

				gettimeofday(&cur_tv, NULL);
				if(buf_list[i].priority <= 0 && buf_list[i].status == SEND)
				{
					if(empty_list[i] != -1)
					{
						if(empty_list[i] != -1 && buf_list[i].pointer != NULL)
						{
							free(buf_list[i].pointer);
							buf_list[i].pointer = NULL;

							seqCount++;	
							if(!(seqCount%RING_LEN))
							{
								sendSyn();
#if PRINT
								printf("send Sync %d\n", seqCount);
#endif
							}

						}
						empty_list[i] = -1;
					}
				}

				gettimeofday(&cur_tv, NULL);
				if(cur_tv.tv_sec*1000000 + cur_tv.tv_usec - buf_list[i].tv.tv_sec*1000000 - buf_list[i].tv.tv_usec > resendDelay && buf_list[i].status == SEND && buf_list[i].priority > 0)
				{
					buf_list[i].status = RESEND;
				}

			}

			pthread_mutex_unlock(&buf_lock);

			if(sendLen > 0)
				usleep(10000/sendLen);
			else
				usleep(10);
		}
	}

	checkRingRunning = 0;
}

void initRing()
{

	pthread_mutex_init(&buf_lock, NULL);

	memset(empty_list, -1, RING_LEN);
	memset(buf_list, -1, RING_LEN);

	if(checkRingRunning == 0)
	{
		checkRingRunning = 1;
		checkRingSign = 1;
		pthread_create(&checkRing_t, NULL, checkRing, NULL);
	}
}

void emptyRing()
{
	int i = 0;

	for(i = 0; i < RING_LEN; i++)
	{
        if(empty_list[i] == 1)
		{
			free(buf_list[i].pointer);
			empty_list[i] = -1;
		}
	}

	pthread_mutex_destroy(&buf_lock);
}

int getEmpPos()
{
	int i = 0;
	struct timeval cur_tv;
	gettimeofday(&cur_tv, NULL);
	for(i = 0; i < RING_LEN; i++)
	{
		if(empty_list[i] == -1)
			return i;
	}
	return -1;
}

int getIndexPos(unsigned int index)
{
    int i = 0 ;
	for(i = 0; i < RING_LEN; i++)
		if(buf_list[i].index == index)
			if(empty_list[i] == 1)
				return i;

	return -1;
}

char *getPointerByIndex(unsigned int index, int *len, int *Prio)
{
	int pos = 0;
	pos = getIndexPos(index);
	if(pos < 0)
	{
		*len = 0;
		*Prio = 0;
		return NULL;
	}

	*len = buf_list[pos].length;
	*Prio = buf_list[pos].priority;

	return buf_list[pos].pointer;

}

int reg_buff(unsigned int index, char *pointer, unsigned char priority, int len)
{
	int times = 0;
    int pos = 0;
	struct timeval regtv;

	gettimeofday(&regtv, NULL);

	pos = getEmpPos();
	while(pos == -1 && regtv.tv_sec - lastRegTime < RING_WAIT_TIMEOUT)
	{
		gettimeofday(&regtv, NULL);
		pos = getEmpPos();
		usleep(10);
		times++;
	}
	pthread_mutex_lock(&buf_lock);

	if(regtv.tv_sec - lastRegTime >= RING_WAIT_TIMEOUT && lastRegTime != 0)
	{
		printf("block error happened\n");
		exit(0);
	}

	lastRegTime = regtv.tv_sec;

    buf_list[pos].index = index;
    buf_list[pos].pointer = pointer;
    buf_list[pos].priority = priority;
	buf_list[pos].length = len;
	buf_list[pos].status = READY;
    empty_list[pos] = 1;
	pthread_mutex_unlock(&buf_lock);
	seqIndex++;
    return 0;	
}

int unreg_buff(unsigned int index)
{
	int pos = 0;
	struct timeval gettv;

	pthread_mutex_lock(&buf_lock);
	pos = getIndexPos(index);

	if(pos == -1)
	{
		pthread_mutex_unlock(&buf_lock);
		return -1;
	}

	gettimeofday(&gettv, NULL);
	unsigned long new_rtt = ((gettv.tv_sec - buf_list[pos].tv.tv_sec)*1000000 + (gettv.tv_usec - buf_list[pos].tv.tv_usec));
	rtt = rtt*RTT_A + new_rtt - new_rtt*RTT_A;
	rto = rtt*RTT_B;

	if(empty_list[pos] != -1 && buf_list[pos].pointer != NULL)
	{
		free(buf_list[pos].pointer);
		buf_list[pos].pointer = NULL;
		seqCount++;	
		if(!(seqCount%RING_LEN))
		{
			sendSyn();
#if PRINT
			printf("send Sync %d\n", seqCount);
#endif
		}
	}
	empty_list[pos] = -1;

	pthread_mutex_unlock(&buf_lock);
	return 0;
}

#endif
