#include "LogMacros.h"
#include "common_proto.h"
#include "FightPlayerState.h"
#include "FightPlayer.h"
#include "GuildFightPoint.h"
#include "GuildFightArena.h"
#include "FightPlayerStateMachine.h"

using namespace PKGMETA;

void FightPlayerState::ChangeState(FightPlayer* poFightPlayer, int iNewState)
{
    assert(poFightPlayer);
    LOGRUN_r("FightPlayer(%s) change state from %d to %d", poFightPlayer->m_stPlayerInfo.m_szName, poFightPlayer->GetState(), iNewState);

    //退出旧状态
    int iOldState = poFightPlayer->GetState();
    FightPlayerState* poOldState = FightPlayerStateMachine::Instance().GetState(iOldState);
    assert(poOldState);
    poOldState->Exit(poFightPlayer);

    //进入新状态
    FightPlayerState* poNewState = FightPlayerStateMachine::Instance().GetState(iNewState);
    assert(poNewState);
    poFightPlayer->SetState(iNewState);
    poNewState->Enter(poFightPlayer);
}


void FightPlayerState_Idle::Enter(FightPlayer* poFightPlayer)
{
    //将玩家状态变化的消息加入战场状态同步队列
    poFightPlayer->m_poGuildFightArena->AddStateSync(poFightPlayer->m_stPlayerInfo);

}

void FightPlayerState_Idle::Update(FightPlayer* poFightPlayer, int iDeltaTime)
{
    //玩家阵营和据点阵营不相同，应该进入ATTACK状态
    if (poFightPlayer->m_stPlayerInfo.m_bCamp != poFightPlayer->m_poGuildFightPoint->m_stPointInfo.m_bCamp)
    {
        FightPlayerStateMachine::Instance().ChangeState(poFightPlayer, GUILD_FIGHT_PLAYER_STATE_ATTACK);
        return;
    }
}

void FightPlayerState_Idle::Exit(FightPlayer* poFightPlayer)
{

}


void FightPlayerState_Attack::Enter(FightPlayer* poFightPlayer)
{
    poFightPlayer->m_iTimeLeftDamage = poFightPlayer->m_dwDamageCycleMs;
    poFightPlayer->m_stPlayerInfo.m_ullStateParam = poFightPlayer->m_ullTimeMs + poFightPlayer->m_dwDamageCycleMs;

    //将玩家状态变化的消息加入战场状态同步队列
    poFightPlayer->m_poGuildFightArena->AddStateSync(poFightPlayer->m_stPlayerInfo);
}

void FightPlayerState_Attack::Update(FightPlayer* poFightPlayer, int iDeltaTime)
{
    //当玩家阵营和据点阵营相同时，应该进入Idle状态
    if (poFightPlayer->m_stPlayerInfo.m_bCamp == poFightPlayer->m_poGuildFightPoint->m_stPointInfo.m_bCamp)
    {
        FightPlayerStateMachine::Instance().ChangeState(poFightPlayer, GUILD_FIGHT_PLAYER_STATE_IDLE);
        return;
    }

    poFightPlayer->m_iTimeLeftDamage -= iDeltaTime;
    //造成伤害并重置伤害时间
    if (poFightPlayer->m_iTimeLeftDamage <= 0)
    {
        poFightPlayer->m_poGuildFightPoint->Damage(poFightPlayer);
        poFightPlayer->m_iTimeLeftDamage = poFightPlayer->m_dwDamageCycleMs;
        poFightPlayer->m_stPlayerInfo.m_ullStateParam = poFightPlayer->m_ullTimeMs + poFightPlayer->m_dwDamageCycleMs;
    }
}

void FightPlayerState_Attack::Exit(FightPlayer* poFightPlayer)
{

}


void FightPlayerState_Move::Enter(FightPlayer* poFightPlayer)
{
    //移动后,将玩家从所在据点删除
    poFightPlayer->m_poGuildFightPoint->DelPlayer(poFightPlayer);
    poFightPlayer->m_stPlayerInfo.m_ullStateParam = poFightPlayer->m_ullTimeMs + poFightPlayer->m_iTimeLeftMove;

    //将玩家状态变化的消息加入战场状态同步队列
    poFightPlayer->m_poGuildFightArena->AddStateSync(poFightPlayer->m_stPlayerInfo);
}

void FightPlayerState_Move::Update(FightPlayer* poFightPlayer, int iDeltaTime)
{
    poFightPlayer->m_iTimeLeftMove -= iDeltaTime;
    //移动
    if (poFightPlayer->m_iTimeLeftMove <= 0)
    {
        poFightPlayer->m_stPlayerInfo.m_wPointId = poFightPlayer->m_wDstPointId;
        GuildFightPoint* poGuildFightPoint = poFightPlayer->m_poGuildFightArena->GetPoint(poFightPlayer->m_wDstPointId);
        assert(poGuildFightPoint);
        poFightPlayer->m_poGuildFightPoint = poGuildFightPoint;

        //将玩家加入新据点，根据新据点的阵营，玩家的新状态可能不同,状态的变化由AddPlayer函数控制
        poGuildFightPoint->AddPlayer(poFightPlayer);
    }
}

