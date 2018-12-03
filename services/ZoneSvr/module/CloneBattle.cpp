#include "CloneBattle.h"
#include "../gamedata/GameDataMgr.h"
#include "Match.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "player/PlayerMgr.h"
#include "Item.h"
#include "GeneralCard.h"
#include "Lottery.h"
#include "dwlog_def.h"
#include "TaskAct.h"


bool CloneBattle::Init()
{
    RESBASIC* poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(CONST_RESBASIC_ID_BASE_INFO);
    assert(poResBasic);
    m_dwTicketId = (uint32_t) poResBasic->m_para[2];
    m_dwWinNumLimit = (uint32_t)poResBasic->m_para[0];

    poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(CONST_RESBASIC_ID_LV_LIMIT);
    assert(poResBasic);
    m_dwLvLimit = (uint16_t)poResBasic->m_para[0];

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_CLONE_BATTLE_ZONESVR_ONLINE_NTF;
    ZoneSvrMsgLayer::Instance().SendToCloneBattleSvr(m_stSsPkg);

    return true;
}

void CloneBattle::InitPlayerData(PlayerData& roPData)
{
    DT_ROLE_CLONE_BATTLE_INFO& rstCloneBattleInfo = roPData.GetCloneBattleInfo();
    if (rstCloneBattleInfo.m_ullTeamId != 0 && rstCloneBattleInfo.m_ullLastUptTime != 0 
        && rstCloneBattleInfo.m_ullLastUptTime < m_ullLastUptTimeMs)
    {
        bzero(&rstCloneBattleInfo, sizeof(DT_ROLE_CLONE_BATTLE_INFO));
        rstCloneBattleInfo.m_ullLastUptTime = m_ullLastUptTimeMs;
    }
    
}

int CloneBattle::CheckFight(PlayerData& roPData)
{
    DT_ROLE_CLONE_BATTLE_INFO& rstCloneBattleInfo = roPData.GetCloneBattleInfo();
    if (rstCloneBattleInfo.m_ullTeamId == 0)
    {
        LOGERR("Uin<%lu> the teamid is 0", roPData.m_ullUin);
        return ERR_NOT_SATISFY_COND;
    }
    if (CheckLvLimit(roPData) != ERR_NONE)
    {
        return ERR_LEVEL_LIMIT;
    }
    if (rstCloneBattleInfo.m_bWinNum >= m_dwWinNumLimit)
    {
        LOGERR("Uin<%lu> the fight num<%hhu> is full is NULL", roPData.m_ullUin, rstCloneBattleInfo.m_bWinNum);
        return ERR_NOT_SATISFY_COND;
    }
    return ERR_NONE;
}


int CloneBattle::CheckLvLimit(PlayerData& roPData)
{
    if (roPData.GetLv() < m_dwLvLimit)
    {
        LOGERR("Uin<%lu> current lv<%hu> is smaller than the limit lv<%hu>", roPData.m_ullUin,
            roPData.GetLv(), m_dwLvLimit);
        return ERR_LEVEL_LIMIT;
    }
	return ERR_NONE;
}

int CloneBattle::InitMatInfo(PlayerData& roPData, uint32_t dwGCardId, OUT  PKGMETA::DT_CLONE_BATTLE_MATE_INFO& rstMateInfo)
{
    rstMateInfo.m_ullUin = roPData.m_ullUin;
    rstMateInfo.m_wLv = roPData.GetMajestyInfo().m_wLevel;
    StrCpy(rstMateInfo.m_szName, roPData.GetMajestyInfo().m_szName, MAX_NAME_LENGTH);
    return Match::Instance().InitOneTroopInfo(&roPData, dwGCardId, rstMateInfo.m_stTroopInfo);
}



