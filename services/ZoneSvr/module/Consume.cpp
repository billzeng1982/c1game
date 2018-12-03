#include "Consume.h"
#include "LogMacros.h"
#include "../gamedata/GameDataMgr.h"
#include "Item.h"
#include "ZoneLog.h"
#include "dwlog_svr.h"
#include "strutil.h"
#include "player/Player.h"
#include "VIP.h"
#include "Task.h"
#include "Props.h"
#include "Pay.h"

using namespace PKGMETA;
using namespace DWLOG;

#define MAX_PLAYER_GOLD_NUM      (99999999)
#define MAX_PLAYER_DIAMOND_NUM   (99999999)
#define MAX_PLAYER_YUAN_NUM   (99999999)
#define MAX_PALYER_TICKET_NUM (65536)



bool Consume::IsEnough(PlayerData* pstData, uint8_t bType, uint32_t dwId, uint32_t dwValue)
{
    switch (bType)
    {
    case ITEM_TYPE_PROPS:
        return Props::Instance().IsEnough(pstData, dwId, dwValue);
    default:
        return this->IsEnough(pstData, bType, dwValue);
    }
}


bool Consume::IsEnough(PlayerData* pstData, uint8_t bType, uint32_t dwValue)
{
	switch (bType)
	{
	case ITEM_TYPE_DIAMOND:
		return IsEnoughDiamond(pstData, dwValue);
	case ITEM_TYPE_GOLD:
		return IsEnoughGold(pstData, dwValue);
	case ITEM_TYPE_GUILD_CONTRIBUTION:
		return IsEnoughGuildContribution(pstData, dwValue);
	case ITEM_TYPE_YUAN:
		return IsEnoughYuan(pstData, dwValue);
	case ITEM_TYPE_GENERAL_COIN:
		return IsEnoughGeneralCoin(pstData, dwValue);
	case ITEM_TYPE_EQUIP_COIN:
		return IsEnoughEquipCoin(pstData, dwValue);
	case ITEM_TYPE_SYNC_PVP_COIN:
		return IsEnoughSyncPVPCoin(pstData, dwValue);
	case ITEM_TYPE_ASYNC_PVP_COIN:
		return IsEnoughAsyncPVPCoin(pstData, dwValue);
    case ITEM_TYPE_PEAK_ARENA_COIN:
		return IsEnoughPeakArenaCoin(pstData, dwValue);
    case ITEM_TYPE_AWAKE_SHOP_COIN:
        return IsEnoughAwakeEquipCoin(pstData, dwValue);
	case ITEM_TYPE_EXPEDITION_COIN:
		return IsEnoughExpeditionCoin(pstData, dwValue);
	default:
		return false;
	}
}

int Consume::PurchaseGold(PlayerData* pstData, uint32_t dwNum, SC_PKG_PURCHASE_RSP& rstScPkgBodyRsp)
{
    if (dwNum <= 0)
    {
        LOGERR("PurchaseGold dwNum is 0");
        return ERR_SYS;
    }

    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
#if true
    if (rstMajestyInfo.m_bGoldPurchaseTimes + dwNum > rstMajestyInfo.m_bGoldPurchaseTimesMax)
    {
        return ERR_AP_PURCHASE_LIMIT;
    }
#endif

    RESPURCHASE* pstResPurchase = CGameDataMgr::Instance().GetResPurchaseMgr().Find(PURCHASE_ID_GOLD);
    if (pstResPurchase == NULL)
    {
        LOGERR("pstResPurchase is NULL");
        return ERR_SYS;
    }

    uint32_t dwConsume = 0;
    uint32_t dwGain = 0;
    for (uint32_t i=0; i<dwNum; i++)
    {
        dwConsume += pstResPurchase->m_consumeNumber[rstMajestyInfo.m_bGoldPurchaseTimes + i];
        dwGain += pstResPurchase->m_gainNumber[rstMajestyInfo.m_bGoldPurchaseTimes + i];
    }

    if (!IsEnoughDiamond(pstData, dwConsume))
    {
        return ERR_NOT_ENOUGH_DIAMOND;
    }

    rstMajestyInfo.m_bGoldPurchaseTimes += dwNum;

    rstScPkgBodyRsp.m_bPurchaseTimes = rstMajestyInfo.m_bGoldPurchaseTimes;

    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -dwConsume, rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_CONSUME_BUY_GOLD);
    Item::Instance().RewardItem(pstData, ITEM_TYPE_GOLD, 0, dwGain, rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_CONSUME_BUY_GOLD);

	//任务修改
	Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_BUY, 1, 2/*para1*/);

    return ERR_NONE;
}

