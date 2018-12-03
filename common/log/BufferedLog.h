#ifndef BUFFERED_LOG_IORI
#define BUFFERED_LOG_IORI


#include	<stdio.h>
#include	<limits.h>
#include	<string.h>
#include	<stdarg.h>
#include    <sys/time.h>
#include    <time.h>
#include    <string>
#include    "oi_log.h"
#include    "oi_misc.h"
#include    "define.h"
#include 	"../thread/ThreadMutex.h"


using namespace std;

// 提供App缓冲的Log类

class CBufferedLog
{
private:
    enum
    {
        BUFFERED_TEMP_LEN = 1024 * 4
    };

    struct  STBufferedLog
    {
        LogFile m_stLogFile;
        char    *m_pcLogBuff;
        int     m_iLogBuffLen;
        int     m_iLogBuffUsedLen;

        STBufferedLog():m_pcLogBuff(NULL),m_iLogBuffLen(0),
                                     m_iLogBuffUsedLen(0)
        {}

        ~STBufferedLog()
        {
            SpaceFree();
        }

        void SpaceFree()
        {
            if (m_pcLogBuff)
            {
                InfoFlush();

                delete  []m_pcLogBuff;
                m_pcLogBuff = NULL;
            }
        }

        void InfoFlush()
        {
            if (m_pcLogBuff && m_iLogBuffUsedLen > 0)
            {
                ::Log(&m_stLogFile,3,"%s",m_pcLogBuff);
                m_iLogBuffUsedLen = 0;
            }
        }

        int SetBuffLen(int iBuffLen)
        {
            if (iBuffLen <= 0)
                return  -1;

            SpaceFree();
            m_pcLogBuff = new char[iBuffLen];
            m_pcLogBuff[iBuffLen - 1] = 0;

            m_iLogBuffLen = iBuffLen;
            m_iLogBuffUsedLen = 0;

            return  0;
        }
    };    

    bool 	m_bBuffedSwitchEnable;
    char    m_sTempBuff[BUFFERED_TEMP_LEN + 1];
    hash_map_t<int,STBufferedLog*> m_stBufferedLogArray;

    int     LogReally(int iLogType, const char * sFormat, va_list & ap,timeval *pstNowTime = NULL);

public:
    enum
    {
        LOGTRACE = 0x01,
        LOGRUN = 0x02,
        LOGERR = 0x04,
        LOGWARN = 0x08,
        LOGCORE = 0x10,
        LOGSTATS = 0x20, // 用于统计的log
    };

    CBufferedLog():m_bBuffedSwitchEnable(true)
    {
        memset(m_sTempBuff,0,sizeof(m_sTempBuff));
    }

    ~CBufferedLog()
    {
        hash_map_t<int,STBufferedLog*>::iterator iter;
        for (iter = m_stBufferedLogArray.begin();iter != m_stBufferedLogArray.end();iter++)
            delete  iter->second;
    }

    int     Init(const char *sPathName,const char *sAppName, int iShiftType, int iMaxLogNum, 
                    long lVal,int iLogMask = LOGRUN|LOGERR|LOGWARN|LOGCORE|LOGSTATS);
    int     SetLogBuffLen(int iLogType,int iBufferLen);

    int     LogRun(const char *sFormat,...);
    int     LogRun(timeval *pstNowTime, const char *sFormat,...);
    
    int     LogErr(const char *sFormat,...);
    int     LogErr(timeval *pstNowTime,const char *sFormat,...);
    
    int     LogWarn(const char *sFormat,...);
    int     LogWarn(timeval *pstNowTime,const char * sFormat, ...);
    
    int     LogTrace(const char *sFormat,...);
    int     LogTrace(timeval *pstNowTime,const char * sFormat, ...);
    
    int     LogCore(const char *sFormat,...);
    int     LogCore(timeval *pstNowTime,const char * sFormat, ...);

	int     LogStats(const char *sFormat,...);
    int     LogStats(timeval *pstNowTime,const char * sFormat, ...);
	
    void    LogFlush(); // 将当前缓存的所有日志信息dump到对应文件中

    bool    IsBufferedEnabled()
    {
        return  m_bBuffedSwitchEnable;
    }
    
    void    DisableBuffered()
    {
        m_bBuffedSwitchEnable = false;
    }

    void    EnableBuffered()
    {
        m_bBuffedSwitchEnable = true;
    }
    
    const char* GetCurDateTimeStr(timeval * pstNowTime = NULL);

	CThreadMutex& GetLogMutex( int iType );

private:
	CThreadMutex m_oLogRunMutex;
	CThreadMutex m_oLogErrMutex;
	CThreadMutex m_oLogWarnMutex;
	CThreadMutex m_oLogTraceMutex;
	CThreadMutex m_oLogCoreMutex;
	CThreadMutex m_oLogStatsMutex;
};

#endif

