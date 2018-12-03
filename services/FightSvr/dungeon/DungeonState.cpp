#include <assert.h>
#include "GameTime.h"
#include "FakeRandom.h"
#include "LogMacros.h"
#include "ObjectUpdatorMgr.h"
#include "Dungeon.h"
#include "DungeonLogic.h"
#include "DungeonState.h"
#include "DungeonStateMachine.h"
#include "DungeonMgr.h"
#include "../module/fightobj/Troop.h"
#include "../module/fightobj/City.h"

using namespace PKGMETA;

void DungeonState::ChangeState(Dungeon* poDungeon, int iNewState)
{
    if (!poDungeon)
    {
        assert(false);
        return;
    }

    LOGRUN("Dungeon change state from <%d> to <%d>", poDungeon->GetState(), iNewState);

    // 当前状态退出
    this->Exit(poDungeon);

    // 设置新状态
    poDungeon->SetState(iNewState);

    DungeonState* poNewState = DungeonStateMachine::Instance().GetState(iNewState);

    // 进入新状态
    if (poNewState)
    {
        poNewState->Enter(poDungeon);
    }
}

void DungeonState_WaitConnect::Enter(Dungeon* poDungeon)
{
    poDungeon->m_iTimeLeft4WaitConnect = Dungeon::TimeOut4WaitConnect;

    // 开始副本调度
    LOGRUN("schedule dungeon id<%u>.", poDungeon->m_dwDungeonId);
    SCHEDULE_OBJECT(poDungeon);

}

void DungeonState_WaitConnect::Update(Dungeon* poDungeon, int iDeltaTime)
{
    poDungeon->m_iTimeLeft4WaitConnect -= iDeltaTime;
    if (poDungeon->m_bOnlinePlayerCnt >= poDungeon->m_bMaxPlayer)
    {
        if (poDungeon->m_bMatchType == MATCH_TYPE_PEAK_ARENA)
        {
            DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_CHOOSE);
        }
        else
        {
            DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_WAIT_LOADING);
        }
    }
    else if (poDungeon->m_iTimeLeft4WaitConnect <= 0)
    {
        DungeonLogic::Instance().HandleFightOffLine(poDungeon);
    }
}

void DungeonState_WaitConnect::Exit(Dungeon* poDungeon)
{

}

void DungeonState_Choose::Enter(Dungeon* poDungeon)
{
    poDungeon->m_iTimeLeft4Choose = Dungeon::TimeOut4ChooseGeneral;
    poDungeon->m_iCntLeft4Choose = Dungeon::CntOut4Choose;

    //进入选人阶段时，发送选人开始通知
    DungeonLogic::Instance().SendChooseStart(poDungeon);
    //发送回合开始通知
    DungeonLogic::Instance().SendTurnStart(poDungeon);
}

void DungeonState_Choose::Update(Dungeon* poDungeon, int iDeltaTime)
{
    //选人阶段有人掉线，直接结算
    if (poDungeon->m_bOnlinePlayerCnt < poDungeon->m_bMaxPlayer)
    {
        DungeonLogic::Instance().HandleFightOffLine(poDungeon);
        return;
    }

    poDungeon->m_iTimeLeft4Choose -= iDeltaTime;

    if ((poDungeon->m_bChooseCount >= Dungeon::ChooseOrder[poDungeon->m_iCntLeft4Choose-1]) ||
        (poDungeon->m_iTimeLeft4Choose <= 0))
    {
        DungeonLogic::Instance().ChgToNextTurn(poDungeon);
    }
}

void DungeonState_Choose::Exit(Dungeon* poDungeon)
{

}

void DungeonState_WaitLoading::Enter(Dungeon* poDungeon)
{
    poDungeon->m_iTimeLeft4WaitLoading = Dungeon::TimeOut4WaitLoading;

    //发送战斗开始通知
    DungeonLogic::Instance().SendFightStart(poDungeon);
}

