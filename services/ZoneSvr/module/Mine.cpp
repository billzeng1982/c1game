
#include "Mine.h"
#include "MasterSkill.h"
#include "Match.h"
#include "GameTime.h"
#include "Item.h"
#include "dwlog_svr.h"
#include "Lottery.h"
#include "Consume.h"
#include "Majesty.h"


bool Mine::Init()
{
    if (!LoadGameData())
    {
        LOGERR("LoadGameData ERROR!");
        return false;
    }
    return true;
}


void Mine::UpdateServer()
{
	if ((uint64_t) CGameTime::Instance().GetSecOfCycleSecInCurrDay(m_dwSettleBeinSecOfDay) > m_ullOpenBeginTime)
	{
		m_ullOpenBeginTime += SECONDS_OF_DAY;
		m_ullOpenEndTime += SECONDS_OF_DAY;
	}
	if ((uint64_t)CGameTime::Instance().GetCurrSecond() > m_ullSeasonEndTime)
	{
		m_ullSeasonEndTime += SECONDS_OF_WEEK;
		m_ullSeasonStartTime += SECONDS_OF_WEEK;
	}
}

bool Mine::LoadGameData()
{
    RESBASIC * pResBasic = NULL;
    pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_MINE_EXPLORE_PARA);
    if (!pResBasic)  return false;
    m_bNormalExploreCountMax = (uint8_t)pResBasic->m_para[0];
    m_dwExploreCountConsumeId = (uint32_t)pResBasic->m_para[1];



    pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_MINE_CHALLENGE_PARA);
    if (!pResBasic)  return false;
    m_bNormalChallengeCountMax = (uint8_t)pResBasic->m_para[0];
    m_dwChallengeCountConsumeId = (uint32_t)pResBasic->m_para[1];

    pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_MINE_REVENGE_PARA);
    if (!pResBasic)  return false;
    m_bNormalRevengeCountMax = (uint8_t)pResBasic->m_para[0];
    m_dwRevengeCountConsumeId = (uint32_t)pResBasic->m_para[1];

    pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_MINE_EXPLORE_PARA);
    if (!pResBasic)  return false;
    m_dwMaxGCardAP = (uint32_t)pResBasic->m_para[0];



    pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_MINE_MINING_BEGIN_TIME);
    if (!pResBasic)  return false;
    m_dwSettleBeinSecOfDay = pResBasic->m_para[0] * 3600 + pResBasic->m_para[1] * 60 + pResBasic->m_para[2];

    pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_MINE_MINING_END_TIME);
    if (!pResBasic)  return false;
    m_dwSettleEndSecOfDay = pResBasic->m_para[0] * 3600 + pResBasic->m_para[1] * 60 + pResBasic->m_para[2];


    m_dwDaySettleTime = m_dwSettleEndSecOfDay + pResBasic->m_para[3] * 60;
    m_ullCurSettleTime = CGameTime::Instance().GetSecOfCycleSecInCurrDay(m_dwDaySettleTime);
    m_ullOpenBeginTime = CGameTime::Instance().GetSecOfCycleSecInCurrDay(m_dwSettleBeinSecOfDay);
    m_ullOpenEndTime = m_ullOpenBeginTime - m_dwSettleBeinSecOfDay + m_dwSettleEndSecOfDay;

	pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_MINE_SEASON);
	if (!pResBasic)  return false;
	//	一周内的开放
	m_ullSeasonStartTime = CGameTime::Instance().GetSecByWeekDay(pResBasic->m_para[0], 0);
	m_ullSeasonEndTime = CGameTime::Instance().GetSecByWeekDay(pResBasic->m_para[1], 23) + 59 * 60 + 59;

	pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(9315);
	if (!pResBasic)	return false;
	m_dwLimitLv = (uint32_t) pResBasic->m_para[0];

	pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(9302);
	if (!pResBasic)	return false;
	m_fTeamLiRate = pResBasic->m_para[0];

    return true;
}


