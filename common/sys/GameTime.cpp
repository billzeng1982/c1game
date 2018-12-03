#include "GameTime.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

const char* CGameTime::m_pszDefaultTimeFmt = "%Y/%m/%d %H:%M:%S";
const char* CGameTime::m_pszTimeFmtNew = "%Y-%m-%d %H:%M:%S";


CGameTime::CGameTime()
{
    bzero( &m_stCurrTv, sizeof(m_stCurrTv));
    bzero( &m_stCurrTm, sizeof(m_stCurrTm) );
    m_lBegineOfDay = 0;
    this->_SetUtcTimeOff();
}


void CGameTime::UpdateTime()
{
    time_t lLastTime = m_stCurrTv.tv_sec;

    gettimeofday(&m_stCurrTv, NULL);
    localtime_r(&m_stCurrTv.tv_sec, &m_stCurrTm);
    m_ullCurrTmMs = ((uint64_t)m_stCurrTv.tv_sec)*1000 + ((uint64_t)m_stCurrTv.tv_usec)/1000;

    if( lLastTime != m_stCurrTv.tv_sec &&
        this->IsDayAlternate(m_stCurrTv.tv_sec, lLastTime) )
    {
        // 跨天
        struct tm tTime = {0};
        tTime.tm_year = m_stCurrTm.tm_year;
        tTime.tm_mon = m_stCurrTm.tm_mon;
        tTime.tm_mday = m_stCurrTm.tm_mday;
        m_lBegineOfDay = mktime(&tTime);
    }
}

int CGameTime::GetIntervalSecByHour(int iHour)
{
    int iCurHour = GetCurrHour();
    int iDiffHour = iHour - iCurHour;

    if (iDiffHour <= 0)
    {
        iDiffHour = 24 + iDiffHour;
    }

    int iDiffSec = iDiffHour*3600 - GetCurrMin()*60 - GetCurrSec();

    return iDiffSec;
}

int CGameTime::GetIntervalMsByHour(int iHour)
{
    int iCurHour = GetCurrHour();
    int iDiffHour = iHour - iCurHour;

    if (iDiffHour <= 0)
    {
        iDiffHour = 24 + iDiffHour;
    }

    int iDiffMs = (iDiffHour*3600 - GetCurrMin()*60 - GetCurrSec())*1000 - m_ullCurrTmMs % 1000;
    return iDiffMs;
}

bool CGameTime::IsDayAlternate( time_t lNowTime, time_t lLastTime )
{
    if( 0 == lNowTime )
    {
        return false;
    }

    if( 0 == lLastTime )
    {
        return true;
    }

    if( lNowTime == lLastTime )
    {
        return false;
    }

    BYTE bNowDay = 0,bLastDay = 0;
    struct tm stNowTime;
    localtime_r(&lNowTime, &stNowTime);
    bNowDay = stNowTime.tm_mday;

    struct tm stLastTime;
    localtime_r(&lLastTime, &stLastTime);
    bLastDay = stLastTime.tm_mday;

    if( ( bNowDay == bLastDay ) && ( abs( lNowTime - lLastTime ) < SECONDS_OF_DAY ))
    {
        return false;
    }

    return true;
}


time_t CGameTime::GetSecToNextDayBreak()
{
    struct tm tmNew;
    memcpy(&tmNew, &m_stCurrTm, sizeof(tmNew));

    tmNew.tm_hour = 0;
    tmNew.tm_min = 0;
    tmNew.tm_sec = 0;

    time_t tNew = mktime(&tmNew);
    tNew += SECONDS_OF_DAY;

    return (tNew - m_stCurrTv.tv_sec);
}

time_t CGameTime::GetSecToNextDay(time_t lSomeTime)
{
    struct tm *tmNew = localtime(&lSomeTime);

    tmNew->tm_hour = 0;
    tmNew->tm_min = 0;
    tmNew->tm_sec = 0;

    time_t tNew = mktime(tmNew);
    tNew += SECONDS_OF_DAY;

    return (tNew - lSomeTime);
}

time_t CGameTime::GetSecOfHourInCurrDay(float fHourInCurrDay)
{
    if (fHourInCurrDay < 0)
    {
        fHourInCurrDay = 0;
    }
    else if (fHourInCurrDay > 24)
    {
        fHourInCurrDay = 24;
    }

    struct tm tmNew;
    memcpy(&tmNew, &m_stCurrTm, sizeof(tmNew));

    tmNew.tm_hour = 0;
    tmNew.tm_min = 0;
    tmNew.tm_sec = 0;

    time_t tNew = mktime(&tmNew);
    tNew += (int)(SECONDS_OF_HOUR * fHourInCurrDay);

    return tNew;
}

