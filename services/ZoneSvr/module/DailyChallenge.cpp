#include "DailyChallenge.h"
#include "GameTime.h"
#include "LogMacros.h"
#include "Item.h"
#include "FakeRandom.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "Task.h"
#include "dwlog_def.h"
#include "Message.h"
#include "Majesty.h"
#include "Match.h"
#include "ZoneLog.h"
#include "TaskAct.h"
using namespace PKGMETA;

bool DailyChallenge::Init()
{
    //获取胜场和失败上限
    ResDailyChallengeBasicMgr_t& rstBasicMgr = CGameDataMgr::Instance().GetResDailyChallengeBasicMgr();
    RESDAILYCHALLENGEBASIC* pResBasic = rstBasicMgr.Find(1);
    if (!pResBasic)
    {
        LOGERR("pResBasic is NULL");
        return false;
    }
    m_bWinTimesLimit = pResBasic->m_param[0];
    m_bLoseTimesLimit = pResBasic->m_param[1];

    //初始化时间戳和更新时间
    ResBasicMgr_t& rstResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
    RESBASIC* pUpdateTime = rstResBasicMgr.Find(COMMON_UPDATE_TIME);
    if (!pUpdateTime)
    {
        LOGERR("pUpdateTime is NULL");
        return false;
    }
    m_iUptTimeHour = (int)(pUpdateTime->m_para[0]);
    m_ullTimeStamp = CGameTime::Instance().GetSecOfHourInCurrDay(m_iUptTimeHour);
    if (CGameTime::Instance().GetCurrHour() < m_iUptTimeHour)
    {
        m_ullTimeStamp -= SECONDS_OF_DAY;
    }
    m_bUptFlag = false;

	RESBASIC* poBasicMorale = rstResBasicMgr.Find(BASIC_MORALE);              // 初始阶段
	if (!poBasicMorale)
	{
		LOGERR("poBasicMorale is null.");
		return false;
	}
	m_fMoraleMax = poBasicMorale->m_para[0];
    m_fInitMorale = poBasicMorale->m_para[2];

    RESBASIC* poBasicRequireLv = rstResBasicMgr.Find(DAILY_CHALLENGE_REQUIRE_LV);
    if (!poBasicRequireLv)
    {
        LOGERR("poBasicRequireLv is null.");
        return false;
    }
    m_bRequireLevel = poBasicRequireLv->m_para[0];

	for (int i = 0; i < MAX_VIP_LEVEL+1; i++)
	{
		RESVIP* pResVip = CGameDataMgr::Instance().GetResVIPMgr().Find(i+1);
		if (!pResVip)
		{
			LOGERR("pResVip is null");
			return false;
		}

		m_wBuffNum[i] = pResVip->m_bDailyChaBuffNum;

	}

    for (int i = 0; i < MAX_BUFF_TYPE_NUM; i++)
    {
        m_bRandomList[i] = i;
    }

    m_oRandomNodeMap.clear();

    return true;
}


int DailyChallenge::CheckMatch(PlayerData* pstData)
{
    DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChallengeInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;
    if (rstDailyChallengeInfo.m_bState != DAILY_CHALLENGE_STATE_GOING_ON)
    {
        LOGERR("Player(%s) Uin(%lu) can't match, state(%d)", pstData->GetRoleBaseInfo().m_szRoleName,
                pstData->m_ullUin, rstDailyChallengeInfo.m_bState);
        return ERR_NOT_SATISFY_COND;
    }

    if ( m_bRequireLevel > pstData->GetMajestyInfo().m_wLevel)
    {
        LOGERR("level<%d> is less than required<%d>.", pstData->GetMajestyInfo().m_wLevel, m_bRequireLevel);
        return ERR_MAJESTY_UN_SATISFY;
    }

    return ERR_NONE;
}


void DailyChallenge::UpdatePlayerData(PlayerData* pstData)
{
    DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChallengeInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;
    if (rstDailyChallengeInfo.m_ullTimestamp >= m_ullTimeStamp)
    {
        return;
    }

    if (rstDailyChallengeInfo.m_bState == DAILY_CHALLENGE_STATE_END)
    {
        rstDailyChallengeInfo.m_bState = DAILY_CHALLENGE_STATE_GOING_ON;
    }
    else if (rstDailyChallengeInfo.m_bState == DAILY_CHALLENGE_STATE_GOING_ON && rstDailyChallengeInfo.m_bRewardWinCount > 0)
    {
        rstDailyChallengeInfo.m_bState = DAILY_CHALLENGE_STATE_WAIT_RECV;
        rstDailyChallengeInfo.m_ullRewardTimestamp = rstDailyChallengeInfo.m_ullTimestamp;
    }

    this->_ResetPlayerData(pstData);
}


void DailyChallenge::UpdateServer()
{
    int iHour = CGameTime::Instance().GetCurrHour();
    if ((iHour == m_iUptTimeHour) && m_bUptFlag)
    {
        m_bUptFlag = false;
        m_ullTimeStamp = CGameTime::Instance().GetSecOfHourInCurrDay(m_iUptTimeHour);
        //结算排行榜
        this->_SendSettleRankNtf();
        //清空排行榜
        this->_SendClearRankNtf();
    }
    else if (iHour != m_iUptTimeHour)
    {
        m_bUptFlag = true;
    }
}


