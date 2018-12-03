#pragma once
#include "singleton.h"
#include "StateMachine.h"
#include "PlayerState.h"

class PlayerStateMachine: public IStateMachine<PlayerState, Player>, public TSingleton<PlayerStateMachine> 
{
public:
	PlayerStateMachine() {}
	virtual ~PlayerStateMachine() {}
	virtual PlayerState* GetState(int iState);

protected:
	PlayerState_Null m_oStateNull;
	PlayerState_Login m_oStateLogin;
	PlayerState_InGame m_oStateInGame;
	PlayerState_Logout m_oStateLogout;
	PlayerState_ReconnWait m_oStateReconnWait;
	PlayerState_ReconnLogin m_oStateReconnLogin;
};

inline PlayerState* PlayerStateMachine::GetState(int iState) 
{
	switch (iState) 
	{
	case PLAYER_STATE_NULL:
		return &m_oStateNull;
	case PLAYER_STATE_LOGIN:
		return &m_oStateLogin;
	case PLAYER_STATE_INGAME:
		return &m_oStateInGame;
	case PLAYER_STATE_LOGOUT:
		return &m_oStateLogout;
	case PLAYER_STATE_RECONN_WAIT:
		return &m_oStateReconnWait;
	case PLAYER_STATE_RECONN_LOGIN:
		return &m_oStateReconnLogin;
	}

	return NULL;
}

