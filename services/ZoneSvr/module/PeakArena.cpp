#include "PeakArena.h"
#include "GameDataMgr.h"
#include "LogMacros.h"
#include "Item.h"
#include "ZoneSvrMsgLayer.h"
#include "FakeRandom.h"
#include "dwlog_def.h"
#include "Consume.h"
#include "oi_misc.h"
#include "comm_func.h"

using namespace PKGMETA;

bool PeakArena::Init()
{
    if (!this->_InitBasic())
    {
        return false;
    }

    if (!this->_InitScore())
    {
        return false;
    }

    if (!this->_InitSeasonAndRule())
    {
        return false;
    }

    return true;
}

bool PeakArena::_InitBasic()
{
    //初始化刷新时间数据
	RESBASIC* poBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(COMMON_UPDATE_TIME);
	if (!poBasic)
	{
		LOGERR("poBasic is null");
		return false;
	}
    m_iUpdateTime = (int)poBasic->m_para[0];

    m_ullLastUptTimestamp = CGameTime::Instance().GetSecOfHourInCurrDay(m_iUpdateTime);
    if (CGameTime::Instance().GetCurrHour() < m_iUpdateTime)
    {
        m_ullLastUptTimestamp -= SECONDS_OF_DAY;
    }
    m_bUptFlag = false;

    //初始化产出物品相关
    RESPEAKARENAPARA* pResPara = CGameDataMgr::Instance().GetResPeakArenaParaMgr().Find(6);
    if (!pResPara)
    {
        return false;
    }
    for (int i=0; i<MAX_PEAK_ARENA_OUTPUT_NUM; i++)
    {
        m_OutputType[i] = pResPara->m_paramList[i*3 + 0];
        m_OutputId[i] = pResPara->m_paramList[i*3 + 1];
    }

    pResPara = CGameDataMgr::Instance().GetResPeakArenaParaMgr().Find(5);
    if (!pResPara)
    {
        return false;
    }
    m_wOutputInterval = pResPara->m_paramList[0];

    pResPara = CGameDataMgr::Instance().GetResPeakArenaParaMgr().Find(16);
    if (!pResPara)
    {
        return false;
    }
    m_bSettleRewardLimit = pResPara->m_paramList[0];
    m_bRewardTimesBuyLimit = pResPara->m_paramList[1];
    m_bRewardTimesBuyCost = pResPara->m_paramList[2];

    pResPara = CGameDataMgr::Instance().GetResPeakArenaParaMgr().Find(8);
    if (!pResPara)
    {
        return false;
    }
    m_bStreakTimes = pResPara->m_paramList[0];

    pResPara = CGameDataMgr::Instance().GetResPeakArenaParaMgr().Find(10);
    if (!pResPara)
    {
        return false;
    }
    m_dwSpeedUpDiamand = pResPara->m_paramList[0];
    m_ullSpeedUpLastTime = pResPara->m_paramList[1];
    m_dwSpeedUpOutputNum[0] = pResPara->m_paramList[2];
    m_dwSpeedUpOutputNum[1] = pResPara->m_paramList[3];

    return true;
}

