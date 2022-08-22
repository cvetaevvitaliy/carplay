/*
 * =====================================================================================\
 *
 *       Filename:  debug.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/22/2013 05:12:49 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  AVNC&IS
 *\
 * =====================================================================================\
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>

#define DEBUG_MODULE_ENABLED

typedef struct _DEBUG_INFO_
{
	const char * name;
} DEBUG_INFO;

#define UNUSED(...)     (void *)__VA_ARGS__

#define DEBUG_ERR(...)  fprintf(stderr, "!!! %s ERROR:", __FUNCTION__);fprintf(stderr, __VA_ARGS__)

#define DEBUG_TIME_BEGIN() \
{ \
	struct timeval _start, _current, _result; \
	printf("**********TIME MEASURING BEGIN**********\n"); \
	gettimeofday(&_start, NULL);

#define DEBUG_TIME_DUR() \
	gettimeofday(&_current, NULL); \
	timersub(&_current, &_start, &_result); \
	_start  =   _current; \
	{ \
		static float _max = 0, _min = 10000, _avg = 0; \
		static int _count = 0; \
		float _tmp; \
		_tmp    =   _result.tv_sec + (float)_result.tv_usec / 1000000; \
		if(_tmp > _max) _max =   _tmp; \
		if(_tmp < _min) _min =   _tmp; \
		_avg    +=  (_tmp - _avg) / ( ++ _count); \
		fprintf(stdout, "***MAX:%03fs MIN:%03fs AVG:%03fs\n", _max, _min, _avg); \
	}

#define DEBUG_TIME_END() \
	printf("***********TIME MEASURING END***********\n"); \
}

#ifdef DEBUG_MODULE_ENABLED

#define DEBUG_MODULE_INITIALIZER(module)    {#module}
#define DEBUG_MODULE(module)    static DEBUG_INFO debug_info = DEBUG_MODULE_INITIALIZER(module)
#define DEBUG_MSG(...)  \
{\
	char * msg_modules = getenv("msg_modules");\
\
	if(msg_modules != NULL)\
	{\
		int msg_enabled = strstr(msg_modules, debug_info.name) != NULL;\
\
		if(msg_enabled)\
		{fprintf(stdout, "*** ");fprintf(stdout, __VA_ARGS__);}\
	}\
}
#define DEBUG_LINE()    fprintf(stderr, "!!!%s:%d %s\n", __FILE__, __LINE__, __FUNCTION__)
#else
#define DEBUG_MODULE_INITIALIZER(module)
#define DEBUG_MODULE(module)
#define DEBUG_MSG(...)
#define DEBUG_LINE()
#endif

#define DEBUG_BACKTRACE() \
{ \
	void * _array[20]; \
	size_t _size; \
	char ** _strings; \
	int i; \
\
	_size   =   backtrace(_array, 20); \
	_strings    =   backtrace_symbols(_array, _size); \
\
	fprintf(stdout, "***stack depth %d\n", _size);\
	for(i = 0; i < _size; i ++) fprintf(stdout, "***%s\n", _strings[i]); \
\
	free(_strings);\
}

#define DEBUG_HEX_DISPLAY(D, L) \
{ \
	char _tmpChar[100] = {0}; \
	int _cc, i; \
\
	printf( "%lld\n",(long long int) (L) );\
	fprintf(stdout, "*******HEX DISPLAY BEGIN (%03lld bytes)******\n",(long long int) (L));\
\
	for(_cc = 0; _cc < (L); _cc += 16) \
	{ \
		memset(_tmpChar, 0, sizeof(_tmpChar)); \
\
		for(i = 0; i < 16 && i + _cc < (L); i ++) \
		{ \
			_tmpChar[i * 3]     =   ((D)[_cc + i] >> 4)["0123456789ABCDEF"]; \
			_tmpChar[i * 3 + 1] =   ((D)[_cc + i] & 0x0F)["0123456789ABCDEF"]; \
			_tmpChar[i * 3 + 2] =   ' '; \
		} \
\
		fprintf(stdout, "\t%s\n",  _tmpChar); \
	} \
	fprintf(stdout, "************HEX DISPLAY END*************\n");\
}

#define DEBUG_FUNCTION_FAILURE(r, e, f, ...) \
{ \
	int _ret	=	f(__VA_ARGS__); \
	if(_ret != r) \
	{ \
		fprintf(stderr, "%s %d %s failed(%d)\n", __FILE__, __LINE__, #f, _ret); \
		goto e; \
	} \
}

#define DEBUG_FUNCTION_REPORT(r, f, ...) \
{ \
	int _ret	=	f(__VA_ARGS__); \
	if(_ret != r) \
	{ \
		fprintf(stderr, "%s %d %s failed(%d)\n", __FILE__, __LINE__, #f, _ret); \
	} \
}

#if DEBUG_MESS
#define iAP2debug_mess(format, ...) printf(format, ##__VA_ARGS__)
#else
#define iAP2debug_mess(format, ...)
#endif

#if DEBUG_MESS_TIME
#define iAP2debug_mess_time(format, ...) \
		iAP2LogDbg(" (file:%s line:%d)\n",__FILE__,__LINE__); printf(format, ##__VA_ARGS__) 
#else
#define iAP2debug_mess_time(format, ...)
#endif

/*
#if DEBUG_MESS

#if DEBUG_MESS_TIME
#define iAP2debug_mess(format, ...) \
		iAP2LogDbg(" (line %d):",__LINE__); printf(format, ##__VA_ARGS__)
#else
#define iAP2debug_mess(format, ...) printf(format, ##__VA_ARGS__)
#endif

#else
#define iAP2debug_mess(format, ...)
#endif
*/
#endif
