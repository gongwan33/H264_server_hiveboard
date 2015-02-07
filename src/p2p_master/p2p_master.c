/*AUTHOER:WANGGONG, CHINA
 *VERSION:1.0
 *FUNCTION:MASTER
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <pthread.h>
#include <p2p/JEANP2PPRO.h>
#include <p2p/commonkey.h>
#include <p2p/ring.h>
#include <p2p/DSet.h>
#include <slice_analyse.h>

#define MAX_TRY 5
#define SEND_BUFF_SIZE 1024*3
#define MAX_RECEIVE 1024*1024
#define MAX_RECV_BUF 1024*1024*10
#define MTU 1000
#define CONTROL_BUF_SIZE 2000

//#define server_ip_1 "192.168.1.109"
#define server_ip_1 "192.168.1.149"
//#define server_ip_1 "23.89.232.109"

#define USERNAME "wang"
#define PASSWD "123456"

#define ACT_NETCARD "eth0"
#define server_port 61000
#define server_turn_port 61001
#define server_cmd_port 61002
#define server_control_port 61003
#define local_port 6888

static char recvSign;
static char controlSign;
struct sockaddr_in slave_sin, turnaddr;
int sockfd;
static struct sockaddr_in servaddr1, local_addr, recv_sin, host_sin;
static struct ifreq ifr, *pifr;
static struct ifconf ifc;
static char ip_info[50];
static int cmdfd;
static int controlfd;
static int port, sin_size, recv_sin_len;
static char mac[6], ip[4], buff[1024];
static pthread_t keep_connection;
static char pole_res;
static unsigned int sendIndex;
static unsigned int sliceIndex;
static unsigned int getNum;
static unsigned int sendNum;
static char* recvBuf;
static char* recvProcessBuf;
static char* recvProcessBackBuf;
static pthread_t recvDat_id = 0;
static pthread_mutex_t recvBuf_lock;
static pthread_mutex_t synGetCount_lock;
static pthread_mutex_t ring_lock;
static pthread_mutex_t send_lock;
static unsigned int recvBufP;
static unsigned int recvProcessBufP;
static unsigned int recvProcessBackBufP;
static char recvThreadRunning = 0;
static int synGetCount = 0;
static unsigned char windowLen = 0;

int JEAN_recv_timeout = 1000;//1s
int commonKey = 0;
static int controlChanThreadRunning = 0;
static pthread_t control_t;

unsigned char connectionStatus = FAIL;

extern unsigned int page;

int init_CONTROL_CHAN();
int close_CONTROL_CHAN();
int send_control(void *, int);
int recv_control(void *, int);

int local_net_init(int port){
	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	local_addr.sin_port = htons(port);

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
		return -1;
	}

	if(bind(sockfd, (struct sockaddr *) (&local_addr),sizeof(local_addr)) == -1){
		printf ("Bind error: %s\a\n", strerror (errno));
		return -2;
	}

	return 0;
}

void init_recv_sin(){
	memset(&recv_sin, 0, sizeof(recv_sin));
	recv_sin.sin_family = AF_INET;
	recv_sin.sin_addr.s_addr = inet_addr("1.1.1.1");
	recv_sin.sin_port = htons(10000);
	recv_sin_len = sizeof(recv_sin);
}

void init_host_sin(char * str_ip, int p){
	memset(&host_sin, 0, sizeof(host_sin));
	host_sin.sin_family = AF_INET;
	host_sin.sin_addr.s_addr = inet_addr(str_ip);
	host_sin.sin_port = htons(p);
}

int get_local_ip_port(){
	int i = 0;
	sin_size = sizeof(local_addr);
	getsockname(sockfd, (struct sockaddr*)&local_addr, &sin_size);
	port = ntohs(local_addr.sin_port);

	memset(&ifr, 0, sizeof(ifr));
	ifc.ifc_len = sizeof(buff);
	ifc.ifc_buf = buff;

	if(ioctl(sockfd, SIOCGIFCONF, &ifc) < 0){
		printf("SIOCGIFCONF screwed up\n");
		return -1;
	}

	pifr = (struct ifreq *)(ifc.ifc_req);

	for(i = ifc.ifc_len / sizeof(struct ifreq); --i >= 0; pifr++){
		strcpy(ifr.ifr_name, pifr->ifr_name);    //eth0 eth1 ...

		if(strcmp(ACT_NETCARD, ifr.ifr_name) != 0)
			continue;

		if(ioctl(sockfd, SIOCGIFADDR, &ifr) < 0){
			printf("ip screwed up\n");
			return -2;
		}
		memcpy(ip, ifr.ifr_addr.sa_data+2, 4);
		//printf("%s\n",inet_ntoa(*(struct in_addr *)ip));
	}	

	printf("local ip is %s\n",inet_ntoa(*(struct in_addr *)ip));
	printf("local port is %d\n", port);

	init_host_sin(inet_ntoa(*(struct in_addr*)ip), port);

	return 0;
}

int set_ip1_struct(char * ip1, int port){
	memset(&servaddr1, 0, sizeof(servaddr1));
	servaddr1.sin_family = AF_INET;
	servaddr1.sin_port = htons(port);
	
	if( inet_pton(AF_INET, ip1, &servaddr1.sin_addr) <= 0){
		printf("inet_pton error for %s\n",ip1);
		return -1;
	}

	memset(&turnaddr, 0, sizeof(turnaddr));
	turnaddr.sin_family = AF_INET;
	turnaddr.sin_port = htons(server_turn_port);
	
	if( inet_pton(AF_INET, ip1, &turnaddr.sin_addr) <= 0){
		printf("inet_pton error for %s\n",ip1);
		return -1;
	}
	return 0;
}

void set_rec_timeout(int usec, int sec){
	struct timeval tv_out;
    tv_out.tv_sec = sec;
    tv_out.tv_usec = usec;

	setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv_out, sizeof(tv_out));
}

int Send_VUAP(){
	struct p2p_head head;
	memcpy(&head.logo, "VUP", 3);
	if(strlen(USERNAME) > 10 || strlen(PASSWD) > 10) 
		return -1;

	memcpy(head.data, USERNAME, 10);
	memcpy(head.data + 10, PASSWD, 10);
	memcpy(head.data + 20, &host_sin, sizeof(struct sockaddr_in));

	sendto(sockfd, &head, sizeof(struct p2p_head), 0, (struct sockaddr *)&servaddr1, sizeof(servaddr1));
	return 0;
}

int Send_CLOSE(){
	struct p2p_head head;
	memcpy(&head.logo, "QIT", 3);

	memcpy(head.data, USERNAME, 10);

	sendto(sockfd, &head, sizeof(struct p2p_head), 0, (struct sockaddr *)&servaddr1, sizeof(servaddr1));
	return 0;
}

int Send_TURN(){
	struct p2p_head head;
	memcpy(&head.logo, "TRN", 3);

	sendto(sockfd, &head, sizeof(struct p2p_head), 0, (struct sockaddr *)&servaddr1, sizeof(servaddr1));
	return 0;
}

int Send_CMDOPEN(){
	struct p2p_head head;
	memcpy(&head.logo, "CMD", 3);
	head.data[0] = 'M';

	sendto(sockfd, &head, sizeof(struct p2p_head), 0, (struct sockaddr *)&servaddr1, sizeof(servaddr1));
	return 0;
}

int Send_CONTROLOPEN(){
	struct p2p_head head;
	memcpy(&head.logo, "CTL", 3);
	head.data[0] = 'M';

	sendto(sockfd, &head, sizeof(struct p2p_head), 0, (struct sockaddr *)&servaddr1, sizeof(servaddr1));
	return 0;
}

void Send_POL(char req,struct sockaddr_in * sock){
	ip_info[0] = req;
	sendto(sockfd, ip_info, 2, 0, (struct sockaddr *)sock, sizeof(struct sockaddr_in));
}

void Send_CMD(char Ctls, char Res){
	struct p2p_head head;
	if(Ctls == GET_REQ)
		memcpy(head.logo, "GRQ", 3);
	else if(Ctls == KEEP_CON)
		memcpy(head.logo, "KEP", 3);
	head.data[0] = Res;
	sendto(sockfd, &head, sizeof(struct p2p_head), 0, (struct sockaddr *)&servaddr1, sizeof(servaddr1));
}

void Send_CMD_TO_SLAVE(char Ctls, char Res){
	ip_info[0] = Ctls;
	ip_info[1] = Res;
	sendto(sockfd, ip_info, sizeof(ip_info), 0, (struct sockaddr *)&slave_sin, sizeof(struct sockaddr_in));
}

void *Keep_con(){
	pthread_detach(pthread_self());
	while(1){
		Send_CMD(KEEP_CON, 0x01);
		printf("Send KEEP_CON!\n");
		sleep(10);
	}
}

void clean_rec_buff(){
	char tmp[50];
	int ret = 1;
	set_rec_timeout(100000, 0);//(usec, sec)
	while(ret > 0){
		ret = recvfrom(sockfd, tmp, 10, 0, (struct sockaddr *)&recv_sin, &recv_sin_len);
		printf("Clean recv buff %d.\n", ret);
	}
	set_rec_timeout(0, 1);//(usec, sec)
}

void sendGet(unsigned int index)
{
	struct get_head getSt;
	unsigned char * buf;
	unsigned char num[4];
	memcpy(&getSt, "GET", 3);
	buf = (char *)malloc(sizeof(struct get_head) + sizeof(u_int32_t));
	memcpy(buf, &getSt, sizeof(getSt));

	num[0] = index & 0xff;
	num[1] = (index>>8) & 0xff;
	num[2] = (index>>16) & 0xff;
	num[3] = (index>>24) & 0xff;

	memcpy(buf + sizeof(getSt), num, 4);
	
	send_control(buf, sizeof(struct get_head) + sizeof(u_int32_t));
	free(buf);
}

void sendWin(unsigned char winLen)
{
	struct win_head winSt;
	unsigned char * buf;
	memcpy(&winSt, "WIN", 3);
	buf = (char *)malloc(sizeof(struct win_head) + sizeof(unsigned char));
	memcpy(buf, &winSt, sizeof(struct win_head));
	buf[3] = winLen;
	
	send_control(buf, sizeof(struct win_head) + sizeof(unsigned char));
	printf("send win %d\n", buf[3]);
	free(buf);
}

void sendSyn()
{
	struct syn_head getSt;
	memcpy(&getSt, "SYN", 3);

	send_control((void *)&getSt, sizeof(struct syn_head));
}

int init_CMD_CHAN()
{
    struct sockaddr_in pin;

	bzero(&pin,sizeof(pin));
	pin.sin_family = AF_INET;
	pin.sin_addr.s_addr = inet_addr(server_ip_1);
	pin.sin_port = htons(server_cmd_port);

	if((cmdfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("Error opening socket \n");
		return -1;
	}

	if(connect(cmdfd, (void *)&pin, sizeof(pin)) == -1)
	{
		printf("Error connecting to socket \n");
		return -1;
	}

	return 0;
}

int close_CMD_CHAN()
{
	close(cmdfd);
}

int send_cmd(char *data, int len)
{
	int sendLen = 0;
	if(len < 0)
		return -1;
	sendLen = send(cmdfd, data, len, 0);
	if(sendLen == -1)
	{
		printf("Error in send\n");
		return -1;
	}

	return sendLen;
}

int recv_cmd(char *data, int len)
{
	int recvLen = 0;
	recvLen = recv(cmdfd, data, len, 0);
	if(recvLen == -1)
	{
		printf("Error in recv\n");
		return -1;
	}

	return recvLen;
}

int init_CONTROL_CHAN()
{
	struct sockaddr_in pin;

	bzero(&pin,sizeof(pin));
	pin.sin_family = AF_INET;
	pin.sin_addr.s_addr = inet_addr(server_ip_1);
	pin.sin_port = htons(server_control_port);

	if((controlfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("control:Error opening socket \n");
		return -1;
	}

	if(connect(controlfd, (void *)&pin, sizeof(pin)) == -1)
	{
		printf("control:Error connecting to socket \n");
		return -1;
	}

	return 0;
}

int close_CONTROL_CHAN()
{
	close(controlfd);
}

int send_control(void *data, int len)
{
	int sendLen = 0;
	if(len < 0)
		return -1;
	sendLen = send(controlfd, data, len, 0);
	if(sendLen == -1)
	{
		printf("control:Error in send\n");
		return -1;
	}

	return sendLen;
}

int recv_control(void *data, int len)
{
	int recvLen = 0;
	recvLen = recv(controlfd, data, len, 0);
	if(recvLen == -1)
	{
		printf("control:Error in recv\n");
		return -1;
	}

	return recvLen;
}

void* controlChanThread(void *argc)
{
	controlChanThreadRunning = 1;

	pthread_detach(pthread_self());

	int recvLen = 0;
	unsigned char controlBuf[CONTROL_BUF_SIZE];
	int controlBufP = 0;

	while(controlSign == 1)
	{
		recvLen = recv_control(controlBuf, CONTROL_BUF_SIZE); 
		if(recvLen <= 0)
			continue;
		else
			controlBufP += recvLen;

		if(controlBufP >= sizeof(struct syn_head))
		{
			int scanP = 0;
			struct get_head get;

			while(scanP + sizeof(struct syn_head) <= controlBufP)
			{
				if(controlBuf[scanP] == 'G' && controlBuf[scanP + 1] == 'E' && controlBuf[scanP + 2] == 'T')
				{
					
					if(controlBufP - scanP - sizeof(struct get_head) >= sizeof(u_int32_t))
					{
						unsigned int index = 0;
						index = (controlBuf[scanP + 3]) | ((controlBuf[scanP + 4]<<8) & 0xff00) | ((controlBuf[scanP + 5]<<16) & 0xff0000) | ((controlBuf[scanP + 6]<<24) & 0xff000000);
						unreg_buff(index);
						scanP = scanP + sizeof(struct get_head) + sizeof(u_int32_t);
#if PRINT
						printf("get %d\n", index);
#endif
					}
					else 
						break;


				}
				else if(controlBuf[scanP] == 'S' && controlBuf[scanP + 1] == 'Y' && controlBuf[scanP + 2] == 'N')
				{
#if PRINT
					printf("syn\n");
#endif
					pthread_mutex_lock(&synGetCount_lock);
					synGetCount++;
					pthread_mutex_unlock(&synGetCount_lock);
					scanP = scanP + sizeof(struct syn_head);
				}
				else if(controlBuf[scanP] == 'S' && controlBuf[scanP + 1] == 'O' && controlBuf[scanP + 2] == 'K')
				{
#if PRINT
					printf("sok\n");
#endif
					page++;
					scanP = scanP + sizeof(struct sok_head);
				}
				else if(controlBuf[scanP] == 'W' && controlBuf[scanP + 1] == 'I' && controlBuf[scanP + 2] == 'N')
				{
					
					if(controlBufP - scanP - sizeof(struct win_head) >= sizeof(unsigned char))
					{
						windowLen = controlBuf[scanP + 3];

						scanP = scanP + sizeof(struct get_head) + sizeof(unsigned char);
#if PRINT
						printf("win %d\n", windowLen);
#endif
					}
					else 
						break;
				}

				else
					scanP++;
			}

			if(scanP == controlBufP)
			{
				controlBufP = 0;
				continue;
			}
			else if(scanP < controlBufP)
			{
				controlBufP -= scanP;
				memcpy(controlBuf, controlBuf + scanP, controlBufP);
			}

		}

	}
	controlChanThreadRunning = 0;
}

int findIndexInBuf(char *buf, int *start, int *end, int *datLen, u_int32_t index)
{
	int scanP = 0;
	struct load_head head;

	while(scanP + sizeof(struct load_head) < *datLen)
	{
		if(buf[scanP] == 'J' && buf[scanP + 1] == 'E' && buf[scanP + 2] == 'A' && buf[scanP + 3] == 'N')
		{
			memcpy(&head, buf + scanP, sizeof(struct load_head));
			if(index == head.index)
			{
				*start = scanP + sizeof(struct load_head);
				*end = scanP + head.length + sizeof(struct load_head);
				memcpy(buf + scanP, buf + scanP + head.length + sizeof(struct load_head), *datLen - scanP - head.length - sizeof(struct load_head));
				*datLen -= (sizeof(struct load_head) + head.length);
				return head.priority;
			}

			if(*datLen - scanP - sizeof(struct load_head) >= head.length)
				scanP = scanP + sizeof(struct load_head) + head.length;
			else
				break;
		}
		else
			scanP++;
	}

	return -1;

}

void* recvData(void *argc)
{
	recvThreadRunning = 1;

	int recvLen = 0;
	int recv_size = 0;
	int err = 0;
	socklen_t optlen = 0;
	char *retryData;
	u_int32_t lastIndex = 0;
	int pauseSign = 0;
	int recordLostNum = 0;
	int getLostNum = 0;

	pthread_detach(pthread_self());

	recvBufP = 0;
	recvProcessBufP = 0;

	optlen = sizeof(recv_size); 
	err = getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &recv_size, &optlen); 
	if(err < 0)
	{ 
		printf("Fail to get recbuf size\n"); 
	} 

	set_rec_timeout(1, 0);//(usec, sec)
	while(recvSign)
	{
		recvLen = 0;
		if(connectionStatus == P2P)
		{
			recvLen = recvfrom(sockfd, recvProcessBuf + recvProcessBufP, MAX_RECEIVE, 0, (struct sockaddr *)&recv_sin, &recv_sin_len);
		}
		else if(connectionStatus == TURN)
		{
			recvLen = recvfrom(sockfd, recvProcessBuf + recvProcessBufP, MAX_RECEIVE, 0, (struct sockaddr *)&recv_sin, &recv_sin_len);
		}
		else 
			break;

		if(recvLen <= 0)
		{
			continue;
		}

	    getNum += recvLen;
		recvProcessBufP += recvLen;

		if(recvProcessBufP > MAX_RECV_BUF - recv_size)
		{
			printf("recvBuf overflow!!\n");
			recvProcessBufP = 0;
		}

		if(pauseSign == 0)
		{
			recordLostNum = 0;
			getLostNum = 0;
		}

		if(getLostNum >= recordLostNum - 1)
			pauseSign = 0;

		if(pauseSign == 0 && recvProcessBackBufP >= 0)
		{
#if PRINT
			printf("pause status out!\n");
#endif
			memcpy(recvProcessBuf + recvProcessBufP, recvProcessBackBuf, recvProcessBackBufP);
			recvProcessBackBufP = 0;
		}

	    if(recvProcessBufP >= sizeof(struct load_head))
		{
			int scanP = 0;
			struct load_head head;
			struct get_head get;

			while(scanP + sizeof(struct load_head) < recvProcessBufP)
			{
				if(recvProcessBuf[scanP] == 'J' && recvProcessBuf[scanP + 1] == 'E' && recvProcessBuf[scanP + 2] == 'A' && recvProcessBuf[scanP + 3] == 'N')
				{
					int lostNum = 0;
					memcpy(&head, recvProcessBuf + scanP, sizeof(struct load_head));
					lostNum = head.index - lastIndex;

					if(pauseSign == 1 && lostNum == 1)
						getLostNum++;

					if(lostNum <= 0 && head.index != 0)
					{
						scanP += (head.length + sizeof(struct load_head));
						continue;
					}

					if(head.index != 0 && lostNum > 1)
					{
						if(recvProcessBackBufP != 0)
						{
						    int start, end;
						    int pri;
                            pri = findIndexInBuf(recvProcessBackBuf, &start, &end, &recvProcessBackBufP, lastIndex + 1);
						    if(pri >= 0)
						    {
								if(pri > 0)
								    sendGet(lastIndex + 1);
								lastIndex = lastIndex + 1;
								memcpy(recvBuf+recvBufP, recvProcessBackBuf + start, end - start);
								break;
						    }
						}
#if PRINT
						printf("lost pack happen, reget!!\n");
#endif
                        int i = 0;
						pauseSign = 1;

						if(recvProcessBackBufP + recvProcessBufP - scanP < MAX_RECV_BUF)
						{
						    memcpy(recvProcessBackBuf + recvProcessBackBufP, recvProcessBuf + scanP, recvProcessBufP - scanP);
						    recvProcessBackBufP = recvProcessBackBufP + (recvProcessBufP - scanP);
						    recvProcessBufP = 0;
						}
						else
						{
							printf("back buf overflow!!\n");
							recvProcessBackBufP = 0;
						}
	
						if(lostNum > recordLostNum)
							recordLostNum = lostNum;

						break;
					}

					lastIndex = head.index;
#if PRINT
					printf("load head: %c %d %d %d %d\n", head.logo[0], head.index,  head.priority, (unsigned int)head.length, head.sliceIndex);
#endif
					if(recvProcessBufP - scanP - sizeof(struct load_head) >= head.length)
					{
						if(head.priority > 0)
							sendGet(head.index);
				
					    if(head.length == 0)
						{
#if PRINT
							printf("Empty load!Actually lost.\n");
#endif
						}

						if(recvBufP + head.length > MAX_RECV_BUF)
						{
							printf("recv processed buf overflow!!\n");
							recvBufP = 0;
						}
						pthread_mutex_lock(&recvBuf_lock);
						memcpy(recvBuf + recvBufP, recvProcessBuf + scanP + sizeof(struct load_head), head.length);
						recvBufP += head.length;
						pthread_mutex_unlock(&recvBuf_lock);
						scanP = scanP + sizeof(struct load_head) + head.length;
					}
					else
						break;
				}
				else
					scanP++;
			}

			if(scanP == recvProcessBufP)
			{
				recvProcessBufP = 0;
				continue;
			}
			else if(scanP < recvProcessBufP)
			{
				recvProcessBufP -= scanP;
				memcpy(recvProcessBuf, recvProcessBuf + scanP, recvProcessBufP);
			}
		}
	}

	recvThreadRunning = 0;
}

int JEAN_init_master(int serverPort, int localPort, char *setIp)
{
	int  i;
	char Ctl_Rec[50];
	char Rec_W;
	int ret = 0;
	char Pole_ret = -1;
    
	recvSign = 1;
    recvBuf = (char*)malloc(MAX_RECV_BUF);
    recvProcessBuf = (char*)malloc(MAX_RECV_BUF);
    recvProcessBackBuf = (char*)malloc(MAX_RECV_BUF);


	if (pthread_mutex_init(&recvBuf_lock, NULL) != 0) 
	{
		printf("mutex init error\n");
		return -1;
	}

    if (pthread_mutex_init(&synGetCount_lock, NULL) != 0) 
	{
		printf("mutex init error\n");
		return -1;
	}

    if (pthread_mutex_init(&ring_lock, NULL) != 0) 
	{
		printf("mutex init error\n");
		return -1;
	}

    if (pthread_mutex_init(&send_lock, NULL) != 0) 
	{
		printf("mutex init error\n");
		return -1;
	}

	ret = local_net_init(localPort);
	if(ret < 0){
		printf("Local bind failed!!%d\n", ret);
		return ret;
	}

	ret = get_local_ip_port();
	if(ret < 0){
		printf("Geting local ip and port failed!!%d\n", ret);
		return ret;
	}

	ret = set_ip1_struct(setIp, serverPort);
	if(ret < 0){
		printf("Set ip1 failed!!%d\n", ret);
		return ret;
	}

	set_rec_timeout(0, 1);//(usec, sec)

	printf("------------------- Connection and user name verifying ---------------------\n");
	for(i = 0; i < MAX_TRY; i++){
		Send_VUAP();
		printf("Send uname and passwd\n");

		recvfrom(sockfd, Ctl_Rec, sizeof(Ctl_Rec), 0, (struct sockaddr *)&recv_sin, &recv_sin_len);
		char result;
		result = Ctl_Rec[1];

		if(Ctl_Rec[0] == V_RESP){
			printf("Receive ctl_w = %d result = %d\n", Rec_W, result);
			if(result == 1){
				printf("Verify and regist success!\n");
				break;
			}
			else if(result == 3){
				printf("Verify success but regist failed. Maybe node already exists!\n");
				break;
			}
			else if(result == 2){ 
				printf("Verify failed!\n");
				return WRONG_VERIFY;
			}
		}
	}

	if(i >= MAX_TRY) return OUT_TRY;
	
	ret = pthread_create(&keep_connection, NULL, Keep_con, NULL);
	if (ret != 0)
		printf("can't create thread: %s\n", strerror(ret));

	clean_rec_buff();
	printf("------------------ Wait for slave IP!-------------------\n");

	int slaver_act = 0;
	while(!slaver_act){
		recvfrom(sockfd, Ctl_Rec, sizeof(Ctl_Rec), 0, (struct sockaddr *)&recv_sin, &recv_sin_len);
		Rec_W = Ctl_Rec[0];

		if(Rec_W == S_IP){
			slaver_act = 1;
			memset(&slave_sin, 0, sizeof(slave_sin));
			memcpy(&slave_sin, Ctl_Rec + 1, sizeof(struct sockaddr_in));
			printf("Get slave IP info! Slave IP is %s\n", inet_ntoa(slave_sin.sin_addr));

			for(i = 0; i < MAX_TRY + 1 ; i++){
				Send_CMD(GET_REQ, 0x08);
				char result = 0;

				recvfrom(sockfd, Ctl_Rec, sizeof(Ctl_Rec), 0, (struct sockaddr *)&recv_sin, &recv_sin_len);
				if(Ctl_Rec[0] == GET_REQ && Ctl_Rec[1] == 0x9) break;
			}

			if(i >= MAX_TRY + 1) return OUT_TRY;

		}
	}

	clean_rec_buff();
	printf("------------------ Wait for slave to establish connection!-------------------\n");
	while(Pole_ret == -1){
		memset(Ctl_Rec, 0, 50);

		recvfrom(sockfd, Ctl_Rec, sizeof(Ctl_Rec), 0, (struct sockaddr *)&recv_sin, &recv_sin_len);
		Rec_W = Ctl_Rec[0];

		switch(Rec_W){
			case POL_REQ:
				printf("Get pole request!\n");
				Send_CMD_TO_SLAVE(GET_REQ, 0x0a);
			break;

			case CON_ESTAB:
				pole_res = Ctl_Rec[1];
				commonKey = Ctl_Rec[2] || (Ctl_Rec[3]<<8) || (Ctl_Rec[4]<<16);
				Send_CMD(GET_REQ, 0x14);
				printf("Pole result = %d, key = %d.\n", pole_res, commonKey);
				Pole_ret = pole_res;
#ifndef TEST_TURN
				if(Pole_ret == 1)
					connectionStatus = P2P;
				else
#endif
				{
					int i = 0;
					connectionStatus = TURN;
					for(i = 0; i < MAX_TRY + 1 ; i++){
						printf("send turn \n");
						Send_TURN();
						char result = 0;

						recvfrom(sockfd, Ctl_Rec, sizeof(Ctl_Rec), 0, (struct sockaddr *)&recv_sin, &recv_sin_len);
						if(Ctl_Rec[0] == GET_REQ) 
							break;
					}

					if(i >= MAX_TRY + 1) return OUT_TRY;

				}

				clean_rec_buff();
				for(i = 0; i < MAX_TRY + 1 ; i++){
					printf("require cmd channel open \n");
					Send_CMDOPEN();
					char result = 0;

					recvfrom(sockfd, Ctl_Rec, sizeof(Ctl_Rec), 0, (struct sockaddr *)&recv_sin, &recv_sin_len);
					if(Ctl_Rec[0] == GET_REQ) 
						break;
					sleep(1);
				}


				clean_rec_buff();
				for(i = 0; i < MAX_TRY + 1 ; i++){
					printf("require control channel open \n");
					Send_CONTROLOPEN();
					char result = 0;

					recvfrom(sockfd, Ctl_Rec, sizeof(Ctl_Rec), 0, (struct sockaddr *)&recv_sin, &recv_sin_len);
					if(Ctl_Rec[0] == GET_REQ) 
						break;
					sleep(1);
				}

				if(i >= MAX_TRY + 1) return OUT_TRY;

				if(-1 == init_CONTROL_CHAN())
				{   
					printf("control chan open failed\n");
					return -1;
				}

				controlSign = 1;

				if(controlChanThreadRunning == 0)
				{
					controlChanThreadRunning = 1;
					pthread_create(&control_t, NULL, controlChanThread, NULL);
				}

				if(recvThreadRunning == 0)
				{
					recvThreadRunning = 1;
					pthread_create(&recvDat_id, NULL, recvData, NULL);
				}

				initRing();	
				break;

			case M_POL_REQ:
				Send_CMD(GET_REQ, 0x12);
				printf("Get M_POL_REQ from server.\n");
				for(i = 0; i < MAX_TRY; i++){
					memset(Ctl_Rec, 0, 50);
					Send_POL(POL_REQ, &slave_sin);
					printf("Send POL_REQ to slave.\n");
					recvfrom(sockfd, Ctl_Rec, sizeof(Ctl_Rec), 0, (struct sockaddr *)&recv_sin, &recv_sin_len);
					if(Ctl_Rec[0] == GET_REQ && Ctl_Rec[1] == 0x0a){
						printf("Pole ok! Connection established.\n");
						Send_CMD(GET_REQ, 0x0e);
						break;
					}
				}
				if(i >= MAX_TRY){
					Send_CMD(GET_REQ, 0x0f);
					printf("Pole failed! Requiring slave mode.\n");
				}
				break;	

		}
	}
	sleep(2);
	return 0;
}

int JEAN_send_master(char *data, int len, unsigned char priority, unsigned char video_analyse, int video_head_len)
{
	int sendLen = 0;
    char *buffer;
	struct load_head lHead;
	NALU_t nalu;
	int tSendLen = 0;

	if(video_analyse >= 1)
	{
		GetAnnexbNALU (data + video_head_len, len - video_head_len, &nalu);
		switch(nalu.nal_unit_type)
		{
			case NALU_TYPE_IDR:
				priority = 7;
				break;
			case NALU_TYPE_SLICE:
			case NALU_TYPE_DPA:
			case NALU_TYPE_DPB:
			case NALU_TYPE_DPC:
				priority = 1;
				break;
			case NALU_TYPE_SEI:
			case NALU_TYPE_PPS:
			case NALU_TYPE_SPS:
				priority = 7;
				break;
			default:
				priority = 0;
		}
	}
	
	int sP = 0;
	int curLen = 0;
	int total = len;
	int count = 0;
	int macroParam = 0;
	pthread_mutex_lock(&send_lock);
	while(sP < len)
	{
		if(sP + MTU <= len)
			curLen = MTU;
		else
			curLen = len - sP;
		
		if(count * 100/total > 30)
			macroParam = 3;
		else if(count * 100/total > 50)
			macroParam = 1;
		else
			macroParam = 0;

		buffer = (char *)malloc(curLen + sizeof(struct load_head));
		memcpy(lHead.logo, "JEAN", 4);
		lHead.index = sendIndex;
		lHead.priority = (priority + macroParam)/2;
		lHead.length = curLen;
		lHead.direction = 1;
		lHead.address = sP;
		lHead.totalLen = total;
		lHead.sliceIndex = sliceIndex;

		memcpy(buffer, &lHead, sizeof(lHead));
		memcpy(buffer + sizeof(lHead), data + sP, curLen);

		sendLen = curLen + sizeof(lHead);
		reg_buff(sendIndex, buffer, priority, curLen + sizeof(lHead));

		count++;
		tSendLen += sendLen;
		sendIndex++;
		
		sP += MTU;
	}
	sliceIndex++;
	pthread_mutex_unlock(&send_lock);

	sendNum += tSendLen;
	return tSendLen;
}

int JEAN_recv_master(char *data, int len, unsigned char priority, unsigned char video_analyse)
{
	int recvLen = 0;
    unsigned long int waitTime = 0;

	while(recvBufP == 0 && waitTime < JEAN_recv_timeout * 10)
	{
		usleep(10);
		waitTime += 10;
	}

	if(recvBufP == 0)
		return 0;

	if(recvBufP < len)
	{
		memcpy(data, recvBuf, recvBufP);
		recvLen = recvBufP;
		recvBufP = 0;
	}
	else
	{
		memcpy(data, recvBuf, len);
		recvLen = len;

		pthread_mutex_lock(&recvBuf_lock);
        memcpy(recvBuf, recvBuf + len, recvBufP - len);
		pthread_mutex_unlock(&recvBuf_lock);

		recvBufP -= len;
	}

    return recvLen;
}

int JEAN_close_master()
{
	int i = 0;
	char Ctl_Rec[50];

	recvSign = 0;
	clean_rec_buff();
	sleep(1);
	set_rec_timeout(0, 1);//(usec, sec)
	for(i = 0; i < MAX_TRY + 1 ; i++){
		printf("send close \n");
		Send_CLOSE();
		char result = 0;

		recvfrom(sockfd, Ctl_Rec, sizeof(Ctl_Rec), 0, (struct sockaddr *)&recv_sin, &recv_sin_len);
		if(Ctl_Rec[0] == GET_REQ) 
			break;
	}

	if(i >= MAX_TRY + 1) return OUT_TRY;

	controlSign = 0;
	recvSign = 0;
	pthread_mutex_destroy(&recvBuf_lock);
	pthread_mutex_destroy(&synGetCount_lock);
	pthread_mutex_destroy(&ring_lock);
	pthread_mutex_destroy(&send_lock);
	close_CONTROL_CHAN();
	close_CMD_CHAN();
	free(recvBuf);
	free(recvProcessBuf);
	free(recvProcessBackBuf);
	emptyRing();
	close(sockfd);
	return 0;
}

