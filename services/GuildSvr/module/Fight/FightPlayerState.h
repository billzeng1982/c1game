#pragma once

#include "FightPlayer.h"
#include "state.h"

class FightPlayerState: public IState<FightPlayer>
{
public:
    FightPlayerState() {}
    virtual ~FightPlayerState() {}

    virtual void Enter(FightPlayer* poFightPlayer) {}

    virtual void Update(FightPlayer* poFightPlayer, int iDeltaTime) {}

    virtual void Exit(FightPlayer* poFightPlayer) {}

    virtual void ChangeState(FightPlayer* poFightPlayer, int iNewState);

    virtual void HandleEvent(FightPlayer* poFightPlayer, int iEventID, void* pvEventPara) {}
};

class FightPlayerState_Quit : public FightPlayerState
{
public:
    FightPlayerState_Quit() {}
    virtual ~FightPlayerState_Quit() {}
    virtual void Enter(FightPlayer* poFightPlayer){}
    virtual void Update(FightPlayer* poFightPlayer, int iDeltaTime){}
    virtual void Exit(FightPlayer* poFightPlayer){}
};


class FightPlayerState_None : public FightPlayerState
{
public:
    FightPlayerState_None() {}
    virtual ~FightPlayerState_None() {}
    virtual void Enter(FightPlayer* poFightPlayer){}
    virtual void Update(FightPlayer* poFightPlayer, int iDeltaTime){}
    virtual void Exit(FightPlayer* poFightPlayer){}
};

class FightPlayerState_Idle : public FightPlayerState
{
public:
    FightPlayerState_Idle() {}
    virtual ~FightPlayerState_Idle() {}
    virtual void Enter(FightPlayer* poFightPlayer);
    virtual void Update(FightPlayer* poFightPlayer, int iDeltaTime);
    virtual void Exit(FightPlayer* poFightPlayer);
};

class FightPlayerState_Attack : public FightPlayerState
{
public:
    FightPlayerState_Attack() {}
    virtual ~FightPlayerState_Attack() {}
    virtual void Enter(FightPlayer* poFightPlayer);
    virtual void Update(FightPlayer* poFightPlayer, int iDeltaTime);
    virtual void Exit(FightPlayer* poFightPlayer);
};

class FightPlayerState_Move : public FightPlayerState
{
public:
    FightPlayerState_Move() {}
    virtual ~FightPlayerState_Move() {}
    virtual void Enter(FightPlayer* poFightPlayer);
    virtual void Update(FightPlayer* poFightPlayer, int iDeltaTime);
    virtual void Exit(FightPlayer* poFightPlayer);
};

class FightPlayerState_Dead : public FightPlayerState
{
public:
    FightPlayerState_Dead() {}
    virtual ~FightPlayerState_Dead() {}
    virtual void Enter(FightPlayer* poFightPlayer);
    virtual void Update(FightPlayer* poFightPlayer, int iDeltaTime);
    virtual void Exit(FightPlayer* poFightPlayer);
};

class FightPlayerState_Match : public FightPlayerState
{
public:
    FightPlayerState_Match() {}
    virtual ~FightPlayerState_Match() {}
    virtual void Enter(FightPlayer* poFightPlayer);
    virtual void Update(FightPlayer* poFightPlayer, int iDeltaTime);
    virtual void Exit(FightPlayer* poFightPlayer);
};

class FightPlayerState_MatchEnd : public FightPlayerState
{
public:
    FightPlayerState_MatchEnd() {}
    virtual ~FightPlayerState_MatchEnd() {}
    virtual void Enter(FightPlayer* poFightPlayer);
    virtual void Update(FightPlayer* poFightPlayer, int iDeltaTime);
    virtual void Exit(FightPlayer* poFightPlayer);
};

class FightPlayerState_PreMatch : public FightPlayerState
{
public:
    FightPlayerState_PreMatch() {}
    virtual ~FightPlayerState_PreMatch() {}
    virtual void Enter(FightPlayer* poFightPlayer);
    virtual void Update(FightPlayer* poFightPlayer, int iDeltaTime);
    virtual void Exit(FightPlayer* poFightPlayer);
};

class FightPlayerState_God : public FightPlayerState
{
public:
    FightPlayerState_God() {}
    virtual ~FightPlayerState_God() {}
    virtual void Enter(FightPlayer* poFightPlayer);
    virtual void Update(FightPlayer* poFightPlayer, int iDeltaTime);
    virtual void Exit(FightPlayer* poFightPlayer);
};

