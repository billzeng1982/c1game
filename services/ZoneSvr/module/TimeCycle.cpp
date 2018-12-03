#include "TimeCycle.h"
#include "GameTime.h"
#include "SvrTime.h"

using namespace PKGMETA;


bool TimeCycle::Init()
{
    return LoadGameData();
}


bool TimeCycle::LoadGameData()
{
    m_Usage4Pvp6v6Vector.clear();
    int iResNum = CGameDataMgr::Instance().GetResTimeCycleMgr().GetResNum();
    RESTIMECYCLE* poResTimeCycle = NULL;
    for (int i = 0 ; i < iResNum; i++)
    {
        poResTimeCycle = CGameDataMgr::Instance().GetResTimeCycleMgr().GetResByPos(i);
        if (!poResTimeCycle)
        {
            LOGERR(" poResTimeCycle<Pos=%d> is NULL", i);
            continue;
        }
        if (poResTimeCycle->m_dwUsageType == TIME_CYCLE_USER_MATCH_ELO)
        {
            m_Usage4Pvp6v6Vector.push_back(poResTimeCycle->m_dwId);
        }
    }
    RESBASIC* poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(COMMON_UPDATE_TIME);
    if (!poResBasic)
    {
        m_iDailyResetTime = 5;
    }
    m_iDailyResetTime = poResBasic->m_para[0];
    return true;
}

int TimeCycle::CheckTime(uint32_t dwTimeId, uint64_t ullPara, OUT uint64_t* pullStart, OUT uint64_t* pullEnd)
{
    RESTIMECYCLE* poResTimeCycle = CGameDataMgr::Instance().GetResTimeCycleMgr().Find(dwTimeId);
    if (!poResTimeCycle)
    {
        LOGERR("poResTimeCycle<%u> is NULL", dwTimeId);
        return ERR_NOT_FOUND;
    }
    //判断开服间隔时间，如值为非0， 就必须要满足开服多少秒后，才行。
    if (poResTimeCycle->m_dwSvrOpenDeltaTime != 0 &&
        SvrTime::Instance().GetOpenSvrTime() + poResTimeCycle->m_dwSvrOpenDeltaTime > (uint64_t)CGameTime::Instance().GetCurrSecond())
    {
        return ERR_NOT_START;
    }

    int iRet = 0;
    switch (poResTimeCycle->m_bCycleType)
    {
    case 0:
        //不循环
        iRet = _CheckNoCycle(poResTimeCycle, ullPara, pullStart, pullEnd);
        break;
    case 2:
        //按秒循环
        iRet = _CheckSecondCycle(poResTimeCycle, ullPara, pullStart, pullEnd);
        break;
//     case 1:
//         //按月循环
//         iRet = _CheckMonthCycle(poResTimeCycle, ullPara, pullStart, pullEnd);
        break;
    default:
        LOGERR("CycleType(%d) error", poResTimeCycle->m_bCycleType);
        iRet = ERR_SYS;
    }

    return iRet;
}

int TimeCycle::CheckTime4Pvp6v6()
{
    uint64_t ullStartTime, ullEndTime = 0;
    for (size_t i = 0; i < m_Usage4Pvp6v6Vector.size(); i++)
    {
        if (ERR_NONE == CheckTime(m_Usage4Pvp6v6Vector[i], 0, &ullStartTime, &ullEndTime))
        {
            return ERR_NONE;
        }
    }
    return ERR_NOT_START;
}

int TimeCycle::CheckTime(uint32_t dwTimeId)
{
    uint64_t ullStartTime, ullEndTime = 0;
    return CheckTime(dwTimeId, 0, &ullStartTime, &ullEndTime);
}

// 不循环,一次性
int TimeCycle::_CheckNoCycle(RESTIMECYCLE* poResTimeCycle, uint64_t ullPara, OUT uint64_t* pullStart,  OUT uint64_t* pullEnd)
{
    int iRet = this->_GetStartTime(poResTimeCycle, ullPara, pullStart);
    if (iRet != ERR_NONE)
    {
        return iRet;
    }

    *pullEnd = *pullStart + poResTimeCycle->m_dwDuration;

    if (!CGameTime::Instance().IsContainCur(*pullStart, *pullEnd))
    {
        return ERR_NOT_START;
    }

    return ERR_NONE;
}