int DailyChallenge::FightSettle(PlayerData* pstData, uint8_t bResult, DT_SYNC_ITEM_INFO& rstItemInfo)
{
    DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChallengeInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();

    if (rstDailyChallengeInfo.m_bState != DAILY_CHALLENGE_STATE_GOING_ON)
    {
        return ERR_NOT_SATISFY_COND;
    }

    (bResult == 1) ? rstDailyChallengeInfo.m_bWinCount++ : rstDailyChallengeInfo.m_bLoseCount++;
    rstDailyChallengeInfo.m_bRewardWinCount = rstDailyChallengeInfo.m_bWinCount;

    //不再检查失败次数
    if (/*rstDailyChallengeInfo.m_bLoseCount >= m_bLoseTimesLimit ||*/
        rstDailyChallengeInfo.m_bWinCount >= m_bWinTimesLimit)
    {
        rstDailyChallengeInfo.m_bState = DAILY_CHALLENGE_STATE_WAIT_RECV;
        rstDailyChallengeInfo.m_ullRewardTimestamp = m_ullTimeStamp;
    }

    //获胜后需要更新积分,并发奖励
    if (bResult == 1)
    {
        RESDAILYCHALLENGEMATCH* pResMatch =    CGameDataMgr::Instance().GetResDailyChallengeMatchMgr().Find(rstDailyChallengeInfo.m_bWinCount);
        if (!pResMatch)
        {
            LOGERR("pResMatch is NULL");
            return ERR_SYS;
        }

        RESDAILYCHALLENGEFIGHTREWARD* pResReward =    CGameDataMgr::Instance().GetResDailyChallengeFightRewardMgr().Find(rstDailyChallengeInfo.m_bWinCount);
        if (!pResReward)
        {
            LOGERR("pResReward is NULL");
            return ERR_SYS;
        }

        RESVIP* pResVip = CGameDataMgr::Instance().GetResVIPMgr().Find(rstMajestyInfo.m_bVipLv + 1);
        if (!pResVip)
        {
            LOGERR("pResVip is NULL");
            return ERR_SYS;
        }

        //发奖励
        int iDouble = 1;
        if (TaskAct::Instance().IsActTypeOpen(pstData, ACT_TYPE_DOUBLE_DAILY_CHANLLENGE))
        {
            iDouble = 2;
        }
        for (int i=0; i<pResReward->m_bRewardCount; i++)
        {
            Item::Instance().RewardItem(pstData, pResReward->m_szRewardTypeList[i], pResReward->m_rewardIdList[i],
                                        pResReward->m_rewardCountList[i] * iDouble, rstItemInfo, DWLOG::DAILY_CHALLENGE_WIN_COUNT_REWARD, Item::CONST_SHOW_PROPERTY_NORMAL);
        }

        //更新积分
        rstDailyChallengeInfo.m_dwScore = rstMajestyInfo.m_dwHighestLi * pResMatch->m_fScoreRatio * pResVip->m_fScoreRatio;

		if (rstDailyChallengeInfo.m_bWinCount > rstDailyChallengeInfo.m_bHighestWinCount)
		{
			rstDailyChallengeInfo.m_bHighestWinCount = rstDailyChallengeInfo.m_bWinCount;
		}

        //更新积分排名
        this->_UpdateScoreRank(pstData);

        //广播消息
        if (rstDailyChallengeInfo.m_bWinCount == 6)
        {
            Message::Instance().AutoSendSysMessage(1701, "Name=%s", pstData->GetRoleName());
            Message::Instance().AutoSendWorldMessage(pstData, 1704);
        }
        else if (rstDailyChallengeInfo.m_bWinCount == 9)
        {
            Message::Instance().AutoSendSysMessage( 1702, "Name=%s", pstData->GetRoleName());
            Message::Instance().AutoSendWorldMessage(pstData, 1705);
        }
        else if (rstDailyChallengeInfo.m_bWinCount == 12)
        {
            Message::Instance().AutoSendSysMessage( 1703, "Name=%s", pstData->GetRoleName());
            Message::Instance().AutoSendWorldMessage(pstData,  1706);
        }

    }

    return ERR_NONE;
}


int DailyChallenge::RecvReward(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstItemInfo)
{
    DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChallengeInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;
    if (rstDailyChallengeInfo.m_bState != DAILY_CHALLENGE_STATE_WAIT_RECV)
    {
        return ERR_NOT_SATISFY_COND;
    }
	uint32_t dwAwardId = 0;
	if (pstData->GetLv() > 50)
	{
		//取另一档奖励数据
		dwAwardId = rstDailyChallengeInfo.m_bRewardWinCount + CGameDataMgr::Instance().GetResDailyChallengeFinialRewardMgr().GetResNum() / 2;
	}
	else
	{
		dwAwardId = rstDailyChallengeInfo.m_bRewardWinCount;
	}
    RESDAILYCHALLENGEFINIALREWARD* pResReward =    CGameDataMgr::Instance().GetResDailyChallengeFinialRewardMgr().Find(dwAwardId);
    if (pResReward)
    {
        //发奖励
        for (int i=0; i<pResReward->m_bRewardCount; i++)
        {
            Item::Instance().RewardItem(pstData, pResReward->m_szRewardTypeList[i], pResReward->m_rewardIdList[i],
                                        pResReward->m_rewardCountList[i], rstItemInfo, 0, Item::CONST_SHOW_PROPERTY_NORMAL);
        }
    }

    //领取完奖励后需要清空数据
    rstDailyChallengeInfo.m_bRewardWinCount = 0;
    rstDailyChallengeInfo.m_bState = rstDailyChallengeInfo.m_ullRewardTimestamp < m_ullTimeStamp ? DAILY_CHALLENGE_STATE_GOING_ON : DAILY_CHALLENGE_STATE_END;

    return ERR_NONE;
}


int DailyChallenge::GenFakePlayer(PlayerData* pstData, DT_FIGHT_PLAYER_INFO& rstPlayerInfo, OUT uint32_t* pdwLi)
{
    rstPlayerInfo.m_bTroopNum = MAX_TROOP_NUM_PVP;
    int iRet = this->_GenFakeGeneralList(pstData, rstPlayerInfo);
    if (iRet != ERR_NONE)
    {
        LOGERR("GenFakeGeneralList Failed, Ret=%d", iRet);
        return iRet;
    }

    DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChallengeInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();

    // 屏蔽假玩家的阵法
    rstPlayerInfo.m_bTacticsType = 0;
    rstPlayerInfo.m_bTacticsLevel = 0;


    RESDAILYCHALLENGEMATCH* pResMatch =    CGameDataMgr::Instance().GetResDailyChallengeMatchMgr().Find(rstDailyChallengeInfo.m_bWinCount+1);
    assert(pResMatch);

    uint32_t dwLi = rstMajestyInfo.m_dwHighestLi * pResMatch->m_fForceRatio /MAX_TROOP_NUM_PVP;
    LOGRUN("GenFakeGeneralList, Li=%u, MyHighestLi=%u, ForceRatio=%f",
            dwLi, rstMajestyInfo.m_dwHighestLi, pResMatch->m_fForceRatio);

    if (pdwLi != NULL)
    {
        *pdwLi = dwLi;
    }

    for (int i=0; i<MAX_TROOP_NUM_PVP; i++)
    {
        iRet = this->_GenTroopData(dwLi, rstPlayerInfo.m_astTroopList[i]);
        if (iRet != ERR_NONE)
        {
            LOGERR("GenTroopData Failed, Ret=%d", iRet);
            return iRet;
        }
    }

    return ERR_NONE;
}


