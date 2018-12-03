
#ifndef _OI_LOG_H
#define _OI_LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

typedef struct {
	FILE	*pLogFile;
    char    sBaseFileName[256];
    char    sLogFileName[512];
	int		iShiftType;		// 0 -> shift by size,  1 -> shift by LogCount, 2 -> shift by interval, 3 ->shift by day, 4 -> shift by hour, 5 -> shift by min
	int		iMaxLogNum; 	// 日志文件数量
	long	lMaxSize;
	long	lMaxCount;
	long	lLogCount;
	time_t	lLastShiftTime;
} LogFile;

#ifdef __cplusplus
extern "C"
{
#endif

int InitLogFile(LogFile* pstLogFile, char* sLogBaseName, long iShiftType, long iMaxLogNum, long iMAX);

int Log(LogFile* pstLogFile, int iLogTime, const char* sFormat, ...);
int LogWTime(LogFile* pstLogFile, struct timeval *pstNowTime, int iLogTime, const char* sFormat, ...);
int Logv(LogFile* pstLogFile, const char* sFormat, va_list ap);

#ifdef __cplusplus
}
#endif

#endif