// 循环,按秒循环, 起始时间为ullStartTime, 一个周期为(m_dwDuration + m_dwCycleTakt) 有效时间为 m_dwDuration, 无效时间是为 m_dwCycleTakt
int TimeCycle::_CheckSecondCycle(RESTIMECYCLE* poResTimeCycle, uint64_t ullPara, OUT uint64_t* pullStart, OUT uint64_t* pullEnd)
{
    int iRet = this->_GetStartTime(poResTimeCycle, ullPara, pullStart);
    if (iRet != ERR_NONE)
    {
        return iRet;
    }

    uint64_t ullCurSec = CGameTime::Instance().GetCurrSecond();
    if (ullCurSec < *pullStart)
    {
        return ERR_NOT_START;
    }

    int iCycleN = (ullCurSec - *pullStart) / (poResTimeCycle->m_dwDuration + poResTimeCycle->m_dwCycleTakt);
    *pullStart += iCycleN * (poResTimeCycle->m_dwDuration + poResTimeCycle->m_dwCycleTakt);
    *pullEnd = *pullStart + poResTimeCycle->m_dwDuration;

    if (!CGameTime::Instance().IsContainCur(*pullStart, *pullEnd))
    {
        return ERR_NOT_START;
    }

    return ERR_NONE;
}

// 按月循环:每个月同样的时间时间开启, 有效时间为Duration,之后关闭
int TimeCycle::_CheckMonthCycle(RESTIMECYCLE* poResTimeCycle, uint64_t ullPara, OUT uint64_t* pullStart , OUT uint64_t* pullEnd)
{
#if 0
    tm startTm = { 0 } ;
    localtime_r((time_t*)&poResTimeCycle->m_ullStartTime, &startTm);
    //  大于28号,循环会有问题,不好处理,1月跳到2月 怎么设置? 而且也没循环的必要了
    //      所以强制到28号
    if (startTm.tm_mday > 28)
    {
        startTm.tm_mday = 28;
    }

    if (startTm.tm_mon != CGameTime::Instance().GetCurrMonth())
    {
        if (startTm.tm_mon == 11 )    //这里11实际是12月份
        {
            startTm.tm_year ++ ;
            startTm.tm_mon = 0;
        }
        else
        {
            startTm.tm_mon ++;
        }
    }

    uint64_t ullCycleStartTime = (uint64_t)mktime(&startTm);
    uint64_t ullCycleEndTime = ullCycleStartTime + poResTimeCycle->m_dwDuration;
    if (!CGameTime::Instance().IsContainCur(ullCycleStartTime, ullCycleEndTime))
    {
        return -32;
    }

    if (NULL != pullStart && NULL != pullEnd)
    {
        *pullStart  = ullCycleStartTime;
        *pullEnd    = ullCycleEndTime;
    }
#endif
    return 0;
}

int TimeCycle::_GetStartTime(RESTIMECYCLE* poResTimeCycle, uint64_t ullPara, OUT uint64_t* pullStart)
{
    if (poResTimeCycle->m_bType == TYPE_OPEN_SVC_TIME)
    {
        *pullStart = SvrTime::Instance().GetOpenSvrTime() + poResTimeCycle->m_ullDeltaTime;
    }
    else if (poResTimeCycle->m_bType == TYPE_ANY_TIME)
    {
        time_t tStartTime;
        if (CGameTime::Instance().ConvStrToTime(poResTimeCycle->m_szStartTime, tStartTime) != 0)
        {
            LOGERR("poResTimeCycle(%u) Convert StartTime faild! ", poResTimeCycle->m_dwId);
            return ERR_SYS;
        }
        *pullStart = (uint64_t)tStartTime;
    }
    else if (poResTimeCycle->m_bType == TYPE_ANY_WEEK)
    {
        time_t tStartTime = CGameTime::Instance().GetSecByWeekDay(1, 0);
        *pullStart = (uint64_t)tStartTime + poResTimeCycle->m_ullDeltaTime;
    }
    else if (poResTimeCycle->m_bType == TYPE_USER_REGISTRY_TIME && ullPara != 0)
    {
        //ullPara 在这个类型表示玩家注册时间
        //计算的开始时间 是玩家注册时间所在的当天游戏通用刷新时间(即五点以前,是上一天的五点,五点以后则是今天的五点) 再加上 DelaTime 所表示的几个整天(多余的舍去)
        //uint64_t ullFirstLoginTime = ullPara;
        uint64_t ullNewInitTime = CGameTime::Instance().GetSecOfCycleHourInSomeDay(ullPara, m_iDailyResetTime);
        *pullStart = ullNewInitTime + poResTimeCycle->m_ullDeltaTime / SECONDS_OF_DAY * SECONDS_OF_DAY;
    }
    else if (poResTimeCycle->m_bType == TYPE_ANY_DAY)
    {
        *pullStart = CGameTime::Instance().GetBeginOfDay() + poResTimeCycle->m_ullDeltaTime;
    }
    else
    {
        LOGERR("poResTimeCycle(%u) Type(%d) error, ullPara<%lu>", poResTimeCycle->m_dwId, poResTimeCycle->m_bType, ullPara);
        return ERR_SYS;
    }

    return ERR_NONE;
}