int DailyChallenge::SkipFight(PlayerData* pstData, uint8_t bWinCount, DT_SYNC_ITEM_INFO& rstSyncInfo)
{
    DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChallengeInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();

    if (rstDailyChallengeInfo.m_bState != DAILY_CHALLENGE_STATE_GOING_ON)
    {
        return ERR_NOT_SATISFY_COND;
    }

    //TODO:校验
	if (!_CheckShipNumIsLegal(pstData, bWinCount))
	{
		return ERR_NOT_SATISFY_COND;
	}


    //发奖励
	int i = rstDailyChallengeInfo.m_bWinCount == 0 ? 1 : rstDailyChallengeInfo.m_bWinCount;
    for (; i<=bWinCount; i++)
    {
        RESDAILYCHALLENGEFIGHTREWARD* pResReward =    CGameDataMgr::Instance().GetResDailyChallengeFightRewardMgr().Find(i);
        if (!pResReward)
        {
            LOGERR("pResReward is NULL,index is:%d", i);
            return ERR_SYS;
        }

        //发奖励
        for (int j=0; j<pResReward->m_bRewardCount; j++)
        {
            Item::Instance().RewardItem(pstData, pResReward->m_szRewardTypeList[j], pResReward->m_rewardIdList[j],
                                        pResReward->m_rewardCountList[j], rstSyncInfo, 0, Item::CONST_SHOW_PROPERTY_NORMAL);
        }
    }

    RESVIP* pResVip = CGameDataMgr::Instance().GetResVIPMgr().Find(rstMajestyInfo.m_bVipLv + 1);
    if (!pResVip)
    {
        LOGERR("pResVip is NULL");
        return ERR_SYS;
    }

    RESDAILYCHALLENGEMATCH* pResMatch =    CGameDataMgr::Instance().GetResDailyChallengeMatchMgr().Find(bWinCount);
    if (!pResMatch)
    {
        LOGERR("pResMatch is NULL");
        return ERR_SYS;
    }
    //更新胜场
    rstDailyChallengeInfo.m_bWinCount = bWinCount;
    rstDailyChallengeInfo.m_bRewardWinCount = bWinCount;

    //更新积分
    rstDailyChallengeInfo.m_dwScore = rstMajestyInfo.m_dwHighestLi * pResMatch->m_fScoreRatio * pResVip->m_fScoreRatio;

    //更新积分排名
    this->_UpdateScoreRank(pstData);
	for (int i = 0; i < bWinCount; i++)
	{
		Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_PVP, 1/*value*/, 5, 3/*参与*/);
		Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_PVP, 1/*value*/, 5, 1/*胜利*/);
	}

    return ERR_NONE;
}

int DailyChallenge::SetSiegeEquipment(PlayerData* pstData, uint8_t bType)
{
	DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChaInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;

	int iRet = ERR_NONE;
	do
	{
		if (bType > 2)
		{
			LOGERR("Wrong Para SiegeEquipmentType<%d>", bType);
			iRet = ERR_WRONG_PARA;
			break;
		}

		rstDailyChaInfo.m_bSiegeEquipment = bType;

	} while (false);

	return iRet;
}

int DailyChallenge::PurchaseBuff(PlayerData* pstData, CS_PKG_DAILY_CHALLENGE_BUY_BUFF_REQ& rstCsReq, SC_PKG_DAILY_CHALLENGE_BUY_BUFF_RSP& rstScRsp)
{
	DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChaInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;
	DT_DAILY_CHALLENGE_BUFFS& rstBuffInfo = rstDailyChaInfo.m_stBuffs;
	DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
    rstScRsp.m_stSelfTroop.m_bCount = 0;

	int iRet = ERR_NONE;
	do
	{
		if (rstCsReq.m_bCount + rstDailyChaInfo.m_bBuyBuffTimeToday > m_wBuffNum[rstMajestyInfo.m_bVipLv])
		{
			LOGERR("the left buff buy time<%d> is not enough. BuyCount<%d>", m_wBuffNum[rstMajestyInfo.m_bVipLv] - rstDailyChaInfo.m_bBuyBuffTimeToday, rstCsReq.m_bCount);
			iRet = ERR_WRONG_PARA;
			break;
		}

		for (int i = 0; i < rstCsReq.m_bCount; i++)
		{
			uint8_t bIndex = rstCsReq.m_szBuffIndexList[i];
			//改栏位的buf是否已被购买
			if (rstBuffInfo.m_szIsSelectedList[bIndex] != 0)
			{
				LOGERR("the bIndex<%d> is sold out.", bIndex);
				iRet = ERR_WRONG_PARA;
				break;
			}
		}

		if (iRet != ERR_NONE)
		{
			break;
		}

		for (int i = 0; i < rstCsReq.m_bCount; i++)
		{
			uint8_t bIndex = rstCsReq.m_szBuffIndexList[i];

			//该栏位已使用
			rstBuffInfo.m_szIsSelectedList[bIndex] = 1;
			//今日栏位已购买次数
			rstDailyChaInfo.m_bBuyBuffTimeToday++;
			//处理buff信息
			if (rstDailyChaInfo.m_stBuffs.m_szRdBuffIndexList[bIndex] == BUFF_ADD_MORALE)
			{
				_AddBuffBenifitMorale(pstData, rstScRsp.m_stSelfTroop);
			}
			if (rstDailyChaInfo.m_stBuffs.m_szRdBuffIndexList[bIndex] == BUFF_ADD_HP)
			{
				_AddBuffBenifitHp(pstData, rstScRsp.m_stSelfTroop);
			}
		}

		memcpy(&rstScRsp.m_stBuffInfo, &rstBuffInfo, sizeof(DT_DAILY_CHALLENGE_BUFFS));
		rstScRsp.m_bBuyTimeUsed = rstDailyChaInfo.m_bBuyBuffTimeToday;
        rstScRsp.m_stSelfTroop.m_nCurMorale = rstDailyChaInfo.m_stGeneralsInfo.m_nCurMorale;
        rstScRsp.m_stSelfTroop.m_dwCityHpLoss = rstDailyChaInfo.m_stGeneralsInfo.m_dwCityHpLoss;

	} while (false);

	return iRet;
}