bool Mine::IsOpen()
{
    uint64_t ullNow = CGameTime::Instance().GetCurrSecond();
    return m_ullSeasonStartTime < ullNow && ullNow < m_ullSeasonEndTime &&  m_ullOpenBeginTime < ullNow && ullNow < m_ullOpenEndTime;
}


void Mine::UpdatePlayer(PlayerData& rstData)
{
	DT_ROLE_MINE_INFO& rstMineInfo = rstData.GetMineInfo();

	//刷新时间由功能开启时间判断
	if (rstMineInfo.m_ullLastUptTime < m_ullOpenBeginTime )
	{
		rstMineInfo.m_ullLastUptTime = m_ullOpenBeginTime;
		rstMineInfo.m_bChallengeBuyCount = 0;
		rstMineInfo.m_bExploreBuyCount = 0;
		rstMineInfo.m_bRevengeBuyCount = 0;
		rstMineInfo.m_bChallengeCount = 0;
		rstMineInfo.m_bRevengeCount = 0;
		rstMineInfo.m_bExploreCount = 0;
		GeneralCard::Instance().ClearMineAp(&rstData);
	}
}

bool Mine::IsExploreCountEnough(DT_ROLE_MINE_INFO& rstMineInfo)
{
    return rstMineInfo.m_bExploreCount < m_bNormalExploreCountMax + rstMineInfo.m_bExploreBuyCount;
}

bool Mine::IsChallengeCountEnough(DT_ROLE_MINE_INFO& rstMineInfo)
{
    return rstMineInfo.m_bChallengeCount < m_bNormalChallengeCountMax + rstMineInfo.m_bChallengeBuyCount;
}

bool Mine::IsRevengeCountEnough(DT_ROLE_MINE_INFO& rstMineInfo)
{
    return rstMineInfo.m_bRevengeCount < m_bNormalRevengeCountMax + rstMineInfo.m_bRevengeBuyCount;
}




bool Mine::IsGCardAPEnough(PlayerData& rstData)
{
	DT_BATTLE_ARRAY_INFO* pstBattleInfo = Majesty::Instance().GetBattleArrayInfo(&rstData, BATTLE_ARRAY_TYPE_MINE_CHALLENGE_BATTLE);
	if (!pstBattleInfo)
	{
		LOGERR("Uin<%lu> the battle info of BATTLE_ARRAY_TYPE_MINE_CHALLENGE_BATTLE is null", rstData.m_ullUin);
		return false;
	}
	for (uint32_t i = 0; i < pstBattleInfo->m_bGeneralCnt; i++)
	{
		if (pstBattleInfo->m_GeneralList[i] == 0)
		{
			continue;
		}
		DT_ITEM_GCARD* pstGCard = GeneralCard::Instance().Find(&rstData, pstBattleInfo->m_GeneralList[i]);
		if (!pstGCard)
		{
			LOGERR("Uin<%lu> SetGCardState the pstGCard<%u> is NULL", rstData.m_ullUin, pstBattleInfo->m_GeneralList[i]);
			continue;
		}
		if (pstGCard->m_bMineAP >= m_dwMaxGCardAP)
		{
			LOGERR("Uin<%lu> pstGCardInfo<%u> AP not enough", rstData.m_ullUin, pstBattleInfo->m_GeneralList[i]);
			return false;
		}
	}
	return true;
}

int Mine::GetAwardByAwardLog(PlayerData& rstData, uint8_t bAwardCount, DT_MINE_AWARD* pastAwardList, SC_PKG_MINE_GET_AWARD_RSP& rstRsp)
{
    for (int j = 0; j < bAwardCount; j++)
    {
        for (int i = 0; i < pastAwardList[j].m_bPropNum; i++)
        {
            if (pastAwardList[j].m_bState != COMMON_AWARD_STATE_AVAILABLE )
            {
                continue;
            }
            Item::Instance().RewardItem(&rstData, pastAwardList[j].m_astPropList[i].m_bItemType, 
				pastAwardList[j].m_astPropList[i].m_dwItemId,
                pastAwardList[j].m_astPropList[i].m_dwItemNum, rstRsp.m_stSyncItemInfo,
				DWLOG::METHOD_MINE_GET_AWARD_BY_ORE_PRODUCE, Item::CONST_SHOW_PROPERTY_NORMAL, true);
        }
    }
    return ERR_NONE;
}

