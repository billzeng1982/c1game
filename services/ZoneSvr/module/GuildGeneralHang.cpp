#include "GuildGeneralHang.h"
#include "LogMacros.h"
#include "dwlog_def.h"
#include "GameTime.h"
#include "../gamedata/GameDataMgr.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "GameTime.h"
#include "GeneralCard.h"
#include "Item.h"


using namespace PKGMETA;

bool GuildGeneralHang::Init()
{
	RESBASIC* pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find((int)GUILD_HANG_EXP_INTERVAL);
	if (!pResBasic)
	{
		LOGERR("pResBasic is null");
		return false;
	}
	m_dwUpdateInterval = pResBasic->m_para[0];

	for (int i = 0; i < MAX_VIP_LEVEL+1; i++)
	{
		RESVIP* pResVip = CGameDataMgr::Instance().GetResVIPMgr().Find(i+1);
		if (!pResVip)
		{
			LOGERR("pResVip is null");
			return false;
		}

		m_wVipVacancy[i] = pResVip->m_bGuildHangExNum;

	}

	m_dwExpValue[0]=0;
	m_dwExpSpeed[0]=0;
	m_wLevelVacancy[0]=0;
	for (int i = 1; i < RES_MAX_GENERAL_LV_LEN+1; i++)
	{
		RESGUILDGENERALHANG* pResGuildHang = CGameDataMgr::Instance().GetResGuildGeneralHangMgr().Find(i);
		if (!pResGuildHang)
		{
			LOGERR("pResGuildHang is null");
			return false;
		}
		m_dwExpValue[i] = pResGuildHang->m_dwExp;
		m_dwExpSpeed[i] = pResGuildHang->m_dwSpeedExp;
		m_wLevelVacancy[i] = pResGuildHang->m_bLevelSlotNum;
	}

	//购买栏位价格
	pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find((int)GUILD_GENERAL_HANG);
	if (!pResBasic)
	{
		LOGERR("pResBasic is null");
		return false;
	}
	int iLevelPriceIndex = pResBasic->m_para[0];
	int iVipPriceIndex = pResBasic->m_para[1];
	//等级栏位
	RESCONSUME* pConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(iLevelPriceIndex);
	if (!pConsume)
	{
		LOGERR("pConsume is null. iLevelPriceIndex<%d>", iLevelPriceIndex);
		return false;
	}
	if (pConsume->m_dwLvCount > GUILD_GENERAL_HANG_LEVEL_NUM)
	{
		LOGERR("pConsume->m_dwLvCount is larger than GUILD_GENERAL_HANG_LEVEL_NUM. Count<%d>, iLevelPriceIndex<%d>", pConsume->m_dwLvCount, iLevelPriceIndex);
		return false;
	}
	for (int i=0; i<(int)pConsume->m_dwLvCount; i++)
	{
		m_iLevelPrice[i] = pConsume->m_lvList[i];
	}
	//vip栏位
	pConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(iVipPriceIndex);
	if (!pConsume)
	{
		LOGERR("pConsume is null. iVipPriceIndex<%d>", iVipPriceIndex);
		return false;
	}
	if (pConsume->m_dwLvCount > GUILD_GENERAL_HANG_VIP_NUM)
	{
		LOGERR("pConsume->m_dwLvCount is larger than GUILD_GENERAL_HANG_VIP_NUM. Count<%d>, iLevelPriceIndex<%d>", pConsume->m_dwLvCount, iVipPriceIndex);
		return false;
	}
	for (int i = 0; i < (int)pConsume->m_dwLvCount; i++)
	{
		m_iVipPrice[i] = pConsume->m_lvList[i];
	}

	return true;
}

bool GuildGeneralHang::InitPlayerData(PlayerData* pstData)
{
	//_SettleExpAllSlot(pstData);

	return true;
}

