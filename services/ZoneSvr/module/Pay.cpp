#include "Pay.h"
#include "VIP.h"
#include "Item.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "ZoneLog.h"
#include "dwlog_def.h"
#include "Consume.h"
#include "SvrTime.h"
#include "Task.h"
#include "Message.h"
#include "TaskAct.h"

using namespace PKGMETA;
using namespace DWLOG;

int DoublePayCmp(const void *pstFirst, const void *pstSecond)
{
    int iResult = (int)( *(uint32_t*)pstFirst - *(uint32_t*)pstSecond );

    return iResult;
}

int LiCmp(const void *pstFirst, const void *pstSecond)
{
    int iResult = (int)(*(uint32_t*)pstFirst - *(uint32_t*)pstSecond);

    return iResult;
}

int GiftBagCmp(const void *pstFirst, const void *pstSecond)
{
    DT_GIFTBAG_INFO* pstFirstGiftBag = (DT_GIFTBAG_INFO*)pstFirst;
    DT_GIFTBAG_INFO* pstSecondGiftBag = (DT_GIFTBAG_INFO*)pstSecond;

    if (pstFirstGiftBag->m_dwId > pstSecondGiftBag->m_dwId)
    {
        return 1;
    }
    else if (pstFirstGiftBag->m_dwId < pstSecondGiftBag->m_dwId)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int CardCmp(const void *pstFirst, const void *pstSecond)
{
    DT_CARD_INFO* pstFirstGiftBag = (DT_CARD_INFO*)pstFirst;
    DT_CARD_INFO* pstSecondGiftBag = (DT_CARD_INFO*)pstSecond;

    if (pstFirstGiftBag->m_dwId > pstSecondGiftBag->m_dwId)
    {
        return 1;
    }
    else if (pstFirstGiftBag->m_dwId < pstSecondGiftBag->m_dwId)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}


bool Pay::Init()
{
    RESBASIC *poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(COMMON_UPDATE_TIME);
    if (NULL == poResBasic)
    {
        LOGERR("Pay Init error!");
        return false;
    }
    m_iUptHour = (int)poResBasic->m_para[0];

    m_bSettleFlag = 0;

    //初始化开服折扣的数据档
    int iNum = CGameDataMgr::Instance().GetResDiscountPropsMgr().GetResNum();
    RESDISCOUNTPROPS* pResProps = NULL;
    int iCursor = 0;
    for (int i = 0; i < iNum; i++)
    {
        pResProps = CGameDataMgr::Instance().GetResDiscountPropsMgr().GetResByPos(i);
        assert(pResProps);
        for (int j = 0; j < pResProps->m_bCount; j++)
        {
            m_astDiscountPropsList[iCursor].m_dwId = pResProps->m_propsId[j];
            m_astDiscountPropsList[iCursor].m_dwNum = pResProps->m_propsNum[j];
            m_astDiscountPropsList[iCursor].m_bType = pResProps->m_szPropsType[j];
            m_astDiscountPropsList[iCursor].m_dwPrice = pResProps->m_propsPrice[j];
            m_astDiscountPropsList[iCursor].m_bDiscount = pResProps->m_szPropsDiscount[j];
            m_astDiscountPropsList[iCursor].m_dwTotalNum = pResProps->m_propsTotalNum[j];
            m_astDiscountPropsList[iCursor].m_dwRemainNum = pResProps->m_propsTotalNum[j];
            m_astDiscountPropsList[iCursor].m_bVip = pResProps->m_szPropsVip[j];
            iCursor++;
        }
    }

    //初始化战力活动的数据档
    m_bLiRewardSecCnt = (uint8_t)CGameDataMgr::Instance().GetResLiRewardMgr().GetResNum();
    RESACTIVITYFORCEFORCEREWARD* pResLiReward = NULL;
    m_LiRewardSec = new uint32_t[iNum];
    for (int i = 0; i < iNum; i++)
    {
        pResLiReward = CGameDataMgr::Instance().GetResLiRewardMgr().GetResByPos(i);
        assert(pResLiReward);
        m_LiRewardSec[i] = pResLiReward->m_dwForceLower;
    }

    if (!this->_InitFromFile())
    {
        return false;
    }

    this->_WriteToFile();

    return true;
}

bool Pay::_InitFromFile()
{
    if (access(DISCOUNT_INFO_FILE, F_OK))
    {
        m_fp = fopen(DISCOUNT_INFO_FILE, "wb+");
        return (m_fp!=NULL);
    }
    else
    {
        m_fp = fopen(DISCOUNT_INFO_FILE, "rb+");
        fseek(m_fp, 0, SEEK_SET);
        if(fread(&m_bSettleFlag, sizeof(m_bSettleFlag), 1, m_fp) != 1)
        {
            LOGERR_r("Read m_bSettleFlag from file failed");
            return false;
        }
        for (int i=0; i<MAX_DISCOUNT_ACTIVITY_DAY*MAX_DISCOUNT_PROPS_NUM; i++)
        {
            if(fread(&m_astDiscountPropsList[i].m_dwRemainNum, sizeof(uint32_t), 1, m_fp) != 1)
            {
                LOGERR_r("Read m_astDiscountPropsList[%d] from file failed", i);
                return false;
            }
        }

        return true;
    }
}

void Pay::_WriteToFile()
{
    fseek(m_fp, 0, SEEK_SET);
    if(fwrite(&m_bSettleFlag, sizeof(m_bSettleFlag), 1, m_fp) != 1)
    {
       LOGERR_r("Write m_bSettleFlag to file failed");
       return;
    }
    for (int i=0; i<MAX_DISCOUNT_ACTIVITY_DAY*MAX_DISCOUNT_PROPS_NUM; i++)
    {
        if(fwrite(&m_astDiscountPropsList[i].m_dwRemainNum, sizeof(uint32_t), 1, m_fp) != 1)
        {
            LOGERR_r("Write m_astDiscountPropsList[%d] to file failed", i);
            return;
        }
    }

    fflush(m_fp);
}

void Pay::Fini()
{
    this->_WriteToFile();
}


void Pay::UpdateServer()
{

}

void Pay::UpdatePlayerData(PlayerData* pstData)
{
    if (m_bSettleFlag && (pstData->GetMiscInfo().m_stPayActivityInfo.m_bLiRewardInfo==0))
    {
        this->_SettleLiReward(pstData);
    }
    RESBASIC* poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(4001);
    if (poResBasic == NULL)
    {
        LOGERR("poResBasic is NULL");
    }
    uint8_t bRefreshHour = poResBasic->m_para[0];
    uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();
    uint64_t ull5OClock = CGameTime::Instance().GetSecOfGivenTime(pstData->GetMiscInfo().m_stPayInfo.m_ullLastBuyOutletsTime, bRefreshHour);
    if (ullCurTime - ull5OClock > SECONDS_OF_DAY)
    {
        pstData->GetMiscInfo().m_stPayInfo.m_bOutletsState = 0;
    }

    //月卡，周卡等各种卡检查是否需要发奖励
    if (pstData->m_ullCardLastUptTime < CGameTime::Instance().GetBeginOfDay())
    {
        this->CheckCardReward(pstData);
        pstData->m_ullCardLastUptTime = CGameTime::Instance().GetBeginOfDay();
    }
}

bool Pay::LoadGameData()
{
    return true;
}

void Pay::DoPayOk(PKGMETA::SS_PKG_SDK_PAY_CB_NTF& rstPayCbNtf)
{
    RESPAY* pResPay = CGameDataMgr::Instance().GetResPayMgr().Find(rstPayCbNtf.m_stPayCbInfo.m_dwProductID);
    if (pResPay == NULL)
    {
        LOGRUN("product is not exist.");
        return ;
    }

    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstPayCbNtf.m_stPayCbInfo.m_ullRoleID);
    //玩家在内存中，直接发钻石
    if (poPlayer)
    {
        PlayerData* pstData = &poPlayer->GetPlayerData();

        if (pResPay->m_bPayType == 3        //奥特莱斯
            || pResPay->m_bPayType == 4)    //皮肤礼包
        {
            _PayForGiftBag(pstData, pResPay);
        }
        else if (pResPay->m_bPayType == 2)
        {//月卡
            _PayForMonthCard(pstData, pResPay);
        }
        else
        {//普通充值
            _PayForDiamond(pstData, pResPay);
        }

        DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
        rstMajestyInfo.m_dwTempTotalCharge += pResPay->m_dwCurrencyNum;

        //记录充值数据到TD
        this->_SendtoTD(rstPayCbNtf, pResPay, poPlayer);

        //记录充值Log
        ZoneLog::Instance().WritePayLog(poPlayer, rstPayCbNtf.m_stPayCbInfo, pResPay->m_dwPrice);
    }
    else
    {//不在内存中，记录一下,通过客服补偿,这个慨率是非常非常低的
        LOGERR("Uin<%lu> player pays ok but is offline,pack award to mail to him. ProductId<%u> orderId<%s>", rstPayCbNtf.m_stPayCbInfo.m_ullRoleID,
            rstPayCbNtf.m_stPayCbInfo.m_dwProductID, rstPayCbNtf.m_stPayCbInfo.m_szOrderId);
    }
}