void CloneBattle::BroadcastMate(PKGMETA::SS_PKG_CLONE_BATTLE_BROADCAST_NTF& rstNtf)
{
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CLONE_BATTLE_BROADCAST_NTF;
    SC_PKG_CLONE_BATTLE_BROADCAST_NTF& rstScNtf = m_stScPkg.m_stBody.m_stCloneBattleBroadCastNtf;
    rstScNtf.m_bNtfType = rstNtf.m_bNtfType;
    memcpy(&rstScNtf.m_stTeamInfo, &rstNtf.m_stTeamInfo, sizeof(DT_CLONE_BATTLE_TEAM_INFO));
    Player* poPlayer = NULL;
    
    for (int i = 0 ; i < MAX_NUM_CLONE_BATTLE_MATE; i++)
    {
        if (rstNtf.m_stTeamInfo.m_astMateInfo[i].m_ullUin == 0)
        {
            continue;
        }
        poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstNtf.m_stTeamInfo.m_astMateInfo[i].m_ullUin);
        if (!poPlayer)
        {
            continue;
        }
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    }
}

void CloneBattle::SendTeamReward(PKGMETA::DT_CLONE_BATTLE_TEAM_INFO& rstTeamInfo)
{
    RESPRIMAIL* poResPriMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(CONST_RESPRIMAIL_ID_FO_REWARD);
    assert(poResPriMail);
    RESCOMMONREWARD* poResCommonReward = CGameDataMgr::Instance().GetResCommonRewardMgr().Find(CONST_RESCOMMONREWARD_ID_TEAM_REWARD);
    assert(poResCommonReward);
    RESGENERAL* poResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(rstTeamInfo.m_dwBossId);
    if (!poResGeneral)
    {
        LOGERR("poResGeneral<%u> is NULL", rstTeamInfo.m_dwBossId);
        return;
    }
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
    SS_PKG_MAIL_ADD_REQ& rstMailAddReq = m_stSsPkg.m_stBody.m_stMailAddReq;
    rstMailAddReq.m_nUinCount = 0;
    for (int i = 0; i < MAX_NUM_CLONE_BATTLE_MATE; i++)
    {
        if (rstTeamInfo.m_astMateInfo[i].m_ullUin == 0)
        {
            continue;
        }
        rstMailAddReq.m_UinList[rstMailAddReq.m_nUinCount++] = rstTeamInfo.m_astMateInfo[i].m_ullUin;
    }
    DT_MAIL_DATA& rstMailData = rstMailAddReq.m_stMailData;
    rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
    rstMailData.m_bState = MAIL_STATE_UNOPENED;
    StrCpy(rstMailData.m_szTitle, poResPriMail->m_szTitle, MAX_MAIL_TITLE_LEN);
    StrCpy(rstMailData.m_szContent, poResPriMail->m_szContent, MAX_MAIL_CONTENT_LEN);
    rstMailData.m_ullFromUin = 0;
    rstMailData.m_bAttachmentCount = MIN(poResPriMail->m_bAttachmentNum, MAX_MAIL_ATTACHMENT_NUM);
    for (int j = 0; j < rstMailData.m_bAttachmentCount; j++)
    {
        rstMailData.m_astAttachmentList[j].m_bItemType = poResPriMail->m_astAttachmentList[j].m_bType;
        rstMailData.m_astAttachmentList[j].m_dwItemId = poResPriMail->m_astAttachmentList[j].m_dwId;
        rstMailData.m_astAttachmentList[j].m_iValueChg = poResPriMail->m_astAttachmentList[j].m_dwNum;
        if (j == 0)
        {
            //替换第一个位置的奖励为对应的Boss碎片ID
            rstMailData.m_astAttachmentList[j].m_dwItemId = poResGeneral->m_dwExchangeId;
        }
    }
    ZoneSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg); 
}