int GuildGeneralHang::UnlockLevelSlot(PlayerData* pstData)
{
	DT_GUILD_HANG_INFO& rstHangInfo = pstData->GetGuildInfo().m_stGuildHangInfo;
	uint16_t wLevel = pstData->GetMajestyInfo().m_wLevel;

	uint16_t wCount = m_wLevelVacancy[wLevel];
    uint8_t bIsChgStateFlag = false;
	int iRet = ERR_NONE;
	do 
	{
		if (wCount > rstHangInfo.m_bLevelSlotCount)
		{
			iRet = ERR_SYS;
			break;
		}

		for (int i = 0; i < wCount; i++)
		{
			if (rstHangInfo.m_astLevelSlotList[i].m_bState == GUILD_HANG_STATE_LOCKED)
			{
				rstHangInfo.m_astLevelSlotList[i].m_bState = GUILD_HANG_STATE_SEMI_LOCK;
                bIsChgStateFlag = true;
			}
		}
	} while (false);

    if (bIsChgStateFlag)
    {
        _HangInfoNtf(pstData);
    }

	return iRet;
}

int GuildGeneralHang::BuyLevelSlot(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint8_t bIndex)
{
	DT_GUILD_HANG_INFO& rstHangInfo = pstData->GetGuildInfo().m_stGuildHangInfo;
	
	int iRet = ERR_NONE;
	do 
	{
		if (bIndex >= rstHangInfo.m_bLevelSlotCount)
		{
			LOGERR("bIndex<%d> is large than rstHangInfo.m_bLevelSlotCount<%d>", bIndex, rstHangInfo.m_bLevelSlotCount);
			iRet = ERR_WRONG_PARA;
		}

		if (rstHangInfo.m_astLevelSlotList[bIndex].m_bState != GUILD_HANG_STATE_SEMI_LOCK)
		{
			LOGERR("the slot %d is not semilock. state<%d>", bIndex, rstHangInfo.m_astLevelSlotList[bIndex].m_bState);
			iRet = ERR_SYS;
			break;
		}

		int iDiamondCost = m_iLevelPrice[bIndex];
		if (!Item::Instance().IsEnough(pstData, ITEM_TYPE_DIAMOND, 0, iDiamondCost))
		{
			iRet = ERR_NOT_ENOUGH_DIAMOND;
			break;
		}

		Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -iDiamondCost, rstSyncItemInfo, DWLOG::METHOD_PURCHASE_HANG_LEVEL_SLOT);
		rstHangInfo.m_astLevelSlotList[bIndex].m_bState = GUILD_HANG_STATE_UNLOCKED;

	} while (false);

	return iRet;
}

int GuildGeneralHang::UnlockVipSlot(PlayerData* pstData)
{
	DT_GUILD_HANG_INFO& rstHangInfo = pstData->GetGuildInfo().m_stGuildHangInfo;
	uint16_t wLevel = pstData->GetMajestyInfo().m_bVipLv;

	uint16_t wCount = m_wVipVacancy[wLevel];
	int iRet = ERR_NONE;
    uint8_t bIsChgStateFlag = false;
	do 
	{
		if (wCount > rstHangInfo.m_bVipSlotCount)
		{
			iRet = ERR_SYS;
			break;
		}

		for (int i = 0; i < wCount; i++)
		{
			if (rstHangInfo.m_astVipSlotList[i].m_bState == GUILD_HANG_STATE_LOCKED)
			{
				rstHangInfo.m_astVipSlotList[i].m_bState = GUILD_HANG_STATE_SEMI_LOCK;
                bIsChgStateFlag = true;
			}
		}
	} while (false);

    if (bIsChgStateFlag)
    {
        _HangInfoNtf(pstData);
    }

	return iRet;
}

int GuildGeneralHang::BuyVipSlot(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint8_t bIndex)
{
	DT_GUILD_HANG_INFO& rstHangInfo = pstData->GetGuildInfo().m_stGuildHangInfo;

	int iRet = ERR_NONE;
	do 
	{
		if (bIndex >= rstHangInfo.m_bVipSlotCount)
		{
			LOGERR("bIndex<%d> is large than rstHangInfo.m_bVipSlotCount<%d>", bIndex, rstHangInfo.m_bVipSlotCount);
			iRet = ERR_WRONG_PARA;
		}

		if (rstHangInfo.m_astVipSlotList[bIndex].m_bState != GUILD_HANG_STATE_SEMI_LOCK)
		{
			LOGERR("the slot %d is not semilock. state<%d>", bIndex, rstHangInfo.m_astVipSlotList[bIndex].m_bState);
			iRet = ERR_SYS;
			break;
		}

		int iDiamondCost = m_iVipPrice[bIndex];
		if (!Item::Instance().IsEnough(pstData, ITEM_TYPE_DIAMOND, 0, iDiamondCost))
		{
			iRet = ERR_NOT_ENOUGH_DIAMOND;
			break;
		}

		Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -iDiamondCost, rstSyncItemInfo, DWLOG::METHOD_PURCHASE_HANG_VIP_SLOT);
		rstHangInfo.m_astVipSlotList[bIndex].m_bState = GUILD_HANG_STATE_UNLOCKED;

	} while (false);

	return iRet;
}