//将订单信息发送到talkingdata
void Pay::_SendtoTD(SS_PKG_SDK_PAY_CB_NTF& rstPayCbNtf, RESPAY* pResPay, Player* poPlayer)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_SDKDM_TDATA_SEND_ORDER_NTF;
    SS_PKG_SDKDM_TDATA_SEND_ORDER_NTF& rstNtf = m_stSsPkg.m_stBody.m_stSdkDMTDataSendOrderNtf;

    snprintf(rstNtf.m_stOrderMsg.m_szMsgID, 65, "%lu_%lu", poPlayer->GetUin(), CGameTime::Instance().GetCurrSecond());
    snprintf(rstNtf.m_stOrderMsg.m_szStatus, 10, "success");
    snprintf(rstNtf.m_stOrderMsg.m_szAccountID, 65, "%lu", poPlayer->GetUin());
    if (rstPayCbNtf.m_stPayCbInfo.m_szOrderId[0] != 0)
    {
        StrCpy(rstNtf.m_stOrderMsg.m_szOrderID, rstPayCbNtf.m_stPayCbInfo.m_szOrderId, MAX_LEN_SDK_PARA);
    }
    else
    {
        StrCpy(rstNtf.m_stOrderMsg.m_szOrderID, rstNtf.m_stOrderMsg.m_szMsgID, 65);
    }
    rstNtf.m_stOrderMsg.m_fCurrencyAmount = (float)pResPay->m_dwPrice/100.0f;
    snprintf(rstNtf.m_stOrderMsg.m_szCurrencyType, 4, "CNY");
    rstNtf.m_stOrderMsg.m_fVirtualCurrencyAmount = (float)pResPay->m_dwCurrencyNum;
    rstNtf.m_stOrderMsg.m_ullChargeTime = CGameTime::Instance().GetCurrSecond();
    snprintf(rstNtf.m_stOrderMsg.m_szIapID, 65, "%u", pResPay->m_dwProductId);
    snprintf(rstNtf.m_stOrderMsg.m_szGameServer, 17, "%d", ZoneSvr::Instance().GetConfig().m_wSvrId);

    StrCpy(rstNtf.m_stOrderMsg.m_szOS, poPlayer->GetOSType(), 10);
    rstNtf.m_stOrderMsg.m_iLevel = poPlayer->GetRoleLv();
    StrCpy(rstNtf.m_stOrderMsg.m_szPartner, poPlayer->GetSubChannelName(), PKGMETA::MAX_NAME_LENGTH);

    ZoneSvrMsgLayer::Instance().SendToSdkDMSvr(m_stSsPkg);
}


void Pay::GmDoPayOk(uint64_t ullUin, uint64_t ullProductID)
{
    RESPAY* pResPay = CGameDataMgr::Instance().GetResPayMgr().Find(ullProductID);
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(ullUin);
    if (poPlayer)
    {
        PlayerData* pstData = &poPlayer->GetPlayerData();

        if (pResPay->m_bPayType == 3)
        {//奥特莱斯
            _PayForGiftBag(pstData, pResPay);
        }
        else if (pResPay->m_bPayType == 2)
        {//月卡
            _PayForMonthCard(pstData, pResPay);
        }
        else
        {//普通充值
            _PayForDiamond(pstData, pResPay);
        }
    }
}

void Pay::AwardFirstDoubleAward(PlayerData* pstData, RESPAY* pResPay, OUT SC_PKG_PAY_SYN_NTF& rstSync)
{
    if (PAY_TYPE_NORMAL != pResPay->m_bPayType)
    {
        return;
    }
    DT_ROLE_PAY_INFO& rstPayInfo = pstData->GetMiscInfo().m_stPayInfo;

    uint32_t dwSearchedId = pResPay->m_dwProductId;
    int iEqual = 0;
    MyBSearch(&dwSearchedId, rstPayInfo.m_DoublePayStateList, rstPayInfo.m_wDoublePayNum, sizeof(dwSearchedId), &iEqual, DoublePayCmp);
    if (!iEqual)
    {//没有,插入,发送双倍

        size_t nmemb = (size_t)rstPayInfo.m_wDoublePayNum;
        if (nmemb >= MAX_PAY_NORMAL_ID_NUM)
        {
            LOGERR("rstPayInfo.m_wDoublePayNum<%d> reaches MAX_PAY_NORMAL_ID_NUM", rstPayInfo.m_wDoublePayNum);
            return;
        }
        if (MyBInsert(&dwSearchedId, rstPayInfo.m_DoublePayStateList, &nmemb, sizeof(dwSearchedId), 1, DoublePayCmp))
        {
            rstPayInfo.m_wDoublePayNum = (uint16_t)nmemb;
        }
        Item::Instance().RewardItem(pstData, pResPay->m_bCurrencyType, 0, pResPay->m_dwCurrencyNum,
            rstSync.m_stSyncItemInfo, METHOD_PAY_NORMAL_FIRST_DOUBLE_AWARD);
        LOGRUN("Uin<%lu> pay first double award ok! ProductId<%u> CurrencyType<%hhu> CurrencyNum<%u> ", pstData->m_ullUin, pResPay->m_dwProductId, pResPay->m_bCurrencyType, pResPay->m_dwCurrencyNum);
    }
}

