#include "Item.h"
#include "../gamedata/GameDataMgr.h"
#include "LogMacros.h"
#include "Equip.h"
#include "GeneralCard.h"
#include "Props.h"
#include "Consume.h"
#include "player/Player.h"
#include "Majesty.h"
#include "AP.h"
#include "Guild.h"
#include "ZoneLog.h"
#include "dwlog_svr.h"
#include "Marquee.h"
#include "VIP.h"
#include "GloryItemsMgr.h"
#include "SkillPoint.h"
#include "Task.h"
#include "Message.h"
#include "Skin.h"

using namespace std;
using namespace PKGMETA;
using namespace DWLOG;

Item::Item()
{

}

Item::~Item()
{

}

int ItemCmp(const void *pstFirst, const void *pstSecond)
{
    DT_ITEM* pstItemFirst = (DT_ITEM*)pstFirst;
    DT_ITEM* pstItemSecond = (DT_ITEM*)pstSecond;


    if (pstItemFirst->m_bItemType < pstItemSecond->m_bItemType)
    {
        return -1;
    }
    else if (pstItemFirst->m_bItemType > pstItemSecond->m_bItemType)
    {
        return 1;
    }
    else
    {
        //type相等
        if (pstItemFirst->m_dwItemId < pstItemSecond->m_dwItemId)
        {
            return -1;
        }
        else if (pstItemFirst->m_dwItemId > pstItemSecond->m_dwItemId)
        {
            return 1;
        }
        else
        {
            return 0;
        }

    }

}