void DailyChallenge::UpdateTroopInfo(PlayerData* pstData, uint8_t bResult, DT_DAILY_CHALLENGE_TROOP_INFO& rstSelfTroopInfo, DT_DAILY_CHALLENGE_TROOP_INFO& rstEnemyTroopInfo)
{
	DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChallengeInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;

    ZoneLog::Instance().WriteDailyChallengeLog(pstData, rstDailyChallengeInfo);

     assert( rstDailyChallengeInfo.m_bIsGotFightDungeonInfo <= 1 );

	if (bResult == 1)
	{
        _RandomBuffs(pstData);

        //重置敌方部队血量
		rstDailyChallengeInfo.m_stEnemyTroopInfo.m_bCount = 0;
		rstDailyChallengeInfo.m_stEnemyTroopInfo.m_dwCityHpLoss = 0;
		rstDailyChallengeInfo.m_stEnemyTroopInfo.m_nCurMorale = m_fInitMorale;

        //胜利进入下一层
        rstDailyChallengeInfo.m_bIsGotFightDungeonInfo = 0;
        rstDailyChallengeInfo.m_bAlreadyInCurrentLevel = 0;
	}
	else
	{
		//更新敌方信息
		for (int i = 0; i < rstEnemyTroopInfo.m_bCount; i++)
		{
			int iGeneralIndex = _IsEnemyGetHurt(pstData, rstEnemyTroopInfo.m_astDCGCardList[i].m_dwGeneralId);
			if (iGeneralIndex < 0)
			{
                if (rstDailyChallengeInfo.m_stEnemyTroopInfo.m_bCount >= MAX_TROOP_NUM_PVP)
                {
                    LOGERR("EnemyTroopInfoList count<%d> is larger than %d",rstDailyChallengeInfo.m_stEnemyTroopInfo.m_bCount, MAX_TROOP_NUM_PVP);
                    continue;
                }
                memcpy(&rstDailyChallengeInfo.m_stEnemyTroopInfo.m_astDCGCardList[rstDailyChallengeInfo.m_stEnemyTroopInfo.m_bCount++], &rstEnemyTroopInfo.m_astDCGCardList[i], sizeof(DT_DAILY_CHALLENGE_GCARD_INFO));
				continue;
			}
			memcpy(&rstDailyChallengeInfo.m_stEnemyTroopInfo.m_astDCGCardList[iGeneralIndex], &rstEnemyTroopInfo.m_astDCGCardList[i], sizeof(DT_DAILY_CHALLENGE_GCARD_INFO));
		}

		rstDailyChallengeInfo.m_stEnemyTroopInfo.m_nCurMorale = rstEnemyTroopInfo.m_nCurMorale;
		rstDailyChallengeInfo.m_stEnemyTroopInfo.m_dwCityHpLoss = rstEnemyTroopInfo.m_dwCityHpLoss;

        //失败停留在该层
        rstDailyChallengeInfo.m_bIsGotFightDungeonInfo = 1;
        rstDailyChallengeInfo.m_bAlreadyInCurrentLevel = 1;
	}

	//更新我方信息
	for (int i = 0; i < rstSelfTroopInfo.m_bCount; i++)
	{
		int iGeneralIndex = _IsGeneralGetHurt(pstData, rstSelfTroopInfo.m_astDCGCardList[i].m_dwGeneralId);
		if (iGeneralIndex < 0)
		{
            if (rstDailyChallengeInfo.m_stGeneralsInfo.m_bCount >= MAX_NUM_ROLE_GCARD)
            {
                LOGERR("SelfTroopInfoList count<%d> is larger than %d",rstDailyChallengeInfo.m_stGeneralsInfo.m_bCount, MAX_TROOP_NUM_PVP);
                continue;
            }
            memcpy(&rstDailyChallengeInfo.m_stGeneralsInfo.m_astDCGCardList[rstDailyChallengeInfo.m_stGeneralsInfo.m_bCount++], &rstSelfTroopInfo.m_astDCGCardList[i], sizeof(DT_DAILY_CHALLENGE_GCARD_INFO));
            continue;
		}
		memcpy(&rstDailyChallengeInfo.m_stGeneralsInfo.m_astDCGCardList[iGeneralIndex], &rstSelfTroopInfo.m_astDCGCardList[i], sizeof(DT_DAILY_CHALLENGE_GCARD_INFO));
	}
    rstDailyChallengeInfo.m_stGeneralsInfo.m_nCurMorale = rstSelfTroopInfo.m_nCurMorale;
    rstDailyChallengeInfo.m_stGeneralsInfo.m_dwCityHpLoss = rstSelfTroopInfo.m_dwCityHpLoss;
}

int DailyChallenge::CheckGenerals(PlayerData* pstData)
{


    DT_BATTLE_ARRAY_INFO* pstBattleArrayInfo = Majesty::Instance().GetBattleArrayInfo(pstData, BATTLE_ARRAY_TYPE_DAILYCHALLENGE);
    if (pstBattleArrayInfo == NULL)
    {
        LOGERR("pstBattleArrayInfo is NULL.");
        return ERR_SYS;
    }

    int iRet = ERR_NONE;
    for (int i = 0; i < pstBattleArrayInfo->m_bGeneralCnt; i++)
    {
        if (_CheckGeneralAlive(pstData, pstBattleArrayInfo->m_GeneralList[i]) < 0)
        {
            iRet = ERR_GENERAL_ALREADY_DEAD;
            break;
        }
    }

    return iRet;

}


bool DailyChallenge::_CheckShipNumIsLegal(PlayerData* pstData, uint8_t bWinCount)
{
	DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChallengeInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;
	DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();

	ResVIPMgr_t& rResVipMgr = CGameDataMgr::Instance().GetResVIPMgr();
	ResDailyChallengeSkipFightMgr_t& rResDailyChallengeSkipFightMgr = CGameDataMgr::Instance().GetResDailyChallengeSkipFightMgr();

	uint8_t bMaxSkipNum = 0;

	RESDAILYCHALLENGESKIPFIGHT* pstDailyChallengeSkipFight = rResDailyChallengeSkipFightMgr.Find(rstMajestyInfo.m_wLevel);
	if (!pstDailyChallengeSkipFight)
	{
		LOGERR("pstDailyChallengeSkipFight is null.");
		return false;
	}

	bMaxSkipNum = pstDailyChallengeSkipFight->m_bSkipCount;

	RESVIP* pstVip = rResVipMgr.Find(rstMajestyInfo.m_bVipLv+1);
	if (!pstVip)
	{
		LOGERR("pstVip is null.");
		return false;
	}

	bMaxSkipNum = pstVip->m_bSyncpvpSkipNum > bMaxSkipNum ? pstVip->m_bSyncpvpSkipNum : bMaxSkipNum;

	bMaxSkipNum = rstDailyChallengeInfo.m_bHighestWinCount < bMaxSkipNum ? rstDailyChallengeInfo.m_bHighestWinCount : bMaxSkipNum;

	if (bWinCount <= bMaxSkipNum)
	{
		return true;
	}
	else
	{
		return false;
	}

}

