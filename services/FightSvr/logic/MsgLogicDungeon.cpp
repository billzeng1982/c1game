#include "MsgLogicDungeon.h"
#include <string.h>
#include "define.h"
#include "common_proto.h"
#include "strutil.h"
#include "ObjectUpdatorMgr.h"
#include "../FightSvr.h"
#include "LogMacros.h"
#include "../framework/FightSvrMsgLayer.h"
#include "../dungeon/Dungeon.h"
#include "../dungeon/DungeonMgr.h"
#include "../dungeon/DungeonLogic.h"
#include "../dungeon/DungeonStateMachine.h"
#include "../player/PlayerMgr.h"
#include "../module/fightobj/FightPlayer.h"
#include "../module/fightobj/Troop.h"
#include "../module/fightobj/City.h"
#include "../module/skill/FilterManager.h"
#include "../module/skill/GeneralSkill.h"

using namespace PKGMETA;

int DungeonCreateReq_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    SS_PKG_DUNGEON_CREATE_REQ& rstSsPkgBodyReq = rstSsPkg.m_stBody.m_stDungeonCreateReq;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_DUNGEON_CREATE_RSP;
    SS_PKG_DUNGEON_CREATE_RSP& rstSsPkgBodyRsp = m_stSsPkg.m_stBody.m_stDungeonCreateRsp;

    Dungeon* poDungeon = DungeonMgr::Instance().New();
    assert(poDungeon);

    if (poDungeon->Init(&rstSsPkgBodyReq.m_stDungeonInfo))
    {
        rstSsPkgBodyRsp.m_nErrNo = ERR_NONE;
        rstSsPkgBodyRsp.m_stDungeonInfo = rstSsPkgBodyReq.m_stDungeonInfo;
        rstSsPkgBodyRsp.m_stDungeonInfo.m_dwDungeonId = poDungeon->m_dwDungeonId;
    }
    else
    {
        rstSsPkgBodyRsp.m_nErrNo = ERR_DUNGEON_CREATE;
        rstSsPkgBodyRsp.m_stDungeonInfo = rstSsPkgBodyReq.m_stDungeonInfo;
        DungeonMgr::Instance().Delete(poDungeon);
    }

    rstSsPkgBodyRsp.m_iFightSvrIp = CFightSvr::Instance().GetConfig().m_iConnSvrIp;
    rstSsPkgBodyRsp.m_wFightSvrPort = CFightSvr::Instance().GetConfig().m_wConnSvrPort;

    DT_FIGHT_PLAYER_INFO& rstPlayer1 = rstSsPkgBodyReq.m_stDungeonInfo.m_astFightPlayerList[0];
    if (rstPlayer1.m_ullUin > 10000)
    {
        rstSsPkgBodyRsp.m_ullUin = rstPlayer1.m_ullUin;
        m_stSsPkg.m_stHead.m_ullReservId = (uint64_t)rstPlayer1.m_iZoneSvrId;
        FightSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    }

    DT_FIGHT_PLAYER_INFO& rstPlayer2 = rstSsPkgBodyReq.m_stDungeonInfo.m_astFightPlayerList[1];
    if (rstPlayer2.m_ullUin > 10000)
    {
        rstSsPkgBodyRsp.m_ullUin = rstPlayer2.m_ullUin;
        m_stSsPkg.m_stHead.m_ullReservId = (uint64_t)rstPlayer2.m_iZoneSvrId;
        FightSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    }

    LOGRUN("DungeonCreateReq_SS player(%s) uin(%lu) zone(%d) vs player(%s) uin(%lu) zone(%d)",
            rstPlayer1.m_szName, rstPlayer1.m_ullUin, rstPlayer1.m_iZoneSvrId, rstPlayer2.m_szName, rstPlayer2.m_ullUin, rstPlayer2.m_iZoneSvrId);

    return 0;
}

int ChooseGeneralReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("poPlayer is null");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (!poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    //只有选人状态才能选人
    if (poDungeon->GetState() != DUNGEON_STATE_CHOOSE)
    {
        return -1;
    }

    CS_PKG_CHOOSE_GENERAL_REQ& rstCsChooseReq = rstCsPkg.m_stBody.m_stChooseGeneralReq;

    FightPlayer* poFightPlayer = poDungeon->GetFightPlayerByUin(rstCsChooseReq.m_ullUin);
    if (!poFightPlayer)
    {
        LOGERR("FightPlayer id(%lu) is not in dungeon", rstCsChooseReq.m_ullUin);
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CHOOSE_GENERAL_SYN;
    SC_PKG_CHOOSE_GENERAL_SYN& rstScChooseSync = m_stScPkg.m_stBody.m_stChooseGeneralSyn;
    rstScChooseSync.m_ullUin = poFightPlayer->m_ullUin;

    if (poDungeon->m_iCntLeft4Choose > 1)
    {
        uint8_t bCount = 0;
        DT_TROOP_INFO* pTroop = NULL;
        //先处理确定选择的武将
        for (int i=0; i<rstCsChooseReq.m_bGeneralIdCnt; i++)
        {
            uint32_t dwGeneralId = rstCsChooseReq.m_GeneralIdList[i];
            if (dwGeneralId == 0)
            {
                continue;
            }

            pTroop = poFightPlayer->ChooseGeneral(dwGeneralId);
            if (pTroop)
            {
                bCount++;
                rstScChooseSync.m_stMSkillInfo.m_bId = 0;
                rstScChooseSync.m_stTroopInfo = *pTroop;
                poDungeon->Broadcast(&m_stScPkg);
            }
        }
        //再处理随机选择的
        for (int j=0; j<(rstCsChooseReq.m_bGeneralIdCnt-bCount); j++)
        {
            pTroop = poFightPlayer->RandomChooseGenaral();
            if (pTroop)
            {
                rstScChooseSync.m_stMSkillInfo.m_bId = 0;
                rstScChooseSync.m_stTroopInfo = *pTroop;
                poDungeon->Broadcast(&m_stScPkg);
            }
        }
    }
    else
    {
        MasterSkill* pMSkill = poFightPlayer->ChooseMasterSkill(rstCsChooseReq.m_bMSkillId);
        if (pMSkill)
        {
            rstScChooseSync.m_stMSkillInfo.m_bId = pMSkill->m_dwSkillId;
            rstScChooseSync.m_stMSkillInfo.m_bLevel = pMSkill->m_bSkillLevel;
            poDungeon->Broadcast(&m_stScPkg);
        }
    }

    return 0;
}

int FightChgSkinReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("poPlayer is null");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (!poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    //只有选人状态才能选人
    if (poDungeon->GetState() != DUNGEON_STATE_CHOOSE)
    {
        return -1;
    }

    CS_PKG_FIGHT_CHG_SKIN_REQ& rstCsChgSkinReq = rstCsPkg.m_stBody.m_stFightChgSkinReq;
    FightPlayer* poFightPlayer = poDungeon->GetFightPlayerByUin(rstCsChgSkinReq.m_ullUin);
    if (!poFightPlayer)
    {
        LOGERR("FightPlayer id(%lu) is not in dungeon", rstCsChgSkinReq.m_ullUin);
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_CHG_SKIN_SYN;
    SC_PKG_FIGHT_CHG_SKIN_SYN& rstScChgSkinSync = m_stScPkg.m_stBody.m_stFightChgSkinSyn;
    rstScChgSkinSync.m_ullUin = rstCsChgSkinReq.m_ullUin;
    rstScChgSkinSync.m_bSkinCnt = rstCsChgSkinReq.m_bSkinCnt;
    for (int i = 0; i < rstCsChgSkinReq.m_bSkinCnt && i < MAX_TROOP_NUM_PVP; i++)
    {
        rstScChgSkinSync.m_szTroopList[i] = rstCsChgSkinReq.m_szTroopList[i];
        rstScChgSkinSync.m_SkinList[i] = rstCsChgSkinReq.m_SkinList[i];
    }
    poDungeon->Broadcast(&m_stScPkg);

    return 0;
}


int DungeonStartReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("poPlayer is null");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    LOGRUN("DungeonStartReq_CS Uin-%lu", poPlayer->m_ullUin);

    if (poDungeon->FightReady(poPlayer))
    {
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_DUNGEON_START_RSP;
        SC_PKG_DUNGEON_START_RSP& rstScPkgBody = m_stScPkg.m_stBody.m_stDungeonStartRsp;
        rstScPkgBody.m_nErrNo = ERR_NONE;

        if (poDungeon->GetState() == DUNGEON_STATE_WAIT_LOADING)
        {
            poDungeon->Broadcast(&m_stScPkg);

            // 战斗开始动画
            DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_OP);
        }
        else
        {
            poDungeon->Broadcast(&m_stScPkg, poPlayer);
        }
    }

    return 0;
}


int DungeonOpEndNtf_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("poPlayer is null");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    if (poDungeon->GetState() == DUNGEON_STATE_OP)
    {
        // 正常战斗
        DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_FIGHT);
    }
    else
    {
        // 同步一次副本时间
        DungeonLogic::Instance().SyncDungeonTime(poDungeon);
    }

    return 0;
}

int FightPosNtf_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_FIGHT_POS_NTF& rstCsPkgBody = rstCsPkg.m_stBody.m_stFightPosNtf;

    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    // 目标对象
    FightObj* poTarget = poDungeon->GetFightObj(rstCsPkgBody.m_stPosInfo.m_stTargetObj);
    if (poTarget == NULL)
    {
        LOGERR("target is null");
        return -1;
    }

    if (poTarget->m_iHpCur <= 0)
    {
        LOGERR("target is dead, can't move.");
        return -1;
    }

    poTarget->Move(rstCsPkgBody.m_stPosInfo);

    // 给同副本的玩家同步战斗信息
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_POS_SYN;
    SC_PKG_FIGHT_POS_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightPosSyn;
    rstScPkgBody.m_stPosInfo = rstCsPkgBody.m_stPosInfo;
    //poDungeon->Broadcast(&m_stScPkg, poPlayer);
    poDungeon->Broadcast( &m_stScPkg );

    return 0;
}

