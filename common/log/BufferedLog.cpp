#include "BufferedLog.h"

int	CBufferedLog::Init(const char *sPathName,const char *sAppName,int iShiftType,int iMaxLogNum,
							long lVal,int iLogMask)
{
	STBufferedLog	*pstBufferedLog = NULL;
	std::string		strFileName;

	if ((iLogMask & LOGTRACE) 
		&& (m_stBufferedLogArray.find(LOGTRACE) == m_stBufferedLogArray.end()))
	{
		pstBufferedLog = new STBufferedLog();
		strFileName = std::string(sPathName) + "/" + sAppName + "_trace";

		::InitLogFile(&pstBufferedLog->m_stLogFile,const_cast<char*>(strFileName.c_str()),
						iShiftType,iMaxLogNum,lVal);

		m_stBufferedLogArray[LOGTRACE] = pstBufferedLog;
	}

	if ((iLogMask & LOGRUN)
		&& (m_stBufferedLogArray.find(LOGRUN) == m_stBufferedLogArray.end()))
	{
		pstBufferedLog = new STBufferedLog();
		strFileName = std::string(sPathName) + "/" + sAppName + "_run";
		::InitLogFile(&pstBufferedLog->m_stLogFile,const_cast<char*>(strFileName.c_str()),
						iShiftType,iMaxLogNum,lVal);

		m_stBufferedLogArray[LOGRUN] = pstBufferedLog;
	}

	if ((iLogMask & LOGERR)
		&& (m_stBufferedLogArray.find(LOGERR) == m_stBufferedLogArray.end()))
	{
		pstBufferedLog = new STBufferedLog();
		strFileName = std::string(sPathName) + "/" + sAppName + "_err";
		::InitLogFile(&pstBufferedLog->m_stLogFile,const_cast<char*>(strFileName.c_str()),
						iShiftType,iMaxLogNum,lVal);

		m_stBufferedLogArray[LOGERR] = pstBufferedLog;
	}

	if ((iLogMask & LOGWARN)
		&& (m_stBufferedLogArray.find(LOGWARN) == m_stBufferedLogArray.end()))
	{
		pstBufferedLog = new STBufferedLog();
		strFileName = std::string(sPathName) + "/" + sAppName + "_warn";
		::InitLogFile(&pstBufferedLog->m_stLogFile,const_cast<char*>(strFileName.c_str()),
						iShiftType,iMaxLogNum,lVal);

		m_stBufferedLogArray[LOGWARN] = pstBufferedLog;
	}

	if ((iLogMask & LOGCORE)
		&& (m_stBufferedLogArray.find(LOGCORE) == m_stBufferedLogArray.end()))
	{
		pstBufferedLog = new STBufferedLog();
		strFileName = std::string(sPathName) + "/" + sAppName + "_core";
		::InitLogFile(&pstBufferedLog->m_stLogFile,const_cast<char*>(strFileName.c_str()),
						iShiftType,iMaxLogNum,lVal);

		m_stBufferedLogArray[LOGCORE] = pstBufferedLog;
	}

   if ((iLogMask & LOGSTATS)
		&& (m_stBufferedLogArray.find(LOGSTATS) == m_stBufferedLogArray.end()))
	{
		pstBufferedLog = new STBufferedLog();
		strFileName = std::string(sPathName) + "/" + sAppName + "_stats";
		::InitLogFile(&pstBufferedLog->m_stLogFile,const_cast<char*>(strFileName.c_str()),
						iShiftType,iMaxLogNum,lVal);

		m_stBufferedLogArray[LOGSTATS] = pstBufferedLog;
	}

    // init log mutexes
    if( !m_oLogCoreMutex.Init() ||
        !m_oLogErrMutex.Init()  ||
        !m_oLogRunMutex.Init()  ||
        !m_oLogStatsMutex.Init()||
        !m_oLogTraceMutex.Init()||
        !m_oLogWarnMutex.Init() )
    {
        return -1;
    }
        
	return	m_stBufferedLogArray.size();
}

int	CBufferedLog::SetLogBuffLen(int iLogType, int iBufferLen)
{
	hash_map_t<int,STBufferedLog*>::iterator iter;

	if ((iter = m_stBufferedLogArray.find(iLogType)) == m_stBufferedLogArray.end())
		return	-1;

	STBufferedLog *pstBufferedLog = iter->second;

	return	pstBufferedLog->SetBuffLen(iBufferLen);
}

const char *CBufferedLog::GetCurDateTimeStr(timeval *pstNowTime)
{
	struct timeval stLogTv;
	static char sTimeStr[100 + 1] = {0};

    if (!pstNowTime)
    {
	    gettimeofday(&stLogTv,NULL);
    }
    else
    {
        stLogTv = *pstNowTime;
    }

	snprintf(sTimeStr,100,"[%s.%.6ld] ",
				DateTimeStr(&(stLogTv.tv_sec)),(long)stLogTv.tv_usec);

	return	sTimeStr;
}