time_t CGameTime::GetSecOfHourBigThanCurr(float fHourInDay)
{
    if (fHourInDay < 0)
    {
        fHourInDay = 0;
    }
    else if (fHourInDay > 24)
    {
        fHourInDay = 24;
    }

    struct tm tmNew;
    memcpy(&tmNew, &m_stCurrTm, sizeof(tmNew));

    tmNew.tm_hour = 0;
    tmNew.tm_min = 0;
    tmNew.tm_sec = 0;

    time_t tNew = mktime(&tmNew);
    tNew += (int)(SECONDS_OF_HOUR * fHourInDay);

    int iCurHour = GetCurrHour();
    if (iCurHour >= fHourInDay)
    {
        // 取明天的时刻
        tNew += SECONDS_OF_DAY;
    }

    return tNew;
}

void CGameTime::_SetUtcTimeOff()
{
    time_t lNow = 0;
    time( &lNow );
    struct tm tmCurr = {0};
    localtime_r( &lNow, &tmCurr);
    m_iUtcOff = tmCurr.tm_gmtoff;
}

uint32_t CGameTime::GetCurDayOfWeek()
{
    uint64_t tempTime = this->GetCurrSecond() + m_iUtcOff;
    tempTime /= SECONDS_OF_DAY; //1970-1-1 星期四 -> 天数
    tempTime = (tempTime + 3) % 7 + 1;
    return (uint32_t)tempTime;
}

time_t CGameTime::GetSecByWeekDay(int iWeekDay, int iHour)
{
    int iCurWeekDay = GetCurDayOfWeek();
    int iDeltaTime = (iWeekDay - iCurWeekDay) * SECONDS_OF_DAY + iHour * SECONDS_OF_HOUR;

    if (iDeltaTime >= 0)
    {
        return m_lBegineOfDay + (time_t)iDeltaTime;
    }
    else
    {
        return m_lBegineOfDay - (time_t)(-iDeltaTime);
    }
}

time_t CGameTime::GetSecByLastWeekDay(int iWeekDay, int iHour)
{
    int iCurWeekDay = GetCurDayOfWeek();
    int iDeltaTime = (iWeekDay - iCurWeekDay) * SECONDS_OF_DAY + iHour * SECONDS_OF_HOUR;

    if (iDeltaTime >= 0)
    {
        return m_lBegineOfDay + (time_t)iDeltaTime - 7 * SECONDS_OF_DAY;
    }
    else
    {
        return m_lBegineOfDay - (time_t)(-iDeltaTime) - 7 * SECONDS_OF_DAY;
    }
}

bool CGameTime::IsInSameDay(time_t lTime1, time_t lTime2)
{
    //	这个需要加上时区偏移量来判断, 因为DayTime1是取整的,有误差
    uint64_t DayTime1 = (lTime1 + m_iUtcOff) / SECONDS_OF_DAY;
    uint64_t DayTime2 = (lTime2 + m_iUtcOff) / SECONDS_OF_DAY;

    if (DayTime1 == DayTime2)
    {
        return true;
    }

    return false;
}

// ( 天数 + 星期几) / 7 +1 = 第几周
int CGameTime::_GetWeekNum( time_t lTime )
{
    long lWeekNum = lTime + m_iUtcOff;
    lWeekNum /= SECONDS_OF_DAY;		// 算天数
    lWeekNum += 4;                  // 1970-1-1 星期四
    lWeekNum /= 7;                  // 每周7天
    lWeekNum += 1;                  // 第1天就是第一周
    return (int)lWeekNum;
}

//适用于仅能获得具体日期而没有time_t的情况如果有time_t则可以使用localtime()
uint8_t CGameTime::GetDayOfWeekBySomeDay(int iYear, int iMonth, int iDay)
{
    if (iMonth == 1 || iMonth == 12)
    {
        iMonth += 12;
        --iYear;
    }
    uint8_t bWeek = (iDay + 2 * iMonth + 3 * (iMonth + 1) / 5 + iYear + iYear / 4 - iYear / 100 + iYear / 400) % 7;
    //这里的bWeek = 0~6故应返回+1之后的正确星期数
    return bWeek + 1;
}

bool CGameTime::IsInSameWeek( time_t lTime1, time_t lTime2 )
{
    int iWeek1 = this->_GetWeekNum( lTime1 );
    int iWeek2 = this->_GetWeekNum( lTime2 );
    return ( iWeek1 == iWeek2 );
}

tm* CGameTime::GetSysTime()
{
    time(&m_Time);
    tm tmSys;
    m_pTm = localtime_r(&m_Time, &tmSys);
    return m_pTm;
}

bool CGameTime::IsContain(struct tm* curTm, struct tm* startTm,  struct tm* endTm)
{
    time_t curTime = mktime(curTm);
    time_t startTime = mktime(startTm);
    time_t endTime = mktime(endTm);
    if((curTime >= startTime) && (curTime < endTime))
    {
        return true;
    }

    return false;
}

