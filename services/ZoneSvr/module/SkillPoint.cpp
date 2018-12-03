#include "SkillPoint.h"
#include "LogMacros.h"
#include "dwlog_def.h"
#include "../gamedata/GameDataMgr.h"
#include "GameTime.h"
#include "Item.h"

using namespace PKGMETA;

bool SkillPoint::Init()
{
    RESBASIC* pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find((int)SKILL_POINT_CYCLE);
    if (!pResBasic)
    {
        LOGERR("pResBasic is null");
        return false;
    }
    uint32_t dwBaseUpdateInterval = (uint32_t)(pResBasic->m_para[0]);

    for (int i = 0; i < MAX_VIP_LEVEL+1; i++)
    {
        RESVIP* pResVip = CGameDataMgr::Instance().GetResVIPMgr().Find(i+1);
        if (!pResVip)
        {
            LOGERR("pResVip is null");
            return false;
        }

        m_dwUpdateInterval[i] = dwBaseUpdateInterval - pResVip->m_wSPTimeReduce;

		m_wTopSPLimit[i] = pResVip->m_wSPNumTopLimit;

		m_wBuySPLTimes[i] = pResVip->m_wSPNumBuyLimit;
    }

	pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find((int)DIAMOND_BUY_SP);
	if (!pResBasic)
	{
		LOGERR("pResBasic is null");
		return false;
	}
	m_iSPGetIndex = pResBasic->m_para[0];
	m_iDiamondCostIndex = pResBasic->m_para[1];

    return true;
}

bool SkillPoint::InitPlayerData(PlayerData* pstData)
{
    DT_ROLE_MAJESTY_INFO& rstInfo = pstData->GetMajestyInfo();

    // 计算离线期间体力的增长情况
    uint64_t ullDiff = (uint64_t)CGameTime::Instance().GetCurrSecond() - rstInfo.m_ullSPResumeLastTime;
    int32_t iSPIncValue = ullDiff/m_dwUpdateInterval[rstInfo.m_bVipLv];
    this->Add(pstData, iSPIncValue);

    rstInfo.m_ullSPResumeLastTime = (uint64_t)CGameTime::Instance().GetCurrSecond() - ullDiff % m_dwUpdateInterval[rstInfo.m_bVipLv];

    return true;

}

void SkillPoint::UpdatePlayerData(PlayerData* pstData)
{
    DT_ROLE_MAJESTY_INFO& rstInfo = pstData->GetMajestyInfo();

    uint64_t ullDiff = (uint64_t)CGameTime::Instance().GetCurrSecond() - rstInfo.m_ullSPResumeLastTime;
    if (ullDiff >= m_dwUpdateInterval[rstInfo.m_bVipLv])
    {
        this->Add(pstData, 1);
        rstInfo.m_ullSPResumeLastTime = (uint64_t)CGameTime::Instance().GetCurrSecond();
    }
}

uint32_t SkillPoint::Add(PlayerData* pstData, int32_t iValue)
{
    DT_ROLE_MAJESTY_INFO& rstInfo = pstData->GetMajestyInfo();

	if (iValue > 0)
	{
        //增加
        rstInfo.m_wSkillPoint += iValue;

		uint16_t wMaxSP = m_wTopSPLimit[rstInfo.m_bVipLv];

        if (rstInfo.m_wSkillPoint > wMaxSP)
        {
            rstInfo.m_wSkillPoint = wMaxSP;
        }
	}
	else
	{
		// 消耗
		if (rstInfo.m_wSkillPoint > -iValue)
		{
			rstInfo.m_wSkillPoint += iValue;
		}
		else
		{
			rstInfo.m_wSkillPoint = 0;
		}
	}

    return rstInfo.m_wSkillPoint;
}

bool SkillPoint::IsEnough(PlayerData* pstData, uint16_t wValue)
{
	if (pstData->GetMajestyInfo().m_wSkillPoint >= wValue)
	{
		return true;
	}

	return false;
}

int SkillPoint::PurchaseSP(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
	DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();

	int iRet = ERR_NONE;
	
	do 
	{
		uint16_t wMaxPurchaseTime = m_wBuySPLTimes[rstMajestyInfo.m_bVipLv];
		if (wMaxPurchaseTime <= rstMajestyInfo.m_bSPPurchaseTimes)
		{
			iRet = ERR_SP_PURCHASE_LIMIT;
			break;
		}

		RESCONSUME* pConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(m_iSPGetIndex);
		if (!pConsume)
		{
			LOGERR("pConsume is null. m_iSPGetIndex<%d>", m_iSPGetIndex);
			iRet = ERR_SYS;
			break;
		}
		int iSPGet = pConsume->m_lvList[rstMajestyInfo.m_bSPPurchaseTimes];

		pConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(m_iDiamondCostIndex);
		if (!pConsume)
		{
			LOGERR("pConsume is null. m_iDiamondCostIndex<%d>", m_iDiamondCostIndex);
			iRet = ERR_SYS;
			break;
		}
		int iDiamondCost = pConsume->m_lvList[rstMajestyInfo.m_bSPPurchaseTimes];
		
		Item::Instance().ConsumeItem(pstData, ITEM_TYPE_SKILL_POINT, 0, iSPGet, rstSyncItemInfo, DWLOG::METHOD_PURCHASE_SP);
		Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -iDiamondCost, rstSyncItemInfo, DWLOG::METHOD_PURCHASE_SP);

		rstMajestyInfo.m_bSPPurchaseTimes++;

	} while (false);
	

	return iRet;
}


