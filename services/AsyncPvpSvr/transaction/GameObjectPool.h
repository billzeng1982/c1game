#pragma once

#include "ObjectPool.h"
#include "singleton.h"
#include "AsyncPvpTransaction.h"

enum GAMEOBJECT_TYPE
{
    GAMEOBJ_GET_PLAYER_ACTION = 1,
    GAMEOBJ_CREATE_PLAYER_ACTION = 2,
    GAMEOBJ_GET_TEAM_ACTION = 3,
    GAMEOBJ_CREATE_TEAM_ACTION = 4,
    GAMEOBJ_COMPOSITE_ACTION = 5,
    GAMEOBJ_GETDATA_TRANS = 6,
    GAMEOBJ_ADDDATA_TRANS = 7,
    GAMEOBJ_REFRESH_OPPONENT_TRANS = 8,
    GAMEOBJ_GET_TOPLIST_TRANS = 9,
    GAMEOBJ_START_FIGHT_TRANS = 10,
    GAMEOBJ_SETTLE_FIGHT_TRANS = 11,
	GAMEOBJ_WORSHIP_TRANS = 12,
	GAMEOBJ_WORSHIP_MAIL_TRANS = 13,
	GAMEOBJ_GET_PLAYER_INFO_TRANS = 14,
};

class GameObjectPool : public ObjectPool<GameObjectPool>, public TSingleton<GameObjectPool>
{
public:
    GameObjectPool()
    {
        REGISTER_OBJ_CREATOR( GAMEOBJ_GET_PLAYER_ACTION,  	         GameObjectPool,  GetPlayerAction );
        REGISTER_OBJ_CREATOR( GAMEOBJ_CREATE_PLAYER_ACTION, 	     GameObjectPool,  CreatePlayerAction );
        REGISTER_OBJ_CREATOR( GAMEOBJ_GET_TEAM_ACTION,  	         GameObjectPool,  GetTeamAction );
        REGISTER_OBJ_CREATOR( GAMEOBJ_CREATE_TEAM_ACTION, 	         GameObjectPool,  CreateTeamAction );
        REGISTER_OBJ_CREATOR( GAMEOBJ_COMPOSITE_ACTION, 	         GameObjectPool,  CompositeAction);
        REGISTER_OBJ_CREATOR( GAMEOBJ_GETDATA_TRANS, 	             GameObjectPool,  GetDataTransAction);
        REGISTER_OBJ_CREATOR( GAMEOBJ_ADDDATA_TRANS,                 GameObjectPool,  AddDataTransAction);
        REGISTER_OBJ_CREATOR( GAMEOBJ_REFRESH_OPPONENT_TRANS, 	     GameObjectPool,  RefreshOpponentTransAction);
        REGISTER_OBJ_CREATOR( GAMEOBJ_GET_TOPLIST_TRANS, 	         GameObjectPool,  GetTopListTransAction);
        REGISTER_OBJ_CREATOR( GAMEOBJ_START_FIGHT_TRANS, 	         GameObjectPool,  FightStartTransAction);
        REGISTER_OBJ_CREATOR( GAMEOBJ_SETTLE_FIGHT_TRANS, 	         GameObjectPool,  FightSettleTransAction);
		REGISTER_OBJ_CREATOR( GAMEOBJ_WORSHIP_TRANS, 				 GameObjectPool,  WorshipTransAction);
		REGISTER_OBJ_CREATOR( GAMEOBJ_WORSHIP_MAIL_TRANS, 		     GameObjectPool,  WorshipMailTransAction);
		REGISTER_OBJ_CREATOR( GAMEOBJ_GET_PLAYER_INFO_TRANS, 		 GameObjectPool,  GetPlayerInfoTransAction);
    }

private:
    OBJ_CREATOR(GetPlayerAction);
    OBJ_CREATOR(CreatePlayerAction);
    OBJ_CREATOR(GetTeamAction);
    OBJ_CREATOR(CreateTeamAction);
    OBJ_CREATOR(CompositeAction);
    OBJ_CREATOR(GetDataTransAction);
    OBJ_CREATOR(AddDataTransAction);
    OBJ_CREATOR(RefreshOpponentTransAction);
    OBJ_CREATOR(GetTopListTransAction);
    OBJ_CREATOR(FightStartTransAction);
    OBJ_CREATOR(FightSettleTransAction);
	OBJ_CREATOR(WorshipTransAction);
	OBJ_CREATOR(WorshipMailTransAction);
	OBJ_CREATOR(GetPlayerInfoTransAction);
};


#define RELEASE_GAMEOBJECT(p) do{ if(p) { GameObjectPool::Instance().Release((IObject*)p); p=NULL; } }while(0)
#define GET_GAMEOBJECT(ClassType, EnumType) dynamic_cast<ClassType*>(GameObjectPool::Instance().Get(EnumType))
#define FIND_GAMEOBJECT(ClassType, ObjID)	dynamic_cast<ClassType*>(GameObjectPool::Instance().Find(ObjID))