void FightPlayerState_Move::Exit(FightPlayer* poFightPlayer)
{

}


void FightPlayerState_Dead::Enter(FightPlayer* poFightPlayer)
{
    poFightPlayer->m_iTimeLeftRevive = poFightPlayer->m_dwReviveCoolTimeMs;
    poFightPlayer->m_stPlayerInfo.m_ullStateParam = poFightPlayer->m_ullTimeMs + poFightPlayer->m_dwReviveCoolTimeMs;

    //将玩家状态变化的消息加入战场状态同步队列
    poFightPlayer->m_poGuildFightArena->AddStateSync(poFightPlayer->m_stPlayerInfo);
}

void FightPlayerState_Dead::Update(FightPlayer* poFightPlayer, int iDeltaTime)
{
    poFightPlayer->m_iTimeLeftRevive -= iDeltaTime;
    if (poFightPlayer->m_iTimeLeftRevive <= 0)
    {
        poFightPlayer->Revive();
    }
}

void FightPlayerState_Dead::Exit(FightPlayer* poFightPlayer)
{

}

void FightPlayerState_Match::Enter(FightPlayer* poFightPlayer)
{
    //将玩家状态变化的消息加入战场状态同步队列
    poFightPlayer->m_poGuildFightArena->AddStateSync(poFightPlayer->m_stPlayerInfo);
}

void FightPlayerState_Match::Update(FightPlayer* poFightPlayer, int iDeltaTime)
{

}

void FightPlayerState_Match::Exit(FightPlayer* poFightPlayer)
{

}

void FightPlayerState_MatchEnd::Enter(FightPlayer* poFightPlayer)
{
    //将玩家状态变化的消息加入战场状态同步队列
    poFightPlayer->m_poGuildFightArena->AddStateSync(poFightPlayer->m_stPlayerInfo);
}

void FightPlayerState_MatchEnd::Update(FightPlayer* poFightPlayer, int iDeltaTime)
{

}

void FightPlayerState_MatchEnd::Exit(FightPlayer* poFightPlayer)
{

}


void FightPlayerState_PreMatch::Enter(FightPlayer* poFightPlayer)
{
    //将玩家状态变化的消息加入战场状态同步队列
    poFightPlayer->m_poGuildFightArena->AddStateSync(poFightPlayer->m_stPlayerInfo);
}

void FightPlayerState_PreMatch::Update(FightPlayer* poFightPlayer, int iDeltaTime)
{

}

void FightPlayerState_PreMatch::Exit(FightPlayer* poFightPlayer)
{

}

void FightPlayerState_God::Enter(FightPlayer* poFightPlayer)
{
    //伤害时间
    poFightPlayer->m_iTimeLeftDamage = poFightPlayer->m_dwDamageCycleMs;
    //God时间
    poFightPlayer->m_iGodLeftTimeMs = poFightPlayer->m_iIniGodLeftTimeMs;

    poFightPlayer->m_stPlayerInfo.m_ullStateParam = poFightPlayer->m_ullTimeMs + poFightPlayer->m_iGodLeftTimeMs;

    //将玩家状态变化的消息加入战场状态同步队列
    poFightPlayer->m_poGuildFightArena->AddStateSync(poFightPlayer->m_stPlayerInfo);
}

void FightPlayerState_God::Update(FightPlayer* poFightPlayer, int iDeltaTime)
{
    if (poFightPlayer->m_iGodLeftTimeMs <= iDeltaTime)
    {
        //  从无敌状态退出时,再匹配
        if(! poFightPlayer->m_poGuildFightPoint->GodExitMatch(poFightPlayer)) 
        {
            FightPlayerStateMachine::Instance().ChangeState(poFightPlayer, GUILD_FIGHT_PLAYER_STATE_IDLE);
        }
        poFightPlayer->m_iGodLeftTimeMs = 0;
        return;
    }
    poFightPlayer->m_iGodLeftTimeMs  -= iDeltaTime;
    poFightPlayer->m_stPlayerInfo.m_ullStateParam = poFightPlayer->m_ullTimeMs + poFightPlayer->m_iGodLeftTimeMs;

    poFightPlayer->m_iTimeLeftDamage -= iDeltaTime;
    //造成伤害并重置伤害时间
    if (poFightPlayer->m_iTimeLeftDamage <= 0 && poFightPlayer->m_stPlayerInfo.m_bCamp != poFightPlayer->m_poGuildFightPoint->m_stPointInfo.m_bCamp)
    {
        poFightPlayer->m_poGuildFightPoint->Damage(poFightPlayer);
        poFightPlayer->m_iTimeLeftDamage = poFightPlayer->m_dwDamageCycleMs;
    }
}

void FightPlayerState_God::Exit(FightPlayer* poFightPlayer)
{
}
