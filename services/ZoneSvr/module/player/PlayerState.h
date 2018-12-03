#pragma once
#include "state.h"

class Player;
class PlayerState: public IState<Player>
{
public:
	PlayerState() {}
	virtual ~PlayerState() {}

	// this will execute when the state is entered
	virtual void Enter(Player* poPlayer) {}

	// this is the states normal update function
	virtual void Update(Player* poPlayer, int iDeltaTime) {}

	// this will execute when the state is exited
	virtual void Exit(Player* poPlayer) {}

	virtual void ChangeState(Player* poPlayer, int iNewState);

	virtual void HandleEvent(Player* poPlayer, int iEventID, void* pvEventPara) {}
};

class PlayerState_Null : public PlayerState
{
public:
	PlayerState_Null() {}
	virtual ~PlayerState_Null() {}
};

class PlayerState_Login : public PlayerState
{
public:
	PlayerState_Login() {}
	virtual ~PlayerState_Login() {}
	virtual void Enter(Player* poPlayer);
    virtual void Update(Player* poPlayer, int iDeltaTime);
	virtual void Exit(Player* poPlayer);
};

class PlayerState_InGame : public PlayerState
{
public:
	PlayerState_InGame() {}
	virtual ~PlayerState_InGame() {}
	virtual void Enter(Player* poPlayer);
	virtual void Update(Player* poPlayer, int iDeltaTime);
	virtual void Exit(Player* poPlayer);
};

class PlayerState_Logout : public PlayerState
{
public:
	PlayerState_Logout() {}
	virtual ~PlayerState_Logout() {}
	virtual void Enter(Player* poPlayer);
	virtual void Exit(Player* poPlayer);
};

class PlayerState_ReconnWait : public PlayerState
{
public:
	PlayerState_ReconnWait() {}
	virtual ~PlayerState_ReconnWait() {}
	virtual void Enter(Player* poPlayer);
	virtual void Update(Player* poPlayer, int iDeltaTime);
	virtual void Exit(Player* poPlayer);
};

class PlayerState_ReconnLogin : public PlayerState
{
public:
	PlayerState_ReconnLogin() {}
	virtual ~PlayerState_ReconnLogin() {}
	virtual void Enter(Player* poPlayer);
    virtual void Update(Player* poPlayer, int iDeltaTime);
	virtual void Exit(Player* poPlayer);
};

