#include "Props.h"
#include "LogMacros.h"
#include "../gamedata/GameDataMgr.h"
#include "Consume.h"
#include "Item.h"
#include "dwlog_svr.h"
#include "ZoneLog.h"
#include "Lottery.h"
#include "../module/player/Player.h"

#include "CpuSampleStats.h"
using namespace PKGMETA;
using namespace DWLOG;


//获取批量宝箱的最大数量,不能超过50ms,单词抽奖0.1ms
#define GET_MAX_BATCH_NUM(POOL, TOTAL) ( MIN( 500/(POOL) , TOTAL ) )
#define MAX_BATCH_NUM 100
int PropsCmp(const void *pstFirst, const void *pstSecond)
{
    DT_ITEM_PROPS* pstItemFirst = (DT_ITEM_PROPS*)pstFirst;
    DT_ITEM_PROPS* pstItemSecond = (DT_ITEM_PROPS*)pstSecond;

    int iResult = (int)pstItemFirst->m_dwId - (int)pstItemSecond->m_dwId;
    return iResult;
}

Props::Props()
{
    bzero((char*)&m_stProps, sizeof(m_stProps));
}

int Props::Init()
{
    return 0;
}

int Props::Add(PlayerData* pstData, uint32_t dwId, int32_t iNum, uint32_t dwApproach)
{
	DT_ITEM_PROPS* pItem = Find(pstData, dwId);
    uint8_t bChgType = 0;
    uint32_t dwChgValue = 0;
    uint32_t dwAfterValue = 0;
	if (iNum > 0)
	{
        dwChgValue = iNum;
        bChgType = 1;
		// 道具增加
		if (pItem != NULL)
		{
			pItem->m_dwNum += iNum;
            dwAfterValue = pItem->m_dwNum;
		}
		else
		{
			DT_ROLE_PROPS_INFO& rstInfo = pstData->GetPropsInfo();

			if (rstInfo.m_iCount >= MAX_NUM_ROLE_PROPS)
			{
				return -1;
			}

            // 新增道具要检查，数据档是否存在
            ResPropsMgr_t& rstResPropsMgr = CGameDataMgr::Instance().GetResPropsMgr();
            RESPROPS* pstResProps = rstResPropsMgr.Find(dwId);
            if (!pstResProps)
            {
                LOGERR("Uin<%lu> pstResProps<%u> is null", pstData->m_ullUin, dwId);
                return ERR_NOT_FOUND;
            }

			m_stProps.m_dwId = dwId;
			m_stProps.m_dwNum = iNum;

			size_t nmemb = (size_t)rstInfo.m_iCount;
            if (nmemb >= MAX_NUM_ROLE_PROPS)
            {
                LOGERR("rstInfo.m_iCount<%d> reaches the max.", rstInfo.m_iCount);
                return ERR_SYS;
            }
			if (MyBInsert(&m_stProps, rstInfo.m_astData, &nmemb, sizeof(DT_ITEM_PROPS), 1, PropsCmp))
			{
				rstInfo.m_iCount = (int32_t)nmemb;
			}
            dwAfterValue = iNum;

		}
	}
	else
	{
        dwChgValue = (uint32_t) -iNum;
        bChgType = 2;
        uint32_t MinusNum = (uint32_t)(-iNum);

		// 道具消耗
		if (pItem != NULL)
		{
			if (pItem->m_dwNum > MinusNum)
			{
				pItem->m_dwNum -= MinusNum;
                dwAfterValue = pItem->m_dwNum;
			}
			else
			{
				pItem->m_dwNum = 0;
				DT_ROLE_PROPS_INFO& rstInfo = pstData->GetPropsInfo();
				m_stProps.m_dwId = dwId;
				size_t nmemb = (size_t)rstInfo.m_iCount;
				MyBDelete(&m_stProps, rstInfo.m_astData, &nmemb, sizeof(DT_ITEM_PROPS), PropsCmp);
				rstInfo.m_iCount = (int32_t)nmemb;
                dwAfterValue = 0;
			}
		}
	}
    if (iNum != 0)
	{
		ZoneLog::Instance().WritePropLog(pstData, dwId, ITEM_TYPE_PROPS, dwChgValue, bChgType, dwAfterValue, dwApproach);
    }

    return ERR_NONE;
}

