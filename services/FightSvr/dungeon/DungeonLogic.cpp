#include "strutil.h"
#include "LogMacros.h"
#include "ObjectUpdatorMgr.h"
#include "DungeonLogic.h"
#include "DungeonMgr.h"
#include "DungeonStateMachine.h"
#include "../module/fightobj/City.h"
#include "../framework/GameObjectPool.h"
#include "../framework/FightSvrMsgLayer.h"
#include "GameTime.h"
#include "FakeRandom.h"

using namespace PKGMETA;

void DungeonLogic::SyncDungeonTime(Dungeon* poDungeon)
{
    if (poDungeon == NULL)
    {
        return;
    }

    m_stScPkg.m_stHead.m_wMsgId = PKGMETA::SC_MSG_DUNGEON_TIME_SYN;
    m_stScPkg.m_stBody.m_stDungeonTimeSyn.m_dwTimeLeft = (uint32_t)(poDungeon->m_iTimeLeft4Fight >= 0 ? poDungeon->m_iTimeLeft4Fight : 0);

    poDungeon->Broadcast(&m_stScPkg);
}

void DungeonLogic::SyncServerTime(Dungeon* poDungeon)
{
    // 对时协议
    m_stScPkg.m_stHead.m_wMsgId = PKGMETA::SC_MSG_TIME_SYNC_NTF;
    m_stScPkg.m_stBody.m_stTimeSyncNtf.m_llTimeStamp = (int64_t)(CGameTime::Instance().GetCurrTimeMs());
    LOGRUN("sync time-%ld", m_stScPkg.m_stBody.m_stTimeSyncNtf.m_llTimeStamp);
    poDungeon->BroadcastWithoutAck(&m_stScPkg);
}

void DungeonLogic::SendChooseStart(Dungeon* poDungeon)
{
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CHOOSE_BEGIN_SYN;
    SC_PKG_CHOOSE_BEGIN_SYN& rstChooseStartSyn = m_stScPkg.m_stBody.m_stChooseBeginSyn;

    poDungeon->GenChooseGeneral();

    RESPEAKARENAPARA* pResPara = CGameDataMgr::Instance().GetResPeakArenaParaMgr().Find(14);
    uint32_t dwInvisibleNum = pResPara->m_paramList[0];

    rstChooseStartSyn.m_bGeneralCount = poDungeon->m_bGeneralCount;
    for (int i=0; i<rstChooseStartSyn.m_bGeneralCount; i++)
    {
        rstChooseStartSyn.m_astGeneralList[i] = poDungeon->m_GeneralList[i];

        uint32_t dwRandomNum = CFakeRandom::Instance().Random(rstChooseStartSyn.m_bGeneralCount - i);
        if (dwRandomNum < dwInvisibleNum)
        {
            rstChooseStartSyn.m_szVisible[i] = 1;
            dwInvisibleNum--;
        }
        else
        {
            rstChooseStartSyn.m_szVisible[i] = 0;
        }
    }

    rstChooseStartSyn.m_bMSkillCount = poDungeon->m_bMSkillCount;
    for (int i=0; i<rstChooseStartSyn.m_bMSkillCount; i++)
    {
        rstChooseStartSyn.m_astMSkillList[i] = poDungeon->m_MSkillList[i];
    }

    poDungeon->Broadcast(&m_stScPkg);
}

void DungeonLogic::SendTurnStart(Dungeon* poDungeon)
{
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_TURN_BEGIN_SYN;
    SC_PKG_TURN_BEGIN_SYN& rstTurnStartSyn = m_stScPkg.m_stBody.m_stTurnBeginSyn;

    FightPlayer* poPlayer = poDungeon->GetFightPlayerByGroup(poDungeon->m_bChooseGroup);
    if (poPlayer)
    {
        rstTurnStartSyn.m_ullUin = poPlayer->m_ullUin;
        rstTurnStartSyn.m_dwTimeLeft = poDungeon->m_iTimeLeft4Choose - 1 * 1000;
        rstTurnStartSyn.m_bChooseNum = Dungeon::ChooseOrder[poDungeon->m_iCntLeft4Choose-1];
        rstTurnStartSyn.m_bChooseType = (poDungeon->m_iCntLeft4Choose==1) ? 1 : 0;
        poDungeon->Broadcast(&m_stScPkg);
    }
}

