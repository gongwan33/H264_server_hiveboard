#ifndef JEAN_P2P_H
#define JEAN_P2P_H

int JEAN_init_master(int serverPort, int localPort, char *setIp);
int JEAN_send_master(char *data, int len, unsigned char priority, unsigned char video_analyse, int video_head_len);
int JEAN_recv_master(char *data, int len, unsigned char priority, unsigned char video_analyse);
int JEAN_close_master();
int JEAN_init_slave(int setServerPort, int setLocalPort, char *setIp);
int JEAN_send_slave(char *data, int len, unsigned char priority, unsigned char video_analyse, int video_head_len);
int JEAN_recv_slave(char *data, int len, unsigned char priority, unsigned char video_analyse);
int JEAN_close_slave();
int init_CMD_CHAN();
int close_CMD_CHAN();
int send_cmd(char *data, int len);
int recv_cmd(char *data, int len);
void sendSyn();
void sendWin(unsigned char winlen);

#endif