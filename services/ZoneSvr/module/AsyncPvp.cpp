#include "AsyncPvp.h"
#include "strutil.h"
#include "GameTime.h"
#include "GeneralCard.h"
#include "Majesty.h"
#include "MasterSkill.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "Equip.h"
#include "Consume.h"
#include "Item.h"
#include "dwlog_def.h"
#include "ZoneLog.h"
#include "dwlog_def.h"
#include "Task.h"
#include "Tactics.h"

using namespace PKGMETA;

AsyncPvp::AsyncPvp()
{
}


AsyncPvp::~AsyncPvp()
{
}


bool AsyncPvp::Init()
{
    //初始化时间戳和更新时间
    ResBasicMgr_t& rstResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
    RESBASIC* pUpdateTime = rstResBasicMgr.Find(COMMON_UPDATE_TIME);
    if (pUpdateTime == NULL)
    {
        LOGERR("pUpdateTime is NULL");
        return false;
    }

    //每日刷新时间
    m_iUptTimeHour = (int)(pUpdateTime->m_para[0]);
    m_ullLastUptTime = CGameTime::Instance().GetSecOfHourInCurrDay(m_iUptTimeHour);
    if (CGameTime::Instance().GetCurrHour() < m_iUptTimeHour)
    {
        m_ullLastUptTime -= SECONDS_OF_DAY;
    }
    m_bUptFlag = false;

    //刷新和挑战次数相关参数
    RESBASIC* pRefreshTimes = rstResBasicMgr.Find(ASYNCPVP_REFRESH_OPPONENT_TIME);
    if (pRefreshTimes == NULL)
    {
        LOGERR("pRefreshTimes is NULL");
        return false;
    }
    m_iFreeRefreshTimes = (int)pRefreshTimes->m_para[0];

    RESBASIC* pFightTimes = rstResBasicMgr.Find(ASYNCPVP_CHALLENGE_OPPONENT_TIME);
    if (pFightTimes == NULL)
    {
        LOGERR("pRefreshTimes is NULL");
        return false;
    }
    m_iFreeFightTimes = (int)pFightTimes->m_para[0];

    ResConsumeMgr_t& rstResConsumeMgr = CGameDataMgr::Instance().GetResConsumeMgr();

    RESBASIC* pRefreshConsumeId = rstResBasicMgr.Find(ASYNCPVP_REFRESH_OPPONENT_TIME_BUY);
    if (pRefreshConsumeId == NULL)
    {
        LOGERR("pRefreshConsumeId is NULL");
        return false;
    }

    m_pResRefreshConsume = rstResConsumeMgr.Find((int)pRefreshConsumeId->m_para[0]);
    if (m_pResRefreshConsume == NULL)
    {
        LOGERR("m_pResRefreshConsume is NULL");
        return false;
    }

    RESBASIC* pFightConsumeId = rstResBasicMgr.Find(ASYNCPVP_CHALLENGE_OPPONENT_TIME_BUY);
    if (pFightConsumeId == NULL)
    {
        LOGERR("pFightConsumeId is NULL");
        return false;
    }

    m_pResFightBuyConsume = rstResConsumeMgr.Find((int)pFightConsumeId->m_para[0]);
    if (m_pResFightBuyConsume == NULL)
    {
        LOGERR("m_pResFightConsume is NULL");
        return false;
    }

	//冷却时间
	RESBASIC* pColdDownTime = rstResBasicMgr.Find(CD_ASYNCPVP);
	if (NULL == pColdDownTime)
	{
		LOGERR("pColdDownTime is null");
		return false;
	}
	ResVIPMgr_t& rResVipMgr = CGameDataMgr::Instance().GetResVIPMgr();


	for (int i=0; i<MAX_VIP_LEVEL; i++)
	{
		RESVIP* pResVip = rResVipMgr.Find(i+1);
		if (NULL == pResVip)
		{
			LOGERR("pResVip is null");
			return false;
		}
		m_ullColdTimeList[i] = pColdDownTime->m_para[0] - pResVip->m_wAsyncpvpTimeReduce;
	}

	m_iColdResetCostDia = pColdDownTime->m_para[1];

    //排行榜
    m_ullTopListTimeStamp = 0;
    bzero(m_astTopList, sizeof(m_astTopList));

	//膜拜
	RESBASIC* pWorshipReward = rstResBasicMgr.Find(ASYNCPVP_WORSHIP_REWARD);
	if (NULL == pWorshipReward)
	{
		LOGERR("pWorshipReward is null");
		return false;
	}
	m_iWorshipperGoldOnce = pWorshipReward->m_para[0];
	m_iWorshipperGoldMax = pWorshipReward->m_para[1];
	m_iWorshippedGoldOnce = pWorshipReward->m_para[2];
	m_iWorshippedGoldMax = pWorshipReward->m_para[3];

	RESBASIC* pWorshipFreshTime = rstResBasicMgr.Find(ASYNCPVP_WORSHIP_FRESH_TIME);
	if (NULL == pWorshipFreshTime)
	{
		LOGERR("pWorshipFreshTime is null");
		return false;
	}
	m_iWorshipTimeHour1 = (int)pWorshipFreshTime->m_para[0];
	m_iWorshipTimeHour2 = (int)pWorshipFreshTime->m_para[1];

	int iHour = CGameTime::Instance().GetCurrHour();
	if ((iHour < m_iWorshipTimeHour1) || (iHour >= m_iWorshipTimeHour2))
	{
		iHour = m_iWorshipTimeHour2;
	}
	else
	{
		iHour = m_iWorshipTimeHour1;
	}
	m_ullLastWorshipUptTime = CGameTime::Instance().GetSecOfHourInCurrDay(iHour);
	if (CGameTime::Instance().GetCurrHour() < m_iWorshipTimeHour1)
	{
		m_ullLastWorshipUptTime -= SECONDS_OF_DAY;
	}
	m_bWorshipUptFlag = false;

    return true;
}


