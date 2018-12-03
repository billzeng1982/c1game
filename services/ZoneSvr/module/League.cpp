#include "League.h"
#include "Item.h"
#include "GameTime.h"
#include "LogMacros.h"
#include "Props.h"
#include "../gamedata/GameDataMgr.h"
#include "dwlog_def.h"
#include "Majesty.h"
#include "GloryItemsMgr.h"

using namespace PKGMETA;
using namespace DWLOG;

bool League::Init()
{
#if 0
    uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();

    ResWeekLeagueParaMgr_t& rstResWeekLeagueParaMgr = CGameDataMgr::Instance().GetResWeekLeagueParaMgr();

    RESWEEKLEAGUEPARA* pPara = rstResWeekLeagueParaMgr.Find(WEEK_LEAGUE_PARAM_START_TIME1);
    uint64_t ullStartTime = CGameTime::Instance().GetSecByWeekDay(pPara->m_paramList[0], pPara->m_paramList[1]);
    m_dwLastTime = pPara->m_paramList[2] * SECONDS_OF_HOUR;
    m_TimeList.push_back(ullStartTime);
    pPara = rstResWeekLeagueParaMgr.Find(WEEK_LEAGUE_PARAM_START_TIME2);
    ullStartTime = CGameTime::Instance().GetSecByWeekDay(pPara->m_paramList[0], pPara->m_paramList[1]);
    m_TimeList.push_back(ullStartTime);

    int i = 0;
    int iSize = m_TimeList.size();
    list<uint64_t>::iterator iter;

    for (; i < iSize; i++)
    {
        iter = m_TimeList.begin();
        if (ullCurTime < *iter)
        {
            m_ullTimestamp = *iter;
            m_bIsStart = false;
            break;
        }
        else if ((ullCurTime >= *iter) && (ullCurTime<= (*iter + m_dwLastTime)))
        {
            m_ullTimestamp = *iter;
            m_bIsStart = true;
            break;
        }
        else
        {
            m_TimeList.push_back(*iter + SECONDS_OF_WEEK);
            m_TimeList.erase(iter);
        }
    }

    if (i == iSize)
    {
        iter = m_TimeList.begin();
        m_ullTimestamp = *iter;
        m_bIsStart = false;
    }

    pPara = rstResWeekLeagueParaMgr.Find(WEEK_LEAGUE_PARAM_WIN_LIMIT);
    m_bWInTimesLimit = pPara->m_paramList[0];

    pPara = rstResWeekLeagueParaMgr.Find(WEEK_LEAGUE_PARAM_LOSE_LIMIT);
    m_bLoseTimesLimit = pPara->m_paramList[0];

    pPara = rstResWeekLeagueParaMgr.Find(WEEK_LEAGUE_PARAM_TICKET);
    m_dwTicketId = pPara->m_paramList[0];
    m_dwTicketConsume = pPara->m_paramList[1];
#endif
    return true;
}


void League::UpdateServer()
{
#if 0
    uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();

    if (m_bIsStart)
    {
        //联赛结束
        if (ullCurTime >= m_ullTimestamp + m_dwLastTime)
        {
            m_bIsStart = false;
            m_ullTimestamp = m_TimeList.front();
            m_TimeList.pop_front();
            m_TimeList.push_back(m_ullTimestamp + SECONDS_OF_WEEK);
        }
    }
    else
    {
        //联赛开始
        if (ullCurTime >= m_ullTimestamp)
        {
            m_bIsStart = true;
        }
    }
#endif
}

void League::InitPlayerData(PlayerData* pstData)
{
#if 0
    DT_ROLE_WEEK_LEAGUE_INFO& rstLeagueInfo  = pstData->GetELOInfo().m_stWeekLeagueInfo;
    if (rstLeagueInfo.m_bState == WEEK_LEAGUE_STATE_MATCH &&
      (rstLeagueInfo.m_ullTimestamp < m_ullTimestamp ||
      rstLeagueInfo.m_bWinCount >= m_bWInTimesLimit ||
      rstLeagueInfo.m_bLoseCount >= m_bLoseTimesLimit))
    {
        rstLeagueInfo.m_bState = WEEK_LEAGUE_STATE_RECV_REWARD;
    }
#endif
}