void Pay::ActivateFirstPayAward(PlayerData* pstData)
{
    DT_ROLE_PAY_INFO& rstPayInfo = pstData->GetMiscInfo().m_stPayInfo;
    if (COMMON_AWARD_STATE_NONE == rstPayInfo.m_bFirstPayAwardState)
    {
        rstPayInfo.m_bFirstPayAwardState = COMMON_AWARD_STATE_AVAILABLE;
        LOGRUN("Uin<%lu> activate the FirstPay award ok!!", pstData->m_ullUin);
    }
}

void Pay::ActivateFDayPayAward(PlayerData* pstData)
{
    uint64_t ullFDayStartTime = pstData->GetMiscInfo().m_stPayInfo.m_ullFDayPayStartTime;
    uint8_t &brFDayState = pstData->GetMiscInfo().m_stPayInfo.m_bFDayPayAwardState;
    if (brFDayState == COMMON_AWARD_STATE_NONE &&
        (ullFDayStartTime == 0 ||  CGameTime::Instance().IsContainCur(ullFDayStartTime, ullFDayStartTime + SECONDS_OF_DAY)) )
    {
        brFDayState = COMMON_AWARD_STATE_AVAILABLE;
        LOGRUN("Uin<%lu> activate the FDayPay award ok!!", pstData->m_ullUin);
    }
}

void Pay::_PayForMonthCard(PlayerData* pstData, RESPAY* pResPay)
{
    uint32_t dwMonthCardId = pResPay->m_dwParam;
    RESMONTHDAILYBAG *poResMonthDailyBag = CGameDataMgr::Instance().GetResMonthDailyBagMgr().Find(dwMonthCardId);
    if (NULL == poResMonthDailyBag)
    {
        LOGERR("Uin<%lu> Buy MonthCard failed. The RESMONTHDAILYBAG<id %u> is NULL", pstData->m_ullUin, dwMonthCardId);
        return;
    }
    DT_ROLE_PAY_INFO& rstPayInfo = pstData->GetMiscInfo().m_stPayInfo;
    DT_MONTH_CARD_INFO& rstMonthCard = rstPayInfo.m_astMonthCard[dwMonthCardId-1];

    rstMonthCard.m_bValid = 1;
    rstMonthCard.m_bType = poResMonthDailyBag->m_bType;
    rstMonthCard.m_ullBuyTime = CGameTime::Instance().GetCurrSecond();
    rstMonthCard.m_bAwardNum = 0;
    rstMonthCard.m_ullLastAwarTime = 0;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PAY_SYN_NTF;
    SC_PKG_PAY_SYN_NTF& rstSync = m_stScPkg.m_stBody.m_stPaySynNtf;
    rstSync.m_stSyncItemInfo.m_bSyncItemCount = 0;
    rstSync.m_dwProductId = pResPay->m_dwProductId;
    Item::Instance().RewardItem(pstData, ITEM_TYPE_VIP_EXP, 0, pResPay->m_dwVipExp,
        rstSync.m_stSyncItemInfo, METHOD_PAY_BUY_MONTH_CARD);
    Item::Instance().RewardItem(pstData, poResMonthDailyBag->m_bPropsType, poResMonthDailyBag->m_dwPropsId,
        poResMonthDailyBag->m_dwPropsNum, rstSync.m_stSyncItemInfo, METHOD_PAY_BUY_MONTH_CARD);

    //首充激活
    ActivateFirstPayAward(pstData);
    //首日激活
    ActivateFDayPayAward(pstData);
    //充值活动处理
    DoActPay(pstData, pResPay->m_dwVipExp);

    rstSync.m_stPayInfo = pstData->GetMiscInfo().m_stPayInfo;
    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
    //发送消息广播
    if (rstMonthCard.m_bType == 1) //小月卡
    {
        Message::Instance().AutoSendSysMessage(1401, "Name=%s", pstData->GetRoleName());
        Message::Instance().AutoSendWorldMessage(pstData, 1403);
    }
    else if (rstMonthCard.m_bType == 2) //永久月卡
    {
        Message::Instance().AutoSendSysMessage(1402, "Name=%s", pstData->GetRoleName());
        Message::Instance().AutoSendWorldMessage(pstData, 1404);
    }
}

void Pay::_PayForDiamond(PlayerData* pstData, RESPAY* pResPay)
{
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PAY_SYN_NTF;
    SC_PKG_PAY_SYN_NTF& rstSync = m_stScPkg.m_stBody.m_stPaySynNtf;
    rstSync.m_stSyncItemInfo.m_bSyncItemCount = 0;
    rstSync.m_dwProductId = pResPay->m_dwProductId;
    //奖励钻石
    Item::Instance().RewardItem(pstData, pResPay->m_bCurrencyType, 0, pResPay->m_dwCurrencyNum,
        rstSync.m_stSyncItemInfo, METHOD_PAY_NORMAL);
    //奖励Vip经验
    Item::Instance().RewardItem(pstData, ITEM_TYPE_VIP_EXP, 0, pResPay->m_dwVipExp,
        rstSync.m_stSyncItemInfo, METHOD_PAY_NORMAL);
    //双倍奖励
    AwardFirstDoubleAward(pstData, pResPay, rstSync);
    //首充激活
    ActivateFirstPayAward(pstData);
    //首日激活
    ActivateFDayPayAward(pstData);

    //充值活动处理
    DoActPay(pstData, pResPay->m_dwVipExp);

    rstSync.m_stPayInfo = pstData->GetMiscInfo().m_stPayInfo;
    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
}

void Pay::_PayForGiftBag(PlayerData * pstData, RESPAY * pResPay)
{
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PAY_SYN_NTF;
    SC_PKG_PAY_SYN_NTF& rstSync = m_stScPkg.m_stBody.m_stPaySynNtf;
    rstSync.m_stSyncItemInfo.m_bSyncItemCount = 0;
    rstSync.m_dwProductId = pResPay->m_dwProductId;

    if (!CheckGiftBag(pstData, pResPay->m_dwProductId))
    {
        LOGERR("Player<%lu> CheckGiftBag<%u> failed", pstData->m_ullUin, pResPay->m_dwParam);
        return;
    }

    if (!_GiftBagReward(pstData, rstSync.m_stSyncItemInfo, pResPay->m_dwParam))
    {
        LOGERR("Player<%lu> Giftbag<%u> Reward failed", pstData->m_ullUin, pResPay->m_dwParam);
        return;
    }

    //奖励Vip经验
    Item::Instance().RewardItem(pstData, ITEM_TYPE_VIP_EXP, 0, pResPay->m_dwVipExp,
        rstSync.m_stSyncItemInfo, METHOD_PAY_NORMAL);

    //充值活动处理
    DoActPay(pstData, pResPay->m_dwVipExp);
    //首充激活
    ActivateFirstPayAward(pstData);
    //首日激活
    ActivateFDayPayAward(pstData);

    rstSync.m_stPayInfo = pstData->GetMiscInfo().m_stPayInfo;

    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
}

