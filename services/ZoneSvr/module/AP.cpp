#include "AP.h"
#include "LogMacros.h"
#include "common_proto.h"
#include "sys/GameTime.h"
#include "../gamedata/GameDataMgr.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "ov_res_keywords.h"
#include "Consume.h"
#include "Item.h"
#include "dwlog_def.h"
#include "Task.h"
#include "Majesty.h"
#include "GloryItemsMgr.h"

using namespace PKGMETA;
using namespace DWLOG;

AP::AP()
{

}

AP::~AP()
{

}

bool AP::Init()
{
    m_ullUpdateInterval = CGameDataMgr::Instance().GetResBasicMgr().Find((int)BASIC_AP_INTERVAL)->m_para[0];

    LOGRUN("init ap info");
    LOGRUN("---------------------------------------------------");
    LOGRUN("m_ullUpdateInterval = %lu", m_ullUpdateInterval);
    LOGRUN("---------------------------------------------------");

    return true;
}

// 创建账号，初始化时调用
bool AP::CreatePlayerData(PlayerData* pstData)
{
    DT_ROLE_MAJESTY_INFO& rstInfo = pstData->GetMajestyInfo();
    rstInfo.m_ullAPResumeLastTime = (uint64_t)CGameTime::Instance().GetCurrSecond();
    return true;
}

// 非初始化登陆调用
bool AP::InitPlayerData(PlayerData* pstData)
{
    DT_ROLE_MAJESTY_INFO& rstInfo = pstData->GetMajestyInfo();

    // 计算离线期间体力的增长情况
    uint64_t ullDiff = (uint64_t)CGameTime::Instance().GetCurrSecond() - rstInfo.m_ullAPResumeLastTime;
    uint64_t ullAPIncOffline = ullDiff/m_ullUpdateInterval;
    Add(pstData, ullAPIncOffline);
    rstInfo.m_ullAPResumeLastTime = (uint64_t)CGameTime::Instance().GetCurrSecond() - ullDiff%m_ullUpdateInterval;

    return true;
}

// 在线更新购买次数限制，更新回复时间
void AP::UpdatePlayerData(PlayerData* pstData)
{
    DT_ROLE_MAJESTY_INFO& rstInfo = pstData->GetMajestyInfo();

    uint64_t ullDiff = (uint64_t)CGameTime::Instance().GetCurrSecond() - rstInfo.m_ullAPResumeLastTime;
    if (ullDiff >= m_ullUpdateInterval)
    {
        Add(pstData, 1);
        rstInfo.m_ullAPResumeLastTime = (uint64_t)CGameTime::Instance().GetCurrSecond();
    }
}

int AP::PurchaseAp(PlayerData* pstData, uint32_t dwNum, SC_PKG_PURCHASE_RSP& rstScPkgBodyRsp)
{
	if (dwNum <= 0)
	{
		LOGERR("Uin<%lu> PurchaseAp dwNum is 0",pstData->m_ullUin);
		return ERR_SYS;
	}

	DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
	if (rstMajestyInfo.m_bAPPurchaseTimes + dwNum > rstMajestyInfo.m_bAPPurchaseTimesMax)
	{
		return ERR_AP_PURCHASE_LIMIT;
	}

	RESPURCHASE* pstResPurchase = CGameDataMgr::Instance().GetResPurchaseMgr().Find(PURCHASE_ID_AP);
	if (pstResPurchase == NULL)
	{
		LOGERR("Uin<%lu> pstResPurchase is NULL", pstData->m_ullUin);
		return ERR_SYS;
	}

	uint32_t dwConsume = 0;
	uint32_t dwGain = 0;
	for (uint32_t i=0; i<dwNum; i++)
	{
		dwConsume += pstResPurchase->m_consumeNumber[rstMajestyInfo.m_bAPPurchaseTimes + i];
		dwGain += pstResPurchase->m_gainNumber[rstMajestyInfo.m_bAPPurchaseTimes + i];
	}

	if (!Consume::Instance().IsEnoughDiamond(pstData, dwConsume))
	{
		return ERR_NOT_ENOUGH_DIAMOND;
	}

	rstMajestyInfo.m_bAPPurchaseTimes += dwNum;

	rstScPkgBodyRsp.m_bPurchaseTimes = rstMajestyInfo.m_bAPPurchaseTimes;

	Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -dwConsume, rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_AP_BUY_AP);
	Item::Instance().RewardItem(pstData, ITEM_TYPE_AP, 0, dwGain, rstScPkgBodyRsp.m_stSyncItemInfo,    METHOD_AP_BUY_AP);

	//任务修改
	Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_BUY, 1, 1/*para1*/);

	return ERR_NONE;
}

