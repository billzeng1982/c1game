#pragma once

#include "ObjectPool.h"
#include "singleton.h"

enum GAMEOBJECT_TYPE
{
	GAMEOBJ_LOGIC_TIMER = 1,
};


class GameObjectPool : public ObjectPool<GameObjectPool>, public TSingleton<GameObjectPool>
{
public:
	GameObjectPool()
	{

	}

private:

};

#define RELEASE_GAMEOBJECT( p ) do{ if(p) { GameObjectPool::Instance().Release(p); p=NULL; } }while(0)
#define GET_GAMEOBJECT(ClassType, EnumType) dynamic_cast<ClassType*>(GameObjectPool::Instance().Get(EnumType) )
#define FIND_GAMEOBJECT(ClassType, ObjID)	dynamic_cast<ClassType*>(GameObjectPool::Instance().Find(ObjID) )

