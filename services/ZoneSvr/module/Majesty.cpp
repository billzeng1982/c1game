#include "Majesty.h"
#include "LogMacros.h"
#include "../gamedata/GameDataMgr.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "./player/Player.h"
#include "MasterSkill.h"
#include "AP.h"
#include "Item.h"
#include "Lottery.h"
#include "Task.h"
#include "ActivityMgr.h"
#include "ov_res_public.h"
#include "dwlog_svr.h"
#include "ZoneLog.h"
#include "Guild.h"
#include "GeneralCard.h"
#include "AsyncPvp.h"
#include "RankMgr.h"
#include "GuildGeneralHang.h"
#include "Tactics.h"


using namespace PKGMETA;

Majesty::Majesty()
{
    m_iMaxLv = 0;
    m_dwMaxExp = 0;
}

bool Majesty::Init()
{
    ResMajestyLvMgr_t& roResMgr = CGameDataMgr::Instance().GetResMajestyLvMgr();
    m_iMaxLv = roResMgr.GetResNum();

    RESMAJESTYLV* pstResLv = roResMgr.Find(m_iMaxLv);
    if (!pstResLv)
    {
        return false;
    }

    m_dwMaxExp = pstResLv->m_dwExp;

    m_iUpdateTime = (int)CGameDataMgr::Instance().GetResBasicMgr().Find((int)COMMON_UPDATE_TIME)->m_para[0];
	m_iChgNamePrice = (int)CGameDataMgr::Instance().GetResBasicMgr().Find((int)DIAMOND_CHANGE_ROLE_NAME)->m_para[0];

    LOGRUN("init majesty info");
    LOGRUN("---------------------------------------------------");
    LOGRUN("m_iMaxLv = %d", m_iMaxLv);
    LOGRUN("m_dwMaxExp = %u", m_dwMaxExp);
    LOGRUN("m_iUpdateTime = %d", m_iUpdateTime);
    LOGRUN("---------------------------------------------------");

    return true;
}

int Majesty::SetName(PlayerData* pstData, char* szName)
{
    int iLen = (int)strlen(szName);
    if (iLen>MAX_NAME_LENGTH)
    {
        return -1;
    }

    char* pszName = pstData->GetMajestyInfo().m_szName;
    bzero(pszName, MAX_NAME_LENGTH);
    STRNCPY(pszName, szName, MAX_NAME_LENGTH-1);

    return 0;
}

void Majesty::UpdatePlayerData(PlayerData* pstData)
{
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();

    uint64_t ullUpdateTime;
    if (CGameTime::Instance().IsNeedUpdateByHour(rstMajestyInfo.m_ullLastUpdateTime * 1000, m_iUpdateTime, ullUpdateTime))
    {
        // 需要更新主公信息
        // 刷新主公信息更新时间(S)
        rstMajestyInfo.m_ullLastUpdateTime = ullUpdateTime / 1000;
        rstMajestyInfo.m_bAPPurchaseTimes = 0;
        rstMajestyInfo.m_bGoldPurchaseTimes = 0;
        rstMajestyInfo.m_bPVPRewardTimes = 0;
		rstMajestyInfo.m_bSPPurchaseTimes = 0;
		pstData->GetMiscInfo().m_dwConsumeAPCnt = 0;
    }

    if (pstData->m_bIsNeedCalLv)
    {
        CalMajestyLv(pstData);
    }
}

void Majesty::CalMajestyLv(PlayerData* pstData)
{
    // 经验奖励延后处理，防止奖励经验时触发新奖励，导致奖励经验的批次奖励对前台有问题
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();


    while(rstMajestyInfo.m_wLevel < m_iMaxLv)
    {
        uint32_t dwExpNextLv = this->GetExpNextLv(pstData);
		if (rstMajestyInfo.m_dwExp >= dwExpNextLv)
		{
			rstMajestyInfo.m_wLevel++;

			_HandleLvReward(pstData);

			AP::Instance().HandleMajestLvUp(pstData);

			ActivityMgr::Instance().HandleMajestLvUp(pstData);

			GeneralCard::Instance().HandleGroupCard(pstData, 1/*升级类型组卡开孔*/, rstMajestyInfo.m_wLevel);

			Guild::Instance().RefreshMemberInfo(pstData);

			Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_LEVEL, rstMajestyInfo.m_wLevel);
			Task::Instance().TaskCondTrigger(pstData, TASK_COND_TYPE_MAJESTY_LV);

			GuildGeneralHang::Instance().UnlockLevelSlot(pstData);

			LOGRUN("majesty uin=%lu exp=%u, lv=%u", pstData->GetRoleBaseInfo().m_ullUin, rstMajestyInfo.m_dwExp, rstMajestyInfo.m_wLevel);
		}
		else
		{
			break;
		}
    }

    pstData->m_bIsNeedCalLv = false;
}