int DailyChallenge::_GenTroopData(uint32_t dwLi, DT_TROOP_INFO& rstTroopInfo)
{
    LOGRUN("GenTroopData, Li(%u)", dwLi);

    RESGENERAL* poResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(rstTroopInfo.m_stGeneralInfo.m_dwId);
    if (!poResGeneral)
    {
        return ERR_SYS;
    }

    //一阶兵种
    RESARMY* poResArmy = CGameDataMgr::Instance().GetResArmyMgr().Find(1 * 100 + poResGeneral->m_bArmyType);
    if (!poResArmy)
    {
        return ERR_SYS;
    }

    RESDAILYCHALLENGEBASIC* pResBasic = CGameDataMgr::Instance().GetResDailyChallengeBasicMgr().Find(5);
    if (!pResBasic)
    {
        return ERR_SYS;
    }

    //计算战力中的技能系数
    float fSkillRatio = 0;

    DT_ITEM_GCARD& rstGCard = rstTroopInfo.m_stGeneralInfo;
    RESLIGCARDSKILL* poResGCardSkillWake = CGameDataMgr::Instance().GetResLiGCardSkillMgr().Find(poResGeneral->m_dwTalent - 10);
    if (!poResGCardSkillWake)
    {
        return ERR_SYS;
    }

    int tmpIndex = 0;
    for (uint8_t i = 0 ; i < MAX_NUM_GENERAL_SKILL; i++)
    {
        uint32_t dwId = CONST_GCARD_SKILL_DETA * (i + 1) + poResGeneral->m_dwTalent - 10;
        RESLIGCARDSKILL* poResGCardSkill = CGameDataMgr::Instance().GetResLiGCardSkillMgr().Find(dwId);
        if (!poResGCardSkill)
        {
            return ERR_SYS;
        }

        tmpIndex = rstGCard.m_szSkillLevel[i] > 0 ? int(rstGCard.m_szSkillLevel[i]-1) : 0;
        
        if (i == 1)
        {   //主动技 要乘以技能觉醒参数          
            fSkillRatio += poResGCardSkill->m_value[ tmpIndex ] * poResGCardSkillWake->m_value[1];
        }
        else
        {
            fSkillRatio += poResGCardSkill->m_value[ tmpIndex ];
        }
    }

    //兵种技对应ID
    uint32_t dwId = CONST_GCARD_SKILL_DETA * (MAX_NUM_GENERAL_SKILL + 1) + poResGeneral->m_dwTalent  - 10;
    RESLIGCARDSKILL* poResGCardSkill = CGameDataMgr::Instance().GetResLiGCardSkillMgr().Find(dwId);
    if (!poResGCardSkill)
    {
        return ERR_SYS;
    }
    tmpIndex = rstGCard.m_bArmyLv > 0 ? int(rstGCard.m_bArmyLv-1) : 0;
    fSkillRatio += poResGCardSkill->m_value[rstGCard.m_bArmyLv];
    fSkillRatio /= 3;

    float fSpecialRatio = 0;
    //初始化特殊属性
    switch (poResGeneral->m_bArmyType)
    {
        case ARMY_SHIELDMAN:
            rstTroopInfo.m_AttrAddValue[ATTR_BASE_CITYATK] = pResBasic->m_param[0];
            break;
        case ARMY_CAVALRY:
        case ARMY_SPEARMAN:
            rstTroopInfo.m_AttrAddValue[ATTR_BASE_CITYATK] = pResBasic->m_param[1];
            break;
        default:
            rstTroopInfo.m_AttrAddValue[ATTR_BASE_CITYATK] = pResBasic->m_param[3];
            break;
    }
    rstTroopInfo.m_AttrAddValue[ATTR_BASE_NORMALATK] = poResGeneral->m_dwBaseDamageAtkNormal;
    rstTroopInfo.m_AttrAddValue[ATTR_BASE_ARMYSKILL] = poResArmy->m_fBaseValue;

    rstTroopInfo.m_AttrAddValue[ATTR_CHANCE_HIT] = poResGeneral->m_fInitChanceHit + SPECIAL_ATTR_VALUE ;
    fSpecialRatio += rstTroopInfo.m_AttrAddValue[ATTR_CHANCE_HIT] / (100.0 - rstTroopInfo.m_AttrAddValue[ATTR_CHANCE_HIT]);

    rstTroopInfo.m_AttrAddValue[ATTR_CHANCE_DODGE] = poResGeneral->m_fInitChanceDodge + SPECIAL_ATTR_VALUE;
    fSpecialRatio += rstTroopInfo.m_AttrAddValue[ATTR_CHANCE_DODGE] / (100.0 -  rstTroopInfo.m_AttrAddValue[ATTR_CHANCE_DODGE]);

    rstTroopInfo.m_AttrAddValue[ATTR_CHANCE_CRITICAL] = poResGeneral->m_fInitChanceCritical + SPECIAL_ATTR_VALUE;
    fSpecialRatio += rstTroopInfo.m_AttrAddValue[ATTR_CHANCE_CRITICAL] /100.0 ;//* (poResGeneral->m_fInitParaCritical -1);

    rstTroopInfo.m_AttrAddValue[ATTR_CHANCE_ANTICRITICAL] = poResGeneral->m_fInitChanceAntiCritical + SPECIAL_ATTR_VALUE;
    fSpecialRatio += rstTroopInfo.m_AttrAddValue[ATTR_CHANCE_ANTICRITICAL] /100.0;// * (poResGeneral->m_fInitParaCritical-1);

    rstTroopInfo.m_AttrAddValue[ATTR_SUCKBLOOD] = poResGeneral->m_fInitParaSuckBlood + SPECIAL_ATTR_VALUE;
    fSpecialRatio += rstTroopInfo.m_AttrAddValue[ATTR_SUCKBLOOD]/100.0;

    fSpecialRatio /= 3;

    float fRatio = 1 + fSkillRatio + fSpecialRatio;

    LOGRUN("GenTroopData, General(%d) Ratio(%f)", rstGCard.m_dwId, fRatio);

    float fTotalAttrValue = dwLi / fRatio;

    RESLIGCARDBASE* poResLiGCardBase = CGameDataMgr::Instance().GetResLiGCardBaseMgr().Find(rstGCard.m_dwId);
    if (!poResLiGCardBase)
    {
        return ERR_SYS;
    }

    RESDAILYCHALLENGEARMYFORCE* poResArmyForce = CGameDataMgr::Instance().GetResDailyChallengeArmyForceMgr().Find(poResGeneral->m_bArmyType);
    if (!poResArmyForce)
    {
        return ERR_SYS;
    }

    float fFiveAttrPara = poResArmyForce->m_attributeList[0] * poResLiGCardBase->m_wHpFactor /1000.0f  +
                      poResArmyForce->m_attributeList[1] * poResLiGCardBase->m_wAtkFactor/1000.0f +
                      poResArmyForce->m_attributeList[2] * poResLiGCardBase->m_wWitFactor /1000.0f +
                      poResArmyForce->m_attributeList[3] * poResLiGCardBase->m_wAtkDefFactor /1000.0f +
                      poResArmyForce->m_attributeList[4] * poResLiGCardBase->m_wWitDefFactor /1000.0f;

    float fUintAttr = fTotalAttrValue/fFiveAttrPara;

    for (int i=ATTR_HP; i <= ATTR_WITDEF; i++)
    {
        rstTroopInfo.m_AttrAddValue[i] = poResArmyForce->m_attributeList[i-1] * fUintAttr;
    }

    for (int i=ATTR_HP; i <= ATTR_WITDEF; i++)
    {
        LOGRUN("GenTroopData, General(%u) AttrId(%d) AttrValue(%u)", rstGCard.m_dwId, i, rstTroopInfo.m_AttrAddValue[i]);
    }

    // 星级判断
    if( rstGCard.m_bStar==0 )
    {
        rstGCard.m_bStar = 1;
    }else if( rstGCard.m_bStar > 6 ) // TODO: 使用宏，最大星级
    {
        LOGERR("Attention!! DailyChallenge, general id<%u> star num<%u> is invalid!!", rstGCard.m_dwId, rstGCard.m_bStar);
        rstGCard.m_bStar = 6;
    }

    return ERR_NONE;
}


