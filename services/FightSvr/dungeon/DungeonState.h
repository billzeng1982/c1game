#pragma once
#include "state.h"
#include "cs_proto.h"

class Dungeon;

enum DUNGEON_STATE
{
	DUNGEON_STATE_NONE = 0,
	DUNGEON_STATE_WAIT_CONNECT,	// 等待连接
	DUNGEON_STATE_CHOOSE,		// 等待选人
	DUNGEON_STATE_WAIT_LOADING,	// 等待加载
	DUNGEON_STATE_OP,			// 开场动画
	DUNGEON_STATE_FIGHT,		// 正常战斗
	DUNGEON_STATE_LOCK_AMBUSH,	// 伏兵暂停
	DUNGEON_STATE_LOCK_DEAD,	// 死亡暂停
	DUNGEON_STATE_LOCK_SKILL,	// 技能暂停
	DUNGEON_STATE_LOCK_SOLO,	// 单挑暂停
	DUNGEON_STATE_LOCK_MSKILL,	// 军师技暂停
	DUNGEON_STATE_ED,			// 战斗结束
};

class DungeonState: public IState<Dungeon>
{
public:
	DungeonState() {}
	virtual ~DungeonState() {}

	// this will execute when the state is entered
	virtual void Enter(Dungeon* poDungeon) {}

	// this is the states normal update function
	virtual void Update(Dungeon* poDungeon, int iDeltaTime) {}

	// this will execute when the state is exited
	virtual void Exit(Dungeon* poDungeon) {}

	virtual void ChangeState(Dungeon* poDungeon, int iNewState);

	virtual void HandleEvent(Dungeon* poDungeon, int iEventID, void* pvEventPara) {}
};


class DungeonState_WaitConnect : public DungeonState
{
public:
	DungeonState_WaitConnect() {}
	virtual ~DungeonState_WaitConnect() {}

	virtual void Enter(Dungeon* poDungeon);

	virtual void Update(Dungeon* poDungeon, int iDeltaTime);

	virtual void Exit(Dungeon* poDungeon);
};


class DungeonState_Choose : public DungeonState
{
public:
	DungeonState_Choose() {}
	virtual ~DungeonState_Choose() {}

	virtual void Enter(Dungeon* poDungeon);

	virtual void Update(Dungeon* poDungeon, int iDeltaTime);

	virtual void Exit(Dungeon* poDungeon);
};


class DungeonState_WaitLoading: public DungeonState
{
public:
	DungeonState_WaitLoading() {}
	virtual ~DungeonState_WaitLoading() {}

	// this will execute when the state is entered
	virtual void Enter(Dungeon* poDungeon);

	// this is the states normal update function
	virtual void Update(Dungeon* poDungeon, int iDeltaTime);

	// this will execute when the state is exited
	virtual void Exit(Dungeon* poDungeon) {}
};

class DungeonState_Op: public DungeonState
{
public:
	DungeonState_Op() {}
	virtual ~DungeonState_Op() {}

	// this will execute when the state is entered
	virtual void Enter(Dungeon* poDungeon);

	// this is the states normal update function
	virtual void Update(Dungeon* poDungeon, int iDeltaTime);

	// this will execute when the state is exited
	virtual void Exit(Dungeon* poDungeon) {}
};

class DungeonState_Fight: public DungeonState
{
public:
	DungeonState_Fight() {}
	virtual ~DungeonState_Fight() {}

	// this will execute when the state is entered
	virtual void Enter(Dungeon* poDungeon);

	// this is the states normal update function
	virtual void Update(Dungeon* poDungeon, int iDeltaTime);

	// this will execute when the state is exited
	virtual void Exit(Dungeon* poDungeon) {}
};

class DungeonState_LockAmbush: public DungeonState
{
public:
	DungeonState_LockAmbush() {}
	virtual ~DungeonState_LockAmbush() {}

	// this will execute when the state is entered
	virtual void Enter(Dungeon* poDungeon);

	// this is the states normal update function
	virtual void Update(Dungeon* poDungeon, int iDeltaTime);

	// this will execute when the state is exited
	virtual void Exit(Dungeon* poDungeon) {}
};

class DungeonState_LockDead: public DungeonState
{
public:
	DungeonState_LockDead() {}
	virtual ~DungeonState_LockDead() {}

	// this will execute when the state is entered
	virtual void Enter(Dungeon* poDungeon);

	// this is the states normal update function
	virtual void Update(Dungeon* poDungeon, int iDeltaTime);

	// this will execute when the state is exited
	virtual void Exit(Dungeon* poDungeon) {}
};

class DungeonState_LockSkill: public DungeonState
{
public:
	DungeonState_LockSkill() {}
	virtual ~DungeonState_LockSkill() {}

	// this will execute when the state is entered
	virtual void Enter(Dungeon* poDungeon);

	// this is the states normal update function
	virtual void Update(Dungeon* poDungeon, int iDeltaTime);

	// this will execute when the state is exited
	virtual void Exit(Dungeon* poDungeon) {}
};

class DungeonState_LockSolo: public DungeonState
{
public:
	DungeonState_LockSolo() {}
	virtual ~DungeonState_LockSolo() {}

	// this will execute when the state is entered
	virtual void Enter(Dungeon* poDungeon);

	// this is the states normal update function
	virtual void Update(Dungeon* poDungeon, int iDeltaTime);

	// this will execute when the state is exited
	virtual void Exit(Dungeon* poDungeon) {}
};

class DungeonState_LockMSkill: public DungeonState
{
public:
	DungeonState_LockMSkill() {}
	virtual ~DungeonState_LockMSkill() {}

	// this will execute when the state is entered
	virtual void Enter(Dungeon* poDungeon);

	// this is the states normal update function
	virtual void Update(Dungeon* poDungeon, int iDeltaTime);

	// this will execute when the state is exited
	virtual void Exit(Dungeon* poDungeon) {}
};

class DungeonState_Ed: public DungeonState
{
public:
	DungeonState_Ed() {}
	virtual ~DungeonState_Ed() {}

	// this will execute when the state is entered
	virtual void Enter(Dungeon* poDungeon);

	// this is the states normal update function
	virtual void Update(Dungeon* poDungeon, int iDeltaTime) {}

	// this will execute when the state is exited
	virtual void Exit(Dungeon* poDungeon) {}
};
