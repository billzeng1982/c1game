#pragma once

#include "ObjectPool.h"
#include "singleton.h"
#include "../module/FriendMgr.h"
#include "../module/FriendTransaction.h"

enum GAMEOBJECT_TYPE
{
	GAMEOBJ_GET_FRIEND_ACTION = 1,
	GAMEOBJ_GETCREATE_FRIEND_ACTION,
	GAMEOBJ_FRIEND_TRANSACTION,
	GAMEOBJ_GET_AGREE_APPLY_ACTION,
	GAMEOBJ_GET_PLAYER_INFO_ACTION
};

class GameObjectPool : public ObjectPool<GameObjectPool>, public TSingleton<GameObjectPool>
{
public:
	GameObjectPool()
	{
		REGISTER_OBJ_CREATOR(GAMEOBJ_FRIEND_TRANSACTION,			GameObjectPool, FriendTransaction);
		REGISTER_OBJ_CREATOR(GAMEOBJ_GET_FRIEND_ACTION,				GameObjectPool, GetFriendAction);
		REGISTER_OBJ_CREATOR(GAMEOBJ_GETCREATE_FRIEND_ACTION,		GameObjectPool, GetCreateFriendAction);
		REGISTER_OBJ_CREATOR(GAMEOBJ_GET_AGREE_APPLY_ACTION,		GameObjectPool, GetAgreeApplyAction);
		REGISTER_OBJ_CREATOR(GAMEOBJ_GET_PLAYER_INFO_ACTION,		GameObjectPool, GetPlayerInfoAction);

	}

private:

	OBJ_CREATOR(GetFriendAction);
	OBJ_CREATOR(FriendTransaction);
	OBJ_CREATOR(GetAgreeApplyAction);
	OBJ_CREATOR(GetCreateFriendAction);
	OBJ_CREATOR(GetPlayerInfoAction);
};

#define RELEASE_GAMEOBJECT( p ) do{ if(p) { GameObjectPool::Instance().Release(p); p=NULL; } }while(0)
#define GET_GAMEOBJECT(ClassType, EnumType) dynamic_cast<ClassType*>(GameObjectPool::Instance().Get(EnumType) )
// #define FIND_GAMEOBJECT(ClassType, ObjID)	dynamic_cast<ClassType*>(GameObjectPool::Instance().Find(ObjID) )