void AsyncPvp::UpdateServer()
{
    int iHour = CGameTime::Instance().GetCurrHour();
    //每日更新时间
    if ((iHour == m_iUptTimeHour) && m_bUptFlag)
    {
        m_bUptFlag = false;
        m_ullLastUptTime = CGameTime::Instance().GetCurrSecond();
    }
    else if ((iHour != m_iUptTimeHour))
    {
        m_bUptFlag = true;
    }

	//膜拜次数每日刷新两次
	if ((iHour == m_iWorshipTimeHour1 || iHour == m_iWorshipTimeHour2)  && m_bWorshipUptFlag)
	{
		m_bWorshipUptFlag = false;
		m_ullLastWorshipUptTime = CGameTime::Instance().GetCurrSecond();
	}
	else if (iHour != m_iWorshipTimeHour1 && iHour != m_iWorshipTimeHour2)
	{
		m_bWorshipUptFlag = true;
	}
}


void AsyncPvp::UpdatePlayerData(PlayerData* pstData)
{
    DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = pstData->GetELOInfo().m_stAsyncPvpInfo;

	//刷新可膜拜次数
	if ((rstAsyncPvpInfo.m_bOpenFlag != 0) && (rstAsyncPvpInfo.m_ullWorshipTimeStamp < m_ullLastWorshipUptTime))
	{
		for (int i=0; i<MAX_TOP_PLAYER_NUM; i++)
		{
			rstAsyncPvpInfo.m_astTopTenPlayerList[i].m_ullUin = m_astTopList[i].m_stBaseInfo.m_ullUin;
			rstAsyncPvpInfo.m_astTopTenPlayerList[i].m_bIsWorshipped = 0;
		}
		rstAsyncPvpInfo.m_ullWorshipTimeStamp = m_ullLastWorshipUptTime;
	}

    if ((rstAsyncPvpInfo.m_bOpenFlag == 0) || (rstAsyncPvpInfo.m_ullTimestamp >= m_ullLastUptTime))
    {
        return;
    }

    this->_ResetPlayerData(pstData);
}


void AsyncPvp::_ResetPlayerData(PlayerData* pstData)
{
    DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = pstData->GetELOInfo().m_stAsyncPvpInfo;
    rstAsyncPvpInfo.m_bFightTimes = 0;
    rstAsyncPvpInfo.m_bFightTimesBuy = 0;
    rstAsyncPvpInfo.m_bRefreshTimes = 0;
    rstAsyncPvpInfo.m_wScore = 0;
    rstAsyncPvpInfo.m_dwScoreRewardFlag = 0;
	rstAsyncPvpInfo.m_dwWorshipGold = 0;
	rstAsyncPvpInfo.m_dwWorshippedGold = 0;
    rstAsyncPvpInfo.m_ullTimestamp = m_ullLastUptTime;


}


