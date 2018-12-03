#include "strutil.h"
#include "SdkDMMgr.h"
#include "../../framework/ZoneSvrMsgLayer.h"
#include "sys/GameTime.h"
void SdkDMMgr::TDataSendOrder(DT_TDATA_ODER_INFO& rstInfo)
{
    bzero(&m_stSsPkg, sizeof(m_stSsPkg));
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_SDKDM_TDATA_SEND_ORDER_NTF;
    memcpy(&m_stSsPkg.m_stBody.m_stSdkDMTDataSendOrderNtf.m_stOrderMsg, &rstInfo, sizeof(rstInfo));
    ZoneSvrMsgLayer::Instance().SendToSdkDMSvr(m_stSsPkg);
}


void SdkDMMgr::TestSend(uint64_t ullNum)
{

    DT_TDATA_ODER_INFO stInfo = {0};

    uint64_t ulTime = CGameTime::Instance().GetCurrTimeMs(); 
    StrCpy(stInfo.m_szStatus, "success", 10);
    StrCpy(stInfo.m_szOS, "android", 10);
    
    stInfo.m_fCurrencyAmount = 888;
    StrCpy(stInfo.m_szCurrencyType, "CNY", 10);
    stInfo.m_fVirtualCurrencyAmount = 8880;
    stInfo.m_ullChargeTime = ulTime;
    StrCpy(stInfo.m_szIapID, "Iap_001", 10);
    StrCpy(stInfo.m_szPaymentType, "zhifubao", 10);
    StrCpy(stInfo.m_szGameServer, "Sid_001", 10);
    StrCpy(stInfo.m_szGameVersion, "1.0.0", 10);
    stInfo.m_iLevel = 18;
    StrCpy(stInfo.m_szMission, "Task_001", 10);
    StrCpy(stInfo.m_szPartner, "Xiyou", 10);

    for (size_t i = 0; i < ullNum; i++)
    {
        snprintf(stInfo.m_szMsgID, 64, "MsgIdTest_%lu_%lu",  ulTime, i);
        snprintf(stInfo.m_szOrderID, 64, "OrderIdTest_%lu_%lu", ulTime, i);
        snprintf(stInfo.m_szAccountID, 64, "player_%010lu", i);
        TDataSendOrder(stInfo);
        stInfo.m_szMsgID[0] = '\0';
        stInfo.m_szOrderID[0] = '\0';
        stInfo.m_szAccountID[0] = '\0';
    }
}






