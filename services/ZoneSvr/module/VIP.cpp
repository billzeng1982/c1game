#include "VIP.h"
#include "GameTime.h"
#include "LogMacros.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "Item.h"
#include "dwlog_svr.h"
#include "ZoneLog.h"
#include "Sign.h"
#include "Task.h"
#include "GuildGeneralHang.h"
#include "Message.h"
using namespace PKGMETA;
using namespace DWLOG;
VIP::VIP()
{
}

VIP::~VIP()
{
}


bool VIP::Init()
{
    LOGRUN("Init Vip module");

    ResBasicMgr_t& rResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
    RESBASIC* poBasic = rResBasicMgr.Find(COMMON_UPDATE_TIME);
    if (NULL == poBasic)
    {
        LOGERR("poBasic is null");
        return false;
    }

    m_iUptTime = (int)poBasic->m_para[0];
    m_bUptFlag = false;

    m_ullUptTimeStamp = CGameTime::Instance().GetSecOfHourInCurrDay(m_iUptTime) * 1000;
    if (CGameTime::Instance().GetCurrHour() < m_iUptTime)
    {
        m_ullUptTimeStamp -= MSSECONDS_OF_DAY;
    }

    ResVIPMgr_t& rstResVipMgr = CGameDataMgr::Instance().GetResVIPMgr();
    ResGifPkgMgr_t& rstResGifPkgMgr = CGameDataMgr::Instance().GetResGifPkgMgr();
    for (int i=0; i<MAX_VIP_LEVEL; i++)
    {
        RESVIP* pResVip = rstResVipMgr.GetResByPos(i+1);
        assert(pResVip);

        m_VipExpToLevelList[i] = pResVip->m_dwMoney;
        m_ResVIPList[i] = pResVip;

        //RESGIFTPACKAGE* pResDailyGif = rstResGifPkgMgr.Find(pResVip->m_dwDailyGift);
        RESGIFTPACKAGE* pResLevelGif = rstResGifPkgMgr.Find(pResVip->m_dwLevelGift);
        //assert(pResDailyGif);
        assert(pResLevelGif);

        //m_ResDailyGifList[i] = pResDailyGif;
        m_ResLevelGifList[i] = pResLevelGif;
    }

    return true;
}


uint32_t VIP::AddExp(PlayerData* pstData, uint32_t dwValue, uint32_t dwApproach)
{
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
    if (rstMajestyInfo.m_bVipLv >= MAX_VIP_LEVEL)
    {
        return rstMajestyInfo.m_dwVipExp;
    }

    rstMajestyInfo.m_dwVipExp += dwValue;
    uint8_t bOldLevel = rstMajestyInfo.m_bVipLv;
    bool bIsUpLv = false;
    while(rstMajestyInfo.m_bVipLv < MAX_VIP_LEVEL &&
            rstMajestyInfo.m_dwVipExp >= m_VipExpToLevelList[rstMajestyInfo.m_bVipLv])
    {
        rstMajestyInfo.m_bVipLv++;
        bIsUpLv = true;
    }

    if (rstMajestyInfo.m_bVipLv > bOldLevel)
    {
        RESVIP* pResVIP = m_ResVIPList[rstMajestyInfo.m_bVipLv -1];
        rstMajestyInfo.m_bGoldPurchaseTimesMax = pResVIP->m_dwBuyGoldenTimes;
        rstMajestyInfo.m_bAPPurchaseTimesMax = pResVIP->m_dwBuyAPTimes;
    }

    //SendLvUpNtf(pstData);
    if (bIsUpLv)
    {
        //Vip升级后通知其他功能处理
        Sign::Instance().Compensate30dAward(pstData);
		Task::Instance().TaskCondTrigger(pstData, TASK_COND_TYPE_VIP_LV);
		GuildGeneralHang::Instance().UnlockVipSlot(pstData);
    }
    LOGRUN("Uin<%lu> add VipExp<%u> CurVipExp<%u> OldVipLv<%hhu>,CurVipLv<%hhu>, approach<%u>",
        pstData->m_ullUin, dwValue, rstMajestyInfo.m_dwVipExp, bOldLevel, rstMajestyInfo.m_bVipLv, dwApproach);

    return rstMajestyInfo.m_dwVipExp;
}