void AsyncPvp::UptToAsyncSvr(PlayerData* pstData)
{
    DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = pstData->GetELOInfo().m_stAsyncPvpInfo;
    if (rstAsyncPvpInfo.m_bOpenFlag == 0)
    {
        return;
    }

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ASYNC_PVP_UPT_PLAYER_NTF;
    m_stSsPkg.m_stHead.m_ullUin = pstData->GetRoleBaseInfo().m_ullUin;
    SS_PKG_ASYNC_PVP_UPT_PLAYER_NTF& rstNtf = m_stSsPkg.m_stBody.m_stAsyncpvpUptPlayerNtf;
    this->GetShowData(pstData, rstNtf.m_stShowData);
    this->GetTeamData(pstData, rstNtf.m_stTeamData);

    ZoneSvrMsgLayer::Instance().SendToAsyncPvpSvr(m_stSsPkg);
}

int AsyncPvp::GetShowData(PlayerData* pstData, DT_ASYNC_PVP_PLAYER_SHOW_DATA& rstShowData)
{
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
    DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = pstData->GetELOInfo().m_stAsyncPvpInfo;

    rstShowData.m_stBaseInfo.m_ullUin = pstData->m_ullUin;
    rstShowData.m_stBaseInfo.m_wHeadIconId = rstMajestyInfo.m_wIconId;
    rstShowData.m_stBaseInfo.m_wHeadFrameId = rstMajestyInfo.m_wFrameId;
    rstShowData.m_stBaseInfo.m_bLv = rstMajestyInfo.m_wLevel;
    rstShowData.m_stBaseInfo.m_dwLi = GeneralCard::Instance().GetTeamLi(pstData, BATTLE_ARRAY_TYPE_ASYNCPVP);
    rstShowData.m_stBaseInfo.m_dwWinCnt = 0;
    StrCpy(rstShowData.m_stBaseInfo.m_szName, pstData->GetRoleBaseInfo().m_szRoleName, MAX_NAME_LENGTH);
	rstShowData.m_bVipLv = rstMajestyInfo.m_bVipLv;
	StrCpy(rstShowData.m_szDeclaration, rstAsyncPvpInfo.m_szDeclaration, MAX_SIGNATURE_LENGTH);

	DT_BATTLE_ARRAY_INFO* pstBattleInfo = Majesty::Instance().GetBattleArrayInfo(pstData, BATTLE_ARRAY_TYPE_ASYNCPVP);
	if (pstBattleInfo == NULL)
	{
		return ERR_NOT_FOUND;
	}

	DT_ITEM_MSKILL* pstMSkillInfo = MasterSkill::Instance().GetMSkillInfo(pstData, pstBattleInfo->m_bMSId);
	if (pstMSkillInfo == NULL)
	{
		return ERR_NOT_FOUND;
	}

	rstShowData.m_dwMSkillId = pstMSkillInfo->m_bId;
	rstShowData.m_bMSkillLevel = pstMSkillInfo->m_bLevel;

    rstShowData.m_bCount = pstBattleInfo->m_bGeneralCnt;
    for (uint8_t i = 0; i < rstShowData.m_bCount; i++)
    {
        rstShowData.m_astGeneralList[i].m_dwGeneralID = pstBattleInfo->m_GeneralList[i];

        if (rstShowData.m_astGeneralList[i].m_dwGeneralID == 0)
        {
            continue;
        }

        DT_ITEM_GCARD* pstGCardInfo = GeneralCard::Instance().Find(pstData, rstShowData.m_astGeneralList[i].m_dwGeneralID);
        if (!pstGCardInfo)
        {
            return ERR_NOT_FOUND;
        }

        rstShowData.m_astGeneralList[i].m_bLv = pstGCardInfo->m_bLevel;
        rstShowData.m_astGeneralList[i].m_bStar = pstGCardInfo->m_bStar;
        rstShowData.m_astGeneralList[i].m_bGrade = pstGCardInfo->m_bPhase;
        rstShowData.m_astGeneralList[i].m_dwSkinId = pstGCardInfo->m_dwSkinId;
    }


    DT_ITEM_TACTICS* pstTactics = Tactics::Instance().GetTacticsItem(pstData, pstBattleInfo->m_bTacticsType);
    if (pstTactics == NULL)
        {
            LOGERR("Uin<%lu> the pstTacticsType<%hhu> is NULL!", pstData->m_ullUin, pstBattleInfo->m_bTacticsType);
            rstShowData.m_bTacticsType = 0;
            rstShowData.m_bTacticsLevel = 0;
        }
    else
    {
        rstShowData.m_bTacticsType = pstTactics->m_bType;
        rstShowData.m_bTacticsLevel = pstTactics->m_bLevel;
    }


    rstShowData.m_dwLeaderValue = pstData->m_dwLeaderValue;

    return ERR_NONE;
}