void Pay::DoActPay(PlayerData* pstData, uint32_t dwDetaValue)
{
    Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_PAY, dwDetaValue, 1, 0, 0);
    Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_TOTAL_PAY, dwDetaValue, 1, 0, 0);
    uint64_t StartTime = 0, EndTime = 0;
    if (!TaskAct::Instance().IsActTypeOpen(pstData, ACT_TYPE_TOTAL_PAY_ACTIVITY, StartTime, EndTime))
    {
        //活动未开
        return;
    }
    //累计充值
    this->_AddActTotalPay(pstData, StartTime, dwDetaValue);
}

void Pay::DoActConsume(PlayerData* pstData, uint32_t dwDetaValue)
{
    uint64_t StartTime = 0, EndTime = 0;
    if (!TaskAct::Instance().IsActTypeOpen(pstData, ACT_TYPE_DIAMOND_CUME_ACTIVITY, StartTime, EndTime))
    {
        //活动未开
        return;
    }
    //累计充值
    this->_AddActTotalConsume(pstData, StartTime, dwDetaValue);
}

void Pay::_AddActTotalPay(PlayerData* pstData, uint64_t ullStartTime, uint32_t dwDetaValue)
{
    DT_ROLE_PAY_INFO& rstPayInfo = pstData->GetMiscInfo().m_stPayInfo;

    //检查是否需要重置
    if (rstPayInfo.m_ullActPayStartTime < ullStartTime)
    {
        //保存的还是上次参加活动的信息, 重置
        rstPayInfo.m_ullActPayStartTime = CGameTime::Instance().GetCurrSecond();
        rstPayInfo.m_dwActPayTotal = 0;
    }
    rstPayInfo.m_dwActPayTotal += dwDetaValue;
}

void Pay::_AddActTotalConsume(PlayerData* pstData, uint64_t ullStartTime, uint32_t dwDetaValue)
{
    DT_ROLE_PAY_INFO& rstPayInfo = pstData->GetMiscInfo().m_stPayInfo;
    //检查是否需要重置
    if (rstPayInfo.m_ullActPayStartTime < ullStartTime)
    {
        //保存的还是上次参加活动的信息, 重置
        rstPayInfo.m_ullActPayStartTime = CGameTime::Instance().GetCurrSecond();
        rstPayInfo.m_dwActPayTotal = 0;
    }
    rstPayInfo.m_dwActConsumeTotal += dwDetaValue;
}


void Pay::GetMonthCardAward(PlayerData* pstData, CS_PKG_PAY_GET_AWARD_REQ& rstPkgReq)
{
    DT_ROLE_PAY_INFO& rstPayInfo = pstData->GetMiscInfo().m_stPayInfo;
    uint32_t dwMonthCardId = rstPkgReq.m_dwGetTypePara;
    int iRet = ERR_NONE;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PAY_GET_AWARD_RSP;
    SC_PKG_PAY_GET_AWARD_RSP& rstPkgRsp = m_stScPkg.m_stBody.m_stGetPayAwardRsp;

    rstPkgRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    do
    {
        RESMONTHDAILYBAG *poResMonthDailyBag = CGameDataMgr::Instance().GetResMonthDailyBagMgr().Find(dwMonthCardId);
        if (NULL == poResMonthDailyBag)
        {
            LOGERR("Uin<%lu> GetMonthCardAward failed. The RESMONTHDAILYBAG<id %u> is NULL", pstData->m_ullUin, dwMonthCardId);
            iRet = ERR_SYS;
            break;
        }

        if (!_CheckMonthCard(pstData, dwMonthCardId))
        {
            iRet = ERR_NOT_SATISFY_COND;
            break;
        }

        DT_MONTH_CARD_INFO& rstMonthCard =  pstData->GetMiscInfo().m_stPayInfo.m_astMonthCard[dwMonthCardId-1];

        Item::Instance().RewardItem(pstData, poResMonthDailyBag->m_bDailyType, poResMonthDailyBag->m_dwDailyId,
            poResMonthDailyBag->m_dwDailyNum, rstPkgRsp.m_stSyncItemInfo, METHOD_PAY_MONTH_CARD_DAILY_AWARD);
        rstMonthCard.m_bAwardNum ++;
        rstMonthCard.m_ullLastAwarTime = CGameTime::Instance().GetCurrSecond();

        LOGRUN("Uin<%lu> GetmonthCardAward OK. MonthCard:id<%u>, valid<%hhu>, type<%hhu>, AwardNum<%hhu>, BuyTime<%lu>, LastAwardTime<%lu> ",
            pstData->m_ullUin, dwMonthCardId, rstMonthCard.m_bValid, rstMonthCard.m_bType, rstMonthCard.m_bAwardNum, rstMonthCard.m_ullBuyTime, rstMonthCard.m_ullLastAwarTime);
    } while (0);
    rstPkgRsp.m_nErrNo = iRet;
    rstPkgRsp.m_stPayInfo = rstPayInfo;
    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
}

bool Pay::_CheckMonthCard(PlayerData* pstData, uint32_t dwMonthCardId)
{
    DT_MONTH_CARD_INFO& rstMonthCard =  pstData->GetMiscInfo().m_stPayInfo.m_astMonthCard[dwMonthCardId-1];
    if (rstMonthCard.m_bValid == 0)
    {
        return false;
    }

    uint64_t ullStartTime = CGameTime::Instance().GetSecOfCycleHourInSomeDay(rstMonthCard.m_ullBuyTime, m_iUptHour);    //月卡生效时间
    uint64_t ullEndTime = dwMonthCardId == CONST_MONTH_CARD_FOREVER ? (CGameTime::Instance().GetCurrSecond() + SECONDS_OF_DAY) : (ullStartTime + m_ullMonthCardDurationSec - 1); //月卡结束时间                                                //月卡结束时间
    //获取当前的刷新周期(1天,凌晨3点到次日3点为一个领奖周期)的刷新时间
    //  如果玩家上次领取时间比ullDailyAwardUptTime小,说明这次可以领取,反之不能领取
    uint64_t ullDailyAwardUptTime = CGameTime::Instance().GetSecOfCycleHourInCurrday(m_iUptHour);
    uint16_t wAwardMax = dwMonthCardId == CONST_MONTH_CARD_FOREVER ? 999: MAX_MONTH_CARD_DAILY_NUM;
    //永久月卡 领取次数不限制,m_bAwardNum++后会重置,小于999
    if (!(rstMonthCard.m_bAwardNum < wAwardMax &&                     //次数是否超过
        CGameTime::Instance().IsContainCur(ullStartTime, ullEndTime) &&                 //月卡时间是否有效
        rstMonthCard.m_ullLastAwarTime < ullDailyAwardUptTime) )                        //上次领取时间与这次是否在同一个周期
    {
        LOGERR("Uin<%lu> GetmonthCardAward failed. condition limit! MonthCard:id<%u>, valid<%hhu>, type<%hhu>, AwardNum<%hhu>, BuyTime<%lu>, LastAwardTime<%lu> ",
            pstData->m_ullUin, dwMonthCardId, rstMonthCard.m_bValid, rstMonthCard.m_bType, rstMonthCard.m_bAwardNum, rstMonthCard.m_ullBuyTime, rstMonthCard.m_ullLastAwarTime);
        return false;
    }

    return true;
}

