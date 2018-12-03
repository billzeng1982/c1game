#include "ZoneLog.h"
#include "./player/PlayerData.h"
#include "../../../common/sys/GameTime.h"
#include "dwlog_svr.h"
#include "common_proto.h"
#include "./player/PlayerMgr.h"
#include "SvrTime.h"
#include "../../../common/utils/oi_misc.h"
#include "../../../common/log/LogMacros.h"
#include "../../../common/utils/strutil.h"

using namespace DWLOG;

bool ZoneLog::Init(char *pszConfPath, char *pszCltMetaFile, char *pszSvrMetaFile)
{
    if (!CltLog.Init(BUFFER_SIZE, pszConfPath, "ZoneSvr", pszCltMetaFile))
    {
        return false;
    }

    if (!SvrLog.Init(BUFFER_SIZE, pszConfPath, "ZoneSvr", pszSvrMetaFile))
    {
        return false;
    }

    return true;
}

int ZoneLog::WriteCltLog(const char *pszTableName, const char *pstLog, int iLen)
{
    return CltLog.Write(pszTableName, pstLog, iLen);
}

int ZoneLog::WriteSvrLog(const char *pszTableName, const char *pstLog, int iLen)
{
    return SvrLog.Write(pszTableName, pstLog, iLen);
}