int Item::AddItem(PlayerData* pstData, uint8_t bItemType, uint32_t dwId, int32_t iNum, DT_ITEM& rstItem, uint32_t dwApproach)
{
    int iRet = 0;
    rstItem.m_bItemType = bItemType;
    rstItem.m_dwItemId = dwId;
    rstItem.m_iValueChg = iNum;
    rstItem.m_bShowProperty = CONST_SHOW_PROPERTY_NO;
    switch (bItemType)
    {
    case ITEM_TYPE_GCARD:
        {
			int32_t iStar = iNum; //对于武将卡，iNum代表星级
            iStar = (iNum > MAX_GCARD_START_NUM ? MAX_GCARD_START_NUM : iNum);
            rstItem.m_stItemData.m_stGCard.m_dwPropId = 0;
            rstItem.m_stItemData.m_stGCard.m_dwPropNum = 0;
            DT_ITEM_GCARD* pstItem = GeneralCard::Instance().Find(pstData, dwId);
            if (pstItem == NULL)
            {
                //增加
                pstItem = GeneralCard::Instance().AddWrapPrimary(pstData, dwId, iStar);
                if (pstItem == NULL)
                {
                    LOGERR("Uin<%lu> add the GCard<%u> error", pstData->m_ullUin, dwId);
                    return ERR_SYS;
                }
                rstItem.m_stItemData.m_stGCard = *pstItem;
                //拥有X星武将个数
                for (uint8_t i = 1; i <= iStar; i++)
                {
                    //只需在获取武将的时候这么处理
                    Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_GENERAL_LEVEL, 1/*value*/, 2, i/*当前星级*/);
                }
                ZoneLog::Instance().WriteGeneralGetLog(pstData, dwId, dwApproach);/* 奖励获得武将 */
                Marquee::Instance().SaveMarqueeForGCard(pstData,  Marquee::GCARD_GAIN, dwId, iStar);
                if (iStar >= 4)
                {
                    Message::Instance().AutoSendSysMessage(1301, "Name=%s|Star=%d|GCardId=%u", pstData->GetRoleName(), iStar, dwId);
                    Message::Instance().AutoSendWorldMessage(pstData, 1302, "Star=%d|GCardId=%u", iStar, dwId);
                }
                while (iStar >= 2)
                {
                    Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_GCARD_STAR, 1, 1, dwId, iStar);
                    --iStar;
                }
            }
            else
            {
                // 已有武将,分解处理
                rstItem.m_stItemData.m_stGCard = *pstItem;
                // 重置品阶和星级, 前台展示需要,其他数据不关心
                rstItem.m_stItemData.m_stGCard.m_bPhase = 1;
                rstItem.m_stItemData.m_stGCard.m_bStar = iStar;
                LOGRUN("Uin<%lu> exist general-%u exchange, Appproach<%u>", pstData->m_ullUin, dwId, dwApproach);
                uint32_t dwFragmentId = 0;
                int iGcardFragmentNum = GeneralCard::Instance().GetFragmentNum(pstData, dwId, iStar, &dwFragmentId);
                if (iGcardFragmentNum < 0)
                {
                    LOGERR("Uin<%lu>, GCardId<%u> star<%u> is not found", pstData->m_ullUin, dwId, iStar);
                    return iGcardFragmentNum;
                }

                rstItem.m_stItemData.m_stGCard.m_dwPropNum = iGcardFragmentNum;
                rstItem.m_stItemData.m_stGCard.m_dwPropId = dwFragmentId;
                //增加碎片到包裹中
                DT_ITEM stTmpItem;
                iRet = AddItem(pstData, ITEM_TYPE_PROPS, dwFragmentId, iGcardFragmentNum, stTmpItem, dwApproach);
            }
            if (dwApproach >= METHOD_LOTTERY_DRAW_TYPE_GOLD && dwApproach <= METHOD_LOTTERY_DRAW_TYPE_DIAMOND_CNT)
            {
                Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_GET_GCARD, 1/*value*/, 1, dwId/*武将ID*/); //普通抽奖
                Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_GET_GCARD, 1/*value*/, 3, dwId/*武将ID*/);// 所有抽奖
            }
            else if (dwApproach >= METHOD_LOTTERY_DRAW_TYPE_ACT && dwApproach <= METHOD_LOTTERY_DRAW_TYPE_ACT_CNT_COMMON)
            {
                Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_GET_GCARD, 1/*value*/, 2, dwId/*武将ID*/); //魂匣抽奖
                Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_GET_GCARD, 1/*value*/, 3, dwId/*武将ID*/);   //所有抽奖
            }
            Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_GET_GCARD, 1/*value*/, 4, dwId/*武将ID*/);   //所有途径获得武将
            break;
        }

    case ITEM_TYPE_EQUIP:
        {
            return -1;
            break;
        }
    case ITEM_TYPE_PROPS:
        {
            iRet = Props::Instance().Add(pstData, dwId, iNum, dwApproach);
            if (iRet < 0)
            {
                return iRet;
            }

            DT_ITEM_PROPS* pstItem = Props::Instance().Find(pstData, dwId);
            if (!pstItem)
            {
                // 消耗完毕在服务器会被删除掉
                LOGRUN("Props id = %u not exist.", dwId);
                rstItem.m_dwValueAfterChg = 0;
                rstItem.m_stItemData.m_stProps.m_dwId = dwId;
                rstItem.m_stItemData.m_stProps.m_dwNum = 0;
            }
            else
            {
                rstItem.m_dwValueAfterChg = pstItem->m_dwNum;
                rstItem.m_stItemData.m_stProps.m_dwId = dwId;
                rstItem.m_stItemData.m_stProps.m_dwNum = pstItem->m_dwNum;
            }
            if (iNum > 0)
            {
                //特殊道具广播
                if ((dwId > 1000 && dwId < 2000) || (dwId >= 2101 && dwId <= 2206)) //广播获取专属武器的道具
                {
                    Message::Instance().AutoSendSysMessage(1201, "Name=%s|PropId=%u", pstData->GetRoleName(), dwId);
                }
            }
            break;
        }
    case ITEM_TYPE_MSKILL:
        {
            break;
        }
    case ITEM_TYPE_GOLD:
        {
            rstItem.m_dwValueAfterChg = Consume::Instance().AddGold(pstData, iNum, dwApproach);
            break;
        }
    case ITEM_TYPE_YUAN:
        {
            rstItem.m_dwValueAfterChg = Consume::Instance().AddYuan(pstData, iNum, dwApproach);
            break;
        }
    case ITEM_TYPE_DIAMOND:
        {
            rstItem.m_dwValueAfterChg = Consume::Instance().AddDiamond(pstData, iNum, dwApproach);
            break;
        }
    case ITEM_TYPE_EXP:
        {
            rstItem.m_dwValueAfterChg = Majesty::Instance().AddExp(pstData, iNum);
            break;
        }
    case ITEM_TYPE_AP:
        {
            rstItem.m_dwValueAfterChg = AP::Instance().AddNoLimit(pstData, iNum);
            DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
            rstItem.m_stItemData.m_stAP.m_ullLastResumeTime = rstMajestyInfo.m_ullAPResumeLastTime;
            break;
        }
    case ITEM_TYPE_GUILD_CONTRIBUTION:
        {
            rstItem.m_dwValueAfterChg = Guild::Instance().AddGuildContribution(pstData, iNum, dwApproach);
            break;
        }
    case ITEM_TYPE_GUILD_VITALITY:
        {
            rstItem.m_dwValueAfterChg = Guild::Instance().AddGuildVitality(pstData, iNum, dwApproach);
            break;
        }
    case ITEM_TYPE_VIP_EXP:
        {
            rstItem.m_dwValueAfterChg = VIP::Instance().AddExp(pstData, iNum, dwApproach);
            break;
        }
	case ITEM_TYPE_MAJESTY_ITEM:
		{
			GloryItemsMgr::Instance().AddMajestyItems(pstData, dwId);
			break;
		}
	case ITEM_TYPE_GENERAL_COIN:
		{
			rstItem.m_dwValueAfterChg = Consume::Instance().AddGeneralCoin(pstData, iNum, dwApproach);
			break;
		}
	case ITEM_TYPE_EQUIP_COIN:
		{
			rstItem.m_dwValueAfterChg = Consume::Instance().AddEquipCoin(pstData, iNum, dwApproach);
			break;
		}
	case ITEM_TYPE_SYNC_PVP_COIN:
		{
			rstItem.m_dwValueAfterChg = Consume::Instance().AddSyncPVPCoin(pstData, iNum, dwApproach);
			break;
		}
	case ITEM_TYPE_ASYNC_PVP_COIN:
		{
			rstItem.m_dwValueAfterChg = Consume::Instance().AddAsyncPVPCoin(pstData, iNum, dwApproach);
			break;
		}
    case ITEM_TYPE_SKILL_POINT:
		{
			rstItem.m_dwValueAfterChg = SkillPoint::Instance().Add(pstData, iNum);
            DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
            rstItem.m_stItemData.m_stSkillPoint.m_ullLastResumeTime = rstMajestyInfo.m_ullSPResumeLastTime;
			break;
		}
    case ITEM_TYPE_PEAK_ARENA_COIN:
        {
            rstItem.m_dwValueAfterChg = Consume::Instance().AddPeakArenaCoin(pstData, iNum, dwApproach);
			break;
        }
    case ITEM_TYPE_VIRTUAL_EXP:
        {
            break;
        }
    case ITEM_TYPE_AWAKE_SHOP_COIN:
        {
            rstItem.m_dwValueAfterChg = Consume::Instance().AddAwakeEquipCoin(pstData, iNum, dwApproach);
            break;
        }
	case ITEM_TYPE_EXPEDITION_COIN:
	{
		rstItem.m_dwValueAfterChg = Consume::Instance().AddExpeditionCoin(pstData, iNum, dwApproach);
		break;
	}
    case ITEM_TYPE_TASK_ACTIVATION:
    {
        rstItem.m_dwValueAfterChg = Task::Instance().AddActivation(pstData, iNum);
		break;
    }
    case ITEM_TYPE_SKIN:
    {
        DT_ITEM_SKIN &rstSkin = rstItem.m_stItemData.m_stSkin;
        rstSkin.m_dwId = dwId;
        rstSkin.m_llEndTime = -1;
        Skin::Instance().AddSkin(pstData, rstSkin);
		break;
    }
    default:
        LOGERR("err type %u", bItemType);
        return -1;
    }
    return 0;
}