int FightSpeedNtf_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_FIGHT_SPEED_NTF& rstCsPkgBody = rstCsPkg.m_stBody.m_stFightSpeedNtf;

    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    // 目标对象
    FightObj* poTarget = poDungeon->GetFightObj(rstCsPkgBody.m_stSpeedInfo.m_stTargetObj);
    if (poTarget != NULL)
    {
        poTarget->UpdateSpeed(rstCsPkgBody.m_stSpeedInfo);
    }

    // 给同副本的玩家同步战斗信息
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_SPEED_SYN;
    SC_PKG_FIGHT_SPEED_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightSpeedSyn;
    rstScPkgBody.m_stSpeedInfo = rstCsPkgBody.m_stSpeedInfo;
    //poDungeon->Broadcast(&m_stScPkg, poPlayer);
    poDungeon->Broadcast(&m_stScPkg);

    LOGRUN("Dungeon<%u>, Player<%s> Chg Target<%d> SpeedRatio %d, and send it back.",
        poDungeon->m_dwDungeonId, poPlayer->m_szName, rstCsPkgBody.m_stSpeedInfo.m_stTargetObj.m_bId, rstCsPkgBody.m_stSpeedInfo.m_nSpeedRatio);

    return 0;
}

int FightHpNtf_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_FIGHT_HP_NTF& rstCsPkgBody = rstCsPkg.m_stBody.m_stFightHpNtf;

    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    DT_HP_INFO& rHpInfo = rstCsPkgBody.m_stHpInfo;

    LOGRUN("Dungeon<%u>, Player<%s> Recv HpNtf, ValueChgType=%d, ValueChgPara=%d, Source=%d, Target=%d, HpChgBefore=%d, HpChgAfter=%d, Damage=%d",
           poDungeon->m_dwDungeonId, poPlayer->m_szName, rHpInfo.m_nValueChgType, rHpInfo.m_iValueChgPara, rHpInfo.m_stSourceObj.m_bId,
           rHpInfo.m_stTargetObj.m_bId, rHpInfo.m_iHpChgBefore, rHpInfo.m_iHpChgAfter, rHpInfo.m_iHpChgBefore - rHpInfo.m_iHpChgAfter);

    // 是否需要等待处理
    bool bNeedWaitDeal = false;
    switch (poDungeon->GetState())
    {
    case DUNGEON_STATE_LOCK_AMBUSH:
        {
            if (rHpInfo.m_nValueChgType != VALUE_CHG_TYPE::CHG_HP_ATK_AMBUSH)
            {
                bNeedWaitDeal = true;
            }
            break;
        }
    case DUNGEON_STATE_LOCK_SKILL:
        {
            if (rHpInfo.m_nValueChgType != VALUE_CHG_TYPE::CHG_HP_GENERALSKILL)
            {
                bNeedWaitDeal = true;
            }
            break;
        }
    case DUNGEON_STATE_LOCK_SOLO:
        {
            if (rHpInfo.m_nValueChgType != VALUE_CHG_TYPE::CHG_HP_SOLO)
            {
                bNeedWaitDeal = true;
            }
            break;
        }
    case DUNGEON_STATE_LOCK_MSKILL:
        {
            if (rHpInfo.m_nValueChgType != VALUE_CHG_TYPE::CHG_HP_MASTERSKILL)
            {
                bNeedWaitDeal = true;
            }
            break;
        }
    default:
        break;
    }

    if (bNeedWaitDeal)
    {
        poDungeon->PushCsPkgWaitDeal(pstSession->m_dwSessionId, rstCsPkg);
        LOGRUN("need wait deal, rHpInfo.m_nValueChgType = %d", rHpInfo.m_nValueChgType);
        return 0;
    }

    FightObj* poSource = poDungeon->GetFightObj(rHpInfo.m_stSourceObj);
    FightObj* poTarget = poDungeon->GetFightObj(rHpInfo.m_stTargetObj);

    // 攻击者 是 troop
    if(poSource != NULL)
    {
        if (poSource->m_chType == FIGHTOBJ_TROOP)
        {
            Troop* poSourceTroop = (Troop*)poSource;
            poSourceTroop->m_stKillPos = rHpInfo.m_stSourcePos;
        }
    }

    int iHpBefore = rHpInfo.m_iHpChgBefore;
    int iHpAfter = rHpInfo.m_iHpChgAfter;
	int iDamageFxSrc = 0;
	int iDamageFxTar = 0;
    if (poTarget != NULL)
    {
        poTarget->ChgHp(rHpInfo.m_nValueChgType, rHpInfo.m_iValueChgPara, poSource, rHpInfo.m_iDamageRef, iHpBefore, iHpAfter, iDamageFxSrc, iDamageFxTar);

        rHpInfo.m_iHpChgBefore = iHpBefore;
        rHpInfo.m_iHpChgAfter = iHpAfter;
		rHpInfo.m_iDamageFxSrc = iDamageFxSrc;
		rHpInfo.m_iDamageFxTar = iDamageFxTar;
#if 0
        // 这儿不能要，因为有些护盾会把血量过滤到零，客户端也需等效过滤
        if (iHpAfter == iHpBefore)
        {
            LOGRUN("invalid hp chg, no need to send to client.");
            return 0;
        }
#endif
    }

    // 给同副本的玩家同步战斗信息
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_HP_SYN;
    SC_PKG_FIGHT_HP_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightHpSyn;
    rstScPkgBody.m_stHpInfo = rstCsPkgBody.m_stHpInfo;
    poDungeon->Broadcast(&m_stScPkg);

    // 血量改变后事件处理
    if (poTarget != NULL)
    {
        poTarget->AfterChgHp(rHpInfo.m_nValueChgType, rHpInfo.m_iValueChgPara, poSource, iHpBefore, iHpAfter);
    }

    return 0;
}

