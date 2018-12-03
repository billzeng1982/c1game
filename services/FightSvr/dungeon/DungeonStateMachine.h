#pragma once
#include "singleton.h"
#include "StateMachine.h"
#include "DungeonState.h"

class DungeonStateMachine: public IStateMachine<DungeonState, Dungeon>,
	public TSingleton<DungeonStateMachine>
{
public:
	DungeonStateMachine() {}
	virtual ~DungeonStateMachine() {}

	virtual DungeonState* GetState(int iState);

protected:
	DungeonState_WaitConnect    m_oDungeonWaitConnect;
	DungeonState_Choose			m_oDungeonChoose;
	DungeonState_WaitLoading	m_oDungeonWaitLoading;
	DungeonState_Op				m_oDungeonOp;
	DungeonState_Fight			m_oDungeonFight;
	DungeonState_LockAmbush		m_oDungeonLockAmbush;
	DungeonState_LockDead		m_oDungeonLockDead;
	DungeonState_LockSkill		m_oDungeonLockSkill;
	DungeonState_LockSolo		m_oDungeonLockSolo;
	DungeonState_LockMSkill		m_oDungeonLockMSkill;
	DungeonState_Ed				m_oDungeonEd;
};

inline DungeonState* DungeonStateMachine::GetState(int iState)
{
	switch (iState)
	{
	case DUNGEON_STATE_WAIT_CONNECT:
		return &m_oDungeonWaitConnect;
	case DUNGEON_STATE_CHOOSE:
		return &m_oDungeonChoose;
	case DUNGEON_STATE_WAIT_LOADING:
		return &m_oDungeonWaitLoading;
	case DUNGEON_STATE_OP:
		return &m_oDungeonOp;
	case DUNGEON_STATE_FIGHT:
		return &m_oDungeonFight;
	case DUNGEON_STATE_LOCK_AMBUSH:
		return &m_oDungeonLockAmbush;
	case DUNGEON_STATE_LOCK_DEAD:
		return &m_oDungeonLockDead;
	case DUNGEON_STATE_LOCK_SKILL:
		return &m_oDungeonLockSkill;
	case DUNGEON_STATE_LOCK_SOLO:
		return &m_oDungeonLockSolo;
	case DUNGEON_STATE_LOCK_MSKILL:
		return &m_oDungeonLockMSkill;
	case DUNGEON_STATE_ED:
		return &m_oDungeonEd;
	}

	return NULL;
}