int GuildGeneralHang::LayGCard(PlayerData* pstData, SC_PKG_GUILD_HANG_LAY_GENERAL_RSP& rstRsp, CS_PKG_GUILD_HANG_LAY_GENERAL_REQ& rstReq)
{
	uint8_t bType = rstReq.m_bType;
	uint8_t bIndex = rstReq.m_bIndex;
	uint32_t dwGeneralId = rstReq.m_dwGeneralID;

	DT_GUILD_HANG_INFO& rstHangInfo = pstData->GetGuildInfo().m_stGuildHangInfo;
	DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();

	int iRet = ERR_NONE;
	do 
	{
		if (bType>2)
		{
			LOGERR("bType is larger than 2.");
			iRet = ERR_WRONG_PARA;
			break;
		}

		if (bType==1 && bIndex >= rstHangInfo.m_bLevelSlotCount)
		{
			LOGERR("bIndex<%d> is large than rstHangInfo.m_bLevelSlotCount<%d>", bIndex, rstHangInfo.m_bLevelSlotCount);
			iRet = ERR_WRONG_PARA;
			break;
		}

		if (bType==2 && bIndex >= rstHangInfo.m_bVipSlotCount)
		{
			LOGERR("bIndex<%d> is large than rstHangInfo.m_bVipSlotCount<%d>", bIndex, rstHangInfo.m_bVipSlotCount);
			iRet = ERR_WRONG_PARA;
			break;
		}

		DT_ONE_GUILD_HANG_INFO* pstOneHangInfo = NULL;
		switch (bType)
		{
		case 0:
			pstOneHangInfo = &rstHangInfo.m_stDefaultSlot;
			break;
		case 1:
			pstOneHangInfo = &rstHangInfo.m_astLevelSlotList[bIndex];
			break;
		case 2:
			pstOneHangInfo = &rstHangInfo.m_astVipSlotList[bIndex];
			break;
		default:
			break;
		}

		if (pstOneHangInfo == NULL)
		{
			LOGERR("pstOneHangInfo is null.");
			iRet = ERR_SYS;
			break;
		}

		if (pstOneHangInfo->m_bState != GUILD_HANG_STATE_UNLOCKED)
		{
			LOGERR("the hang is not unlocked. bType<%d> bIndex<%d>", bType, bIndex);
			iRet = ERR_WRONG_PARA;
			break;
		}

		DT_ITEM_GCARD* pstGCard = GeneralCard::Instance().Find(pstData, dwGeneralId);
		if (!pstGCard)
		{
			LOGERR("pstGCard is null. dwGeneralId<%d>", dwGeneralId);
			iRet = ERR_SYS;
			break;
		}

		if (pstGCard->m_bLevel >= rstMajestyInfo.m_wLevel)
		{
			LOGRUN("pstGCard is max level.gcard<%d> role<%d>", pstGCard->m_bLevel, rstMajestyInfo.m_wLevel);
			iRet = ERR_ALREADY_MAX_LEVEL;
			break;
		}

		if (pstOneHangInfo->m_dwGeneralId == 0)
		{
			pstOneHangInfo->m_ullLastSettleTimestamp = CGameTime::Instance().GetCurrSecond();
		}
	
		rstRsp.m_dwExpGot = _SettleExp(pstData, *pstOneHangInfo);

		pstOneHangInfo->m_dwGeneralId = dwGeneralId;
		pstOneHangInfo->m_ullStartTimestamp = CGameTime::Instance().GetCurrSecond();
		pstOneHangInfo->m_bStartLevel = pstGCard->m_bLevel;
		pstOneHangInfo->m_dwExpGot = 0;
		pstOneHangInfo->m_dwSettleExp = 0;

		rstRsp.m_bIndex = rstReq.m_bIndex;
		rstRsp.m_bType = rstReq.m_bType;
		memcpy(&rstRsp.m_stOneGuildHangInfo, pstOneHangInfo, sizeof(DT_ONE_GUILD_HANG_INFO));

	} while (false);

	return iRet;
}


