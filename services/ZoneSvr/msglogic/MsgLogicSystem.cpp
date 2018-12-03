#include "MsgLogicSystem.h"
#include "LogMacros.h"
#include "CpuSampleStats.h"
#include "common_proto.h"
#include "../gamedata/GameDataMgr.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "../module/player/Player.h"
#include "../module/player/PlayerMgr.h"
#include "../module/player/PlayerData.h"
#include "../module/Equip.h"
#include "../module/Props.h"
#include "../module/GeneralCard.h"
#include "../module/Task.h"
#include "../module/Lottery.h"
#include "../module/Consume.h"
#include "../module/Tutorial.h"
#include "../module/MasterSkill.h"
#include "../module/AP.h"
#include "../module/FightPVP.h"
#include "../module/Majesty.h"
#include "../module/Item.h"
#include "../module/Replay.h"
#include "../module/Sign.h"
#include "../module/Guild.h"
#include "../module/Gem.h"
#include "../module/FightPVE.h"
#include "../module/GrowthAward.h"
#include "../module/PropSynthetic.h"
#include "../module/Affiche.h"
#include "../module/Mall.h"
#include "../module/RankMgr.h"
#include "../module/League.h"
#include "dwlog_def.h"
#include "../module/Gm/Gm.h"
#include "../module/ZoneLog.h"
#include "dwlog_svr.h"
#include "../module/VIP.h"
#include "../module/Marquee.h"
#include "../module/PvpRoomMgr.h"
#include "../module/Pay.h"
#include "BitMap.h"
#include "../module/Shops.h"
#include "../module/DailyChallenge.h"
#include "../module/AsyncPvp.h"
#include "../module/PeakArena.h"
#include "../module/SkillPoint.h"
#include "../module/Serial.h"
#include "../module/Tactics.h"
#include "../module/GeneralReborn.h"
#include "../module/Skin.h"

using namespace PKGMETA;
using namespace DWLOG;

bool _IsCurrPhaseAchieved(RESBASIC *pData, int iCurrHour, int64_t llPrevTime);

int ChgGeneralList_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_CHG_GENERAL_LIST_REQ & rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stChgGeneralListReq;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    // 保存匹配当前组卡信息
    Majesty::Instance().SetBattleGeneralList(&poPlayer->GetPlayerData(),
        rstCsPkgBodyReq.m_bGeneralCount, rstCsPkgBodyReq.m_GeneralList, rstCsPkgBodyReq.m_bType);

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CHG_GENERAL_LIST_RSP;
    SC_PKG_CHG_GENERAL_LIST_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stChgGeneralListRsp;
    rstScPkgBodyRsp.m_nErrNo = ERR_NONE;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int MasterSkillSelect_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

	CS_PKG_MASTER_SKILL_SELECT_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stMasterSkillSelectReq;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MASTER_SKILL_SELECT_RSP;
    SC_PKG_MASTER_SKILL_SELECT_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stMasterSkillSelectRsp;

	rstScPkgBodyRsp.m_nErrNo = Majesty::Instance().SetBattleMSkill(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_dwMasterSkillId, rstCsPkgBodyReq.m_bType);
    rstScPkgBodyRsp.m_dwMasterSkillId = rstCsPkgBodyReq.m_dwMasterSkillId;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int ChgBattleArrayFlag_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

	CS_PKG_CHG_BATTLE_ARRAY_FLAG_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stChgBattleArrayFlagReq;

	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CHG_BATTLE_ARRAY_FLAG_RSP;
	SC_PKG_CHG_BATTLE_ARRAY_FLAG_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stChgBattleArrayFlagRsp;

	rstScPkgBodyRsp.m_nErrNo = Majesty::Instance().SetBattleSkillFlag(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_bSkillFlag, rstCsPkgBodyReq.m_bType);
	rstScPkgBodyRsp.m_bType = rstCsPkgBodyReq.m_bType;
	rstScPkgBodyRsp.m_bSkillFlag = rstCsPkgBodyReq.m_bSkillFlag;

	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
	return 0;
}

int ChgGeneralSkillFlag_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

	CS_PKG_CHG_GENERAL_SKILL_FLAG_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stChgGeneralSkillFlagReq;

	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CHG_GENERAL_SKILL_FLAG_RSP;
	SC_PKG_CHG_GENERAL_SKILL_FLAG_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stChgGeneralSkillFlagRsp;

	rstScPkgBodyRsp.m_nErrNo = GeneralCard::Instance().ChgSkillFlag(&poPlayer->GetPlayerData(),
		rstCsPkgBodyReq.m_dwGeneralId, rstCsPkgBodyReq.m_dwSkillFlag, rstCsPkgBodyReq.m_dwAIFlag);

	// 如果是异步PVP，需要同步到异步PVP阵容，待做

	rstScPkgBodyRsp.m_dwGeneralId = rstCsPkgBodyReq.m_dwGeneralId;
	rstScPkgBodyRsp.m_dwSkillFlag = rstCsPkgBodyReq.m_dwSkillFlag;
	rstScPkgBodyRsp.m_dwAIFlag = rstCsPkgBodyReq.m_dwAIFlag;

	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
	return 0;
}
int GmMultKick_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
	SS_PKG_GM_MULT_KICK_RSP &rstSsPkgBodyRsp = m_stSsPkg.m_stBody.m_stGmMulKickRsp;
	rstSsPkgBodyRsp.m_dwCount = rstSsPkg.m_stBody.m_stGmMulKickReq.m_dwCount;

	GmMgr::Instance().HanldeMsgMultKick(rstSsPkg.m_stBody.m_stGmMulKickReq, rstSsPkgBodyRsp);

	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GM_MULT_KICK_RSP;
	m_stSsPkg.m_stBody.m_stGmMulKickRsp = rstSsPkgBodyRsp;
	return ZoneSvrMsgLayer::Instance().SendToIdipAgentSvr(m_stSsPkg);
}

int EquipLvUp_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_EQUIP_LV_UP_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stEquipLvUpReq;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_EQUIP_LV_UP_RSP;
    SC_PKG_EQUIP_LV_UP_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stEquipLvUpRsp;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    rstScPkgBodyRsp.m_nErrNo = Equip::Instance().LvUp(&poPlayer->GetPlayerData(), rstCsPkgBodyReq, rstScPkgBodyRsp);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    // 任务计数修改，装备进阶
    if (rstScPkgBodyRsp.m_nErrNo == ERR_NONE)
    {
        Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_LVUP, 1/*value*/, 2, 1);
    }

    return 0;
}

int EquipPhaseUp_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_EQUIP_PHASE_UP_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stEquipPhaseUpReq;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_EQUIP_PHASE_UP_RSP;
    SC_PKG_EQUIP_PHASE_UP_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stEquipPhaseUpRsp;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

	int iRet = 0;
	if (rstCsPkgBodyReq.m_bIsTotal)
	{
		Equip::Instance().PhaseUpTotal(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_dwGeneralId, rstScPkgBodyRsp);
	}
	else
	{
		iRet = Equip::Instance().PhaseUp(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_dwSeq, rstScPkgBodyRsp);
		rstScPkgBodyRsp.m_nErrNo = (iRet >= 0) ? ERR_NONE : iRet;
	}

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    if (iRet >= 0)
    {
        Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_EQUIP, 1/*value*/, 1/*装备进阶*/, iRet/*阶数*/);

        // 全身达到x阶
        DT_ITEM_EQUIP* pstItemEquip = Equip::Instance().Find(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_dwSeq);
        if (!pstItemEquip)
        {
            return 0;
        }

        DT_ITEM_GCARD* pstItemGcard = GeneralCard::Instance().Find(&poPlayer->GetPlayerData(), pstItemEquip->m_dwGCardId);
        uint16_t wPhaseMin = 99;
        if (pstItemGcard)
        {
            for (int i=0 ;i<EQUIP_TYPE_MAX_NUM; i++)
            {
                DT_ITEM_EQUIP* pstEquipCmp = Equip::Instance().Find(&poPlayer->GetPlayerData(), pstItemGcard->m_astEquipList[i].m_dwEquipSeq);
                if (pstEquipCmp && (pstEquipCmp->m_bPhase < wPhaseMin))
                {
                    wPhaseMin = pstEquipCmp->m_bPhase;
                }
            }

            Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_GENERAL_EQUIP, 1/*value*/, 1/*全身装备进阶*/, wPhaseMin/*阶数*/);
        }
    }

    return 0;
}

#if 0
int EquipTotalLvUp_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_EQUIP_TOTAL_LV_UP_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stEquipTotalLvUpReq;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_EQUIP_TOTAL_LV_UP_RSP;
    SC_PKG_EQUIP_TOTAL_LV_UP_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stEquipTotalLvUpRsp;
    rstScPkgRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    int iRet = Equip::Instance().TotalLvUp(&poPlayer->GetPlayerData(), rstCsPkgReq.m_dwGeneralId, rstScPkgRsp.m_stSyncItemInfo);
    if (iRet < 0)
    {
        rstScPkgRsp.m_nErrNo = iRet;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return -1;
    }

    rstScPkgRsp.m_nErrNo = ERR_NONE;

    DT_ITEM_GCARD* pstGeneral = GeneralCard::Instance().Find(&poPlayer->GetPlayerData(), rstCsPkgReq.m_dwGeneralId);
    DT_ITEM_EQUIP* pstEquip = NULL;
    for (int i=0; i<EQUIP_TYPE_MAX_NUM; i++)
    {
        pstEquip = Equip::Instance().Find(&poPlayer->GetPlayerData(), pstGeneral->m_astEquipList[i].m_dwEquipSeq);
        if (pstEquip == NULL)
        {
            continue;
        }
        rstScPkgRsp.m_astEquipList[i] = *pstEquip;
    }

    Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_LVUP, iRet/*value*/, 2, 1);
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}
#endif

int EquipUpStar_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_EQUIP_UP_STAR_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stEquipUpStarReq;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_EQUIP_UP_STAR_RSP;
    SC_PKG_EQUIP_UP_STAR_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stEquipUpStarRsp;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    rstScPkgBodyRsp.m_nErrNo = Equip::Instance().UpStar(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_dwSeq, rstScPkgBodyRsp.m_stSyncItemInfo);

    DT_ITEM_EQUIP* pstEquip = Equip::Instance().Find(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_dwSeq);
    if (pstEquip == NULL)
    {
        rstScPkgBodyRsp.m_nErrNo = ERR_NOT_FOUND;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return -1;
    }
    rstScPkgBodyRsp.m_bCurStar = pstEquip->m_wStar;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    // 任务计数
    if (pstEquip)
    {
        Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_LVUP, 1/*value*/, 2/*para1*/, 2/*para2*/);

        Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_EQUIP, 1/*value*/, 2/*装备升星*/, pstEquip->m_wStar/*星数*/);

        // 全身达到x星
        DT_ITEM_GCARD* pstItemGcard = GeneralCard::Instance().Find(&poPlayer->GetPlayerData(), pstEquip->m_dwGCardId);
        uint16_t wStarMin = 99;
        if (pstItemGcard)
        {
            for (int i=0 ;i<EQUIP_TYPE_MAX_NUM; i++)
            {
                DT_ITEM_EQUIP* pstEquipCmp = Equip::Instance().Find(&poPlayer->GetPlayerData(), pstItemGcard->m_astEquipList[i].m_dwEquipSeq);
                if (pstEquipCmp && (pstEquipCmp->m_wStar < wStarMin))
                {
                    wStarMin = pstEquipCmp->m_wStar;
                }
            }

            Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_GENERAL_EQUIP, 1/*value*/, 2/*装备升星*/, wStarMin/*星数*/);
        }
    }

    return 0;
}

int GCardLvUp_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_GCARD_LVUP_REQ& rstCsGCardLvUpReq = rstCsPkg.m_stBody.m_stGCardLvUpReq;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GCARD_LVUP_RSP;
    SC_PKG_GCARD_LVUP_RSP& rstScGCardLvUpRsp = m_stScPkg.m_stBody.m_stGCardLvUpRsp;
    rstScGCardLvUpRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    DT_ITEM_GCARD* pstGeneral = GeneralCard::Instance().Find(&poPlayer->GetPlayerData(), rstCsGCardLvUpReq.m_dwGeneralId);
    if (NULL == pstGeneral)
    {
        LOGERR_r("pstGeneral is null");
        rstScGCardLvUpRsp.m_nErrNo = ERR_NOT_FOUND;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return -1;
    }
    uint32_t dwOldExp = pstGeneral->m_dwExp;
    uint32_t dwOldLv = pstGeneral->m_bLevel;

    int iRet = GeneralCard::Instance().LvUp(&poPlayer->GetPlayerData(), rstCsGCardLvUpReq.m_dwGeneralId, rstCsGCardLvUpReq.m_stConsumeList, rstScGCardLvUpRsp.m_stSyncItemInfo);
    if (iRet < 0)
    {
        rstScGCardLvUpRsp.m_nErrNo = iRet;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return -1;
    }

    // 任务计数
    Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_LVUP, iRet/*value*/, 1, 1);
    for (uint32_t dwLv = dwOldLv; dwLv <= (dwOldLv + iRet); dwLv++)
    {
        Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_GENERAL_LEVEL, 1/*value*/, 1, dwLv);
    }

    rstScGCardLvUpRsp.m_nErrNo = ERR_NONE;
    rstScGCardLvUpRsp.m_bCurLv = pstGeneral->m_bLevel;
    rstScGCardLvUpRsp.m_dwCurExp = pstGeneral->m_dwExp;
    rstScGCardLvUpRsp.m_dwIncExp = rstScGCardLvUpRsp.m_dwCurExp - dwOldExp;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int GCardLvPhaseUp_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_GCARD_LVPHASE_UP_REQ& rstCsGCardLvPhaseUpReq = rstCsPkg.m_stBody.m_stGCardLvPhaseUpReq;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GCARD_LVPHASE_UP_RSP;
    SC_PKG_GCARD_LVPHASE_UP_RSP& rstScGCardLvPhaseUpRsp = m_stScPkg.m_stBody.m_stGCardLvPhaseUpRsp;
    rstScGCardLvPhaseUpRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    int iRet = GeneralCard::Instance().LvPhaseUp(&poPlayer->GetPlayerData(), rstCsGCardLvPhaseUpReq.m_dwGeneralId, rstScGCardLvPhaseUpRsp.m_stSyncItemInfo);

    rstScGCardLvPhaseUpRsp.m_nErrNo = (iRet < 0) ? iRet : ERR_NONE;
    rstScGCardLvPhaseUpRsp.m_bCurLvPhase = iRet;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int GCardStarUp_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_GCARD_STAR_UP_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stGCardStarUpReq;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GCARD_STAR_UP_RSP;
    SC_PKG_GCARD_STAR_UP_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stGCardStarUpRsp;
    rstScPkgBodyRsp.m_dwGeneralId = rstCsPkgBodyReq.m_dwGeneralId;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    int iRet = GeneralCard::Instance().StarUp(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_dwGeneralId, rstScPkgBodyRsp);
    rstScPkgBodyRsp.m_nErrNo = (iRet >= 0) ? ERR_NONE : (int16_t)iRet;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    //拥有X星武将个数
    Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_GENERAL_LEVEL, 1/*value*/, 2, iRet/*当前星级*/);
    //武将升星次数
    Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_LVUP, 1/*value*/, 1/*武将*/, 2/*升星*/) ;
    return 0;
}

