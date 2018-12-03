#pragma once
#include "singleton.h"
#include "StateMachine.h"
#include "GFightMgrState.h"
#include "common_proto.h"

using namespace PKGMETA;

class GFightMgrStateMachine: public IStateMachine<GFightMgrState, GuildFightMgr>, public TSingleton<GFightMgrStateMachine>
{
public:
	GFightMgrStateMachine() {}
	virtual ~GFightMgrStateMachine() {}
	virtual GFightMgrState* GetState(int iState);

protected:
	GFightMgrState_None m_oStateNull;
	GFightMgrState_ApplyStart m_oStateApplyStart;
	GFightMgrState_ApplyEnd m_oStateApplyEnd;
    GFightMgrState_FightPrepare m_oStateFightPrepare;
    GFightMgrState_FightStart m_oStateFightStart;
    GFightMgrState_NotOpen m_oStateNotOpen;
};

inline GFightMgrState* GFightMgrStateMachine::GetState(int iState)
{
	switch (iState)
	{
	case GUILD_FIGHT_STATE_NONE:
		return &m_oStateNull;
	case GUILD_FIGHT_STATE_APPLY_START:
		return &m_oStateApplyStart;
	case GUILD_FIGHT_STATE_APPLY_END:
		return &m_oStateApplyEnd;
    case GUILD_FIGHT_STATE_FIGHT_PREPARE:
		return &m_oStateFightPrepare;
    case GUILD_FIGHT_STATE_FIGHT_START:
		return &m_oStateFightStart;
    case GUILD_FIGHT_STATE_NOT_OPEN:
		return &m_oStateNotOpen;
	}
	return NULL;
}