int AsyncPvp::GetTeamData(PlayerData* pstData, DT_ASYNC_PVP_PLAYER_TEAM_DATA& rstTeamData)
{
    rstTeamData.m_ullUin = pstData->GetRoleBaseInfo().m_ullUin;

	DT_BATTLE_ARRAY_INFO* pstBattleInfo = Majesty::Instance().GetBattleArrayInfo(pstData, BATTLE_ARRAY_TYPE_ASYNCPVP);
	if (pstBattleInfo == NULL)
	{
		return ERR_NOT_FOUND;
	}

    rstTeamData.m_bTroopNum = pstBattleInfo->m_bGeneralCnt;
    for (uint8_t i = 0; i < rstTeamData.m_bTroopNum; i++)
    {
        DT_TROOP_INFO& rstTroopInfo = rstTeamData.m_astTroopList[i];
        if (pstBattleInfo->m_GeneralList[i] == 0)
        {
            rstTroopInfo.m_stGeneralInfo.m_dwId = 0;
            continue;
        }
        bzero(rstTroopInfo.m_AttrAddValue, sizeof(uint32_t)*MAX_ATTR_ADD_NUM);

        DT_ITEM_GCARD* pstGCardInfo = GeneralCard::Instance().Find(pstData, pstBattleInfo->m_GeneralList[i]);
        if (pstGCardInfo == NULL)
        {
            return ERR_NOT_FOUND;
        }

        // 武将信息
        rstTroopInfo.m_stGeneralInfo = *pstGCardInfo;

        // 武将装备
        for (uint8_t j = 0; j < EQUIP_TYPE_MAX_NUM; j++)
        {
            uint32_t dwSeq = pstGCardInfo->m_astEquipList[j].m_dwEquipSeq;
            DT_ITEM_EQUIP* pstEquipInfo = Equip::Instance().Find(pstData, dwSeq);
            if (pstEquipInfo == NULL)
            {
                return ERR_NOT_FOUND;
            }
            rstTroopInfo.m_astEquipInfoList[j] = *pstEquipInfo;
        }
    }

    return ERR_NONE;
}

int AsyncPvp::CheckFight(PlayerData* pstData)
{
    DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = pstData->GetELOInfo().m_stAsyncPvpInfo;
    if (rstAsyncPvpInfo.m_bOpenFlag == 0)
    {
        return ERR_NOT_SATISFY_COND;
    }

    if (rstAsyncPvpInfo.m_bFightTimes >= rstAsyncPvpInfo.m_bFightTimesBuy + m_iFreeFightTimes)
    {
        return ERR_NOT_SATISFY_COND;
    }

    return ERR_NONE;
}

