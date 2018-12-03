#include "Sign.h"
#include "../gamedata/GameDataMgr.h"
#include "../utils/FakeRandom.h"
#include "GameTime.h"
#include "LogMacros.h"
#include "Equip.h"
#include "GeneralCard.h"
#include "Props.h"
#include "Item.h"
#include "Consume.h"
#include "player/Player.h"
#include "dwlog_svr.h"
#include "ZoneLog.h"
#include "Lottery.h"
#include "../framework/ZoneSvrMsgLayer.h"

using namespace PKGMETA;
using namespace DWLOG;

bool Sign::Init()
{

    return true;
}

//七日签到处理
int Sign::HandleSign7dAward(PlayerData* pstData, OUT SC_PKG_SIGN7D_CLICK_RSP& rstSign7dClickRsp)
{
    DT_ROLE_MISC_INFO& rstMiscInfo = pstData->GetMiscInfo();
    ResSign7dAwardMgr_t& rstResSign7dAwardMgr = CGameDataMgr::Instance().GetResSign7dAwardMgr();
    m_lCurTimeSec = CGameTime::Instance().GetCurrSecond();
    //判断是否领取完
    //m_bSign7dCount=7表示已领取完
    if (MAX_SIGN7D_NUM <= rstMiscInfo.m_bSign7dCount)
    {
        LOGERR("Uin<%lu> Sign7d award had already finshed! ", pstData->m_ullUin);
        return ERR_SIGN7D_MAX_NUM;
    }

    RESSIGN7DAWARD* pstResRecord = rstResSign7dAwardMgr.GetResByPos(rstMiscInfo.m_bSign7dCount);
    if (NULL == pstResRecord)
    {
        LOGERR("Uin<%lu> pstResRecord<index=%hhu> is null", pstData->m_ullUin, rstMiscInfo.m_bSign7dCount);
        return ERR_SYS;
    }

    //检查上次领取时间与现在是否同一天
    if (CGameTime::Instance().IsInSameDay(m_lCurTimeSec, rstMiscInfo.m_llSign7dLastTime))
    {
        LOGERR("Uin<%lu> get award twice a day! last award time<%ld>", pstData->m_ullUin, rstMiscInfo.m_llSign7dLastTime);
        return ERR_SIGN7D_AWARD_TODAY;
    }

    // 添加奖励

    for (uint32_t i = 0; i<pstResRecord->m_bRewardNum; i++)
    {
        Item::Instance().RewardItem(pstData,
            pstResRecord->m_szRewardPropType[i], pstResRecord->m_rewardPropId[i], pstResRecord->m_rewardPropNum[i],
            rstSign7dClickRsp.m_stSyncItemInfo, METHOD_SIGN_7D);
    }

    rstMiscInfo.m_bSign7dCount++;
    rstMiscInfo.m_llSign7dLastTime = m_lCurTimeSec;

    ZoneLog::Instance().WriteAwardLog(pstData, METHOD_SIGN_7D, rstSign7dClickRsp.m_stSyncItemInfo);

    return  ERR_NONE;
}

