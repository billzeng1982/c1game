#include "LogMacros.h"
#include "../../framework/GameObjectPool.h"
#include "Tower.h"
#include "CpuSampleStats.h"

Tower::Tower()
{
	this->_Construct();
}

Tower::~Tower()
{
}

void Tower::Clear()
{
	this->_Construct();
	FightObj::Clear();
}

void Tower::_Construct()
{

}

Tower* Tower::Get()
{
	return GET_GAMEOBJECT(Tower, GAMEOBJ_TOWER);
}

void Tower::Release(Tower* pObj)
{
	RELEASE_GAMEOBJECT(pObj);
}

bool Tower::Init(Dungeon* poDungeon, FightPlayer* poFightPlayer, uint8_t bId)
{
	FightObj::Init(poDungeon, poFightPlayer, FIGHTOBJ_TOWER, bId);

	// 初始化血量
	int iHp = 6;
	RESBASIC* poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_HP_TOWER);
	if (poResBasic != NULL)
	{
		iHp = (int)poResBasic->m_para[0];
	}

	m_oOriginRtData.m_arrAttrValue[ATTR_HP] = iHp;
	m_oCurrentRtData.m_arrAttrValue[ATTR_HP] = iHp;

	m_iHpCur = m_oCurrentRtData.m_arrAttrValue[ATTR_HP];
	return true;
}

void Tower::ChgHp(int iValueChgType, int iValueChgPara, FightObj* poSource, int iDamageRef, int& iHpChgBefore, int& iHpChgAfter, int& iDamageFxSrc, int& iDamageFxTar)
{
	FightObj::ChgHp(iValueChgType, iValueChgPara, poSource, iDamageRef, iHpChgBefore, iHpChgAfter, iDamageFxSrc, iDamageFxTar);
}

void Tower::AfterChgHp(int iValueChgType, int iValueChgPara, FightObj* poSource, int iHpChgBefore, int iHpChgAfter)
{
	FightObj::AfterChgHp(iValueChgType, iValueChgPara, poSource, iHpChgBefore, iHpChgAfter);

	if (iHpChgBefore > 0 && iHpChgAfter <= 0)
	{
		// 每日任务统计
		if (poSource->m_poFightPlayer != NULL)
		{
			poSource->m_poFightPlayer->m_stTaskInfo.m_dwDestoryObstacle++;
		}
	}
}