int Mine::GetTroopInfo(PlayerData& rstData, uint8_t bMSId, uint32_t* pTroopFormation, OUT PKGMETA::SS_PKG_MINE_DEAL_ORE_REQ& rstReq)
{
    // 军师技
    DT_ITEM_MSKILL* pstMSkillInfo = MasterSkill::Instance().GetMSkillInfo(&rstData, bMSId);
    if (pstMSkillInfo == NULL)
    {
        LOGERR("Uin<%lu> the pstMSkillInfo<%hhu> is NULL! ", rstData.m_ullUin, bMSId);
        return ERR_MSKILL_NOT_FOUND;
    }
    memcpy(&rstReq.m_stMasterSkill, pstMSkillInfo, sizeof(rstReq.m_stMasterSkill));
    rstReq.m_bTroopCount = 0;
    for (int i = 0 ; i < MAX_MINE_TROOP_NUM; i++)
    {
        if (pTroopFormation[i] == 0)
        {
            continue;
        }
        DT_ITEM_GCARD* pstGCardInfo = GeneralCard::Instance().Find(&rstData, pTroopFormation[i]);
        if (pstGCardInfo == NULL)
        {
            LOGERR("Uin(%lu) pstGCardInfo<%u> is null", rstData.m_ullUin, pTroopFormation[i]);
            return ERR_NOT_FOUND;
        }

        if (GeneralCard::Instance().GetModuleState(pstGCardInfo, GeneralCard::MODULE_MINE_USE))
        {
            LOGERR("Uin(%lu) pstGCardInfo<%u> already used", rstData.m_ullUin, pTroopFormation[i]);
            return ERR_MINE_GCARD_IN_USE;
        }
        if (ERR_NONE != Match::Instance().InitOneTroopInfo(&rstData, pstGCardInfo, rstReq.m_astTroopInfo[rstReq.m_bTroopCount]))
        {
            LOGERR("Uin<%lu> InitTroopInfo<GCardId=%u> error! ", rstData.m_ullUin, pTroopFormation[i]);
            return ERR_TROOP_INIT_FAIL;
        }
        rstReq.m_bTroopCount++;
    }
    if (rstReq.m_bTroopCount == 0)
    {
        LOGERR("Uin<%lu> the pstMSkillInfo<%hhu> is NULL! ", rstData.m_ullUin, bMSId);
        return ERR_MINE_GCARD_NUM_IS_ZERO;
    }
    return ERR_NONE;
}





int Mine::InvestigateOre(PlayerData& rstData, DT_MINE_ORE_INFO& rstOreInfo, OUT PKGMETA::DT_SYNC_ITEM_INFO& rstSyncItem)
{
    RESMINE* pResMine = CGameDataMgr::Instance().GetResMineMgr().Find(rstOreInfo.m_dwOreId);
    if (!pResMine)
    {
        LOGERR("Uin<%lu> Investigate the pResMine<id %u> is NULL", rstData.m_ullUin, rstOreInfo.m_dwOreId);
        return ERR_SYS;
    }
    uint32_t dwPondId = pResMine->m_dwProduceTime;
    rstSyncItem.m_bSyncItemCount = 0;
    return Lottery::Instance().DrawLotteryByPond(&rstData, dwPondId, rstSyncItem, 
        DWLOG::METHOD_MINE_GET_AWARD_BY_ORE_INVESTIGATE);
}




