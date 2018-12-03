#include "LogMacros.h"
#include "../../framework/GameObjectPool.h"
#include "../../gamedata/GameDataMgr.h"
#include "Catapult.h"

Catapult::Catapult()
{
	this->_Construct();
}

Catapult::~Catapult()
{
}

void Catapult::Clear()
{
	this->_Construct();
	FightObj::Clear();
}

void Catapult::_Construct()
{
    m_poTroopOccupy = NULL;
}

Catapult* Catapult::Get()
{
	return GET_GAMEOBJECT(Catapult, GAMEOBJ_CATAPULT);
}

void Catapult::Release(Catapult* pObj)
{
	RELEASE_GAMEOBJECT(pObj);
}

bool Catapult::Init(Dungeon* poDungeon, uint8_t bId)
{
	FightObj::Init(poDungeon, NULL, FIGHTOBJ_CATAPULT, bId);

	return true;
}