bool PeakArena::_InitSeasonAndRule()
{
    RESPEAKARENAPARA* pResPara = CGameDataMgr::Instance().GetResPeakArenaParaMgr().Find(9);
    if (!pResPara)
    {
        return false;
    }
    m_bSeasonLastMonth = pResPara->m_paramList[0];

    //计算赛季开始月和赛季结束月
    struct tm tTime;
    bzero(&tTime, sizeof(tTime));

    //计算赛季开始时间
    int iStartYear = CGameTime::Instance().GetCurrYear();
    int iStartMonth = CGameTime::Instance().GetCurrMonth();
    int iDeltaMonth = ((iStartYear + 1900 - pResPara->m_paramList[1]) * 12 + (iStartMonth + 1 - pResPara->m_paramList[2])) % m_bSeasonLastMonth;
    if (iStartMonth < iDeltaMonth)
    {
        iStartYear--;
        iStartMonth = iStartMonth + 12 - iDeltaMonth;
    }
    else
    {
        iStartMonth -= iDeltaMonth;
    }

    tTime.tm_year = iStartYear;
    tTime.tm_mon = iStartMonth;
    tTime.tm_mday = 1;
    m_ullSeasonStartTime = mktime(&tTime);

    //计算赛季结束时间
    int iEndYear = iStartYear;
    int iEndMonth = iStartMonth + m_bSeasonLastMonth;
    if (iEndMonth > 11)
    {
        iEndYear++;
        iEndMonth -= 12;
    }
    tTime.tm_year = iEndYear;
    tTime.tm_mon = iEndMonth;
    tTime.tm_mday = 1;
    m_ullSeasonEndTime = mktime(&tTime);

    LOGRUN("Init Season, Start Year(%d) Month(%d), Stop Year(%d) Month(%d)", iStartYear, iStartMonth, iEndYear, iEndMonth);
	LOGRUN("Init PeakArena Season, StartTime<%lu>, StopTime<%lu>", m_ullSeasonStartTime, m_ullSeasonEndTime);
    return true;
}

bool PeakArena::_InitScore()
{
    ScoreNode stNode;
    RESPEAKARENASCORE* pResScore;
    uint32_t dwScore = MAX_PEAK_ARENA_SCORE;
    ResPeakArenaScoreMgr_t& rstResScoreMgr = CGameDataMgr::Instance().GetResPeakArenaScoreMgr();
    for (int i=rstResScoreMgr.GetResNum()-1; i>=0; i--)
    {
        pResScore = rstResScoreMgr.GetResByPos(i);

        stNode.m_dwLow = pResScore->m_dwScore;
        stNode.m_dwHigh = dwScore;
        dwScore = stNode.m_dwLow - 1;

        m_oScore2IdMap.insert(MapScore2Id_t::value_type(stNode, pResScore->m_dwId));
    }

    return true;
}

void PeakArena::_GenRuleList()
{
    int iNum = CGameDataMgr::Instance().GetResPeakArenaChooseRuleMgr().GetResNum();
    int iStart = 0;

    CFakeRandom::Instance().SetSeed((uint32_t)m_ullSeasonStartTime);

    for (int i=0; i<iNum; i++)
    {
        m_RuleList[i+iStart] = i + 1;
    }

    for (int j=0; j<(iNum/2+1); j++)
    {
        int m = CFakeRandom::Instance().Random(iNum);
        int n = CFakeRandom::Instance().Random(iNum);

        //交换m, n位置上的数
        uint8_t bTemp = m_RuleList[m+iStart];
        m_RuleList[m+iStart] = m_RuleList[n+iStart];
        m_RuleList[n+iStart] = bTemp;
    }
}

void PeakArena::UpdateServer()
{
    int iHour = CGameTime::Instance().GetCurrHour();
    if ((iHour == m_iUpdateTime) && m_bUptFlag)
    {
        m_bUptFlag = false;
        m_ullLastUptTimestamp = CGameTime::Instance().GetCurrSecond();
    }
    else if ((iHour != m_iUpdateTime))
    {
        m_bUptFlag = true;
    }

    uint64_t ullCurTime =  CGameTime::Instance().GetCurrSecond();
    if (ullCurTime >= m_ullSeasonEndTime)
    {
        this->_SettleSeason();
        int iYear = CGameTime::Instance().GetCurrYear();
        int iMonth = CGameTime::Instance().GetCurrMonth() + m_bSeasonLastMonth;
        if (iMonth > 11)
        {
            iMonth -= 12;
            iYear++;
        }
        struct tm tTime;
        bzero(&tTime, sizeof(tTime));
        tTime.tm_year = iYear;
        tTime.tm_mon = iMonth;
        tTime.tm_mday = 1;
        m_ullSeasonStartTime = m_ullSeasonEndTime;
        m_ullSeasonEndTime = mktime(&tTime);
		LOGRUN("UpdateServer PeakArena Season, StartTime<%lu>, StopTime<%lu>", m_ullSeasonStartTime, m_ullSeasonEndTime);
    }
}