int FightMoraleNtf_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_FIGHT_MORALE_NTF& rstCsPkgBody = rstCsPkg.m_stBody.m_stFightMoraleNtf;

    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    // 副本士气修正
    poDungeon->ChgMorale(rstCsPkgBody.m_stMoraleInfo.m_chGroup, rstCsPkgBody.m_stMoraleInfo.m_nChgMorale / 100.0f);

    // 给同副本的玩家同步战斗信息
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_MORALE_SYN;
    SC_PKG_FIGHT_MORALE_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightMoraleSyn;
    rstScPkgBody.m_stMoraleInfo = rstCsPkgBody.m_stMoraleInfo;
    poDungeon->Broadcast(&m_stScPkg);

    return 0;
}

int FightAtkLockNtf_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_FIGHT_ATK_LOCK_NTF& rstCsPkgBody = rstCsPkg.m_stBody.m_stFightAtkLockNtf;

    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    // 给同副本的玩家同步战斗信息
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_ATK_LOCK_SYN;
    SC_PKG_FIGHT_ATK_LOCK_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightAtkLockSyn;
    rstScPkgBody.m_stLockInfo = rstCsPkgBody.m_stLockInfo;
    poDungeon->Broadcast(&m_stScPkg);

    return 0;
}

int FightSkillStartNtf_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_FIGHT_SKILL_START_NTF& rstCsPkgBody = rstCsPkg.m_stBody.m_stFightSkillStartNtf;

    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    int16_t errNo = ERR_NONE;

    // 校验技能能否释放
    Troop* poSource = poDungeon->GetTroopById(rstCsPkgBody.m_stStartInfo.m_stSourceObj.m_bId);
    do
    {
        // 是否是正常战斗状态
        if (poDungeon->GetState() != DUNGEON_STATE_FIGHT)
        {
            LOGRUN("dungeon is locking");
            errNo = -1;
            break;
        }

        // 检查当前释放技能队伍是否Dead 或者 Retreat，是则放弃当前操作
        if (poSource != NULL)
        {
            if (poSource->m_iHpCur <= 0 || poSource->m_bIsRetreat)
            {
                LOGRUN("troop is dead or retreat.");
                errNo = -1;
                break;
            }
        }
        else
        {
            LOGERR("troop not found.");
            errNo = -1;
            break;
        }

        // 士气值是否足够
        if (poSource->m_poFightPlayer != NULL)
        {
            float fMorale = poSource->m_poFightPlayer->GetMoraleForPreUpdate();
            if (fMorale < poSource->m_oGeneral.m_poActiveSkill->m_fMoraleNeedCurr)
            {
                LOGERR("player's morale not enough.");
                errNo = -1;
                break;
            }
        }
        else
        {
            LOGERR("playerinfo not found.");
            errNo = -1;
            break;
        }

        // 目标是否存在，目标能否被攻击
        int iTargetCnt = rstCsPkgBody.m_stStartInfo.m_bTargetObjCnt;
        if (iTargetCnt > 0)
        {
            for (int i=0; i<rstCsPkgBody.m_stStartInfo.m_bTargetObjCnt; i++)
            {
                DT_FIGHTOBJ& rstFightObj = rstCsPkgBody.m_stStartInfo.m_astTargetObjList[i];
                if (rstFightObj.m_chGroup != poSource->m_chGroup && rstFightObj.m_chType == FIGHTOBJ_TROOP)
                {
                    // 敌人如果是部队，需校验是否死亡或在城内
                    Troop* poTarget = poDungeon->GetTroopById(rstFightObj.m_bId);

                    if (poTarget != NULL)
                    {
                        if (poTarget->m_iHpCur <= 0 || poTarget->m_bIsRetreat)
                        {
                            iTargetCnt--;
                        }
                    }
                    else
                    {
                        LOGERR("target not found.");
                        errNo = -1;
                        break;
                    }
                }
            }

            if (iTargetCnt <= 0)
            {
                LOGERR("target count is zero.");
                errNo = -1;
                break;
            }
        }
    } while (false);

    if (errNo == ERR_NONE)
    {
        // 切到技能暂停状态
        DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_LOCK_SKILL);

        // 给同副本的玩家同步战斗信息
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_SKILL_START_SYN;
        SC_PKG_FIGHT_SKILL_START_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightSkillStartSyn;
        rstScPkgBody.m_stStartInfo = rstCsPkgBody.m_stStartInfo;
        rstScPkgBody.m_stStartInfo.m_nErrNo = errNo;

        poDungeon->Broadcast(&m_stScPkg);

        // 任务统计
        if (poSource->m_poFightPlayer != NULL)
        {
            poSource->m_poFightPlayer->m_stTaskInfo.m_dwGeneralSkill++;
        }

        LOGRUN("Dungeon<%u>, Player<%s> Troop<%d> Start SKill, TargetNum<%d>",
                poDungeon->m_dwDungeonId, poPlayer->m_szName, rstCsPkgBody.m_stStartInfo.m_stSourceObj.m_bId, rstCsPkgBody.m_stStartInfo.m_bTargetObjCnt);
    }
    else
    {
        // 技能释放失败
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_SKILL_START_SYN;
        SC_PKG_FIGHT_SKILL_START_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightSkillStartSyn;
        rstScPkgBody.m_stStartInfo = rstCsPkgBody.m_stStartInfo;
        rstScPkgBody.m_stStartInfo.m_nErrNo = errNo;

        FightSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    }

    return 0;
}