void Pay::CheckCardReward(PlayerData* pstData)
{
    uint64_t ullCurTime = CGameTime::Instance().GetBeginOfDay();

    DT_ROLE_PAY_INFO& rstPayInfo = pstData->GetMiscInfo().m_stPayInfo;

    for (int i = 0; i < rstPayInfo.m_bCardCnt; i++)
    {
        DT_CARD_INFO& rstCardInfo = rstPayInfo.m_astCardList[i];

        uint64_t ullEndTime = ullCurTime < rstCardInfo.m_ullEndTime ? ullCurTime : rstCardInfo.m_ullEndTime;
        if (rstCardInfo.m_ullLastAwardTime >= ullEndTime)
        {
            continue;
        }

        RESOUTLETS *pResOutlets = CGameDataMgr::Instance().GetResOutletsMgr().Find(rstCardInfo.m_dwId);
        if (!pResOutlets)
        {
            LOGERR("Player<%lu> ResOutlets<%u> is NULL", pstData->m_ullUin, rstCardInfo.m_dwId);
            continue;
        }
        RESPRIMAIL *pResMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(pResOutlets->m_dwMailId);
        if (!pResMail)
        {
            LOGERR("Player<%lu> ResMail<%u> is NULL", pstData->m_ullUin, pResOutlets->m_dwMailId);
            continue;
        }

        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
        SS_PKG_MAIL_ADD_REQ& rstMailAddReq = m_stSsPkg.m_stBody.m_stMailAddReq;
        rstMailAddReq.m_nUinCount = 1;
        rstMailAddReq.m_UinList[0] = pstData->m_ullUin;
        DT_MAIL_DATA& rstMailData = rstMailAddReq.m_stMailData;
        rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
        rstMailData.m_bState = MAIL_STATE_UNOPENED;
        StrCpy(rstMailData.m_szTitle, pResMail->m_szTitle, MAX_MAIL_TITLE_LEN);
        StrCpy(rstMailData.m_szContent, pResMail->m_szContent, MAX_MAIL_CONTENT_LEN);
        rstMailData.m_ullFromUin = 0;
        rstMailData.m_bAttachmentCount = pResOutlets->m_bPropsCount;
        for (int j = 0; j < pResOutlets->m_bPropsCount; j++)
        {
            rstMailData.m_astAttachmentList[j].m_bItemType = pResOutlets->m_szPropType[j];
            rstMailData.m_astAttachmentList[j].m_dwItemId = pResOutlets->m_propId[j];
            rstMailData.m_astAttachmentList[j].m_iValueChg = pResOutlets->m_propNum[j];
        }

        do
        {
            rstCardInfo.m_ullLastAwardTime += SECONDS_OF_DAY;
            rstMailData.m_ullTimeStampMs = rstCardInfo.m_ullLastAwardTime;
            ZoneSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
        }while(rstCardInfo.m_ullLastAwardTime < ullEndTime);
    }
}

bool Pay::_CardReward(PlayerData* pstData, uint32_t dwId)
{
    RESOUTLETS *pResOutlets = CGameDataMgr::Instance().GetResOutletsMgr().Find(dwId);
    if (!pResOutlets)
    {
        LOGERR("Player<%lu> ResOutlets<%u> is NULL", pstData->m_ullUin, dwId);
        return false;
    }

    DT_ROLE_PAY_INFO& rstPayInfo = pstData->GetMiscInfo().m_stPayInfo;
    DT_CARD_INFO Temp;
    //需要增加时间
    Temp.m_dwId = dwId;
    Temp.m_ullEndTime = 0;
    Temp.m_ullLastAwardTime = 0;
    int iEqual = 0;
    int iIndex = MyBSearch(&Temp, rstPayInfo.m_astCardList, rstPayInfo.m_bCardCnt, sizeof(DT_CARD_INFO), &iEqual, CardCmp);
    if (!iEqual)
    {
        size_t nmemb = (size_t)rstPayInfo.m_bCardCnt;
        if (!MyBInsertIndex(&Temp, rstPayInfo.m_astCardList, &nmemb, sizeof(DT_CARD_INFO), 1, &iIndex, CardCmp))
        {
            LOGERR("Player<%lu> Insert Card<%u> failed", pstData->m_ullUin, dwId);
            return false;
        }
        rstPayInfo.m_bCardCnt = nmemb;
    }

    uint64_t ullCurTime = CGameTime::Instance().GetBeginOfDay();
    DT_CARD_INFO& rstCardInfo = rstPayInfo.m_astCardList[iIndex];
    if (rstCardInfo.m_ullEndTime < ullCurTime)
    {
        rstCardInfo.m_ullLastAwardTime = ullCurTime - SECONDS_OF_DAY;
        rstCardInfo.m_ullEndTime = rstCardInfo.m_ullLastAwardTime + pResOutlets->m_dwDuration;
        //购买后立即获得第一天的奖励
        this->CheckCardReward(pstData);
    }
    else
    {
        rstCardInfo.m_ullEndTime += pResOutlets->m_dwDuration;
    }

    return true;
}

bool Pay::_GiftBagReward(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncInfo, uint32_t dwId)
{
    RESOUTLETS *pResOutlets = CGameDataMgr::Instance().GetResOutletsMgr().Find(dwId);
    if (!pResOutlets)
    {
        LOGERR("Player<%lu> ResOutlets<%u> is NULL", pstData->m_ullUin, dwId);
        return false;
    }

    DT_ROLE_PAY_INFO& rstPayInfo = pstData->GetMiscInfo().m_stPayInfo;
    DT_GIFTBAG_INFO Temp;
    Temp.m_dwId = pResOutlets->m_dwId;
	int iEqual = 0;
	int iIndex = MyBSearch(&Temp, rstPayInfo.m_astGiftBagList, rstPayInfo.m_bGiftBagListCnt, sizeof(DT_GIFTBAG_INFO), &iEqual, GiftBagCmp);
	if (!iEqual)
	{
        return false;
	}
    else
    {
        rstPayInfo.m_astGiftBagList[iIndex].m_bBuyTimes++;
    }

    if (pResOutlets->m_bType == 4)
    {
        return this->_CardReward(pstData, dwId);
    }

    for (int i = 0; i < pResOutlets->m_bPropsCount; i++)
    {
        Item::Instance().RewardItem(pstData, pResOutlets->m_szPropType[i], pResOutlets->m_propId[i], pResOutlets->m_propNum[i],
                                    rstSyncInfo, METHOD_OUTLETS_BUYING);
    }

    return true;
}