void PeakArena::UpdatePlayerData(PlayerData* pstData)
{
    DT_ROLE_PEAK_ARENA_INFO& rstPeakArenaInfo = pstData->GetELOInfo().m_stPeakArenaInfo;

    if (rstPeakArenaInfo.m_ullCurTimestamp < m_ullLastUptTimestamp)
    {
        rstPeakArenaInfo.m_wCurFightCount = 0;
        rstPeakArenaInfo.m_bBuyTimes = 0;
        rstPeakArenaInfo.m_ullCurTimestamp = m_ullLastUptTimestamp;
    }

    uint64_t ullDiff = (uint64_t)CGameTime::Instance().GetCurrSecond() - rstPeakArenaInfo.m_ullOutputLastUptTime;
    if (ullDiff >= m_wOutputInterval)
    {
        this->_AddOutput(pstData, ullDiff/m_wOutputInterval);
        rstPeakArenaInfo.m_ullOutputLastUptTime = (uint64_t)CGameTime::Instance().GetCurrSecond() - (ullDiff % m_wOutputInterval);
    }

    if (rstPeakArenaInfo.m_ullSeasonStartTime < m_ullSeasonStartTime)
    {
        this->_ResetPlayerData(pstData);
    }
}

void PeakArena::_ResetPlayerData(PlayerData* pstData)
{
    DT_ROLE_PEAK_ARENA_INFO& rstPeakArenaInfo = pstData->GetELOInfo().m_stPeakArenaInfo;

    rstPeakArenaInfo.m_ullSeasonStartTime = m_ullSeasonStartTime;
    rstPeakArenaInfo.m_ullSeasonEndTime = m_ullSeasonEndTime;

    rstPeakArenaInfo.m_wWinCount = 0;
    rstPeakArenaInfo.m_wLoseCount = 0;
    rstPeakArenaInfo.m_wActiveValue = 0;
    rstPeakArenaInfo.m_ullActiveRewardFlag = 0;
    rstPeakArenaInfo.m_bStreakTimes = 0;
    rstPeakArenaInfo.m_bELOLvId = 1;
    rstPeakArenaInfo.m_dwScore = 0;

    for (int i=0; i<MAX_PEAK_ARENA_OUTPUT_NUM; i++)
    {
        rstPeakArenaInfo.m_OutputAddValue[i] = 0;
        rstPeakArenaInfo.m_OutputValue[i] = 0;
    }
}

void PeakArena::_AddOutput(PlayerData* pstData, uint16_t wNum)
{
    DT_ROLE_PEAK_ARENA_INFO& rstPeakArenaInfo = pstData->GetELOInfo().m_stPeakArenaInfo;
    uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();
    //if (rstPeakArenaInfo.m_dwScore == 0)
    //{
    //    return;
    //}
    RESPEAKARENAOUTPUTREWARD* pResOutput = CGameDataMgr::Instance().GetResPeakArenaOutputRewardMgr().Find(rstPeakArenaInfo.m_bELOLvId);
    if (!pResOutput)
    {
        LOGERR("pResOutput is null");
        return;
    }

/*	RESPEAKARENAACTIVEREWARD* pstActiveReward = this->_GetActiveRewardByActiveVal( rstPeakArenaInfo.m_wActiveValue );
	if( !pstActiveReward )
	{
		assert(pstActiveReward);
		LOGERR("Can not find RESPEAKARENAACTIVEREWARD* for active value <%u>", rstPeakArenaInfo.m_wActiveValue);
	}
*/
	
    for (int i=0; i<MAX_PEAK_ARENA_OUTPUT_NUM; i++)
    {
        uint32_t dwAddvalue = pResOutput->m_rewardNum[i] + rstPeakArenaInfo.m_OutputAddValue[i];
		
		// buff 产出不累加，直接根据活跃度从配置档读取
        	/*uint32_t dwAddvalue = pResOutput->m_rewardNum[i];
        	if( pstActiveReward )
        	{
        		dwAddvalue += pstActiveReward->m_buffRewardNum[i];
        	}*/
		
        if (ullCurTime <= rstPeakArenaInfo.m_ullSpeedUpEndTime)
        {
            //dwAddvalue = dwAddvalue * m_wSpeedUpRatio / 100;
            dwAddvalue += m_dwSpeedUpOutputNum[i];
        }
        rstPeakArenaInfo.m_OutputValue[i] += dwAddvalue * wNum;
        if (rstPeakArenaInfo.m_OutputValue[i] > pResOutput->m_rewardLimit[i])
        {
            rstPeakArenaInfo.m_OutputValue[i] = pResOutput->m_rewardLimit[i];
        }
    }

    return;
}

