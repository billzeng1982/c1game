#include "MasterSkill.h"
#include "LogMacros.h"
#include "../gamedata/GameDataMgr.h"
#include "Props.h"
#include "Majesty.h"
#include "Item.h"
#include "Task.h"
#include "Consume.h"
#include "dwlog_svr.h"
#include "ZoneLog.h"
#include "strutil.h"
#include "player/Player.h"
#include "GeneralCard.h"
#include "RankMgr.h"

using namespace PKGMETA;
using namespace DWLOG;

int MasterSkillCmp(const void *pstFirst, const void *pstSecond)
{
	DT_ITEM_MSKILL* pstItemFirst = (DT_ITEM_MSKILL*)pstFirst;
	DT_ITEM_MSKILL* pstItemSecond = (DT_ITEM_MSKILL*)pstSecond;

	int iResult = (int)pstItemFirst->m_bId - (int)pstItemSecond->m_bId;
	return iResult;
}

MasterSkill::MasterSkill()
{
    m_dwDefaultMSId = 0;
}

bool MasterSkill::Init()
{
	ResMasterSkillMgr_t& rResMasterSkillMgr = CGameDataMgr::Instance().GetResMasterSkillMgr();
	RESMASTERSKILL* pMasterSkill = NULL;

	int iNum = rResMasterSkillMgr.GetResNum();
	for (int i= 0; i < iNum; i++)
	{
		pMasterSkill = rResMasterSkillMgr.GetResByPos(i);
        assert(pMasterSkill);

		if (pMasterSkill->m_bInitLevel > 0)
		{
			m_dwDefaultMSId = pMasterSkill->m_bInitLevel;
			return true;
		}
	}

    return true;
}


int MasterSkill::UpgradeMS(PlayerData* pstData, SC_PKG_MS_UPGRADE_RSP& rstMSkillUpdateRsp)
{
	DT_ITEM_MSKILL *pMSkill = this->Find(pstData, rstMSkillUpdateRsp.m_bMSId);
	if(NULL == pMSkill)
	{
		LOGERR("Uin<%lu> pMasterSkill(%d) is not found",pstData->m_ullUin, rstMSkillUpdateRsp.m_bMSId);
		return ERR_NOT_FOUND;
	}

	ResMasterSkillMgr_t& rResMasterSkillMgr = CGameDataMgr::Instance().GetResMasterSkillMgr();
	RESMASTERSKILL *pResMSkill = rResMasterSkillMgr.Find(rstMSkillUpdateRsp.m_bMSId);
	if(NULL == pResMSkill)
	{
		LOGERR("Uin<%lu> pResMSkill(%d) is not found",pstData->m_ullUin, rstMSkillUpdateRsp.m_bMSId);
		return ERR_SYS;
	}

	DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();

	if (pMSkill->m_bLevel >= RES_MAX_MS_LEVEL)
	{
		return ERR_TOP_LEVEL;
	}
	else if (pMSkill->m_bLevel >= rstMajestyInfo.m_wLevel)
	{
		//去掉主公等级对于军师技等级的限制
		////秘策等级不能超过主公等级
		//pMSkill->m_bLevel = rstMajestyInfo.m_wLevel;
		//return ERR_TOP_LEVEL;
	}
	else if (pMSkill->m_bLevel > 0)
	{
		// 已开启
	}
	else
	{
		// 是要新解锁，则需要看是否满足解锁条件
		if (rstMajestyInfo.m_wLevel < pResMSkill->m_bLimitLv)
		{
			// 开启主公等级不满足
			return ERR_LEVEL_LIMIT;
		}

		if (pResMSkill->m_bLimitPrevId != 0)
		{
			DT_ITEM_MSKILL *pMSkillPrev = this->Find(pstData, pResMSkill->m_bLimitPrevId);
			if(NULL == pMSkillPrev)
			{
				LOGERR("Uin<%lu> pMasterSkill(%d) is not found",pstData->m_ullUin, rstMSkillUpdateRsp.m_bMSId);
				return ERR_NOT_FOUND;
			}

			if (pMSkillPrev->m_bLevel < pResMSkill->m_bLimitPrevLv)
			{
				// 开启前置条件不满足
				return ERR_LEVEL_LIMIT;
			}
		}
	}

    ResConsumeMgr_t& rResConsumeMgr = CGameDataMgr::Instance().GetResConsumeMgr();

	//判断碎片是否够
	for (int i=0; i<pResMSkill->m_bMaterialCount; i++)
	{
		// 获取消耗规则
		RESCONSUME *pConsumeProps = rResConsumeMgr.Find(pResMSkill->m_materialConsumeIdList[i]);
		if (NULL == pConsumeProps)
		{
			LOGERR("Uin<%lu> pConsume(%u) is not found",pstData->m_ullUin, pResMSkill->m_materialConsumeIdList[i]);
			return ERR_SYS;
		}

		// 碎片是否足够
		if (!Props::Instance().IsEnough(pstData, pResMSkill->m_materialIdList[i], pConsumeProps->m_lvList[pMSkill->m_bLevel]))
		{
			return ERR_NOT_ENOUGH_PROPS;
		}
	}

	//判断金币是否够
	RESCONSUME *pConsumeGold = rResConsumeMgr.Find(pResMSkill->m_dwMSMoneyCostRuleID);
	if(NULL == pConsumeGold)
	{
		LOGERR("Uin<%lu> pConsumeGold(%u) is not found",pstData->m_ullUin, pResMSkill->m_dwMSMoneyCostRuleID);
		return ERR_SYS;
	}
	if (!Consume::Instance().IsEnoughGold(pstData, pConsumeGold->m_lvList[pMSkill->m_bLevel]))
	{
		return ERR_NOT_ENOUGH_GOLD;
	}

	// 升级成功，更新消耗
	LOGRUN("Uin<%lu> upgrade master skill id<%d> lv<%d> succeed", pstData->m_ullUin, rstMSkillUpdateRsp.m_bMSId, pMSkill->m_bLevel);

	for (int i=0; i<pResMSkill->m_bMaterialCount; i++)
	{
		RESCONSUME *pConsumeProps = rResConsumeMgr.Find(pResMSkill->m_materialConsumeIdList[i]);
		Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, pResMSkill->m_materialIdList[i], -pConsumeProps->m_lvList[pMSkill->m_bLevel], rstMSkillUpdateRsp.m_stSyncItemInfo, METHOD_MASTERSKILL_LVUP);
	}
	Item::Instance().ConsumeItem(pstData, ITEM_TYPE_GOLD, 0, -pConsumeGold->m_lvList[pMSkill->m_bLevel], rstMSkillUpdateRsp.m_stSyncItemInfo, METHOD_MASTERSKILL_LVUP);

	rstMSkillUpdateRsp.m_bMSLevel = ++pMSkill->m_bLevel;

	if (pMSkill->m_bLevel == 1)
	{
		// 任务计数
		// 新拥有一个军师技
		Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_MASTERSKILL, 1/*value*/, 1);
	}

	// 任务计数，一次一次升级
	//Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_MASTERSKILL, 1/*value*/, (uint32_t)rstMSkillUpdateRsp.m_bMSLevel);

	//军师培养日志日志
	ZoneLog::Instance().WriteMSKillLog(pstData, rstMSkillUpdateRsp.m_bMSId, rstMSkillUpdateRsp.m_bMSLevel);

    //战力增量计算
    GeneralCard::Instance().CalAllCardLiByMSkillUp(pstData);
    RankMgr::Instance().UpdateLi(pstData);
	return ERR_NONE;
}