int Mine::BuyExploreCount(PlayerData& rstData, OUT DT_SYNC_ITEM_INFO& rstSyncTiem)
{
    DT_ROLE_MINE_INFO& rstMineInfo = rstData.GetMineInfo();
    RESCONSUME* pResConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(m_dwExploreCountConsumeId);
    if (!pResConsume)
    {
        LOGERR("Uin<%lu> the pResConsume<%u> is NULL", rstData.m_ullUin, m_dwExploreCountConsumeId);
        return ERR_SYS;
    }
    if (rstMineInfo.m_bExploreBuyCount >= pResConsume->m_dwLvCount)
    {
        LOGERR("Uin<%lu> buy count is full ", rstData.m_ullUin);
        return ERR_BUY_COUNT_LIMIT;
    }
    if (!Item::Instance().IsEnough(&rstData, ITEM_TYPE_DIAMOND, 0, pResConsume->m_lvList[rstMineInfo.m_bExploreBuyCount]))
    {
        LOGERR("Uin<%lu> BuyExploreCount not enough diamond", rstData.m_ullUin);
        return ERR_NOT_ENOUGH_DIAMOND;
    }
    rstSyncTiem.m_bSyncItemCount = 0;
    Item::Instance().ConsumeItem(&rstData, ITEM_TYPE_DIAMOND, 0, 
        -pResConsume->m_lvList[rstMineInfo.m_bExploreBuyCount], rstSyncTiem, DWLOG::METHOD_MINE_BUY_COUNT_FOR_EXPLORE);
    rstMineInfo.m_bExploreBuyCount++;
    return ERR_NONE;
}

int Mine::BuyChallengeCount(PlayerData& rstData, OUT DT_SYNC_ITEM_INFO& rstSyncTiem)
{
    DT_ROLE_MINE_INFO& rstMineInfo = rstData.GetMineInfo();
    RESCONSUME* pResConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(m_dwChallengeCountConsumeId);
    if (!pResConsume)
    {
        LOGERR("Uin<%lu> the pResConsume<%u> is NULL", rstData.m_ullUin, m_dwChallengeCountConsumeId);
        return ERR_SYS;
    }
    if (rstMineInfo.m_bChallengeBuyCount >= pResConsume->m_dwLvCount)
    {
        LOGERR("Uin<%lu> buy count is full", rstData.m_ullUin);
        return ERR_BUY_COUNT_LIMIT;
    }
    if (!Item::Instance().IsEnough(&rstData, ITEM_TYPE_DIAMOND, 0, pResConsume->m_lvList[rstMineInfo.m_bChallengeBuyCount]))
    {
        LOGERR("Uin<%lu> BuyChallengeCount not enough diamond", rstData.m_ullUin);
        return ERR_NOT_ENOUGH_DIAMOND;
    }
    rstSyncTiem.m_bSyncItemCount = 0;
    Item::Instance().ConsumeItem(&rstData, ITEM_TYPE_DIAMOND, 0,
        -pResConsume->m_lvList[rstMineInfo.m_bChallengeBuyCount], rstSyncTiem, DWLOG::METHOD_MINE_BUY_COUNT_FOR_CHALLENGE);
    rstMineInfo.m_bChallengeBuyCount++;
    return ERR_NONE;
}

int Mine::BuyRevengeCount(PlayerData& rstData, OUT DT_SYNC_ITEM_INFO& rstSyncTiem)
{
    DT_ROLE_MINE_INFO& rstMineInfo = rstData.GetMineInfo();
    RESCONSUME* pResConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(m_dwRevengeCountConsumeId);
    if (!pResConsume)
    {
        LOGERR("Uin<%lu> the pResConsume<%u> is NULL", rstData.m_ullUin, m_dwRevengeCountConsumeId);
        return ERR_SYS;
    }
    if (rstMineInfo.m_bRevengeBuyCount >= pResConsume->m_dwLvCount)
    {
        LOGERR("Uin<%lu> buy count is full", rstData.m_ullUin);
        return ERR_BUY_COUNT_LIMIT;
    }
    if (!Item::Instance().IsEnough(&rstData, ITEM_TYPE_DIAMOND, 0, pResConsume->m_lvList[rstMineInfo.m_bRevengeBuyCount]))
    {
        LOGERR("Uin<%lu> BuyRevengeCount not enough diamond", rstData.m_ullUin);
        return ERR_NOT_ENOUGH_DIAMOND;
    }
    rstSyncTiem.m_bSyncItemCount = 0;
    Item::Instance().ConsumeItem(&rstData, ITEM_TYPE_DIAMOND, 0,
        -pResConsume->m_lvList[rstMineInfo.m_bRevengeBuyCount], rstSyncTiem, DWLOG::METHOD_MINE_BUY_COUNT_FOR_REVENGE);
    rstMineInfo.m_bRevengeBuyCount++;
    return ERR_NONE;
}