bool Props::IsEnough(PlayerData* pstData, uint32_t dwId, uint32_t dwNum)
{
    if (0 == dwNum)
    {//消耗为0 满足,从逻辑上也说得通
        return true;
    }
    DT_ITEM_PROPS* pItem = Find(pstData, dwId);
    if(!pItem)
    {
        return false;
    }

    if (pItem->m_dwNum < dwNum)
    {
        return false;
    }

    return true;

}

uint32_t Props::GetNum(PlayerData* pstData, uint32_t dwId)
{
	DT_ITEM_PROPS* pstProps = Find(pstData, dwId);
	if (!pstProps)
	{
		return 0;
	}
	return  pstProps->m_dwNum;
}

DT_ITEM_PROPS* Props::Find(PlayerData* pstData, uint32_t dwId)
{
    m_stProps.m_dwId = dwId;
    DT_ROLE_PROPS_INFO& rstInfo = pstData->GetPropsInfo();
    int iEqual = 0;
    int iIndex = MyBSearch(&m_stProps, rstInfo.m_astData, rstInfo.m_iCount, sizeof(DT_ITEM_PROPS), &iEqual, PropsCmp);
    if (!iEqual)
    {
        return NULL;
    }

    return &rstInfo.m_astData[iIndex];
}

int Props::GetClassScore(uint32_t dwId)
{
    ResPropsMgr_t& rstResPropsMgr = CGameDataMgr::Instance().GetResPropsMgr();
    RESPROPS* pstResProps = rstResPropsMgr.Find(dwId);


    if (pstResProps == NULL)
    {
        LOGERR("dwId-%u is not found", dwId);
        return -1;
    }

    return (int)pstResProps->m_bPhase;
}

int Props::AddDataForNewPlayer(PlayerData* pstData)
{
#if 0
    RESPROPS* pProps = NULL;
    ResPropsMgr_t& rstResMgr = CGameDataMgr::Instance().GetResPropsMgr();
    int iNum = rstResMgr.GetResNum();
    for (int i=0; i<iNum; i++)
    {
        pProps = rstResMgr.GetResByPos(i);
        if (!pProps)
        {
            LOGERR("pProps is null.");
            return -1;
        }

        this->Add(pstData, pProps->m_dwItemId, 7);
        LOGRUN("add props id=%u", pProps->m_dwItemId);
    }
#endif
    return ERR_NONE;
}

int Props::AddDataForDebug(PlayerData* pstData, DT_ITEM* pstItemList)
{
    RESPROPS* pProp = NULL;
    ResPropsMgr_t& rstResMgr = CGameDataMgr::Instance().GetResPropsMgr();
    int iNum = rstResMgr.GetResNum();
    if (iNum > MAX_DEBUG_CNT_NUM)
    {
        iNum = MAX_DEBUG_CNT_NUM;
        LOGERR("Props : ResNum is larger than max num");
    }
    int iIndex = 0;
    int iRet = 0;

    for (int i=0; i<iNum; i++)
    {
        pProp = rstResMgr.GetResByPos(i);
        if (!pProp)
        {
            LOGERR("pEquip is null.");
            return -1;
        }

        iRet = this->Add(pstData, pProp->m_dwItemId, 100, METHOD_GM_DEBUG);
        if (iRet == ERR_NONE)
        {
			DT_ITEM_PROPS* pPropsItem = Find(pstData, pProp->m_dwItemId);

            DT_ITEM& rstItem = pstItemList[iIndex];
            rstItem.m_bItemType = ITEM_TYPE_PROPS;
			rstItem.m_dwItemId = pPropsItem->m_dwId;
			rstItem.m_iValueChg = 100;
			rstItem.m_dwValueAfterChg = pPropsItem->m_dwNum;
            rstItem.m_stItemData.m_stProps.m_dwId = pPropsItem->m_dwId;
            rstItem.m_stItemData.m_stProps.m_dwNum = pPropsItem->m_dwNum; // 数量

            iIndex++;
        }

        //LOGRUN("add equip id=%u", pEquip->m_dwItemId);
    }
    return iIndex;
}

