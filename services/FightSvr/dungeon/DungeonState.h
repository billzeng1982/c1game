#pragma once
#include "state.h"
#include "cs_proto.h"

class Dungeon;

enum DUNGEON_STATE
{
	DUNGEON_STATE_NONE = 0,
	DUNGEON_STATE_WAIT_CONNECT,	// �ȴ�����
	DUNGEON_STATE_CHOOSE,		// �ȴ�ѡ��
	DUNGEON_STATE_WAIT_LOADING,	// �ȴ�����
	DUNGEON_STATE_OP,			// ��������
	DUNGEON_STATE_FIGHT,		// ����ս��
	DUNGEON_STATE_LOCK_AMBUSH,	// ������ͣ
	DUNGEON_STATE_LOCK_DEAD,	// ������ͣ
	DUNGEON_STATE_LOCK_SKILL,	// ������ͣ
	DUNGEON_STATE_LOCK_SOLO,	// ������ͣ
	DUNGEON_STATE_LOCK_MSKILL,	// ��ʦ����ͣ
	DUNGEON_STATE_ED,			// ս������
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