int DailyChallenge::_GenFakeGeneralList(PlayerData* pstData, DT_FIGHT_PLAYER_INFO& rstPlayerInfo)
{
    DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChallengeInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;
    RESDAILYCHALLENGEMATCH* pResMatch =    CGameDataMgr::Instance().GetResDailyChallengeMatchMgr().Find(rstDailyChallengeInfo.m_bWinCount+1);
    if (!pResMatch)
    {
        LOGERR("pResMatch is NULL, WinCount=%d", rstDailyChallengeInfo.m_bWinCount);
        return ERR_SYS;
    }

    int iCursor = CFakeRandom::Instance().Random(pResMatch->m_bBattalArrayCount);

    //从当前胜场的配置中随机选出一个阵容
    RESDAILYCHALLENGEBATTLEARRAY* pResBattleArray = CGameDataMgr::Instance().GetResDailyChallengeBattleArrayMgr().Find(pResMatch->m_battalArrayList[iCursor]);
    if (!pResBattleArray)
    {
        LOGERR("pResBattleArray is NULL, iCursor=%d, Id=%d", iCursor, pResMatch->m_battalArrayList[iCursor]);
        return ERR_SYS;
    }

    m_oRandomNodeMap.clear();

    //统计从各个池子中各抽取几个
    for (int i=0; i<MAX_TROOP_NUM_PVP; i++)
    {
        m_oIter = m_oRandomNodeMap.find(pResBattleArray->m_battleArray[i]);
        if (m_oIter == m_oRandomNodeMap.end())
        {
            RESDAILYCHALLENGEGENERALPOOL* pResPool = CGameDataMgr::Instance().GetResDailyChallengeGeneralPoolMgr().Find(pResBattleArray->m_battleArray[i]);
            if (!pResPool)
            {
                LOGERR("pResPool is Null, Id(%u) not found", pResBattleArray->m_battleArray[i]);
            }
            RandomNode stTemp;
            stTemp.m_bCount = 1;
            stTemp.m_pResPool = pResPool;
            m_oRandomNodeMap.insert(MapRandomNode_t::value_type((uint8_t)pResBattleArray->m_battleArray[i], stTemp));
        }
        else
        {
            m_oIter->second.m_bCount++;
        }
    }

    //从每个池子中各抽取相应数量的不重复的
    for (m_oIter=m_oRandomNodeMap.begin(); m_oIter!= m_oRandomNodeMap.end(); m_oIter++)
    {
        RandomNode& rstNode = m_oIter->second;
        CFakeRandom::Instance().Random((uint32_t)rstNode.m_pResPool->m_bGeneralCount, (uint32_t)rstNode.m_bCount, rstNode.m_RandomList);
    }

    //生成最终的阵容
    for (int i=0; i<MAX_TROOP_NUM_PVP; i++)
    {
        m_oIter = m_oRandomNodeMap.find(pResBattleArray->m_battleArray[i]);
        if (m_oIter != m_oRandomNodeMap.end())
        {
            RandomNode& rstNode = m_oIter->second;
            if (rstNode.m_bCount <= 0)
            {
                LOGERR("_GenFakeGeneralList failed");
                return ERR_SYS;
            }
            rstPlayerInfo.m_astTroopList[i].m_stGeneralInfo.m_dwId = rstNode.m_pResPool ->m_generalList[rstNode.m_RandomList[--rstNode.m_bCount]];
            LOGRUN("GenFakeGeneralList, General(%d) Id(%u)", i, rstPlayerInfo.m_astTroopList[i].m_stGeneralInfo.m_dwId);
        }
        else
        {
            LOGERR("_GenFakeGeneralList failed, BattleArray(%d) not found", pResBattleArray->m_battleArray[i]);
            return ERR_SYS;
        }
    }

    return ERR_NONE;
}


void DailyChallenge::_ResetPlayerData(PlayerData* pstData)
{
    DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChallengeInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;
    rstDailyChallengeInfo.m_bLoseCount = 0;
    rstDailyChallengeInfo.m_bWinCount = 0;
    rstDailyChallengeInfo.m_dwScore = 0;
    rstDailyChallengeInfo.m_ullTimestamp = m_ullTimeStamp;

    rstDailyChallengeInfo.m_bBuyBuffTimeToday = 0;
    rstDailyChallengeInfo.m_bIsGotFightDungeonInfo = 0;
    rstDailyChallengeInfo.m_bAlreadyInCurrentLevel = 0;
    rstDailyChallengeInfo.m_stEnemyTroopInfo.m_bCount = 0;
    rstDailyChallengeInfo.m_stEnemyTroopInfo.m_dwCityHpLoss = 0;
    rstDailyChallengeInfo.m_stEnemyTroopInfo.m_nCurMorale = m_fInitMorale;
    rstDailyChallengeInfo.m_stGeneralsInfo.m_bCount = 0;
    rstDailyChallengeInfo.m_stGeneralsInfo.m_dwCityHpLoss = 0;
    rstDailyChallengeInfo.m_stGeneralsInfo.m_nCurMorale = m_fInitMorale;

    //应策划要求，添加翌日阵容清理
    DT_BATTLE_ARRAY_INFO* pstBattleArray = Majesty::Instance().GetBattleArrayInfo(pstData, BATTLE_ARRAY_TYPE_DAILYCHALLENGE);
    if (pstBattleArray != NULL)
    {
        pstBattleArray->m_bGeneralCnt = 0;
    }
    else
    {
        LOGERR("pstBattleArray is null.Type<%d>", BATTLE_ARRAY_TYPE_DAILYCHALLENGE);
    }

    _RandomBuffs(pstData);
}