int MasterSkill::UpgradeMSTotal(PlayerData* pstData, SC_PKG_MS_UPGRADE_RSP& rstMSkillUpdateRsp)
{
	DT_ITEM_MSKILL *pMSkill = this->Find(pstData, rstMSkillUpdateRsp.m_bMSId);
	if(NULL == pMSkill)
	{
		LOGERR("pMasterSkill(%d) is not found", rstMSkillUpdateRsp.m_bMSId);
		return ERR_NOT_FOUND;
	}

	ResMasterSkillMgr_t& rResMasterSkillMgr = CGameDataMgr::Instance().GetResMasterSkillMgr();
	RESMASTERSKILL *pResMSkill = rResMasterSkillMgr.Find(rstMSkillUpdateRsp.m_bMSId);
	if(NULL == pResMSkill)
	{
		LOGERR("pResMSkill(%d) is not found", rstMSkillUpdateRsp.m_bMSId);
		return ERR_SYS;
	}

	ResConsumeMgr_t& rResConsumeMgr = CGameDataMgr::Instance().GetResConsumeMgr();

	uint32_t dwGoldCost = 0;
	uint32_t dwPropCost[pResMSkill->m_bMaterialCount + 1];
	for (int i = 0; i < pResMSkill->m_bMaterialCount; i++)
	{
		dwPropCost[i] = 0;
	}

	uint8_t bLevel = pMSkill->m_bLevel;
	int iRet = ERR_NONE;

	while (true)
	{
		if (pMSkill->m_bLevel >= RES_MAX_MS_LEVEL)
		{
			iRet =  ERR_TOP_LEVEL;
			break;
		}

		//去掉主公等级对于军师技等级的限制
		//if (bLevel >= pstData->GetMajestyInfo().m_wLevel)
		//{
		//	iRet = ERR_TOP_LEVEL;
		//	break;
		//}

		uint8_t bEnoughProps = 1;
		//判断碎片是否够
		for (int i = 0; i < pResMSkill->m_bMaterialCount; i++)
		{
			// 获取消耗规则
			RESCONSUME *pConsumeProps = rResConsumeMgr.Find(pResMSkill->m_materialConsumeIdList[i]);
			if (NULL == pConsumeProps)
			{
				LOGERR("pConsume(%u) is not found", pResMSkill->m_materialConsumeIdList[i]);
				return ERR_SYS;
			}
			if (0 == pConsumeProps->m_lvList[bLevel])
			{
				continue;
			}
			uint32_t dwPropsConsum = pConsumeProps->m_lvList[bLevel] + dwPropCost[i];
			// 碎片是否足够
			if (!Props::Instance().IsEnough(pstData, pResMSkill->m_materialIdList[i], dwPropsConsum))
			{
				iRet = ERR_NOT_ENOUGH_PROPS;
				bEnoughProps = 0;
				break;
			}
		}

		if (!bEnoughProps)
		{
			break;
		}

		//判断金币是否够
		RESCONSUME *pConsumeGold = rResConsumeMgr.Find(pResMSkill->m_dwMSMoneyCostRuleID);
		if(NULL == pConsumeGold)
		{
			LOGERR("pConsumeGold(%u) is not found", pResMSkill->m_dwMSMoneyCostRuleID);
			return ERR_SYS;
		}
		if (!Consume::Instance().IsEnoughGold(pstData, dwGoldCost + pConsumeGold->m_lvList[bLevel]))
		{
			iRet = ERR_NOT_ENOUGH_GOLD;
			break;
		}

		// 升级成功，更新消耗
		dwGoldCost += pConsumeGold->m_lvList[bLevel];

		for (int i = 0; i < pResMSkill->m_bMaterialCount; i++)
		{
			RESCONSUME *pConsumeProps = rResConsumeMgr.Find(pResMSkill->m_materialConsumeIdList[i]);
			if (NULL == pConsumeProps)
			{
				LOGERR("pConsumeProps is null");
				return ERR_SYS;
			}
			dwPropCost[i] += pConsumeProps->m_lvList[bLevel];

		}
		bLevel++;
	}

	rstMSkillUpdateRsp.m_bMSLevel = bLevel;
	if (bLevel == pMSkill->m_bLevel)
	{
		return iRet;
	}
	else
	{
		pMSkill->m_bLevel = bLevel;
		Item::Instance().ConsumeItem(pstData, ITEM_TYPE_GOLD, 0, -dwGoldCost, rstMSkillUpdateRsp.m_stSyncItemInfo, METHOD_MASTERSKILL_LVUP);
		for (int i = 0; i < pResMSkill->m_bMaterialCount; i++)
		{
			RESCONSUME *pConsumeProps = rResConsumeMgr.Find(pResMSkill->m_materialConsumeIdList[i]);
			if (NULL == pConsumeProps)
			{
				LOGERR("pConsumeProps is null");
				return ERR_SYS;
			}
			Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, pResMSkill->m_materialIdList[i], -dwPropCost[i], rstMSkillUpdateRsp.m_stSyncItemInfo, METHOD_MASTERSKILL_LVUP);
		}
	}

	// 任务计数，一次一次升级
	//Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_MASTERSKILL, 1/*value*/, (uint32_t)rstMSkillUpdateRsp.m_bMSLevel);
    //战力增量计算
    GeneralCard::Instance().CalAllCardLiByMSkillUp(pstData);
    RankMgr::Instance().UpdateLi(pstData);
	return ERR_NONE;
}