uint32_t Consume::Add(PlayerData* pstData, uint8_t bType, int32_t iValue, uint32_t dwApproach)
{
	switch (bType)
	{
	case ITEM_TYPE_DIAMOND:
		return AddDiamond(pstData, iValue, dwApproach);
	case ITEM_TYPE_GOLD:
		return AddGold(pstData, iValue, dwApproach);
	case ITEM_TYPE_YUAN:
		return AddYuan(pstData, iValue, dwApproach);
	case ITEM_TYPE_GUILD_CONTRIBUTION:
		return AddGuildContribution(pstData, iValue, dwApproach);
	case ITEM_TYPE_GENERAL_COIN:
		return AddGeneralCoin(pstData, iValue, dwApproach);
	case ITEM_TYPE_EQUIP_COIN:
		return AddEquipCoin(pstData, iValue, dwApproach);
	case ITEM_TYPE_SYNC_PVP_COIN:
		return AddSyncPVPCoin(pstData, iValue, dwApproach);
	case ITEM_TYPE_ASYNC_PVP_COIN:
		return AddAsyncPVPCoin(pstData, iValue, dwApproach);
	case ITEM_TYPE_PEAK_ARENA_COIN:
		return AddPeakArenaCoin(pstData, iValue, dwApproach);
	case ITEM_TYPE_AWAKE_SHOP_COIN:
		return AddAwakeEquipCoin(pstData, iValue, dwApproach);
	case ITEM_TYPE_EXPEDITION_COIN:
		return AddExpeditionCoin(pstData, iValue, dwApproach);
	default:
		return 0;
	}
}


uint32_t Consume::BaseAdd(PlayerData* pstData, uint8_t bType, int32_t iValue, uint32_t dwApproach, INOUT uint32_t& rdwOutValue)
{
	uint8_t bChgType = 0;
	uint32_t dwChgValue = 0;
	if (iValue > 0)
	{
		// 加装备代币
		rdwOutValue += iValue;
		if (rdwOutValue > MAX_PLAYER_GOLD_NUM)
		{
			rdwOutValue = MAX_PLAYER_GOLD_NUM;
		}

		bChgType = 1;
		dwChgValue = iValue;
	}
	else
	{
		// 减装备代币
		uint32_t dwMinusNum = (uint32_t)(-iValue);
		if (rdwOutValue >= dwMinusNum)
		{
			rdwOutValue -= dwMinusNum;
		}
		else
		{
			rdwOutValue = 0;
		}

		bChgType = 2;
		dwChgValue = (uint32_t)(-iValue);
		if (bType == ITEM_TYPE_GOLD)
		{
			Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_RESOURCE_CONSUME, dwChgValue, 2);
		}
		else if (bType == ITEM_TYPE_DIAMOND)
		{
			Pay::Instance().DoActConsume(pstData, dwChgValue);
			Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_RESOURCE_CONSUME, dwChgValue, 1);
		}

	}

	if (dwChgValue != 0)
	{
		ZoneLog::Instance().WriteCoinLog(pstData, dwChgValue, bType, bChgType, rdwOutValue, dwApproach);
	}

	return rdwOutValue;
}

bool Consume::IsEnoughGold(PlayerData* pstData, uint32_t dwValue)
{
	return pstData->GetMajestyInfo().m_dwGold >= dwValue;
}

uint32_t Consume::AddGold(PlayerData* pstData, int32_t iValue, uint32_t dwApproach)
{

	return BaseAdd(pstData, ITEM_TYPE_GOLD, iValue, dwApproach, pstData->GetMajestyInfo().m_dwGold);
}

bool Consume::IsEnoughDiamond(PlayerData* pstData, uint32_t dwValue)
{
	return pstData->GetMajestyInfo().m_dwDiamond >= dwValue;
}

uint32_t Consume::AddDiamond(PlayerData* pstData, int32_t iValue, uint32_t dwApproach)
{
	return BaseAdd(pstData, ITEM_TYPE_DIAMOND, iValue, dwApproach, pstData->GetMajestyInfo().m_dwDiamond);
}

bool Consume::IsEnoughYuan(PlayerData* pstData, uint32_t dwValue)
{
	return pstData->GetMajestyInfo().m_dwYuan >= dwValue;
}