void DungeonState_WaitLoading::Update(Dungeon* poDungeon, int iDeltaTime)
{
    //加载阶段有人掉线，直接结算
    if (poDungeon->m_bOnlinePlayerCnt < poDungeon->m_bMaxPlayer)
    {
        DungeonLogic::Instance().HandleFightOffLine(poDungeon);
    }

    poDungeon->m_iTimeLeft4WaitLoading -= iDeltaTime;
    if (poDungeon->m_iTimeLeft4WaitLoading <= 0)
    {
        LOGRUN("dungeon id<%u> wait start timeout.", poDungeon->m_dwDungeonId);

        DungeonLogic::Instance().SendDungeonStart(poDungeon, ERR_DUNGEON_WAIT_START_TIMEOUT);

        // 切换到开战状态
        DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_OP);
    }
}

void DungeonState_Op::Enter(Dungeon* poDungeon)
{
    poDungeon->m_iTimeLeft4Op = Dungeon::TimeOut4Op;
}

void DungeonState_Op::Update(Dungeon* poDungeon, int iDeltaTime)
{
    poDungeon->m_iTimeLeft4Op -= iDeltaTime;
    if (poDungeon->m_iTimeLeft4Op <= 0)
    {
        LOGRUN("dungeon id<%u> op ani timeout.", poDungeon->m_dwDungeonId);

        // 切到正常战斗
        DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_FIGHT);
    }
}

void DungeonState_Fight::Enter(Dungeon* poDungeon)
{
    // 每进入正常战斗状态都进行一次时间同步给客户端
    DungeonLogic::Instance().SyncDungeonTime(poDungeon);

    // 如果有死亡动画未播的部队，则切到死亡暂停状态
    if (poDungeon->HasTroopDead())
    {
        DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_LOCK_DEAD);
        return;
    }

    // 如果有伏兵动画未播的部队，则切到伏兵暂停状态
    if (poDungeon->HasTroopAmbush())
    {
        DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_LOCK_AMBUSH);
        return;
    }

    // 处理待处理的包
    poDungeon->DealCsPkgWaitDeal();
}

void DungeonState_Fight::Update(Dungeon* poDungeon, int iDeltaTime)
{
    // 结算判断
    int8_t chWinGroup = PLAYER_GROUP_NONE;
    int8_t chEndReason = FIGHTSETTLE_NONE;

    // 没有玩家在线，直接结算
    if (poDungeon->m_bOnlinePlayerCnt == 0)
    {
        chEndReason = FIGHTSETTLE_OFFLINE;
    }

    poDungeon->m_iTimeLeft4Fight -= iDeltaTime;
    if (poDungeon->m_iTimeLeft4Fight <= 0)
    {
        LOGRUN("dungeon id<%u> timeover.", poDungeon->m_dwDungeonId);
        chEndReason = FIGHTSETTLE_TIMEOVER;
    }

    City* poCityUp = poDungeon->GetCityById(PLAYER_GROUP_UP);
    City* poCityDown = poDungeon->GetCityById(PLAYER_GROUP_DOWN);
    if ((poCityUp->m_iHpCur <= 0) || (poCityDown->m_iHpCur <= 0))
    {
        LOGRUN("dungeon id<%u> end, HP(UP)-%d, HP(DOWN)-%d", poDungeon->m_dwDungeonId, poCityUp->m_iHpCur, poCityDown->m_iHpCur);
        chEndReason = FIGHTSETTLE_CITYBREAK;
    }

    if (chEndReason != 0)
    {
        if (poCityUp->m_iHpCur > poCityDown->m_iHpCur)
        {
            chWinGroup = poCityUp->m_chGroup;
        }
        else if (poCityUp->m_iHpCur < poCityDown->m_iHpCur)
        {
            chWinGroup = poCityDown->m_chGroup;
        }

        DungeonLogic::Instance().SendFightSettle(poDungeon, chWinGroup, chEndReason);

        // 切换到副本结束状态
        DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_ED);
        return;
    }else
    {
        // 士气更新
        poDungeon->UpdateMorale(iDeltaTime);
    }
}