int MasterSkill::Composite(PlayerData* pstData, uint32_t dwId, SC_PKG_MS_COMPOSITE_RSP& rstScPkgRsp)
{
    DT_ITEM_MSKILL *pMSkill = this->Find(pstData, dwId);
	if (pMSkill != NULL)
	{
		LOGERR("pMasterSkill(%d) is already exist", dwId);
		return ERR_ALREADY_EXISTED;
	}

    ResMasterSkillMgr_t& rResMasterSkillMgr = CGameDataMgr::Instance().GetResMasterSkillMgr();
	RESMASTERSKILL *pResMSkill = rResMasterSkillMgr.Find(dwId);
	if(NULL == pResMSkill)
	{
		LOGERR("pResMSkill(%d) is not found", dwId);
		return ERR_SYS;
	}

    // 碎片是否足够
    if (!Props::Instance().IsEnough(pstData, pResMSkill->m_dwCompositePropsId, pResMSkill->m_dwCompositePropsNum))
    {
       return ERR_NOT_ENOUGH_PROPS;
    }

    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, pResMSkill->m_dwCompositePropsId, -pResMSkill->m_dwCompositePropsNum, rstScPkgRsp.m_stSyncItemInfo, METHOD_MASTERSKILL_COMPOSITE);

    rstScPkgRsp.m_bMSId = dwId;

    this->Add(pstData, dwId);
    //战力增量计算
    GeneralCard::Instance().CalAllCardLiByMSkillUp(pstData);
    RankMgr::Instance().UpdateLi(pstData);
    return ERR_NONE;
}