int FightSkillFinishNtf_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    if (poDungeon->GetState() == DUNGEON_STATE_LOCK_SKILL)
    {
        // 正常战斗
        DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_FIGHT);
    }
    else
    {
        // 同步一次副本时间
        DungeonLogic::Instance().SyncDungeonTime(poDungeon);
    }

    return 0;
}

int FightBuffAddNtf_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_FIGHT_BUFF_ADD_NTF& rstCsPkgBody = rstCsPkg.m_stBody.m_stFightBuffAddNtf;

    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    // 服务器副本添加Buff
    DT_BUFF_INFO& rstBuffInfo = rstCsPkgBody.m_stBuffInfo;
    FightObj* poOwner = poDungeon->GetFightObj(rstBuffInfo.m_stOwnerObj);
    FightObj* poSource = poDungeon->GetFightObj(rstBuffInfo.m_stSourceObj);
    std::list<FightObj*> listOwner;
    for (int i=0; i<rstBuffInfo.m_bOwnerObjCnt; i++)
    {
        FightObj* poObj = poDungeon->GetFightObj((rstBuffInfo.m_astOwnerObjList[i]));
        listOwner.push_back(poObj);
    }

    if (poOwner != NULL)
    {
        poOwner->m_oBuffManager.AddBuff(rstBuffInfo.m_dwBuffId, poSource, &listOwner);
    }

    LOGRUN("Dungeon<%u>, Player<%s> AddBuff, ownerId=%d, buffId=%u",
            poDungeon->m_dwDungeonId, poPlayer->m_szName, (int)poOwner->m_bId, rstBuffInfo.m_dwBuffId);

    // 给同副本的玩家同步战斗信息
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_BUFF_ADD_SYN;
    SC_PKG_FIGHT_BUFF_ADD_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightBuffAddSyn;
    rstScPkgBody.m_stBuffInfo = rstCsPkgBody.m_stBuffInfo;
    poDungeon->Broadcast(&m_stScPkg);

    return 0;
}

int FightBuffDelNtf_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_FIGHT_BUFF_DEL_NTF& rstCsPkgBody = rstCsPkg.m_stBody.m_stFightBuffDelNtf;

    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    // 服务器副本删除Buff
    FightObj* poOwner = poDungeon->GetFightObj(rstCsPkgBody.m_stBuffInfo.m_stOwnerObj);
    if (poOwner != NULL)
    {
        poOwner->m_oBuffManager.DelBuff(rstCsPkgBody.m_stBuffInfo.m_dwBuffId, rstCsPkgBody.m_stBuffInfo.m_bBuffType);
    }

    LOGRUN("Dungeon<%u>, Player<%s>, DelBuff ownerId = %d, buffId = %u",
            poDungeon->m_dwDungeonId, poPlayer->m_szName, (int)poOwner->m_bId, rstCsPkgBody.m_stBuffInfo.m_dwBuffId);

    // 给同副本的玩家同步战斗信息
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_BUFF_DEL_SYN;
    SC_PKG_FIGHT_BUFF_DEL_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightBuffDelSyn;
    rstScPkgBody.m_stBuffInfo = rstCsPkgBody.m_stBuffInfo;
    poDungeon->Broadcast(&m_stScPkg);

    return 0;
}