void DungeonState_LockAmbush::Enter(Dungeon* poDungeon)
{
    poDungeon->m_iTimeLeft4LockAmbush = Dungeon::TimeOut4LockAmbush;
}

void DungeonState_LockAmbush::Update(Dungeon* poDungeon, int iDeltaTime)
{
    poDungeon->m_iTimeLeft4LockAmbush -= iDeltaTime;
    if (poDungeon->m_iTimeLeft4LockAmbush <= 0)
    {
        LOGRUN("dungeon id<%u> lock ambush timeout.", poDungeon->m_dwDungeonId);

        poDungeon->ClearTroopAmbush();

        // 切到正常战斗
        DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_FIGHT);
    }
}

void DungeonState_LockDead::Enter(Dungeon* poDungeon)
{
    poDungeon->m_iTimeLeft4LockDead = Dungeon::TimeOut4LockDead;
}

void DungeonState_LockDead::Update(Dungeon* poDungeon, int iDeltaTime)
{
    poDungeon->m_iTimeLeft4LockDead -= iDeltaTime;
    if (poDungeon->m_iTimeLeft4LockDead <= 0)
    {
        LOGRUN("dungeon id<%u> lock dead timeout.", poDungeon->m_dwDungeonId);

        poDungeon->ClearTroopDead();

        // 切到正常战斗
        DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_FIGHT);
    }
}

void DungeonState_LockSkill::Enter(Dungeon* poDungeon)
{
    poDungeon->m_iTimeLeft4LockSkill = Dungeon::TimeOut4LockSkill;
}

void DungeonState_LockSkill::Update(Dungeon* poDungeon, int iDeltaTime)
{
    poDungeon->m_iTimeLeft4LockSkill -= iDeltaTime;
    if (poDungeon->m_iTimeLeft4LockSkill <= 0)
    {
        LOGRUN("dungeon id<%u> lock skill timeout.", poDungeon->m_dwDungeonId);

        // 切到正常战斗
        DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_FIGHT);
    }
}

void DungeonState_LockSolo::Enter(Dungeon* poDungeon)
{
    poDungeon->m_iTimeLeft4LockSolo = Dungeon::TimeOut4LockSolo;
}

void DungeonState_LockSolo::Update(Dungeon* poDungeon, int iDeltaTime)
{
    poDungeon->m_iTimeLeft4LockSolo -= iDeltaTime;
    if (poDungeon->m_iTimeLeft4LockSolo <= 0)
    {
        LOGRUN("dungeon id<%u> lock solo timeout.", poDungeon->m_dwDungeonId);

        DungeonLogic::Instance().SendSoloSettle(poDungeon);

        // 切到正常战斗
        DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_FIGHT);
    }
}

void DungeonState_LockMSkill::Enter(Dungeon* poDungeon)
{
    poDungeon->m_iTimeLeft4LockMSkill = Dungeon::TimeOut4LockMSkill;
}

void DungeonState_LockMSkill::Update(Dungeon* poDungeon, int iDeltaTime)
{
    poDungeon->m_iTimeLeft4LockMSkill -= iDeltaTime;
    if (poDungeon->m_iTimeLeft4LockMSkill <= 0)
    {
        LOGRUN("dungeon id<%u> lock mskill timeout.", poDungeon->m_dwDungeonId);

        // 切到正常战斗
        DungeonStateMachine::Instance().ChangeState(poDungeon, DUNGEON_STATE_FIGHT);
    }
}

void DungeonState_Ed::Enter(Dungeon* poDungeon)
{
    // 取消调度
    LOGRUN("unschedule dungeon id<%u>.", poDungeon->m_dwDungeonId);
    UNSCHEDULE_OBJECT(poDungeon);

    // 清除副本玩家
    poDungeon->ClearPlayer();

    // 回收副本实例
    LOGRUN("delete dungeon id<%u>.", poDungeon->m_dwDungeonId);
    DungeonMgr::Instance().Delete(poDungeon);
}