bool CGameTime::IsContainCur(struct tm* startTm, struct tm* endTm)
{
    time_t startTime = mktime(startTm);
    time_t endTime = mktime(endTm);
    time_t curTime = GetCurrSecond();

    if((curTime >= startTime) && (curTime < endTime))
    {
        return true;
    }

    return false;
}

bool CGameTime::IsContainCur(uint64_t ullStartTm, uint64_t ullEndTm)
{
    uint64_t ullTm = (uint64_t) GetCurrSecond();
    if((ullTm >= ullStartTm) && (ullTm < ullEndTm))
    {
        return true;
    }

    return false;
}


bool CGameTime::IsContain(uint64_t ullTmMs, uint64_t ullStartTmMs, uint64_t ullEndTmMs)
{
    if((ullTmMs >= ullStartTmMs) && (ullTmMs < ullEndTmMs))
    {
        return true;
    }

    return false;
}

// 给定一个每日更新Hour，判断此Hour是否在当前时间到离线时间之间
bool CGameTime::IsNeedUpdateByHour(uint64_t ullOfflineTmMs, int iHour)
{
    uint64_t ullUpdateTmMs;

    return IsNeedUpdateByHour(ullOfflineTmMs, iHour, ullUpdateTmMs);
}

bool CGameTime::IsNeedUpdateByHour(uint64_t ullOfflineTmMs, int iHour, OUT uint64_t& ullUpdateTmMs)
{
    // 看更新Hour是今天还是昨天
    uint64_t ullDiffMs = GetIntervalMsByHour(iHour);

    // 上次应更新的时间
    ullUpdateTmMs = m_ullCurrTmMs + ullDiffMs - SECONDS_OF_DAY * 1000;

    //加1是为了让ullUpdateTmMs与ullOfflineTmMs不相等
    return IsContain(ullUpdateTmMs, ullOfflineTmMs + 1, m_ullCurrTmMs);
}


int CGameTime::ConvStrToTime(const char* pszTimeStr, time_t& rTime, const char* pszTimeFmt)
{
    if (!pszTimeFmt)
    {
        pszTimeFmt = m_pszDefaultTimeFmt;
    }

    struct tm stTm;
    bzero(&stTm, sizeof(tm));
    if(!strptime(pszTimeStr, pszTimeFmt, &stTm))
    {
        return -1;
    }

    rTime = mktime(&stTm);
    return 0;
}

int CGameTime::ConvStrToDate(const char* pszTimeStr, tm& rDate, const char* pszTimeFmt)
{
    if (!pszTimeFmt)
    {
        pszTimeFmt = m_pszDefaultTimeFmt;
    }

    if(!strptime(pszTimeStr, pszTimeFmt, &rDate))
    {
        return -1;
    }

    return 0;
}


time_t CGameTime::GetSecOfCycleHourInSomeDay(time_t lSomeTime, int iHour)
{
    struct tm tmSome;
    localtime_r(&lSomeTime, &tmSome);
    int iOldHour = tmSome.tm_hour;
    tmSome.tm_hour = iHour;
    tmSome.tm_min = 0;
    tmSome.tm_sec = 0;
    time_t lRetTime = mktime(&tmSome);
    if (iOldHour < iHour)
    {
        lRetTime -= SECONDS_OF_DAY;
    }
    return lRetTime;
}

time_t CGameTime::GetSecOfCycleSecInCurrDay(int iSec)
{
	time_t ullSomeTime = GetBeginOfDay() + iSec;
	if ( GetCurrSecond() < ullSomeTime )
	{
		ullSomeTime -= SECONDS_OF_DAY;
	}
	return ullSomeTime;
}

time_t CGameTime::GetBeginOfDayInSomeDay(time_t lSomeTime)
{
    struct tm tmSome;
    localtime_r(&lSomeTime, &tmSome);
    tmSome.tm_hour = 0;
    tmSome.tm_min = 0;
    tmSome.tm_sec = 0;



	return mktime(&tmSome);
}

time_t CGameTime::GetSecOfCycleHourInCurrday(int iHour)
{
    return GetSecOfCycleHourInSomeDay(GetCurrSecond(), iHour);
}

time_t CGameTime::GetSecOfGivenTime(time_t lSomeTime, int iHour)
{
    struct tm tmSome;
    localtime_r(&lSomeTime, &tmSome);
    int iOldHour = tmSome.tm_hour;
    tmSome.tm_hour = iHour;
    tmSome.tm_min = 0;
    tmSome.tm_sec = 0;
    time_t lRetTime = mktime(&tmSome);
    return lRetTime;
}

void CGameTime::GetTimeFmtStr(OUT char*pszTimeStr)
{
	strftime(pszTimeStr, MAX_TIME_STR_LENGTH, m_pszTimeFmtNew, this->GetCurrTm());
}