bool Item::IsEnough(PlayerData* pstData, uint8_t bItemType, uint32_t dwId, int32_t iNum)
{
    bool bRet = false;
    switch (bItemType)
    {
    case ITEM_TYPE_GOLD:
    case ITEM_TYPE_YUAN:
    case ITEM_TYPE_DIAMOND:
        bRet = Consume::Instance().IsEnough(pstData, bItemType, iNum);
        break;
    case ITEM_TYPE_PROPS:
        bRet = Props::Instance().IsEnough(pstData,dwId, iNum);
        break;
    default:
        bRet = false;
        break;
    }
    return bRet;
}
/*
int Item::RewardItem(PlayerData * pstData, uint8_t bItemType, uint32_t dwId, int32_t iNum, DT_SYNC_ITEM_INFO & rstSyncItemInfo, uint32_t dwApproach, uint8_t bShowProperty)
{
    int iRet = ERR_NONE;
    DT_ITEM& rstItem = rstSyncItemInfo.m_astSyncItemList[rstSyncItemInfo.m_bSyncItemCount];
    if (bItemType == ITEM_TYPE_GCARD)
    {
        if (rstSyncItemInfo.m_bSyncItemCount >= MAX_SYNC_ITEM_NUM)
        {
            LOGERR("Uin<%lu> DealItemChange:no space,Type<%hhu> id<%u> Num<%d> ", pstData->m_ullUin, bItemType, dwId, iNum);
            return ERR_SYS;
        }
        bool bIsAddSuccess = false;
        int32_t iStar = iNum; //对于武将卡，iNum代表星级
        if (NULL == GeneralCard::Instance().Find(pstData, dwId))
        {
            // 不需要分解
            iRet = AddItem(pstData, bItemType, dwId, iStar, rstItem, dwApproach);
            if (iRet == ERR_NONE)
            {
                //需要展示
                rstItem.m_bShowProperty = bShowProperty;
                rstItem.m_bIsReplace = CONST_ISREPLACE_FALSE;
                rstSyncItemInfo.m_bSyncItemCount++;
                bIsAddSuccess = true;
            }
        }
        if (!bIsAddSuccess)
        {
            // 有分解
            if (rstSyncItemInfo.m_bSyncItemCount >= (MAX_SYNC_ITEM_NUM - 1))
            {
                LOGERR("Uin<%lu> DealItemChange:no space for Fragment:Type<%hhu> id<%u> Num<%d> ", pstData->m_ullUin, bItemType, dwId, iNum);
                return ERR_SYS;
            }
            DT_ITEM& rstItemReplace = rstSyncItemInfo.m_astSyncItemList[rstSyncItemInfo.m_bSyncItemCount + 1];
            RESGENERAL* pstResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(dwId);
            if (pstResGeneral == NULL)
            {
                LOGERR("Uin<%lu> dwId-%u is not found", pstData->m_ullUin, dwId);
                return ERR_SYS;
            }
            else
            {
                LOGRUN("Uin<%lu> exist general-%u exchange", pstData->m_ullUin, dwId);
                int iGcardFragmentNum = GeneralCard::Instance().GetFragmentNum(pstData, dwId, iStar);
                if (iGcardFragmentNum < 0)
                {
                    LOGERR("Uin<%lu>, GCardId<%u> star<%u> is not found", pstData->m_ullUin, dwId, iStar);
                    return iGcardFragmentNum;
                }
                //增加替换物品
                iRet = AddItem(pstData, ITEM_TYPE_PROPS, pstResGeneral->m_dwExchangeId, iGcardFragmentNum, rstItemReplace, dwApproach);
                if (iRet == ERR_NONE)
                {
                    //不展示替换后的物品
                    rstItemReplace.m_bShowProperty = CONST_SHOW_PROPERTY_NO;
                    //增加原始的物品
                    rstItem.m_bItemType = bItemType;
                    rstItem.m_dwItemId = dwId;
                    rstItem.m_iValueChg = 1;

                    rstItem.m_stItemData.m_stGCard.m_dwId = dwId;
                    rstItem.m_stItemData.m_stGCard.m_bLevel = 1;
                    rstItem.m_stItemData.m_stGCard.m_bStar = iStar;
                    rstItem.m_stItemData.m_stGCard.m_bPhase = 1;
                    rstItem.m_stItemData.m_stGCard.m_dwExp = 0;
                    rstItem.m_bShowProperty = bShowProperty;   //普通展示
                    rstItem.m_bIsReplace = CONST_ISREPLACE_TRUE;
                    rstSyncItemInfo.m_bSyncItemCount += 2;
                }
            }
        }
    }
    else
    {
        iRet = AddItem(pstData, bItemType, dwId, iNum, rstItem, dwApproach);
        if (iRet == ERR_NONE)
        {
            rstItem.m_bShowProperty = bShowProperty;
            rstSyncItemInfo.m_bSyncItemCount++;
        }
    }
    return iRet;

}
*/
int Item::ConsumeItem(PlayerData * pstData, uint8_t bItemType, uint32_t dwId, int32_t iNum, DT_SYNC_ITEM_INFO & rstSyncItemInfo, uint32_t dwApproach)
{
    if (rstSyncItemInfo.m_bSyncItemCount >= MAX_SYNC_ITEM_NUM)
    {
        LOGERR("Uin<%lu> no space", pstData->m_ullUin);
        return -1;
    }
    int iRet = AddItem(pstData, bItemType, dwId, iNum, rstSyncItemInfo.m_astSyncItemList[rstSyncItemInfo.m_bSyncItemCount], dwApproach);
    if (iRet == ERR_NONE)
    {
        rstSyncItemInfo.m_bSyncItemCount++;
    }
    return iRet;
}

