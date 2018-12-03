#ifndef _GAME_TIME_H_
#define _GAME_TIME_H_

// 游戏时间，单例

#include <sys/time.h>
#include "define.h"
#include "singleton.h"
#include <time.h>

enum WEEK_DAY
{
    SUNDAY,
    MONDAY,
    TUESDAY,
    WENDNESDAY,
    THURDDAY,
    FRIDAY,
    SATURDAY,
};

#define MS_PER_SECOND   1000
#define SECONDS_OF_HOUR 3600
#define SECONDS_OF_DAY 86400
#define SECONDS_OF_WEEK  604800
#define MSSECONDS_OF_DAY  86400000

#define MAX_TIME_STR_LENGTH (40)

/** 简单说明:
 ** m_stCurrTv: 保存的是UTC时间          {tv_sec = 1463474017, tv_usec = 75768}
 ** m_stCurrTm: 保存的是本地化的日历时间,即加过时区偏移的
 ** 特别需要注意的是
 **     GetCurrSecond 这个函数返回的是 UTC 时间秒数
 **     GetCurrSec|GetCurrMin|GetCurrHour|GetCurrYear|GetCurrMonth|GetCurrDay 是返回本地化后的时间对应的 年月日时分秒,是对m_stCurrTm的操作.年需要加1900才能与现在对应
**/

//  tm结构参数详解
//  struct tm
//  {
//    int tm_sec;                   /* Seconds.     [0-60] (1 leap second) */
//    int tm_min;                   /* Minutes.     [0-59] */
//    int tm_hour;                  /* Hours.       [0-23] */
//    int tm_mday;                  /* Day.         [1-31] */
//    int tm_mon;                   /* Month.       [0-11] */
//    int tm_year;                  /* Year - 1900.  */
//    int tm_wday;                  /* Day of week. [0-6] */
//    int tm_yday;                  /* Days in year.[0-365] */
//    int tm_isdst;                 /* DST.         [-1/0/1]*/
//  
//  #ifdef  __USE_BSD
//    long int tm_gmtoff;           /* Seconds east of UTC.  */
//    __const char *tm_zone;        /* Timezone abbreviation.  */
//  #else
//    long int __tm_gmtoff;         /* Seconds east of UTC.  */
//    __const char *__tm_zone;      /* Timezone abbreviation.  */
//  #endif
//  };


class CGameTime : public TSingleton<CGameTime>
{
public:
    CGameTime();
    ~CGameTime() {}

    void UpdateTime();

    struct timeval* GetCurrTime( ){ return &m_stCurrTv; }
    time_t GetCurrSecond( ) { return m_stCurrTv.tv_sec; }

    // 当前秒过了多少毫秒
    uint16_t GetCurrMsInSec() { return (uint16_t)( m_stCurrTv.tv_usec / 1000 ); }

    // 获得时间戳ms
    uint64_t GetCurrTimeMs() { return m_ullCurrTmMs; }
    int GetCurrSec() { return m_stCurrTm.tm_sec; }
    int GetCurrMin() { return m_stCurrTm.tm_min; }
    int GetCurrHour() { return m_stCurrTm.tm_hour; }
    int GetCurrYear(){ return m_stCurrTm.tm_year; }
    int GetCurrMonth(){ return m_stCurrTm.tm_mon; }// 月份[0, 11]
    int GetCurrDay(){ return m_stCurrTm.tm_mday; } // 一月中的天数 [1, 31]

    // 通过日时，获取每日更新下一间隔的时间
    int GetIntervalSecByHour(int iHour);	// Sec
    int GetIntervalMsByHour(int iHour);	// Ms

    struct tm* GetCurrTm(){ return &m_stCurrTm; }

    //判断是否跨天
    bool IsDayAlternate( time_t lNowTime, time_t lLastTime );

    //到明天零点还有多少秒
    time_t GetSecToNextDayBreak();

    //第二天零点的时间戳
    time_t GetSecToNextDay(time_t lSomeTime);

    // 获得当天开始时间基准点
    time_t GetBeginOfDay() { return m_lBegineOfDay; }

    // 获得给定时间戳的当天开始时间基准点
    time_t GetBeginOfDayInSomeDay(time_t lSomeTime);

    // 获得当天具体小时的秒计数
    time_t GetSecOfHourInCurrDay(float fHourInCurrDay);

    // 获得刚好比当前时间大的小时的时刻
    time_t GetSecOfHourBigThanCurr(float fHourInDay);

    //获取本周某天某时的秒数(参数WeekDay表示星期几，取值范围1-7)
    time_t GetSecByWeekDay(int iWeekDay, int iHour);

    //获取上周某天某时的秒数(参数WeekDay表示星期几，取值范围1-7)
    time_t GetSecByLastWeekDay(int iWeekDay, int iHour);

    //获取给定时间戳当天基于某小时的24小时循环周期的所处周期的起始时间
    time_t GetSecOfCycleHourInSomeDay(time_t lSomeTime, int iHour);

    //获取当前时间戳基于某小时的24小时循环周期的所处周期的起始时间
    time_t GetSecOfCycleHourInCurrday(int iHour);

	//获取当前时间戳基于某秒的24小时循环周期的所处周期的起始时间
	time_t GetSecOfCycleSecInCurrDay(int iSec);

    //获取给定时间戳当日的某小时的时间戳
    time_t GetSecOfGivenTime(time_t lSomeTime, int iHour);

    //获取指定日期的星期数
    uint8_t GetDayOfWeekBySomeDay(int iYear, int iMonth, int iDay);

    bool IsInSameWeek( time_t lTime1, time_t lTime2 );

    struct tm* GetSysTime();
    uint32_t GetCurDayOfWeek();
    bool IsInSameDay(time_t lTime1, time_t lTime2);

    //*** IsContain系列函数都采用 左闭右开的方式判断包含关系
    //可使NowTime==StartTime时也生效.否则,会造成 在下一次判断才会生效,导致结果不是预期的.
    bool IsContain(struct tm* curTm, struct tm* startTm,  struct tm* endTm);
    bool IsContainCur(struct tm* startTm,  struct tm* endTm);
    bool IsContain(uint64_t ullTmMs, uint64_t ullStartTmMs, uint64_t ullEndTmMs);
    bool IsContainCur(uint64_t ullStartTm, uint64_t ullEndTm);
    bool IsNeedUpdateByHour(uint64_t ullOfflineTmMs, int iHour);
    bool IsNeedUpdateByHour(uint64_t ullOfflineTmMs, int iHour, OUT uint64_t& ullUpdateTmMs);

    const static char* m_pszDefaultTimeFmt;
    const char* GetDefaultTmFmt() { return m_pszDefaultTimeFmt;}

	const static char* m_pszTimeFmtNew;

    int ConvStrToTime(const char* pszTimeStr, time_t& rTime, const char* pszTimeFmt = NULL);
    int ConvStrToDate(const char* pszTimeStr, tm& rDate, const char* pszTimeFmt = NULL);

	void GetTimeFmtStr(OUT char*pszTimeStr);

private:
    void _SetUtcTimeOff();
    int _GetWeekNum( time_t lTime );

private:
    struct timeval 	m_stCurrTv;     //这个保存的是UTC时间
    struct tm      	m_stCurrTm;     //本地化日历时间
    uint64_t		m_ullCurrTmMs;
    time_t		   	m_lBegineOfDay; // 当天的基准时间( utc )
    int				m_iUtcOff; // 和UTC时区之间的差值
    time_t m_Time;
    tm* m_pTm;
};

#endif