bool Pay::CheckGiftBag(PlayerData * pstData, uint32_t dwPayId)
{
    RESPAY* pResPay = CGameDataMgr::Instance().GetResPayMgr().Find(dwPayId);
    if (!pResPay)
    {
        LOGERR("Player<%lu> ResPay<%u> is NULL", pstData->m_ullUin, dwPayId);
        return false;
    }

    RESOUTLETS* pResOutlets = CGameDataMgr::Instance().GetResOutletsMgr().Find(pResPay->m_dwParam);
    if (!pResOutlets)
    {
        LOGERR("Player<%lu> ResOutlets<%u> is NULL", pstData->m_ullUin, pResPay->m_dwParam);
        return false;
    }

    RESTIMECYCLE* poResTimeCycle = CGameDataMgr::Instance().GetResTimeCycleMgr().Find(pResOutlets->m_dwTimeCycle);
    if (!poResTimeCycle)
    {
        LOGERR("Player<%lu> ResTimeCycle<%u> is NULL", pstData->m_ullUin, pResOutlets->m_dwTimeCycle);
        return false;
    }

    //检查是否在开启时间内
    uint64_t ullStartTime = 0;
    uint64_t ullEndTime = 0;
    int iRet = TimeCycle::Instance().CheckTime(pResOutlets->m_dwTimeCycle, 0, &ullStartTime, &ullEndTime);
    if (iRet != ERR_NONE)
    {
        LOGERR("Player<%lu> ResOutlets<%u> TimeCycle<%u> is not Open", pstData->m_ullUin, pResOutlets->m_dwId, pResOutlets->m_dwTimeCycle);
        return false;
    }

    //计算刷新时间
    uint64_t ullRefreshTime = ullStartTime;
    if (pResOutlets->m_dwRefreshTime != 0)
    {
        uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();
        ullRefreshTime = ullStartTime +  (ullCurTime - ullStartTime) / pResOutlets->m_dwRefreshTime * pResOutlets->m_dwRefreshTime;
    }

    //检查礼包信息
    DT_ROLE_PAY_INFO& rstPayInfo = pstData->GetMiscInfo().m_stPayInfo;
    DT_GIFTBAG_INFO Temp;
    Temp.m_dwId = pResOutlets->m_dwId;
    Temp.m_bBuyTimes = 0;
    Temp.m_ullLastUpdateTime = 0;
	int iEqual = 0;
	int iIndex = MyBSearch(&Temp, rstPayInfo.m_astGiftBagList, rstPayInfo.m_bGiftBagListCnt, sizeof(DT_GIFTBAG_INFO), &iEqual, GiftBagCmp);
	if (!iEqual)
	{
        size_t nmemb = (size_t)rstPayInfo.m_bGiftBagListCnt;
        if (!MyBInsertIndex(&Temp, rstPayInfo.m_astGiftBagList, &nmemb, sizeof(DT_GIFTBAG_INFO), 1, &iIndex, GiftBagCmp))
        {
            LOGERR("Player<%lu> Insert GiftBag<%u> failed", pstData->m_ullUin, pResOutlets->m_dwId);
            return false;
        }
        rstPayInfo.m_bGiftBagListCnt = nmemb;
	}

    //检查是否已经刷新
    DT_GIFTBAG_INFO& rstGiftBag = rstPayInfo.m_astGiftBagList[iIndex];
    if (pResOutlets->m_bType != 3 && rstGiftBag.m_ullLastUpdateTime < ullRefreshTime)
    {
        rstGiftBag.m_ullLastUpdateTime =  ullRefreshTime;
        rstGiftBag.m_bBuyTimes = 0;
    }

    //检查是否超过购买限制
    if (rstGiftBag.m_bBuyTimes >= pResOutlets->m_dwLimitCount)
    {
        LOGERR("Player<%lu> GiftBag<%u> buy exceed limit", pstData->m_ullUin, pResOutlets->m_dwId);
        return false;
    }

    return true;
}

void Pay::GetFirstPayAward(PlayerData* pstData, CS_PKG_PAY_GET_AWARD_REQ& rstPkgReq)
{
    int iRet = ERR_NONE   ;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PAY_GET_AWARD_RSP;
    SC_PKG_PAY_GET_AWARD_RSP& rstPkgRsp = m_stScPkg.m_stBody.m_stGetPayAwardRsp;

    rstPkgRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    DT_ROLE_PAY_INFO& rstPayInfo = pstData->GetMiscInfo().m_stPayInfo;
    do
    {
        if (COMMON_AWARD_STATE_AVAILABLE != rstPayInfo.m_bFirstPayAwardState)
        {
            iRet = ERR_NOT_SATISFY_COND;
            LOGERR("Uin<%lu> cant get the first pay award. AwardState<%hhu> ", pstData->m_ullUin, rstPayInfo.m_bFirstPayAwardState);
            break;
        }
        ;
        RESFIRSTPAYBAG* poResFirstPayBag = CGameDataMgr::Instance().GetResFirstPayBagMgr().Find(FIRST_PAY_BAG_ID);
        if (!poResFirstPayBag)
        {
            iRet = ERR_SYS;
            LOGERR("Uin<%lu> cant get the FirstPayAward. RESFIRSTPAYBAG is NULL, Id<%d>", pstData->m_ullUin, FIRST_PAY_BAG_ID);
            break;
        }
        for (int i = 0; i < poResFirstPayBag->m_bArraySize; i++)
        {
            Item::Instance().RewardItem(pstData, poResFirstPayBag->m_szType[i], poResFirstPayBag->m_propsId[i] ,
                poResFirstPayBag->m_propsNum[i], rstPkgRsp.m_stSyncItemInfo, METHOD_PAY_FIRST_PAY_AWARD);
        }
        rstPayInfo.m_bFirstPayAwardState = COMMON_AWARD_STATE_DRAWED;
        LOGRUN("Uin<%lu> get FirstPayAward ok! ", pstData->m_ullUin);
    } while (0);

    rstPkgRsp.m_nErrNo = iRet;
    rstPkgRsp.m_stPayInfo = rstPayInfo;
    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
}

void Pay::GetActTotalPayAward(PlayerData* pstData, CS_PKG_PAY_GET_AWARD_REQ& rstPkgReq)
{
    uint8_t bAwardId = (uint8_t) rstPkgReq.m_dwGetTypePara2;
    uint32_t dwActId = rstPkgReq.m_dwGetTypePara;
    uint8_t bIndex = bAwardId % 100;
    DT_ROLE_PAY_INFO& rstPayInfo = pstData->GetMiscInfo().m_stPayInfo;
    RESTOTALPAYBAG *poResTotalPayBag = CGameDataMgr::Instance().GetResTotalPayBagMgr().Find(bAwardId);

    int iRet = ERR_NONE;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PAY_GET_AWARD_RSP;
    SC_PKG_PAY_GET_AWARD_RSP& rstPkgRsp = m_stScPkg.m_stBody.m_stGetPayAwardRsp;

    rstPkgRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    do
    {
        if (NULL == poResTotalPayBag || bIndex >= MAX_PAY_ACT_TOTAL_AWARD)
        {
            iRet = ERR_SYS;
            LOGERR("Uin<%lu> ActId<%u> GetActTotalPayAward poResTotalPayBag<id %hhu> is NULL ", pstData->m_ullUin,
               dwActId, bAwardId);
            break;
        }

        if ( !(rstPayInfo.m_dwActPayTotal >= poResTotalPayBag->m_dwTotalNum &&
            rstPayInfo.m_szActPayTotalAwardList[bIndex] == COMMON_DO_SOME_STATE_NONE))
        {
            iRet = ERR_NOT_SATISFY_COND;
            LOGERR("Uin<%lu> ActId<%u> GetActTotalPayAward AwardId<%hhu> OwnTotalActPayNum<%u> RequireTotalPayNum<%u> the condi limit",
                pstData->m_ullUin, dwActId, bAwardId, rstPayInfo.m_dwActPayTotal, poResTotalPayBag->m_dwTotalNum);
            break;
        }

        for (size_t i = 0; i < poResTotalPayBag->m_bArraySize; i++)
        {
            Item::Instance().RewardItem(pstData, poResTotalPayBag->m_szPropsType[i], poResTotalPayBag->m_propsId[i],
                poResTotalPayBag->m_propsNum[i], rstPkgRsp.m_stSyncItemInfo, METHOD_PAY_ACT_TOTAL_AWARD);
        }
        rstPayInfo.m_szActPayTotalAwardList[bIndex] = COMMON_DO_SOME_STATE_FINISHED;
        LOGRUN("Uin<%lu> get ActTotalAward AwardId<%hhu>  ok! ", pstData->m_ullUin, bAwardId);
    } while (0);
    rstPkgRsp.m_nErrNo = iRet;
    rstPkgRsp.m_stPayInfo = rstPayInfo;
    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
}

