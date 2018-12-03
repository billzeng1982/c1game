#pragma once

#include "ObjectPool.h"
#include "singleton.h"
#include "../module/MessageMgr.h"
#include "../module/MessageTransaction.h"

enum GAMEOBJECT_TYPE
{
	GAMEOBJ_MESSAGE_TRANSACTION = 1,
	GAMEOBJ_GET_MESSAGE_ACTION,
	GAMEOBJ_MESSAGE_ONE_RECORD
};

class GameObjectPool : public ObjectPool<GameObjectPool>, public TSingleton<GameObjectPool>
{
public:
	GameObjectPool()
	{
		REGISTER_OBJ_CREATOR(GAMEOBJ_MESSAGE_TRANSACTION,				GameObjectPool, MessageTransaction);
		REGISTER_OBJ_CREATOR(GAMEOBJ_GET_MESSAGE_ACTION,				GameObjectPool, GetMessageAction);
		REGISTER_OBJ_CREATOR(GAMEOBJ_MESSAGE_ONE_RECORD,				GameObjectPool, OneRecordObj);

	}

private:

	OBJ_CREATOR(MessageTransaction);
	OBJ_CREATOR(GetMessageAction);
	OBJ_CREATOR(OneRecordObj);
};

#define RELEASE_GAMEOBJECT( p ) do{ if(p) { GameObjectPool::Instance().Release(p); p=NULL; } }while(0)
#define GET_GAMEOBJECT(ClassType, EnumType) dynamic_cast<ClassType*>(GameObjectPool::Instance().Get(EnumType) )
// #define FIND_GAMEOBJECT(ClassType, ObjID)	dynamic_cast<ClassType*>(GameObjectPool::Instance().Find(ObjID) )