uint32_t Majesty::AddExp(PlayerData* pstData, uint32_t dwValue)
{
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();

    if (rstMajestyInfo.m_dwExp >= m_dwMaxExp)
    {
        // 已经是满经验了，直接返回
        rstMajestyInfo.m_dwExp = m_dwMaxExp;
    }
    else
    {
        // 本次不满，加完后满
        rstMajestyInfo.m_dwExp += dwValue;
        if (rstMajestyInfo.m_dwExp >= m_dwMaxExp)
        {
            rstMajestyInfo.m_dwExp = m_dwMaxExp;
        }
        pstData->m_bIsNeedCalLv = true;
    }

    return rstMajestyInfo.m_dwExp;
}

uint32_t Majesty::GetExp(PlayerData* pstData)
{
    return pstData->GetMajestyInfo().m_dwExp;
}


uint32_t Majesty::GetExpNextLv(PlayerData* pstData)
{
	DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();

	ResMajestyLvMgr_t& roResMgr = CGameDataMgr::Instance().GetResMajestyLvMgr();
	RESMAJESTYLV* pstResLv = roResMgr.Find((int)rstMajestyInfo.m_wLevel);
	if (pstResLv == NULL)
	{
		LOGERR("pstResLv is null, %d", rstMajestyInfo.m_wLevel);
		return 0xffffffff;
	}

	return pstResLv->m_dwExp;
}

uint16_t Majesty::GetLevel(PlayerData* pstData)
{
    return pstData->GetMajestyInfo().m_wLevel;
}

void Majesty::GmSetLv(PlayerData* pstData, uint16_t Lv)
{
	ResMajestyLvMgr_t& roResMgr = CGameDataMgr::Instance().GetResMajestyLvMgr();
	RESMAJESTYLV* pstResLv = roResMgr.Find((int)Lv);
	if ( NULL == pstResLv  )
	{
		return;
	}
	DT_ROLE_MAJESTY_INFO& rstInfo = pstData->GetMajestyInfo();
	rstInfo.m_wLevel = Lv;
	rstInfo.m_dwExp = pstResLv->m_dwExp;
}

int Majesty::SetDefaultItems(DT_ROLE_MAJESTY_INFO& rstMajestyInfo, DT_ROLE_ITEMS_INFO& rstRoleItemsInfo)
{
	RESMAJESTYITEM* poResMajestyItem = NULL;
	ResMajestyItemMgr_t& rResMajestyItemMgr = CGameDataMgr::Instance().GetResMajestyItemMgr();
	int iResSum = rResMajestyItemMgr.GetResNum();
	rstRoleItemsInfo.m_iCount = 0;

	for (int i = 0; i < iResSum; i++)
	{
		poResMajestyItem = rResMajestyItemMgr.GetResByPos(i);
		if (NULL == poResMajestyItem)
		{
			continue;
		}

		if (MAJESTY_ITEM_ACCESS_INIT == poResMajestyItem->m_bAccessType)
		{
			rstRoleItemsInfo.m_astData[rstRoleItemsInfo.m_iCount++].m_dwId = poResMajestyItem->m_dwId;

			if (MAJESTY_ITEM_HEAD_ICON == poResMajestyItem->m_bType)
			{
				rstMajestyInfo.m_wIconId = poResMajestyItem->m_dwId;
			}
			else if (MAJESTY_ITEM_HEAD_FRAME == poResMajestyItem->m_bType)
			{
				rstMajestyInfo.m_wFrameId = poResMajestyItem->m_dwId;
			}
			else
			{
				rstMajestyInfo.m_wTitleId = poResMajestyItem->m_dwId;
			}
		}
	}

	return 0;
}