uint32_t MasterSkill::GetDefaultMSId()
{
	return m_dwDefaultMSId;
}

DT_ITEM_MSKILL* MasterSkill::GetMSkillInfo(PlayerData* pstData, uint32_t dwMSId)
{
	return this->Find(pstData, dwMSId);
}

int MasterSkill::AddDataForNewPlayer(PlayerData* pstData)
{
	ResMasterSkillMgr_t& rResMasterSkillMgr = CGameDataMgr::Instance().GetResMasterSkillMgr();
	RESMASTERSKILL* pMasterSkill = NULL;

	int iNum = rResMasterSkillMgr.GetResNum();
	for (int i= 0; i < iNum; i++)
	{
		pMasterSkill = rResMasterSkillMgr.GetResByPos(i);
		if (pMasterSkill == NULL)
		{
			LOGERR("pMasterSkill is null.");
			return -1;
		}

		this->Add(pstData, pMasterSkill->m_dwId, pMasterSkill->m_bInitLevel);

		if (pMasterSkill->m_bInitLevel > 0)
		{
			// 新拥有一个军师技
			Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_MASTERSKILL, 1/*value*/, 1);
		}

		LOGRUN("add master skill id=%u lv=%d", pMasterSkill->m_dwId, pMasterSkill->m_bInitLevel);
	}

	return ERR_NONE;
}

int MasterSkill::AddDataForDebug(PlayerData* pstData, DT_ITEM* pstItemList, int iIdx /* = 0*/)
{
	ResMasterSkillMgr_t& rResMasterSkillMgr = CGameDataMgr::Instance().GetResMasterSkillMgr();
	RESMASTERSKILL* pMasterSkill = NULL;

    DT_ITEM_MSKILL *pItemMSkill = NULL;

	int iNum = rResMasterSkillMgr.GetResNum();
    if (iNum > MAX_DEBUG_CNT_NUM)
    {
        iNum = MAX_DEBUG_CNT_NUM;
        LOGERR("MasterSkill : ResNum is larger than max num");
    }

    int iIndex = iIdx;

	for (int i= 0; i<iNum; i++)
	{
		pMasterSkill = rResMasterSkillMgr.GetResByPos(i);
		if (!pMasterSkill)
		{
			LOGERR("pMasterSkill is null.");
			return -1;
		}
		this->Add(pstData, pMasterSkill->m_dwId);
        pItemMSkill = this->Find(pstData, pMasterSkill->m_dwId);
        if (!pItemMSkill)
        {
            continue;
        }

        pItemMSkill->m_bLevel = 10;
        pstItemList[iIndex].m_bItemType = ITEM_TYPE_MSKILL;
		pstItemList[iIndex].m_stItemData.m_stMSkill = *pItemMSkill;
        iIndex++;
		LOGRUN("add master skill id=%u, lv=%u", pMasterSkill->m_dwId, pItemMSkill->m_bLevel);
	}

    return iIndex;
}

int MasterSkill::Add(PlayerData* pstData, uint32_t dwMSId, uint8_t bLevel /* = 1*/)
{
	DT_ITEM_MSKILL* pItem = this->Find(pstData, dwMSId);
	if (pItem != NULL)
	{
		return -1;
	}

	DT_ROLE_MSKILL_INFO& rstInfo = pstData->GetMSkillInfo();
	m_stMasterSkill.m_bId = dwMSId;
	m_stMasterSkill.m_bLevel = bLevel;

	size_t nmemb = (size_t)rstInfo.m_iCount;
	if (nmemb >= MAX_NUM_ROLE_MSKILL)
	{
		LOGERR("rstInfo.m_iCount<%d> reaches MAX_NUM_ROLE_MSKILL", rstInfo.m_iCount);
		return -1;
	}
	if (MyBInsert(&m_stMasterSkill, rstInfo.m_astData, &nmemb, sizeof(DT_ITEM_MSKILL), 1, MasterSkillCmp))
	{
		rstInfo.m_iCount = (int32_t)nmemb;
	}

	return ERR_NONE;
}

DT_ITEM_MSKILL* MasterSkill::Find(PlayerData* pstData, uint32_t dwMSId)
{
	m_stMasterSkill.m_bId = dwMSId;
	DT_ROLE_MSKILL_INFO& rstInfo = pstData->GetMSkillInfo();
	int iEqual = 0;
	int iIndex = MyBSearch(&m_stMasterSkill, rstInfo.m_astData, rstInfo.m_iCount, sizeof(DT_ITEM_MSKILL), &iEqual, MasterSkillCmp);
	if (!iEqual)
	{
		return NULL;
	}

	return &rstInfo.m_astData[iIndex];
}
