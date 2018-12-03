#include "RankMgr.h"
#include "LogMacros.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "GameTime.h"
#include "../gamedata/GameDataMgr.h"
#include "ov_res_keywords.h"
#include "GeneralCard.h"
#include "Majesty.h"


//#define RANK_UPDATE_INTERVAL_MS         (1200000)
//#define RANK_UPDATE_START_INTERVAL_MS   (30000)
//#define RANK_UPDATE_WAIT_MS             (3000)
//
#define RANK_LI_UPDATE_TIME             1800



bool RankMgr::Init()
{
    m_dwRankCount = 0;
    m_ullLastUpdateTime = 0;
    m_dwRankLimitLow = (uint32_t)CGameDataMgr::Instance().GetResBasicMgr().Find((int)RANK_LIMIT_LOW)->m_para[0];
    m_dwMaxNum = MAX_RANK_TOP_NUM;
    return true;
}

int RankMgr::Update()
{
    if (CGameTime::Instance().GetCurrSecond() - m_ullLastUpdateTime > RANK_LI_UPDATE_TIME)
    {
        m_ullLastUpdateTime = CGameTime::Instance().GetCurrSecond();
    }
    return 0;
}

void RankMgr::UpdatePlayerData(PlayerData * pstData)
{

    if (pstData->GetMiscInfo().m_ullRankLiLastUpdate < m_ullLastUpdateTime)
    {
        UpdateLi(pstData);
        pstData->GetMiscInfo().m_ullRankLiLastUpdate = m_ullLastUpdateTime;
    }

}

int RankMgr::UpdateGCardCnt(PlayerData * pstData)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_RANK_COMMON_UPDATE_NTF;
	SS_PKG_RANK_COMMON_UPDATE_NTF& rstRankUpdateNtf = m_stSsPkg.m_stBody.m_stRankCommonUpdateNtf;
    DT_RANK_ROLE_INFO& rstRankInfo = rstRankUpdateNtf.m_stReqInfo.m_stRankRoleInfo;
	rstRankUpdateNtf.m_bType = RANK_ROLE_INFO_TYPE;
    rstRankInfo.m_dwValue = pstData->GetGCardInfo().m_iCount;
    rstRankInfo.m_bRankType = RANK_TYPE_GCARD_CNT;
    _PadOtherRankInfo(pstData, rstRankInfo);
    if (rstRankInfo.m_szRoleName[0] != '\0' && rstRankInfo.m_dwValue > 0)
    {
        ZoneSvrMsgLayer::Instance().SendToRankSvr(m_stSsPkg);
    }
    return 0;
}

int RankMgr::UpdatePveStar(PlayerData * pstData)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_RANK_COMMON_UPDATE_NTF;
	SS_PKG_RANK_COMMON_UPDATE_NTF& rstRankUpdateNtf = m_stSsPkg.m_stBody.m_stRankCommonUpdateNtf;
	DT_RANK_ROLE_INFO& rstRankInfo = rstRankUpdateNtf.m_stReqInfo.m_stRankRoleInfo;
	rstRankUpdateNtf.m_bType = RANK_ROLE_INFO_TYPE;
    rstRankInfo.m_dwValue = pstData->GetPveInfo().m_dwPveTotalStar;
    rstRankInfo.m_bRankType = RANK_TYPE_PVE_STAR;
    _PadOtherRankInfo(pstData, rstRankInfo);
    if (rstRankInfo.m_szRoleName[0] != '\0' && rstRankInfo.m_dwValue > 0)
    {
        ZoneSvrMsgLayer::Instance().SendToRankSvr(m_stSsPkg);
    }
    return 0;
}



int RankMgr::UpdateLi(PlayerData* pstData)
{

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_RANK_COMMON_UPDATE_NTF;
	SS_PKG_RANK_COMMON_UPDATE_NTF& rstRankUpdateNtf = m_stSsPkg.m_stBody.m_stRankCommonUpdateNtf;
	DT_RANK_ROLE_INFO& rstRankInfo = rstRankUpdateNtf.m_stReqInfo.m_stRankRoleInfo;
	rstRankUpdateNtf.m_bType = RANK_ROLE_INFO_TYPE;
    rstRankInfo.m_dwValue = GeneralCard::Instance().GetTeamLi(pstData);
    rstRankInfo.m_bRankType = RANK_TYPE_LI;
    _PadOtherRankInfo(pstData, rstRankInfo);
    if (rstRankInfo.m_szRoleName[0] != '\0' && rstRankInfo.m_dwValue > 0)
    {
        ZoneSvrMsgLayer::Instance().SendToRankSvr(m_stSsPkg);
    }
    return 0;
}

int RankMgr::UpdateGCardLi(PlayerData * pstData, uint32_t dwId, bool bIsAdd)
{
    DT_ITEM_GCARD * pstGeneral = GeneralCard::Instance().Find(pstData, dwId);
    if (NULL == pstGeneral)
    {
        LOGERR("Uin<%lu> don't find the GCard<%u>!", pstData->m_ullUin, dwId);
        return ERR_SYS;
    }
    return UpdateGCardLi(pstData, pstGeneral, bIsAdd);
}

