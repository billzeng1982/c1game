#pragma once

#include "ObjectPool.h"
#include "singleton.h"
#include "../UDP/UDPTimer.h"
#include "../UDP/UDPClientBuffIndex.h"
#include "../UDP/UDPBlock.h"

enum GAMEOBJECT_TYPE
{
	GAMEOBJ_UDPTIMER = 1,
	GAMEOBJ_UDPBUFFINDEX = 2,	
	GAMEOBJ_UDPBLOCK = 3,
};

class GameObjectPool : public ObjectPool<GameObjectPool>, public TSingleton<GameObjectPool>
{
public:
	GameObjectPool()
	{
		REGISTER_OBJ_CREATOR( GAMEOBJ_UDPTIMER, 	GameObjectPool, UDPTimer );
		REGISTER_OBJ_CREATOR( GAMEOBJ_UDPBUFFINDEX, GameObjectPool, UDPClientBuffIndex );
		REGISTER_OBJ_CREATOR( GAMEOBJ_UDPBLOCK, 	GameObjectPool, UDPBlock );
	}

private:
	OBJ_CREATOR( UDPTimer );
	OBJ_CREATOR( UDPClientBuffIndex );
	OBJ_CREATOR( UDPBlock );
};

#define RELEASE_GAMEOBJECT( p ) do{ if(p) { GameObjectPool::Instance().Release(p); p=NULL; } }while(0)
#define GET_GAMEOBJECT(ClassType, EnumType) dynamic_cast<ClassType*>(GameObjectPool::Instance().Get(EnumType) )
#define FIND_GAMEOBJECT(ClassType, ObjID)	dynamic_cast<ClassType*>(GameObjectPool::Instance().Find(ObjID) )