int AsyncPvp::SkipFight(PlayerData* pstData, uint8_t bFightCount, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
    DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = pstData->GetELOInfo().m_stAsyncPvpInfo;
    if (rstAsyncPvpInfo.m_bOpenFlag == 0)
    {
        return ERR_NOT_SATISFY_COND;
    }

    int iRet = 0;
    int iFightTimeBuy = (int)(rstAsyncPvpInfo.m_bFightTimes + bFightCount) - (int)(rstAsyncPvpInfo.m_bFightTimesBuy + m_iFreeFightTimes);

    //需要购买次数
    if (iFightTimeBuy > 0)
    {
        iRet = this->FightTimesBuy(pstData, (uint8_t)iFightTimeBuy, rstSyncItemInfo);
        if (iRet < 0)
        {
            return iRet;
        }
    }

    iRet = AsyncPvp::Instance().CheckColdTime(pstData);
    if(iRet != ERR_NONE)
    {
        iRet = AsyncPvp::Instance().CheckResetColdTime(pstData);
        if(iRet != ERR_NONE)
        {
            return iRet;
        }
    }

    rstAsyncPvpInfo.m_bFightTimes += bFightCount;

    int16_t wWinScore = (uint16_t)CGameDataMgr::Instance().GetResBasicMgr().Find((int)ASYNCPVP_SUCESS_GAIN_SCORE)->m_para[0];
    rstAsyncPvpInfo.m_wScore += wWinScore * bFightCount;

    Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_ASYNCPVP, bFightCount, 3, 1);
    Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_ASYNCPVP, bFightCount, 3, 3);
    Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_ASYNCPVP, wWinScore * bFightCount, 1);

    return ERR_NONE;
}


int AsyncPvp::CheckRefresh(PlayerData* pstData)
{
    DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = pstData->GetELOInfo().m_stAsyncPvpInfo;
    if (rstAsyncPvpInfo.m_bOpenFlag == 0)
    {
        return ERR_NOT_SATISFY_COND;
    }

    if (rstAsyncPvpInfo.m_bRefreshTimes < m_iFreeRefreshTimes)
    {
        return ERR_NONE;
    }

    int iDiamandRefreshTimes = rstAsyncPvpInfo.m_bRefreshTimes - m_iFreeRefreshTimes;
    if (iDiamandRefreshTimes > (int)(m_pResRefreshConsume->m_dwLvCount -1))
    {
        iDiamandRefreshTimes = m_pResRefreshConsume->m_dwLvCount -1;
    }

    if (!Consume::Instance().IsEnoughDiamond(pstData, m_pResRefreshConsume->m_lvList[iDiamandRefreshTimes]))
    {
        return ERR_NOT_ENOUGH_DIAMOND;
    }

    return ERR_NONE;
}

int AsyncPvp::AfterRefresh(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
    DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = pstData->GetELOInfo().m_stAsyncPvpInfo;
    if (++rstAsyncPvpInfo.m_bRefreshTimes <= m_iFreeRefreshTimes)
    {
        return ERR_NONE;
    }

    int iDiamandRefreshTimes = rstAsyncPvpInfo.m_bRefreshTimes - m_iFreeRefreshTimes;
    if (iDiamandRefreshTimes > (int)m_pResRefreshConsume->m_dwLvCount)
    {
        iDiamandRefreshTimes = m_pResRefreshConsume->m_dwLvCount;
    }

    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -m_pResRefreshConsume->m_lvList[iDiamandRefreshTimes-1],
                                rstSyncItemInfo, DWLOG::METHOD_ASYNC_PVP_REFRESH_LIST);
    return ERR_NONE;
}

int AsyncPvp::AfterStartFight(PlayerData* pstData)
{
    DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = pstData->GetELOInfo().m_stAsyncPvpInfo;
    rstAsyncPvpInfo.m_bFightTimes++;

    return ERR_NONE;
}

int AsyncPvp::FightTimesBuy(PlayerData* pstData, uint8_t bBuyCount, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
    DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = pstData->GetELOInfo().m_stAsyncPvpInfo;
    uint32_t dwDiamandConsume = 0;

    for (uint8_t i=0; i<bBuyCount; i++)
    {
        uint8_t bBuyTimes = (rstAsyncPvpInfo.m_bFightTimesBuy + i) > (m_pResFightBuyConsume->m_dwLvCount -1) ? \
                     (m_pResFightBuyConsume->m_dwLvCount -1) : (rstAsyncPvpInfo.m_bFightTimesBuy + i);

        dwDiamandConsume += m_pResFightBuyConsume->m_lvList[bBuyTimes];
    }

    if (!Consume::Instance().IsEnoughDiamond(pstData, dwDiamandConsume))
    {
        return ERR_NOT_ENOUGH_DIAMOND;
    }

    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -dwDiamandConsume, rstSyncItemInfo, DWLOG::METHOD_ASYNC_PVP_TIME_PUCHASE);

	//附赠重置冷却时间
	rstAsyncPvpInfo.m_ullCdTimeStamp = 0;
    rstAsyncPvpInfo.m_bFightTimesBuy += bBuyCount;

    return rstAsyncPvpInfo.m_bFightTimesBuy;
}