void ZoneLog::WriteLoginLog(Player * poPlayer)
{
    if (poPlayer == NULL)
    {
        LOGERR("player is NULL");
        return;
    }

    AccountLoginLog stTempLoginLog;
    stTempLoginLog.m_ullPlayerID = poPlayer->GetUin();
    StrCpy(stTempLoginLog.m_szAccountName, poPlayer->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stTempLoginLog.m_szRoleName, poPlayer->GetRoleName(), DWLOG::MAX_NAME_LENGTH);
    if (strlen(poPlayer->GetSubChannelName()) == 0)
    {
        StrCpy(stTempLoginLog.m_szChannelName, "NoChannel", DWLOG::MAX_NAME_LENGTH);
    }
    else
    {
        StrCpy(stTempLoginLog.m_szChannelName, poPlayer->GetSubChannelName(), DWLOG::MAX_NAME_LENGTH);
    }
    StrCpy(stTempLoginLog.m_szPhoneType, poPlayer->GetPhoneType(), DWLOG::MAX_NAME_LENGTH);
    stTempLoginLog.m_wLevel = poPlayer->GetPlayerData().GetMajestyInfo().m_wLevel;
    time_t LoginTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&LoginTime);
    if (pTime)
    {
        StrCpy(stTempLoginLog.m_szLoginTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    DT_ROLE_BASE_INFO stBaseInfo = poPlayer->GetPlayerData().GetRoleBaseInfo();
    _GetDaySinceReg(stBaseInfo, &stTempLoginLog.m_wDaySinceReg);


    stTempLoginLog.m_wDaySinceSvrOpen = SvrTime::Instance().GetOpenSvrDay();
    WriteSvrLog("AccountLoginLog", (char*)&stTempLoginLog, sizeof(stTempLoginLog));
}

void ZoneLog::WriteLogoutLog(Player* poPlayer, int iLogoutReason)
{
    if (NULL == poPlayer)
    {
        LOGERR("player is null");
        return;
    }

    // write log
    AccountLogoutLog stTempLogoutLog;
    stTempLogoutLog.m_ullPlayerID = poPlayer->GetUin();
    StrCpy(stTempLogoutLog.m_szAccountName, poPlayer->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stTempLogoutLog.m_szRoleName, poPlayer->GetRoleName(), DWLOG::MAX_NAME_LENGTH);
    if (strlen(poPlayer->GetSubChannelName()) == 0)
    {
        StrCpy(stTempLogoutLog.m_szChannelName, "NoChannel", DWLOG::MAX_NAME_LENGTH);
    }
    else
    {
        StrCpy(stTempLogoutLog.m_szChannelName, poPlayer->GetSubChannelName(), DWLOG::MAX_NAME_LENGTH);
    }
    StrCpy(stTempLogoutLog.m_szPhoneType, poPlayer->GetPhoneType(), DWLOG::MAX_NAME_LENGTH);
    stTempLogoutLog.m_wLevel = poPlayer->GetPlayerData().GetMajestyInfo().m_wLevel;

    time_t startTime = poPlayer->GetPlayerData().GetRoleBaseInfo().m_llLastLoginTime;
    time_t finishTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&finishTime);

    if (pTime)
    {
        StrCpy(stTempLogoutLog.m_szLogoutTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    stTempLogoutLog.m_wLogoutReason = iLogoutReason;
    pTime = DateTimeStr(&startTime);
    if (pTime)
    {
        StrCpy(stTempLogoutLog.m_szLoginTime, pTime, DWLOG::MAX_DAY_TIME_LENGTH);
    }

    DT_ROLE_BASE_INFO stBaseInfo = poPlayer->GetPlayerData().GetRoleBaseInfo();
    _GetDaySinceReg(stBaseInfo, &stTempLogoutLog.m_wDaySinceReg);

    stTempLogoutLog.m_wDaySinceSvrOpen = SvrTime::Instance().GetOpenSvrDay();
    WriteSvrLog("AccountLogoutLog", (char*)&stTempLogoutLog, sizeof(stTempLogoutLog));
}

void ZoneLog::WriteCreateNewLog(Player* poPlayer)
{
    if (NULL == poPlayer)
    {
        LOGERR("player is null");
        return;
    }

    CreateNewLog stCreateNewLog;
    stCreateNewLog.m_ullPlayerID = poPlayer->GetUin();
    StrCpy(stCreateNewLog.m_szAccountName, poPlayer->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);
    if (pTime)
    {
        StrCpy(stCreateNewLog.m_szCreateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    WriteSvrLog("CreateNewLog", (char*)&stCreateNewLog, sizeof(stCreateNewLog));
}

void ZoneLog::WriteGoldLog(PlayerData* pstData, uint32_t dwChgValue, uint8_t bChgType, uint64_t dwCurValue, uint32_t dwApproach)
{
    GoldLog stTempGoldLog;
    stTempGoldLog.m_ullPlayerID = pstData->m_pOwner->GetUin();
    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);

    if (pTime)
    {
        StrCpy(stTempGoldLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    StrCpy(stTempGoldLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stTempGoldLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stTempGoldLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;

    stTempGoldLog.m_dwChgValue = dwChgValue;
    stTempGoldLog.m_chChgType = bChgType;
    stTempGoldLog.m_dwCurValue = dwChgValue;
    stTempGoldLog.m_wApproach = dwApproach;
    _GetDaySinceReg(pstData->GetRoleBaseInfo(), &stTempGoldLog.m_wDaySinceReg);
    stTempGoldLog.m_wDaySinceSvrOpen = SvrTime::Instance().GetOpenSvrDay();

    WriteSvrLog("GoldLog", (char*)&stTempGoldLog, sizeof(stTempGoldLog));
}

void ZoneLog::WriteDiamondLog(PlayerData* pstData, uint32_t dwChgValue, uint8_t bChgType, uint64_t dwCurValue, uint32_t dwApproach)
{
    DiamondLog stTempDiamondLog;
    stTempDiamondLog.m_ullPlayerID = pstData->m_pOwner->GetUin();
    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);

    if (pTime)
    {
        StrCpy(stTempDiamondLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    StrCpy(stTempDiamondLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stTempDiamondLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stTempDiamondLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;

    stTempDiamondLog.m_dwChgValue = dwChgValue;
    stTempDiamondLog.m_chChgType = bChgType;
    stTempDiamondLog.m_dwCurValue = dwCurValue;
    stTempDiamondLog.m_wApproach = dwApproach;
    _GetDaySinceReg(pstData->GetRoleBaseInfo(), &stTempDiamondLog.m_wDaySinceReg);
    stTempDiamondLog.m_wDaySinceSvrOpen = SvrTime::Instance().GetOpenSvrDay();

    WriteSvrLog("DiamondLog", (char*)&stTempDiamondLog, sizeof(stTempDiamondLog));
}

void ZoneLog::WriteYuanLog(PlayerData* pstData, uint32_t dwChgValue, uint8_t bChgType, uint64_t dwCurValue, uint32_t dwApproach)
{
    YuanLog stTempYuanLog;
    stTempYuanLog.m_ullPlayerID = pstData->m_pOwner->GetUin();
    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);

    if (pTime)
    {
        StrCpy(stTempYuanLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    DT_ROLE_BASE_INFO stBaseInfo = pstData->GetRoleBaseInfo();
    _GetDaySinceReg(stBaseInfo, &stTempYuanLog.m_wDaySinceReg);

    StrCpy(stTempYuanLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stTempYuanLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stTempYuanLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;

    stTempYuanLog.m_dwChgValue = dwChgValue;
    stTempYuanLog.m_chChgType = bChgType;
    stTempYuanLog.m_dwCurValue = dwCurValue;
    stTempYuanLog.m_wApproach = dwApproach;

    WriteSvrLog("YuanLog", (char*)&stTempYuanLog, sizeof(stTempYuanLog));
}

void ZoneLog::WriteRebornLog(Player* poPlayer, uint32_t dwID, uint8_t bLevel, uint8_t bType)
{
    if (NULL == poPlayer)
    {
        LOGERR("player is null");
        return;
    }

    RebornLog stRebornLog;
    stRebornLog.m_ullPlayerID = poPlayer->GetUin();
    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);

    if (pTime)
    {
        StrCpy(stRebornLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    StrCpy(stRebornLog.m_szAccountName, poPlayer->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stRebornLog.m_szRoleName, poPlayer->GetRoleName(), DWLOG::MAX_NAME_LENGTH);
    stRebornLog.m_wLevel = poPlayer->GetPlayerData().GetMajestyInfo().m_wLevel;
    stRebornLog.m_dwGeneralID = dwID;
    stRebornLog.m_chGeneralLevel = bLevel;
    stRebornLog.m_chRebornType = bType;

    WriteSvrLog("RebornLog", (char*)&stRebornLog, sizeof(stRebornLog));
}

void ZoneLog::WriteGeneralLog
(PlayerData* pstData, uint32_t dwID, uint16_t wLevel, uint16_t wOptType, uint32_t dwOpPara1, uint32_t dwOpPara2, uint32_t dwOpPara3,
    uint8_t bGeneralPhase, uint16_t wFameHallLvl, uint16_t wGeneralStar)
{
    GeneralLog stTempLog;
    stTempLog.m_ullPlayerID = pstData->GetRoleBaseInfo().m_ullUin;
    StrCpy(stTempLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stTempLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);
    char* pHour = const_cast<char*>(string(pTime).substr(11, 2).c_str());
    if (pTime)
    {
        StrCpy(stTempLog.m_szDateTime, pTime, DWLOG::MAX_DAY_TIME_LENGTH);
        StrCpy(stTempLog.m_szCurrentHour, pHour, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    stTempLog.m_dwGeneralID = dwID;
    stTempLog.m_wLevel = wLevel;
    stTempLog.m_wOpType = wOptType;
    stTempLog.m_dwOpPara1 = dwOpPara1;
    stTempLog.m_dwOpPara2 = dwOpPara2;
    stTempLog.m_dwOpPara3 = dwOpPara3;
    stTempLog.m_chGeneralPhase = bGeneralPhase;
    stTempLog.m_wFameHallLvl = wFameHallLvl;
    stTempLog.m_wGeneralStar = wGeneralStar;

    WriteSvrLog("GeneralLog", (char*)&stTempLog, sizeof(stTempLog));
}

void ZoneLog::WriteGuildLog(PlayerData* pstData, uint16_t wOptType, uint64_t ullGuildID, const char* pszPara1, uint64_t ullPara2, uint32_t dwPara3)
{
    GuildLog stGuildLog;
    bzero(&stGuildLog, sizeof(stGuildLog));
    stGuildLog.m_ullPlayerID = pstData->GetRoleBaseInfo().m_ullUin;
    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);
    if (pTime)
    {
        StrCpy(stGuildLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    StrCpy(stGuildLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stGuildLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stGuildLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;
    stGuildLog.m_wOptType = wOptType;
    stGuildLog.m_ullGuildID = ullGuildID;
    if (NULL != pszPara1)
    {
        StrCpy(stGuildLog.m_szReservePara1, pszPara1, DWLOG::MAX_NAME_LENGTH);
    }
    stGuildLog.m_ullReservePara2 = ullPara2;
    stGuildLog.m_dwReservePara3 = dwPara3;
    WriteSvrLog("GuildLog", (char*)&stGuildLog, sizeof(stGuildLog));
}

void ZoneLog::WriteMatchLog(PlayerData* pstData, uint8_t bResult, uint16_t wMatchType)
{
    MatchLog stMatchLog;
    stMatchLog.m_ullPlayerID = pstData->GetRoleBaseInfo().m_ullUin;
    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);

    if (pTime)
    {
        StrCpy(stMatchLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
        StrCpy(stMatchLog.m_szFinishTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    time_t StartTime = CGameTime::Instance().GetCurrSecond();
    pTime = DateTimeStr(&StartTime);
    if (pTime)
    {
        StrCpy(stMatchLog.m_szStartTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }
    StrCpy(stMatchLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stMatchLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stMatchLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;
    stMatchLog.m_wRanking = pstData->GetELOInfo().m_stPvp6v6Info.m_stBaseInfo.m_bELOLvId;
    stMatchLog.m_chResult = bResult;
    stMatchLog.m_wMatchType = wMatchType;

    WriteSvrLog("MatchLog", (char*)&stMatchLog, sizeof(stMatchLog));
}

void ZoneLog::WriteLevelPassLog(PlayerData* pstData, const CS_PKG_FIGHT_PVE_SETTLE_REQ& rstCsPkgBodyReq, int iIsFirstPass)
{
    LevelPassLog stTempLog;
    stTempLog.m_dwLevelID = rstCsPkgBodyReq.m_dwFLevelID;
    stTempLog.m_ullPlayerID = pstData->m_pOwner->GetUin();
    StrCpy(stTempLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stTempLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stTempLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;
    stTempLog.m_chIsPass = rstCsPkgBodyReq.m_bIsPass;
    stTempLog.m_wStarEvalResult = rstCsPkgBodyReq.m_bStarEvalResult;
    memcpy(&stTempLog.m_dwGeneral1, rstCsPkgBodyReq.m_GeneralList, sizeof(uint32_t)*DWLOG::MAX_GENERAL_NUM);
    stTempLog.m_dwMSkillID = rstCsPkgBodyReq.m_dwMSkillID;
    stTempLog.m_chMSkillUseNum = rstCsPkgBodyReq.m_bMSkillUseNum;

    if (iIsFirstPass)
    {
        stTempLog.m_chIsPass = 2;/*第一次通关*/;
    }

    time_t finishTime = CGameTime::Instance().GetCurrSecond();
    stTempLog.m_dwTimeCost = rstCsPkgBodyReq.m_ullCostTime;/*采用游戏内时间*/
    char * pTime = DateTimeStr(&finishTime);

    if (pTime)
    {
        StrCpy(stTempLog.m_szEndTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    pTime = DateTimeStr((time_t*)&(rstCsPkgBodyReq.m_ullTimeStampStart));

    if (pTime)
    {
        StrCpy(stTempLog.m_szStartTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    DT_ROLE_BASE_INFO& rstBaseInfo = pstData->GetRoleBaseInfo();
    stTempLog.m_dwTime2AccRegister = finishTime - rstBaseInfo.m_llFirstLoginTime;
    stTempLog.m_wPlayerLevel = pstData->GetMajestyInfo().m_wLevel;
    stTempLog.m_chIsAutoBattle = rstCsPkgBodyReq.m_bIsAutoBattle;
    stTempLog.m_chIsAccelerated = rstCsPkgBodyReq.m_bIsAccelerated;
    stTempLog.m_iMaxFramePerSecond = rstCsPkgBodyReq.m_iMaxFramePerSecond;
    stTempLog.m_iMinFramePerSecond = rstCsPkgBodyReq.m_iMinFramePerSecond;
    stTempLog.m_iAverageFramePerSecond = rstCsPkgBodyReq.m_iAverageFramePerSecond;
    stTempLog.m_dwReservePara1 = 0;
    stTempLog.m_dwReservePara2 = 0;

    WriteSvrLog("LevelPassLog", (char*)&stTempLog, sizeof(stTempLog));
}

void ZoneLog::WritePubMailLog(PlayerData* pstData, uint32_t dwMailID)
{
    PubMailLog stPubMailLog;
    stPubMailLog.m_ullPlayerID = pstData->m_pOwner->GetUin();
    StrCpy(stPubMailLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stPubMailLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stPubMailLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;

    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);

    if (pTime)
    {
        StrCpy(stPubMailLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    stPubMailLog.m_dwMailID = dwMailID;

    WriteSvrLog("PubMailLog", (char*)&stPubMailLog, sizeof(stPubMailLog));
}

void ZoneLog::WriteFriendLog(PlayerData* pstData, uint8_t bType, uint64_t ullUinReceiver)
{
    if (bType == FRIEND_HANDLE_TYPE_AGREE || bType == FRIEND_HANDLE_TYPE_DELETE)
    {
        Player* poReceiverPlayer = PlayerMgr::Instance().GetPlayerByUin(ullUinReceiver);
        if (poReceiverPlayer != NULL)
        {
            FriendLog stFriendLog;
            stFriendLog.m_ullPlayerID = pstData->GetRoleBaseInfo().m_ullUin;
            StrCpy(stFriendLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
            StrCpy(stFriendLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
            stFriendLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;
            stFriendLog.m_ullFriendID = poReceiverPlayer->GetUin();
            StrCpy(stFriendLog.m_szFriendName, poReceiverPlayer->GetRoleName(), DWLOG::MAX_NAME_LENGTH);

            time_t CurTime = CGameTime::Instance().GetCurrSecond();
            char * pTime = DateTimeStr(&CurTime);
            if (pTime)
            {
                StrCpy(stFriendLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
            }
            stFriendLog.m_wOptType = bType + METHOD_FRIEND_HANDLE_TYPE_OFFSET;

            WriteSvrLog("FriendLog", (char*)&stFriendLog, sizeof(stFriendLog));
        }
    }
}

void ZoneLog::WriteGeneralGetLog(PlayerData* pstData, uint32_t dwID, uint16_t wMethod)
{
    GeneralGetLog stGeneralGetLog;
    stGeneralGetLog.m_ullPlayerID = pstData->GetRoleBaseInfo().m_ullUin;
    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);
    if (pTime)
    {
        StrCpy(stGeneralGetLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }
    StrCpy(stGeneralGetLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stGeneralGetLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stGeneralGetLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;

    stGeneralGetLog.m_dwGeneralID = dwID;
    stGeneralGetLog.m_wType = wMethod;

    WriteSvrLog("GeneralGetLog", (char*)&stGeneralGetLog, sizeof(stGeneralGetLog));
}

void ZoneLog::WriteItemPurchaseLog(PlayerData* pstData, const DT_ITEM_SHOW& rstItem, uint16_t wApproach)
{
    ItemPurchaseLog stItemPurchaseLog;
    stItemPurchaseLog.m_ullPlayerID = pstData->GetRoleBaseInfo().m_ullUin;
    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);
    char* pHour = const_cast<char*>(string(pTime).substr(11, 2).c_str());
    if (pTime)
    {
        StrCpy(stItemPurchaseLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
        StrCpy(stItemPurchaseLog.m_szCurrentHour, pHour, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    StrCpy(stItemPurchaseLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stItemPurchaseLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stItemPurchaseLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;
    stItemPurchaseLog.m_chItemType = rstItem.m_bItemType;
    stItemPurchaseLog.m_dwItemID = rstItem.m_dwItemId;
    stItemPurchaseLog.m_dwItemNum = rstItem.m_dwItemNum;
    stItemPurchaseLog.m_chPriceType = rstItem.m_bPriceType;
    stItemPurchaseLog.m_dwPriceValue = rstItem.m_dwPriceValue;
    stItemPurchaseLog.m_wPurchaseApproach = wApproach;

    LOGWARN("MoneyType:%d", stItemPurchaseLog.m_chPriceType);

    WriteSvrLog("ItemPurchaseLog", (char*)&stItemPurchaseLog, sizeof(stItemPurchaseLog));
}

void ZoneLog::WriteMSKillLog(PlayerData* pstData, uint8_t bMSID, uint8_t bMSLevel)
{
    MSKillLog stTempLog;
    stTempLog.m_ullPlayerID = pstData->GetRoleBaseInfo().m_ullUin;
    StrCpy(stTempLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stTempLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stTempLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;

    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);
    if (pTime)
    {
        StrCpy(stTempLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }
    stTempLog.m_dwMSkillID = bMSID;
    stTempLog.m_wMSkillLv = bMSLevel;

    WriteSvrLog("MSKillLog", (char*)&stTempLog, sizeof(MSKillLog));
}

void ZoneLog::WritePropLog(PlayerData* pstData, uint32_t dwID, uint16_t wPropType, uint32_t dwChgValue, uint8_t bChgType, uint32_t dwCurValue, uint32_t dwApproach)
{
    PropLog stTempPropLog;
    stTempPropLog.m_ullPlayerID = pstData->GetRoleBaseInfo().m_ullUin;
    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);
    if (pTime)
    {
        StrCpy(stTempPropLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    StrCpy(stTempPropLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stTempPropLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stTempPropLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;

    stTempPropLog.m_dwPropID = dwID;
    stTempPropLog.m_wPropType = wPropType;
    stTempPropLog.m_dwChgValue = dwChgValue;
    stTempPropLog.m_chChgType = bChgType;
    stTempPropLog.m_dwCurValue = dwCurValue;
    stTempPropLog.m_wApproach = dwApproach;
    WriteSvrLog("PropLog", (char*)&stTempPropLog, sizeof(stTempPropLog));
}

void ZoneLog::WriteAwardLog(PlayerData* pstData, uint16_t wMethode, const DT_SYNC_ITEM_INFO& rstRewardItemInfo)
{
    AwardLog stAwardLog;
    stAwardLog.m_ullPlayerID = pstData->GetRoleBaseInfo().m_ullUin;
    StrCpy(stAwardLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stAwardLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char *pTime = DateTimeStr(&CurTime);

    if (pTime)
    {
        StrCpy(stAwardLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    stAwardLog.m_wMethod = wMethode;
    stAwardLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;
    stAwardLog.m_chItemType = rstRewardItemInfo.m_astSyncItemList[0].m_bItemType;
    stAwardLog.m_dwItemID = rstRewardItemInfo.m_astSyncItemList[0].m_dwItemId;

    WriteSvrLog("AwardLog", (char*)&stAwardLog, sizeof(stAwardLog));
}

void ZoneLog::WriteTaskLog(PlayerData* pstData, uint32_t dwTaskID, const char* pszTaskType)
{
    TaskLog stTaskLog;
    stTaskLog.m_ullPlayerID = pstData->GetRoleBaseInfo().m_ullUin;
    StrCpy(stTaskLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stTaskLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stTaskLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;

    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);
    if (pTime)
    {
        StrCpy(stTaskLog.m_szDrawTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }
    stTaskLog.m_dwTaskID = dwTaskID;
    //记录任务类型
    StrCpy(stTaskLog.m_szTaskType, pszTaskType, DWLOG::MAX_NAME_LENGTH);
    DT_ROLE_BASE_INFO stBaseInfo = pstData->GetRoleBaseInfo();
    _GetDaySinceReg(stBaseInfo, &stTaskLog.m_wDaySinceReg);

    WriteSvrLog("TaskLog", (char*)&stTaskLog, sizeof(stTaskLog));
}

void ZoneLog::WriteLevelSweepLog(PlayerData* pstData, uint32_t dwLevelID, uint8_t bSweepCnt)
{
    LevelSweepLog stTempLog;
    stTempLog.m_dwLevelID = dwLevelID;
    stTempLog.m_ullPlayerID = pstData->m_pOwner->GetUin();
    StrCpy(stTempLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stTempLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stTempLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;
    time_t finishTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&finishTime);
    if (pTime)
    {
        StrCpy(stTempLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }
    stTempLog.m_wSweepCnt = bSweepCnt;
    WriteSvrLog("LevelSweepLog", (char*)&stTempLog, sizeof(stTempLog));
}

void ZoneLog::WriteTutorialStepLog(PlayerData* pstData, uint8_t bFlag, uint64_t ullStep)
{
    TutorialStepLog stTutorialStepLog;
    stTutorialStepLog.m_ullPlayerID = pstData->m_pOwner->GetUin();
    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);
    if (pTime)
    {
        StrCpy(stTutorialStepLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    StrCpy(stTutorialStepLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stTutorialStepLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stTutorialStepLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;

    //将步骤数字改写成字符串
    char szStep[32];
    char buf[32];
    bool flag = true;
    uint64_t ulltmp = ullStep;
    while (ulltmp > 0)
    {
        uint32_t mod = ulltmp % 100;
        ulltmp /= 100;
        if (flag)
        {
            sprintf(szStep, "%d", mod);
            flag = false;
        }
        else
        {
            StrCpy(buf, szStep, strlen(szStep) + 1);
            sprintf(szStep, "%d_%s", mod, buf);
        }
    }

    stTutorialStepLog.m_chFlag = bFlag;
    StrCpy(stTutorialStepLog.m_szTutorialStep, szStep, strlen(szStep) + 1);
    _GetDaySinceReg(pstData->GetRoleBaseInfo(), &stTutorialStepLog.m_wDaySinceReg);

    WriteSvrLog("TutorialStepLog", (char*)&stTutorialStepLog, sizeof(stTutorialStepLog));
}

void ZoneLog::WriteLotteryLog(PlayerData* pstData, uint8_t bDrawType, const DT_SYNC_ITEM_INFO& rstRewardItemInfo, uint8_t bCount)
{
    if (bDrawType == LOTTERY_DRAW_TYPE_DIAMOND || bDrawType == LOTTERY_DRAW_TYPE_GOLD || bDrawType == LOTTERY_DRAW_TYPE_ACT)
    {
        LotteryLog stTempLog;
        stTempLog.m_ullPlayerID = pstData->GetRoleBaseInfo().m_ullUin;
        StrCpy(stTempLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
        StrCpy(stTempLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
        stTempLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;
        time_t finishTime = CGameTime::Instance().GetCurrSecond();
        char * pTime = DateTimeStr(&finishTime);
        if (pTime)
        {
            StrCpy(stTempLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
        }
        stTempLog.m_wType = bDrawType + METHOD_LOTTERY_DRAW_TYPE_OFFSET;
        stTempLog.m_chItemType = rstRewardItemInfo.m_astSyncItemList[bCount].m_bItemType;
        stTempLog.m_dwItemID = rstRewardItemInfo.m_astSyncItemList[bCount].m_dwItemId;
        stTempLog.m_dwItemNum = rstRewardItemInfo.m_astSyncItemList[bCount].m_iValueChg;
        WriteSvrLog("LotteryLog", (char*)&stTempLog, sizeof(stTempLog));
    }
    else if (bDrawType == LOTTERY_DRAW_TYPE_DIAMOND_CNT || bDrawType == LOTTERY_DRAW_TYPE_GOLD_CNT || bDrawType == LOTTERY_DRAW_TYPE_ACT_CNT)
    {
        LotteryCntLog stTempLog;
        stTempLog.m_ullPlayerID = pstData->GetRoleBaseInfo().m_ullUin;
        StrCpy(stTempLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
        StrCpy(stTempLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
        stTempLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;
        time_t finishTime = CGameTime::Instance().GetCurrSecond();
        char * pTime = DateTimeStr(&finishTime);
        if (pTime)
        {
            StrCpy(stTempLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
        }
        stTempLog.m_chType = bDrawType;
        int iCount = bCount - MAX_LOTTERY_CNT_NUM * 2 + 2;
        if (iCount >= 0)
        {
            for (int i = 0; i < MAX_LOTTERY_CNT_NUM && iCount < MAX_SYNC_ITEM_NUM; ++i)
            {
                int8_t* pchTemp = (int8_t *)((size_t)(&stTempLog.m_chItem1Type) + i * (sizeof(int8_t) + sizeof(uint32_t) + sizeof(uint32_t)));
                *pchTemp = rstRewardItemInfo.m_astSyncItemList[iCount].m_bItemType;
                uint32_t* pdwTemp = (uint32_t *)((size_t)(&stTempLog.m_dwItem1ID) + i * (sizeof(int8_t) + sizeof(uint32_t) + sizeof(uint32_t)));
                *pdwTemp = rstRewardItemInfo.m_astSyncItemList[iCount].m_dwItemId;
                pdwTemp = (uint32_t *)((size_t)(&stTempLog.m_dwItem1Num) + i * (sizeof(int8_t) + sizeof(uint32_t) + sizeof(uint32_t)));
                *pdwTemp = rstRewardItemInfo.m_astSyncItemList[iCount].m_iValueChg;
                iCount += 2;
            }
        }
        WriteSvrLog("LotteryCntLog", (char*)&stTempLog, sizeof(stTempLog));
    }
}

void ZoneLog::WriteFightPvPLog(PlayerData* pstData, uint32_t dwOldScore, uint32_t dwNewScore, uint16_t wOldRank, uint16_t wNewRank)
{
    FightPVPRankLog stFightPVPRankLog;
    stFightPVPRankLog.m_ullPlayerID = pstData->GetRoleBaseInfo().m_ullUin;
    StrCpy(stFightPVPRankLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stFightPVPRankLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stFightPVPRankLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;
    stFightPVPRankLog.m_chVipLvl = pstData->GetVIPLv();

    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);

    if (pTime)
    {
        StrCpy(stFightPVPRankLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    stFightPVPRankLog.m_dwOldScore = dwOldScore;
    stFightPVPRankLog.m_dwNewScore = dwNewScore;
    stFightPVPRankLog.m_wOldRank = wOldRank;
    stFightPVPRankLog.m_wNewRank = wNewRank;

    WriteSvrLog("FightPVPRankLog", (char*)&stFightPVPRankLog, sizeof(stFightPVPRankLog));
}

void ZoneLog::WriteSerialNumLog(PlayerData* pstData, const char* pszSerialNum)
{
    SerialNumLog stSerialNumLog;
    stSerialNumLog.m_ullPlayerID = pstData->GetRoleBaseInfo().m_ullUin;
    StrCpy(stSerialNumLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), PKGMETA::MAX_NAME_LENGTH);
    StrCpy(stSerialNumLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stSerialNumLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;

    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);

    if (pTime)
    {
        StrCpy(stSerialNumLog.m_szDateTime, pTime, PKGMETA::MAX_TIMEINFO_LENGTH);
    }

    StrCpy(stSerialNumLog.m_szSerialNum, pszSerialNum, PKGMETA::MAX_SERIAL_NUM_LEN);
    WriteSvrLog("SerialNumLog", (char*)&stSerialNumLog, sizeof(stSerialNumLog));
}

void ZoneLog::WriteActivityLog(PlayerData* pstData, const CS_PKG_FIGHT_PVE_SETTLE_REQ& rstCsPkgBodyReq, uint32_t dwPara1, uint32_t dwPara2, uint32_t dwPara3, uint8_t bEvaluateLvl, uint8_t bHardLvl)
{
    ActivityLog stActivityLog;
    stActivityLog.m_ullPlayerID = pstData->GetRoleBaseInfo().m_ullUin;
    StrCpy(stActivityLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), PKGMETA::MAX_NAME_LENGTH);
    StrCpy(stActivityLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stActivityLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;
    stActivityLog.m_dwLevelID = rstCsPkgBodyReq.m_dwFLevelID;
    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);

    if (pTime)
    {
        StrCpy(stActivityLog.m_szDateTime, pTime, PKGMETA::MAX_TIMEINFO_LENGTH);
        StrCpy(stActivityLog.m_szFinishTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    pTime = DateTimeStr((time_t*)&(rstCsPkgBodyReq.m_ullTimeStampStart));
    if (pTime)
    {
        StrCpy(stActivityLog.m_szStartTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    DT_ROLE_BASE_INFO stBaseInfo = pstData->GetRoleBaseInfo();
    stActivityLog.m_dwReservePara1 = dwPara1;
    stActivityLog.m_dwReservePara2 = dwPara2;
    stActivityLog.m_dwReservePara3 = dwPara3;
    stActivityLog.m_dwSecSinceReg = CurTime - stBaseInfo.m_llFirstLoginTime;
    _GetDaySinceReg(stBaseInfo, &stActivityLog.m_wDaySinceReg);
    stActivityLog.m_chEvaluateLvl = bEvaluateLvl;
    stActivityLog.m_chHardLvl = bHardLvl;

    WriteSvrLog("ActivityLog", (char*)&stActivityLog, sizeof(stActivityLog));
}

void ZoneLog::WritePayLog(Player* poPlayer, const DT_SDK_PAY_CB& rstSdkPayCb, uint32_t dwMoneyPurchased)
{
    PayLog stPayLog;
    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);

    if (pTime)
    {
        StrCpy(stPayLog.m_szDateTime, pTime, PKGMETA::MAX_TIMEINFO_LENGTH);
    }

    stPayLog.m_dwProductID = rstSdkPayCb.m_dwProductID;
    stPayLog.m_ullRoleID = rstSdkPayCb.m_ullRoleID;
    stPayLog.m_dwMoneyPurchased = dwMoneyPurchased;
    StrCpy(stPayLog.m_szOrderID, rstSdkPayCb.m_szOrderId, DWLOG::MAX_LEN_ORDER_ID);
    StrCpy(stPayLog.m_szUserID, rstSdkPayCb.m_szUserID, DWLOG::MAX_NAME_LENGTH);

    stPayLog.m_wDaySinceSvrOpen = SvrTime::Instance().GetOpenSvrDay();

    DT_ROLE_BASE_INFO stBaseInfo = poPlayer->GetPlayerData().GetRoleBaseInfo();
    _GetDaySinceReg(stBaseInfo, &stPayLog.m_wDaySinceReg);

    if (NULL != poPlayer)
    {
        //只能保证玩家在内存中才记录渠道号
        StrCpy(stPayLog.m_szChannelName, poPlayer->GetSubChannelName(), DWLOG::MAX_NAME_LENGTH);
    }

    WriteSvrLog("PayLog", (char*)&stPayLog, sizeof(stPayLog));
}

void ZoneLog::WriteCoinLog(PlayerData* pstData, uint32_t dwChgValue, uint8_t bCoinType, uint8_t bChgType, uint64_t dwCurValue, uint32_t dwApproach)
{
    CoinLog stTempCoinLog;
    stTempCoinLog.m_ullPlayerID = pstData->m_pOwner->GetUin();
    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);

    if (pTime)
    {
        StrCpy(stTempCoinLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    StrCpy(stTempCoinLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stTempCoinLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stTempCoinLog.m_wLevel = pstData->GetMajestyInfo().m_wLevel;

    stTempCoinLog.m_chCoinType = bCoinType;
    stTempCoinLog.m_dwChgValue = dwChgValue;
    stTempCoinLog.m_chChgType = bChgType;
    stTempCoinLog.m_dwCurValue = dwCurValue;
    stTempCoinLog.m_wApproach = dwApproach;

    _GetDaySinceReg(pstData->GetRoleBaseInfo(), &stTempCoinLog.m_wDaySinceReg);
    stTempCoinLog.m_wDaySinceSvrOpen = SvrTime::Instance().GetOpenSvrDay();

    WriteSvrLog("CoinLog", (char*)&stTempCoinLog, sizeof(stTempCoinLog));
}

void ZoneLog::WriteMajestyLog(PlayerData* pstData)
{
    MajestyLog stTempMajestyLog;
    stTempMajestyLog.m_ullPlayerID = pstData->GetRoleBaseInfo().m_ullUin;
    StrCpy(stTempMajestyLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stTempMajestyLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stTempMajestyLog.m_wMajestyLvl = pstData->GetMajestyInfo().m_wLevel;

    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char* pTime = DateTimeStr(&CurTime);
    char* pHour = const_cast<char*>(string(pTime).substr(11, 2).c_str());
    LOGWARN("Current Hour: %s", pHour);
    if (pTime)
    {
        StrCpy(stTempMajestyLog.m_szDateTime, pTime, DWLOG::MAX_DAY_TIME_LENGTH);//记录时间时，只记录年-月-日
        StrCpy(stTempMajestyLog.m_szCurrentHour, pHour, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    DT_ROLE_BASE_INFO stBaseInfo = pstData->GetRoleBaseInfo();
    stTempMajestyLog.m_dwSecSinceReg = CurTime - stBaseInfo.m_llFirstLoginTime;
    _GetDaySinceReg(stBaseInfo, &stTempMajestyLog.m_wDaySinceReg);
    stTempMajestyLog.m_chVIPLvl = pstData->GetVIPLv();

    WriteSvrLog("MajestyLog", (char*)&stTempMajestyLog, sizeof(stTempMajestyLog));
}

void ZoneLog::WritePvPLog(PlayerData * pstData, const char* pszPvPType, uint16_t wWinCount)
{
    PvPLog stPvPLog;
    stPvPLog.m_ullPlayerID = pstData->GetRoleBaseInfo().m_ullUin;
    StrCpy(stPvPLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stPvPLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stPvPLog.m_wMajestyLvl = pstData->GetMajestyInfo().m_wLevel;

    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char* pTime = DateTimeStr(&CurTime);
    char* pHour = const_cast<char*>(string(pTime).substr(11, 2).c_str());
    if (pTime)
    {
        StrCpy(stPvPLog.m_szDateTime, pTime, DWLOG::MAX_DAY_TIME_LENGTH);//记录时间时，只记录年-月-日
        StrCpy(stPvPLog.m_szCurrentHour, pHour, DWLOG::MAX_TIMEINFO_LENGTH);
    }
    stPvPLog.m_chVIPLvl = pstData->GetVIPLv();
    stPvPLog.m_dwCombatEffectiveness = pstData->GetMajestyInfo().m_dwCurrentLi;
    stPvPLog.m_chEloLvlID = pstData->GetELOInfo().m_stPvp6v6Info.m_stBaseInfo.m_bELOLvId;
    stPvPLog.m_wWinCount = wWinCount;
    LOGWARN("WinCount: %d", stPvPLog.m_wWinCount);
    StrCpy(stPvPLog.m_szPvPType, pszPvPType, DWLOG::MAX_NAME_LENGTH);

    WriteSvrLog("PvPLog", (char*)&stPvPLog, sizeof(stPvPLog));
}

void ZoneLog::WriteArenaLog(const DT_ASYNC_PVP_PLAYER_SHOW_DATA * pstData, time_t CurTime)
{
    ArenaLog stArenaLog;
    stArenaLog.m_chVIPLvl = pstData->m_bVipLv;
    stArenaLog.m_ullPlayerID = pstData->m_stBaseInfo.m_ullUin;
    stArenaLog.m_chRanking = pstData->m_stBaseInfo.m_dwRank;
    stArenaLog.m_dwCombatEffectiveness = pstData->m_stBaseInfo.m_dwLi;
    stArenaLog.m_wMajestyLvl = pstData->m_stBaseInfo.m_bLv;

    char* pTime = DateTimeStr(&CurTime);
    char* pHour = const_cast<char*>(string(pTime).substr(11, 2).c_str());
    if (pTime)
    {
        StrCpy(stArenaLog.m_szDateTime, pTime, DWLOG::MAX_DAY_TIME_LENGTH);//记录时间时，只记录年-月-日
        StrCpy(stArenaLog.m_szCurrentHour, pHour, DWLOG::MAX_TIMEINFO_LENGTH);
    }
    WriteSvrLog("ArenaLog", (char*)&stArenaLog, sizeof(stArenaLog));
}

void ZoneLog::WriteMarketRefreshLog(PlayerData * pstData, const char* pszShopType)
{
    MarketRefreshLog stTempMarketRefreahLog;
    stTempMarketRefreahLog.m_ullPlayerID = pstData->GetRoleBaseInfo().m_ullUin;
    StrCpy(stTempMarketRefreahLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stTempMarketRefreahLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);

    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char* pTime = DateTimeStr(&CurTime);
    LOGWARN("CurrentDay: %s", pTime);
    if (pTime)
    {
        StrCpy(stTempMarketRefreahLog.m_szDateTime, pTime, DWLOG::MAX_DAY_TIME_LENGTH);
    }
    LOGWARN("CurrentDay(ClassMember): %s", stTempMarketRefreahLog.m_szDateTime);

    StrCpy(stTempMarketRefreahLog.m_szShopType, pszShopType, DWLOG::MAX_NAME_LENGTH);

    WriteSvrLog("MarketRefreshLog", (char*)&stTempMarketRefreahLog, sizeof(stTempMarketRefreahLog));
}

void ZoneLog::WritePromotionalActivityLog(PlayerData * pstData, uint8_t bConsumeAward, uint8_t bCombatEffectivenessAward, uint16_t wCarnivalAward, uint16_t wNew7DayAward, uint16_t wNew7DayDailyProgress)
{
    PromotionalActivityLog stTempPromotionalActivityLog;
    stTempPromotionalActivityLog.m_ullPlayerID = pstData->GetRoleBaseInfo().m_ullUin;
    StrCpy(stTempPromotionalActivityLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stTempPromotionalActivityLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stTempPromotionalActivityLog.m_wMajestyLvl = pstData->GetMajestyInfo().m_wLevel;

    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);
    char* pHour = const_cast<char*>(string(pTime).substr(11, 2).c_str());
    if (pTime)
    {
        StrCpy(stTempPromotionalActivityLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
        StrCpy(stTempPromotionalActivityLog.m_szCurrentHour, pHour, DWLOG::MAX_TIMEINFO_LENGTH);
    }
    DT_ROLE_BASE_INFO stBaseInfo = pstData->GetRoleBaseInfo();
    _GetDaySinceReg(stBaseInfo, &stTempPromotionalActivityLog.m_wDaySinceReg);
    stTempPromotionalActivityLog.m_chConsumeAward = bConsumeAward;
    stTempPromotionalActivityLog.m_chCombatEffectivenessAward = bCombatEffectivenessAward;
    stTempPromotionalActivityLog.m_chCarnivalAward = wCarnivalAward;
    stTempPromotionalActivityLog.m_chSevenDayAward = wNew7DayAward;
    stTempPromotionalActivityLog.m_chNew7DayDailyProgress = wNew7DayDailyProgress;

    WriteSvrLog("PromotionalActivityLog", (char*)&stTempPromotionalActivityLog, sizeof(stTempPromotionalActivityLog));
}

void ZoneLog::WriteGuildBossLog(PlayerData * pstData, uint32_t dwDamage, uint16_t wBossID)
{
    GuildBossLog stGuildBossLog;
    stGuildBossLog.m_ullUin = pstData->GetRoleBaseInfo().m_ullUin;
    stGuildBossLog.m_ullGuildID = pstData->GetGuildInfo().m_ullGuildId;
    stGuildBossLog.m_dwDamage = dwDamage;
    StrCpy(stGuildBossLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stGuildBossLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stGuildBossLog.m_dwCombatEffectiveness = pstData->GetMajestyInfo().m_dwCurrentLi;
    stGuildBossLog.m_wBossID = wBossID;
    stGuildBossLog.m_wMajestyLvl = pstData->GetMajestyInfo().m_wLevel;
    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char* pTime = DateTimeStr(&CurTime);
    if (pTime)
    {
        StrCpy(stGuildBossLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    WriteSvrLog("GuildBossLog", (char*)&stGuildBossLog, sizeof(stGuildBossLog));
}

void ZoneLog::WriteCloneBattleLog(Player * poPlayer, int8_t chMemCount, uint32_t dwBossID)
{
    CloneBattleLog stCloneBattleLog;
    stCloneBattleLog.m_ullUin = poPlayer->GetUin();
    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);

    if (pTime)
    {
        StrCpy(stCloneBattleLog.m_szDateTime, pTime, PKGMETA::MAX_TIMEINFO_LENGTH);
    }
    StrCpy(stCloneBattleLog.m_szAccountName, poPlayer->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stCloneBattleLog.m_szRoleName, poPlayer->GetRoleName(), DWLOG::MAX_NAME_LENGTH);

    stCloneBattleLog.m_chMemCount = chMemCount;
    stCloneBattleLog.m_wWinCount = poPlayer->GetPlayerData().GetCloneBattleInfo().m_bWinNum;
    stCloneBattleLog.m_dwBossID = dwBossID;

    WriteSvrLog("CloneBattleLog", (char*)&stCloneBattleLog, sizeof(stCloneBattleLog));
}

void ZoneLog::WriteDailyChallengeLog(PlayerData * pstData, const DT_ROLE_DAILY_CHALLENGE_INFO& stDailyChallengeInfo)
{
    DailyChallengeLog stDailyChallengeLog;
    stDailyChallengeLog.m_ullPlayerID = pstData->GetRoleBaseInfo().m_ullUin;
    StrCpy(stDailyChallengeLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stDailyChallengeLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);
    stDailyChallengeLog.m_wMajestyLvl = pstData->GetMajestyInfo().m_wLevel;

    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char* pTime = DateTimeStr(&CurTime);
    if (pTime)
    {
        StrCpy(stDailyChallengeLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }
    stDailyChallengeLog.m_chVIPLvl = pstData->GetVIPLv();
    stDailyChallengeLog.m_dwCombatEffectiveness = pstData->GetMajestyInfo().m_dwCurrentLi;
    stDailyChallengeLog.m_wWinCount = stDailyChallengeInfo.m_bWinCount;
    stDailyChallengeLog.m_wSiegeEquipment = stDailyChallengeInfo.m_bSiegeEquipment;

    for (int i = 0 ; i < MAX_BUFF_RANDOM_NUM; ++i)
    {
        int8_t* pchTemp = &stDailyChallengeLog.m_chBuff1 + sizeof(int8_t) * i;
        if (stDailyChallengeInfo.m_stBuffs.m_szIsSelectedList[i] == 1)
        {
            *pchTemp = (int8_t)stDailyChallengeInfo.m_stBuffs.m_szRdBuffIndexList[i];
        }
        else
        {
            *pchTemp = -1;
        }
    }

    WriteSvrLog("DailyChallengeLog", (char*)&stDailyChallengeLog, sizeof(stDailyChallengeLog));
}

void ZoneLog::WriteClientCheatLog(PlayerData* pstData)
{
    CheatLog stTempCheatLog;
    stTempCheatLog.m_ullPlayerID = pstData->m_pOwner->GetUin();
    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char * pTime = DateTimeStr(&CurTime);

    if (pTime)
    {
        StrCpy(stTempCheatLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }

    //DT_ROLE_BASE_INFO stBaseInfo = pstData->GetRoleBaseInfo();

    StrCpy(stTempCheatLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stTempCheatLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);

    WriteSvrLog("CheatLog", (char*)&stTempCheatLog, sizeof(stTempCheatLog));
}

void ZoneLog::WriteMineOpLog(PlayerData * pstData, const SS_PKG_MINE_DEAL_ORE_RSP& rstMineRsp)
{
    MineOptLog stMineLog;
    _WritePlayerInfo<MineOptLog>(pstData, stMineLog);
    stMineLog.m_wOptType = rstMineRsp.m_bDealType;
    stMineLog.m_ullOwnerUin = rstMineRsp.m_stOre.m_ullOwnerUin;
    stMineLog.m_wTroopCount = rstMineRsp.m_stOre.m_bTroopCount;
    stMineLog.m_wGeneral1 = rstMineRsp.m_stOre.m_astTroopInfo[0].m_stGeneralInfo.m_dwId;
    stMineLog.m_wGeneral2 = rstMineRsp.m_stOre.m_astTroopInfo[1].m_stGeneralInfo.m_dwId;
    stMineLog.m_wGeneral3 = rstMineRsp.m_stOre.m_astTroopInfo[2].m_stGeneralInfo.m_dwId;
    stMineLog.m_wMineId = rstMineRsp.m_stOre.m_dwOreId;

    WriteSvrLog("MineOptLog", (char*)&stMineLog, sizeof(stMineLog));
}

void ZoneLog::WriteMineExploreLog(PlayerData * pstData, const SS_PKG_MINE_EXPLORE_RSP & rstMineRsp)
{
    MineExploreLog stMineExploreLog;
    _WritePlayerInfo<MineExploreLog>(pstData, stMineExploreLog);
    stMineExploreLog.m_wMineId1 = rstMineRsp.m_astExploreOreList[0].m_dwOreId;
    stMineExploreLog.m_wMineId2 = rstMineRsp.m_astExploreOreList[1].m_dwOreId;
    stMineExploreLog.m_wMineId3 = rstMineRsp.m_astExploreOreList[2].m_dwOreId;
    stMineExploreLog.m_wMineId4 = rstMineRsp.m_astExploreOreList[3].m_dwOreId;
    stMineExploreLog.m_wMineId5 = rstMineRsp.m_astExploreOreList[4].m_dwOreId;
    stMineExploreLog.m_wOreType1 = rstMineRsp.m_astExploreOreList[0].m_bOreType;
    stMineExploreLog.m_wOreType2 = rstMineRsp.m_astExploreOreList[1].m_bOreType;
    stMineExploreLog.m_wOreType3 = rstMineRsp.m_astExploreOreList[2].m_bOreType;
    stMineExploreLog.m_wOreType4 = rstMineRsp.m_astExploreOreList[3].m_bOreType;
    stMineExploreLog.m_wOreType5 = rstMineRsp.m_astExploreOreList[4].m_bOreType;

    WriteSvrLog("MineExploreLog", (char*)&stMineExploreLog, sizeof(stMineExploreLog));
}



void ZoneLog::_GetDaySinceReg(const DT_ROLE_BASE_INFO & stRoleBaseInfo, uint16_t * pwDaySinceReg)
{
    time_t uRegSec = stRoleBaseInfo.m_llFirstLoginTime;
    tm* poRegDay = localtime(&uRegSec);
    poRegDay->tm_hour = 0;
    poRegDay->tm_min = 0;
    poRegDay->tm_sec = 0;
    uint64_t ullRegMidnight = mktime(poRegDay);
    time_t lCurSec = CGameTime::Instance().GetCurrSecond();
    int iDeltaTime = lCurSec - ullRegMidnight;
    *pwDaySinceReg = iDeltaTime / SECONDS_OF_DAY + 1;
}

template<typename LOG_TYPE>
inline void ZoneLog::_WritePlayerInfo(PlayerData* pstData, LOG_TYPE &stLog)
{
    stLog.m_ullPlayerID = pstData->GetRoleBaseInfo().m_ullUin;
    StrCpy(stLog.m_szAccountName, pstData->m_pOwner->GetAccountName(), DWLOG::MAX_NAME_LENGTH);
    StrCpy(stLog.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, DWLOG::MAX_NAME_LENGTH);

    time_t CurTime = CGameTime::Instance().GetCurrSecond();
    char* pTime = DateTimeStr(&CurTime);
    if (pTime)
    {
        StrCpy(stLog.m_szDateTime, pTime, DWLOG::MAX_TIMEINFO_LENGTH);
    }
    stLog.m_chVIPLvl = pstData->GetVIPLv();
}
