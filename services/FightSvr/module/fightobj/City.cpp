#include "LogMacros.h"
#include "../../framework/GameObjectPool.h"
#include "City.h"
#include "CpuSampleStats.h"
City::City()
{
	this->_Construct();
}

City::~City()
{
}

void City::Clear()
{
	this->_Construct();
	FightObj::Clear();
}

void City::_Construct()
{
}

City* City::Get()
{
	return GET_GAMEOBJECT(City, GAMEOBJ_CITY);
}

void City::Release(City* pObj)
{
	RELEASE_GAMEOBJECT(pObj);
}

bool City::Init(Dungeon*poDungeon, FightPlayer* poFightPlayer)
{
	FightObj::Init(poDungeon, poFightPlayer, FIGHTOBJ_WALL, poFightPlayer->m_chGroup);

#if 0
	int iHp = 3400;
	RESBASIC* poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_HP_CITY);
	if (poResBasic != NULL)
	{
		iHp = (int)poResBasic->m_para[0];
	}
#else
    // 对战城墙血量修改为根据主公等级赋值
    int iHp = (int)poFightPlayer->m_stPlayerInfo.m_dwCityHp;    
#endif

	m_oOriginRtData.m_arrAttrValue[ATTR_HP] = iHp;
	m_oCurrentRtData.m_arrAttrValue[ATTR_HP] = iHp;

	m_iHpCur = m_oCurrentRtData.m_arrAttrValue[ATTR_HP];
	return true;
}

void City::ChgHp(int iValueChgType, int iValueChgPara, FightObj* poSource, int iDamageRef, int& iHpChgBefore, int& iHpChgAfter, int& iDamageFxSrc, int& iDamageFxTar)
{
	FightObj::ChgHp(iValueChgType, iValueChgPara, poSource, iDamageRef, iHpChgBefore, iHpChgAfter, iDamageFxSrc, iDamageFxTar);
}

void City::AfterChgHp(int iValueChgType, int iValueChgPara, FightObj* poSource, int iHpChgBefore, int iHpChgAfter)
{
	FightObj::AfterChgHp(iValueChgType, iValueChgPara, poSource, iHpChgBefore, iHpChgAfter);
}