//月签到处理
int Sign::HandleSign30dAward(INOUT PlayerData* pstData, IN CS_PKG_SIGN30D_CLICK_REQ& rstSign30dClickReq, OUT SC_PKG_SIGN30D_CLICK_RSP& rstSign30dClickRsp)
{
    if (rstSign30dClickReq.m_bType == SIGN30D_TYPE_EXTRA_AWARD)
    {//累计签到奖励
        return _HandleSign30dExtraAward(pstData, rstSign30dClickRsp);
    }

    DT_ROLE_MISC_INFO& rstMiscInfo = pstData->GetMiscInfo();
    m_lCurTimeSec = CGameTime::Instance().GetCurrSecond();
    const tm* pstCurTm = CGameTime::Instance().GetCurrTm();

    localtime_r((const time_t*)&rstMiscInfo.m_llSign30dFirstTime, &m_stFirstTm);
    localtime_r((const time_t*)&rstMiscInfo.m_llSign30dLastTime, &m_stLastTm);
    if ( m_stFirstTm.tm_mon != pstCurTm->tm_mon )
    {//翻月,重置次数
        _Sign30dReset(rstMiscInfo);
        localtime_r((const time_t*)&rstMiscInfo.m_llSign30dFirstTime, &m_stFirstTm);
        localtime_r((const time_t*)&rstMiscInfo.m_llSign30dLastTime, &m_stLastTm);
    }
    int iRet = 0;

    /*
    if ((pstCurTm->tm_mon+1) == m_bOpenMonth && pstCurTm->tm_year == (m_wOpenYear - 1900))
    {//开服第一个月特殊处理                                                   当前要签到的次数 > 全月到目前为止可签到的次数 - 开服前不能签到的次数
        if (m_lCurTimeSec < rstMiscInfo.m_llSign30dLastTime || (rstMiscInfo.m_bSign30dCount + 1) > (pstCurTm->tm_mday - m_bOpenday + 1))
        {  //是否超过当前允许的最大领取天数
            LOGERR("Sign30d: sys time back or player<%lu> cheat! ",pstData->m_pOwner->GetUin());
            return ERR_SYS;
        }
    }
    else
    */

    if (m_lCurTimeSec < rstMiscInfo.m_llSign30dLastTime /*|| (rstMiscInfo.m_bSign30dCount + 1) > pstCurTm->tm_mday */)
    {//检查时间 是否有错(可以不用检测), 是否超过当前允许的最大领取天数
        LOGERR("Sign30d: sys time back or player<%lu> cheat! ",pstData->m_pOwner->GetUin());
        return ERR_SYS;
    }

    switch (rstSign30dClickReq.m_bType)
    {
    case SIGN30D_TYPE_NORMAL:
        iRet = _HandleSign30dNormal(pstData, rstSign30dClickReq, rstSign30dClickRsp);
        break;
    /*
    case SIGN30D_TYPE_FORGET_FREE:
        iRet = _HandleSign30dForgetFree(pstData, rstSign30dClickReq, rstSign30dClickRsp);
        break;
    case SIGN30D_TYPE_FORGET_DIAMOND:
        iRet = _HandleSign30dForgetDiamond(pstData, rstSign30dClickReq, rstSign30dClickRsp);
        break;
    */
    default:
        iRet = ERR_SYS;
        break;
    }

    ZoneLog::Instance().WriteAwardLog(pstData, METHOD_SIGN_30D, rstSign30dClickRsp.m_stSyncItemInfo);

    return  iRet;
}

void Sign::_Sign30dReset(INOUT DT_ROLE_MISC_INFO& rstMiscInfo)
{
    rstMiscInfo.m_bSign30dCount = 0;
    rstMiscInfo.m_bSign30dDiamondCount = 0 ;
    rstMiscInfo.m_bSign30dForgetFreeCount = 0;
    rstMiscInfo.m_llSign30dFirstTime = 0;
    rstMiscInfo.m_llSign30dLastTime = 0;
    rstMiscInfo.m_bSign30dCompensateAward = 0;
}

int Sign::_HandleSign30dNormal(PlayerData* pstData, IN CS_PKG_SIGN30D_CLICK_REQ& rstSign30dClickReq, OUT SC_PKG_SIGN30D_CLICK_RSP& rstSign30dClickRsp)
{
    DT_ROLE_MISC_INFO& rstMiscInfo = pstData->GetMiscInfo();
    //ResSign30dAwardMgr_t& rstResSign30dAwardMgr = CGameDataMgr::Instance().GetResSign30dAwardMgr();
    const tm* pstCurTm = CGameTime::Instance().GetCurrTm();


    //检查上次领取时间与现在是否同一天
    if (CGameTime::Instance().IsInSameDay(m_lCurTimeSec, rstMiscInfo.m_llSign30dLastTime))
    {
        LOGERR("Sign30d:already award! Invalid action!<Uin:%lu>! ",pstData->m_pOwner->GetUin());
        return ERR_SIGN7D_AWARD_TODAY;
    }

    int iRet = _AwardProp(pstData, pstCurTm->tm_mon+1, rstMiscInfo.m_bSign30dCount, rstSign30dClickRsp);

    if (ERR_NONE != iRet)
    {
        return iRet;
    }

    rstMiscInfo.m_bSign30dCount++;
    rstMiscInfo.m_dwSign30dTotalCount++;
    rstMiscInfo.m_llSign30dLastTime = m_lCurTimeSec;
    rstMiscInfo.m_bSign30dCompensateAward = 0;
    if (0 == rstMiscInfo.m_llSign30dFirstTime)
    {
        rstMiscInfo.m_llSign30dFirstTime = m_lCurTimeSec;
    }
    LOGRUN("Uin<%lu> _HandleSign30dNormal, Count<%u>", pstData->m_ullUin, rstMiscInfo.m_bSign30dCount);

    return iRet;
}