int League::CheckMatch(PlayerData* pstData)
{
#if 0
    DT_ROLE_WEEK_LEAGUE_INFO& rstLeagueInfo  = pstData->GetELOInfo().m_stWeekLeagueInfo;
    if (rstLeagueInfo.m_bState != WEEK_LEAGUE_STATE_MATCH)
    {
        LOGERR("Uin<%lu> cant match. state<%hhu> ", pstData->m_ullUin, rstLeagueInfo.m_bState );
        return ERR_NOT_SATISFY_COND;
    }

    if ( IsReward(pstData, rstLeagueInfo) )
    {
        LOGERR("Uin<%lu> week fight cant match ,may be in reward state, State<%d> PStartTime<%lu> SysStartTime<%lu> Win<%hhu> Lost<%hhu>", pstData->GetRoleBaseInfo().m_ullUin, rstLeagueInfo.m_bState,
            rstLeagueInfo.m_ullTimestamp, m_ullTimestamp, rstLeagueInfo.m_bWinCount, rstLeagueInfo.m_bLoseCount);
        return ERR_DEFAULT;
    }
#endif
    return ERR_NONE;
}

int League::WeekLeagueApply(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
#if 0
    if (!m_bIsStart)
    {
        return ERR_NOT_START;
    }

    DT_ROLE_WEEK_LEAGUE_INFO& rstLeagueInfo  = pstData->GetELOInfo().m_stWeekLeagueInfo;
    if (rstLeagueInfo.m_bState != WEEK_LEAGUE_STATE_APPLY)
    {
        return ERR_DEFAULT;
    }

    if (!Props::Instance().IsEnough(pstData, m_dwTicketId, m_dwTicketConsume))
    {
        return ERR_NOT_ENOUGH_PROPS;
    }

    rstLeagueInfo.m_bState = WEEK_LEAGUE_STATE_MATCH;
    rstLeagueInfo.m_bWinCount = 0;
    rstLeagueInfo.m_bLoseCount = 0;
    rstLeagueInfo.m_ullTimestamp = m_ullTimestamp;

    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, m_dwTicketId, -m_dwTicketConsume, rstSyncItemInfo, METHOD_MATCH_WEEK_LEAGUE);
#endif
    return ERR_NONE;
}

int League::WeekLeagueRecvReward(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
#if 0
    DT_ROLE_WEEK_LEAGUE_INFO& rstLeagueInfo  = pstData->GetELOInfo().m_stWeekLeagueInfo;

    if ( !IsReward(pstData, rstLeagueInfo) )
    {
        LOGERR("Uin<%lu> WeekLeague Reward failed, State<%d> PStartTime<%lu> SysStartTime<%lu> Win<%hhu> Lost<%hhu>", pstData->GetRoleBaseInfo().m_ullUin, rstLeagueInfo.m_bState,
            rstLeagueInfo.m_ullTimestamp, m_ullTimestamp, rstLeagueInfo.m_bWinCount, rstLeagueInfo.m_bLoseCount);
        return ERR_DEFAULT;
    }

    ResWeekLeagueRewardMgr_t& rstResWeekLeagueRewardMgr = CGameDataMgr::Instance().GetResWeekLeagueRewardMgr();
    RESWEEKLEAGUEREWARD* pResReward = rstResWeekLeagueRewardMgr.Find(rstLeagueInfo.m_bWinCount);
    if (pResReward == NULL)
    {
        LOGERR("Player(%s) Uin(%lu) Recv WeekLeague Reward failed, WinCount=(%d)", pstData->GetRoleBaseInfo().m_szRoleName,
                pstData->GetRoleBaseInfo().m_ullUin, rstLeagueInfo.m_bWinCount);
        return ERR_SYS;
    }

    for (int i=0; i<pResReward->m_bRewardCount; i++)
    {
        Item::Instance().RewardItem(pstData, pResReward->m_szItemType[i], pResReward->m_itemId[i], pResReward->m_itemNum[i],
                                    rstSyncItemInfo, METHOD_MATCH_WEEK_LEAGUE);
    }

	GloryItemsMgr::Instance().AddMajestyItems(pstData, MAJESTY_ITEM_ACCESS_WIN_WEEKLEAGUE, rstLeagueInfo.m_bLoseCount);
    rstLeagueInfo.m_bState = WEEK_LEAGUE_STATE_APPLY;
#endif
    return ERR_NONE;
}

bool League::IsReward(PlayerData* pstData , const DT_ROLE_WEEK_LEAGUE_INFO& rstLeagueInfo)
{
#if 0
    bool bRet = WEEK_LEAGUE_STATE_MATCH == rstLeagueInfo.m_bState &&
        (rstLeagueInfo.m_ullTimestamp < m_ullTimestamp ||
        rstLeagueInfo.m_bWinCount >= m_bWInTimesLimit ||
        rstLeagueInfo.m_bLoseCount >= m_bLoseTimesLimit);
    return bRet;
#endif
    return true;
}

