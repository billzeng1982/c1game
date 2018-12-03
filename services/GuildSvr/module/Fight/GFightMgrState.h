#pragma once

#include "GuildFightMgr.h"
#include "state.h"

class GFightMgrState: public IState<GuildFightMgr>
{
public:
	GFightMgrState() {}
	virtual ~GFightMgrState() {}

	virtual void Enter(GuildFightMgr* poGFightMgr) {}

	virtual void Update(GuildFightMgr* poGFightMgr, int iDeltaTime) {}

	virtual void Exit(GuildFightMgr* poGFightMgr) {}

	virtual void ChangeState(GuildFightMgr* poGFightMgr, int iNewState);

	virtual void HandleEvent(GuildFightMgr* poGFightMgr, int iEventID, void* pvEventPara) {}
};

class GFightMgrState_NotOpen : public GFightMgrState
{
public:
	GFightMgrState_NotOpen() {}
	virtual ~GFightMgrState_NotOpen() {}
    virtual void Enter(GuildFightMgr* poGFightMgr);
    virtual void Update(GuildFightMgr* poGFightMgr, int iDeltaTime);
	virtual void Exit(GuildFightMgr* poGFightMgr);
};

class GFightMgrState_None : public GFightMgrState
{
public:
	GFightMgrState_None() {}
	virtual ~GFightMgrState_None() {}
    virtual void Enter(GuildFightMgr* poGFightMgr);
    virtual void Update(GuildFightMgr* poGFightMgr, int iDeltaTime);
	virtual void Exit(GuildFightMgr* poGFightMgr);
};

class GFightMgrState_ApplyStart : public GFightMgrState
{
public:
	GFightMgrState_ApplyStart() {}
	virtual ~GFightMgrState_ApplyStart() {}
	virtual void Enter(GuildFightMgr* poGFightMgr);
    virtual void Update(GuildFightMgr* poGFightMgr, int iDeltaTime);
	virtual void Exit(GuildFightMgr* poGFightMgr);
};

class GFightMgrState_ApplyEnd : public GFightMgrState
{
public:
	GFightMgrState_ApplyEnd() {}
	virtual ~GFightMgrState_ApplyEnd() {}
	virtual void Enter(GuildFightMgr* poGFightMgr);
    virtual void Update(GuildFightMgr* poGFightMgr, int iDeltaTime);
	virtual void Exit(GuildFightMgr* poGFightMgr);
};

class GFightMgrState_FightPrepare : public GFightMgrState
{
public:
	GFightMgrState_FightPrepare() {}
	virtual ~GFightMgrState_FightPrepare() {}
	virtual void Enter(GuildFightMgr* poGFightMgr);
    virtual void Update(GuildFightMgr* poGFightMgr, int iDeltaTime);
	virtual void Exit(GuildFightMgr* poGFightMgr);
};

class GFightMgrState_FightStart : public GFightMgrState
{
public:
	GFightMgrState_FightStart() {}
	virtual ~GFightMgrState_FightStart() {}
	virtual void Enter(GuildFightMgr* poGFightMgr);
    virtual void Update(GuildFightMgr* poGFightMgr, int iDeltaTime);
	virtual void Exit(GuildFightMgr* poGFightMgr);
};