uint32_t GuildGeneralHang::_SettleExp(PlayerData* pstData, DT_ONE_GUILD_HANG_INFO& rstOneHangInfo)
{
	uint32_t iRet = 0;
	
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();

	do 
	{
		if (rstOneHangInfo.m_bState != GUILD_HANG_STATE_UNLOCKED)
		{
			break;
		}

		if (rstOneHangInfo.m_dwGeneralId == 0)
		{
			break;
		}

		DT_ITEM_GCARD* pstGCard = GeneralCard::Instance().Find(pstData, rstOneHangInfo.m_dwGeneralId);
		if (pstGCard == NULL)
		{
			LOGRUN("pstGCard is null. generalId<%d>", rstOneHangInfo.m_dwGeneralId);
			break;
		}
		
		uint64_t ullCurSec = (uint64_t)CGameTime::Instance().GetCurrSecond();
		uint64_t ullTimePass = ullCurSec - rstOneHangInfo.m_ullLastSettleTimestamp;
		uint32_t dwExp = ullTimePass/m_dwUpdateInterval * m_dwExpValue[rstMajestyInfo.m_wLevel] + rstOneHangInfo.m_dwSettleExp;
        rstOneHangInfo.m_dwSettleExp = 0;

        LOGRUN("ullTimePass<%lu>, m_dwUpdateInterval<%d> expPerTime<%d>", ullTimePass, m_dwUpdateInterval, m_dwExpValue[rstMajestyInfo.m_wLevel]);

		GeneralCard::Instance().LvUp(pstData, rstOneHangInfo.m_dwGeneralId, dwExp);
		
		rstOneHangInfo.m_dwExpGot += dwExp;
		rstOneHangInfo.m_ullLastSettleTimestamp = ullCurSec - ullTimePass % m_dwUpdateInterval;

		iRet = dwExp;

	} while (false);

	return iRet;
}

int GuildGeneralHang::SettleExpAllSlot(PlayerData* pstData, SC_PKG_GUILD_HANG_SETTLE_RSP& rstRsp)
{


	_SettleExpAllSlot(pstData, &rstRsp);

	return ERR_NONE;
}

