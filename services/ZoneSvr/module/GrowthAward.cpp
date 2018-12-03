#include "./GrowthAward.h"
#include "../gamedata/GameDataMgr.h"
#include "LogMacros.h"
#include "Item.h"
#include "ZoneLog.h"
#include "dwlog_svr.h"

using namespace PKGMETA;
using namespace DWLOG;



//等级成长奖励
int GrowthAward::GrowthAwardLevel(IN PlayerData* pstData,  IN PKGMETA::CS_PKG_GROWTH_AWARD_REQ& rstReq, OUT PKGMETA::SC_PKG_GROWTH_AWARD_RSP& rstRsp)
{
    rstRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    if (NULL == pstData)
    {
        LOGERR("PlayerData is NULL");
        return ERR_SYS;
    }
    ResGrowthAwardLevelMgr_t& rstResGrowthAwardLevelMgr = CGameDataMgr::Instance().GetResGrowthAwardLevelMgr();
    RESGROWTHAWARDLEVEL* poResLevel = rstResGrowthAwardLevelMgr.Find(rstReq.m_bId);
    if (NULL == poResLevel)
    {
        LOGERR("poResLevel is null from ResGrowthAwardLevel  id<%d>", rstReq.m_bId);
        return ERR_SYS;
    }
    uint16_t wLv = pstData->GetMajestyInfo().m_wLevel;
    if (wLv < poResLevel->m_bLevelNum)
    {
        LOGERR("player  cheat, level limit!");
        return ERR_LEVEL_LIMIT;
    }
    uint32_t& rdwAwardLevel = pstData->GetMiscInfo().m_dwGrowthAwardLevel;
    if ( _CheckAward(rdwAwardLevel, rstReq.m_bId) )
    {
        LOGERR("player  cheat, ERR_AWARD_FINISHED!");
        return ERR_AWARD_FINISHED;
    }

    _SetAward(rdwAwardLevel, rstReq.m_bId);
    rstRsp.m_dwResult = rdwAwardLevel;
    
    for (int i=0; i<poResLevel->m_bPropsNum; i++)
    {
        //注明来源
        Item::Instance().RewardItem(pstData, poResLevel->m_szItemType[i], poResLevel->m_rewardId[i],
            poResLevel->m_rewardNum[i], rstRsp.m_stSyncItemInfo, METHOD_GROWTH_AWARD_LV);
    }

    ZoneLog::Instance().WriteAwardLog(pstData, METHOD_GROWTH_AWARD_LV, rstRsp.m_stSyncItemInfo);

    return ERR_NONE;
}


//在线时间成长奖励
int GrowthAward::GrowthAwardOnline(PlayerData* pstData, IN PKGMETA::CS_PKG_GROWTH_AWARD_REQ& rstReq, OUT PKGMETA::SC_PKG_GROWTH_AWARD_RSP& rstRsp)
{
    rstRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    if (NULL == pstData)
    {
        LOGERR("PlayerData is NULL");
        return ERR_SYS;
    }
    ResGrowthAwardOnlineMgr_t& rstResGrowthAwardOnlineMgr = CGameDataMgr::Instance().GetResGrowthAwardOnlineMgr();
    RESGROWTHAWARDONLINE* poResOnline = rstResGrowthAwardOnlineMgr.Find(rstReq.m_bId);
    if (NULL == poResOnline)
    {
        LOGERR("poResOnline is null from ResGrowthAwardOnline  id<%d>", rstReq.m_bId);
        return ERR_SYS;
    }
    uint64_t ullCurTimeSec = CGameTime::Instance().GetCurrSecond();
    uint32_t& dwAwarOnlineTime = pstData->GetMiscInfo().m_dwGrowthAwardOnlineTime;
    uint32_t& rdwAwardOnline = pstData->GetMiscInfo().m_dwGrowthAwardOnline;


    uint64_t ullOnlineTime = ullCurTimeSec - pstData->GetRoleBaseInfo().m_llLastLoginTime + dwAwarOnlineTime;	//现在累计在线时间 (秒)
    //LOGRUN("Uin<%lu> ullOnlineTime<%lu>, PickTime<%hu> dwAwarOnlineTime<%u>", pstData->m_ullUin, ullOnlineTime, poResOnline->m_wPickTime, dwAwarOnlineTime);
    //  m_wPickTime为当前奖励档的累计总在线时长.
    if (ullOnlineTime < poResOnline->m_wPickTime)
    {
        LOGERR("player  cheat, time interval limit!");
        return ERR_NOT_SATISFY_COND;
    }

    //这里验证 之前的在线时间奖励已领取
    for (int i=1; i<rstReq.m_bId; i++)
    {
        if (!_CheckAward(rdwAwardOnline, i))
        {
            LOGERR("player  cheat, the forward award dos't finished !");
            return ERR_NOT_SATISFY_COND;
        }
    }

    if (_CheckAward(rdwAwardOnline, rstReq.m_bId))
    {
        LOGERR("player  cheat, ERR_AWARD_FINISHED!");
        return ERR_AWARD_FINISHED;
    }

    _SetAward(rdwAwardOnline, rstReq.m_bId);
    dwAwarOnlineTime = 0;
    rstRsp.m_dwResult = rdwAwardOnline;
    
    for (int i = 0; i < poResOnline->m_bPropsNum; i++)
    {
        Item::Instance().RewardItem(pstData, poResOnline->m_szItemType[i], poResOnline->m_rewardId[i], poResOnline->m_rewardNum[i], rstRsp.m_stSyncItemInfo, METHOD_GROWTH_AWARD_ONLINE);
    }
    
    LOGRUN("Uin<%lu> get GrowthAwardOnline AwardId<%hhu>, AwardOnlineTime<%u>, AwardOnlineStat<%u>",
        pstData->m_ullUin, rstReq.m_bId, dwAwarOnlineTime, rdwAwardOnline);
    pstData->GetMiscInfo().m_ullGrowthAwardOnlineLastTime = ullCurTimeSec;
    ZoneLog::Instance().WriteAwardLog(pstData, METHOD_GROWTH_AWARD_ONLINE, rstRsp.m_stSyncItemInfo);

    return ERR_NONE;
}