void DailyChallenge::_UpdateScoreRank(PlayerData* pstData)
{
    m_stSsPkg.m_stHead.m_ullUin = pstData->GetRoleBaseInfo().m_ullUin;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_DAILY_CHALLENGE_UPDATE_RANK_NTF;

    DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChallengeInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;

    SS_PKG_DAILY_CHALLENGE_UPDATE_RANK_NTF& rstNtf = m_stSsPkg.m_stBody.m_stDailyChallengeUptRankNtf;
    rstNtf.m_stSelfInfo.m_dwScore = rstDailyChallengeInfo.m_dwScore;
    rstNtf.m_stSelfInfo.m_ullTimeStampMs = CGameTime::Instance().GetCurrSecond();
    rstNtf.m_stSelfInfo.m_ullUin = pstData->GetRoleBaseInfo().m_ullUin;
    rstNtf.m_stSelfInfo.m_bWinCount = rstDailyChallengeInfo.m_bWinCount;
    StrCpy(rstNtf.m_stSelfInfo.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, MAX_NAME_LENGTH);

    ZoneSvrMsgLayer::Instance().SendToRankSvr(m_stSsPkg);
}


void DailyChallenge::_SendClearRankNtf()
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_DAILY_CHALLENGE_CLEAR_RANK_NTF;
    ZoneSvrMsgLayer::Instance().SendToRankSvr(m_stSsPkg);
}


void DailyChallenge::_SendSettleRankNtf()
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_DAILY_CHALLENGE_SETTLE_RANK_NTF;
    ZoneSvrMsgLayer::Instance().SendToRankSvr(m_stSsPkg);
}

int DailyChallenge::_AddBuffBenifitHp(PlayerData* pstData, DT_DAILY_CHALLENGE_TROOP_INFO& rstSelfTroop)
{
	DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChaInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;


    RESDAILYCHALLENGEBUFF* resDailyChaBuff = CGameDataMgr::Instance().GetResDailyChallengeBuffMgr().Find(rstDailyChaInfo.m_bWinCount+1);
    if (resDailyChaBuff == NULL)
    {
        LOGERR("resDailyChaBuff is not found. index<%d>", rstDailyChaInfo.m_bWinCount+1);
        return ERR_SYS;
    }

    DT_BATTLE_ARRAY_INFO* pstBattleArrayInfo = Majesty::Instance().GetBattleArrayInfo(pstData, BATTLE_ARRAY_TYPE_DAILYCHALLENGE);
    if (pstBattleArrayInfo == NULL)
    {
        LOGERR("pstBattleArrayInfo is NULL.");
        return ERR_SYS;
    }

	uint32_t dwHpHeal = resDailyChaBuff->m_buffList[BUFF_ADD_HP];

	for (int i = 0; i < pstBattleArrayInfo->m_bGeneralCnt; i++)
	{
		int iIndex = _IsGeneralGetHurt(pstData, (uint32_t)pstBattleArrayInfo->m_GeneralList[i]);
		if (iIndex < 0)
		{
			continue;
		}

		DT_DAILY_CHALLENGE_GCARD_INFO& rstGCardInfo = rstDailyChaInfo.m_stGeneralsInfo.m_astDCGCardList[iIndex];
        if (rstGCardInfo.m_bIsDeadToday)
        {
            continue;
        }

		if (dwHpHeal >= rstGCardInfo.m_dwLossHP)
		{
			rstGCardInfo.m_dwLossHP = 0;
		}
		else
		{
			rstGCardInfo.m_dwLossHP -= dwHpHeal;
		}

		memcpy(&rstSelfTroop.m_astDCGCardList[rstSelfTroop.m_bCount++], &rstGCardInfo, sizeof(DT_DAILY_CHALLENGE_GCARD_INFO));
	}

	return ERR_NONE;
}

int DailyChallenge::_AddBuffBenifitMorale(PlayerData* pstData, DT_DAILY_CHALLENGE_TROOP_INFO& rstSelfTroop)
{
    DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChaInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;


    RESDAILYCHALLENGEBUFF* resDailyChaBuff = CGameDataMgr::Instance().GetResDailyChallengeBuffMgr().Find(rstDailyChaInfo.m_bWinCount+1);
    if (resDailyChaBuff == NULL)
    {
        LOGERR("resDailyChaBuff is not found. index<%d>", rstDailyChaInfo.m_bWinCount+1);
        return ERR_SYS;
    }

    rstDailyChaInfo.m_stGeneralsInfo.m_nCurMorale += resDailyChaBuff->m_buffList[BUFF_ADD_MORALE];
    if (rstDailyChaInfo.m_stGeneralsInfo.m_nCurMorale > m_fMoraleMax)
    {
        rstDailyChaInfo.m_stGeneralsInfo.m_nCurMorale = m_fMoraleMax;
    }

    rstSelfTroop.m_nCurMorale = rstDailyChaInfo.m_stGeneralsInfo.m_nCurMorale;
    rstSelfTroop.m_dwCityHpLoss = rstDailyChaInfo.m_stGeneralsInfo.m_dwCityHpLoss;

    return ERR_NONE;
}


int DailyChallenge::_IsGeneralGetHurt(PlayerData* pstData, uint32_t dwGeneralId)
{
	DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChaInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;
	DT_DAILY_CHALLENGE_GENERALS_INFO& rstGeneralsInfo = rstDailyChaInfo.m_stGeneralsInfo;

	for (int i = 0; i < rstGeneralsInfo.m_bCount; i++)
	{
		if (dwGeneralId == rstGeneralsInfo.m_astDCGCardList[i].m_dwGeneralId)
		{
			return i;
		}
	}

	return -1;
}

int DailyChallenge::_IsEnemyGetHurt(PlayerData* pstData, uint32_t dwGeneralId)
{
	DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChaInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;
	DT_DAILY_CHALLENGE_TROOP_INFO& rstEnemyTroopInfo = rstDailyChaInfo.m_stEnemyTroopInfo;

	for (int i = 0; i < rstEnemyTroopInfo.m_bCount; i++)
	{
		if (dwGeneralId == rstEnemyTroopInfo.m_astDCGCardList[i].m_dwGeneralId)
		{
			return i;
		}
	}

	return -1;
}