void Pay::GetTotalConsumeAward(PlayerData* pstData, CS_PKG_PAY_GET_AWARD_REQ& rstPkgReq)
{
    uint8_t bAwardId = (uint8_t) rstPkgReq.m_dwGetTypePara2;
    uint32_t dwActId = rstPkgReq.m_dwGetTypePara;
    DT_ROLE_PAY_INFO& rstPayInfo = pstData->GetMiscInfo().m_stPayInfo;
    RESTOTALPAYBAG *poResTotalPayBag = CGameDataMgr::Instance().GetResTotalPayBagMgr().Find(bAwardId);
    uint8_t bIndex = bAwardId % 100;

    int iRet = ERR_NONE;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PAY_GET_AWARD_RSP;
    SC_PKG_PAY_GET_AWARD_RSP& rstPkgRsp = m_stScPkg.m_stBody.m_stGetPayAwardRsp;

    rstPkgRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    do
    {
        if (NULL == poResTotalPayBag ||bIndex >= MAX_PAY_ACT_TOTAL_AWARD)
        {
            iRet = ERR_SYS;
            LOGERR("Uin<%lu> ActId<%u> GetActTotalPayAward poResTotalPayBag<id %hhu> is NULL ", pstData->m_ullUin,
               dwActId, bAwardId);
            break;
        }

        if ( !(rstPayInfo.m_dwActConsumeTotal >= poResTotalPayBag->m_dwTotalNum &&
            rstPayInfo.m_szActConsumeTotalAwardList[bIndex] == COMMON_DO_SOME_STATE_NONE))
        {
            iRet = ERR_NOT_SATISFY_COND;
            LOGERR("Uin<%lu> ActId<%u> GetActTotalPayAward AwardId<%hhu> OwnTotalActPayNum<%u> RequireTotalPayNum<%u> the condi limit",
                pstData->m_ullUin, dwActId, bAwardId, rstPayInfo.m_dwActPayTotal, poResTotalPayBag->m_dwTotalNum);
            break;
        }

        for (size_t i = 0; i < poResTotalPayBag->m_bArraySize; i++)
        {
            Item::Instance().RewardItem(pstData, poResTotalPayBag->m_szPropsType[i], poResTotalPayBag->m_propsId[i],
                poResTotalPayBag->m_propsNum[i], rstPkgRsp.m_stSyncItemInfo, METHOD_PAY_ACT_TOTAL_AWARD);
        }
        rstPayInfo.m_szActConsumeTotalAwardList[bIndex] = COMMON_DO_SOME_STATE_FINISHED;
        LOGRUN("Uin<%lu> get ActTotalAward AwardId<%hhu>  ok! ", pstData->m_ullUin, bAwardId);
    } while (0);
    rstPkgRsp.m_nErrNo = iRet;
    rstPkgRsp.m_stPayInfo = rstPayInfo;
    LOGWARN("WriteTotalConsumeAward");
    ZoneLog::Instance().WritePromotionalActivityLog(pstData, bIndex, 0, 0, 0, 0);
    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
}

void Pay::GetFDayPayAward(PlayerData* pstData, CS_PKG_PAY_GET_AWARD_REQ& rstPkgReq)
{
    int iRet = ERR_NONE;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PAY_GET_AWARD_RSP;
    SC_PKG_PAY_GET_AWARD_RSP& rstPkgRsp = m_stScPkg.m_stBody.m_stGetPayAwardRsp;

    rstPkgRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    DT_ROLE_PAY_INFO& rstPayInfo = pstData->GetMiscInfo().m_stPayInfo;

    do
    {
        if (COMMON_AWARD_STATE_AVAILABLE != rstPayInfo.m_bFDayPayAwardState)
        {
            iRet = ERR_NOT_SATISFY_COND;
            LOGERR("Uin<%lu> cant get the first pay award. AwardState<%hhu> ", pstData->m_ullUin, rstPayInfo.m_bFDayPayAwardState);
            break;
        }
        ;
        RESFIRSTPAYBAG* poResFirstPayBag = CGameDataMgr::Instance().GetResFirstPayBagMgr().Find(FIRST_DAY_PAY_BAG_ID);
        if (!poResFirstPayBag)
        {
            iRet = ERR_SYS;
            LOGERR("Uin<%lu> cant get the FirstPayAward. RESFIRSTPAYBAG is NULL, Id<%d>", pstData->m_ullUin, FIRST_DAY_PAY_BAG_ID);
            break;
        }
        for (int i = 0; i < poResFirstPayBag->m_bArraySize; i++)
        {
            Item::Instance().RewardItem(pstData, poResFirstPayBag->m_szType[i], poResFirstPayBag->m_propsId[i] ,
                poResFirstPayBag->m_propsNum[i], rstPkgRsp.m_stSyncItemInfo, METHOD_PAY_FDAY_PAY_AWARD);
        }
        rstPayInfo.m_bFDayPayAwardState = COMMON_AWARD_STATE_DRAWED;
        LOGRUN("Uin<%lu> get FDayPayAward ok! ", pstData->m_ullUin);
    } while (0);

    rstPkgRsp.m_nErrNo = iRet;
    rstPkgRsp.m_stPayInfo = rstPayInfo;
    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
}

void Pay::GetDiscountProps(PlayerData* pstData, CS_PKG_DISCOUNT_PROPS_GET_REQ& rstPkgReq, SC_PKG_DISCOUNT_PROPS_GET_RSP& rstPkgRsp)
{
    if (rstPkgReq.m_bDay <= 0 || rstPkgReq.m_bDay > MAX_DISCOUNT_ACTIVITY_DAY)
    {
        rstPkgRsp.m_nErrNo = ERR_NOT_SATISFY_COND;
        return;
    }

    DT_PAY_ACTIVITY_INFO& rstPayActivityInfo = pstData->GetMiscInfo().m_stPayActivityInfo;

    int iCursor = MAX_DISCOUNT_PROPS_NUM * (rstPkgReq.m_bDay-1);

    rstPkgRsp.m_bCount = MAX_DISCOUNT_PROPS_NUM;
    for (int i=0; i<MAX_DISCOUNT_PROPS_NUM; i++)
    {
        rstPkgRsp.m_astPropsInfo[i] = m_astDiscountPropsList[iCursor + i];
        rstPkgRsp.m_astPropsInfo[i].m_bState = rstPayActivityInfo.m_szDiscountActivityInfo[iCursor + i];
    }

    rstPkgRsp.m_nErrNo = ERR_NONE;

    return;
}