int GCardPhaseUp_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_GCARD_PHASE_UP_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stGCardPhaseUpReq;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GCARD_PHASE_UP_RSP;
    SC_PKG_GCARD_PHASE_UP_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stGCardPhaseUpRsp;
    rstScPkgBodyRsp.m_dwGeneralId = rstCsPkgBodyReq.m_dwGeneralId;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    rstScPkgBodyRsp.m_nErrNo = GeneralCard::Instance().PhaseUp(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_dwGeneralId, rstScPkgBodyRsp);
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int GCardSkillLvUp_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG & rstCsPkg, void* pvPara )
{
    CS_PKG_GCARD_SKILL_LVUP_REQ & rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stGCardSkillLvUpReq;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return ERR_SYS;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GCARD_SKILL_LVUP_RSP;
    SC_PKG_GCARD_SKILL_LVUP_RSP & rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stGCardSkillLvUpRsp;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    rstScPkgBodyRsp.m_dwGeneralId = rstCsPkgBodyReq.m_dwGeneralId;
    rstScPkgBodyRsp.m_bSkillId = rstCsPkgBodyReq.m_bSkillId;

    rstScPkgBodyRsp.m_nErrNo = GeneralCard::Instance().SkillLvUp(&poPlayer->GetPlayerData(), rstCsPkgBodyReq, rstScPkgBodyRsp);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

	if (ERR_NONE == rstScPkgBodyRsp.m_nErrNo)
	{
		Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_LVUP, 1/*value*/, 4/*para1*/, 1/*para2*/);
	}

    return 0;
}

int GCardComposite_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_GCARD_COMPOSITE_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stGCardCompositeReq;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GCARD_COMPOSITE_RSP;
    SC_PKG_GCARD_COMPOSITE_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stGCardCompositeRsp;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    rstScPkgBodyRsp.m_nErrNo = GeneralCard::Instance().Composite(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_dwFragId, rstScPkgBodyRsp);

    if (!rstScPkgBodyRsp.m_nErrNo)
    {
        // 任务计数修改，武将主动技升级
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

//  兵种升级
int GCardArmyLvUpReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    // 预处理
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);

    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_GCARD_ARMY_LVUP_REQ& rstCSPkgBodyReq = rstCsPkg.m_stBody.m_stGCardArmyLvUpReq;
    SC_PKG_GCARD_ARMY_LVUP_RSP& rstSCPkgBodyRsp =  m_stScPkg.m_stBody.m_stGCardArmyLvUpRsp;
	rstSCPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    // 功能处理
	int iRet = ERR_NONE;
	if (rstCSPkgBodyReq.m_bIsTotal)
	{
		iRet = GeneralCard::Instance().ArmyLvUpTotal(&poPlayer->GetPlayerData(), rstCSPkgBodyReq.m_dwGeneralId, rstSCPkgBodyRsp);
	}
	else
	{
		iRet = GeneralCard::Instance().ArmyLvUp(&poPlayer->GetPlayerData(), rstCSPkgBodyReq.m_dwGeneralId, rstSCPkgBodyRsp);
	}

    // 任务计数
    if (iRet > 0)
    {
        Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_LVUP, 1 /*value*/, 3, 1);
    }

    // 填写回复包
    rstSCPkgBodyRsp.m_nErrNo = (iRet >= 0) ? ERR_NONE : iRet;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GCARD_ARMY_LVUP_RSP;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

//  兵种进阶
int GCardArmyPhaseUpReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    //预处理
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_GCARD_ARMY_PHASEUP_REQ& rstCSPkgBodyReq = rstCsPkg.m_stBody.m_stGCardArmyPhaseUpReq;
    SC_PKG_GCARD_ARMY_PHASEUP_RSP& rstSCPkgBodyRsp =  m_stScPkg.m_stBody.m_stGCardArmyPhaseUpRsp;


    //功能处理
    int iRet = GeneralCard::Instance().ArmyPhaseUp(&poPlayer->GetPlayerData(), rstCSPkgBodyReq.m_dwGeneralId, rstSCPkgBodyRsp);

    // 任务计数，只能一级一级升阶
    if (iRet >= 0)
    {
        Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_LVUP, 1 /*value*/, 3, 3);
        Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_GENERAL_ARMY, 1/*value*/, iRet/*阶数*/);
    }

    //填写回复包
    rstSCPkgBodyRsp.m_nErrNo = (iRet>=0) ? ERR_NONE : (int16_t)iRet;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GCARD_ARMY_PHASEUP_RSP;
    iRet = ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int CheatsLvUp_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_CHEATS_LVUP_REQ & rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stCheatsLvUpReq;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CHEATS_LVUP_RSP;
    SC_PKG_CHEATS_LVUP_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stCheatsLvUpRsp;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    rstScPkgBodyRsp.m_nErrNo = GeneralCard::Instance().CheatsLvUp(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_dwGeneralId, rstCsPkgBodyReq.m_bCheatsType, rstScPkgBodyRsp);
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int CheatsChg_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_CHEATS_CHG_REQ & rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stCheatsChgReq;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CHEATS_CHG_RSP;
    SC_PKG_CHEATS_CHG_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stCheatsChgRsp;
    rstScPkgBodyRsp.m_nErrNo = GeneralCard::Instance().CheatsChg(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_dwGeneralId, rstCsPkgBodyReq.m_bCheatsType);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int GCardReborn_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_GCARD_REBORN_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stGCardRebornReq;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GCARD_REBORN_RSP;
    SC_PKG_GCARD_REBORN_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stGCardRebornRsp;

    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    int iRet = GeneralReborn::Instance().Reborn(rstCsPkgReq.m_dwGeneralId, rstCsPkgReq.m_bType, &rstScPkgBodyRsp.m_stSyncItemInfo,
        &poPlayer->GetPlayerData());

    rstScPkgBodyRsp.m_nErrNo = iRet;
    if (iRet != ERR_NONE)
    {
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return 0;
    }
    DT_ITEM_GCARD* pstGeneral = GeneralCard::Instance().Find(&poPlayer->GetPlayerData(), rstCsPkgReq.m_dwGeneralId);
    rstScPkgBodyRsp.m_stGeneralInfo = *pstGeneral;

    for (int i=0; i<MAX_EQUIP_TYPE_NUM; i++)
    {
        uint32_t dwSeq = pstGeneral->m_astEquipList[i].m_dwEquipSeq;
        DT_ITEM_EQUIP* pstEquip = Equip::Instance().Find(&poPlayer->GetPlayerData(), dwSeq);
        if (!pstEquip)
        {
            LOGERR("equip<%d> not found", dwSeq);
        }
        else
        {
            rstScPkgBodyRsp.m_astEquipInfo[i] = *pstEquip;
        }
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int GCardFeedTrain_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
	CS_PKG_GCARD_TRAIN_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stGCardTrainReq;
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GCARD_TRAIN_RSP;
	SC_PKG_GCARD_TRAIN_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stGCardTrainRsp;
	rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

	rstScPkgBodyRsp.m_nErrNo = GeneralCard::Instance().FeedTrain(&poPlayer->GetPlayerData(), rstCsPkgBodyReq, rstScPkgBodyRsp);

	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

	return 0;
}

int GCardTrainLvUp_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
	CS_PKG_TRAIN_LVUP_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stGCardTrainLvUpReq;
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_TRAIN_LVUP_RSP;
	SC_PKG_TRAIN_LVUP_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stGCardTrainLvUpRsp;
	rstScPkgBodyRsp.m_stRewardItemInfo.m_bSyncItemCount = 0;

	rstScPkgBodyRsp.m_nErrNo = GeneralCard::Instance().TrainLvUp(&poPlayer->GetPlayerData(), rstCsPkgBodyReq, rstScPkgBodyRsp);

	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

	return 0;
}

int LotteryDraw_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);

    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    uint8_t bDrawType = rstCsPkg.m_stBody.m_stLotteryDrawReq.m_bDrawType;
    uint8_t bIsFree = rstCsPkg.m_stBody.m_stLotteryDrawReq.m_bIsFree;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_LOTTERY_DRAW_RSP;
    SC_PKG_LOTTERY_DRAW_RSP& rstScPkgBodyRsp =  m_stScPkg.m_stBody.m_stLotteryDrawRsp;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    if (bDrawType == LOTTERY_DRAW_TYPE_ACT || bDrawType == LOTTERY_DRAW_TYPE_ACT_CNT)
    {
        rstScPkgBodyRsp.m_nErrNo = Lottery::Instance().HandleLotteryDrawByAct(&poPlayer->GetPlayerData(), bDrawType, rstScPkgBodyRsp);
    }
    else if (bDrawType == LOTTERY_DRAW_TYPE_EQUIP || bDrawType == LOTTERY_DRAW_TYPE_EQUIP_CNT)
    {
        rstScPkgBodyRsp.m_nErrNo = Lottery::Instance().HandleAwakeEquipLottery(&poPlayer->GetPlayerData(), bDrawType, bIsFree, rstScPkgBodyRsp);
    }
    else
    {
        rstScPkgBodyRsp.m_nErrNo = Lottery::Instance().HandleLotteryDrawByNormal(&poPlayer->GetPlayerData(), bDrawType, bIsFree, rstScPkgBodyRsp);
    }




    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int LotteryInfo_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    Lottery::Instance().HandleLotteryInfo(&poPlayer->GetPlayerData(), m_stScPkg);
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int TutorialDataSyn_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_TUTORIAL_DATA_SYN& rstTutorialDataSyn = rstCsPkg.m_stBody.m_stTutorialDataSyn;

    Tutorial::Instance().RecordTutorialData(&poPlayer->GetPlayerData(), rstTutorialDataSyn.m_bFlag, rstTutorialDataSyn.m_ullDataValue);

    return 0;
}

int TutorialLotteryReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    //先记录新手节点信息
    CS_PKG_TUTORIAL_LOTTERY_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stTutorialLotteryReq;
    Tutorial::Instance().RecordTutorialData(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_bFlag, rstCsPkgBodyReq.m_ullDataValue);

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_LOTTERY_DRAW_RSP;
    SC_PKG_LOTTERY_DRAW_RSP& rstScPkgBodyRsp =  m_stScPkg.m_stBody.m_stLotteryDrawRsp;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    uint8_t bDrawType = rstCsPkgBodyReq.m_bDrawType;
    uint8_t bIsFree = rstCsPkgBodyReq.m_bIsFree;
    rstScPkgBodyRsp.m_nErrNo = Lottery::Instance().HandleLotteryDrawByNormal(&poPlayer->GetPlayerData(), bDrawType, bIsFree, rstScPkgBodyRsp);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int TutorialGCardLvUpReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    //先记录新手节点信息
    CS_PKG_TUTORIAL_GCARD_LVUP_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stTutorialGCardLvUpReq;
    Tutorial::Instance().RecordTutorialData(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_bFlag, rstCsPkgBodyReq.m_ullDataValue);

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GCARD_LVUP_RSP;
    SC_PKG_GCARD_LVUP_RSP& rstScGCardLvUpRsp = m_stScPkg.m_stBody.m_stGCardLvUpRsp;
    rstScGCardLvUpRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    DT_ITEM_GCARD* pstGeneral = GeneralCard::Instance().Find(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_dwGeneralId);
    if (NULL == pstGeneral)
    {
        LOGERR_r("pstGeneral is null");
        rstScGCardLvUpRsp.m_nErrNo = ERR_NOT_FOUND;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return -1;
    }
    uint32_t dwOldExp = pstGeneral->m_dwExp;
    uint32_t dwOldLv = pstGeneral->m_bLevel;

    int iRet = GeneralCard::Instance().LvUp(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_dwGeneralId, rstCsPkgBodyReq.m_stConsumeList, rstScGCardLvUpRsp.m_stSyncItemInfo);
    if (iRet < 0)
    {
        rstScGCardLvUpRsp.m_nErrNo = iRet;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return -1;
    }

    // 任务计数
    Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_LVUP, iRet/*value*/, 1, 1);

    for (uint32_t dwLv = dwOldLv; dwLv <= (dwOldLv + iRet); dwLv++)
    {
        Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_GENERAL_LEVEL, 1/*value*/, 1, dwLv);
    }

    rstScGCardLvUpRsp.m_nErrNo = ERR_NONE;
    rstScGCardLvUpRsp.m_bCurLv = pstGeneral->m_bLevel;
    rstScGCardLvUpRsp.m_dwCurExp = pstGeneral->m_dwExp;
    rstScGCardLvUpRsp.m_dwIncExp = rstScGCardLvUpRsp.m_dwCurExp - dwOldExp;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int TutorialEquipLvUpReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_TUTORIAL_EQUIP_LVUP_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stTutorialEquipLvUpReq;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_EQUIP_LV_UP_RSP;
    SC_PKG_EQUIP_LV_UP_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stEquipLvUpRsp;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    CS_PKG_EQUIP_LV_UP_REQ stEquipLvUpReq;
    stEquipLvUpReq.m_dwSeq = rstCsPkgBodyReq.m_dwSeq;
    stEquipLvUpReq.m_stConsumeList = rstCsPkgBodyReq.m_stConsumeList;

    rstScPkgBodyRsp.m_nErrNo = Equip::Instance().LvUp(&poPlayer->GetPlayerData(), stEquipLvUpReq, rstScPkgBodyRsp);
    if (rstScPkgBodyRsp.m_nErrNo == ERR_NONE)
    {
         //记录新手节点信息
        Tutorial::Instance().RecordTutorialData(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_bFlag, rstCsPkgBodyReq.m_ullDataValue);
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    // 任务计数修改，装备升级
    if (rstScPkgBodyRsp.m_nErrNo == ERR_NONE)
    {
        Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_LVUP, 1/*value*/, 2, 1);
    }

    return 0;
}

int TutorialGCardCompositeReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    //先记录新手节点信息
    CS_PKG_TUTORIAL_GCARD_COMPOSITE_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stTutorialGcardCompositeReq;
    Tutorial::Instance().RecordTutorialData(&poPlayer->GetPlayerData(), rstCsPkgReq.m_bFlag, rstCsPkgReq.m_ullDataValue);

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GCARD_COMPOSITE_RSP;
    SC_PKG_GCARD_COMPOSITE_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stGCardCompositeRsp;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    rstScPkgBodyRsp.m_nErrNo = GeneralCard::Instance().Composite(&poPlayer->GetPlayerData(), rstCsPkgReq.m_dwFragId, rstScPkgBodyRsp);

    if (!rstScPkgBodyRsp.m_nErrNo)
    {
        //任务计数修改，武将主动技升级
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int TutorialBonusReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);

    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_TUTORIAL_BONUS_RSP;
    SC_PKG_TUTORIAL_BONUS_RSP& rstScPkgBodyRsp =  m_stScPkg.m_stBody.m_stTutorialBonusRsp;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;


    // 获取武将
    rstScPkgBodyRsp.m_bBonusCnt = rstCsPkg.m_stBody.m_stTutorialBonusReq.m_bBonusCnt;

    rstScPkgBodyRsp.m_nErrNo = (int16_t)Tutorial::Instance().DrawBonus(&poPlayer->GetPlayerData(),
        rstScPkgBodyRsp.m_bBonusCnt, rstScPkgBodyRsp.m_stSyncItemInfo);

    DT_ROLE_MISC_INFO& rstInfo = poPlayer->GetPlayerData().GetMiscInfo();
    LOGRUN("tutorial bonus, account-%s, step-%lu, bonus-%hhu", poPlayer->GetAccountName(), rstInfo.m_ullTutorialStep, rstScPkgBodyRsp.m_bBonusCnt);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int Purchase_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_PURCHASE_REQ & rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stPurchaseReq;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);

    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PURCHASE_RSP;
    SC_PKG_PURCHASE_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stPurchaseRsp;
    rstScPkgBodyRsp.m_bItemType = rstCsPkgBodyReq.m_bItemType;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    switch (rstCsPkgBodyReq.m_bItemType)
    {
    case ITEM_TYPE_PROPS:
        {
            rstScPkgBodyRsp.m_nErrNo = Props::Instance().PurchaseProps(&poPlayer->GetPlayerData(),
                rstCsPkgBodyReq.m_dwItemId, rstCsPkgBodyReq.m_dwItemNum, rstScPkgBodyRsp);
        }
        break;
    case ITEM_TYPE_GOLD:
        {
            rstScPkgBodyRsp.m_nErrNo = Consume::Instance().PurchaseGold(&poPlayer->GetPlayerData(),
                rstCsPkgBodyReq.m_dwItemNum, rstScPkgBodyRsp);
        }
        break;
    case ITEM_TYPE_AP:
        {
            rstScPkgBodyRsp.m_nErrNo = AP::Instance().PurchaseAp(&poPlayer->GetPlayerData(),
                rstCsPkgBodyReq.m_dwItemNum, rstScPkgBodyRsp);
        }
        break;
    default:
        break;
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int GetTopList_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_RANK_GET_TOPLIST_REQ & rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stRankTopListReq;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);

    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    bzero(&m_stScPkg, sizeof(m_stScPkg));

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_RANK_GET_TOPLIST_RSP;
    SC_PKG_RANK_GET_TOPLIST_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stRankTopListRsp;

    if (rstCsPkgBodyReq.m_chType == MATCH_TYPE_6V6)
    {
        Fight6V6::Instance().GetTopList(rstScPkgBodyRsp.m_astTopList, &rstScPkgBodyRsp.m_bCount);
    }
    else
    {

    }

    rstScPkgBodyRsp.m_nErrNo = ERR_NONE;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int MajestyChgImageId_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_MAJESTY_ITEM_CHANGE_REQ & rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stMajestyItemChangeReq;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MAJESTY_ITEM_CHANGE_RSP;
    SC_PKG_MAJESTY_ITEM_CHANGE_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stMajestyItemChangeRsp;
    rstScPkgBodyRsp.m_nErrNo = Majesty::Instance().ChgImageId(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_dwId);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int OpenBox_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_OPEN_BOX_REQ & rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stOpenBoxReq;
    uint32_t dwPropsId = rstCsPkgBodyReq.m_dwBoxID;
    int32_t iCount = (int32_t)rstCsPkgBodyReq.m_wCount;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_OPEN_BOX_RSP;
    SC_PKG_OPEN_BOX_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stOpenBoxRsp;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

	if ( Props::Instance().IsLevelBox(dwPropsId) )
	{
		rstScPkgBodyRsp.m_nErrNo = Props::Instance().OpenLevelBox(&poPlayer->GetPlayerData(), dwPropsId, iCount,
			rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_OPEN_BOX);
	}
	else
	{
		rstScPkgBodyRsp.m_nErrNo = Props::Instance().OpenTreasureBox(&poPlayer->GetPlayerData(), dwPropsId, iCount,
			rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_OPEN_BOX);
	}

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 1;
}

int PropDecomposition_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

	CS_PKG_PROP_DECOMPOSITION_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stPropDecompositionReq;
	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PROP_DECOMPOSITION_RSP;
	SC_PKG_PROP_DECOMPOSITION_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stPropDecompositionRsp;

	//分解逻辑
	DT_CONSUME_ITEM_INFO& rstConsumeItemInfo = rstCsPkgBodyReq.m_stConsumeItemInfo;
	DT_SYNC_ITEM_INFO& rstSyncItemInfo = rstScPkgBodyRsp.m_stSyncItemInfo;
	rstSyncItemInfo.m_bSyncItemCount = 0;
	PlayerData* pstData = &(poPlayer->GetPlayerData());
	ResPropsMgr_t& rResPropsMgr = CGameDataMgr::Instance().GetResPropsMgr();

	uint32_t dwCoin = 0;
	for (int i=0; i<rstConsumeItemInfo.m_bConsumeCount; i++)
	{
		DT_ITEM_CONSUME& rstItemConsume = rstConsumeItemInfo.m_astConsumeList[i];
		RESPROPS* pResProps = rResPropsMgr.Find(rstItemConsume.m_dwItemId);
		uint32_t m_dwAnalyzeNum = pResProps->m_dwAnalyzeNum;
		dwCoin += m_dwAnalyzeNum*rstItemConsume.m_dwItemNum;
		Item::Instance().ConsumeItem(pstData, rstItemConsume.m_bItemType, rstItemConsume.m_dwItemId, -rstItemConsume.m_dwItemNum, rstSyncItemInfo, METHOD_PROP_DECOMPOSITION);
	}

	switch (rstCsPkgBodyReq.m_bType)
	{
	case 1:  //武将代币
		rstScPkgBodyRsp.m_bType = 1;
		rstScPkgBodyRsp.m_nErrNo = ERR_NONE;
		Item::Instance().RewardItem(pstData, ITEM_TYPE_GENERAL_COIN, 0, dwCoin, rstSyncItemInfo, METHOD_PROP_DECOMPOSITION);
		break;
	case 2:  //装备代币
		rstScPkgBodyRsp.m_bType = 2;
		rstScPkgBodyRsp.m_nErrNo = ERR_NONE;
		Item::Instance().RewardItem(pstData, ITEM_TYPE_EQUIP_COIN, 0, dwCoin, rstSyncItemInfo, METHOD_PROP_DECOMPOSITION);
		break;
	default:
		rstScPkgBodyRsp.m_nErrNo = ERR_DEFAULT;
		break;
	}

	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

	return 0;
}

int UploadReplay_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_UPLOAD_REPLAY_REQ & rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stUpLoadReplayReq;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_UPLOAD_REPLAY_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    SS_PKG_UPLOAD_REPLAY_REQ& rstSsPkgBodyReq = m_stSsPkg.m_stBody.m_stUploadReplayReq;
    memcpy(&rstSsPkgBodyReq.m_stFileHead, &rstCsPkgBodyReq.m_stFileHead, sizeof(DT_REPLAY_RECORD_FILE_HEADER));

    if (rstCsPkgBodyReq.m_stFileHead.m_bMatchType == MATCH_TYPE_GUILD_PVP)
    {
        //如果是公会约战录像，发给GuildSvr处理
        m_stSsPkg.m_stHead.m_ullReservId = poPlayer->GetPlayerData().GetGuildInfo().m_ullGuildId;
        ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    }
    else
    {
        //世界录像发给ReplaySvr处理
        ZoneSvrMsgLayer::Instance().SendToReplaySvr(m_stSsPkg);
    }

    return 0;
}

int UploadReplay_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_UPLOAD_REPLAY_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stUploadReplayRsp;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGERR("poPlayer is null");
        return -1;
    }

    //直接将ReplaySvr的结果发给Clinet
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_UPLOAD_REPLAY_RSP;
    SC_PKG_UPLOAD_REPLAY_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stUpLoadReplayRsp;
    rstScPkgBodyRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;
    StrCpy(rstScPkgBodyRsp.m_szURL, rstSsPkgBodyRsp.m_szURL, MAX_LEN_URL);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int RefreshReplay_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_REFRESH_REPLAYLIST_REQ & rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stRefreshReplayListReq;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_REFRESH_REPLAYLIST_RSP;
    SC_PKG_REFRESH_REPLAYLIST_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stRefreshReplayListRsp;

    Replay::Instance().GetReplayList(rstCsPkgBodyReq, rstScPkgBodyRsp);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int RefreshReplayNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_REFRESH_REPLAYLIST_NTF& rstSsPkgBodyNtf = rstSsPkg.m_stBody.m_stRefreshReplayNtf;

    Replay::Instance().UpdateReplay(rstSsPkgBodyNtf);

    return 0;
}

//  七天签到
int Sign7dClick_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_SIGN7D_CLICK_RSP;
    SC_PKG_SIGN7D_CLICK_RSP& rstScPkgBodyRsp =  m_stScPkg.m_stBody.m_stSign7dClickRsp;
    //不置0,奖励会有问题.

    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    rstScPkgBodyRsp.m_nErrNo = Sign::Instance().HandleSign7dAward(&poPlayer->GetPlayerData(), rstScPkgBodyRsp);
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

//  月签到
int Sign30dClick_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);

    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    SC_PKG_SIGN30D_CLICK_RSP& rstScPkgBodyRsp =  m_stScPkg.m_stBody.m_stSign30dClickRsp;

    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    int iRet = Sign::Instance().HandleSign30dAward(&poPlayer->GetPlayerData(), rstCsPkg.m_stBody.m_stSign30dClickReq, rstScPkgBodyRsp);

    if (iRet == ERR_NONE && rstCsPkg.m_stBody.m_stSign30dClickReq.m_bType == SIGN30D_TYPE_NORMAL)
    {
        Task::Instance().ModifyData(&(poPlayer->GetPlayerData()), TASK_VALUE_TYPE_LOGIN, 1);
    }
    
    //填写回复包
    rstScPkgBodyRsp.m_nErrNo = iRet;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_SIGN30D_CLICK_RSP;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

//  宝石穿上/替换
int GemUpReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    //预处理
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_GEM_UP_REQ& rstCSPkgBodyReq = rstCsPkg.m_stBody.m_stGemUpReq;
    SC_PKG_GEM_UP_RSP& rstSCPkgBodyRsp =  m_stScPkg.m_stBody.m_stGemUpRsp;


    //功能处理
    int iRet = Gem::Instance().Up(&poPlayer->GetPlayerData(), rstCSPkgBodyReq);


    //填写回复包
    rstSCPkgBodyRsp.m_nErrNo = iRet;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GEM_UP_RSP;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

//  宝石卸下
int GemDownReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    //预处理
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_GEM_DOWN_REQ& rstCSPkgBodyReq = rstCsPkg.m_stBody.m_stGemDownReq;
    SC_PKG_GEM_DOWN_RSP& rstSCPkgBodyRsp =  m_stScPkg.m_stBody.m_stGemDownRsp;


    //功能处理
    int iRet = Gem::Instance().Down(&poPlayer->GetPlayerData(), rstCSPkgBodyReq);


    //填写回复包
    rstSCPkgBodyRsp.m_nErrNo = iRet;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GEM_DOWN_RSP;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

//  宝石合成  一个或全部
int GemSyntheticReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    //预处理
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_GEM_SYNTHETIC_REQ& rstCSPkgBodyReq = rstCsPkg.m_stBody.m_stGemSyntheticReq;
    SC_PKG_GEM_SYNTHETIC_RSP& rstSCPkgBodyRsp =  m_stScPkg.m_stBody.m_stGemSyntheticRsp;


    //功能处理
    int iRet = Gem::Instance().Synthetic(&poPlayer->GetPlayerData(), rstCSPkgBodyReq, rstSCPkgBodyRsp);


    //填写回复包
    rstSCPkgBodyRsp.m_nErrNo = iRet;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GEM_SYNTHETIC_RSP;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