int FightSoloStartNtf_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_FIGHT_SOLO_START_NTF& rstCsPkgBody = rstCsPkg.m_stBody.m_stFightSoloStartNtf;

    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    int16_t errNo = ERR_NONE;
    do
    {
        // 是否是正常战斗状态
        if (poDungeon->GetState() != DUNGEON_STATE_FIGHT)
        {
            LOGRUN("dungeon is locking");
            errNo = -1;
            break;
        }

        // 检查当前释放技能队伍是否Dead 或者 Retreat，是则放弃当前操作
        Troop* poSource = poDungeon->GetTroopById(rstCsPkgBody.m_stStartInfo.m_stSourceObj.m_bId);
        if (poSource != NULL)
        {
            if (poSource->m_iHpCur <= 0 || poSource->m_bIsRetreat)
            {
                LOGRUN("troop is dead or retreat.");
                errNo = -1;
                break;
            }
        }
        else
        {
            LOGERR("troop not found.");
            errNo = -1;
            break;
        }

        Troop* poTarget = poDungeon->GetTroopById(rstCsPkgBody.m_stStartInfo.m_stTargetObj.m_bId);
        if (poTarget != NULL)
        {
            if (poTarget->m_iHpCur <= 0 || poTarget->m_bIsRetreat)
            {
                LOGRUN("troop is dead or retreat.");
                errNo = -1;
                break;
            }
        }
        else
        {
            LOGERR("troop not found.");
            errNo = -1;
            break;
        }
    } while (false);

    if (errNo == ERR_NONE)
    {
        // 切到单挑暂停状态
        DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_LOCK_SOLO);

        // 给同副本的玩家同步战斗信息
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_SOLO_START_SYN;
        SC_PKG_FIGHT_SOLO_START_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightSoloStartSyn;
        rstScPkgBody.m_stStartInfo = rstCsPkgBody.m_stStartInfo;
        rstScPkgBody.m_nErrNo = errNo;
        poDungeon->Broadcast(&m_stScPkg);
    }
    else
    {
        // 单挑失败
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_SOLO_START_SYN;
        SC_PKG_FIGHT_SOLO_START_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightSoloStartSyn;
        rstScPkgBody.m_stStartInfo = rstCsPkgBody.m_stStartInfo;
        rstScPkgBody.m_nErrNo = errNo;
        FightSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    }

    return 0;
}

int FightSoloSettleNtf_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
#if 0
    CS_PKG_FIGHT_SOLO_SETTLE_NTF& rstCsPkgBody = rstCsPkg.m_stBody.m_stFightSoloSettleNtf;

    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    // 判断是否已收到对手的单挑按键成绩
    int8_t chWinGroup = PLAYER_GROUP_NONE;
    if (poDungeon->SoloSettle(&rstCsPkgBody.m_stSettleInfo, chWinGroup))
    {
        // 给同副本的玩家同步战斗信息
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_SOLO_SETTLE_SYN;
        SC_PKG_FIGHT_SOLO_SETTLE_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightSoloSettleSyn;

        // 把当前的SOLO结果广播给其他玩家
        rstScPkgBody.m_stSettleInfo = rstCsPkgBody.m_stSettleInfo;
        poDungeon->Broadcast(&m_stScPkg, poPlayer);

        // 把保存的SOLO结果发给该次的玩家
        rstScPkgBody.m_stSettleInfo = poDungeon->m_stSoloSettleInfo;
        FightSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    }
#endif
    return 0;
}

int FightSoloFinishNtf_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
#if 0
    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    if (poDungeon->GetState() == DUNGEON_STATE_LOCK_SOLO)
    {
        // 正常战斗
        DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_FIGHT);
    }
    else
    {
        // 同步一次副本时间
        DungeonLogic::Instance().SyncDungeonTime(poDungeon);
    }
#endif
    return 0;
}

int FightMSkillStartNtf_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    CS_PKG_FIGHT_MSKILL_START_NTF& rstCsPkgBody = rstCsPkg.m_stBody.m_stFightMSkillStartNtf;

    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    int16_t errNo = ERR_NONE;

    // 是否是正常战斗状态
    if (poDungeon->GetState() != DUNGEON_STATE_FIGHT)
    {
        LOGRUN("dungeon is locking");
        errNo = -1;
    }

    FightPlayer* poFightPlayer = poDungeon->GetFightPlayerById(rstCsPkgBody.m_stStartInfo.m_stSourceObj.m_bId);
    if (poFightPlayer == NULL)
    {
        LOGERR("FightPlayer not found");
        errNo = -1;
    }

    if (errNo == ERR_NONE)
    {
        // 切到技能暂停状态
        DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_LOCK_MSKILL);
        poFightPlayer->m_poMasterSkill->m_fPowerProgress = rstCsPkgBody.m_stStartInfo.m_wPowerProgress / 10000.0f;

        // 给同副本的玩家同步战斗信息
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_MSKILL_START_SYN;
        SC_PKG_FIGHT_MSKILL_START_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightMSkillStartSyn;
        rstScPkgBody.m_stStartInfo = rstCsPkgBody.m_stStartInfo;
        rstScPkgBody.m_stStartInfo.m_nErrNo = errNo;
        poDungeon->Broadcast(&m_stScPkg);

        // 每日任务统计
        poFightPlayer->m_stTaskInfo.m_dwMasterSkill++;
    }
    else
    {
        // 军师技能释放失败
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_MSKILL_START_SYN;
        SC_PKG_FIGHT_MSKILL_START_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightMSkillStartSyn;
        rstScPkgBody.m_stStartInfo = rstCsPkgBody.m_stStartInfo;
        rstScPkgBody.m_stStartInfo.m_nErrNo = errNo;
        FightSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    }

    return 0;
}

