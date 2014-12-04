/*
 * protocol.h
 *  the wileless commuicate protocol
 *  jgfntu@163.com
 */

#ifndef __NETWORK_H
#define __NETWORK_H
void network(void);
void send_talk_end_resp(void);
void send_music_played_over(void);
void send_alarm_notify(u8 alarm_kind);
#define BLOWFISH
#ifdef BLOWFISH
#define BLOWFISH_KEY "-shanghai-hangzhou"
void BlowfishKeyInit(char* strKey,int nLen);
#endif

#endif