void PeakArena::HandleELOSettle(PlayerData* pstData, uint8_t bIsWin)
{
    DT_ROLE_ELO_INFO& rstInfo = pstData->GetELOInfo();
    DT_ROLE_PEAK_ARENA_INFO& rstPeakArenaInfo = rstInfo.m_stPeakArenaInfo;

    ScoreNode stNode;
    stNode.m_dwHigh = rstPeakArenaInfo.m_dwScore;
    stNode.m_dwLow = rstPeakArenaInfo.m_dwScore;

    MapScore2Id_t::iterator iter = m_oScore2IdMap.find(stNode);
    if (iter == m_oScore2IdMap.end())
    {
        LOGERR("Player(%s) Uin(%lu) HandleELOSettle failed, score(%u) not found in m_oScore2IdMap",
               pstData->GetRoleBaseInfo().m_szRoleName, pstData->m_ullUin, rstPeakArenaInfo.m_dwScore);
        return;
    }

    RESPEAKARENASCORE* pResScore = CGameDataMgr::Instance().GetResPeakArenaScoreMgr().Find(iter->second);
    if (!pResScore)
    {
        LOGERR("Player(%s) Uin(%lu) HandleELOSettle failed, pResScore id(%u) not found",
               pstData->GetRoleBaseInfo().m_szRoleName, pstData->m_ullUin, iter->second);
        return;
    }

    //胜利
    if (bIsWin == 1)
    {
        rstPeakArenaInfo.m_wWinCount++;
        rstPeakArenaInfo.m_bStreakTimes++;

        rstPeakArenaInfo.m_dwScore++;
        // 满足连胜奖励，额外奖励1颗星
        if ((rstPeakArenaInfo.m_bStreakTimes >= m_bStreakTimes) && (pResScore->m_dwIsBonus == 1))
        {
            rstPeakArenaInfo.m_dwScore++;
        }
    }
    //失败
    else if (bIsWin == 0)
    {
        rstPeakArenaInfo.m_wLoseCount++;
        rstPeakArenaInfo.m_bStreakTimes = 0;

        // 检查当前等级是否减分
        if (pResScore->m_bIsMinus == 1)
        {
            rstPeakArenaInfo.m_dwScore = (rstPeakArenaInfo.m_dwScore == 0) ? 0 : (rstPeakArenaInfo.m_dwScore - 1);
        }
    }
    //平局
    else
    {
        rstPeakArenaInfo.m_bStreakTimes = 0;
    }

    // 计算新的排名等级
    stNode.m_dwHigh = rstPeakArenaInfo.m_dwScore;
    stNode.m_dwLow = rstPeakArenaInfo.m_dwScore;
    iter = m_oScore2IdMap.find(stNode);
    if (iter == m_oScore2IdMap.end())
    {
        LOGERR("Player(%s) Uin(%lu) HandleELOSettle failed, score(%u) not found in m_oScore2IdMap",
                pstData->GetRoleBaseInfo().m_szRoleName, pstData->m_ullUin, rstPeakArenaInfo.m_dwScore);
        return;
    }
    rstPeakArenaInfo.m_bELOLvId = iter->second;

    return;
}