int FightMSkillFinishNtf_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    if (poDungeon->GetState() == DUNGEON_STATE_LOCK_MSKILL)
    {
        // 正常战斗
        DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_FIGHT);
    }
    else
    {
        // 同步一次副本时间
        DungeonLogic::Instance().SyncDungeonTime(poDungeon);
    }

    return 0;
}

int FightRetreatingNtf_CS ::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_FIGHT_RETREATING_NTF& rstCsPkgBody = rstCsPkg.m_stBody.m_stFightRetreatingNtf;

    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    // 给同副本的玩家同步战斗信息
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_RETREATING_SYN;
    SC_PKG_FIGHT_RETREATING_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightRetreatingSyn;
    rstScPkgBody.m_stSourceObj = rstCsPkgBody.m_stSourceObj;
    poDungeon->Broadcast(&m_stScPkg);

    return 0;
};


int FightRetreatNtf_CS ::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_FIGHT_RETREAT_NTF& rstCsPkgBody = rstCsPkg.m_stBody.m_stFightRetreatNtf;

    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    if (poDungeon->GetState() != DUNGEON_STATE_FIGHT)
    {
        // 在非正常战斗流程的回城包，缓存下来
        poDungeon->PushCsPkgWaitDeal(pstSession->m_dwSessionId, rstCsPkg);
        LOGRUN("need wait deal for this retreat pkg");

        return 0;
    }

    Troop* poSource = poDungeon->GetTroopById(rstCsPkgBody.m_stSourceObj.m_bId);
    if (poSource == NULL)
    {
        LOGERR("troop not found.");
        return -1;
    }

    if (poSource->m_iHpCur >0 && rstCsPkgBody.m_bIsDead == 1)
    {
        // 部队未死亡，但是发的报文是死亡回城
        LOGERR("troop is not dead but retreat is dead mode.");
        return -1;
    }

    if (poSource->m_iHpCur <= 0 && rstCsPkgBody.m_bIsDead == 0)
    {
        // 部队已死亡，但是发的报文不是死亡回城
        LOGERR("troop is dead but retreat is not dead mode.");
        return -1;
    }

    if (poSource->m_bIsRetreat)
    {
        // 部队已撤退，又触发撤退
        LOGERR("troop is retreat again.");
        return -1;
    }

    poSource->m_bIsRetreat = true;

    // 给同副本的玩家同步战斗信息
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_RETREAT_SYN;
    SC_PKG_FIGHT_RETREAT_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightRetreatSyn;
    rstScPkgBody.m_bIsDead = rstCsPkgBody.m_bIsDead;
    rstScPkgBody.m_stSourceObj = rstCsPkgBody.m_stSourceObj;
    poDungeon->Broadcast(&m_stScPkg);

    return 0;
};

int FightOutCityNtf_CS ::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_FIGHT_OUTCITY_NTF& rstCsPkgBody = rstCsPkg.m_stBody.m_stFightOutCityNtf;

    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    Troop* poSource = poDungeon->GetTroopById(rstCsPkgBody.m_stOutCityInfo.m_stSourceObj.m_bId);
    if (poSource == NULL)
    {
        LOGERR("troop not found.");
        return -1;
    }

    if (rstCsPkgBody.m_stOutCityInfo.m_chOutCityType == OUTCITY_END)
    {
        poSource->m_bIsRetreat = false;
    }

    // 给同副本的玩家同步战斗信息
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_OUTCITY_SYN;
    SC_PKG_FIGHT_OUTCITY_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightOutCitySyn;
    rstScPkgBody.m_stOutCityInfo  = rstCsPkgBody.m_stOutCityInfo;
    poDungeon->Broadcast(&m_stScPkg, poPlayer);

    return 0;
};

int FightDeadPlayEndNtf_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_FIGHT_DEAD_PLAY_END_NTF& rstCsPkgBody = rstCsPkg.m_stBody.m_stFightDeadPlayEndNtf;
    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    Troop* poTroop = poDungeon->GetTroopById(rstCsPkgBody.m_stSourceObj.m_bId);
    if (poTroop != NULL)
    {
        poDungeon->DelTroopDead(poTroop);
    }

    return 0;
};

int FightAmbushStartNtf_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    CS_PKG_FIGHT_AMBUSH_START_NTF& rstCsPkgBody = rstCsPkg.m_stBody.m_stFightAmbushStartNtf;
    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    int16_t errNo = ERR_NONE;

    // 检查当前伏兵部伍是否Dead 或者 Retreat，是则放弃当前操作
    Troop* poSource = poDungeon->GetTroopById(rstCsPkgBody.m_stStartInfo.m_stSourceObj.m_bId);

    if (poSource != NULL)
    {
        if (poSource->m_iHpCur <= 0 || poSource->m_bIsRetreat)
        {
            LOGRUN("troop is dead or retreat.");
            errNo = -1;
        }
    }
    else
    {
        LOGERR("troop not found.");
        errNo = -1;
    }

    if (errNo == ERR_NONE)
    {
        poDungeon->AddTroopAmbush(poSource);

        // Sync ambush
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_AMBUSH_START_SYN;
        SC_PKG_FIGHT_AMBUSH_START_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightAmbushStartSyn;
        rstScPkgBody.m_stStartInfo = rstCsPkgBody.m_stStartInfo;
        poDungeon->Broadcast(&m_stScPkg);
    }
    else
    {
        // 伏兵攻击失败
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_AMBUSH_START_SYN;
        SC_PKG_FIGHT_AMBUSH_START_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightAmbushStartSyn;
        rstScPkgBody.m_stStartInfo = rstCsPkgBody.m_stStartInfo;
        FightSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    }

    return 0;
}