int Item::RewardItem(PlayerData * pstData, uint8_t bItemType, uint32_t dwId, int32_t iNum,
    DT_SYNC_ITEM_INFO & rstSyncItemInfo, uint32_t dwApproach, uint8_t bShowProperty, bool bIsMerge)
{
    if (bIsMerge)
    {
        return RewardItemByMerge(pstData, bItemType, dwId, iNum, rstSyncItemInfo, dwApproach, bShowProperty);
    }
    DT_ITEM*  pstDestItem = NULL;

    if (rstSyncItemInfo.m_bSyncItemCount >= MAX_SYNC_ITEM_NUM)
    {
        //没有空间也应该增加物品
        LOGERR("Uin<%lu> no space! Approach<%u>", pstData->m_ullUin, dwApproach);
        pstDestItem = &rstSyncItemInfo.m_astSyncItemList[rstSyncItemInfo.m_bSyncItemCount-1];
    }
    else
    {
        pstDestItem = &rstSyncItemInfo.m_astSyncItemList[rstSyncItemInfo.m_bSyncItemCount];
    }

    int iRet = AddItem(pstData, bItemType, dwId, iNum, *pstDestItem, dwApproach);
    if (iRet != ERR_NONE)
    {
        LOGERR("Uin<%lu> AddItem error! Approach<%u>", pstData->m_ullUin, dwApproach);
        return iRet;
    }
    pstDestItem->m_bShowProperty = bShowProperty;
    rstSyncItemInfo.m_bSyncItemCount = MIN(rstSyncItemInfo.m_bSyncItemCount + 1, MAX_SYNC_ITEM_NUM);
    return ERR_NONE;
}