int GrowthAward::BuyLvFund(PlayerData* pstData, IN PKGMETA::CS_PKG_BUY_LV_FUND_REQ& rstReq, OUT PKGMETA::SC_PKG_BUY_LV_FUND_RSP& rstRsp)
{
    rstRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    RESBASIC* poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_LV_FUND);
    assert(poResBasic);
    uint32_t dwConsumeDiamond = (uint32_t) poResBasic->m_para[0];   //消耗的钻石
    uint32_t dwVipLimit = (uint32_t)poResBasic->m_para[1];          //Vip等级限制

    if (!Item::Instance().IsEnough(pstData, ITEM_TYPE_DIAMOND, 0 , dwConsumeDiamond))
    {
        LOGERR("Uin<%lu> have no enough Diamond<%u>", pstData->m_ullUin, dwConsumeDiamond);
        return ERR_NOT_ENOUGH_DIAMOND;
    }
    if (pstData->GetMiscInfo().m_bLvFundActivateTag != COMMON_DO_SOME_STATE_NONE
        || pstData->GetVIPLv() < dwVipLimit)
    {
        LOGERR("Uin<%lu> have already bought the LvFund or VipLv<CurLv=%hhu> is limit", pstData->m_ullUin, pstData->GetVIPLv());
        return ERR_NOT_SATISFY_COND;
    }
    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -dwConsumeDiamond, rstRsp.m_stSyncItemInfo, METHOD_LV_FUND_BUY);
    pstData->GetMiscInfo().m_bLvFundActivateTag = COMMON_DO_SOME_STATE_FINISHED;

    return ERR_NONE;
}

int GrowthAward::GetLvFundAward(PlayerData* pstData, IN PKGMETA::CS_PKG_GET_LV_FUND_AWARD_REQ& rstReq, OUT PKGMETA::SC_PKG_GET_LV_FUND_AWARD_RSP& rstRsp)
{
    rstRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    if (pstData->GetMiscInfo().m_bLvFundActivateTag != COMMON_DO_SOME_STATE_FINISHED)
    {
        LOGERR("Uin<%lu> may cheat in the game!", pstData->m_ullUin);
        return ERR_NOT_SATISFY_COND;
    }
    RESLVFUND* poResLvFund = CGameDataMgr::Instance().GetResLvFundMgr().GetResByPos(rstReq.m_dwIndex);
    if (!poResLvFund)
    {
        LOGERR("Uin<%lu> Index<%u> is invalid ", pstData->m_ullUin, rstReq.m_dwIndex);
        return ERR_SYS;
    }
    if (poResLvFund->m_wLv > pstData->GetLv())
    {
        LOGERR("Uin<%lu> Index<%u> Lv limit CurLv<%hu> LimitLV<%hu>,  ", pstData->m_ullUin, rstReq.m_dwIndex, pstData->GetLv(), poResLvFund->m_wLv);
        return ERR_LEVEL_LIMIT;
    }

    uint32_t& rdwAwardBitMap = pstData->GetMiscInfo().m_dwLvFundAwardBitmap;
    if ( (rdwAwardBitMap & (1 << rstReq.m_dwIndex)) != 0 )
    {
        LOGERR("Uin<%lu> have got the award Index<%u>", pstData->m_ullUin, rstReq.m_dwIndex);
        return ERR_AWARD_FINISHED;
    }
    rdwAwardBitMap |= 1 << rstReq.m_dwIndex;

    for (int i = 0; i < poResLvFund->m_bPropsNum; i++)
    {
        Item::Instance().RewardItem(pstData, poResLvFund->m_szItemType[i], poResLvFund->m_rewardId[i],
            poResLvFund->m_rewardNum[i], rstRsp.m_stSyncItemInfo, METHOD_LV_FUND_GET_AWARD);
    }
    ZoneLog::Instance().WriteAwardLog(pstData, METHOD_LV_FUND_GET_AWARD, rstRsp.m_stSyncItemInfo);
    LOGRUN("Uin<%lu> get LvFundAward AwardId<%u>, AwardIndex<%u>",
        pstData->m_ullUin, poResLvFund->m_dwId, rstReq.m_dwIndex);
    return ERR_NONE;
}