void DungeonLogic::ChgToNextTurn(Dungeon* poDungeon)
{
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CHOOSE_GENERAL_SYN;
    SC_PKG_CHOOSE_GENERAL_SYN& rstScChooseSync = m_stScPkg.m_stBody.m_stChooseGeneralSyn;
    FightPlayer* poPlayer;
    //前7回合选武将
    if (poDungeon->m_iCntLeft4Choose > 1)
    {
        poPlayer = poDungeon->GetFightPlayerByGroup(poDungeon->m_bChooseGroup);
        rstScChooseSync.m_ullUin = poPlayer->m_ullUin;

        //本回合结束时如果当前玩家还没有选择足够的武将，则系统自动帮其随机足够数量武将
        while (poDungeon->m_bChooseCount < Dungeon::ChooseOrder[poDungeon->m_iCntLeft4Choose-1])
        {
            rstScChooseSync.m_stMSkillInfo.m_bId = 0;
            DT_TROOP_INFO* pTroop = poPlayer->RandomChooseGenaral();
            if (pTroop)
            {
                rstScChooseSync.m_stTroopInfo = *pTroop;
                poDungeon->Broadcast(&m_stScPkg);
            }
        }
    }
    //最后一回合选军师技
    else
    {
        MasterSkill* poMSkill;
        for (int8_t i=PLAYER_GROUP_DOWN; i<=PLAYER_GROUP_UP; i++)
        {
            poPlayer = poDungeon->GetFightPlayerByGroup(i);
            rstScChooseSync.m_ullUin = poPlayer->m_ullUin;

            //本回合结束时如果当前玩家还没有选择军师技，则系统自动帮其随机军师技
            if (!poPlayer->m_poMasterSkill)
            {
                poMSkill = poPlayer->RandomChooseMSkill();
                if (poMSkill)
                {
                    rstScChooseSync.m_stMSkillInfo.m_bId = poMSkill->m_dwSkillId;
                    rstScChooseSync.m_stMSkillInfo.m_bLevel = poMSkill->m_bSkillLevel;
                    poDungeon->Broadcast(&m_stScPkg);
                }
            }
        }
    }

    //切换回合，并重置时间
    poDungeon->m_iCntLeft4Choose--;
    if (poDungeon->m_iCntLeft4Choose==1)
    {
        poDungeon->m_iTimeLeft4Choose = Dungeon::TimeOut4ChooseMSkill;
    }
    else
    {
        poDungeon->m_iTimeLeft4Choose = Dungeon::TimeOut4ChooseGeneral;
    }
    poDungeon->m_bChooseCount = 0;

    if (poDungeon->m_iCntLeft4Choose > 0)
    {
        poDungeon->m_bChooseGroup = (poDungeon->m_bChooseGroup == PLAYER_GROUP_UP) ? PLAYER_GROUP_DOWN : PLAYER_GROUP_UP;
        LOGRUN("Chg to turn Group(%d), CntLeft4Choose(%d)", poDungeon->m_bChooseGroup, poDungeon->m_iCntLeft4Choose);
        this->SendTurnStart(poDungeon);
    }
    else
    {
        if (poDungeon->m_bFakeType == MATCH_FAKE_NONE)
        {
            //所有回合均已结束，则切换到下一个状态
            DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_WAIT_LOADING);
        }
        else
        {
            DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_ED);
        }
    }
}

void DungeonLogic::SendFightStart(Dungeon* poDungeon)
{
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_START_SYN;
    SC_PKG_FIGHT_START_SYN& rstFightStartSyn = m_stScPkg.m_stBody.m_stFightStartSyn;
    rstFightStartSyn.m_nErrNo = ERR_NONE;
    poDungeon->Broadcast(&m_stScPkg);
}

void DungeonLogic::SendDungeonStart(Dungeon* poDungeon, int16_t nErrNo)
{
    // 开场动画开始
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_DUNGEON_START_RSP;
    m_stScPkg.m_stBody.m_stDungeonStartRsp.m_nErrNo = nErrNo;
    poDungeon->Broadcast(&m_stScPkg);
}