int VIP::RecvDailyGif(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstRewardItemInfo, uint64_t* pTimestamp)
{
#if 0
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();

    if (rstMajestyInfo.m_ullRecvGifTimeStamp >= m_ullUptTimeStamp)
    {
        return ERR_AWARD_FINISHED;
    }

    if (rstMajestyInfo.m_bVipLv == 0)
    {
        return ERR_NONE;
    }

    RESGIFTPACKAGE* pResDailyGif = m_ResDailyGifList[rstMajestyInfo.m_bVipLv -1];
    for (int i=0; i<pResDailyGif->m_bPropsCount; i++)
    {
        Item::Instance().RewardItem(pstData, pResDailyGif->m_propType[i], pResDailyGif->m_propId[i], 
            pResDailyGif->m_propNum[i], rstRewardItemInfo, METHOD_VIP_LV_GIF);
    }
    rstMajestyInfo.m_ullRecvGifTimeStamp = m_ullUptTimeStamp;

    return ERR_NONE;
#endif
	return ERR_NONE;
}


int VIP::RecvLevelGif(PlayerData* pstData, uint8_t bLevel, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
    if (rstMajestyInfo.m_bVipLv == 0)
    {
        return ERR_NONE;
    }

    if (rstMajestyInfo.m_bVipLv < bLevel)
    {
        return ERR_DEFAULT;
    }

    if (rstMajestyInfo.m_szVipGif[bLevel -1] > 0)
    {
        return ERR_AWARD_FINISHED;
    }

	ResGifPkgMgr_t& rResGifPkgMgr = CGameDataMgr::Instance().GetResGifPkgMgr();
	RESGIFTPACKAGE* pstGiftPackage = rResGifPkgMgr.Find(bLevel);
	if (!pstGiftPackage)
	{
		LOGERR("pstGiftPackage is null.");
		return ERR_SYS;
	}
	
	int iNumDia = pstGiftPackage->m_wCost;
	if (!Item::Instance().IsEnough(pstData, ITEM_TYPE_DIAMOND, 0, iNumDia))
	{
		return ERR_NOT_ENOUGH_DIAMOND;
	}
	Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -iNumDia, rstSyncItemInfo, METHOD_VIP_LV_GIF);

    RESGIFTPACKAGE* pResLevelGif = m_ResLevelGifList[bLevel-1];
    for (int i=0; i<pResLevelGif->m_bPropsCount; i++)
    {
        Item::Instance().RewardItem(pstData, pResLevelGif->m_propType[i], pResLevelGif->m_propId[i],
            pResLevelGif->m_propNum[i], rstSyncItemInfo, METHOD_VIP_LV_GIF);
    }
    rstMajestyInfo.m_szVipGif[bLevel -1] = 1;

    switch (bLevel)
    {
    case 2:
    {
        Message::Instance().AutoSendSysMessage(2001, "Name=%s", pstData->GetRoleName());
        Message::Instance().AutoSendWorldMessage(pstData, 2008);
        break;
    }
    case 4:
        Message::Instance().AutoSendSysMessage(2002, "Name=%s", pstData->GetRoleName());
        Message::Instance().AutoSendWorldMessage(pstData, 2009);
        break;
    case 9:
        Message::Instance().AutoSendSysMessage(2003, "Name=%s", pstData->GetRoleName());
        Message::Instance().AutoSendWorldMessage(pstData, 2010);
        break;
    case 14:
        Message::Instance().AutoSendSysMessage(2004, "Name=%s", pstData->GetRoleName());
        Message::Instance().AutoSendWorldMessage(pstData, 2011);
        break;
    case 15:
        Message::Instance().AutoSendSysMessage(2005, "Name=%s", pstData->GetRoleName());
        break;
    case 17:
        Message::Instance().AutoSendSysMessage(2006, "Name=%s", pstData->GetRoleName());
        break;
    case 18:
        Message::Instance().AutoSendSysMessage(2006, "Name=%s", pstData->GetRoleName());
        break;

    default:
        break;
    }


    return ERR_NONE;
}


int VIP::UpdateServer()
{
    int iHour = CGameTime::Instance().GetCurrHour();
    //每日礼包更新
    if ((iHour == m_iUptTime) && m_bUptFlag)
    {
        m_bUptFlag = false;
        m_ullUptTimeStamp = CGameTime::Instance().GetCurrTimeMs();
    }
    else if (iHour != m_iUptTime)
    {
        m_bUptFlag = true;
    }
    return 1;
}


void VIP::SendLvUpNtf(PlayerData* pstData)
{
    SCPKG stScPkg;
    stScPkg.m_stHead.m_wMsgId = SC_MSG_VIP_LEVEL_UP_NTF;
    SC_PKG_VIP_LEVEL_UP_NTF& rstScNtf = stScPkg.m_stBody.m_stVipLvUpNtf;

    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
    rstScNtf.m_bLevel = rstMajestyInfo.m_bVipLv;
    rstScNtf.m_dwExp = rstMajestyInfo.m_dwVipExp;

    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &stScPkg);
}