//  成长奖励  包括 等级成长 在线时间 的奖励
int GrowthAwardHandleReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    //预处理
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_GROWTH_AWARD_REQ& rstCSPkgBodyReq = rstCsPkg.m_stBody.m_stGrowthAwardReq;
    SC_PKG_GROWTH_AWARD_RSP& rstSCPkgBodyRsp =  m_stScPkg.m_stBody.m_stGrowthAwardRsp;
    int iRet = ERR_NONE;
    switch (rstCSPkgBodyReq.m_bType)
    {
    case GROWTH_AWARD_TYPE_LEVEL:
        iRet = GrowthAward::Instance().GrowthAwardLevel(&poPlayer->GetPlayerData(), rstCSPkgBodyReq, rstSCPkgBodyRsp);
        break;
    case GROWTH_AWARD_TYPE_ONLINE:
        iRet = GrowthAward::Instance().GrowthAwardOnline(&poPlayer->GetPlayerData(), rstCSPkgBodyReq, rstSCPkgBodyRsp);
        break;
    default:
        iRet = ERR_WRONG_PARA;
        break;
    }


    //填写回复包
    rstSCPkgBodyRsp.m_nErrNo = iRet;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GROWTH_AWARD_RSP;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

//领取级基金奖励
int GetLvFundAwardReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_GET_LV_FUND_AWARD_REQ& rstCSPkgBodyReq = rstCsPkg.m_stBody.m_stGetLvFundAwardReq;
    SC_PKG_GET_LV_FUND_AWARD_RSP& rstSCPkgBodyRsp = m_stScPkg.m_stBody.m_stGetLvFundAwardRsp;

    int iRet = GrowthAward::Instance().GetLvFundAward(&poPlayer->GetPlayerData(), rstCSPkgBodyReq, rstSCPkgBodyRsp);

    //填写回复包
    rstSCPkgBodyRsp.m_nErrNo = iRet;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GET_LV_FUND_AWARD_RSP;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

//购买等级基金
int BuyLvFundReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
    //预处理
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_BUY_LV_FUND_REQ& rstCSPkgBodyReq = rstCsPkg.m_stBody.m_stBuyLvFunddReq;
    SC_PKG_BUY_LV_FUND_RSP& rstSCPkgBodyRsp = m_stScPkg.m_stBody.m_stBuyLvFundRsp;

    int iRet = GrowthAward::Instance().BuyLvFund(&poPlayer->GetPlayerData(), rstCSPkgBodyReq, rstSCPkgBodyRsp);

    //填写回复包
    rstSCPkgBodyRsp.m_nErrNo = iRet;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_BUY_LV_FUND_RSP;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);


    return 0;
}


int SerialNumReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_SERIAL_NUM_REQ& rstCsSerialReq = rstCsPkg.m_stBody.m_stSerialNumReq;
    rstCsSerialReq.m_szSerialNum[PKGMETA::MAX_SERIAL_NUM_LEN - 1] = '\0';
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_CHECK_SERIAL_NUM_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
	m_stSsPkg.m_stHead.m_ullReservId = 0;
    int iRet = Serial::Instance().CheckDrawSerail(&poPlayer->GetPlayerData(), rstCsSerialReq.m_szSerialNum);
    if (iRet != ERR_NONE)
    {
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_SERIAL_NUM_RSP;
        SC_PKG_SERIAL_NUM_RSP& rstScSerialRsp = m_stScPkg.m_stBody.m_stSerialNumRsp;
        rstScSerialRsp.m_nErrNo = iRet;
        rstScSerialRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return -1;
    }

    SS_PKG_CHECK_SERIAL_NUM_REQ& rstSsSerialReq = m_stSsPkg.m_stBody.m_stCheckSerialNumReq;
    StrCpy(rstSsSerialReq.m_szSerialNum, rstCsSerialReq.m_szSerialNum, PKGMETA::MAX_SERIAL_NUM_LEN);

    return ZoneSvrMsgLayer::Instance().SendToSerialNumSvr(m_stSsPkg);
}


int CheckSerialNum_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        return -1;
    }

    SS_PKG_CHECK_SERIAL_NUM_RSP& rstSsCheckSerialRsp = rstSsPkg.m_stBody.m_stCheckSerialNumRsp;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_SERIAL_NUM_RSP;
    SC_PKG_SERIAL_NUM_RSP& rstScSerialRsp = m_stScPkg.m_stBody.m_stSerialNumRsp;
    rstScSerialRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    rstScSerialRsp.m_nErrNo = rstSsCheckSerialRsp.m_nErrNo;

    if (rstScSerialRsp.m_nErrNo == ERR_NONE)
    {
        if (rstSsCheckSerialRsp.m_stSerialInfo.m_bRepeatFlag == 0)
        {
            Serial::Instance().AddUsedSerial(&poPlayer->GetPlayerData(), (uint16_t)rstSsCheckSerialRsp.m_stSerialInfo.m_dwSerialId);
        }

        for (int i=0; i<rstSsCheckSerialRsp.m_stSerialInfo.m_bRewardCnt; i++)
        {
            Item::Instance().RewardItem(&poPlayer->GetPlayerData(), rstSsCheckSerialRsp.m_stSerialInfo.m_astRewardList[i].m_bItemType,
                 rstSsCheckSerialRsp.m_stSerialInfo.m_astRewardList[i].m_dwItemId, rstSsCheckSerialRsp.m_stSerialInfo.m_astRewardList[i].m_dwItemNum,
                 rstScSerialRsp.m_stSyncItemInfo, METHOD_SERIAL_NUM);
        }
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}


int GmReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    SC_PKG_GM_RSP& rstSCPkgBodyRsp = m_stScPkg.m_stBody.m_stGmRsp;
    rstSCPkgBodyRsp.m_stMarqueeOnTimeInfo.m_iCount = 0;
    int iRet = GmMgr::Instance().HandleCsMsg(&poPlayer->GetPlayerData(), rstCsPkg.m_stBody.m_stGmReq, rstSCPkgBodyRsp);
    if (iRet > 0)
    {
        //表示不用在这里回包
        return 0;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GM_RSP;
    rstSCPkgBodyRsp.m_nErrNo = iRet;
    rstSCPkgBodyRsp.m_bType = rstCsPkg.m_stBody.m_stGmReq.m_bType;

    return ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
}

int GmReq_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_GM_RSP &rstSsPkgBodyRsp = m_stSsPkg.m_stBody.m_stGmRsp;
    rstSsPkgBodyRsp.m_stMarqueeOnTimeInfo.m_iCount = 0;
    int iRet = GmMgr::Instance().HandleSsMsg(rstSsPkg.m_stBody.m_stGmReq, rstSsPkgBodyRsp);
    LOGRUN("Type<%d>, iRet<%d>", rstSsPkg.m_stBody.m_stGmReq.m_bType, iRet);
    if (iRet > 0)
    {
        //表示不用在这里回包
        return 0;
    }

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GM_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    rstSsPkgBodyRsp.m_nErrNo = iRet;
    rstSsPkgBodyRsp.m_bType = rstSsPkg.m_stBody.m_stGmReq.m_bType;

    return ZoneSvrMsgLayer::Instance().SendToIdipAgentSvr(m_stSsPkg);
}

int GmRoleUpdateInfoRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_ROLE_GM_UPDATE_INFO_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stRoleGmUpdateInfoRsp;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGERR("poPlayer<%lu> is null", rstSsPkg.m_stHead.m_ullUin);
        return -1;
    }


    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GM_RSP;
    SC_PKG_GM_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stGmRsp;
    rstScPkgBodyRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;
    StrCpy(rstSsPkgBodyRsp.m_szResult, rstSsPkgBodyRsp.m_szResult, 1024);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    LOGWARN("Gm get cmd<%s>",  rstSsPkgBodyRsp.m_szResult);
    return 0;
}
int RoleWebTaskReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
	int iRet = 0;
	SS_PKG_ROLE_WEB_TASK_INFO_REQ& rstSsPkgBodyReq = rstSsPkg.m_stBody.m_stRoleWebTaskInfoReq;
	Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
	if (!poPlayer)
	{
		LOGERR("poPlayer<%lu> is null", rstSsPkg.m_stHead.m_ullUin);
		m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ROLE_WEB_TASK_INFO_RSP;
		SS_PKG_ROLE_WEB_TASK_INFO_RSP& rstSsPkgBodyRsp = m_stSsPkg.m_stBody.m_stRoleWebTaskInfoRsp;
		rstSsPkgBodyRsp.m_bCount = 0;
		m_stSsPkg.m_stHead.m_ullReservId = rstSsPkg.m_stHead.m_ullReservId;
		rstSsPkgBodyRsp.m_nErrno = ERR_NOT_FOUND;
		return ZoneSvrMsgLayer::Instance().SendToZoneHttpConn(m_stSsPkg);

	}
	uint64_t ullUin = rstSsPkg.m_stHead.m_ullUin;
	uint8_t bTypeValue = rstSsPkg.m_stBody.m_stRoleWebTaskInfoReq.m_bTaskType;
	uint8_t bCount = 0;
	list<DT_TASK_INFO*> task_list;
	Task::Instance().GetWebTask(ullUin,poPlayer,task_list);

	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ROLE_WEB_TASK_INFO_RSP;
	SS_PKG_ROLE_WEB_TASK_INFO_RSP& rstSsPkgBodyRsp = m_stSsPkg.m_stBody.m_stRoleWebTaskInfoRsp;
	m_stSsPkg.m_stHead.m_ullReservId = rstSsPkg.m_stHead.m_ullReservId;
	rstSsPkgBodyRsp.m_bCount = task_list.size();
	for (list<DT_TASK_INFO*>::iterator it = task_list.begin(); it != task_list.end(); it++)
	{
		rstSsPkgBodyRsp.m_astTaskInfo[bCount++] = *(*it);
	}
	rstSsPkgBodyRsp.m_nErrno = iRet;
	return ZoneSvrMsgLayer::Instance().SendToZoneHttpConn(m_stSsPkg);
}
int RoleWebTaskRwdReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
	int iRet = ERR_NONE;
	do
	{
		SS_PKG_WEB_TASK_GET_RWD_REQ & rstWebtaskGetRwd = rstSsPkg.m_stBody.m_stRoleGetWebTaskRwdReq;
		Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
		if (!poPlayer)
		{
			LOGERR("poPlayer<%lu> is null", rstSsPkg.m_stHead.m_ullUin);
			iRet = ERR_NOT_FOUND;
			break;
		}
		uint32_t dwId = rstWebtaskGetRwd.m_dwTaskId;
		PlayerData * poPlayerData = &poPlayer->GetPlayerData();
		DT_TASK_INFO* pstTaskInfo = Task::Instance().GetPlayerTaskById(poPlayerData, dwId);
		if (!pstTaskInfo)
		{
			iRet = ERR_NOT_FOUND;
			LOGERR("pstTaskInfo is null");
			break;
		}
		if (pstTaskInfo->m_bIsDrawed == COMMON_AWARD_STATE_DRAWED)
		{
			iRet = ERR_AWARD_FINISHED;
			LOGERR("task %d has been Drawed", dwId);
			break;
		}
		if (pstTaskInfo->m_bIsDrawed != COMMON_AWARD_STATE_AVAILABLE)
		{
			iRet = ERR_NOT_SATISFY_COND;
			break;
		}
		//发送网页任务奖励
		RESPRIMAIL* poResPriMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(10205);
		if (!poResPriMail)
		{
			LOGERR("Can't find the mail <10205>");
			iRet = -1;
			break;
		}
		m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
		SS_PKG_MAIL_ADD_REQ& rstMailAddReq = m_stSsPkg.m_stBody.m_stMailAddReq;
		rstMailAddReq.m_nUinCount = 1;
		rstMailAddReq.m_UinList[0] = rstWebtaskGetRwd.m_ullUin;
		DT_MAIL_DATA& rstMailData = rstMailAddReq.m_stMailData;
		rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
		rstMailData.m_bState = MAIL_STATE_UNOPENED;
		StrCpy(rstMailData.m_szTitle, poResPriMail->m_szTitle, MAX_MAIL_TITLE_LEN);
		StrCpy(rstMailData.m_szContent, poResPriMail->m_szContent, MAX_MAIL_CONTENT_LEN);
		rstMailData.m_ullFromUin = 0;
        rstMailData.m_ullTimeStampMs = CGameTime::Instance().GetCurrSecond();

		ResTaskMgr_t& rstResTaskMgr = CGameDataMgr::Instance().GetResTaskMgr();
		RESTASK* pstResReward = rstResTaskMgr.Find(dwId);
		if (!pstResReward)
		{
			iRet = ERR_NOT_FOUND;
			break;
		}
		rstMailData.m_bAttachmentCount = pstResReward->m_bRewardCnt;
		for (int k = 0; k < pstResReward->m_bRewardCnt; k++)
		{
			rstMailData.m_astAttachmentList[k].m_bItemType = pstResReward->m_szRewardType[k];
			rstMailData.m_astAttachmentList[k].m_dwItemId = pstResReward->m_rewardId[k];
			rstMailData.m_astAttachmentList[k].m_iValueChg = pstResReward->m_rewardNum[k];
		}
		if (rstMailAddReq.m_nUinCount >= 0)
		{
			ZoneSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
			pstTaskInfo->m_bIsDrawed = COMMON_AWARD_STATE_DRAWED;
			rstMailAddReq.m_nUinCount = 0;
		}
		else
		{
			iRet = ERR_WRONG_STATE;
		}

        if (iRet == ERR_NONE)
        {
            ZoneLog::Instance().WriteTaskLog(&(poPlayer->GetPlayerData()), dwId, "Web");
        }
	} while (0);

	SS_PKG_WEB_TASK_GET_RWD_RSP &rstwebtaskgaetrwdRsp = m_stSsPkg.m_stBody.m_stRoleGetWebTaskRwdRsp;
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_WEB_TASK_GET_RWD_RSP;
	rstwebtaskgaetrwdRsp.m_nErrno = iRet;
	m_stSsPkg.m_stHead.m_ullReservId = rstSsPkg.m_stHead.m_ullReservId;
	return ZoneSvrMsgLayer::Instance().SendToZoneHttpConn(m_stSsPkg);
}

int SdkGetOrderRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGERR("poPlayer<%lu> is null", rstSsPkg.m_stHead.m_ullUin);
        return -1;
    }

    SS_PKG_SDK_GET_ORDERID_RSP& rstGetOrderRsp = rstSsPkg.m_stBody.m_stSDKGetOrderIDRsp;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PAY_RSP;
    SC_PKG_PAY_RSP& rstPayRsp = m_stScPkg.m_stBody.m_stPayRsp;

    rstPayRsp.m_nErrNo = rstGetOrderRsp.m_nErrNo;
    StrCpy(rstPayRsp.m_szOrderId, rstGetOrderRsp.m_stData.m_szOrderNo, MAX_LEN_SDK_PARA);
    StrCpy(rstPayRsp.m_szExtension, rstGetOrderRsp.m_stData.m_szExtension, MAX_LEN_SDK_EXT_PARA);
    rstPayRsp.m_dwProductId = rstGetOrderRsp.m_dwProductID;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int SdkPayCbNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    Pay::Instance().DoPayOk(rstSsPkg.m_stBody.m_stSDKPayCbNtf);
    return 0;
}

int PropSyntheticReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_PROP_SYNTHETIC_REQ& rstPropsSyntheticReq = rstCsPkg.m_stBody.m_stPropSyntheticReq;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PROP_SYNTHETIC_RSP;
    SC_PKG_PROP_SYNTHETIC_RSP& rstPropsSyntheticRsp = m_stScPkg.m_stBody.m_stPropSynthethicRsp;
    rstPropsSyntheticRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    rstPropsSyntheticRsp.m_nErrNo = Props::Instance().CompositeProps(&poPlayer->GetPlayerData(),
        rstPropsSyntheticReq.m_dwPropsId, rstPropsSyntheticReq.m_wNum, rstPropsSyntheticRsp.m_stSyncItemInfo);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int AfficheGetReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    Affiche::Instance().GetAffiche(&poPlayer->GetPlayerData());

    return 0;
}

int RecvVipDailyGifReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_RECV_VIP_DAILY_GIF_RSP;
    SC_PKG_RECV_VIP_DAILY_GIF_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stRecvVipDailyGifRsp;
    rstScPkgBodyRsp.m_stRewardItemInfo.m_bSyncItemCount = 0;

    rstScPkgBodyRsp.m_nErrNo = VIP::Instance().RecvDailyGif(&poPlayer->GetPlayerData(), rstScPkgBodyRsp.m_stRewardItemInfo, &rstScPkgBodyRsp.m_ullTimeStamp);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int RecvVipLevelGifReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_RECV_VIP_LEVEL_GIF_REQ& rstScPkgBodyReq = rstCsPkg.m_stBody.m_stRecvVipLevelGifReq;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_RECV_VIP_LEVEL_GIF_RSP;
    SC_PKG_RECV_VIP_LEVEL_GIF_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stRecvVipLevelGifRsp;

    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    rstScPkgBodyRsp.m_nErrNo = VIP::Instance().RecvLevelGif(&poPlayer->GetPlayerData(), rstScPkgBodyReq.m_bLevel, rstScPkgBodyRsp.m_stSyncItemInfo);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int MallBuyReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_MALL_BUY_REQ& rstScPkgBodyReq = rstCsPkg.m_stBody.m_stMallBuyReq;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MALL_BUY_RSP;
    SC_PKG_MALL_BUY_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stMallBuyRsp;
    if (rstScPkgBodyReq.m_ullFriendUin != 0)
    {
       rstScPkgBodyRsp.m_nErrNo = Mall::Instance().GiveFriendGift(&poPlayer->GetPlayerData(), rstScPkgBodyReq, rstScPkgBodyRsp);
    }
    else
    {
        rstScPkgBodyRsp.m_nErrNo = Mall::Instance().Buy(&poPlayer->GetPlayerData(), rstScPkgBodyReq, rstScPkgBodyRsp);
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int DailyAp_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_DAILY_AP_RSP;
    SC_PKG_DAILY_AP_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stDailyApRsp;
    rstScPkgBodyRsp.m_nErrNo = ERR_NONE;
    rstScPkgBodyRsp.m_stRewardItemInfo.m_bSyncItemCount = 0;

    PlayerData& dtPlayerData = poPlayer->GetPlayerData();
    DT_ROLE_MISC_INFO& dtMisc = dtPlayerData.GetMiscInfo();

    ResBasicMgr_t& rResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
    RESBASIC* poMorning = rResBasicMgr.Find(DAILY_ARCHIVE_AP_MORNING);
    RESBASIC* poNoon = rResBasicMgr.Find(DAILY_ARCHIVE_AP_NOON);
    RESBASIC* poAfterNoon = rResBasicMgr.Find(SEND_MARQUEE_MSG_AFTER_NOON);
    RESBASIC* poNight = rResBasicMgr.Find(SEND_MARQUEE_MSG_NIGHT);
    if (NULL == poMorning || NULL == poNoon || NULL == poAfterNoon || NULL == poNight)
    {
        LOGERR("Uin<%lu>ResBasic is NULL.", poPlayer->GetUin());
        return -1;
    }

    int iCurrHour = CGameTime::Instance().GetCurrHour();

    if (iCurrHour < poMorning->m_para[0])
    {
        LOGERR("Uin<%lu> the time is error.CurHour<%d> LimitHour<%d>", poPlayer->GetUin(), iCurrHour, (int)poMorning->m_para[0]);
        rstScPkgBodyRsp.m_nErrNo = ERR_DAILY_AP_NOT_TIME;
    }
    else if (_IsCurrPhaseAchieved(poMorning, iCurrHour, (int64_t)dtMisc.m_ullDailyApLastStamp))
    {
        Item::Instance().RewardItem(&poPlayer->GetPlayerData(),
            ITEM_TYPE_AP, 0, (int32_t)poMorning->m_para[2], rstScPkgBodyRsp.m_stRewardItemInfo, METHOD_DAILY_FREE_AP);
        dtMisc.m_ullDailyApLastStamp = (uint64_t)CGameTime::Instance().GetCurrSecond();
    }
    else if(iCurrHour < poNoon->m_para[0])
    {
        LOGERR("Uin<%lu> the time is error.CurHour<%d> LimitHour<%d>", poPlayer->GetUin(), iCurrHour, (int)poMorning->m_para[0]);
        rstScPkgBodyRsp.m_nErrNo = ERR_DAILY_AP_NOT_TIME;
    }
    else if (_IsCurrPhaseAchieved(poNoon, iCurrHour, (int64_t)dtMisc.m_ullDailyApLastStamp))
    {
        Item::Instance().RewardItem(&poPlayer->GetPlayerData(),
            ITEM_TYPE_AP, 0, (int32_t)poNoon->m_para[2], rstScPkgBodyRsp.m_stRewardItemInfo, METHOD_DAILY_FREE_AP);
        dtMisc.m_ullDailyApLastStamp = (uint64_t)CGameTime::Instance().GetCurrSecond();
    }
    else if(iCurrHour < poAfterNoon->m_para[0])
    {
        rstScPkgBodyRsp.m_nErrNo = ERR_DAILY_AP_NOT_TIME;
    }
    else if (_IsCurrPhaseAchieved(poAfterNoon, iCurrHour, (int64_t)dtMisc.m_ullDailyApLastStamp))
    {
        Item::Instance().RewardItem(&poPlayer->GetPlayerData(),
            ITEM_TYPE_AP, 0, (int32_t)poAfterNoon->m_para[2], rstScPkgBodyRsp.m_stRewardItemInfo, METHOD_DAILY_FREE_AP);
        dtMisc.m_ullDailyApLastStamp = (uint64_t)CGameTime::Instance().GetCurrSecond();
    }
    else if(iCurrHour < poNight->m_para[0])
    {
        rstScPkgBodyRsp.m_nErrNo = ERR_DAILY_AP_NOT_TIME;
    }
    else if (_IsCurrPhaseAchieved(poNight, iCurrHour, (int64_t)dtMisc.m_ullDailyApLastStamp))
    {
        Item::Instance().RewardItem(&poPlayer->GetPlayerData(),
            ITEM_TYPE_AP, 0, (int32_t)poNight->m_para[2], rstScPkgBodyRsp.m_stRewardItemInfo, METHOD_DAILY_FREE_AP);
        dtMisc.m_ullDailyApLastStamp = (uint64_t)CGameTime::Instance().GetCurrSecond();
    }
    else
    {
        rstScPkgBodyRsp.m_nErrNo = ERR_DAILY_AP_NOT_TIME;
    }
    rstScPkgBodyRsp.m_ullAchieveTimeStamp = dtMisc.m_ullDailyApLastStamp;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

bool _IsCurrPhaseAchieved(RESBASIC *pData, int iCurrHour, int64_t llPrevTime)
{
    bool bRet = false;

    do
    {
        int iBeginHour = (int)pData->m_para[0];
        int iEndHour = (int)pData->m_para[1];

        if (iCurrHour < iBeginHour || iCurrHour >= iEndHour)
        {
            break;
        }

        // 可领取开始时间
        int64_t dtBeginTime = CGameTime::Instance().GetSecOfHourInCurrDay(iBeginHour);
        int64_t dtSpan2Begin = dtBeginTime - llPrevTime;

        if(dtSpan2Begin <= 0)
        {
            break;;
        }

        bRet = true;
    } while (false);

    return bRet;
}

int PayReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_PAY_REQ& rstPayReq = rstCsPkg.m_stBody.m_stPayReq;

    RESPAY* pResPay = CGameDataMgr::Instance().GetResPayMgr().Find(rstPayReq.m_dwProductId);
    if (pResPay == NULL)
    {
        LOGERR("product is not exist.");
        return -1;
    }

    if (pResPay->m_bPayType == 3 && !(Pay::Instance().CheckGiftBag(&poPlayer->GetPlayerData(), rstPayReq.m_dwProductId)))
    {
        LOGERR("Player<%lu> cannot buy product<%u>", poPlayer->GetUin(), rstPayReq.m_dwProductId);
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PAY_RSP;
        m_stScPkg.m_stBody.m_stPayRsp.m_nErrNo = ERR_WRONG_STATE;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return -1;
    }

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_SDK_GET_ORDERID_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    SS_PKG_SDK_GET_ORDERID_REQ& rstGetOrderReq = m_stSsPkg.m_stBody.m_stSDKGetOrderIDReq;

    rstGetOrderReq.m_ullRoleID = poPlayer->GetUin();
    rstGetOrderReq.m_wServerID = ZoneSvr::Instance().GetConfig().m_wSvrId;
    rstGetOrderReq.m_dwMoney = pResPay->m_dwPrice;
    rstGetOrderReq.m_dwProductID = rstPayReq.m_dwProductId;
    rstGetOrderReq.m_dwPid = rstPayReq.m_dwPid;
    rstGetOrderReq.m_wRoleLevel = poPlayer->GetRoleLv();

    StrCpy(rstGetOrderReq.m_szRoleName, poPlayer->GetRoleName(), PKGMETA::MAX_NAME_LENGTH);
    StrCpy(rstGetOrderReq.m_szUid, poPlayer->GetSdkUserName(), PKGMETA::MAX_NAME_LENGTH);
    StrCpy(rstGetOrderReq.m_szProductName, pResPay->m_szProductName, PKGMETA::MAX_NAME_LENGTH);
    StrCpy(rstGetOrderReq.m_szProductDesc, pResPay->m_szProductDesc, PKGMETA::MAX_NAME_LENGTH);
    StrCpy(rstGetOrderReq.m_szServerName, ZoneSvr::Instance().GetConfig().m_szSvrName, PKGMETA::MAX_NAME_LENGTH);
    StrCpy(rstGetOrderReq.m_szChannelName, rstPayReq.m_szChannelName, PKGMETA::MAX_NAME_LENGTH);
    StrCpy(rstGetOrderReq.m_szToken, rstPayReq.m_szPayToken, PKGMETA::MAX_LEN_SDK_TOKEN_PARA);

    if (strcmp(rstPayReq.m_szChannelName, "XY") == 0)
    {
        snprintf(rstGetOrderReq.m_szExtension, MAX_LEN_SDK_EXT_PARA, "%u-%lu", rstPayReq.m_dwProductId, rstGetOrderReq.m_ullRoleID);
    }
    else
    {
        StrCpy(rstGetOrderReq.m_szExtension, rstPayReq.m_szExt, MAX_LEN_SDK_EXT_PARA);
    }

    ZoneSvrMsgLayer::Instance().SendToXiYouSDKSvr(m_stSsPkg);

    return 0;
}

int RankCommonGetTopListReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

	//CS_PKG_RANK_COMMON_GET_TOPLIST_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stRankCommonTopListReq;

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_RANK_COMMON_GET_TOPLIST_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
	SS_PKG_RANK_COMMON_GET_TOPLIST_REQ& rstSsReqBodyPkg = m_stSsPkg.m_stBody.m_stRankCommonGetTopListReq;
    rstSsReqBodyPkg.m_bType = rstCsPkg.m_stBody.m_stRankCommonTopListReq.m_bType;
	rstSsReqBodyPkg.m_ullVersion = rstCsPkg.m_stBody.m_stRankCommonTopListReq.m_ullVersion;
    rstSsReqBodyPkg.m_dwGcardId = 0;

	DT_ROLE_GCARD_INFO& rstGCardInfo = poPlayer->GetPlayerData().GetGCardInfo();
	if (rstSsReqBodyPkg.m_bType == RANK_TYPE_GCARD_LI && rstGCardInfo.m_iCount != 0)
	{
		uint32_t dwGCardId = 0;
		uint32_t dwGCardLi = 0;
		for (int i=0; i<rstGCardInfo.m_iCount; i++)
		{
			if (dwGCardLi < rstGCardInfo.m_astData[i].m_dwLi)
			{
				dwGCardId = rstGCardInfo.m_astData[i].m_dwId;
				dwGCardLi = rstGCardInfo.m_astData[i].m_dwLi;
			}
		}
		rstSsReqBodyPkg.m_dwGcardId = dwGCardId;
	}

    if (rstSsReqBodyPkg.m_bType == RANK_TYPE_GUILD)
    {
        rstSsReqBodyPkg.m_ullGuildId = poPlayer->GetPlayerData().GetGuildInfo().m_ullGuildId;
    }

    ZoneSvrMsgLayer::Instance().SendToRankSvr(m_stSsPkg);
    return 0;
}

int RankCommonGetTopListRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    SS_PKG_RANK_COMMON_GET_TOPLIST_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stRankCommonGetTopListRsp;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGERR("poPlayer<%lu> is null", rstSsPkg.m_stHead.m_ullUin);
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_RANK_COMMON_GET_TOPLIST_RSP;
    SC_PKG_RANK_COMMON_GET_TOPLIST_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stRankCommonTopListRsp;
    rstScPkgBodyRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;
    rstScPkgBodyRsp.m_bCount = rstSsPkgBodyRsp.m_bCount;
	rstScPkgBodyRsp.m_bType = rstSsPkgBodyRsp.m_bType;
	rstScPkgBodyRsp.m_bRankInfoType = rstSsPkgBodyRsp.m_bRankInfoType;
	rstScPkgBodyRsp.m_dwSelfRank = rstSsPkgBodyRsp.m_dwSelfRank;
	rstScPkgBodyRsp.m_ullVersion = rstSsPkgBodyRsp.m_ullVersion;
    memcpy(rstScPkgBodyRsp.m_astTopList, rstSsPkgBodyRsp.m_astTopList, rstScPkgBodyRsp.m_bCount*sizeof(DT_COMMON_RANK_INFO));

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    if (rstScPkgBodyRsp.m_bCount > 0
		&& rstScPkgBodyRsp.m_bType == RANK_TYPE_GCARD_LI
        && rstScPkgBodyRsp.m_bRankInfoType == RANK_ROLE_INFO_TYPE)
    {
        RankMgr::Instance().SetGCardLiRankNum(rstScPkgBodyRsp.m_bCount);
        RankMgr::Instance().SetGCardLiRankLastValue(rstScPkgBodyRsp.m_astTopList[rstScPkgBodyRsp.m_bCount - 1].m_stRankRoleInfo.m_dwValue);
    }
    return 0;
}

int RankCommonGetRankRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    SS_PKG_RANK_COMMON_GET_RANK_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stRankCommonGetRankRsp;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGERR("poPlayer<%lu> is null", rstSsPkg.m_stHead.m_ullUin);
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PLAYER_INFO_RSP;
    SC_PKG_PLAYER_INFO_RSP& rstScRsp = m_stScPkg.m_stBody.m_stPlayerInfoRsp;
    rstScRsp.m_nErrNo = rstSsRsp.m_nErrNo;
    rstScRsp.m_ullUin = rstSsRsp.m_ullUin;
    rstScRsp.m_dwRank = rstSsRsp.m_dwRank;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int WeekLeagueApplyReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_WEEK_LEAGUE_APPLY_RSP;
    SC_PKG_WEEK_LEAGUE_APPLY_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stWeekLeagueApplyRsp;
    rstScPkgBodyRsp.m_nErrNo = ERR_NONE;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    int iRet = League::Instance().WeekLeagueApply(&poPlayer->GetPlayerData(), rstScPkgBodyRsp.m_stSyncItemInfo);
    rstScPkgBodyRsp.m_nErrNo = iRet;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 1;
}

int WeekLeagueRecvRewardReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_WEEK_LEAGUE_RECV_REWADR_RSP;
    SC_PKG_WEEK_LEAGUE_RECV_REWADR_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stWeekLeagueRecvRwdRsp;
    rstScPkgBodyRsp.m_nErrNo = ERR_NONE;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    int iRet = League::Instance().WeekLeagueRecvReward(&poPlayer->GetPlayerData(),  rstScPkgBodyRsp.m_stSyncItemInfo);
    rstScPkgBodyRsp.m_nErrNo = iRet;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 1;
}

int CommonRewardRes_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_COMMON_REWARD_RSP;
	SC_PKG_COMMON_REWARD_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stCommonRewardRsp;
	rstScPkgBodyRsp.m_bType = rstCsPkg.m_stBody.m_stCommonRewardReq.m_bType;
	rstScPkgBodyRsp.m_bOtherParaCnt = 0;
	rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;


	int iErrNo = ERR_NONE;
	switch (rstCsPkg.m_stBody.m_stCommonRewardReq.m_bType)
	{
	case COMMON_REWARD_TYPE_PVP_DAILY:
		{
			iErrNo = Fight6V6::Instance().HandlePVPDailyReward(&poPlayer->GetPlayerData(), rstCsPkg.m_stBody.m_stCommonRewardReq.m_dwDrawProgress, rstScPkgBodyRsp);
			break;
		}
	case COMMON_REWARD_TYPE_PVP_SEASON:
		{
			iErrNo = Fight6V6::Instance().HandlePVPSeasonReward(&poPlayer->GetPlayerData(), rstCsPkg.m_stBody.m_stCommonRewardReq.m_dwDrawProgress, rstScPkgBodyRsp);
			break;
		}
	default:
		break;
	}

	rstScPkgBodyRsp.m_nErrNo = iErrNo;
	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 1;
}

int GuildUploadReplayReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_UPLOAD_GUILD_REPLAY_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    SS_PKG_UPLOAD_GUILD_REPLAY_REQ& rstSsPkgReq = m_stSsPkg.m_stBody.m_stGuildUploadReplayReq;
    CS_PKG_UPLOAD_GUILD_REPLAY_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stGuildUploadReplayReq;

    rstSsPkgReq.m_ullGuildId = poPlayer->GetPlayerData().GetGuildInfo().m_ullGuildId;
    rstSsPkgReq.m_stFileHead = rstCsPkgReq.m_stFileHead;

    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);

    return 1;
}

int PvpJoinRoomReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PVP_JOIN_ROOM_RSP;
    SC_PKG_PVP_JOIN_ROOM_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stPvpJoinRoomRsp;
    CS_PKG_PVP_JOIN_ROOM_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stPvpJoinRoomReq;

    rstScPkgRsp.m_nErrNo = PvpRoomMgr::Instance().JoinRoom(&poPlayer->GetPlayerData(), rstCsPkgReq, rstScPkgRsp);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 1;
}

int PvpQuitRoomReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PVP_QUIT_ROOM_RSP;
    SC_PKG_PVP_QUIT_ROOM_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stPvpQuitRoomRsp;
    CS_PKG_PVP_QUIT_ROOM_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stPvpQuitRoomReq;

    rstScPkgRsp.m_nErrNo = PvpRoomMgr::Instance().QuitRoom(&poPlayer->GetPlayerData(), rstCsPkgReq, rstScPkgRsp);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 1;
}

int PvpCreateRoomReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PVP_CREATE_ROOM_RSP;
    SC_PKG_PVP_CREATE_ROOM_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stPvpCreateRoomRsp;
    rstScPkgRsp.m_stRoom.m_bPlayerCount = 0;
    CS_PKG_PVP_CREATE_ROOM_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stPvpCreateRoomReq;

    rstScPkgRsp.m_nErrNo = PvpRoomMgr::Instance().CreateRoom(&poPlayer->GetPlayerData(), rstCsPkgReq, rstScPkgRsp);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

	//广播约战
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_MESSAGE_SEND_REQ;
	m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
	SS_PKG_GUILD_MESSAGE_SEND_REQ& rstGuildMessageSendReq = m_stSsPkg.m_stBody.m_stGuildMessageSendReq;
	rstGuildMessageSendReq.m_ullGuildId = poPlayer->GetPlayerData().GetGuildInfo().m_ullGuildId;
	DT_MESSAGE_ONE_RECORD_INFO& stRecord = rstGuildMessageSendReq.m_stRecord;
	stRecord.m_bChannel = MESSAGE_CHANNEL_GUILD;
	stRecord.m_ullSenderUin = poPlayer->GetUin();
	stRecord.m_wHeadIconId = poPlayer->GetPlayerData().GetMajestyInfo().m_wIconId;
	stRecord.m_wHeadFrameId = poPlayer->GetPlayerData().GetMajestyInfo().m_wFrameId;
	stRecord.m_wHeadTitleId = poPlayer->GetPlayerData().GetMajestyInfo().m_wTitleId;
	StrCpy(stRecord.m_szSenderName, poPlayer->GetRoleName(), PKGMETA::MAX_NAME_LENGTH);
	stRecord.m_szRecord[0] = '\0';
	stRecord.m_bIsGuildPVPMsg = 1;

	ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);

    return 1;
}

int PayGetAwardReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{

    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR(" PayGetAwardReq_CS player not exist.");
        return -1;
    }
    CS_PKG_PAY_GET_AWARD_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stGetPayAwardReq;
    switch (rstCsPkgReq.m_bGetType)
    {
    case FIRST_PAY_GET_TYPE: //首充礼包
        Pay::Instance().GetFirstPayAward(&poPlayer->GetPlayerData(), rstCsPkgReq);
        break;
    case MONTH_CARD_DAILY_GET_TYPE: //月卡领取
        Pay::Instance().GetMonthCardAward(&poPlayer->GetPlayerData(), rstCsPkgReq);
        break;
    case TOTAL_PAY_GET_TYPE: //累计充值活动奖励
        Pay::Instance().GetActTotalPayAward(&poPlayer->GetPlayerData(), rstCsPkgReq);
        break;
    case FIRST_PAY_DAY_GET_TYPE: //首日充值礼包
        Pay::Instance().GetFDayPayAward(&poPlayer->GetPlayerData(), rstCsPkgReq);
        break;
    case TOTAL_CONSUME_GET_TYPE:
        Pay::Instance().GetTotalConsumeAward(&poPlayer->GetPlayerData(), rstCsPkgReq);
        break;
    default:
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PAY_GET_AWARD_RSP;
        m_stScPkg.m_stBody.m_stGetPayAwardRsp.m_nErrNo = ERR_SYS;
        m_stScPkg.m_stBody.m_stGetPayAwardRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
        LOGERR("Uin<%lu> PayGetAwardReq_CS get taype error! <%hhu>", poPlayer->GetUin(), rstCsPkgReq.m_bGetType);
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    }
    return 0;
}

int TestPayReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    if (0 == ZoneSvr::Instance().GetConfig().m_iGmDebugSwitch)
    {
        LOGERR("Uin<%lu> Test the Pay function failed: GmDebugSwitch close! ", poPlayer->GetUin());
        return -1;
    }


    CS_PKG_TEST_PAY_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stTestPayReq;

    RESPAY* pResPay = CGameDataMgr::Instance().GetResPayMgr().Find(rstCsPkgReq.m_dwProductID);
    if (pResPay == NULL)
    {
        LOGERR("product is not exist.");
        return -1;
    }

    if (pResPay->m_bPayType == 3 && !(Pay::Instance().CheckGiftBag(&poPlayer->GetPlayerData(), rstCsPkgReq.m_dwProductID)))
    {
        LOGERR("Player<%lu> cannot buy product<%u>", poPlayer->GetUin(), rstCsPkgReq.m_dwProductID);
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PAY_RSP;
        m_stScPkg.m_stBody.m_stPayRsp.m_nErrNo = ERR_WRONG_STATE;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return -1;
    }

    SS_PKG_SDK_PAY_CB_NTF stTestNtf ;
    stTestNtf.m_stPayCbInfo.m_ullRoleID = poPlayer->GetUin();
    StrCpy(stTestNtf.m_stPayCbInfo.m_szUserID, poPlayer->GetAccountName(), PKGMETA::MAX_NAME_LENGTH);
    StrCpy(stTestNtf.m_stPayCbInfo.m_szOrderId, rstCsPkgReq.m_szOrderId, MAX_LEN_SDK_PARA);
    stTestNtf.m_stPayCbInfo.m_dwProductID = rstCsPkgReq.m_dwProductID;

    Pay::Instance().DoPayOk(stTestNtf);
    LOGERR("Uin<%lu> Test the Pay function! Productid<%u> Orderid<%s> ", poPlayer->GetUin(), rstCsPkgReq.m_dwProductID, rstCsPkgReq.m_szOrderId);
    return 0;

}

int DiscountGetReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_DISCOUNT_PROPS_GET_REQ& rstPkgReq = rstCsPkg.m_stBody.m_stDiscountPropsGetReq;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_DISCOUNT_PROPS_GET_RSP;
    SC_PKG_DISCOUNT_PROPS_GET_RSP& rstPkgRsp = m_stScPkg.m_stBody.m_stDiscountPropsGetRsp;

    Pay::Instance().GetDiscountProps(&poPlayer->GetPlayerData(), rstPkgReq, rstPkgRsp);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int DiscountBuyReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_DISCOUNT_PROPS_BUY_REQ& rstPkgReq = rstCsPkg.m_stBody.m_stDiscountPropsBuyReq;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_DISCOUNT_PROPS_BUY_RSP;
    SC_PKG_DISCOUNT_PROPS_BUY_RSP& rstPkgRsp = m_stScPkg.m_stBody.m_stDiscountPropsBuyRsp;
    rstPkgRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    Pay::Instance().BuyDiscountProps(&poPlayer->GetPlayerData(), rstPkgReq, rstPkgRsp);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int CoinShopPurchaseReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

	CS_PKG_COIN_SHOP_PURCHASE_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stCoinShopPurchaseReq;
	SC_PKG_COIN_SHOP_PURCHASE_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stCoinShopPurchaseRsp;
	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_COIN_SHOP_PURCHASE_RSP;

	int iRet = Shops::Instance().PurChase(&poPlayer->GetPlayerData(), rstCsPkgReq, rstScPkgRsp);
	rstScPkgRsp.m_nErrNo = iRet;

	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

	return 0;
}

int CoinShopUpdateReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

	CS_PKG_COIN_SHOP_UPDATE_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stCoinShopUpdateReq;
	SC_PKG_COIN_SHOP_UPDATE_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stCoinShopUpdateRsp;
	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_COIN_SHOP_UPDATE_RSP;
    rstScPkgRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

	rstScPkgRsp.m_nErrNo = Shops::Instance().SynchOrResetShop(&poPlayer->GetPlayerData(), rstCsPkgReq, rstScPkgRsp);

	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
	return 0;
}

int OpenChosenBox_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

	CS_PKG_OPEN_CHOSEN_BOX_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stOpenChosenBoxReq;

	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_OPEN_CHOSEN_BOX_RSP;
	SC_PKG_OPEN_CHOSEN_BOX_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stOpenChosenBoxRsp;
	rstScPkgBodyRsp.m_nErrNo  = Props::Instance().OpenChosenBox(&poPlayer->GetPlayerData(),
        rstCsPkgBodyReq.m_dwBoxID, rstCsPkgBodyReq.m_wNum, rstCsPkgBodyReq.m_bIndex,
		 rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_OPEN_BOX);
	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

	return 1;
}

int TransUniversalFragReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

	CS_PKG_TRANS_UNIVERSAL_FARG_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stTransUniversalFragReq;

	SC_PKG_TRANS_UNIVERSAL_FARG_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stTransUniversalFragRsp;

	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_TRANS_UNIVERSAL_FARG_RSP;

	rstScPkgBodyRsp.m_nErrNo = GeneralCard::Instance().TransUniversalFrag(&poPlayer->GetPlayerData(), rstCsPkgBodyReq, rstScPkgBodyRsp);

	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

	return 1;
}

int DailyChallengeGetTopListRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

    SS_PKG_DAILY_CHALLENGE_GET_TOPLIST_RSP& rstSsPkgRsp = rstSsPkg.m_stBody.m_stDailyChallengeGetTopListRsp;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_DAILY_CHALLENGE_GET_TOPLIST_RSP;
    SC_PKG_DAILY_CHALLENGE_GET_TOPLIST_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stDailyChallengeGetTopListRsp;

    rstScPkgRsp.m_nErrNo = rstSsPkgRsp.m_nErrNo;
    rstScPkgRsp.m_dwSelfRank = rstSsPkgRsp.m_dwSelfRank;
    rstScPkgRsp.m_bTopListCnt = rstSsPkgRsp.m_bTopListCnt;
    for (int i=0; i<rstScPkgRsp.m_bTopListCnt; i++)
    {
        rstScPkgRsp.m_astTopList[i] = rstSsPkgRsp.m_astTopList[i];
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int DailyChallengeGetTopListReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_DAILY_CHALLENGE_GET_TOPLIST_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();

    ZoneSvrMsgLayer::Instance().SendToRankSvr(m_stSsPkg);

    return ERR_NONE;
}

int DailyChallengeRecvRewardReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

	SC_PKG_DAILY_CHALLENGE_RECV_REWADR_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stDailyChallengeRecvRewardRsp;
	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_DAILY_CHALLENGE_RECV_REWADR_RSP;

    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    rstScPkgBodyRsp.m_nErrNo = DailyChallenge::Instance().RecvReward(&poPlayer->GetPlayerData(), rstScPkgBodyRsp.m_stSyncItemInfo);

    DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChallengeInfo = poPlayer->GetPlayerData().GetELOInfo().m_stDailyChallengeInfo;
    rstScPkgBodyRsp.m_bState = rstDailyChallengeInfo.m_bState;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int DailyChallengeShipFightReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_DAILY_CHALLENGE_SKIP_FIGHT_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stDailyChallengeSkipFightReq;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_DAILY_CHALLENGE_SKIP_FIGHT_RSP;
    SC_PKG_DAILY_CHALLENGE_SKIP_FIGHT_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stDailyChallengeSkipFightRsp;

    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    rstScPkgBodyRsp.m_bWinCount = rstCsPkgBodyReq.m_bWinCount;

    rstScPkgBodyRsp.m_nErrNo = DailyChallenge::Instance().SkipFight(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_bWinCount,
                               rstScPkgBodyRsp.m_stSyncItemInfo);

    DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChallengeInfo = poPlayer->GetPlayerData().GetELOInfo().m_stDailyChallengeInfo;
    rstScPkgBodyRsp.m_dwScore = rstDailyChallengeInfo.m_dwScore;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int DailyChallengeBuyBuffReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_DAILY_CHALLENGE_BUY_BUFF_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stDailyChallengeBuyBuffReq;
    SC_PKG_DAILY_CHALLENGE_BUY_BUFF_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stDailyChallengeBuyBuffRsp;
    rstScPkgRsp.m_stSelfTroop.m_bCount = 0;

    rstScPkgRsp.m_nErrNo = DailyChallenge::Instance().PurchaseBuff(&poPlayer->GetPlayerData(), rstCsPkgReq, rstScPkgRsp);
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_DAILY_CHALLENGE_BUY_BUFF_RSP;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int DailyChallengeSelectSiegeEquipReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_DAILY_CHA_SELECT_SIEGE_EQUIP_REQ& rstCsPkgReq = rstCsPkg.m_stBody.m_stDailyChaSelectSiegeEquipReq;
    SC_PKG_DAILY_CHA_SELECT_SIEGE_EQUIP_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stDailyChaSelectSiegeEquipRsp;

    rstScPkgRsp.m_nErrNo = DailyChallenge::Instance().SetSiegeEquipment(&poPlayer->GetPlayerData(), rstCsPkgReq.m_bSiegeEquipment);
    rstScPkgRsp.m_bSiegeEquipment = rstCsPkgReq.m_bSiegeEquipment;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_DAILY_CHA_SELECT_SIEGE_EQUIP_RSP;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int DailyChallengeGetEnemyInfoReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    SC_PKG_DAILY_CHA_GET_ENEMY_INFO_RSP& rstScPkgRsp = m_stScPkg.m_stBody.m_stDailyChaGetEnemyInfoRsp;

    rstScPkgRsp.m_nErrNo = DailyChallenge::Instance().LoadDungeonInfo(&poPlayer->GetPlayerData(), rstScPkgRsp.m_dwLi);

    DT_ROLE_DAILY_CHALLENGE_INFO& rstDailyChaInfo = poPlayer->GetPlayerData().GetELOInfo().m_stDailyChallengeInfo;
    DT_FIGHT_DUNGEON_INFO& rstDungeonInfo = rstDailyChaInfo.m_stFightDungeonInfo;
    rstScPkgRsp.m_bTroopNum = rstDungeonInfo.m_astFightPlayerList[1].m_bTroopNum;
    memcpy(rstScPkgRsp.m_astTroopList, rstDungeonInfo.m_astFightPlayerList[1].m_astTroopList, rstScPkgRsp.m_bTroopNum * sizeof(DT_TROOP_INFO));

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_DAILY_CHA_GET_ENEMY_INFO_RSP;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}


int PlayerInfoReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_PLAYER_INFO_REQ& rstCsReq = rstCsPkg.m_stBody.m_stPlayerInfoReq;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_RANK_COMMON_GET_RANK_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    SS_PKG_RANK_COMMON_GET_RANK_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stRankCommonGetRankReq;
    rstSsReq.m_ullUin = rstCsReq.m_ullUin;
    rstSsReq.m_bType = rstCsReq.m_bRankType;

    ZoneSvrMsgLayer::Instance().SendToRankSvr(m_stSsPkg);

    return 0;
}

int GuildDrawSalaryReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_DRAW_SALARY_RSP;
	SC_PKG_GUILD_DRAW_SALARY_RSP& rstScPkg = m_stScPkg.m_stBody.m_stGuildDrawSalaryRsp;

	DT_ROLE_GUILD_INFO& rstGuildInfo = poPlayer->GetPlayerData().GetGuildInfo();
	if (rstGuildInfo.m_bGuildSalaryDrawed)
	{
		rstScPkg.m_nErrNo = ERR_GUILD_SALARY_DRAWED;
		ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
		return 0;
	}


	SS_PKG_GUILD_DRAW_SALARY_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stGuildDrawSalaryReq;
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_DRAW_SALARY_REQ;
	m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetPlayerData().GetRoleBaseInfo().m_ullUin;

	rstSsReq.m_ullGuildId = rstGuildInfo.m_ullGuildId;

	ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);

	return 0;
}

int WriteSignature_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

	CS_PKG_WRITE_SIGNATURE_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stWriteSignatureReq;

	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_WRITE_SIGNATURE_RSP;

	SC_PKG_WRITE_SIGNATURE_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stWriteSignatureRsp;
	rstScPkgBodyRsp.m_nErrNo = ERR_NONE;
	rstScPkgBodyRsp.m_bType = rstCsPkgBodyReq.m_bType;

	switch (rstCsPkgBodyReq.m_bType)
	{
	case SIGNATURE_TYPE_PERSONAL:
		{
			DT_ROLE_MAJESTY_INFO& rstMajestyInfo = poPlayer->GetPlayerData().GetMajestyInfo();
			StrCpy(rstMajestyInfo.m_szPersonalizedSig, rstCsPkgBodyReq.m_szSignature, PKGMETA::MAX_SIGNATURE_LENGTH);
		}
		break;
	case SIGNATURE_TYPE_ASYNCPVP_DECLARATION:
		{
			DT_ROLE_ELO_INFO& rstELOInfo = poPlayer->GetPlayerData().GetELOInfo();
			StrCpy(rstELOInfo.m_stAsyncPvpInfo.m_szDeclaration, rstCsPkgBodyReq.m_szSignature, PKGMETA::MAX_SIGNATURE_LENGTH);

			// 同步到异步服务器
			AsyncPvp::Instance().UptToAsyncSvr(&poPlayer->GetPlayerData());
		}
		break;
	default:
		rstScPkgBodyRsp.m_nErrNo = ERR_NOT_FOUND;
		break;
	}

	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

	return 0;
}

int ReadSignature_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}
	CS_PKG_READ_SIGNATURE_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stReadSignatureReq;

	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_READ_SIGNATURE_RSP;

	SC_PKG_READ_SIGNATURE_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stReadSignatureRsp;
	rstScPkgBodyRsp.m_nErrNo = ERR_NONE;
	rstScPkgBodyRsp.m_bType = rstCsPkgBodyReq.m_bType;

	switch (rstCsPkgBodyReq.m_bType)
	{
	case SIGNATURE_TYPE_PERSONAL:
		{
			DT_ROLE_MAJESTY_INFO& rstMajestyInfo = poPlayer->GetPlayerData().GetMajestyInfo();
			StrCpy(rstScPkgBodyRsp.m_szSignature, rstMajestyInfo.m_szPersonalizedSig, PKGMETA::MAX_SIGNATURE_LENGTH);
		}
		break;
	case SIGNATURE_TYPE_ASYNCPVP_DECLARATION:
		{
			DT_ROLE_ELO_INFO& rstELOInfo = poPlayer->GetPlayerData().GetELOInfo();
			StrCpy(rstScPkgBodyRsp.m_szSignature, rstELOInfo.m_stAsyncPvpInfo.m_szDeclaration, PKGMETA::MAX_SIGNATURE_LENGTH);
		}
		break;
	default:
		rstScPkgBodyRsp.m_nErrNo = ERR_NOT_FOUND;
		break;
	}

	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

	return 0;
}

//是否含有特殊字符
bool WriteNameChange_CS::_IsContainSpecialChar(char* szName)
{
	for (int i = 0; i < (int)strlen(szName); i++)
	{
		if (szName[i] == '|' || szName[i] == '*' || szName[i] == '%' || szName[i] == ' ' || szName[i] == '\t')
		{
			return true;
		}
	}

	return false;
}


int WriteNameChange_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

	CS_PKG_NAME_CHANGE_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stWriteNameChangeReq;

	//检查钻石是否足够
	DT_ROLE_MISC_INFO& rstMiscInfo = poPlayer->GetPlayerData().GetMiscInfo();
	if (rstMiscInfo.m_wChangeNameCount != 0
		&& !Item::Instance().IsEnough(&poPlayer->GetPlayerData(), ITEM_TYPE_DIAMOND, 0, Majesty::Instance().GetChgNamePrice()))
	{
		m_stScPkg.m_stHead.m_wMsgId = SC_MSG_WRITE_NAME_CHANGE_RSP;
		m_stScPkg.m_stBody.m_stWriteNameChangeRsp.m_nErrNo = ERR_NOT_ENOUGH_DIAMOND;
		ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
		return -1;
	}

	//检查是否含有非法字符,如果含有则考虑为外挂
	if (_IsContainSpecialChar(rstCsPkgBodyReq.m_szName))
	{
		m_stScPkg.m_stHead.m_wMsgId = SC_MSG_WRITE_NAME_CHANGE_RSP;
		m_stScPkg.m_stBody.m_stWriteNameChangeRsp.m_nErrNo = ERR_INVALID_NAME;
		ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

		LOGERR("Invaild RoleName, Uin(%lu)", poPlayer->GetUin());
		return -1;
	}

	SS_PKG_NAME_CHANGE_QUERY_REQ& rstSsPkgBodyReq = m_stSsPkg.m_stBody.m_stNameChangeQueryReq;
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_NAME_CHANGE_QUERY_REQ;
	m_stSsPkg.m_stHead.m_iDstProcId = ZoneSvr::Instance().GetConfig().m_iRoleSvrID;
	m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();

	StrCpy(rstSsPkgBodyReq.m_szName, rstCsPkgBodyReq.m_szName, PKGMETA::MAX_NAME_LENGTH);

	ZoneSvrMsgLayer::Instance().SendToRoleSvr(m_stSsPkg);

	return 0;
}

int NameChangeQueryRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
	if (!poPlayer)
	{
		LOGERR("poPlayer is null");
		return -1;
	}

	SS_PKG_NAME_CHANGE_QUERY_RSP& rstSspkgBodyRsp = rstSsPkg.m_stBody.m_stNameChangeQueryRsp;
	SC_PKG_NAME_CHANGE_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stWriteNameChangeRsp;
	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_WRITE_NAME_CHANGE_RSP;
	rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

	int iRet = ERR_NONE;
	do
	{
		if (rstSspkgBodyRsp.m_nErrNo != ERR_NONE)
		{
			iRet = rstSspkgBodyRsp.m_nErrNo;
			break;
		}

		DT_ROLE_MISC_INFO& rstMiscInfo = poPlayer->GetPlayerData().GetMiscInfo();
		if (rstMiscInfo.m_wChangeNameCount != 0
			&& !Item::Instance().IsEnough(&poPlayer->GetPlayerData(), ITEM_TYPE_DIAMOND, 0, Majesty::Instance().GetChgNamePrice()))
		{
			iRet = ERR_NOT_ENOUGH_DIAMOND;
			break;
		}

		iRet = Majesty::Instance().ChangeRoleName(poPlayer, rstSspkgBodyRsp.m_szName);

		//最后收钱，确保名字一致
		if (rstMiscInfo.m_wChangeNameCount != 0)
		{
			Item::Instance().ConsumeItem(&poPlayer->GetPlayerData(), ITEM_TYPE_DIAMOND, 0,
				-Majesty::Instance().GetChgNamePrice(), rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_RENAME_ROLE);
		}
	} while (false);

	rstScPkgBodyRsp.m_nErrNo = iRet;
	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    //改名后同步一次
    AsyncPvp::Instance().UptToAsyncSvr(&poPlayer->GetPlayerData());

	m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetPlayerData().m_ullUin;
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_FRIEND_CHANGE_NAME_REQ;
	SS_PKG_FRIEND_CHANGE_NAME_REQ& rstSspkgBodyReq = m_stSsPkg.m_stBody.m_stFriendChangeNameReq;

	StrCpy(rstSspkgBodyReq.m_szName, poPlayer->GetPlayerData().GetRoleBaseInfo().m_szRoleName, PKGMETA::MAX_NAME_LENGTH);
	ZoneSvrMsgLayer::Instance().SendToFriendSvr(m_stSsPkg);

	m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetPlayerData().m_ullUin;
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_NAME_CHANGE_NTF;
	SS_PKG_NAME_CHANGE_NTF& rstNameChangeNtf = m_stSsPkg.m_stBody.m_stChangeNameNtf;
	StrCpy(rstNameChangeNtf.m_szName, poPlayer->GetPlayerData().GetRoleBaseInfo().m_szRoleName, PKGMETA::MAX_NAME_LENGTH);
	rstNameChangeNtf.m_ullGuildId = poPlayer->GetPlayerData().GetGuildInfo().m_ullGuildId;
	DT_ROLE_GCARD_INFO& rstGcardInfo = poPlayer->GetPlayerData().GetGCardInfo();
	rstNameChangeNtf.m_bGeneralCount = 0;
	for (int i=0; i<rstGcardInfo.m_iCount; i++)
	{
		if (rstGcardInfo.m_astData[i].m_dwLi >= RankMgr::Instance().GetGCardLiRankLastValue())
		{
			rstNameChangeNtf.m_GeneralList[rstNameChangeNtf.m_bGeneralCount++] = rstGcardInfo.m_astData[i].m_dwId;
		}

	}

	ZoneSvrMsgLayer::Instance().SendToRankSvr(m_stSsPkg);

    // 同步给全局帐号表
    bzero(&m_stSsPkg.m_stHead, sizeof(m_stSsPkg.m_stHead));
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_CLUSTER_ACC_CHG_ROLE_NAME;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    m_stSsPkg.m_stBody.m_stClusterAccChgRoleName.m_ullUin = poPlayer->GetUin();
    StrCpy( m_stSsPkg.m_stBody.m_stClusterAccChgRoleName.m_szSdkUserName, poPlayer->GetSdkUserName(), PKGMETA::MAX_NAME_LENGTH );
    StrCpy( m_stSsPkg.m_stBody.m_stClusterAccChgRoleName.m_szNewRoleName, poPlayer->GetRoleName(), PKGMETA::MAX_NAME_LENGTH );

    ZoneSvrMsgLayer::Instance().SendToClusterAccSvr( m_stSsPkg );

    return 0;
}

int PeakArenaGetActiveRewardReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

    CS_PKG_PEAK_ARENA_GET_ACTIVE_REWARD_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stPeakArenaGetActiveRewardReq;
	SC_PKG_PEAK_ARENA_GET_ACTIVE_REWARD_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stPeakArenaGetActiveRewardRsp;
	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PEAK_ARENA_GET_ACTIVE_REWARD_RSP;
	rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    for (int i=0; i<MAX_PEAK_ARENA_OUTPUT_NUM; i++)
    {
        rstScPkgBodyRsp.m_astOutputBuff[i].m_dwItemNum = 0;
    }

    rstScPkgBodyRsp.m_nErrNo = PeakArena::Instance().RecvActiveReward(&poPlayer->GetPlayerData(), rstCsPkgBodyReq, rstScPkgBodyRsp);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int PeakArenaGetOutputReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

	SC_PKG_PEAK_ARENA_GET_OUTPUT_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stPeakArenaGetOutputRsp;
	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PEAK_ARENA_GET_OUTPUT_RSP;
	rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    rstScPkgBodyRsp.m_nErrNo = PeakArena::Instance().RecvOutput(&poPlayer->GetPlayerData(), rstScPkgBodyRsp.m_stSyncItemInfo);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int PeakArenaGetRuleReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PEAK_ARENA_GET_RULE_RSP;
    SC_PKG_PEAK_ARENA_GET_RULE_RSP& rstRsp = m_stScPkg.m_stBody.m_stPeakArenaGetRuleRsp;

    rstRsp.m_dwRuleId = PeakArena::Instance().GetRuleId();

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int PeakArenaSpeedUpOutputReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PEAK_ARENA_SPEED_UP_OUTPUT_RSP;
    SC_PKG_PEAK_ARENA_SPEED_UP_OUTPUT_RSP& rstRsp = m_stScPkg.m_stBody.m_stPeakArenaSpeedupOutputRsp;

    rstRsp.m_nErrNo = PeakArena::Instance().SpeedUpOutput(&poPlayer->GetPlayerData(), rstRsp);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int PeakArenaBuyTimesReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

    CS_PKG_PEAK_ARENA_BUY_REWARD_TIMES_REQ& rstReq = rstCsPkg.m_stBody.m_stPeakArenaBuyTimesReq;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PEAK_ARENA_BUY_REWARD_TIMES_RSP;
    SC_PKG_PEAK_ARENA_BUY_REWARD_TIMES_RSP& rstRsp = m_stScPkg.m_stBody.m_stPeakArenaBuyTimesRsp;
    rstRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    rstRsp.m_nErrNo = PeakArena::Instance().BuyTimes(&poPlayer->GetPlayerData(), rstReq.m_bTimes, rstRsp.m_stSyncItemInfo);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int SPPurchaseReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_SP_PURCHASE_RSP;

	//CS_PKG_SP_PURCHASE_REQ& rstReq = rstCsPkg.m_stBody.m_stSPPurchaseReq;
	SC_PKG_SP_PURCHASE_RSP& rstRsp = m_stScPkg.m_stBody.m_stSPPurchaseRsp;

	rstRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
	rstRsp.m_nErrNo = SkillPoint::Instance().PurchaseSP(&poPlayer->GetPlayerData(), rstRsp.m_stSyncItemInfo);

	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

	return 0;

}

int TacticsAddReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_TACTICS_ADD_RSP;

    CS_PKG_TACTICS_ADD_REQ& rstReq = rstCsPkg.m_stBody.m_stTacticsAddReq;
    SC_PKG_TACTICS_ADD_RSP& rstRsp = m_stScPkg.m_stBody.m_stTacticsAddRsp;

    Tactics::Instance().Add(&poPlayer->GetPlayerData(), rstReq, rstRsp);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int TacticsLvUpReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_TACTICS_LV_UP_RSP;

    CS_PKG_TACTICS_LV_UP_REQ& rstReq = rstCsPkg.m_stBody.m_stTacticsLvUpReq;
    SC_PKG_TACTICS_LV_UP_RSP& rstRsp = m_stScPkg.m_stBody.m_stTacticsLvUpRsp;

    Tactics::Instance().LvUp(&poPlayer->GetPlayerData(), rstReq, rstRsp);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int TacticsSelectReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_TACTICS_SELECT_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stTacticsSelectReq;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_TACTICS_SELECT_RSP;
    SC_PKG_TACTICS_SELECT_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stTacticsSelectRsp;

    rstScPkgBodyRsp.m_nErrNo = Majesty::Instance().SetBattleTactics(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_bTacticsType, rstCsPkgBodyReq.m_bBattleArrayType);
    rstScPkgBodyRsp.m_bTacticsType = rstCsPkgBodyReq.m_bTacticsType;
    rstScPkgBodyRsp.m_bBattleArrayType = rstCsPkgBodyReq.m_bBattleArrayType;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int GCardEquipStarDownReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION * pstSession, PKGMETA::CSPKG & rstCsPkg, void * pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_GCARD_EQUIP_STAR_DOWN_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stGCardRebornEquipStarDownReq;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GCARD_EQUIP_STAR_DOWN_RSP;
    SC_PKG_GCARD_EQUIP_STAR_DOWN_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stGCardRebornEquipStarDownRsp;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    int iRet = ERR_NONE;
    iRet = GeneralReborn::Instance().EquipStarDown(rstCsPkgBodyReq.m_dwGeneralId, rstCsPkgBodyReq.m_bEquipType,
        &rstScPkgBodyRsp.m_stSyncItemInfo, &poPlayer->GetPlayerData());

    if (iRet == ERR_NONE)
    {
        DT_ITEM_GCARD* pstGeneral = GeneralCard::Instance().Find(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_dwGeneralId);

        uint32_t dwSeq = pstGeneral->m_astEquipList[rstCsPkgBodyReq.m_bEquipType - 1].m_dwEquipSeq;
        DT_ITEM_EQUIP* pstEquip = Equip::Instance().Find(&poPlayer->GetPlayerData(), dwSeq);
        if (pstEquip == NULL)
        {
            iRet = ERR_NOT_FOUND;
        }
        else
        {
            rstScPkgBodyRsp.m_bCurStar = pstEquip->m_wStar;
        }
    }

    rstScPkgBodyRsp.m_nErrNo = iRet;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int GCardEquipRebornReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION * pstSession, PKGMETA::CSPKG & rstCsPkg, void * pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_GCARD_EQUIP_STAR_REBORN_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stGCardEquipStarRebornReq;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GCARD_EQUIP_STAR_REBORN_RSP;
    SC_PKG_GCARD_EQUIP_STAR_REBORN_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stGCardEquipStarRebornRsp;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    int iRet = ERR_NONE;
    iRet = GeneralReborn::Instance().EquipStarReborn(rstCsPkgBodyReq.m_dwGeneralId, &poPlayer->GetPlayerData(), &rstScPkgBodyRsp.m_stSyncItemInfo);

    if (iRet == ERR_NONE)
    {
        DT_ITEM_GCARD* pstGeneral = GeneralCard::Instance().Find(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_dwGeneralId);
        rstScPkgBodyRsp.m_stGeneralInfo = *pstGeneral;

        for (int i = 0; i < MAX_EQUIP_TYPE_NUM; i++)
        {
            uint32_t dwSeq = pstGeneral->m_astEquipList[i].m_dwEquipSeq;
            DT_ITEM_EQUIP* pstEquip = Equip::Instance().Find(&poPlayer->GetPlayerData(), dwSeq);
            if (pstEquip == NULL)
            {
                iRet = ERR_NOT_FOUND;
                break;
            }
            rstScPkgBodyRsp.m_astEquipInfo[i] = *pstEquip;
        }
    }

    rstScPkgBodyRsp.m_nErrNo = iRet;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int FightPlayerCheatNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
    LOGRUN("in FightPlayerCheatNtf_SS::HandleClientMsg");

    uint64_t ullUin = rstSsPkg.m_stBody.m_stFightPlayerCheatNtf.m_ullUin;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(ullUin);
    if (NULL == poPlayer)
    {
        LOGERR("palyer<%lu> is not online", ullUin);
        return ERR_SYS;
    }

    // 记录作弊log
    ZoneLog::Instance().WriteClientCheatLog(&poPlayer->GetPlayerData());

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CHEAT_NTF;
    SC_PKG_CHEAT_NTF& rstNtf = m_stScPkg.m_stBody.m_stCheatNtf;
    rstNtf.m_nErrNo = ERR_CLIENT_CHEAT;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int ChgSkinReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION * pstSession, PKGMETA::CSPKG & rstCsPkg, void * pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_CHG_SKIN_REQ& rstChgSkinReq = rstCsPkg.m_stBody.m_stChgSkinReq;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CHG_SKIN_RSP;
    SC_PKG_CHG_SKIN_RSP& rstChgSkinRsp = m_stScPkg.m_stBody.m_stChgSkinRsp;

    rstChgSkinRsp.m_nErrNo = Skin::Instance().ChgSkin(&poPlayer->GetPlayerData(), rstChgSkinReq.m_dwGeneralId, rstChgSkinReq.m_dwSkinId);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int BuySkinReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION * pstSession, PKGMETA::CSPKG & rstCsPkg, void * pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_BUY_SKIN_REQ& rstBuySkinReq = rstCsPkg.m_stBody.m_stBuySkinReq;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_BUY_SKIN_RSP;
    SC_PKG_BUY_SKIN_RSP& rstBuySkinRsp = m_stScPkg.m_stBody.m_stBuySkinRsp;
    rstBuySkinRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    rstBuySkinRsp.m_nErrNo = Skin::Instance().BuySkin(&poPlayer->GetPlayerData(), rstBuySkinReq.m_dwSkinId, rstBuySkinRsp.m_stSyncItemInfo);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;

}

