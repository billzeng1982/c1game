#include "LogMacros.h"
#include "../../framework/GameObjectPool.h"
#include "../../gamedata/GameDataMgr.h"
#include "Barrier.h"

Barrier::Barrier()
{
	this->_Construct();
}

Barrier::~Barrier()
{
}

void Barrier::Clear()
{
	this->_Construct();
	FightObj::Clear();
}

void Barrier::_Construct()
{
}

Barrier* Barrier::Get()
{
	return GET_GAMEOBJECT(Barrier, GAMEOBJ_BARRIER);
}

void Barrier::Release(Barrier* pObj)
{
	RELEASE_GAMEOBJECT(pObj);
}

bool Barrier::Init(Dungeon* poDungeon, FightPlayer* poFightPlayer, uint8_t bId, uint8_t bHp)
{
	FightObj::Init(poDungeon, poFightPlayer, FIGHTOBJ_BARRIER, bId);

	// 初始化血量

	int iHp = 6;
#if 0
	RESBASIC* poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_HP_BARRIER);
	if (poResBasic != NULL)
	{
		iHp = (int)poResBasic->m_para[0];
	}
#else
	iHp = bHp;
#endif

	if (iHp < 1)
	{
		iHp = 1;
	}

	m_oOriginRtData.m_arrAttrValue[ATTR_HP] = iHp;
	m_oCurrentRtData.m_arrAttrValue[ATTR_HP] = iHp;

	m_iHpCur = m_oCurrentRtData.m_arrAttrValue[ATTR_HP];

	return true;
}

void Barrier::ChgHp(int iValueChgType, int iValueChgPara, FightObj* poSource, int iDamageRef, int& iHpChgBefore, int& iHpChgAfter, int& iDamageFxSrc, int& iDamageFxTar)
{
	FightObj::ChgHp(iValueChgType, iValueChgPara, poSource, iDamageRef, iHpChgBefore, iHpChgAfter, iDamageFxSrc, iDamageFxTar);
}

void Barrier::AfterChgHp(int iValueChgType, int iValueChgPara, FightObj* poSource, int iHpChgBefore, int iHpChgAfter)
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