void PeakArena::HandleRewardSettle(PlayerData* pstData, uint8_t bIsWin, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
    DT_ROLE_PEAK_ARENA_INFO& rstPeakArenaInfo = pstData->GetELOInfo().m_stPeakArenaInfo;
    if (rstPeakArenaInfo.m_ullCurTimestamp < m_ullLastUptTimestamp)
    {
        rstPeakArenaInfo.m_wCurFightCount = 0;
        rstPeakArenaInfo.m_ullCurTimestamp = m_ullLastUptTimestamp;
    }

    if (rstPeakArenaInfo.m_wCurFightCount >= m_bSettleRewardLimit + rstPeakArenaInfo.m_bBuyTimes)
    {
        return;
    }

    //不论输赢都会增加活跃度
    rstPeakArenaInfo.m_wActiveValue++;
    rstPeakArenaInfo.m_wCurFightCount++;

    RESPEAKARENAREWARD* pResReward = CGameDataMgr::Instance().GetResPeakArenaRewardMgr().Find(rstPeakArenaInfo.m_bELOLvId);
    if (!pResReward)
    {
        LOGERR("pResReward is null");
        return;
    }

    if (bIsWin == 1)
    {
        for (int i=0; i<pResReward->m_bWinRewardCnt; i++)
        {
            Item::Instance().RewardItem(pstData, pResReward->m_szWinRewardType[i], pResReward->m_winRewardId[i], pResReward->m_winRewardNum[i], rstSyncItemInfo, DWLOG::METHOD_ARENA_FIGHT_SETTLE_WIN);
        }
    }
    else
    {
        for (int i=0; i<pResReward->m_bLoseRewardCnt; i++)
        {
            Item::Instance().RewardItem(pstData, pResReward->m_szLoseRewardType[i], pResReward->m_loseRewardId[i], pResReward->m_loseRewardNum[i], rstSyncItemInfo, DWLOG::METHOD_ARENA_FIGHT_SETTLE_LOSE);
        }
    }

    return;
}

int PeakArena::RecvActiveReward(PlayerData* pstData, CS_PKG_PEAK_ARENA_GET_ACTIVE_REWARD_REQ& rstReq, SC_PKG_PEAK_ARENA_GET_ACTIVE_REWARD_RSP& rstRsp)
{
    for (int i=0; i<rstReq.m_bRewardCnt; i++)
    {
        int iRet = _RecvOneReward(pstData, rstReq.m_szRewardList[i], rstRsp);
        if (iRet != ERR_NONE)
        {
            return iRet;
        }
    }

    return ERR_NONE;
}

int PeakArena::_RecvOneReward(PlayerData* pstData, uint32_t dwId, SC_PKG_PEAK_ARENA_GET_ACTIVE_REWARD_RSP& rstRsp)
{
    RESPEAKARENAACTIVEREWARD* pResReward = CGameDataMgr::Instance().GetResPeakArenaActiveRewardMgr().Find(dwId);
    if (!pResReward)
    {
        LOGERR("pResReward is null");
        return ERR_NOT_FOUND;
    }

    DT_ROLE_PEAK_ARENA_INFO& rstPeakArenaInfo = pstData->GetELOInfo().m_stPeakArenaInfo;
    if (rstPeakArenaInfo.m_wActiveValue < pResReward->m_dwActiveValue)
    {
        return ERR_NOT_SATISFY_COND;
    }

    if ((rstPeakArenaInfo.m_ullActiveRewardFlag & (1 << dwId)) > 0)
    {
        return ERR_AWARD_FINISHED;
    }

    rstPeakArenaInfo.m_ullActiveRewardFlag = rstPeakArenaInfo.m_ullActiveRewardFlag | (1 << dwId);

    DT_SYNC_ITEM_INFO& rstSyncItemInfo = rstRsp.m_stSyncItemInfo;
    for (int i=0; i<pResReward->m_bRewardCnt; i++)
    {
        Item::Instance().RewardItem(pstData, pResReward->m_szRewardType[i], pResReward->m_rewardId[i], pResReward->m_rewardNum[i], rstSyncItemInfo, DWLOG::METHOD_ARENA_ACTIVE_REWARD);
    }

    for (int i=0; i<MAX_PEAK_ARENA_OUTPUT_NUM; i++)
    {
        //rstPeakArenaInfo.m_OutputAddValue[i] += pResReward->m_buffRewardNum[i];
        //rstRsp.m_astOutputBuff[i].m_dwItemNum += pResReward->m_buffRewardNum[i];
        if( pResReward->m_buffRewardNum[i] > rstPeakArenaInfo.m_OutputAddValue[i] )
        {
            // 领了更高级别的活跃buff
            rstPeakArenaInfo.m_OutputAddValue[i] = pResReward->m_buffRewardNum[i];	
        }

        rstRsp.m_astOutputBuff[i].m_dwItemNum = rstPeakArenaInfo.m_OutputAddValue[i];
	   rstRsp.m_astOutputBuff[i].m_bItemType = m_OutputType[i];
        rstRsp.m_astOutputBuff[i].m_dwItemId = m_OutputId[i];
    }

    return ERR_NONE;
}