uint32_t Majesty::GetBattleMSkill(PlayerData* pstData, uint8_t bBattleArrayType /*= BATTLE_ARRAY_TYPE_NORMAL 阵容类型*/)
{
	DT_BATTLE_ARRAY_INFO* pstBattleInfo = this->GetBattleArrayInfo(pstData, bBattleArrayType);
	if (pstBattleInfo != NULL)
	{
		return pstBattleInfo->m_bMSId;
	}

	return 0;
}

int Majesty::SetBattleMSkill(PlayerData* pstData, uint32_t bMSId, uint8_t bBattleArrayType /*= BATTLE_ARRAY_TYPE_NORMAL 阵容类型*/)
{
	DT_ITEM_MSKILL* pMSkill = MasterSkill::Instance().GetMSkillInfo(pstData, bMSId);
	if (pMSkill == NULL)
	{
		LOGERR("Player <%lu> master skill <%hhu> not exist", pstData->m_ullUin, bMSId);
		return ERR_NOT_FOUND;
	}

	if (pMSkill->m_bLevel == MSKILL_LEVEL_DISABLE)
	{
		LOGERR("Player <%lu> master skill <%hhu> not enabled", pstData->m_ullUin, bMSId);
		return ERR_SYS;
	}

	DT_BATTLE_ARRAY_INFO* pstBattleInfo = this->GetBattleArrayInfo(pstData, bBattleArrayType);
	if (pstBattleInfo == NULL)
	{
		return ERR_NOT_FOUND;
	}

	pstBattleInfo->m_bMSId = bMSId;

	if (bBattleArrayType == BATTLE_ARRAY_TYPE_ASYNCPVP)
	{
		AsyncPvp::Instance().UptToAsyncSvr(pstData);
	}
	return ERR_NONE;
}

int Majesty::GetBattleGeneralList(PlayerData* pstData, uint8_t& bCount, uint16_t* pastGeneralId[], uint8_t bBattleArrayType /*= BATTLE_ARRAY_TYPE_NORMAL*/)
{
	DT_BATTLE_ARRAY_INFO* pstBattleInfo = this->GetBattleArrayInfo(pstData, bBattleArrayType);
	if (pstBattleInfo == NULL)
	{
		bCount = 0;
		return ERR_NOT_FOUND;
	}

	bCount = pstBattleInfo->m_bGeneralCnt;
	*pastGeneralId = pstBattleInfo->m_GeneralList;

	return ERR_NONE;
}

int Majesty::SetBattleTactics(PlayerData* pstData, uint8_t bTacticsType, uint8_t bBattleArrayType /* = BATTLE_ARRAY_TYPE_NORMAL */ )
{
    //DT_ROLE_TACTICS_INFO& rstInfo = pstData->GetTacticsInfo();

    DT_ITEM_TACTICS* pstTactics = Tactics::Instance().GetTacticsItem(pstData, bTacticsType);
    if (!pstTactics)
    {
        LOGERR("pstTactics is NULL. bTacticsType<%d>", bTacticsType);
        return ERR_SYS;
    }

    DT_BATTLE_ARRAY_INFO* pstBattleInfo = this->GetBattleArrayInfo(pstData, bBattleArrayType);
    if (pstBattleInfo == NULL)
    {
        return ERR_NOT_FOUND;
    }

    pstBattleInfo->m_bTacticsType = bTacticsType;

    return ERR_NONE;
}

