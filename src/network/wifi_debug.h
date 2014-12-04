/*
 * wifi_debug.h
 *  wifi debug message
 *  jgfntu@163.com
 *
 */

#ifndef __WIFI_DEBUG_H
#define __EIFI_DEBUG_H

//#define __WIFI_DEBUG_ON
//#define __CMD_DEBUG_ON
//#define __TEXT_DEBUG_ON

#ifdef __WIFI_DEBUG_ON
	#define wifi_dbg(fmt, args...) printf(fmt, ##args)
#else
	#define wifi_dbg(fmt,args...)
#endif

#ifdef __CMD_DEBUG_ON
	#define cmd_dbg(fmt, args...) printf(fmt, ##args)
#else
	#define cmd_dbg(fmt,args...)
#endif

#ifdef __TEXT_DEBUG_ON
	#define text_dbg(fmt, args...) printf(fmt, ##args)
#else
	#define text_dbg(fmt,args...)
#endif

#endif