void Mine::ConsumeGCardAP(PlayerData& rstPData)
{
	DT_BATTLE_ARRAY_INFO* pstBattleInfo = Majesty::Instance().GetBattleArrayInfo(&rstPData, BATTLE_ARRAY_TYPE_MINE_CHALLENGE_BATTLE);
	if (!pstBattleInfo)
	{
		LOGERR("Uin<%lu> the battle info of BATTLE_ARRAY_TYPE_MINE_CHALLENGE_BATTLE is null", rstPData.m_ullUin);
		return;
	}
    for (uint32_t i = 0; i < pstBattleInfo->m_bGeneralCnt; i++)
    {
		if (pstBattleInfo->m_GeneralList[i] == 0)
		{
			continue;
		}
        DT_ITEM_GCARD* pstGCard = GeneralCard::Instance().Find(&rstPData, pstBattleInfo->m_GeneralList[i]);
        if (!pstGCard)
        {
            LOGERR("Uin<%lu> SetGCardState the pstGCard<%u> is NULL", rstPData.m_ullUin, pstBattleInfo->m_GeneralList[i]);
            continue;
        }
        pstGCard->m_bMineAP++;
    }
}

void Mine::SetGCardState(PlayerData& rstData, DT_MINE_ORE_INFO& rstOreInfo, bool bUseTag)
{
    for (uint32_t i = 0; i < rstOreInfo.m_bTroopCount; i++)
    {
        DT_ITEM_GCARD* pstGCard = GeneralCard::Instance().Find(&rstData, rstOreInfo.m_astTroopInfo[i].m_stGeneralInfo.m_dwId);
        if (!pstGCard)
        {
            LOGERR("Uin<%lu> SetGCardState the pstGCard<%u> is NULL", 
                rstData.m_ullUin, rstOreInfo.m_astTroopInfo[i].m_stGeneralInfo.m_dwId);
            continue;
        }
        GeneralCard::Instance().SetModuleState(pstGCard, GeneralCard::MODULE_MINE_USE , bUseTag);
    }
}

void Mine::SendNtfToClient(uint8_t bNtfType, SC_PKG_MINE_INFO_NTF& rstNtf)
{


}

void Mine::RefreshGCardState(PlayerData& rstData, uint8_t bOreNum, DT_MINE_ORE_INFO* pstOreInfo)
{
	vector<uint32_t> GCardList;
	for (int i = 0; i < bOreNum; i++)
	{
		for (int j = 0 ; j < pstOreInfo[i].m_bTroopCount; j++)
		{
			GCardList.push_back(pstOreInfo[i].m_astTroopInfo[j].m_stGeneralInfo.m_dwId);
		}
	}

	DT_ROLE_GCARD_INFO& rstGCardInfo = rstData.GetGCardInfo();
	vector<uint32_t>::iterator iter;
	for (int j = 0; j < rstGCardInfo.m_iCount; j++)
	{
		iter = find(GCardList.begin(), GCardList.end(), rstGCardInfo.m_astData[j].m_dwId);
		if (iter == GCardList.end())
		{
			GeneralCard::Instance().SetModuleState(&rstGCardInfo.m_astData[j], GeneralCard::MODULE_MINE_USE, false);
		}
	}

}