int PeakArena::RecvOutput(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
    DT_ROLE_PEAK_ARENA_INFO& rstPeakArenaInfo = pstData->GetELOInfo().m_stPeakArenaInfo;

    for (int i=0; i<MAX_PEAK_ARENA_OUTPUT_NUM; i++)
    {
        Item::Instance().RewardItem(pstData, m_OutputType[i], m_OutputId[i], rstPeakArenaInfo.m_OutputValue[i], rstSyncItemInfo, DWLOG::METHOD_ARENA_OUTPUT_REWARD);
        rstPeakArenaInfo.m_OutputValue[i] = 0;
    }

    return ERR_NONE;
}

int PeakArena::BuyTimes(PlayerData* pstData, uint8_t bTimes, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
    DT_ROLE_PEAK_ARENA_INFO& rstPeakArenaInfo = pstData->GetELOInfo().m_stPeakArenaInfo;
    if (rstPeakArenaInfo.m_bBuyTimes + bTimes > m_bRewardTimesBuyLimit)
    {
        return ERR_NOT_ENOUGH_TIMES;
    }

    int iDiamandConsume = bTimes * m_bRewardTimesBuyCost;
    if (!Consume::Instance().IsEnoughDiamond(pstData, iDiamandConsume))
    {
        return ERR_NOT_ENOUGH_DIAMOND;
    }

    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -iDiamandConsume, rstSyncItemInfo, DWLOG::METHOD_ARENA_SPEED_UP_OUTPUT);

    rstPeakArenaInfo.m_bBuyTimes += bTimes;

    return ERR_NONE;
}

int PeakArena::SpeedUpOutput(PlayerData* pstData, SC_PKG_PEAK_ARENA_SPEED_UP_OUTPUT_RSP& rstRsp)
{
    DT_ROLE_PEAK_ARENA_INFO& rstPeakArenaInfo = pstData->GetELOInfo().m_stPeakArenaInfo;
    uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();

    rstRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    if (rstPeakArenaInfo.m_ullSpeedUpEndTime > ullCurTime)
    {
        return ERR_ALREADY_EXISTED;
    }

    if (!Item::Instance().IsEnough(pstData, ITEM_TYPE_DIAMOND, 0, m_dwSpeedUpDiamand))
    {
        return ERR_NOT_ENOUGH_DIAMOND;
    }

    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -m_dwSpeedUpDiamand, rstRsp.m_stSyncItemInfo, DWLOG::METHOD_ARENA_SPEED_UP_OUTPUT);

    rstPeakArenaInfo.m_ullSpeedUpEndTime = ullCurTime + m_ullSpeedUpLastTime;

    rstRsp.m_ullEndTime = rstPeakArenaInfo.m_ullSpeedUpEndTime;

    return ERR_NONE;
}

uint32_t PeakArena::GetRuleId()
{
    return 1;
}