int Props::PurchaseProps(PlayerData* pstData, uint32_t dwId, uint32_t dwNum, SC_PKG_PURCHASE_RSP& rstScPkgBodyRsp)
{
	RESPROPS* pResProp = CGameDataMgr::Instance().GetResPropsMgr().Find(dwId);
	if (pResProp == NULL)
	{
		LOGERR("pResProp is null");
		return ERR_SYS;
	}

	uint32_t dwDiamondNeed = pResProp->m_dwDiamondPrice * dwNum;

	if (!Consume::Instance().IsEnoughDiamond(pstData, dwDiamondNeed))
	{
		return ERR_NOT_ENOUGH_DIAMOND;
	}

	Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -dwDiamondNeed, rstScPkgBodyRsp.m_stSyncItemInfo, DWLOG::METHOD_PROPS_BUY_PROP);
	Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, dwId, dwNum, rstScPkgBodyRsp.m_stSyncItemInfo, DWLOG::METHOD_PROPS_BUY_PROP);

	return ERR_NONE;
}


int Props::CompositeProps(PlayerData* pstData, uint32_t dwId, uint32_t dwNum, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
    RESPROPS* pResProp = CGameDataMgr::Instance().GetResPropsMgr().Find(dwId);
    if (!pResProp)
    {
        LOGERR("pResProp is null");
        return ERR_NOT_FOUND;
    }

    if (pResProp->m_bCanBeComposite != 1)
    {
        return ERR_NOT_SATISFY_COND;
    }

    if (dwNum == 0)
    {
        return ERR_NONE;
    }

    uint32_t dwNeedNum = dwNum * pResProp->m_wCompositeNumber;

    if (!Props::Instance().IsEnough(pstData, dwId, dwNeedNum))
    {
        LOGERR("Uin<%lu> have no enough props.Prop<%u> Num<%u> ", pstData->m_ullUin, dwId, dwNeedNum);
        return ERR_NOT_ENOUGH_PROPS;
    }

    //消耗旧的道具
    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, dwId, -dwNeedNum, rstSyncItemInfo, 1);

    //合成新的道具
    Item::Instance().RewardItem(pstData, pResProp->m_bCompositeItemType, pResProp->m_dwCompositeProps, dwNum, rstSyncItemInfo, 1);

    return ERR_NONE;
}


//  开宝箱,调用此函数需要保证id为保宝箱的物品ID
int Props::OpenTreasureBox(PlayerData* pstData, uint32_t dwPropId, uint32_t dwNum, DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint32_t dwApproach)
{
    dwNum = MIN(dwNum, MAX_BATCH_NUM);
    if (!Props::Instance().IsEnough(pstData, dwPropId, dwNum))
    {
        LOGERR("Uin<%lu> have no enough props.Prop<%u> Num<%u> ", pstData->m_ullUin, dwPropId, dwNum);
        return ERR_NOT_ENOUGH_PROPS;
    }

    RESPROPS* pstResProps = CGameDataMgr::Instance().GetResPropsMgr().Find(dwPropId);
    if (pstResProps == NULL)
    {
        LOGERR("Uin<%lu> the pstResProps is null.Prop<%u> Num<%u> ", pstData->m_ullUin, dwPropId, dwNum);
        return ERR_SYS;
    }
    uint32_t dwBoxID = pstResProps->m_dwPropsParam;
    RESTREASUREBOX* pstResBox = CGameDataMgr::Instance().GetResTreasureBoxMgr().Find(dwBoxID);
    if (pstResBox == NULL )
    {
        LOGERR("Uin<%lu> the pstResBox is null.Prop<%u> Num<%u> BoxId<%u>", pstData->m_ullUin, dwPropId, dwNum, dwBoxID);
        return ERR_SYS;
    }

    //dwNum = GET_MAX_BATCH_NUM(pstResBox->m_bLotteryPoolCount, dwNum);
    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, dwPropId, -dwNum, rstSyncItemInfo, dwApproach);
    for (size_t i = 0; i < dwNum; i++)
    {
        for (size_t j = 0; j < pstResBox->m_bLotteryPoolCount; j++)
        {
#ifdef _DEBUG
            CpuSampleStats::Instance().BeginSample("Props::OpenTreasureBox::OnceDraw::Id<%u> Cnt<%u>", dwPropId, dwNum * pstResBox->m_bLotteryPoolCount);
#endif
            Lottery::Instance().DrawLotteryByPond(pstData, pstResBox->m_lotteryPoolIdList[j], rstSyncItemInfo, dwApproach, true);
#ifdef _DEBUG
            CpuSampleStats::Instance().EndSample();
#endif
        }
    }
    return ERR_NONE;
}