void DungeonLogic::SendTroopDead(Dungeon* poDungeon, Troop* poDeadTroop, FightObj* poSource, uint8_t bDeadDelay = 0)
{
    m_stScPkg.m_stHead.m_wMsgId = PKGMETA::SC_MSG_FIGHT_DEAD_SYN;
    SC_PKG_FIGHT_DEAD_SYN& rstScPkgBody = m_stScPkg.m_stBody.m_stFightDeadSyn;
    DT_DEAD_INFO& rstInfo = rstScPkgBody.m_stDeadInfo;
    rstInfo.m_stDeadObj.m_chGroup = poDeadTroop->m_chGroup;
    rstInfo.m_stDeadObj.m_chType = poDeadTroop->m_chType;
    rstInfo.m_stDeadObj.m_bId = poDeadTroop->m_bId;

    rstInfo.m_stSourceObj.m_chGroup = poSource->m_chGroup;
    rstInfo.m_stSourceObj.m_chType = poSource->m_chType;
    rstInfo.m_stSourceObj.m_bId = poSource->m_bId;

    rstInfo.m_bDeadDelay = bDeadDelay;

    if (poSource->m_chType == FIGHTOBJ_TROOP)
    {
        Troop* poTroop = (Troop*)poSource;
        rstInfo.m_stSourcePos.m_iPosX = poTroop->m_stKillPos.m_iPosX;
        rstInfo.m_stSourcePos.m_iPosY = poTroop->m_stKillPos.m_iPosY;
    }

    poDungeon->Broadcast(&m_stScPkg);
}

void DungeonLogic::SendFightSettle(Dungeon* poDungeon, int8_t chWinGroup, int8_t chEndReason)
{
    LOGRUN("SendFightSettle, Dungeon(%u), timestamp(%lu), WinGroup(%d), EndReason(%d)",
            poDungeon->m_dwDungeonId, poDungeon->m_ullTimestamp, chWinGroup, chEndReason);

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_FIGHT_SETTLE_NTF;
    m_stSsPkg.m_stBody.m_stFightSettleNtf.m_chGroup = chWinGroup;
    m_stSsPkg.m_stBody.m_stFightSettleNtf.m_chReason = chEndReason;
    m_stSsPkg.m_stBody.m_stFightSettleNtf.m_ullTimeStamp = poDungeon->m_ullTimestamp;
    m_stSsPkg.m_stBody.m_stFightSettleNtf.m_bMatchType = poDungeon->m_bMatchType;
    m_stSsPkg.m_stBody.m_stFightSettleNtf.m_dwDungeonId = poDungeon->m_dwDungeonId;

    FightPlayer *poFightPlayerUp, *poFightPlayerDown;
    poFightPlayerDown = poDungeon->GetFightPlayerByGroup(PLAYER_GROUP_DOWN);
    poFightPlayerUp = poDungeon->GetFightPlayerByGroup(PLAYER_GROUP_UP);

    // settle task info
    City* poCityUp = poDungeon->GetCityById(PLAYER_GROUP_UP);
    City* poCityDown = poDungeon->GetCityById(PLAYER_GROUP_DOWN);

    // send to player down
    if (poFightPlayerDown != NULL)
    {
        // player down
        poFightPlayerDown->m_stTaskInfo.m_dwEnemySettleHP = poCityUp->m_iHpCur;
        poFightPlayerDown->m_stTaskInfo.m_dwSelfSettleHP = poCityDown->m_iHpCur;
        poFightPlayerDown->m_stTaskInfo.m_bEnemySettleHPPer = (poCityUp->m_iHpCur * 100 / poCityUp->m_oOriginRtData.m_arrAttrValue[ATTR_HP]);
        poFightPlayerDown->m_stTaskInfo.m_bSelfSettleHPPer = (poCityDown->m_iHpCur * 100 / poCityDown->m_oOriginRtData.m_arrAttrValue[ATTR_HP]);

        m_stSsPkg.m_stHead.m_ullReservId = (int)poFightPlayerDown->m_iZoneSvrProcId;
        m_stSsPkg.m_stBody.m_stFightSettleNtf.m_stDungeonTaskInfo = poFightPlayerDown->m_stTaskInfo;
        m_stSsPkg.m_stBody.m_stFightSettleNtf.m_stMyInfo = poFightPlayerDown->m_stPlayerInfo;
        if (poFightPlayerUp != NULL)
        {
            m_stSsPkg.m_stBody.m_stFightSettleNtf.m_stOpponentInfo = poFightPlayerUp->m_stPlayerInfo;
        }
        FightSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    }

    // send to player up
    if (poFightPlayerUp != NULL)
    {
        // player up
        poFightPlayerUp->m_stTaskInfo.m_dwEnemySettleHP = poCityDown->m_iHpCur;
        poFightPlayerUp->m_stTaskInfo.m_dwSelfSettleHP = poCityUp->m_iHpCur;
        poFightPlayerUp->m_stTaskInfo.m_bEnemySettleHPPer = (poCityDown->m_iHpCur * 100 / poCityDown->m_oOriginRtData.m_arrAttrValue[ATTR_HP]);
        poFightPlayerUp->m_stTaskInfo.m_bSelfSettleHPPer = (poCityUp->m_iHpCur * 100 / poCityUp->m_oOriginRtData.m_arrAttrValue[ATTR_HP]);

        m_stSsPkg.m_stHead.m_ullReservId = (int)poFightPlayerUp->m_iZoneSvrProcId;
        m_stSsPkg.m_stBody.m_stFightSettleNtf.m_stDungeonTaskInfo = poFightPlayerUp->m_stTaskInfo;
        m_stSsPkg.m_stBody.m_stFightSettleNtf.m_stMyInfo = poFightPlayerUp->m_stPlayerInfo;

        if (poFightPlayerDown != NULL)
        {
            m_stSsPkg.m_stBody.m_stFightSettleNtf.m_stOpponentInfo = poFightPlayerDown->m_stPlayerInfo;
        }
        FightSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    }
}