int AsyncPvp::GetTopList(DT_ASYNC_PVP_PLAYER_SHOW_DATA  astTopList[], uint64_t& ullTimeStamp)
{
    if (ullTimeStamp >= m_ullTopListTimeStamp)
    {
        return ERR_ALREADY_LATEST;
    }

    for (int i=0; i<MAX_RANK_TOP_NUM; i++)
    {
        astTopList[i] = m_astTopList[i];
    }

    ullTimeStamp = m_ullTopListTimeStamp;

    return ERR_NONE;
}

int AsyncPvp::Worship(PlayerData* pstData, CS_PKG_ASYNC_PVP_WORSHIP_REQ& rstCsReq, SC_PKG_ASYNC_PVP_WORSHIP_RSP& rstScRsp)
{
	DT_ROLE_ASYNC_PVP_INFO& rstAsyncpvpInfo = pstData->GetELOInfo().m_stAsyncPvpInfo;
	//int iRet = ERR_NONE;

	//被膜拜者收益
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ASYNC_PVP_WORSHIPPED_NTF;
	m_stSsPkg.m_stHead.m_ullUin = pstData->GetRoleBaseInfo().m_ullUin;
	SS_PKG_ASYNC_PVP_WORSHIPPED_NTF& rstSsNtf = m_stSsPkg.m_stBody.m_stAsyncpvpWorshippedNtf;
	rstSsNtf.m_bCount = 0;

	rstScRsp.m_bCount = 0;

	int32_t iRewardGold = 0;

	for (int i=0; i<rstCsReq.m_bCount; i++)
	{
		uint8_t bIndex = rstCsReq.m_szIndexList[i];

		if (rstAsyncpvpInfo.m_dwWorshipGold >= (uint32_t) m_iWorshipperGoldMax)
		{
			break;
		}

		if (rstAsyncpvpInfo.m_astTopTenPlayerList[bIndex].m_bIsWorshipped != 0)
		{
			continue;
		}

		iRewardGold += m_iWorshipperGoldOnce;

		rstAsyncpvpInfo.m_dwWorshipGold += m_iWorshipperGoldOnce;

		rstAsyncpvpInfo.m_astTopTenPlayerList[bIndex].m_bIsWorshipped = 1;

		rstScRsp.m_szIndexList[rstScRsp.m_bCount++] = bIndex;

		rstSsNtf.m_WorshippedList[rstSsNtf.m_bCount++] = rstAsyncpvpInfo.m_astTopTenPlayerList[bIndex].m_ullUin;
	}

	if (0!=iRewardGold)
	{
		Item::Instance().RewardItem(pstData, ITEM_TYPE_GOLD, 0, iRewardGold, rstScRsp.m_stSyncItemInfo, DWLOG::METHOD_WORSHIPPER);
	}


	if (rstSsNtf.m_bCount > 0)
	{
		rstSsNtf.m_iWorshippedGoldOnce = m_iWorshippedGoldOnce;
		rstSsNtf.m_iWorshippedGoldMax = m_iWorshippedGoldMax;

		ZoneSvrMsgLayer::Instance().SendToAsyncPvpSvr(m_stSsPkg);

		return ERR_NONE;
	}
	else
	{
		return ERR_MAX_WORSHIP_REWARD;
	}
}

void AsyncPvp::_FreshWorshipList(PlayerData* pstData, DT_ASYNC_PVP_PLAYER_SHOW_DATA* pastOpponentList)
{
	DT_ROLE_ASYNC_PVP_INFO& rstAsyncpvpInfo = pstData->GetELOInfo().m_stAsyncPvpInfo;
	for (int i=0; i<MAX_TOP_PLAYER_NUM; i++)
	{
		int j=0;
		for ( ; j<MAX_TOP_PLAYER_NUM; j++)
		{
			if (pastOpponentList[i].m_stBaseInfo.m_ullUin == rstAsyncpvpInfo.m_astTopTenPlayerList[j].m_ullUin)
			{
				pastOpponentList[i].m_bIsWorshipped = rstAsyncpvpInfo.m_astTopTenPlayerList[j].m_bIsWorshipped;
				break;
			}
		}
		if( MAX_TOP_PLAYER_NUM==j )
		{
			pastOpponentList[i].m_bIsWorshipped = 0;
		}
	}

	for (int i=0; i<MAX_TOP_PLAYER_NUM; i++)
	{
		rstAsyncpvpInfo.m_astTopTenPlayerList[i].m_bIsWorshipped = pastOpponentList[i].m_bIsWorshipped;
		rstAsyncpvpInfo.m_astTopTenPlayerList[i].m_ullUin = pastOpponentList[i].m_stBaseInfo.m_ullUin;
	}

	return;
}