uint32_t Consume::AddYuan(PlayerData* pstData, int32_t iValue, uint32_t dwApproach)
{
	return BaseAdd(pstData, ITEM_TYPE_YUAN, iValue, dwApproach, pstData->GetMajestyInfo().m_dwYuan);
}

bool Consume::IsEnoughGeneralCoin(PlayerData* pstData, uint32_t dwValue)
{
	return pstData->GetMajestyInfo().m_dwGeneralCoin >= dwValue;
}

uint32_t Consume::AddGeneralCoin(PlayerData* pstData, int32_t iValue, uint32_t dwApproach)
{
	return BaseAdd(pstData, ITEM_TYPE_GENERAL_COIN, iValue, dwApproach, pstData->GetMajestyInfo().m_dwGeneralCoin);
}

bool Consume::IsEnoughEquipCoin(PlayerData* pstData, uint32_t dwValue)
{
	return pstData->GetMajestyInfo().m_dwEquipCoin >= dwValue;
}

uint32_t Consume::AddEquipCoin(PlayerData* pstData, int32_t iValue, uint32_t dwApproach)
{
	return BaseAdd(pstData, ITEM_TYPE_EQUIP_COIN, iValue, dwApproach, pstData->GetMajestyInfo().m_dwEquipCoin); 
}

bool Consume::IsEnoughSyncPVPCoin(PlayerData* pstData, uint32_t dwValue)
{
	return pstData->GetMajestyInfo().m_dwSyncPVPCoin >= dwValue;
}

uint32_t Consume::AddSyncPVPCoin(PlayerData* pstData, int32_t iValue, uint32_t dwApproach)
{
	return BaseAdd(pstData, ITEM_TYPE_SYNC_PVP_COIN, iValue, dwApproach, pstData->GetMajestyInfo().m_dwSyncPVPCoin); 
}

bool Consume::IsEnoughAsyncPVPCoin(PlayerData* pstData, uint32_t dwValue)
{
	return pstData->GetMajestyInfo().m_dwAsyncPVPCoin >= dwValue;
}

uint32_t Consume::AddAsyncPVPCoin(PlayerData* pstData, int32_t iValue, uint32_t dwApproach)
{
	return BaseAdd(pstData, ITEM_TYPE_ASYNC_PVP_COIN, iValue, dwApproach, pstData->GetMajestyInfo().m_dwAsyncPVPCoin); 
}

bool Consume::IsEnoughPeakArenaCoin(PlayerData* pstData, uint32_t dwValue)
{
    return pstData->GetMajestyInfo().m_dwPeakArenaCoin >= dwValue;
}

uint32_t Consume::AddPeakArenaCoin(PlayerData* pstData, int32_t iValue, uint32_t dwApproach)
{
	return BaseAdd(pstData, ITEM_TYPE_PEAK_ARENA_COIN, iValue, dwApproach, pstData->GetMajestyInfo().m_dwPeakArenaCoin);
}

bool Consume::IsEnoughAwakeEquipCoin(PlayerData * pstData, uint32_t dwValue)
{
    return pstData->GetMajestyInfo().m_dwAwakeEquipCoin >= dwValue;
}

uint32_t Consume::AddAwakeEquipCoin(PlayerData * pstData, int32_t iValue, uint32_t dwApproach)
{
	return BaseAdd(pstData, ITEM_TYPE_AWAKE_SHOP_COIN, iValue, dwApproach, pstData->GetMajestyInfo().m_dwAwakeEquipCoin); 
}


bool Consume::IsEnoughExpeditionCoin(PlayerData* pstData, uint32_t dwValue)
{
	return pstData->GetMajestyInfo().m_dwExpeditionCoin >= dwValue;
}


uint32_t Consume::AddExpeditionCoin(PlayerData* pstData, int32_t iValue, uint32_t dwApproach)
{
	return BaseAdd(pstData, ITEM_TYPE_EXPEDITION_COIN, iValue, dwApproach, pstData->GetMajestyInfo().m_dwExpeditionCoin);
}

bool Consume::IsEnoughGuildContribution(PlayerData* pstData, uint32_t dwValue)
{
	return pstData->GetGuildInfo().m_dwGuildContribution >= dwValue;
}

uint32_t Consume::AddGuildContribution(PlayerData* pstData, int32_t iValue, uint32_t dwApproach)
{
	return BaseAdd(pstData, ITEM_TYPE_GUILD_CONTRIBUTION, iValue, dwApproach, pstData->GetGuildInfo().m_dwGuildContribution);
}