int Props::OpenChosenBox(PlayerData* pstData, uint32_t dwPropId, uint32_t dwNum, uint8_t bIndex, DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint32_t dwApproach)
{
    rstSyncItemInfo.m_bSyncItemCount = 0;
    dwNum = MIN(dwNum, MAX_BATCH_NUM);
    if (bIndex >= RES_MAX_TREASURE_BOX_REWARD)
    {
        LOGERR("Uin<%lu> the args invalid. bIndex<%hhu> dwPropId<%u> dwNum<%u>", pstData->m_ullUin, bIndex, dwPropId, dwNum);
        return ERR_WRONG_PARA;
    }
	if (!Props::Instance().IsEnough(pstData, dwPropId, dwNum))
	{
		LOGERR("Uin<%lu> have no enough props.Index<%u> Prop<%u>  dwNum<%u>", pstData->m_ullUin, bIndex, dwPropId,  dwNum);
		return ERR_NOT_ENOUGH_PROPS;
	}

	RESPROPS* pstResProps = CGameDataMgr::Instance().GetResPropsMgr().Find(dwPropId);
	if (pstResProps == NULL)
	{
		LOGERR("Uin<%lu> the pstResProps is null.Prop<%u> Index<%u> ", pstData->m_ullUin, dwPropId, bIndex);
		return ERR_SYS;
	}
	uint32_t dwBoxID = pstResProps->m_dwPropsParam;
	RESTREASUREBOX* pstResBox = CGameDataMgr::Instance().GetResTreasureBoxMgr().Find(dwBoxID);
	if (pstResBox == NULL)
	{
		LOGERR("Uin<%lu> the pstResBox is null.Prop<%u> Index<%u> BoxId<%u>", pstData->m_ullUin, dwPropId, bIndex, dwBoxID);
		return ERR_SYS;
	}

	//Lottery::Instance().DrawLotteryByPond(pstData, pstResBox->m_lotteryPoolIdList[bIndex], rstRewardItemInfo, rstSyncItemInfo, dwApproach);
    if (pstResBox->m_szRewardItemTypeList[bIndex] == ITEM_TYPE_GCARD)
    {
        //选择武将只能一个一个操作
        dwNum = 1;
        Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, dwPropId, -dwNum, rstSyncItemInfo, dwApproach);
        Item::Instance().RewardItem(pstData, pstResBox->m_szRewardItemTypeList[bIndex], pstResBox->m_rewardItemIDList[bIndex], pstResBox->m_szRewardNumList[bIndex],
            rstSyncItemInfo, dwApproach);
    }
    else
    {
        Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, dwPropId, -dwNum, rstSyncItemInfo, dwApproach);
        Item::Instance().RewardItem(pstData, pstResBox->m_szRewardItemTypeList[bIndex], pstResBox->m_rewardItemIDList[bIndex], dwNum * pstResBox->m_szRewardNumList[bIndex],
            rstSyncItemInfo, dwApproach);
    }

	return ERR_NONE;

}

