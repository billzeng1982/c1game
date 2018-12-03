#pragma once

#include "ObjectPool.h"
#include "singleton.h"
#include "../dungeon/Dungeon.h"
#include "../dungeon/TimeSyncTimer.h"
#include "../player/Player.h"
#include "../module/fightobj/FightPlayer.h"
#include "../module/fightobj/Troop.h"
#include "../module/fightobj/City.h"
#include "../module/fightobj/Tower.h"
#include "../module/fightobj/Barrier.h"
#include "../module/fightobj/Catapult.h"
#include "../module/skill/GeneralSkill.h"
#include "../module/skill/PassiveSkill.h"
#include "../module/skill/MasterSkill.h"
#include "../module/skill/Buff.h"



enum GAMEOBJECT_TYPE 
{
    GAMEOBJ_PLAYER = 1,
    GAMEOBJ_DUNGEON,
    GAMEOBJ_TSYNCTIMER,

	GAMEOBJ_FIGHTPLAYER = 11,
	GAMEOBJ_TROOP,
	GAMEOBJ_CITY,
	GAMEOBJ_TOWER,
	GAMEOBJ_BARRIER,
	GAMEOBJ_CATAPULT,

	GAMEOBJ_GENERALSKILL = 21,
	GAMEOBJ_PASSIVESKILL,
	GAMEOBJ_MASTERSKILL,
	GAMEOBJ_BUFF,
    GAMEOBJ_TACTICS,
};

class GameObjectPool : public ObjectPool<GameObjectPool>, public TSingleton<GameObjectPool>
{
public:
    GameObjectPool()
    {
        REGISTER_OBJ_CREATOR( GAMEOBJ_PLAYER, GameObjectPool, Player);
        REGISTER_OBJ_CREATOR( GAMEOBJ_DUNGEON, GameObjectPool, Dungeon);
		REGISTER_OBJ_CREATOR( GAMEOBJ_TSYNCTIMER, GameObjectPool, TimeSyncTimer);

		REGISTER_OBJ_CREATOR( GAMEOBJ_FIGHTPLAYER, GameObjectPool, FightPlayer);
		REGISTER_OBJ_CREATOR( GAMEOBJ_TROOP, GameObjectPool, Troop);
		REGISTER_OBJ_CREATOR( GAMEOBJ_CITY, GameObjectPool, City);
		REGISTER_OBJ_CREATOR( GAMEOBJ_TOWER, GameObjectPool, Tower);
		REGISTER_OBJ_CREATOR( GAMEOBJ_BARRIER, GameObjectPool, Barrier);
		REGISTER_OBJ_CREATOR( GAMEOBJ_CATAPULT, GameObjectPool, Catapult);

		REGISTER_OBJ_CREATOR( GAMEOBJ_GENERALSKILL, GameObjectPool, GeneralSkill);
		REGISTER_OBJ_CREATOR( GAMEOBJ_PASSIVESKILL, GameObjectPool, PassiveSkill);
		REGISTER_OBJ_CREATOR( GAMEOBJ_MASTERSKILL, GameObjectPool, MasterSkill);
		REGISTER_OBJ_CREATOR( GAMEOBJ_BUFF, GameObjectPool, Buff);
        REGISTER_OBJ_CREATOR( GAMEOBJ_TACTICS, GameObjectPool, Tactics);

    }

private:
	OBJ_CREATOR(Player);
    OBJ_CREATOR(Dungeon);
	OBJ_CREATOR(TimeSyncTimer);

	OBJ_CREATOR(FightPlayer);
	OBJ_CREATOR(Troop);
	OBJ_CREATOR(City);
	OBJ_CREATOR(Tower);
	OBJ_CREATOR(Barrier);
	OBJ_CREATOR(Catapult);

	OBJ_CREATOR(GeneralSkill);
	OBJ_CREATOR(PassiveSkill);
	OBJ_CREATOR(MasterSkill);
	OBJ_CREATOR(Buff);
    OBJ_CREATOR(Tactics);
};

#define RELEASE_GAMEOBJECT(p) do{ if(p) { GameObjectPool::Instance().Release(p); p=NULL; } }while(0)
#define GET_GAMEOBJECT(ClassType, EnumType) dynamic_cast<ClassType*>(GameObjectPool::Instance().Get(EnumType) )
#define FIND_GAMEOBJECT(ClassType, ObjID)   dynamic_cast<ClassType*>(GameObjectPool::Instance().Find(ObjID) )