/*
int Sign::_HandleSign30dForgetFree(PlayerData* pstData, IN CS_PKG_SIGN30D_CLICK_REQ& rstSign30dClickReq, OUT SC_PKG_SIGN30D_CLICK_RSP& rstSign30dClickRsp)
{
    DT_ROLE_MISC_INFO& rstMiscInfo = pstData->GetMiscInfo();
    //ResSign30dAwardMgr_t& rstResSign30dAwardMgr = CGameDataMgr::Instance().GetResSign30dAwardMgr();
    const tm* pstCurTm = CGameTime::Instance().GetCurrTm();


    if (!CGameTime::Instance().IsInSameDay(m_lCurTimeSec, rstMiscInfo.m_llSign30dLastTime))
    {//上次领取时间和补签要在同一天 才能补签
        LOGERR("Sign30d:Invalid action! type is forget free and must be the same day!<Uin:%lu>! ",pstData->m_pOwner->GetUin());
        return ERR_SYS;
    }
    uint8_t bCnt = GetVipCnt(pstData, SIGN30D_TYPE_FORGET_FREE);

    if (rstMiscInfo.m_bSign30dForgetFreeCount >= bCnt)
    {
        LOGERR("Sign30d:Invalid action! type is forget free and the free count is over!<Uin:%lu>! ",pstData->m_pOwner->GetUin());
        return ERR_SIGN30D_NO_GORGET_FREE_COUNT;
    }

    int iRet = _AwardProp(pstData, pstCurTm->tm_mon, rstMiscInfo.m_bSign30dCount, rstSign30dClickRsp);

    if (ERR_NONE != iRet)
    {
        return iRet;
    }

    rstMiscInfo.m_bSign30dCount++;
    rstMiscInfo.m_llSign30dLastTime = m_lCurTimeSec;
    rstMiscInfo.m_bSign30dForgetFreeCount++;
    LOGRUN("Uin<%lu> _HandleSign30dForgetFree, Count<%u>,FreeCnt<%u>", pstData->m_ullUin, rstMiscInfo.m_bSign30dCount, rstMiscInfo.m_bSign30dForgetFreeCount);
    return iRet;
}

int Sign::_HandleSign30dForgetDiamond(PlayerData* pstData, IN CS_PKG_SIGN30D_CLICK_REQ& rstSign30dClickReq, OUT SC_PKG_SIGN30D_CLICK_RSP& rstSign30dClickRsp)
{
    DT_ROLE_MISC_INFO& rstMiscInfo = pstData->GetMiscInfo();
    //ResSign30dAwardMgr_t& rstResSign30dAwardMgr = CGameDataMgr::Instance().GetResSign30dAwardMgr();
    const tm* pstCurTm = CGameTime::Instance().GetCurrTm();

    if (!CGameTime::Instance().IsInSameDay(m_lCurTimeSec, rstMiscInfo.m_llSign30dLastTime))
    {//上次领取时间和补签要在同一天 才能补签
        LOGERR("Sign30d:Invalid action! type is forget Diamond and must be the same day!<Uin:%lu>! ",pstData->m_pOwner->GetUin());
        return ERR_SYS;
    }
    uint8_t bCnt = GetVipCnt(pstData, SIGN30D_TYPE_FORGET_DIAMOND);
    if (rstMiscInfo.m_bSign30dDiamondCount >= bCnt)
    {
        LOGERR("Sign30d:Invalid action! type is forget Diamond and the Diamond count is over!<Uin:%lu>! ",pstData->m_pOwner->GetUin());
        return ERR_SIGN30D_NO_GORGET_DIAMOND_COUNT;
    }
    ResBasicMgr_t& rResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
    RESBASIC* poBasic = rResBasicMgr.Find(SIGN30D_DIAMOND_CONSUME_RULE);
    if (!poBasic)
    {
        LOGERR("poBasic is null");
        return false;
    }
    //  消耗= MIN(基本值+累加值*次数 , 封顶值)
    int iConsumeDiamond = MIN(poBasic->m_para[0] + poBasic->m_para[1] * rstMiscInfo.m_bSign30dDiamondCount, poBasic->m_para[2]);
    if (!Consume::Instance().IsEnoughDiamond(pstData, iConsumeDiamond))
    {//校验消耗是否够
        return ERR_NOT_ENOUGH_GOLD;
    }
    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -iConsumeDiamond, rstSign30dClickRsp.m_stSyncItemInfo, DWLOG::METHOD_SIGN_30D);
    int iRet = _AwardProp(pstData, pstCurTm->tm_mon, rstMiscInfo.m_bSign30dCount, rstSign30dClickRsp);
    if (ERR_NONE != iRet)
    {
        return iRet;
    }
    rstMiscInfo.m_bSign30dCount++;
    rstMiscInfo.m_llSign30dLastTime = m_lCurTimeSec;
    rstMiscInfo.m_bSign30dDiamondCount++;
    LOGRUN("Uin<%lu> _HandleSign30dForgetDiamond, Count<%u>,DiaCnt<%u>", pstData->m_ullUin, rstMiscInfo.m_bSign30dCount, rstMiscInfo.m_bSign30dDiamondCount);
    return iRet;

}
*/