int Majesty::SetBattleGeneralList(PlayerData* pstData, uint8_t bCount, uint32_t astGeneralId[], uint8_t bBattleArrayType /*= BATTLE_ARRAY_TYPE_NORMAL*/)
{
	if (bCount > MAX_TROOP_NUM_PVP)
	{
		return ERR_WRONG_PARA;
	}

    if (ERR_NONE != _CheckBattleGeneralList(pstData, bCount, astGeneralId))
    {
        return ERR_WRONG_PARA;
    }

    //type参数的最高字段有特殊用途，需要单独提取出来
    uint8_t bFlag = bBattleArrayType & 0x80;
    bBattleArrayType = bBattleArrayType & 0x7F;

	DT_BATTLE_ARRAY_INFO* pstBattleInfo = this->GetBattleArrayInfo(pstData, bBattleArrayType);
	if (pstBattleInfo == NULL)
	{
		return ERR_NOT_FOUND;
	}

	DT_ROLE_MISC_INFO& rstMiscInfo = pstData->GetMiscInfo();

	pstBattleInfo->m_bGeneralCnt = bCount;

	for (int i=0; i<bCount; i++)
	{
		uint8_t bOn = (1 << i) & rstMiscInfo.m_bGroupCardId;
		if (bOn != 0)
		{
			pstBattleInfo->m_GeneralList[i] = astGeneralId[i];
		}
		else
		{
			pstBattleInfo->m_GeneralList[i] = 0;
		}
	}

	if (bBattleArrayType == BATTLE_ARRAY_TYPE_ASYNCPVP  && bFlag == 0)
	{
		AsyncPvp::Instance().UptToAsyncSvr(pstData);
	}
    if (bBattleArrayType == BATTLE_ARRAY_TYPE_NORMAL)
    {
        //主要是更新历史最高战力
        RankMgr::Instance().UpdateLi(pstData);
    }

	return ERR_NONE;
}

int Majesty::GetBattleSkillFlag(PlayerData* pstData, uint8_t& bSkillFlag, uint8_t bBattleArrayType /*= BATTLE_ARRAY_TYPE_NORMAL*/)
{
	DT_BATTLE_ARRAY_INFO* pstBattleInfo = this->GetBattleArrayInfo(pstData, bBattleArrayType);
	if (pstBattleInfo == NULL)
	{
		bSkillFlag = 0;
		return ERR_NOT_FOUND;
	}

	bSkillFlag = pstBattleInfo->m_bSkillFlag;

	return ERR_NONE;
}

int Majesty::SetBattleSkillFlag(PlayerData* pstData, uint8_t bSkillFlag, uint8_t bBattleArrayType /*= BATTLE_ARRAY_TYPE_NORMAL*/)
{
	DT_BATTLE_ARRAY_INFO* pstBattleInfo = this->GetBattleArrayInfo(pstData, bBattleArrayType);
	if (pstBattleInfo == NULL)
	{
		return ERR_NOT_FOUND;
	}

	pstBattleInfo->m_bSkillFlag = bSkillFlag;

	return ERR_NONE;
}

PKGMETA::DT_BATTLE_ARRAY_INFO* Majesty::GetBattleArrayInfo(PlayerData* pstData, uint8_t bBattleArrayType /*= BATTLE_ARRAY_TYPE_NORMAL*/, bool bAddNew /*= true*/)
{
	DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
	for (int i=0; i<rstMajestyInfo.m_bBattleArrayCnt; i++)
	{
		DT_BATTLE_ARRAY_INFO& rstBattleInfo = rstMajestyInfo.m_astBattleArrayList[i];
		if (rstBattleInfo.m_bType == bBattleArrayType)
		{
			return &rstBattleInfo;
		}
	}
    //增加的时候需要检查bBattleArrayType的有效性
	if (bAddNew && bBattleArrayType > BATTLE_ARRAY_TYPE_NONE && bBattleArrayType < BATTLE_ARRAY_TYPE_END)
	{
		// 需要新加
		if (rstMajestyInfo.m_bBattleArrayCnt < MAX_BATTLE_ARRAY_NUM)
		{
			DT_BATTLE_ARRAY_INFO& rstBattleInfo = rstMajestyInfo.m_astBattleArrayList[rstMajestyInfo.m_bBattleArrayCnt];
			rstBattleInfo.m_bType = bBattleArrayType;
			rstBattleInfo.m_bMSId = MasterSkill::Instance().GetDefaultMSId();
			rstBattleInfo.m_bGeneralCnt = 0;
			rstBattleInfo.m_dwLi = 0;
            rstBattleInfo.m_bTacticsType = TACTICS_TYPE_NONE;

			// 阵容数加一
			rstMajestyInfo.m_bBattleArrayCnt++;

			return &rstBattleInfo;
		}
	}
	LOGERR("Player <%lu> BattleArrayInfo <%hhu> not found", pstData->m_ullUin, bBattleArrayType);
	return NULL;
}