int Props::OpenLevelBox(PlayerData* pstData, uint32_t dwPropId, uint32_t dwNum, DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint32_t dwApproach)
{
    dwNum = MIN(dwNum, MAX_BATCH_NUM);
	if (!Props::Instance().IsEnough(pstData, dwPropId, dwNum))
	{
		LOGERR("Uin<%lu> have no enough props.Prop<%u> Num<%u> ", pstData->m_ullUin, dwPropId, dwNum);
		return ERR_NOT_ENOUGH_PROPS;
	}

	RESPROPS* pstResProps = CGameDataMgr::Instance().GetResPropsMgr().Find(dwPropId);
	if (pstResProps == NULL)
	{
		LOGERR("Uin<%lu> the pstResProps is null.Prop<%u> Num<%u> ", pstData->m_ullUin, dwPropId, dwNum);
		return ERR_SYS;
	}
	uint32_t dwLevelBoxID = pstResProps->m_dwPropsParam;
	RESLEVELBOX* pstResBox = CGameDataMgr::Instance().GetResLevelBoxMgr().Find(dwLevelBoxID);
	if (pstResBox == NULL)
	{
		LOGERR("Uin<%lu> the pstResBox is null.Prop<%u> Num<%u> dwLevelBoxID<%u>", pstData->m_ullUin, dwPropId, dwNum, dwLevelBoxID);
		return ERR_SYS;
	}

	uint32_t dwLevel = pstData->GetMajestyInfo().m_wLevel;
	uint32_t dwBoxID = 0;
	for (int i=0; i<pstResBox->m_bLevelCutNum; i++)
	{
		if (dwLevel >= pstResBox->m_levels[i])
		{
			dwBoxID = pstResBox->m_paramList[i];
		}
		else
		{
			break;
		}
	}
	if ( 0 == dwBoxID )
	{
		LOGERR("BoxID=0,check the bytes files.");
		return ERR_SYS;
	}

	ResTreasureBoxMgr_t& rResTreasureBox = CGameDataMgr::Instance().GetResTreasureBoxMgr();
	RESTREASUREBOX* pstResTreasureBox = rResTreasureBox.Find(dwBoxID);
	if (pstResTreasureBox == NULL )
	{
		LOGERR("Uin<%lu> the pstResTreasureBox is null.Prop<%u> Num<%u> dwBoxID<%u>", pstData->m_ullUin, dwPropId, dwNum, dwBoxID);
		return ERR_SYS;
	}
    //dwNum = GET_MAX_BATCH_NUM(pstResTreasureBox->m_bLotteryPoolCount, dwNum);
	Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, dwPropId, -dwNum, rstSyncItemInfo, dwApproach);

	for (size_t i = 0; i < dwNum; i++)
	{
		for (size_t j = 0; j < pstResTreasureBox->m_bLotteryPoolCount; j++)
		{
#ifdef _DEBUG
            CpuSampleStats::Instance().BeginSample("Props::OpenTreasureBox::OnceDraw::Id<%u> Cnt<%u>", dwPropId, dwNum * pstResTreasureBox->m_bLotteryPoolCount);
#endif
			Lottery::Instance().DrawLotteryByPond(pstData, pstResTreasureBox->m_lotteryPoolIdList[j], rstSyncItemInfo, dwApproach, true);
#ifdef _DEBUG
            CpuSampleStats::Instance().EndSample();
#endif
		}
	}
	return ERR_NONE;
}

int Props::GetCommonAward(PlayerData* pstData, uint32_t dwAwardId, DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint32_t dwApproach)
{
    RESCOMMONREWARD* pstReward = CGameDataMgr::Instance().GetResCommonRewardMgr().Find(dwAwardId);
    if (pstReward == NULL)
    {
        LOGERR("Uin<%lu> the ResCommonReward is null.dwAwardId<%u>", pstData->m_ullUin, dwAwardId);
        return ERR_SYS;
    }
    for (size_t i = 0; i < pstReward->m_bNum; i++)
    {
        Item::Instance().RewardItem(pstData, pstReward->m_szPropstype[i], pstReward->m_propsId[i], pstReward->m_propsNum[i],
            rstSyncItemInfo, dwApproach);
    }
    return ERR_NONE;
}

#define LEVEL_BOX_TYPE (26)
bool Props::IsLevelBox(uint32_t dwPropId)
{
	ResPropsMgr_t& rResPropsMgr = CGameDataMgr::Instance().GetResPropsMgr();
	RESPROPS* pstProps = rResPropsMgr.Find(dwPropId);
	if (!pstProps)
	{
		LOGERR("pstProps is null.");
		return false;
	}

	if ( LEVEL_BOX_TYPE == pstProps->m_dwType )
	{
		return true;
	}
	else
	{
		return false;
	}

}

