#pragma once

#include "ObjectPool.h"
#include "singleton.h"
#include "../module/MatchInfo.h"
#include "../module/MatchTimer.h"
#include "../module/MatchMgr.h"

enum GAMEOBJECT_TYPE
{
	GAMEOBJ_MATCH_INFO = 1,
	GAMEOBJ_MATCH_TIMER = 2,
	GAMEOBJ_MATCH_FAKENODE = 3,
};

class GameObjectPool : public ObjectPool<GameObjectPool>, public TSingleton<GameObjectPool>
{
public:
	GameObjectPool()
	{
		REGISTER_OBJ_CREATOR( GAMEOBJ_MATCH_INFO, GameObjectPool, MatchInfo );
		REGISTER_OBJ_CREATOR( GAMEOBJ_MATCH_TIMER, GameObjectPool, MatchTimer );
        REGISTER_OBJ_CREATOR( GAMEOBJ_MATCH_FAKENODE, GameObjectPool, FakeNode );
	}

private:
	OBJ_CREATOR( MatchInfo );
	OBJ_CREATOR( MatchTimer );
    OBJ_CREATOR( FakeNode );
};

#define RELEASE_GAMEOBJECT( p ) do{ if(p) { GameObjectPool::Instance().Release(p); p=NULL; } }while(0)
#define GET_GAMEOBJECT(ClassType, EnumType) dynamic_cast<ClassType*>(GameObjectPool::Instance().Get(EnumType) )
#define FIND_GAMEOBJECT(ClassType, ObjID)	dynamic_cast<ClassType*>(GameObjectPool::Instance().Find(ObjID) )