void CloneBattle::HandleSysDissovleTeam(PKGMETA::DT_CLONE_BATTLE_TEAM_INFO& rstTeamInfo)
{
    Player* poPlayer = NULL;
    for (int i = 0; i < MAX_NUM_CLONE_BATTLE_MATE; i++)
    {
        if (rstTeamInfo.m_astMateInfo[i].m_ullUin == 0)
        {
            continue;
        }
           
        poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstTeamInfo.m_astMateInfo[i].m_ullUin);
        if (!poPlayer)
        {
            continue;
        }
        DT_ROLE_CLONE_BATTLE_INFO& rstCloneBattleInfo = poPlayer->GetPlayerData().GetCloneBattleInfo();
        bzero(&rstCloneBattleInfo, sizeof(DT_ROLE_CLONE_BATTLE_INFO));
        rstCloneBattleInfo.m_ullLastUptTime = m_ullLastUptTimeMs;
    }
}

void CloneBattle::UptSysInfo(PKGMETA::SS_PKG_CLONE_BATTLE_UPT_SYSINFO_NTF& rstNtf)
{
    m_stBossInfo = rstNtf.m_stBossInfo;
    m_ullLastUptTimeMs = rstNtf.m_ullLastUptTimeMs;
}

bool CloneBattle::IsHaveTicket(PlayerData& roPData)
{
   return Item::Instance().IsEnough(&roPData, ITEM_TYPE_PROPS, m_dwTicketId, 1);
}

int CloneBattle::ConsumeTicket(PlayerData& roPData, PKGMETA::DT_SYNC_ITEM_INFO& rstSyncItem)
{
    return Item::Instance().ConsumeItem(&roPData, ITEM_TYPE_PROPS, m_dwTicketId, -1, rstSyncItem, DWLOG::METHOD_CLONEBATTLE_JOIN_TEAM);
}

bool CloneBattle::IsHaveBossGCard(PlayerData& roPData, uint8_t bBossType)
{
    uint32_t dwGCardId = m_stBossInfo.m_BossId[bBossType];
    DT_ITEM_GCARD* pstGCard = GeneralCard::Instance().Find(&roPData, dwGCardId);
    return pstGCard == NULL ? false : true;

}

int CloneBattle::OpenRewardBox(PlayerData& roPData, uint32_t dwBossId, uint8_t bSelect, PKGMETA::DT_SYNC_ITEM_INFO& rstSyncItem)
{
    RESCLONEBATTLEFIGHTREWARD* pResReward = CGameDataMgr::Instance().GetResCloneBattleFightRewardMgr().Find(dwBossId);
    if (!pResReward)
    {
        LOGERR("Uin<%lu> the RESCLONEBATTLEFIGHTREWARD<%u> is NULL", roPData.m_ullUin, dwBossId);
        return ERR_SYS;
    }
    uint32_t dwPropId = pResReward->m_poolList[bSelect];
    RESPROPS* pstResProps = CGameDataMgr::Instance().GetResPropsMgr().Find(dwPropId);
    assert(pstResProps);
    uint32_t dwBoxID = pstResProps->m_dwPropsParam;
    RESTREASUREBOX* pstResBox = CGameDataMgr::Instance().GetResTreasureBoxMgr().Find(dwBoxID);
    assert(pstResBox);


    for (size_t j = 0; j < pstResBox->m_bLotteryPoolCount; j++)
    {
        Lottery::Instance().DrawLotteryByPond(&roPData, pstResBox->m_lotteryPoolIdList[j], rstSyncItem,
            DWLOG::METHOD_CLONEBATTLE_REWARD);
    }

    if (bSelect == 1)   //第二个位置会奖励碎片ID
    {
        int idouble = 1;
        if (TaskAct::Instance().IsActTypeOpen(&roPData, ACT_TYPE_DOUBLE_CLONE))
        {
            idouble = 2;
        }
        RESGENERAL* poResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(dwBossId);
        if (!poResGeneral)
        {
            LOGERR("Uin<%lu> poResGeneral<%u> is NULL", roPData.m_ullUin, dwBossId);
        }
        else
        {
            Item::Instance().RewardItem(&roPData, ITEM_TYPE_PROPS, poResGeneral->m_dwExchangeId, 1 * idouble,
                rstSyncItem, DWLOG::METHOD_CLONEBATTLE_REWARD);
        }
    }
    return ERR_NONE;
}