int Item::RewardItemByMerge(PlayerData* pstData, uint8_t bItemType, uint32_t dwId, int32_t iNum, DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint32_t dwApproach, uint8_t bShowProperty)
{
    DT_ITEM  stTmpItem;
    int iRet = AddItem(pstData, bItemType, dwId, iNum, stTmpItem, dwApproach);
    if (iRet != ERR_NONE)
    {
        LOGERR("Uin<%lu> AddItem error! Approach<%u>", pstData->m_ullUin, dwApproach);
        return iRet;
    }
    stTmpItem.m_bShowProperty = bShowProperty;
    int iEqual = 0;
    int iIndex = MyBSearch(&stTmpItem, rstSyncItemInfo.m_astSyncItemList, rstSyncItemInfo.m_bSyncItemCount, sizeof(DT_ITEM), &iEqual, ItemCmp);
    if (iEqual)
    {
        //有,合并
        DT_ITEM& rstDestItem = rstSyncItemInfo.m_astSyncItemList[iIndex];
        rstDestItem.m_iValueChg += stTmpItem.m_iValueChg;
        rstDestItem.m_dwValueAfterChg = stTmpItem.m_dwValueAfterChg;
        switch (rstDestItem.m_bItemType)
        {
        case ITEM_TYPE_GCARD:
            {
                //返回的是当前获得的值,需要累加合并
                rstDestItem.m_stItemData.m_stGCard.m_dwPropNum += stTmpItem.m_stItemData.m_stGCard.m_dwPropNum;
            }
            break;
        case ITEM_TYPE_PROPS:
            {
                //返回的总数,直接赋值
                rstDestItem.m_stItemData.m_stProps.m_dwNum = stTmpItem.m_stItemData.m_stProps.m_dwNum;
            }
            break;
        default:
            break;
        }
    }
    else
    {
        //插入新的
        //插入新的数据
        if (rstSyncItemInfo.m_bSyncItemCount >= MAX_SYNC_ITEM_NUM)
        {
            LOGERR("Uin<%lu> no space", pstData->m_ullUin);
            return ERR_SYS;
        }
        size_t nmemb = (size_t)rstSyncItemInfo.m_bSyncItemCount;
        MyBInsert(&stTmpItem, rstSyncItemInfo.m_astSyncItemList, &nmemb, sizeof(DT_ITEM), 1, ItemCmp);
        rstSyncItemInfo.m_bSyncItemCount = (int32_t)nmemb;
    }
    return 0;
}




