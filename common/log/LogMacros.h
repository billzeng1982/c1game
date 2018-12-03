#ifndef _LOG_MACROS_H_
#define _LOG_MACROS_H_

#include <stdio.h>
#include <limits.h>
#include "singleton.h"
#include "BufferedLog.h"
#include "GameTime.h"
#include "NetLog.h"

#define LOG_FILE_SIZE 10485760 // 10M
#define LOG_BUFF_SIZE 32768 // 32K
#define LOG_NET_BUFF_SIZE 65535

#ifdef _DEBUG

#define TRACE( _s_, ... ) do{\
	char szMsg[1024];   \
	snprintf( szMsg, sizeof(szMsg), _s_, ##__VA_ARGS__ ); \
	printf( "[TRACE]%s...[%s:%s():%d]\n", szMsg, __FILE__, __FUNCTION__, __LINE__ ); \
}while(0)

#else

#define TRACE( _s_, ... ) 

#endif

//Send log to tlogd
#define LOGSEND( _LogName_, pstLog, iLen ) do {\
	CNetLog::Instance().SendLog2Net( _LogName_, pstLog, iLen );\
}while(0)



// 记录错误日志
#define     LOGERR( _s_, ... ) do{\
	char    szMsg[1024];   \
	snprintf(szMsg,sizeof(szMsg), _s_, ##__VA_ARGS__); \
	timeval stNowTime = *CGameTime::Instance().GetCurrTime();\
	CSingleton<CBufferedLog>::Instance().LogErr(&stNowTime,"%s...[%s:%s():%d]",\
												szMsg,__FILE__,__FUNCTION__,__LINE__); \
	printf( "\033[0;31;40m[ERROR]%s...[%s:%s():%d]\033[0m\n", szMsg,__FILE__,__FUNCTION__,__LINE__ ); \
}while(0)


// 记录运行日志
#define     LOGRUN( _s_, ... ) do{\
	char    szMsg[1024];   \
	snprintf(szMsg,sizeof(szMsg), _s_, ##__VA_ARGS__); \
	timeval stNowTime = *CGameTime::Instance().GetCurrTime();\
	CSingleton<CBufferedLog>::Instance().LogRun(&stNowTime,"%s...[%s:%s():%d]",\
												szMsg,__FILE__,__FUNCTION__,__LINE__);\
	printf( "[RUN]%s...[%s:%s():%d]\n", szMsg,__FILE__,__FUNCTION__,__LINE__ ); \
}while(0)


// 记录警告日志
#define     LOGWARN( _s_, ... ) do{\
	char    szMsg[1024];   \
	snprintf(szMsg,sizeof(szMsg), _s_, ##__VA_ARGS__); \
	timeval stNowTime = *CGameTime::Instance().GetCurrTime();\
	CSingleton<CBufferedLog>::Instance().LogWarn( &stNowTime, "%s...[%s:%s():%d]",\
												 szMsg,__FILE__,__FUNCTION__,__LINE__); \
	printf( "\033[0;33;40m[WARN]%s...[%s:%s():%d]\033[0m\n", szMsg,__FILE__,__FUNCTION__,__LINE__ ); \
}while(0)


#define   	LOGCORE( _s_, ... ) do{\
	char    szMsg[1024];   \
	snprintf(szMsg,sizeof(szMsg), _s_, ##__VA_ARGS__); \
	timeval stNowTime = *CGameTime::Instance().GetCurrTime();\
	CSingleton<CBufferedLog>::Instance().LogCore(&stNowTime,"%s...[%s:%s():%d]",\
												 szMsg,__FILE__,__FUNCTION__,__LINE__); \
	printf( "\033[0;33;40m[CORE]%s...[%s:%s():%d]\033[0m\n", szMsg,__FILE__,__FUNCTION__,__LINE__ ); \
}while(0)


// 记录统计日志
#define LOGSTATS( _s_, ... )  do{\
	char    szMsg[1024];   \
	snprintf(szMsg,sizeof(szMsg), _s_, ##__VA_ARGS__); \
	timeval stNowTime = *CGameTime::Instance().GetCurrTime();\
	CSingleton<CBufferedLog>::Instance().LogStats(&stNowTime,"%s", szMsg); \
	printf( "\033[0;32;40m[STATS]%s\033[0m\n", szMsg); \
}while(0)

/*-----------------------------------------------------------------------------------*/
/* 线程安全版本 r: re-entrant */

#ifdef _DEBUG