void AsyncPvp::GetWorshipInfo(PlayerData* pstData, DT_ASYNC_PVP_PLAYER_SHOW_DATA* pastOpponentList, uint8_t bCount)
{
	assert(bCount>=10);
	_FreshWorshipList(pstData, pastOpponentList);
}

int AsyncPvp::AddWorshipGold(PlayerData* pstData, int32_t iGoldNum, SC_PKG_ASYNC_PVP_GET_WORSHIP_GOLD_RSP& rstScRsp)
{
	DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = pstData->GetELOInfo().m_stAsyncPvpInfo;
	//检查是否开启
	if (rstAsyncPvpInfo.m_bOpenFlag == 0)
	{
		return ERR_DEFAULT;
	}
	if (rstAsyncPvpInfo.m_dwWorshippedGold <(uint32_t) m_iWorshippedGoldMax)
	{
		Item::Instance().RewardItem(pstData, ITEM_TYPE_GOLD, 0, iGoldNum, rstScRsp.m_stSyncItemInfo, DWLOG::METHOD_WORSHIPPED);
		rstAsyncPvpInfo.m_dwWorshippedGold += iGoldNum;
		return ERR_NONE;
	}
	else
	{
		return ERR_MAX_WORSHIP_REWARD;
	}
}

void AsyncPvp::RefreshTopList(DT_ASYNC_PVP_PLAYER_SHOW_DATA  astTopList[], uint64_t ullTimeStamp)
{
    m_ullTopListTimeStamp = ullTimeStamp;

    for (int i=0; i<MAX_RANK_TOP_NUM; i++)
    {
        m_astTopList[i] = astTopList[i];
    }
}

int AsyncPvp::CheckColdTime(PlayerData* pstData)
{
	DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = pstData->GetELOInfo().m_stAsyncPvpInfo;
	uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();

	if (ullCurTime < rstAsyncPvpInfo.m_ullCdTimeStamp)
	{
		return ERR_IN_COLD;
	}

	return ERR_NONE;
}

int AsyncPvp::CheckResetColdTime(PlayerData* pstData)
{
	if(!Consume::Instance().IsEnough(pstData, ITEM_TYPE_DIAMOND, m_iColdResetCostDia))
	{
		return ERR_NOT_ENOUGH_DIAMOND;
	}

	return ERR_NONE;
}

void AsyncPvp::SettleResetDia(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
	Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -m_iColdResetCostDia, rstSyncItemInfo, DWLOG::METHOD_RESET_CD_TIME);
}

void AsyncPvp::UpdateCdTime(PlayerData* pstData, uint64_t& rullCdTimeStamp)
{
	DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = pstData->GetELOInfo().m_stAsyncPvpInfo;
	DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();

	uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();
	rullCdTimeStamp = ullCurTime + m_ullColdTimeList[rstMajestyInfo.m_bVipLv];
	rstAsyncPvpInfo.m_ullCdTimeStamp = rullCdTimeStamp;
}

int AsyncPvp::ResetCdTime(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
	DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = pstData->GetELOInfo().m_stAsyncPvpInfo;

	if(!Consume::Instance().IsEnough(pstData, ITEM_TYPE_DIAMOND, m_iColdResetCostDia))
	{
		return ERR_NOT_ENOUGH_DIAMOND;
	}

	Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -m_iColdResetCostDia, rstSyncItemInfo, DWLOG::METHOD_RESET_CD_TIME);

	//重置冷却时间
	rstAsyncPvpInfo.m_ullCdTimeStamp = 0;

	return ERR_NONE;
}