//  iAwardMouth#实际月份 , iAwardDay#实际要领取的奖励天数-1(刚好是需要奖励的索引)
int Sign::_AwardProp(PlayerData* pstData, int iAwardMonth,  int iAwardDay, OUT SC_PKG_SIGN30D_CLICK_RSP& rstSign30dClickRsp)
{
    ResSign30dAwardMgr_t& rstResSign30dAwardMgr = CGameDataMgr::Instance().GetResSign30dAwardMgr();
    ResSign30dAwardListMgr_t& rstResSign30dAwardListMgr = CGameDataMgr::Instance().GetResSign30dAwardListMgr();
    RESSIGN30DAWARD* pstResRecord = rstResSign30dAwardMgr.Find(iAwardMonth);
    if (NULL == pstResRecord)
    {//检查资源加载
        LOGERR("Uin<%lu> rstResSign30dAwardMgr is null (Id=%d) ", pstData->m_ullUin, iAwardMonth);
        return ERR_SYS;
    }
    int iAwardDouble = pstData->GetVIPLv() >= pstResRecord->m_szVipDoubleLevel[iAwardDay] ? 2 : 1;
    // 添加奖励
    RESSIGN30DAWARDLIST* prsAwardId = rstResSign30dAwardListMgr.Find(pstResRecord->m_rewardIdList[iAwardDay]);
    if (NULL == prsAwardId)
    {//检查资源加载
        LOGERR("Uin<%lu> rstResSign30dAwardListMgr (index=%d) is null", pstData->m_ullUin, iAwardDay);
        return ERR_SYS;
    }


    //正常签到奖励
    Item::Instance().RewardItem(pstData, prsAwardId->m_bItemType, prsAwardId->m_dwItemId,
        prsAwardId->m_wItemNum * iAwardDouble, rstSign30dClickRsp.m_stSyncItemInfo, METHOD_SIGN_30D);
    return ERR_NONE;
}

int Sign::Compensate30dAward(PlayerData* pstData)
{
    m_lCurTimeSec = CGameTime::Instance().GetCurrSecond();
    DT_ROLE_MISC_INFO& rstMiscInfo = pstData->GetMiscInfo();

    if (!CGameTime::Instance().IsInSameDay(m_lCurTimeSec, rstMiscInfo.m_llSign30dLastTime))
    {//检查今天是否领取,没有领取直接返回不管
        return 0;
    }

    int iAwardDay = pstData->GetMiscInfo().m_bSign30dCount -1 ;
    if (iAwardDay < 0 )
    {
        LOGERR("Uin<%lu> Fate Error", pstData->m_ullUin);
        return 0;
    }

    localtime_r((const time_t*)&rstMiscInfo.m_llSign30dLastTime, &m_stLastTm);
    // 添加奖励
    RESSIGN30DAWARD* pstResRecord= CGameDataMgr::Instance().GetResSign30dAwardMgr().Find(m_stLastTm.tm_mon);
    if (NULL == pstResRecord)
    {//检查资源加载
        LOGERR("Uin<%lu> rstResSign30dAwardMgr is null (Id=%d) ", pstData->m_ullUin, m_stLastTm.tm_mon);
        return ERR_SYS;
    }
    if (pstData->GetVIPLv() < pstResRecord->m_szVipDoubleLevel[iAwardDay])
    {// 没有双倍
        return 0;
    }
    if (rstMiscInfo.m_bSign30dCompensateAward != 0)
    {//本次已补偿
        return 0;
    }


    // 补偿奖励
    RESSIGN30DAWARDLIST* prsAwardId = CGameDataMgr::Instance().GetResSign30dAwardListMgr().Find(pstResRecord->m_rewardIdList[iAwardDay]);
    if (NULL == prsAwardId)
    {//检查资源加载
        LOGERR("Uin<%lu> rstResSign30dAwardListMgr (index=%d) is null", pstData->m_ullUin, iAwardDay);
        return ERR_SYS;
    }
    // 由邮件发送


    RESPRIMAIL* poResPriMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(10003);
    if (NULL == poResPriMail)
    {
        LOGERR("Uin<%lu> ResPriMail <10001> is null",pstData->m_ullUin);
        return ERR_SYS;
    }
    rstMiscInfo.m_bSign30dCompensateAward = 1;
    bzero(&m_stSsPkg, sizeof(m_stSsPkg));
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
    m_stSsPkg.m_stBody.m_stMailAddReq.m_nUinCount = 1;
    m_stSsPkg.m_stBody.m_stMailAddReq.m_UinList[0] = pstData->m_ullUin;

    DT_MAIL_DATA& rstMailData = m_stSsPkg.m_stBody.m_stMailAddReq.m_stMailData;
    rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
    rstMailData.m_bState = MAIL_STATE_OPENED;
    StrCpy(rstMailData.m_szTitle, poResPriMail->m_szTitle, MAX_MAIL_TITLE_LEN);
    snprintf(rstMailData.m_szContent, MAX_MAIL_CONTENT_LEN, poResPriMail->m_szContent, pstData->GetRoleBaseInfo().m_szRoleName);
    rstMailData.m_ullFromUin = 0;
    rstMailData.m_bAttachmentCount = 1;
    rstMailData.m_astAttachmentList[0].m_bItemType = prsAwardId->m_bItemType;
    rstMailData.m_astAttachmentList[0].m_dwItemId = prsAwardId->m_dwItemId;
    rstMailData.m_astAttachmentList[0].m_iValueChg = prsAwardId->m_wItemNum;
    rstMailData.m_ullTimeStampMs = CGameTime::Instance().GetCurrSecond();
    ZoneSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);

    return 0;

}