#define TRACE_r( _s_, ... ) do{\
	CThreadMutexGuard lock( CSingleton<CBufferedLog>::Instance().GetLogMutex(CBufferedLog::LOGTACE) ); \
	char szMsg[1024];   \
	snprintf( szMsg, sizeof(szMsg), _s_, ##__VA_ARGS__ ); \
	printf( "[TRACE]%s...[%s:%s():%d]\n", szMsg, __FILE__, __FUNCTION__, __LINE__ ); \
}while(0)

#else

#define TRACE_r( _s_, ... ) 

#endif


// 记录错误日志
#define     LOGERR_r( _s_, ... ) do{\
	CThreadMutexGuard lock( CSingleton<CBufferedLog>::Instance().GetLogMutex(CBufferedLog::LOGERR) ); \
	char    szMsg[1024];   \
	snprintf(szMsg,sizeof(szMsg), _s_, ##__VA_ARGS__); \
	timeval stNowTime = *CGameTime::Instance().GetCurrTime();\
	CSingleton<CBufferedLog>::Instance().LogErr(&stNowTime,"%s...[%s:%s():%d]",\
												szMsg,__FILE__,__FUNCTION__,__LINE__); \
	printf( "\033[0;31;40m[ERROR]%s...[%s:%s():%d]\033[0m\n", szMsg,__FILE__,__FUNCTION__,__LINE__ ); \
}while(0)


// 记录运行日志
#define     LOGRUN_r( _s_, ... ) do{\
	CThreadMutexGuard lock( CSingleton<CBufferedLog>::Instance().GetLogMutex(CBufferedLog::LOGRUN) ); \
	char    szMsg[1024];   \
	snprintf(szMsg,sizeof(szMsg), _s_, ##__VA_ARGS__); \
	timeval stNowTime = *CGameTime::Instance().GetCurrTime();\
	CSingleton<CBufferedLog>::Instance().LogRun(&stNowTime,"%s...[%s:%s():%d]",\
												szMsg,__FILE__,__FUNCTION__,__LINE__);\
	printf( "[RUN]%s...[%s:%s():%d]\n", szMsg,__FILE__,__FUNCTION__,__LINE__ ); \
}while(0)


// 记录警告日志
#define     LOGWARN_r( _s_, ... ) do{\
	CThreadMutexGuard lock( CSingleton<CBufferedLog>::Instance().GetLogMutex(CBufferedLog::LOGWARN) ); \
	char    szMsg[1024];   \
	snprintf(szMsg,sizeof(szMsg), _s_, ##__VA_ARGS__); \
	timeval stNowTime = *CGameTime::Instance().GetCurrTime();\
	CSingleton<CBufferedLog>::Instance().LogWarn( &stNowTime, "%s...[%s:%s():%d]",\
												 szMsg,__FILE__,__FUNCTION__,__LINE__); \
	printf( "\033[0;33;40m[WARN]%s...[%s:%s():%d]\033[0m\n", szMsg,__FILE__,__FUNCTION__,__LINE__ ); \
}while(0)


#define LOGCORE_r( _s_, ... ) do{\
	CThreadMutexGuard lock( CSingleton<CBufferedLog>::Instance().GetLogMutex(CBufferedLog::LOGCORE) ); \
	char    szMsg[1024];   \
	snprintf(szMsg,sizeof(szMsg), _s_, ##__VA_ARGS__); \
	timeval stNowTime = *CGameTime::Instance().GetCurrTime();\
	CSingleton<CBufferedLog>::Instance().LogCore(&stNowTime,"%s...[%s:%s():%d]",\
												 szMsg,__FILE__,__FUNCTION__,__LINE__); \
	printf( "\033[0;33;40m[CORE]%s...[%s:%s():%d]\033[0m\n", szMsg,__FILE__,__FUNCTION__,__LINE__ ); \
}while(0)


// 记录统计日志
#define LOGSTATS_r( _s_, ... )  do{\
	CThreadMutexGuard lock( CSingleton<CBufferedLog>::Instance().GetLogMutex(CBufferedLog::LOGSTATS) ); \
	char    szMsg[1024];   \
	snprintf(szMsg,sizeof(szMsg), _s_, ##__VA_ARGS__); \
	timeval stNowTime = *CGameTime::Instance().GetCurrTime();\
	CSingleton<CBufferedLog>::Instance().LogStats(&stNowTime,"%s", szMsg); \
	printf( "\033[0;32;40m[STATS]%s\033[0m\n", szMsg); \
}while(0)

#endif