int RankMgr::UpdateGCardLi(PlayerData * pstData, DT_ITEM_GCARD * pstGeneral, bool bIsAdd)
{
    if (bIsAdd && m_dwGCardLiRankNum >= m_dwMaxNum && pstGeneral->m_dwLi < m_dwGCardLiRankLastValue)
    {
        //如果战力增加还不能上榜,就不用发给RankSvr
        return 0;
    }
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_RANK_COMMON_UPDATE_NTF;
	SS_PKG_RANK_COMMON_UPDATE_NTF& rstRankUpdateNtf = m_stSsPkg.m_stBody.m_stRankCommonUpdateNtf;
	DT_RANK_ROLE_INFO& rstRankInfo = rstRankUpdateNtf.m_stReqInfo.m_stRankRoleInfo;
	rstRankUpdateNtf.m_bType = RANK_ROLE_INFO_TYPE;
    rstRankInfo.m_dwValue = pstGeneral->m_dwLi;
    rstRankInfo.m_bRankType = RANK_TYPE_GCARD_LI;
    DT_ROLE_MAJESTY_INFO & rstMajestyInfo = pstData->GetMajestyInfo();

    rstRankInfo.m_ullUin = pstData->m_ullUin;
    rstRankInfo.m_wLv = rstMajestyInfo.m_wLevel;
    StrCpy(rstRankInfo.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, MAX_NAME_LENGTH);
    rstRankInfo.m_ullTimeStampMs = CGameTime::Instance().GetCurrTimeMs();
    rstRankInfo.m_bGeneralCount = 1;
    rstRankInfo.m_astGeneralList[0].m_bGrade = pstGeneral->m_bPhase;
    rstRankInfo.m_astGeneralList[0].m_dwGeneralID = pstGeneral->m_dwId;
    rstRankInfo.m_astGeneralList[0].m_bStar = pstGeneral->m_bStar;
	rstRankInfo.m_bVipLv = rstMajestyInfo.m_bVipLv;
    if (rstRankInfo.m_szRoleName[0] != '\0' && rstRankInfo.m_dwValue > 0)
    {
        ZoneSvrMsgLayer::Instance().SendToRankSvr(m_stSsPkg);
    }
    return 0;
}

void RankMgr::_PadOtherRankInfo(PlayerData * pstData, OUT DT_RANK_ROLE_INFO& rstRankInfo)
{
    DT_ROLE_MAJESTY_INFO & rstMajestyInfo = pstData->GetMajestyInfo();
    rstRankInfo.m_ullUin = pstData->m_ullUin;
    rstRankInfo.m_wLv = rstMajestyInfo.m_wLevel;
    StrCpy(rstRankInfo.m_szRoleName, pstData->GetRoleBaseInfo().m_szRoleName, MAX_NAME_LENGTH);
    rstRankInfo.m_ullTimeStampMs = CGameTime::Instance().GetCurrTimeMs();
	rstRankInfo.m_bVipLv = rstMajestyInfo.m_bVipLv;
    rstRankInfo.m_dwLeaderValue = pstData->m_dwLeaderValue;

    DT_BATTLE_ARRAY_INFO* pstBattleArrayInfo = Majesty::Instance().GetBattleArrayInfo(pstData, BATTLE_ARRAY_TYPE_NORMAL);
    if (NULL == pstBattleArrayInfo)
    {
        LOGERR("Uin<%lu> Find pstBattleArrayInfo error", pstData->m_ullUin);
        return;
    }
    rstRankInfo.m_bGeneralCount = pstBattleArrayInfo->m_bGeneralCnt;
    for (size_t i = 0; i < rstRankInfo.m_bGeneralCount && i < MAX_TROOP_NUM_PVP; i++)
    {
        uint32_t dwGeneralId = pstBattleArrayInfo->m_GeneralList[i];

        rstRankInfo.m_astGeneralList[i].m_dwGeneralID = dwGeneralId;
        if (0 == dwGeneralId)
        {
            continue;
        }
        //获取相应的武将卡
        DT_ITEM_GCARD* pstGeneral = GeneralCard::Instance().Find(pstData, dwGeneralId);
        if (!pstGeneral)
        {
            LOGERR("Uin<%lu> cant find the GCard<%u>", pstData->m_ullUin, dwGeneralId);
            continue;
        }
        rstRankInfo.m_astGeneralList[i].m_bGrade = pstGeneral->m_bPhase;
        rstRankInfo.m_astGeneralList[i].m_bStar = pstGeneral->m_bStar;
        rstRankInfo.m_astGeneralList[i].m_dwSkinId = pstGeneral->m_dwSkinId;
        rstRankInfo.m_astGeneralList[i].m_bLv = pstGeneral->m_bLevel;
    }
}


