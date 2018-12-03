#pragma once

#include "ObjectPool.h"
#include "singleton.h"
#include "../transaction/GuildTransaction.h"
#include "GuildFightArena.h"
#include "GuildFightPoint.h"
#include "FightPlayer.h"

enum GAMEOBJECT_TYPE
{
    GAMEOBJ_GET_PLAYER_ACTION = 1,
    GAMEOBJ_GET_GUILD_ACTION = 2,
    GAMEOBJ_CREATE_GUILD_ACTION = 3,
    GAMEOBJ_GUILD_TRANSACTION = 4,
    GAMEOBJ_GUILD_FIGHT_ARENA = 5,
    GAMEOBJ_GUILD_FIGHT_POINT = 6,
    GAMEOBJ_GUILD_FIGHT_PLAYER = 7,
    GAMEOBJ_GUILD_GM_UPDATE_INFO = 8,
    GAMEOBJ_GET_GUILD_BY_NAME_ACTION = 9,
};

class GameObjectPool : public ObjectPool<GameObjectPool>, public TSingleton<GameObjectPool>
{
public:
    GameObjectPool()
    {
        REGISTER_OBJ_CREATOR( GAMEOBJ_GET_PLAYER_ACTION,  	        GameObjectPool,  GetPlayerAction );
        REGISTER_OBJ_CREATOR( GAMEOBJ_GET_GUILD_ACTION, 	        GameObjectPool,  GetGuildAction );
        REGISTER_OBJ_CREATOR( GAMEOBJ_CREATE_GUILD_ACTION, 	        GameObjectPool,  CreateGuildAction );
        REGISTER_OBJ_CREATOR( GAMEOBJ_GUILD_TRANSACTION, 	        GameObjectPool,  GuildTransaction );
        REGISTER_OBJ_CREATOR( GAMEOBJ_GUILD_FIGHT_ARENA, 	        GameObjectPool,  GuildFightArena);
        REGISTER_OBJ_CREATOR( GAMEOBJ_GUILD_FIGHT_POINT, 	        GameObjectPool,  GuildFightPoint );
        REGISTER_OBJ_CREATOR( GAMEOBJ_GUILD_FIGHT_PLAYER, 	        GameObjectPool,  FightPlayer );
        REGISTER_OBJ_CREATOR( GAMEOBJ_GUILD_GM_UPDATE_INFO, 	    GameObjectPool,  GuildGmUpdateInfoAction );
        REGISTER_OBJ_CREATOR( GAMEOBJ_GET_GUILD_BY_NAME_ACTION,     GameObjectPool,  GetGuildByNameAction );
    }

    GetPlayerAction* GETOBJ_GetPlayerAction()
    {
        GetPlayerAction* poAction = dynamic_cast<GetPlayerAction*>(this->Get(GAMEOBJ_GET_PLAYER_ACTION));
        assert(poAction!=NULL);
        poAction->Reset();
        return poAction;
    }

    GetGuildAction* GETOBJ_GetGuildAction()
    {
        GetGuildAction* poAction = dynamic_cast<GetGuildAction*>(this->Get(GAMEOBJ_GET_GUILD_ACTION));
        assert(poAction!=NULL);
        poAction->Reset();
        return poAction;
    }

    CreateGuildAction* GETOBJ_CreateGuildAction()
    {
        CreateGuildAction* poAction = dynamic_cast<CreateGuildAction*>(this->Get(GAMEOBJ_CREATE_GUILD_ACTION));
        assert(poAction!=NULL);
        poAction->Reset();
        return poAction;
    }

    GuildTransaction* GETOBJ_GetGuildTransaction()
    {
        GuildTransaction* poAction = dynamic_cast<GuildTransaction*>(this->Get(GAMEOBJ_GUILD_TRANSACTION));
        assert(poAction!=NULL);
        poAction->Reset();
        return poAction;
    }

    GuildGmUpdateInfoAction* GETOBJ_GuildGmUpdateInfoAction()
    {
        GuildGmUpdateInfoAction* poAction = dynamic_cast<GuildGmUpdateInfoAction*>(this->Get(GAMEOBJ_GUILD_GM_UPDATE_INFO));
        assert(poAction!=NULL);
        poAction->Reset();
        return poAction;
    }

	GetGuildByNameAction* GETOBJ_GetGuildByNameAction()
	{
		GetGuildByNameAction* poAction = dynamic_cast<GetGuildByNameAction*>(this->Get(GAMEOBJ_GET_GUILD_BY_NAME_ACTION));
		assert(poAction!=NULL);
		poAction->Reset();
		return poAction;
	}

private:
    OBJ_CREATOR( GetPlayerAction);
    OBJ_CREATOR( GetGuildAction);
    OBJ_CREATOR( CreateGuildAction);
    OBJ_CREATOR( GuildTransaction);
    OBJ_CREATOR( GuildFightArena);
    OBJ_CREATOR( GuildFightPoint);
    OBJ_CREATOR( FightPlayer);
    OBJ_CREATOR( GuildGmUpdateInfoAction);
    OBJ_CREATOR( GetGuildByNameAction);
};


#define RELEASE_GAMEOBJECT( p ) do{ if(p) { GameObjectPool::Instance().Release((IObject*)p); p=NULL; } }while(0)
#define GET_GAMEOBJECT(ClassType, EnumType) dynamic_cast<ClassType*>(GameObjectPool::Instance().Get(EnumType) )
#define FIND_GAMEOBJECT(ClassType, ObjID)	dynamic_cast<ClassType*>(GameObjectPool::Instance().Find(ObjID) )