void Majesty::_HandleLvReward(PlayerData* pstData)
{
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
    ResMajestyLvMgr_t& roResMgr = CGameDataMgr::Instance().GetResMajestyLvMgr();
    RESMAJESTYLV* pstResLv = roResMgr.Find((int)rstMajestyInfo.m_wLevel);
    if (!pstResLv)
    {
        LOGERR("pstResLv is null");
        return;
    }

    int iRet = 0;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MAJESTY_LV_REWARD_NTF;
    SC_PKG_MAJESTY_LV_REWARD_NTF& rstSCPkgBody = m_stScPkg.m_stBody.m_stMajestLvReward;

    rstSCPkgBody.m_stSyncItemInfo.m_bSyncItemCount = 0;

    int iCount = (int)pstResLv->m_bRewardCount;
    if (iCount != 0)
    {
        int iArraySize = (int)sizeof(pstResLv->m_szRewardType)/sizeof(pstResLv->m_szRewardType[0]);
        if (iCount > iArraySize)
        {
            LOGERR("reward exceed array size.");
            return;
        }

        // 发放奖励
        for (int i=0; i<iCount; i++)
        {
            Item::Instance().RewardItem(pstData,
                pstResLv->m_szRewardType[i], pstResLv->m_rewardID[i], pstResLv->m_rewardNum[i],
                rstSCPkgBody.m_stSyncItemInfo, DWLOG::METHOD_ROLE_LVUP_AWARD);
        }

    }

	////黑科技，先看下效果
	//m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MAJESTY_LV_REWARD_NTF;

    // 增加随机奖励
    iCount = (int)pstResLv->m_bLotteryCount;
    if (iCount != 0)
    {
        int iArraySize = (int)sizeof(pstResLv->m_lotteryPondId)/sizeof(pstResLv->m_lotteryPondId[0]);
        if (iCount > iArraySize)
        {
            LOGERR("reward exceed array size.");
            return;
        }

        for (int i=0; i<iCount; i++)
        {
            iRet = Lottery::Instance().DrawLotteryByPond(pstData, pstResLv->m_lotteryPondId[i], rstSCPkgBody.m_stSyncItemInfo, DWLOG::METHOD_ROLE_LVUP_AWARD);

            if (iRet < 0)
            {
                LOGERR("majesty add random reward failed, ret %d.", iRet);
                continue;
            }

            LOGRUN("player-%s majesty level up, reward pond-%u, general id-%d", pstData->m_pOwner->GetAccountName(),
                pstResLv->m_lotteryPondId[i], rstSCPkgBody.m_stSyncItemInfo.m_astSyncItemList[rstSCPkgBody.m_stSyncItemInfo.m_bSyncItemCount - 1].m_dwItemId);
        }
    }

    rstSCPkgBody.m_bMajestyLv = (uint8_t)(rstMajestyInfo.m_wLevel);
    iRet = ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
    if (iRet < 0)
    {
        LOGERR("send to client failed, ret-%d", iRet);
    }

    return;
}

int Majesty::ChgImageId(PlayerData* pstData, uint16_t wImageId)
{
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();

	int iRet = _CheckItemId(pstData, wImageId);

	if (iRet == ERR_NONE)
	{
		ResMajestyItemMgr_t& rResMajestyItemMgr = CGameDataMgr::Instance().GetResMajestyItemMgr();
		RESMAJESTYITEM* poResMajestyItem = rResMajestyItemMgr.Find(wImageId);
		if (MAJESTY_ITEM_HEAD_ICON == poResMajestyItem->m_bType)
		{
			rstMajestyInfo.m_wIconId = poResMajestyItem->m_dwId;
		}
		else if (MAJESTY_ITEM_HEAD_FRAME == poResMajestyItem->m_bType)
		{
			rstMajestyInfo.m_wFrameId = poResMajestyItem->m_dwId;
		}
		else
		{
			rstMajestyInfo.m_wTitleId = poResMajestyItem->m_dwId;
		}

		Guild::Instance().RefreshMemberInfo(pstData);
	}

    return iRet;
}

int Majesty::IsArriveLevel(PlayerData* pstData, int iId)
{
    uint32_t dwLevel = (uint32_t)pstData->GetMajestyInfo().m_wLevel;
    ResMajestyFunctionMgr_t& rstMajestyFunctionMgr = CGameDataMgr::Instance().GetResMajestyFunctionMgr();
    RESMAJESTYFUNCTION* pMajestyFunction = rstMajestyFunctionMgr.Find(iId);

    if (NULL == pMajestyFunction)
    {
        LOGERR_r("pMajestyFunction is null");
        return ERR_SYS;
    }

    if (dwLevel < pMajestyFunction->m_dwLevel)
    {
        return ERR_LEVEL_LIMIT;
    }

    return ERR_NONE;
}