int Sign::_HandleSign30dExtraAward(IN PlayerData* pstData, OUT SC_PKG_SIGN30D_CLICK_RSP& rstSign30dClickRsp)
{
    DT_ROLE_MISC_INFO& rstMiscInfo = pstData->GetMiscInfo();
    //int iAwardMonth = CGameTime::Instance().GetCurrMonth() + 1;

//     localtime_r(&rstMiscInfo.m_llSign30dExtraLastTime, &m_stLastTm);
//     if (CGameTime::Instance().GetCurrMonth() != m_stLastTm.tm_mon)
//     {//重置次数
//         rstMiscInfo.m_bSign30dExtraCount = 0;
//     }

    RESSIGN30DEXTRAREWARD* pstResExtraReward = CGameDataMgr::Instance().GetResSign30dExtraRewardMgr().GetResByPos(rstMiscInfo.m_wSign30dExtraCount);
    if (NULL == pstResExtraReward)
    {
        LOGERR("Uin<%lu> pstResExtraReward is NULL Pos<%u>", pstData->m_ullUin, rstMiscInfo.m_dwSign30dTotalCount);
        return ERR_SYS;
    }
    if (rstMiscInfo.m_dwSign30dTotalCount < pstResExtraReward->m_wSignCountLimit)
    {
        LOGERR("Uin<%lu>  current SignTotalCount<%u> , SignCountLimitNum<%hu>", pstData->m_ullUin, rstMiscInfo.m_dwSign30dTotalCount, pstResExtraReward->m_wSignCountLimit);
        return ERR_SYS;
    }

    for (uint16_t i = 0; i < pstResExtraReward->m_bRewardNum; i++)
    {
        Item::Instance().RewardItem(pstData, pstResExtraReward->m_szRewardPropType[i], pstResExtraReward->m_rewardPropId[i],
            pstResExtraReward->m_rewardPropNum[i], rstSign30dClickRsp.m_stSyncItemInfo, METHOD_SIGN_30D_EXTRA);
    }

    rstMiscInfo.m_wSign30dExtraCount++ ;
    rstMiscInfo.m_llSign30dExtraLastTime = CGameTime::Instance().GetCurrSecond();
    return ERR_NONE;
}



/*
uint8_t Sign::GetVipCnt(PlayerData* pstData, uint8_t bType)
{
    uint8_t bVipLv = pstData->GetVIPLv();
    RESVIP* poResVip = CGameDataMgr::Instance().GetResVIPMgr().GetResByPos(bVipLv);
    if (NULL == poResVip)
    {
        LOGERR("ResVip is null Uin<%lu>,VipLv<%u>", pstData->m_ullUin, bVipLv);
        return 0;
    }
    uint8_t bCnt = 0;
    switch (bType)
    {
    case SIGN30D_TYPE_NORMAL:
        bCnt = 0;
        break;
    case SIGN30D_TYPE_FORGET_FREE:
        bCnt = poResVip->m_bFreeSign;
        break;
    case SIGN30D_TYPE_FORGET_DIAMOND:
        bCnt = poResVip->m_bPaySign;
        break;
    default:
        bCnt = 0;
        break;
    }
    return  bCnt;
}
*/