void PeakArena::UpdateRank(PlayerData* pstData)
{
    DT_ROLE_PEAK_ARENA_INFO& rstPeakArenaInfo = pstData->GetELOInfo().m_stPeakArenaInfo;

    SSPKG stSsPkg;
    stSsPkg.m_stHead.m_wMsgId = SS_MSG_RANK_COMMON_UPDATE_NTF;

    SS_PKG_RANK_COMMON_UPDATE_NTF& rstNtf = stSsPkg.m_stBody.m_stRankCommonUpdateNtf;
    rstNtf.m_bType = PEAK_ARENA_RANK_INFO_TYPE;

    DT_PEAK_ARENA_RANK_INFO& rstRankInfo = rstNtf.m_stReqInfo.m_stPeakArenaRankInfo;
    rstRankInfo.m_ullUin = pstData->GetRoleBaseInfo().m_ullUin;
    memcpy(rstRankInfo.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, MAX_NAME_LENGTH);
    rstRankInfo.m_dwScore = rstPeakArenaInfo.m_dwScore;
    rstRankInfo.m_wWinCount = rstPeakArenaInfo.m_wWinCount;
    rstRankInfo.m_ullTimeStampMs = CGameTime::Instance().GetCurrTimeMs();
    rstRankInfo.m_bELOId = rstPeakArenaInfo.m_bELOLvId;

    ZoneSvrMsgLayer::Instance().SendToRankSvr(stSsPkg);
}

void PeakArena::_SettleSeason()
{
    SSPKG stSsPkg;
    stSsPkg.m_stHead.m_wMsgId = SS_MSG_PEAK_ARENA_RANK_SETTLE_NTF;
    ZoneSvrMsgLayer::Instance().SendToRankSvr(stSsPkg);
}

static int ActiveValueCmp( const void *key, const void *p )
{
	uint16_t wActiveVal = *(uint16_t*)key;
	RESPEAKARENAACTIVEREWARD* pstActiveReward = (RESPEAKARENAACTIVEREWARD*)p;

	int iResult = (int)wActiveVal - (int)pstActiveReward->m_dwActiveValue;

	return iResult;
}

// 通过活跃值获取巅峰的活跃奖励配置
RESPEAKARENAACTIVEREWARD* PeakArena::_GetActiveRewardByActiveVal(uint16_t wActiveValue)
{
	ResPeakArenaActiveRewardMgr_t& ActiveRewardMgr = CGameDataMgr::Instance().GetResPeakArenaActiveRewardMgr();
	char* pResBuf = ActiveRewardMgr.GetResBuf();

	int iEqual = 0;
	int idx = -1;
	idx = MyBSearch( &wActiveValue, pResBuf, ActiveRewardMgr.GetResNum(), sizeof(RESPEAKARENAACTIVEREWARD), &iEqual, ActiveValueCmp);
	if( iEqual == 0 )
	{
		// not found
		idx--;
	}
	if( idx < 0 )
	{
		assert(false);
		idx==0;
	}
	return ActiveRewardMgr.GetResByPos(idx);
}

// 活跃度产出buff加成改成直接配置，不再使用累加的方式, 适配转换老数据
void PeakArena::AdaptActiveRewardBuff( PlayerData* pstData )
{
    DT_ROLE_PEAK_ARENA_INFO& rstPeakArenaInfo = pstData->GetELOInfo().m_stPeakArenaInfo;

    // 位运算，找出最高位 rstPeakArenaInfo.m_ullActiveRewardFlag 最高位为1的id
    bzero( rstPeakArenaInfo.m_OutputAddValue, sizeof(rstPeakArenaInfo.m_OutputAddValue) );
    int id = bsr_int64(rstPeakArenaInfo.m_ullActiveRewardFlag);
    if( id > 0 )
    {
        RESPEAKARENAACTIVEREWARD* pResReward = CGameDataMgr::Instance().GetResPeakArenaActiveRewardMgr().Find(id);
        if( pResReward ) 
        {
            for (int i=0; i<MAX_PEAK_ARENA_OUTPUT_NUM; i++)
            {
                rstPeakArenaInfo.m_OutputAddValue[i] = pResReward->m_buffRewardNum[i];	
            }
        }
    }        
}