int Majesty::_CheckItemId(PlayerData* pstData, uint16_t wId)
{
	DT_ROLE_ITEMS_INFO& rstItemsInfo = pstData->GetItemsInfo();

	for (int i = 0; i < rstItemsInfo.m_iCount; i++)
	{
		DT_ITEM_MAJESTY_ITEM& rstTmpMajestyItem = rstItemsInfo.m_astData[i];
		if (rstTmpMajestyItem.m_dwId == wId)
		{
			uint64_t ullTimeStamp = CGameTime::Instance().GetCurrSecond();

			if (rstTmpMajestyItem.m_ullValidTime != 0 && ullTimeStamp > rstTmpMajestyItem.m_ullValidTime)
			{
				return ERR_OUT_OF_TIME;
			}

			return ERR_NONE;
		}
	}

	return ERR_NOT_FOUND;
}


//
//int Majesty::_AddMajestyItems(PlayerData* pstData, RESMAJESTYITEM* poResMajestyItem)
//{
//	DT_ROLE_ITEMS_INFO& rstRoleItemsInfo = pstData->GetItemsInfo();
//
//	int idx = 0;
//	for (; idx < rstRoleItemsInfo.m_iCount; idx++)
//	{
//		if (rstRoleItemsInfo.m_astData[idx].m_dwId == poResMajestyItem->m_dwId)
//		{
//			break;
//		}
//	}
//
//	if (idx == rstRoleItemsInfo.m_iCount)
//	{
//		rstRoleItemsInfo.m_iCount++;
//	}
//
//	rstRoleItemsInfo.m_astData[idx].m_dwId = poResMajestyItem->m_dwId;
//	uint64_t ullTimeStamp = poResMajestyItem->m_dwExpiryTime;
//
//	if (ullTimeStamp != 0)
//	{
//		ullTimeStamp += CGameTime::Instance().GetCurrSecond();
//	}
//
//	rstRoleItemsInfo.m_astData[idx].m_ullValidTime = ullTimeStamp;
//
//	return 0;
//}


int Majesty::GetPersonalizedSig(PlayerData* pstData, char* stPersonalizedSig)
{
	DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();

	StrCpy(stPersonalizedSig, rstMajestyInfo.m_szPersonalizedSig, PKGMETA::MAX_SIGNATURE_LENGTH);

	return ERR_NONE;
}

int Majesty::ChangeRoleName(Player* poPlayer, char* stRoleName)
{
	PlayerData* pstData = &poPlayer->GetPlayerData();
	DT_ROLE_BASE_INFO& rstBaseInfo = pstData->GetRoleBaseInfo();
	StrCpy(rstBaseInfo.m_szRoleName, stRoleName, PKGMETA::MAX_NAME_LENGTH);

	DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
	StrCpy(rstMajestyInfo.m_szName, stRoleName, PKGMETA::MAX_NAME_LENGTH);

	DT_ROLE_MISC_INFO& rstMiscInfo = pstData->GetMiscInfo();
	rstMiscInfo.m_wChangeNameCount++;

	poPlayer->UptRoleDataToDB();

	PlayerMgr::Instance().DelFromAccountNameHM(poPlayer);
	PlayerMgr::Instance().AddToAccountNameHM(poPlayer);

	return ERR_NONE;
}


int Majesty::_CheckBattleGeneralList(PlayerData* pstData, uint8_t& bCount, uint32_t astGeneralId[])
{
    for (int i = 0; i < bCount; i++)
    {
        if (astGeneralId[i] == 0)
        {
            continue;
        }

        for (int j = i + 1; j < bCount; j++)
        {
            if (astGeneralId[i] == astGeneralId[j])
            {
                return ERR_WRONG_PARA;
            }
        }
    }

    for (int k = 0; k < bCount; k++)
    {
        if (astGeneralId[k] == 0)
        {
            continue;
        }

        if (!GeneralCard::Instance().Find(pstData, astGeneralId[k]))
        {
            return ERR_WRONG_PARA;
        }
    }

    return ERR_NONE;
}