int AP::HandleMajestLvUp(PlayerData* pstData)
{
    DT_ROLE_MAJESTY_INFO& rstInfo = pstData->GetMajestyInfo();
    ResAPMgr_t& rstResMgr = CGameDataMgr::Instance().GetResAPMgr();
    RESAP* pResAP = rstResMgr.Find((uint32_t)rstInfo.m_wLevel - 1); // 这里是升级前的等级
    if (!pResAP)
    {
        LOGERR("Uin<%lu> pResAP is null", pstData->m_ullUin);
        return ERR_NOT_FOUND;
    }

    RESAP* pResAPNext = rstResMgr.Find((uint32_t)rstInfo.m_wLevel); // Majest升级时已经判断了最高等级上限，这里不会超过上限
    if (!pResAPNext)
    {
        LOGERR("Uin<%lu> pResAPNext is null", pstData->m_ullUin);
        return ERR_NOT_FOUND;
    }

    // 当前经验在当前等级上限和下一等级上限之间，更新刷新时间，开始更新恢复
    if ((rstInfo.m_dwAP >= pResAP->m_wAPLimit) && (rstInfo.m_dwAP < pResAPNext->m_wAPLimit))
    {
        rstInfo.m_ullAPResumeLastTime = (uint64_t)CGameTime::Instance().GetCurrSecond();
    }

    SendSynMsg(pstData);

    return ERR_NONE;
}

void AP::SendSynMsg(PlayerData* pstData)
{
    DT_ROLE_MAJESTY_INFO& rstInfo = pstData->GetMajestyInfo();

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_AP_INC_SYN;
    m_stScPkg.m_stBody.m_stApIncSyn.m_nErrNo = ERR_NONE;
    m_stScPkg.m_stBody.m_stApIncSyn.m_dwCurValue = GetAPCurValue(pstData);
	m_stScPkg.m_stBody.m_stApIncSyn.m_ullCurrTimeSec = CGameTime::Instance().GetCurrSecond();
    m_stScPkg.m_stBody.m_stApIncSyn.m_ullLastResumeTimeSec = rstInfo.m_ullAPResumeLastTime;

    LOGRUN("Uin<%lu> sync ap cur-%u", pstData->m_ullUin, m_stScPkg.m_stBody.m_stApIncSyn.m_dwCurValue);
    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
}

bool AP::IsEnough(PlayerData* pstData, uint32_t dwValue)
{
	if (pstData->GetMajestyInfo().m_dwAP >= dwValue)
	{
		return true;
	}

	return false;
}

uint32_t AP::GetAPCurValue(PlayerData* pstData)
{
	return pstData->GetAP();
}

uint32_t AP::Add(PlayerData* pstData, int32_t iValue)
{
    DT_ROLE_MAJESTY_INFO& rstInfo = pstData->GetMajestyInfo();

	if (iValue > 0)
	{
		// 体力增加
		// 获取当前上限
		ResAPMgr_t& rstResMgr = CGameDataMgr::Instance().GetResAPMgr();
		RESAP* pResAP = rstResMgr.Find((uint32_t)rstInfo.m_wLevel);
		if (!pResAP)
		{
			LOGERR("Uin<%lu> pResAP is null", pstData->m_ullUin);
			return rstInfo.m_dwAP;
		}

		// 已大于上限不修改
		if (rstInfo.m_dwAP >= (uint32_t)pResAP->m_wAPLimit)
		{
			return rstInfo.m_dwAP;
		}

		// 未达到上限，可以达到上限
		if ((rstInfo.m_dwAP + iValue) > (uint32_t)pResAP->m_wAPLimit)
		{
			rstInfo.m_dwAP = (uint32_t)pResAP->m_wAPLimit;
		}
		else
		{
			rstInfo.m_dwAP += iValue;
		}
	}
	else
	{
        uint16_t wAPConsume;
		// 体力消耗
		if ((long)rstInfo.m_dwAP > -iValue)
		{
			rstInfo.m_dwAP += iValue;
            wAPConsume = -iValue;
		}
		else
		{
            wAPConsume = rstInfo.m_dwAP;
			rstInfo.m_dwAP = 0;
		}
        Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_RESOURCE_CONSUME, wAPConsume, 3);
	}

    return rstInfo.m_dwAP;
}

uint32_t AP::AddNoLimit(PlayerData* pstData, int32_t iValue)
{
	DT_ROLE_MAJESTY_INFO& rstInfo = pstData->GetMajestyInfo();
	DT_ROLE_MISC_INFO& rstMiscInfo = pstData->GetMiscInfo();
	if (iValue > 0)
	{
		// 体力增加
		rstInfo.m_dwAP += iValue;
	}
	else
	{
		// 体力消耗
		uint16_t wAPConsume = 0;
		if ((long)rstInfo.m_dwAP > -iValue)
		{
			wAPConsume = -iValue;
			rstInfo.m_dwAP += iValue;
			rstMiscInfo.m_dwConsumeAPCnt += -iValue;
		}
		else
		{
			wAPConsume = rstInfo.m_dwAP;
			rstMiscInfo.m_dwConsumeAPCnt += rstInfo.m_dwAP;
			rstInfo.m_dwAP = 0;
		}
		Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_RESOURCE_CONSUME, wAPConsume, 3);
		//主公称号掉落
		GloryItemsMgr::Instance().AddMajestyItems(pstData, MAJESTY_ITEM_ACCESS_AP, rstMiscInfo.m_dwConsumeAPCnt);
	}

    return rstInfo.m_dwAP;
}
