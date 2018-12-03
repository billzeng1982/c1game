#pragma once
#include "singleton.h"
#include "StateMachine.h"
#include "FightPlayerState.h"
#include "common_proto.h"

using namespace PKGMETA;

class FightPlayerStateMachine: public IStateMachine<FightPlayerState, FightPlayer>, public TSingleton<FightPlayerStateMachine>
{
public:
	FightPlayerStateMachine() {}
	virtual ~FightPlayerStateMachine() {}
	virtual FightPlayerState* GetState(int iState);

protected:
	FightPlayerState_Idle m_oStateIdle;
	FightPlayerState_Attack m_oStateAttack;
	FightPlayerState_Move m_oStateMove;
    FightPlayerState_Dead m_oStateDead;
	FightPlayerState_Match m_oStateMatch;
    FightPlayerState_None m_oStateNone;
    FightPlayerState_MatchEnd m_oStateMatchEnd;
    FightPlayerState_Quit m_oStateQuit;
    FightPlayerState_PreMatch m_oStatePreMatch;
    FightPlayerState_God m_oStateGod;
};

inline FightPlayerState* FightPlayerStateMachine::GetState(int iState)
{
	switch (iState)
	{
    case GUILD_FIGHT_PLAYER_STATE_NONE:
        return &m_oStateNone;
	case GUILD_FIGHT_PLAYER_STATE_IDLE:
		return &m_oStateIdle;
	case GUILD_FIGHT_PLAYER_STATE_ATTACK:
		return &m_oStateAttack;
	case GUILD_FIGHT_PLAYER_STATE_MOVE:
		return &m_oStateMove;
    case GUILD_FIGHT_PLAYER_STATE_DEAD:
		return &m_oStateDead;
	case GUILD_FIGHT_PLAYER_STATE_MATCH:
		return &m_oStateMatch;
    case GUILD_FIGHT_PLAYER_STATE_MATCH_END:
		return &m_oStateMatchEnd;
    case GUILD_FIGHT_PLAYER_STATE_QUIT:
        return &m_oStateQuit;
    case GUILD_FIGHT_PLAYER_STATE_GOD:
        return &m_oStateGod;
    case GUILD_FIGHT_PLAYER_STATE_PRE_MATCH:
        return &m_oStatePreMatch;
	}
	return NULL;
}