void DungeonLogic::SendSoloSettle(Dungeon* poDungeon)
{
#if 0
    if (poDungeon == NULL)
    {
        return;
    }

    if (poDungeon->m_chSoloSettleGroup != PLAYER_GROUP_NONE)
    {
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FIGHT_SOLO_SETTLE_SYN;

        int8_t chWinGroup = PLAYER_GROUP_NONE;
        poDungeon->SoloSettle(NULL, chWinGroup);

        Player* poPlayer = poDungeon->GetPlayerByGroup(poDungeon->m_chSoloSettleGroup);
        if (poPlayer != NULL)
        {
            // 把保存的单挑结果发给其他人
            m_stScPkg.m_stBody.m_stFightSoloSettleSyn.m_stSettleInfo = poDungeon->m_stSoloSettleInfo;

            poDungeon->Broadcast(&m_stScPkg, poPlayer);
        }
        else
        {
            // 发送空结果给所有玩家
            DT_SOLO_SETTLE_INFO& rstSettleInfo = m_stScPkg.m_stBody.m_stFightSoloSettleSyn.m_stSettleInfo;
            bzero(&rstSettleInfo, sizeof(rstSettleInfo));

            rstSettleInfo.m_bScoreCnt = 5;
            poDungeon->Broadcast(&m_stScPkg);
        }
    }
    else
    {
        // 发送空结果给所有玩家
        DT_SOLO_SETTLE_INFO& rstSettleInfo = m_stScPkg.m_stBody.m_stFightSoloSettleSyn.m_stSettleInfo;
        bzero(&rstSettleInfo, sizeof(rstSettleInfo));

        rstSettleInfo.m_bScoreCnt = 5;
        poDungeon->Broadcast(&m_stScPkg);
    }
#endif
}

void DungeonLogic::HandleFightOffLine(Dungeon* poDungeon)
{
    int8_t chWinGroup = PLAYER_GROUP_NONE;

    // 开始加载过程中，一方掉线，直接结算
    Player* poPlayer;
    poPlayer = poDungeon->GetPlayerByGroup(PLAYER_GROUP_DOWN);
    if (poPlayer != NULL)
    {
        chWinGroup = PLAYER_GROUP_DOWN;
    }

    poPlayer = poDungeon->GetPlayerByGroup(PLAYER_GROUP_UP);
    if (poPlayer != NULL)
    {
        chWinGroup = PLAYER_GROUP_UP;
    }

    //结算战斗并直接切到结束状态
    if (poDungeon->m_bFakeType == MATCH_FAKE_NONE)
    {
        SendFightSettle(poDungeon, chWinGroup, (int8_t)FIGHTSETTLE_ESCAPE);
    }
    DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_ED);

    return;
}

void DungeonLogic::HandleFightSurrender(Dungeon* poDungeon, int8_t chSurrenderGroup)
{
    int8_t chWinGroup = 0;
    if (chSurrenderGroup == PLAYER_GROUP_DOWN)
    {
        chWinGroup = PLAYER_GROUP_UP;
    }
    else
    {
        chWinGroup = PLAYER_GROUP_DOWN;
    }

    if (poDungeon->m_bFakeType == MATCH_FAKE_NONE)
    {
        SendFightSettle(poDungeon, chWinGroup, (int8_t)FIGHTSETTLE_SURRENDER);
    }

    DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_ED);
}

void UpdateDungeon( IObject* pObj, int iDeltaTime )
{
    if (NULL == pObj)
    {
        LOGERR("Null obj.");
        return;
    }

    assert( pObj->GetObjType() == GAMEOBJ_DUNGEON );

    Dungeon* poDungeon = dynamic_cast<Dungeon*>(pObj);

    // 更新副本
    poDungeon->Update(iDeltaTime);
}