int FightAmbushFinishNtf_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    CS_PKG_FIGHT_AMBUSH_FINISH_NTF& rstCsPkgBody = rstCsPkg.m_stBody.m_stFightAmbushFinishNtf;
    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    Troop* poTroop = poDungeon->GetTroopById(rstCsPkgBody.m_stSourceObj.m_bId);
    if (poTroop != NULL)
    {
        poDungeon->DelTroopAmbush(poTroop);
    }

    return 0;
}

int FightAtkCityNtf_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    CS_PKG_FIGHT_ATKCITY_NTF& rstCsPkgBody = rstCsPkg.m_stBody.m_stFightAtkCityNtf;
    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    Troop* poTroop = poDungeon->GetTroopById(rstCsPkgBody.m_stAtkCityInfo.m_stSourceObj.m_bId);
    if (poTroop != NULL)
    {
        if (rstCsPkgBody.m_stAtkCityInfo.m_chState == ATKCITY_REAL)
        {
            poTroop->m_bIsAtkCity = rstCsPkgBody.m_stAtkCityInfo.m_chEnable != 0;
        }
    }

    // 攻城同步
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_ATKCITY_SYN;
    SC_PKG_FIGHT_ATKCITY_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightAtkCitySyn;
    rstScPkgBody.m_stAtkCityInfo = rstCsPkgBody.m_stAtkCityInfo;

    poDungeon->Broadcast(&m_stScPkg, poPlayer);

    return 0;
}

int FightCatapultInfoNtf_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    CS_PKG_FIGHT_CATAPULT_INFO_NTF& rstCsPkgBody = rstCsPkg.m_stBody.m_stFightCatapultInfoNtf;
    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    // 投石车同步
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_CATAPULT_INFO_SYN;
    SC_PKG_FIGHT_CATAPULT_INFO_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightCatapultInfoSyn;
    rstScPkgBody.m_stCatapultInfo = rstCsPkgBody.m_stCatapultInfo;

    poDungeon->Broadcast(&m_stScPkg);

    return 0;
}

int TestPackgeIngress_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_TEST_PACKAGE_INGRESS& rstCsPkgBody = rstCsPkg.m_stBody.m_stTestPackageIngress;

    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_TEST_PACKAGE_EGRESS;
    SC_PKG_TEST_PACKAGE_EGRESS& rstScPkgBody = m_stScPkg.m_stBody.m_stTestPackageEgress;
    rstScPkgBody.m_iSeq = rstCsPkgBody.m_iSeq;
    memcpy(rstScPkgBody.m_szData, rstCsPkgBody.m_szData, strlen(rstCsPkgBody.m_szData));

    FightSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
};

int FightTimeSyncReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("poPlayer is null");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    // 重置对时次数
    poDungeon->m_iTimeSyncCnt = Dungeon::TimeSyncCnt;

    return 0;
}

int FightDelaySyncReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    CS_PKG_TIME_DELAY_REQ& rstCsPkgBody = rstCsPkg.m_stBody.m_stTimeDelayReq;

    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_TIME_DELAY_NTF;
    SC_PKG_TIME_DELAY_NTF& rstScPkgBody = m_stScPkg.m_stBody.m_stTimeDelayNtf;
    rstScPkgBody.m_dwSeq = rstCsPkgBody.m_dwSeq;
    rstScPkgBody.m_llTimeStamp = rstCsPkgBody.m_llTimeStamp;

    FightSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int FightSurrenderReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("poPlayer is null");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    int8_t chGroup = poDungeon->GetGroupByPlayerPtr(poPlayer);
    DungeonLogic::Instance().HandleFightSurrender(poDungeon, chGroup);

    return 0;
}

int FightStateNtf_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL */)
{
    CS_PKG_FIGHT_STATE_NTF& rstCsPkgBody = rstCsPkg.m_stBody.m_stFightStateNtf;
    Player* poPlayer = PlayerMgr::Instance().GetBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exsit");
        return -1;
    }

    Dungeon* poDungeon = poPlayer->getDungeon();
    if (NULL == poDungeon)
    {
        LOGERR("player not in dungeon");
        return -1;
    }

    // 状态同步
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_STATE_SYN;
    SC_PKG_FIGHT_STATE_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightStateSyn;
    rstScPkgBody.m_stStateInfo = rstCsPkgBody.m_stStateInfo;

    poDungeon->Broadcast(&m_stScPkg);

    return 0;
}