void GuildGeneralHang::_SettleExpAllSlot(PlayerData* pstData, SC_PKG_GUILD_HANG_SETTLE_RSP* poRsp)
{
	DT_GUILD_HANG_INFO& rstHangInfo = pstData->GetGuildInfo().m_stGuildHangInfo;

    if (poRsp == NULL)
    {
        LOGERR("poRsp is null.");
        return;
    }
    
    poRsp->m_bGeneralCount = 0;

	if (rstHangInfo.m_stDefaultSlot.m_bState == GUILD_HANG_STATE_UNLOCKED && rstHangInfo.m_stDefaultSlot.m_dwGeneralId != 0)
	{
        uint64_t ullTmpTimestamp = rstHangInfo.m_stDefaultSlot.m_ullLastSettleTimestamp;
		uint32_t dwExp = _SettleExp(pstData, rstHangInfo.m_stDefaultSlot);
        if (dwExp != 0)
        {
            poRsp->m_GeneralList[poRsp->m_bGeneralCount] = rstHangInfo.m_stDefaultSlot.m_dwGeneralId;
            poRsp->m_ExpChaValueList[poRsp->m_bGeneralCount++] = dwExp;
            poRsp->m_ullEarliestTimestamp = ullTmpTimestamp;
        }
	}

	for (int i = 0; i < rstHangInfo.m_bLevelSlotCount; i++)
	{
		if (rstHangInfo.m_astLevelSlotList[i].m_bState == GUILD_HANG_STATE_UNLOCKED && rstHangInfo.m_astLevelSlotList[i].m_dwGeneralId != 0)
		{
            uint64_t ullTmpTimestamp = rstHangInfo.m_astLevelSlotList[i].m_ullLastSettleTimestamp;
            uint32_t dwExp = _SettleExp(pstData, rstHangInfo.m_astLevelSlotList[i]);
            if (dwExp != 0)
            {
                poRsp->m_GeneralList[poRsp->m_bGeneralCount] = rstHangInfo.m_astLevelSlotList[i].m_dwGeneralId;
                poRsp->m_ExpChaValueList[poRsp->m_bGeneralCount++] = dwExp;
            }
            if (ullTmpTimestamp < poRsp->m_ullEarliestTimestamp)
            {
                poRsp->m_ullEarliestTimestamp = ullTmpTimestamp;
            }
		}
	}
	for (int i = 0; i < rstHangInfo.m_bVipSlotCount; i++)
	{
		if (rstHangInfo.m_astVipSlotList[i].m_bState == GUILD_HANG_STATE_UNLOCKED && rstHangInfo.m_astVipSlotList[i].m_dwGeneralId != 0)
		{
            uint64_t ullTmpTimestamp = rstHangInfo.m_astVipSlotList[i].m_ullLastSettleTimestamp;
            uint32_t dwExp = _SettleExp(pstData, rstHangInfo.m_astVipSlotList[i]);
            if (dwExp != 0)
            {
                poRsp->m_GeneralList[poRsp->m_bGeneralCount] = rstHangInfo.m_astVipSlotList[i].m_dwGeneralId;
                poRsp->m_ExpChaValueList[poRsp->m_bGeneralCount++] = dwExp;
            }
            if (ullTmpTimestamp < poRsp->m_ullEarliestTimestamp)
            {
                poRsp->m_ullEarliestTimestamp = ullTmpTimestamp;
            }
		}
	}
}

int GuildGeneralHang::AddSpeedExp(PlayerData* pstData, uint8_t bCount)
{	
	DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
	uint32_t dwExpAdd = m_dwExpSpeed[rstMajestyInfo.m_wLevel]*bCount;

	DT_GUILD_HANG_INFO& rstHangInfo = pstData->GetGuildInfo().m_stGuildHangInfo;

	if (rstHangInfo.m_stDefaultSlot.m_bState == GUILD_HANG_STATE_UNLOCKED && rstHangInfo.m_stDefaultSlot.m_dwGeneralId != 0)
	{
		rstHangInfo.m_stDefaultSlot.m_dwSettleExp += dwExpAdd;
	}

	for (int i = 0; i < rstHangInfo.m_bLevelSlotCount; i++)
	{
		if (rstHangInfo.m_astLevelSlotList[i].m_bState == GUILD_HANG_STATE_UNLOCKED && rstHangInfo.m_astLevelSlotList[i].m_dwGeneralId != 0)
		{
			rstHangInfo.m_astLevelSlotList[i].m_dwSettleExp += dwExpAdd;
		}
	}
	for (int i = 0; i < rstHangInfo.m_bVipSlotCount; i++)
	{
		if (rstHangInfo.m_astVipSlotList[i].m_bState == GUILD_HANG_STATE_UNLOCKED && rstHangInfo.m_astVipSlotList[i].m_dwGeneralId != 0)
		{
			rstHangInfo.m_astVipSlotList[i].m_dwSettleExp += dwExpAdd;
		}
	}

	return ERR_NONE;
}

void GuildGeneralHang::_HangInfoNtf(PlayerData* pstData)
{
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_HANG_INFO_NTF;
    SC_PKG_GUILD_HANG_INFO_NTF& rstNtf = m_stScPkg.m_stBody.m_stGuildHangInfoNtf;

    DT_GUILD_HANG_INFO& rstHangInfo = pstData->GetGuildInfo().m_stGuildHangInfo;

    memcpy(&rstNtf.m_stGuildHangInfo, &rstHangInfo, sizeof(DT_GUILD_HANG_INFO));

    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
}