int	CBufferedLog::LogReally(int	iLogType,const char *sFormat,va_list &ap,timeval *pstNowTime)
{
	hash_map_t<int,STBufferedLog*>::iterator	iter;

	if ((iter = m_stBufferedLogArray.find(iLogType)) == m_stBufferedLogArray.end())
		return	-1;

	STBufferedLog *pstBufferedLog = iter->second;

	vsnprintf(m_sTempBuff,BUFFERED_TEMP_LEN,sFormat,ap);

	// 无buffer缓存类型，直接write
	if (!m_bBuffedSwitchEnable || pstBufferedLog->m_pcLogBuff == NULL)
        return  ::LogWTime(&pstBufferedLog->m_stLogFile,pstNowTime,2,"%s",m_sTempBuff);

	int			iLogStrLen = strlen(m_sTempBuff);
	const char	*sTimeStr = GetCurDateTimeStr(pstNowTime);
	int			iTimeStrLen = strlen(sTimeStr);

	if (pstBufferedLog->m_iLogBuffUsedLen + iTimeStrLen + iLogStrLen + 1 
			< pstBufferedLog->m_iLogBuffLen)
	{
		if (pstBufferedLog->m_iLogBuffUsedLen == 0)
			strcpy(pstBufferedLog->m_pcLogBuff,sTimeStr);
		else
			strcat(pstBufferedLog->m_pcLogBuff,sTimeStr);

		pstBufferedLog->m_iLogBuffUsedLen += iTimeStrLen;

		strcat(pstBufferedLog->m_pcLogBuff,m_sTempBuff);
		pstBufferedLog->m_iLogBuffUsedLen += iLogStrLen;

		strcat(pstBufferedLog->m_pcLogBuff,"\n");
		pstBufferedLog->m_iLogBuffUsedLen += 1;

		return	0;
	}

	if (pstBufferedLog->m_iLogBuffUsedLen == 0)
		return	::LogWTime(&pstBufferedLog->m_stLogFile,pstNowTime,2,"%s",m_sTempBuff);

	pstBufferedLog->m_iLogBuffUsedLen = 0;

	return	::LogWTime(&pstBufferedLog->m_stLogFile,pstNowTime,3,"%s%s%s",pstBufferedLog->m_pcLogBuff,
								sTimeStr,m_sTempBuff);
}

int	CBufferedLog::LogRun(const char * sFormat,...)
{
	int		iRet;

	va_list	ap;
	va_start(ap,sFormat);

	iRet = LogReally(LOGRUN,sFormat,ap);

	va_end(ap);

	return	iRet;
}

int CBufferedLog::LogRun(timeval * pstNowTime, const char * sFormat, ...)
{
    int     iRet;

    va_list ap;
    va_start(ap,sFormat);

    iRet = LogReally(LOGRUN,sFormat,ap,pstNowTime);

    va_end(ap);

    return  iRet;
}

int	CBufferedLog::LogErr(const char * sFormat,...)
{
	int		iRet;

	va_list	ap;
	va_start(ap,sFormat);
	
	iRet = LogReally(LOGERR,sFormat,ap);

	va_end(ap);

	return	iRet;
}

int CBufferedLog::LogErr(timeval * pstNowTime, const char * sFormat, ...)
{
	int		iRet;

	va_list	ap;
	va_start(ap,sFormat);
	
	iRet = LogReally(LOGERR,sFormat,ap,pstNowTime);

	va_end(ap);

	return	iRet;
}


int	CBufferedLog::LogWarn(const char * sFormat,...)
{
	int		iRet;

	va_list	ap;
	va_start(ap,sFormat);

	iRet = LogReally(LOGWARN,sFormat,ap);

	va_end(ap);

	return	iRet;
}

int CBufferedLog::LogWarn(timeval * pstNowTime, const char * sFormat, ...)
{
	int		iRet;

	va_list	ap;
	va_start(ap,sFormat);

	iRet = LogReally(LOGWARN,sFormat,ap,pstNowTime);

	va_end(ap);

	return	iRet;
}


int	CBufferedLog::LogTrace(const char * sFormat,...)
{
	int		iRet;

	va_list	ap;
	va_start(ap,sFormat);
	
	iRet = LogReally(LOGTRACE,sFormat,ap);

	va_end(ap);

	return	iRet;
}

int CBufferedLog::LogTrace(timeval * pstNowTime, const char * sFormat, ...)
{
	int		iRet;

	va_list	ap;
	va_start(ap,sFormat);
	
	iRet = LogReally(LOGTRACE,sFormat,ap,pstNowTime);

	va_end(ap);

	return	iRet;
}

void CBufferedLog::LogFlush()
{
	hash_map_t<int,STBufferedLog*>::iterator iter;

	for (iter = m_stBufferedLogArray.begin();
			iter != m_stBufferedLogArray.end();iter++)
	{
		iter->second->InfoFlush();
	}
}

int	CBufferedLog::LogCore(const char * sFormat,...)
{
	int		iRet;

	va_list	ap;
	va_start(ap,sFormat);

	iRet = LogReally(LOGCORE,sFormat,ap);

	va_end(ap);

	return	iRet;
}

int CBufferedLog::LogCore(timeval * pstNowTime, const char * sFormat, ...)
{
	int		iRet;

	va_list	ap;
	va_start(ap,sFormat);

	iRet = LogReally(LOGCORE,sFormat,ap,pstNowTime);

	va_end(ap);

	return	iRet;
}

int	CBufferedLog::LogStats(const char * sFormat,...)
{
	int		iRet;

	va_list	ap;
	va_start(ap,sFormat);

	iRet = LogReally(LOGSTATS,sFormat,ap);

	va_end(ap);

	return	iRet;
}

int CBufferedLog::LogStats(timeval * pstNowTime, const char * sFormat, ...)
{
	int		iRet;

	va_list	ap;
	va_start(ap,sFormat);

	iRet = LogReally(LOGSTATS,sFormat,ap,pstNowTime);

	va_end(ap);

	return	iRet;
}

CThreadMutex& CBufferedLog::GetLogMutex( int iType )
{
    switch( iType )
    {
        case LOGTRACE:  return m_oLogTraceMutex;
        case LOGRUN:    return m_oLogRunMutex;
        case LOGERR:    return m_oLogErrMutex;
        case LOGWARN:   return m_oLogWarnMutex;
        case LOGCORE:   return m_oLogCoreMutex;
        case LOGSTATS:  return m_oLogStatsMutex;
        default:
            assert( false );
            return m_oLogTraceMutex;
    }
}