int DailyChallenge::_RandomBuffs(PlayerData* pstData)
{
	DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChaInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;

    //随机buf
    int iRet = ERR_NONE;
    do
    {
        RESDAILYCHALLENGEBUFF* poDailyChallengeBuff = CGameDataMgr::Instance().GetResDailyChallengeBuffMgr().Find(rstDailyChaInfo.m_bWinCount+1);
        if (!poDailyChallengeBuff)
        {
            LOGERR("poDailyChallengeBuff is null. line<%d>", rstDailyChaInfo.m_bWinCount+1);
            iRet = ERR_SYS;
            break;
        }

        int iCount = 0;
        int iLeft = 0;
        int iRight = poDailyChallengeBuff->m_bCount - 1;

        while (iCount<MAX_BUFF_RANDOM_NUM)
        {
            uint8_t bRandomResult = (uint8_t)CFakeRandom::Instance().Random(iLeft, iRight);
            rstDailyChaInfo.m_stBuffs.m_szRdBuffIndexList[iCount++] = m_bRandomList[bRandomResult];
            int bTmp = m_bRandomList[bRandomResult];
            m_bRandomList[bRandomResult] = m_bRandomList[iRight];
            m_bRandomList[iRight] = bTmp;
            iRight--;
        }
    } while (false);

	if (iRet != ERR_NONE)
	{
        //出错默认0,1,2
        for (int i = 0; i < MAX_BUFF_RANDOM_NUM; i++)
        {
            rstDailyChaInfo.m_stBuffs.m_szRdBuffIndexList[i] = i;
        }
	}

    for (int i = 0; i < MAX_BUFF_RANDOM_NUM; i++)
    {
        rstDailyChaInfo.m_stBuffs.m_szIsSelectedList[i] = 0;
    }

    return iRet;
}

bool DailyChallenge::_CheckGeneralAlive(PlayerData* pstData, uint32_t dwGeneralId)
{
    DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChaInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;
    DT_DAILY_CHALLENGE_GENERALS_INFO& rstGeneralsInfo = rstDailyChaInfo.m_stGeneralsInfo;

    for (int i = 0; i < rstGeneralsInfo.m_bCount; i++)
    {
        if (dwGeneralId == rstGeneralsInfo.m_astDCGCardList[i].m_dwGeneralId)
        {
            if (rstGeneralsInfo.m_astDCGCardList[i].m_bIsDeadToday != 0)
            {
                return false;
            }
            else
            {
                return true;
            }
        }
    }

    return true;
}

int DailyChallenge::LoadDungeonInfo(PlayerData* pstData, uint32_t& rdwLi)
{
    int iRet = ERR_NONE;

    DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChaInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;

    do
    {
        //如果当层已随机
        if (rstDailyChaInfo.m_bIsGotFightDungeonInfo != 0)
        {
            break;
        }

        DT_FIGHT_DUNGEON_INFO& rstDungeonInfo = rstDailyChaInfo.m_stFightDungeonInfo;

        //iRet = Match::Instance().InitFightPlayerInfo(pstData, BATTLE_ARRAY_TYPE_DAILYCHALLENGE);
        //if (iRet != ERR_NONE)
        //{
        //    LOGERR("Player(%s) Uin(%lu) Init player info error", pstData->GetRoleBaseInfo().m_szRoleName, pstData->m_ullUin);
        //    break;
        //}

        rstDungeonInfo.m_bFakeType = MATCH_FAKE_OTHER;
        rstDungeonInfo.m_ullTimeStamp = CGameTime::Instance().GetCurrTimeMs();
        rstDungeonInfo.m_bFightPlayerNum = 2;
        rstDungeonInfo.m_bMatchType = MATCH_TYPE_DAILY_CHALLENGE;
        /*rstDungeonInfo.m_astFightPlayerList[0] = pstData->m_oSelfInfo;*/  //在DungeonCreateRsp_SS::HandleServerMsg填写己方阵容信息，查看敌方信息是不处理
        rstDungeonInfo.m_astFightPlayerList[0].m_chGroup = PLAYER_GROUP_DOWN;
        iRet = GenFakePlayer(pstData, rstDungeonInfo.m_astFightPlayerList[1], &rdwLi);
        if (iRet != ERR_NONE)
        {
            LOGERR("GenFakePlayer fail.");
            break;
        }
        rstDungeonInfo.m_astFightPlayerList[1].m_chGroup = PLAYER_GROUP_UP;
        for (int i=0; i < rstDungeonInfo.m_bFightPlayerNum; i++)
        {
            DT_FIGHT_PLAYER_INFO& rstPlayerInfo = rstDungeonInfo.m_astFightPlayerList[i];

            for (int j=0; j < rstPlayerInfo.m_bTroopNum; j++)
            {
                DT_TROOP_INFO& rstTroopInfo = rstPlayerInfo.m_astTroopList[j];
                rstTroopInfo.m_bId = MAX_TROOP_NUM * rstPlayerInfo.m_chGroup + j + 1;
                if (rstPlayerInfo.m_chGroup == PLAYER_GROUP_DOWN)
                {
                    // 回城卡片状态
                    rstTroopInfo.m_stInitPos.m_iPosX = 0;
                    rstTroopInfo.m_stInitPos.m_iPosY = -2000;
                }
                else
                {
                    // 回城卡片状态
                    rstTroopInfo.m_stInitPos.m_iPosX = 0;
                    rstTroopInfo.m_stInitPos.m_iPosY = 10000;
                }
            }
        }

        rstDailyChaInfo.m_bIsGotFightDungeonInfo = 1;
    } while (false);

    return iRet;
}

int DailyChallenge::LoadSelfFightPlayerInfo(PlayerData* pstData)
{
    DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChaInfo = pstData->GetELOInfo().m_stDailyChallengeInfo;

    DT_FIGHT_DUNGEON_INFO& rstDungeonInfo = rstDailyChaInfo.m_stFightDungeonInfo;

    int iRet = ERR_NONE;
    do
    {
        iRet = Match::Instance().InitFightPlayerInfo(pstData, BATTLE_ARRAY_TYPE_DAILYCHALLENGE);
        if (iRet != ERR_NONE)
        {
            break;
        }

        rstDungeonInfo.m_astFightPlayerList[0] = pstData->m_oSelfInfo;
    } while (false);

    return iRet;
}