void Pay::BuyDiscountProps(PlayerData* pstData, CS_PKG_DISCOUNT_PROPS_BUY_REQ& rstPkgReq, SC_PKG_DISCOUNT_PROPS_BUY_RSP& rstPkgRsp)
{
    if (rstPkgReq.m_bDay <= 0 || rstPkgReq.m_bDay > MAX_DISCOUNT_ACTIVITY_DAY
      || rstPkgReq.m_bPropsId < 1 ||rstPkgReq.m_bPropsId > MAX_DISCOUNT_PROPS_NUM)
    {
        rstPkgRsp.m_nErrNo = ERR_NOT_SATISFY_COND;
        return;
    }

    if (rstPkgReq.m_bDay > SvrTime::Instance().GetOpenSvrDay())
    {
        rstPkgRsp.m_nErrNo = ERR_NOT_SATISFY_COND;
        return;
    }

    int iCursor = MAX_DISCOUNT_PROPS_NUM * (rstPkgReq.m_bDay - 1) + rstPkgReq.m_bPropsId - 1;

    //判断是否已购买
    DT_PAY_ACTIVITY_INFO& rstPayActivityInfo = pstData->GetMiscInfo().m_stPayActivityInfo;
    if (rstPayActivityInfo.m_szDiscountActivityInfo[iCursor] != 0)
    {
        rstPkgRsp.m_nErrNo = ERR_NOT_SATISFY_COND;
        return;
    }

    //判断是否已售完
    DT_DISCOUNT_PROPS_INFO& rstPropsInfo = m_astDiscountPropsList[iCursor];
    if (rstPropsInfo.m_dwRemainNum <= 0)
    {
        rstPkgRsp.m_nErrNo = ERR_NOT_SATISFY_COND;
        return;
    }

    //判断VIP等级是否够
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
    if (rstMajestyInfo.m_bVipLv < rstPropsInfo.m_bVip)
    {
        rstPkgRsp.m_nErrNo = ERR_LEVEL_LIMIT;
        return;
    }

    //判断钻石是否足够
    int iPrice = rstPropsInfo.m_dwPrice*rstPropsInfo.m_bDiscount/100;
    if (!Consume::Instance().IsEnoughDiamond(pstData, iPrice))
    {
        rstPkgRsp.m_nErrNo = ERR_NOT_ENOUGH_DIAMOND;
        return;
    }

    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -iPrice, rstPkgRsp.m_stSyncItemInfo, METHOD_DISCOUNT_BUYING);

    rstPayActivityInfo.m_szDiscountActivityInfo[iCursor] = 1;
    Item::Instance().RewardItem(pstData, rstPropsInfo.m_bType, rstPropsInfo.m_dwId, rstPropsInfo.m_dwNum, rstPkgRsp.m_stSyncItemInfo, METHOD_DISCOUNT_BUYING);
    rstPropsInfo.m_dwRemainNum--;

    rstPkgRsp.m_nErrNo = ERR_NONE;

    DT_ITEM_SHOW stTempItem = { 0 };
    stTempItem.m_bItemType = ITEM_TYPE_PROPS;
    stTempItem.m_dwItemId = rstPropsInfo.m_dwId;
    stTempItem.m_dwItemNum = rstPropsInfo.m_dwNum;
    stTempItem.m_bPriceType = 6;
    stTempItem.m_dwPriceValue = iPrice;

    ZoneLog::Instance().WriteItemPurchaseLog(pstData, stTempItem, METHOD_DISCOUNT_BUYING);

    return;
}


void Pay::_SettleLiReward(PlayerData* pstData)
{
    pstData->GetMiscInfo().m_stPayActivityInfo.m_bLiRewardInfo = 1;

    uint32_t dwLi = pstData->GetMajestyInfo().m_dwHighestLi;
    int iEqual = 0;
    int iIndex = MyBSearch(&dwLi, m_LiRewardSec, m_bLiRewardSecCnt, sizeof(uint32_t), &iEqual, LiCmp);
    if (!iEqual)
    {
        iIndex -= 1;
    }

    //TODEL
    LOGERR("Player(%s) Uin(%lu) SettleLiReward, Li(%u), RewardIndex(%d)",
            pstData->GetRoleBaseInfo().m_szRoleName, pstData->GetRoleBaseInfo().m_ullUin, dwLi, iIndex);

    if (iIndex<0 || iIndex>=m_bLiRewardSecCnt)
    {
        return;
    }

    RESACTIVITYFORCEFORCEREWARD* pResLiReward =    CGameDataMgr::Instance().GetResLiRewardMgr().GetResByPos(iIndex);
    assert(pResLiReward);

    RESPRIMAIL* poResPriMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(LI_REWARD_MAIL_ID);
    assert(pResLiReward);

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
    SS_PKG_MAIL_ADD_REQ& rstMailAddReq = m_stSsPkg.m_stBody.m_stMailAddReq;
    rstMailAddReq.m_nUinCount = 1;
    rstMailAddReq.m_UinList[0] = pstData->GetRoleBaseInfo().m_ullUin;
    DT_MAIL_DATA& rstMailData = rstMailAddReq.m_stMailData;
    rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
    rstMailData.m_bState = MAIL_STATE_UNOPENED;
    StrCpy(rstMailData.m_szTitle, poResPriMail->m_szTitle, MAX_MAIL_TITLE_LEN);
    snprintf(rstMailData.m_szContent, MAX_MAIL_CONTENT_LEN, poResPriMail->m_szContent, pResLiReward->m_dwForceLower);
    rstMailData.m_ullFromUin = 0;
    rstMailData.m_bAttachmentCount = pResLiReward->m_bRewardCount;
    rstMailData.m_ullTimeStampMs = CGameTime::Instance().GetCurrSecond();

    for (int j = 0; j < rstMailData.m_bAttachmentCount; j++)
    {
        rstMailData.m_astAttachmentList[j].m_bItemType = pResLiReward->m_szRewardTypeList[j];
        rstMailData.m_astAttachmentList[j].m_dwItemId = pResLiReward->m_rewardIdList[j];
        rstMailData.m_astAttachmentList[j].m_iValueChg = pResLiReward->m_rewardNumList[j];
    }

    ZoneLog::Instance().WritePromotionalActivityLog(pstData, 0, pResLiReward->m_dwId, 0, 0, 0);

    ZoneSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
}

void Pay::SettleLiRank()
{
    m_bSettleFlag = 1;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_LI_RANK_SETTLE_NTF;
    ZoneSvrMsgLayer::Instance().SendToRankSvr(m_stSsPkg);
}

