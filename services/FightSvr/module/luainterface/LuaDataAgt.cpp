#include <cstring>
#include "LogMacros.h"
#include "LuaDataAgt.h"
#include "../dungeon/Dungeon.h"
#include "../fightobj/FightObj.h"
#include "../fightobj/Troop.h"
#include "../fightobj/FightPlayer.h"
#include "../skill/GeneralSkill.h"
#include "../skill/PassiveSkill.h"
#include "../skill/MasterSkill.h"
#include "../skill/Buff.h"
#include "../../dungeon/Dungeon.h"
#include "../../framework/GameObjectPool.h"
#include "GameDataMgr.h"

bool LuaDataAgt::Init(FightObj* pOwner)
{
	m_poOwner = pOwner;
	return true;
}

bool LuaDataAgt::Equal(LuaDataAgt* poOther)
{
	return (this == poOther);
}

void LuaDataAgt::SetFilter(int iFilterType, int iOwnerGroup, int iPriority)
{
	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return;
	}

	if (m_poSkillBase == NULL)
	{
		LOGERR("m_poSkillBase is NULL");
		return;
	}

	if (iOwnerGroup == TARGET_GROUP::SELF)
	{
		iOwnerGroup = m_poOwner->m_chGroup;
	}
	else if (iOwnerGroup == TARGET_GROUP::ENEMY)
	{
		iOwnerGroup = m_poOwner->m_chGroup == PKGMETA::PLAYER_GROUP_DOWN ? PKGMETA::PLAYER_GROUP_UP : PKGMETA::PLAYER_GROUP_DOWN;
	}
	else
	{
		iOwnerGroup = PKGMETA::PLAYER_GROUP_NONE;
	}

	m_poSkillBase->SetFilter(iFilterType, iOwnerGroup, iPriority);
}

void LuaDataAgt::SetFormula(int iFormulaType, int iFormulaPara, int iPriority)
{
	if (m_poSkillBase == NULL)
	{
		LOGERR("m_poSkillBase is NULL");
		return;
	}

	m_poSkillBase->SetFormula(iFormulaType, iFormulaPara, iPriority);
}

float LuaDataAgt::GetShareValue(int iIdx)
{
	float fRet = 0;
	if (m_poSkillBase == NULL)
	{
		LOGERR("m_poSkillBase is NULL");
		return fRet;
	}

	return m_poSkillBase->GetShareValue(iIdx);
}

void LuaDataAgt::SetShareValue(int iIdx, float fValue)
{
	if (m_poSkillBase == NULL)
	{
		LOGERR("m_poSkillBase is NULL");
		return;
	}

	m_poSkillBase->SetShareValue(iIdx, fValue);
}

int LuaDataAgt::GetGoGroup()
{
	int iRet = 0;
	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return iRet;
	}

	iRet = m_poOwner->m_chGroup;

	return iRet;
}

int LuaDataAgt::GetGoType()
{
	int iRet = 0;
	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return iRet;
	}

	iRet = m_poOwner->m_chType;

	return iRet;
}

int LuaDataAgt::GetGoId()
{
	int iRet = 0;
	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return iRet;
	}

	iRet = m_poOwner->m_bId;

	return iRet;
}

int LuaDataAgt::GetArmyType()
{
	int iRet = 0;

	Troop* poTroop = (Troop* )m_poOwner;
	if(poTroop == NULL || poTroop->GetObjType() != GAMEOBJ_TROOP)
	{
		LOGERR("poTroop is NULL");
		return iRet;
	}

	iRet = poTroop->m_poResArmy->m_bType;

	return iRet;
}

int LuaDataAgt::GetArmyPhase()
{
	int iRet = 0;

	Troop* poTroop = (Troop* )m_poOwner;
	if(poTroop == NULL || poTroop->GetObjType() != GAMEOBJ_TROOP)
	{
		LOGERR("poTroop is NULL");
		return iRet;
	}
	return poTroop->m_iArmyPhase;
}

float LuaDataAgt::GetArmyBaseValueBase()
{
	float fRet = 0;

	Troop* poTroop = (Troop* )m_poOwner;
	if(poTroop == NULL || poTroop->GetObjType() != GAMEOBJ_TROOP)
	{
		LOGERR("poTroop is NULL");
		return fRet;
	}

	fRet = poTroop->GetArmyBaseValueBase();

	return fRet;
}

float LuaDataAgt::GetArmyBaseValueGrow()
{
	float fRet = 0;

	Troop* poTroop = (Troop* )m_poOwner;
	if(poTroop == NULL || poTroop->GetObjType() != GAMEOBJ_TROOP)
	{
		LOGERR("poTroop is NULL");
		return fRet;
	}

	fRet = poTroop->GetArmyBaseValueGrow();

	return fRet;
}

float LuaDataAgt::GetArmyBaseValue()
{
	float fRet = 0;

	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return fRet;
	}

	fRet = m_poOwner->m_oCurrentRtData.GetAttribute(ATTR_BASE_ARMYSKILL);

	return fRet;
}

float LuaDataAgt::GetArmyRestrain(LuaDataAgt* poSource, LuaDataAgt* poTarget)
{
	float fRet = 1;

	Troop* poTroopSource = (Troop* )poSource->m_poOwner;
	if(poTroopSource == NULL || poTroopSource->GetObjType() != GAMEOBJ_TROOP)
	{
		LOGERR("poTroopSource is NULL");
		return fRet;
	}

	Troop* poTroopTarget = (Troop* )poSource->m_poOwner;
	if(poTroopTarget == NULL || poTroopTarget->GetObjType() != GAMEOBJ_TROOP)
	{
		LOGERR("poTroopTarget is NULL");
		return fRet;
	}

	RESARMYRESTRAIN* poResArmyRestrain = CGameDataMgr::Instance().GetResArmyRestrainMgr().Find(poTroopSource->m_poResArmy->m_bType * 1000 + poTroopTarget->m_poResArmy->m_bType);
	if (poResArmyRestrain == NULL)
	{
		LOGERR("poResArmyRestrain is NULL");
		return fRet;
	}

	fRet = poResArmyRestrain->m_fArmyRestrainRatio;

	return fRet;
}

int LuaDataAgt::GetHp(int iValueType)
{
	int iRet = 0;

	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return iRet;
	}

	if (iValueType == VALUE_TYPE::CURRENT)
	{
		// ��ǰֵ
		iRet = m_poOwner->m_iHpCur;
	}
	else if (iValueType == VALUE_TYPE::ORIGIN_CONF)
	{
		// ��������ֵ
		iRet = m_poOwner->m_oOriginRtData.m_arrAttrValue[ATTR_HP];
	}
	else if (iValueType == VALUE_TYPE::RUNTIME_CONF)
	{
		// ����ʱ����ֵ
		iRet = m_poOwner->m_oCurrentRtData.m_arrAttrValue[ATTR_HP];
	}

	return iRet;
}

void LuaDataAgt::ChgHp(int iValueType, float fValue, LuaDataAgt* poSource, int iValueChgType, int iValueChgPara)
{
	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return;
	}

	if (iValueType == VALUE_TYPE::CURRENT)
	{
		// ��ǰֵ
		//int iValue = (int)fValue;
		//m_poOwner->ChgHp(iValueChgType, iValueChgPara, poSource->m_poOwner, iValue, iValue, iValue);
	}
	else if (iValueType == VALUE_TYPE::ORIGIN_CONF)
	{
		// ��������ֵ
		m_poOwner->m_oOriginRtData.m_arrAttrValue[ATTR_HP] += (int)fValue;
	}
	else if (iValueType == VALUE_TYPE::RUNTIME_CONF)
	{
		// ����ʱ����ֵ
		m_poOwner->m_oCurrentRtData.m_arrAttrValue[ATTR_HP] += (int)fValue;
	}
}

void LuaDataAgt::ChgTowerHp(int iTargetType, int iValueType, float fValue)
{
	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return;
	}

	ListFightObj_t& rListTower = m_poOwner->m_poDungeon->m_listTower;
	ListFightObj_t::iterator iter = rListTower.begin();
	for (; iter != rListTower.end(); iter++)
	{
		FightObj* poTower = *iter;
		if (iTargetType == TARGET_GROUP::SELF || iTargetType == TARGET_GROUP::NONE)
		{
			if (poTower->m_chGroup == m_poOwner->m_chGroup)
			{
				if (iValueType == VALUE_TYPE::RUNTIME_CONF)
				{
					poTower->m_oCurrentRtData.m_arrAttrValue[ATTR_HP] += (int)fValue;
					poTower->m_iHpCur += fValue;
				}
			}
		}
		else
		{
			if (poTower->m_chGroup != m_poOwner->m_chGroup)
			{
				if (iValueType == VALUE_TYPE::RUNTIME_CONF)
				{
					poTower->m_oCurrentRtData.m_arrAttrValue[ATTR_HP] += (int)fValue;
					poTower->m_iHpCur += fValue;
				}
			}
		}
	}
}

void LuaDataAgt::ChgBarrierHp(int iTargetType, int iValueType, float fValue)
{
	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return;
	}

	ListFightObj_t& rListBarrier = m_poOwner->m_poDungeon->m_listBarrier;
	ListFightObj_t::iterator iter = rListBarrier.begin();
	for (; iter != rListBarrier.end(); iter++)
	{
		FightObj* poBarrier = *iter;
		if (iTargetType == TARGET_GROUP::SELF || iTargetType == TARGET_GROUP::NONE)
		{
			if (poBarrier->m_chGroup == m_poOwner->m_chGroup)
			{
				if (iValueType == VALUE_TYPE::RUNTIME_CONF)
				{
					poBarrier->m_oCurrentRtData.m_arrAttrValue[ATTR_HP] += (int)fValue;
					poBarrier->m_iHpCur += fValue;
				}
			}
		}
		else
		{
			if (poBarrier->m_chGroup != m_poOwner->m_chGroup)
			{
				if (iValueType == VALUE_TYPE::RUNTIME_CONF)
				{
					poBarrier->m_oCurrentRtData.m_arrAttrValue[ATTR_HP] += (int)fValue;
					poBarrier->m_iHpCur += fValue;
				}
			}
		}
	}
}

float LuaDataAgt::GetMoveSpeed(int iValueType)
{
	float fRet = 0;

	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return fRet;
	}
	if (iValueType == VALUE_TYPE::CURRENT)
	{
		// ��ǰֵ
		fRet = m_poOwner->GetSpeed();
	}
	else if (iValueType == VALUE_TYPE::ORIGIN_CONF)
	{
		// ��������ֵ
		fRet = m_poOwner->m_oOriginRtData.m_arrAttrValue[ATTR_SPEED];
	}
	else if (iValueType == VALUE_TYPE::RUNTIME_CONF)
	{
		// ����ʱ����ֵ
		fRet = m_poOwner->m_oCurrentRtData.m_arrAttrValue[ATTR_SPEED];
	}

	return fRet;
}

void LuaDataAgt::ChgSpeedByRatio(float fValue)
{

}

int LuaDataAgt::GetStrAtk(int iValueType)
{
	int iRet = 0;

	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return iRet;
	}

	if (iValueType == VALUE_TYPE::CURRENT)
	{
		// ��ǰֵ == ����ʱ����ֵ
		iRet = m_poOwner->m_oCurrentRtData.m_arrAttrValue[ATTR_STR];
	}
	else if (iValueType == VALUE_TYPE::ORIGIN_CONF)
	{
		// ��������ֵ
		iRet = m_poOwner->m_oOriginRtData.m_arrAttrValue[ATTR_STR];
	}
	else if (iValueType == VALUE_TYPE::RUNTIME_CONF)
	{
		// ����ʱ����ֵ
		iRet = m_poOwner->m_oCurrentRtData.m_arrAttrValue[ATTR_STR];
	}
	return iRet;
}

void LuaDataAgt::ChgStrAtk(int iValueType, float fValue)
{
	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return;
	}

	if (iValueType == VALUE_TYPE::CURRENT)
	{
		// ��ǰֵ
		m_poOwner->m_oCurrentRtData.m_arrAttrValue[ATTR_STR] += (int)fValue;
	}
	else if (iValueType == VALUE_TYPE::ORIGIN_CONF)
	{
		// ��������ֵ
		m_poOwner->m_oOriginRtData.m_arrAttrValue[ATTR_STR] += (int)fValue;
	}
	else if (iValueType == VALUE_TYPE::RUNTIME_CONF)
	{
		// ����ʱ����ֵ
		m_poOwner->m_oCurrentRtData.m_arrAttrValue[ATTR_STR] += (int)fValue;
	}
}

int LuaDataAgt::GetStrDef(int iValueType)
{
	int iRet = 0;

	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return iRet;
	}

	if (iValueType == VALUE_TYPE::CURRENT)
	{
		// ��ǰֵ == ����ʱ����ֵ
		iRet = m_poOwner->m_oCurrentRtData.m_arrAttrValue[ATTR_STRDEF];
	}
	else if (iValueType == VALUE_TYPE::ORIGIN_CONF)
	{
		// ��������ֵ
		iRet = m_poOwner->m_oOriginRtData.m_arrAttrValue[ATTR_STRDEF];
	}
	else if (iValueType == VALUE_TYPE::RUNTIME_CONF)
	{
		// ����ʱ����ֵ
		iRet = m_poOwner->m_oCurrentRtData.m_arrAttrValue[ATTR_STRDEF];
	}
	return iRet;
}

void LuaDataAgt::ChgStrDef(int iValueType, float fValue)
{
	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return;
	}

	if (iValueType == VALUE_TYPE::CURRENT)
	{
		// ��ǰֵ
		m_poOwner->m_oCurrentRtData.m_arrAttrValue[ATTR_STRDEF] += (int)fValue;
	}
	else if (iValueType == VALUE_TYPE::ORIGIN_CONF)
	{
		// ��������ֵ
		m_poOwner->m_oOriginRtData.m_arrAttrValue[ATTR_STRDEF] += (int)fValue;
	}
	else if (iValueType == VALUE_TYPE::RUNTIME_CONF)
	{
		// ����ʱ����ֵ
		m_poOwner->m_oCurrentRtData.m_arrAttrValue[ATTR_STRDEF] += (int)fValue;
	}
}

int LuaDataAgt::GetWitAtk(int iValueType)
{
	int iRet = 0;

	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return iRet;
	}

	if (iValueType == VALUE_TYPE::CURRENT)
	{
		// ��ǰֵ == ����ʱ����ֵ
		iRet = m_poOwner->m_oCurrentRtData.m_arrAttrValue[ATTR_WIT];
	}
	else if (iValueType == VALUE_TYPE::ORIGIN_CONF)
	{
		// ��������ֵ
		iRet = m_poOwner->m_oOriginRtData.m_arrAttrValue[ATTR_WIT];
	}
	else if (iValueType == VALUE_TYPE::RUNTIME_CONF)
	{
		// ����ʱ����ֵ
		iRet = m_poOwner->m_oCurrentRtData.m_arrAttrValue[ATTR_WIT];
	}
	return iRet;
}

void LuaDataAgt::ChgWitAtk(int iValueType, float fValue)
{
	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return;
	}

	if (iValueType == VALUE_TYPE::CURRENT)
	{
		// ��ǰֵ
		m_poOwner->m_oCurrentRtData.m_arrAttrValue[ATTR_WIT] += (int)fValue;
	}
	else if (iValueType == VALUE_TYPE::ORIGIN_CONF)
	{
		// ��������ֵ
		m_poOwner->m_oOriginRtData.m_arrAttrValue[ATTR_WIT] += (int)fValue;
	}
	else if (iValueType == VALUE_TYPE::RUNTIME_CONF)
	{
		// ����ʱ����ֵ
		m_poOwner->m_oCurrentRtData.m_arrAttrValue[ATTR_WIT] += (int)fValue;
	}
}

int LuaDataAgt::GetWitDef(int iValueType)
{
	int iRet = 0;

	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return iRet;
	}

	if (iValueType == VALUE_TYPE::CURRENT)
	{
		// ��ǰֵ == ����ʱ����ֵ
		iRet = m_poOwner->m_oCurrentRtData.m_arrAttrValue[ATTR_WITDEF];
	}
	else if (iValueType == VALUE_TYPE::ORIGIN_CONF)
	{
		// ��������ֵ
		iRet = m_poOwner->m_oOriginRtData.m_arrAttrValue[ATTR_WITDEF];
	}
	else if (iValueType == VALUE_TYPE::RUNTIME_CONF)
	{
		// ����ʱ����ֵ
		iRet = m_poOwner->m_oCurrentRtData.m_arrAttrValue[ATTR_WITDEF];
	}
	return iRet;
}

void LuaDataAgt::ChgWitDef(int iValueType, float fValue)
{
	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return;
	}

	if (iValueType == VALUE_TYPE::CURRENT)
	{
		// ��ǰֵ
		m_poOwner->m_oCurrentRtData.m_arrAttrValue[ATTR_WITDEF] += (int)fValue;
	}
	else if (iValueType == VALUE_TYPE::ORIGIN_CONF)
	{
		// ��������ֵ
		m_poOwner->m_oOriginRtData.m_arrAttrValue[ATTR_WITDEF] += (int)fValue;
	}
	else if (iValueType == VALUE_TYPE::RUNTIME_CONF)
	{
		// ����ʱ����ֵ
		m_poOwner->m_oCurrentRtData.m_arrAttrValue[ATTR_WITDEF] += (int)fValue;
	}
}

float LuaDataAgt::GetAttribute(int iValueType, int iAttrType)
{
	float fRet = 0;

	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return fRet;
	}

	if (iValueType == VALUE_TYPE::CURRENT)
	{
		// ��ǰֵ == ����ʱ����ֵ
		fRet = m_poOwner->m_oCurrentRtData.GetAttribute(iAttrType);
	}
	else if (iValueType == VALUE_TYPE::ORIGIN_CONF)
	{
		// ��������ֵ
		fRet = m_poOwner->m_oOriginRtData.GetAttribute(iAttrType);
	}
	else if (iValueType == VALUE_TYPE::RUNTIME_CONF)
	{
		// ����ʱ����ֵ
		fRet = m_poOwner->m_oCurrentRtData.GetAttribute(iAttrType);
	}
	return fRet;
}

void LuaDataAgt::ChgAttribute(int iValueType, int iAttrType, float fValue)
{
	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return;
	}

	if (iValueType == VALUE_TYPE::CURRENT)
	{
		// ��ǰֵ
		m_poOwner->m_oCurrentRtData.ChgAttribute(iAttrType, fValue);
	}
	else if (iValueType == VALUE_TYPE::ORIGIN_CONF)
	{
		// ��������ֵ
		m_poOwner->m_oOriginRtData.ChgAttribute(iAttrType, fValue);
	}
	else if (iValueType == VALUE_TYPE::RUNTIME_CONF)
	{
		// ����ʱ����ֵ
		m_poOwner->m_oCurrentRtData.ChgAttribute(iAttrType, fValue);
	}
}

float LuaDataAgt::GetAttributeLimit(int iValueType, int iAttrType)
{
	float fRet = 0;

	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return fRet;
	}

	if (iValueType == VALUE_TYPE::CURRENT)
	{
		// ��ǰֵ == ����ʱ����ֵ
		fRet = m_poOwner->m_oCurrentRtData.GetAttributeLimit(iAttrType);
	}
	else if (iValueType == VALUE_TYPE::ORIGIN_CONF)
	{
		// ��������ֵ
		fRet = m_poOwner->m_oOriginRtData.GetAttributeLimit(iAttrType);
	}
	else if (iValueType == VALUE_TYPE::RUNTIME_CONF)
	{
		// ����ʱ����ֵ
		fRet = m_poOwner->m_oCurrentRtData.GetAttributeLimit(iAttrType);
	}
	return fRet;
}

void LuaDataAgt::ChgAttributeLimit(int iValueType, int iAttrType, float fValue)
{
	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return;
	}

	if (iValueType == VALUE_TYPE::CURRENT)
	{
		// ��ǰֵ
		m_poOwner->m_oCurrentRtData.ChgAttributeLimit(iAttrType, fValue);
	}
	else if (iValueType == VALUE_TYPE::ORIGIN_CONF)
	{
		// ��������ֵ
		m_poOwner->m_oOriginRtData.ChgAttributeLimit(iAttrType, fValue);
	}
	else if (iValueType == VALUE_TYPE::RUNTIME_CONF)
	{
		// ����ʱ����ֵ
		m_poOwner->m_oCurrentRtData.ChgAttributeLimit(iAttrType, fValue);
	}
}

float LuaDataAgt::GetSiegeBack()
{
	float fRet = 0;

	// m_stPlayerInfo
	FightPlayer* poPlayer = (FightPlayer*)m_poOwner->m_poDungeon->GetFightPlayerByGroup(m_poOwner->m_chGroup);
	if (poPlayer == NULL)
	{
		LOGERR("poPlayer is NULL");
		return fRet;
	}

	ResMajestyLvMgr_t& roResMgr = CGameDataMgr::Instance().GetResMajestyLvMgr();
	RESMAJESTYLV* pstResLv = roResMgr.Find((int)poPlayer->m_stPlayerInfo.m_bMajestyLevel);
	if (!pstResLv)
	{
		LOGERR("pstResLv is NULL");
		return fRet;
	}

	// ��ǽ����
	fRet = pstResLv->m_dwWallBackGrow;

	return fRet;
}

float LuaDataAgt::GetSiegeBackRatio()
{
	float fRet = 0;
	Troop* poTroop = (Troop* )m_poOwner;
	if(poTroop == NULL || poTroop->GetObjType() != GAMEOBJ_TROOP)
	{
		LOGERR("poTroop is NULL");
		return fRet;
	}

	if (poTroop->m_poResArmy == NULL)
	{
		LOGERR("m_poResArmy is NULL");
		return fRet;
	}

	fRet = poTroop->m_poResArmy->m_dwCityCountDamagePermillage;

	return fRet;
}

void LuaDataAgt::SetSkillEnable(bool bEnable)
{
	if (m_poSkillBase == NULL)
	{
		LOGERR("m_poSkillBase is NULL");
		return;
	}

	m_poSkillBase->m_bEnable = bEnable;
}

float LuaDataAgt::GetBuffLifeTime(int iValueType)
{
	float fRet = 0;
	Buff* poBuff = (Buff*)m_poSkillBase;
	if (poBuff == NULL || poBuff->GetObjType() != GAMEOBJ_BUFF)
	{
		LOGERR("poBuff is NULL");
		return fRet;
	}

	if (iValueType == VALUE_TYPE::CURRENT)
	{
		// ��ǰֵ
		fRet = (float)(poBuff->m_iLifeTimeCurr)/1000.0f;
	}
	else
	{
		fRet = poBuff->m_fLifeTimeConf;
	}
	return fRet;
}

void LuaDataAgt::SetBuffLifeTime(int iValueType, float fValue)
{
	Buff* poBuff = (Buff*)m_poSkillBase;
	if (poBuff == NULL || poBuff->GetObjType() != GAMEOBJ_BUFF)
	{
		LOGERR("poBuff is NULL");
		return;
	}

	if (iValueType == VALUE_TYPE::CURRENT)
	{
		// ��ǰֵ
		poBuff->m_iLifeTimeCurr = (int)(fValue*1000);
	}
	else
	{
		poBuff->m_fLifeTimeConf = fValue;
	}
}

float LuaDataAgt::GetBuffEffectCD(int iValueType)
{
	float fRet = 0;
	Buff* poBuff = (Buff*)m_poSkillBase;
	if (poBuff == NULL || poBuff->GetObjType() != GAMEOBJ_BUFF)
	{
		LOGERR("poBuff is NULL");
		return fRet;
	}

	if (iValueType == VALUE_TYPE::CURRENT)
	{
		// ��ǰֵ
		fRet = (float)poBuff->m_iEffectCDCurr/1000.0f;
	}
	else
	{
		fRet = poBuff->m_fEffectCDConf;
	}

	return fRet;
}

void LuaDataAgt::SetBuffEffectCD(int iValueType, float fValue)
{
	Buff* poBuff = (Buff*)m_poSkillBase;
	if (poBuff == NULL || poBuff->GetObjType() != GAMEOBJ_BUFF)
	{
		LOGERR("poBuff is NULL");
		return;
	}
	if (iValueType == VALUE_TYPE::CURRENT)
	{
		// ��ǰֵ
		poBuff->m_iEffectCDCurr = (int)(fValue*1000);
	}
	else
	{
		poBuff->m_fEffectCDConf = fValue;
	}
}

int LuaDataAgt::GetBuffLevel()
{
	int iRet = 0;
	Buff* poBuff = (Buff*)m_poSkillBase;
	if (poBuff == NULL || poBuff->GetObjType() != GAMEOBJ_BUFF)
	{
		LOGERR("poBuff is NULL");
		return iRet;
	}

	iRet = poBuff->m_iLevel;

	return iRet;
}

bool LuaDataAgt::HasBuff(uint32_t dwBuffId)
{
	bool bRet = false;
	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return bRet;
	}

	return m_poOwner->m_oBuffManager.HasBuff(dwBuffId);
}

bool LuaDataAgt::HasBuffInType(int iType)
{
	bool bRet = false;
	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return bRet;
	}

	return m_poOwner->m_oBuffManager.HasBuffInType(iType);
}

LuaDataAgt* LuaDataAgt::GetBuffSource()
{
	Buff* poBuff = (Buff*)m_poSkillBase;
	if (poBuff == NULL || poBuff->GetObjType() != GAMEOBJ_BUFF)
	{
		LOGERR("poBuff is NULL");
		return NULL;
	}

	return &poBuff->m_poSource->m_oLuaDataAgt;
}

int LuaDataAgt::GetBuffOwnerListCnt()
{
	int iRet = 0;
	Buff* poBuff = (Buff*)m_poSkillBase;
	if (poBuff == NULL || poBuff->GetObjType() != GAMEOBJ_BUFF)
	{
		LOGERR("poBuff is NULL");
		return iRet;
	}

	iRet = poBuff->m_listOwner.size();;

	return iRet;
}




bool LuaDataAgt::IsAllTroopDead(int iGroup, LuaDataAgt* poExcept)
{
	bool bRet = true;
	if (m_poOwner == NULL || poExcept == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return bRet;
	}

	bRet = m_poOwner->m_poDungeon->IsAllTroopDead(iGroup, poExcept->m_poOwner);

	return bRet;
}

bool LuaDataAgt::IsAttackCity()
{
	bool bRet = false;
	if (m_poOwner == NULL || m_poOwner->m_chType == FIGHTOBJ_TROOP)
	{
		LOGERR("m_poOwner is NULL");
		return bRet;
	}

	return ((Troop*)m_poOwner)->m_bIsAtkCity;
}

int LuaDataAgt::GetMSLevel4Skill()
{
	int iRet = 0;

	return iRet;
}

float LuaDataAgt::GetMSProgress4Skill()
{
	float fRet = 0;

	return fRet;
}

int LuaDataAgt::GetMSLevel4Buff()
{
	int iRet = 0;
	Buff* poBuff = (Buff*)m_poSkillBase;
	if (poBuff == NULL || poBuff->GetObjType() != GAMEOBJ_BUFF)
	{
		LOGERR("poBuff is NULL");
		return iRet;
	}

	FightPlayer* poPlayer = (FightPlayer*)poBuff->m_poSource;
	iRet = poPlayer->m_poMasterSkill->m_bSkillLevel;

	return iRet;
}

float LuaDataAgt::GetMSProgress4Buff()
{
	float fRet = 0;
	Buff* poBuff = (Buff*)m_poSkillBase;
	if (poBuff == NULL || poBuff->GetObjType() != GAMEOBJ_BUFF)
	{
		LOGERR("poBuff is NULL");
		return fRet;
	}

	FightPlayer* poPlayer = (FightPlayer*)poBuff->m_poSource;
	fRet = poPlayer->m_poMasterSkill->m_fPowerProgress;

	return fRet;
}

float LuaDataAgt::GetMSAddRatioTime4Buff()
{
	float fRet = 0;
	Buff* poBuff = (Buff*)m_poSkillBase;
	if (poBuff == NULL || poBuff->GetObjType() != GAMEOBJ_BUFF)
	{
		LOGERR("poBuff is NULL");
		return fRet;
	}

	FightPlayer* poPlayer = (FightPlayer*)poBuff->m_poSource;
	fRet = poPlayer->m_poMasterSkill->m_fMSAddRatioTime;

	return fRet;
}

float LuaDataAgt::GetMSAddValue4Buff()
{
	float fRet = 0.0f;
	Buff* poBuff = (Buff*)m_poSkillBase;
	if (poBuff == NULL)
	{
		LOGERR("poBuff is null");
		return fRet;
	}
	FightPlayer* poPlayer = poBuff->m_poSource->m_poDungeon->GetFightPlayerByGroup(poBuff->m_poSource->m_chGroup);
	;
	fRet = poPlayer->m_poMasterSkill->GetAddValue();

	return fRet;
}

float LuaDataAgt::GetMSAddValue4Skill()
{
	float fRet = 0.0f;
	Buff* poBuff = (Buff*)m_poSkillBase;;
	if (poBuff == NULL)
	{
		LOGERR("poBuff is null");
		return fRet;
	}
	fRet = poBuff->m_poOwner->m_poFightPlayer->m_poMasterSkill->GetAddValue();
	return fRet;
}

void LuaDataAgt::ChgMSAddRatioTime4Buff(float fValue)
{
	Buff* poBuff = (Buff*)m_poSkillBase;
	if (poBuff == NULL || poBuff->GetObjType() != GAMEOBJ_BUFF)
	{
		LOGERR("poBuff is NULL");
		return;
	}

	FightPlayer* poPlayer = poBuff->m_poSource->m_poDungeon->GetFightPlayerByGroup(poBuff->m_poSource->m_chGroup);
	if (poPlayer == NULL)
	{
		LOGERR("poPlayer is NULL");
		return;
	}

	if (fValue > 0)
	{
		poPlayer->m_poMasterSkill->m_fMSAddRatioTime *= fValue;
	}
	else if (fValue < 0)
	{
		poPlayer->m_poMasterSkill->m_fMSAddRatioTime /= (-fValue);
	}
}

float LuaDataAgt::GetMSAddRatioTime()
{
	float fRet = 0;
	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return fRet;
	}

	FightPlayer* poPlayer = (FightPlayer*)m_poOwner->m_poDungeon->GetFightPlayerByGroup(m_poOwner->m_chGroup);
	if (poPlayer == NULL)
	{
		LOGERR("poPlayer is NULL");
		return fRet;
	}

	fRet = poPlayer->m_poMasterSkill->m_fMSAddRatioTime;

	return fRet;
}

void LuaDataAgt::ChgMSAddRatioTime(float fValue)
{
	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return;
	}

	FightPlayer* poPlayer = (FightPlayer*)m_poOwner->m_poDungeon->GetFightPlayerByGroup(m_poOwner->m_chGroup);
	if (poPlayer == NULL)
	{
		LOGERR("poPlayer is NULL");
		return;
	}

	if (fValue > 0)
	{
		poPlayer->m_poMasterSkill->m_fMSAddRatioTime *= fValue;
	}
	else if (fValue < 0)
	{
		poPlayer->m_poMasterSkill->m_fMSAddRatioTime /= (-fValue);
	}
}

float LuaDataAgt::GetMSAddRatioPower4Buff()
{
	float fRet = 0;
	Buff* poBuff = (Buff*)m_poSkillBase;
	if (poBuff == NULL || poBuff->GetObjType() != GAMEOBJ_BUFF)
	{
		LOGERR("poBuff is NULL");
		return fRet;
	}

	FightPlayer* poPlayer = (FightPlayer*)poBuff->m_poSource;
	fRet = poPlayer->m_poMasterSkill->m_fMSAddRatioPower;

	return fRet;
}

void LuaDataAgt::ChgMSAddRatioPower4Buff(float fValue)
{
	Buff* poBuff = (Buff*)m_poSkillBase;
	if (poBuff == NULL || poBuff->GetObjType() != GAMEOBJ_BUFF)
	{
		LOGERR("poBuff is NULL");
		return;
	}

	FightPlayer* poPlayer = poBuff->m_poSource->m_poDungeon->GetFightPlayerByGroup(poBuff->m_poSource->m_chGroup);
	if (poPlayer == NULL)
	{
		LOGERR("poPlayer is NULL");
		return;
	}

	if (fValue > 0)
	{
		poPlayer->m_poMasterSkill->m_fMSAddRatioPower *= fValue;
	}
	else if (fValue < 0)
	{
		poPlayer->m_poMasterSkill->m_fMSAddRatioPower /= (-fValue);
	}
}

float LuaDataAgt::GetMSAddRatioPower()
{
	float fRet = 0;
	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return fRet;
	}

	FightPlayer* poPlayer = (FightPlayer*)m_poOwner->m_poDungeon->GetFightPlayerByGroup(m_poOwner->m_chGroup);
	if (poPlayer == NULL)
	{
		LOGERR("poPlayer is NULL");
		return fRet;
	}

	fRet = poPlayer->m_poMasterSkill->m_fMSAddRatioPower;

	return fRet;
}

void LuaDataAgt::ChgMSAddRatioPower(float fValue)
{
	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return;
	}

	FightPlayer* poPlayer = (FightPlayer*)m_poOwner->m_poDungeon->GetFightPlayerByGroup(m_poOwner->m_chGroup);
	if (poPlayer == NULL)
	{
		LOGERR("poPlayer is NULL");
		return;
	}

	if (fValue > 0)
	{
		poPlayer->m_poMasterSkill->m_fMSAddRatioPower *= fValue;
	}
	else if (fValue < 0)
	{
		poPlayer->m_poMasterSkill->m_fMSAddRatioPower /= (-fValue);
	}
}

float LuaDataAgt::GetActiveSkillMorale(int iValueType)
{
	float fRet = 0;

	Troop* poTroop = (Troop* )m_poOwner;
	if(poTroop == NULL || poTroop->GetObjType() != GAMEOBJ_TROOP)
	{
		LOGERR("poTroop is NULL");
		return fRet;
	}

	if (iValueType == VALUE_TYPE::CURRENT)
	{
		// ��ǰֵ
		fRet = poTroop->m_oGeneral.m_poActiveSkill->m_fMoraleNeedCurr;
	}
	else
	{
		// ����ʱ����ֵ
		fRet = poTroop->m_oGeneral.m_poActiveSkill->m_fMoraleNeedConf;
	}

	return fRet;
}

void LuaDataAgt::ChgActiveSkillMorale(int iValueType, float fValue)
{
	Troop* poTroop = (Troop* )m_poOwner;
	if(poTroop == NULL || poTroop->GetObjType() != GAMEOBJ_TROOP)
	{
		LOGERR("poTroop is NULL");
		return;
	}

	if (iValueType == VALUE_TYPE::CURRENT)
	{
		// ��ǰֵ
		poTroop->m_oGeneral.m_poActiveSkill->m_fMoraleNeedCurr += fValue;
	}
	else
	{
		// ����ʱ����ֵ
		poTroop->m_oGeneral.m_poActiveSkill->m_fMoraleNeedConf += fValue;
	}
}

int LuaDataAgt::GetActiveSkillLevel()
{
	int iRet = 1;

	Troop* poTroop = (Troop* )m_poOwner;
	if(poTroop == NULL || poTroop->GetObjType() != GAMEOBJ_TROOP)
	{
		LOGERR("poTroop is NULL");
		return iRet;
	}

	iRet = poTroop->m_bActiveSkillLevel;
	return iRet;
}

float LuaDataAgt::GetActiveSkillBaseValue(int iParID)
{
	float fRet = 0;
	Troop* poTroop = (Troop* )m_poOwner;
	if(poTroop == NULL || poTroop->GetObjType() != GAMEOBJ_TROOP)
	{
		LOGERR("poTroop is NULL");
		return fRet;
	}

	fRet = poTroop->GetActiveSkillBaseValue(iParID);

	return fRet;
}

float LuaDataAgt::GetActiveSkillRateValue(int iParID)
{
	float fRet = 0;
	Troop* poTroop = (Troop* )m_poOwner;
	if(poTroop == NULL || poTroop->GetObjType() != GAMEOBJ_TROOP)
	{
		LOGERR("poTroop is NULL");
		return fRet;
	}

	fRet = poTroop->GetActiveSkillRateValue(iParID);

	return fRet;
}

float LuaDataAgt::GetActiveSkillAdditionValue(int iParID)
{
	float fRet = 0;
	Troop* poTroop = (Troop* )m_poOwner;
	if(poTroop == NULL || poTroop->GetObjType() != GAMEOBJ_TROOP)
	{
		LOGERR("poTroop is NULL");
		return fRet;
	}
	RESGENERALSKILL* poResGeneralSkill = poTroop->m_oGeneral.m_poActiveSkill->m_poResGeneralSkill;
	if (poResGeneralSkill == NULL)
	{
		LOGERR("poResGeneralSkill is NULL");
		return fRet;
	}
	if(iParID > poResGeneralSkill->m_wParCount || iParID <=0)
	{
		LOGERR("ParID is out of range, ParId=(%d), ParCount=(%d)", iParID, poResGeneralSkill->m_wParCount);
		return fRet;
	}

	//�����˺�
	float fBase = poTroop->GetActiveSkillBaseValue(iParID);
	//�ɳ�����
	float fRatio = poTroop->GetActiveSkillRateValue(iParID);

	int iBaseStr = m_poOwner->m_oCurrentRtData.m_arrAttrValue[ATTR_STR];
	int iBaseWit = m_poOwner->m_oCurrentRtData.m_arrAttrValue[ATTR_WIT];

	fRet = fBase;
	if(poResGeneralSkill->m_parType[iParID -1] == 1)
	{
		fRet += iBaseStr * fRatio;
	}
	else if(poResGeneralSkill->m_parType[iParID -1] == 2)
	{
		fRet += iBaseWit * fRatio;
	}

	return fRet;
}


float LuaDataAgt::GetPassiveSkillBaseValue(int iSkillId, int iParID)
{
	float fRet = 0;
	Troop* poTroop = (Troop* )m_poOwner;
	if (poTroop == NULL || poTroop->GetObjType() != GAMEOBJ_TROOP)
	{
		LOGERR("poTroop is NULL");
		return fRet;
	}

	RESPASSIVESKILL* poResPassiveSkill = CGameDataMgr::Instance().GetResPassiveSkillMgr().Find(iSkillId);;
	if (poResPassiveSkill == NULL)
	{
		LOGERR("poResGeneralSkill is NULL");
		return fRet;
	}

	if (iParID > poResPassiveSkill->m_dwParNum || iParID <=0)
	{
		LOGERR("ParID is out of range, ParId=(%d), ParCount=(%d)", iParID, poResPassiveSkill->m_dwParNum);
		return fRet;
	}
    fRet = poResPassiveSkill->m_parBase[iParID - 1];

	std::map<uint32_t, uint8_t>::iterator Iter;
	Iter = (poTroop->m_oGeneral.m_dictPassiveSkillId).find(iSkillId);
	if (Iter == poTroop->m_oGeneral.m_dictPassiveSkillId.end())
	{
		return fRet;
	}
	uint8_t bLevel = Iter->second;

	RESCONSUME * poResBaseGrow = CGameDataMgr::Instance().GetResConsumeMgr().Find(poResPassiveSkill->m_parBaseGrowId[iParID -1]);
	if (poResBaseGrow == NULL)
	{
		LOGERR("poResBaseGrow is NULL, BaseGrowId (%d) not found", poResPassiveSkill->m_parBaseGrowId[iParID -1]);
		return fRet;
	}
	if ((uint32_t)bLevel > poResBaseGrow->m_dwLvCount || bLevel <= 0)
	{
		LOGERR("SkillLevel is out of range, Level=(%d), LevelMax=(%d)", bLevel, poResBaseGrow->m_dwLvCount);
		return fRet;
	}
	if (poResBaseGrow->m_dwDivideRate == 0)
	{
		LOGERR(" DivideRate is Zero");
		return fRet;
	}

	fRet *= (1.0 + poResBaseGrow->m_lvList[bLevel-1] /(float)poResBaseGrow->m_dwDivideRate * poResPassiveSkill->m_parBasePVPRatio[iParID -1] /1000);
	return fRet;
}

bool LuaDataAgt::IsHaveActiveSkillCheats(int iType)
{
	bool bRet = false;
	if(iType == 0)
	{
		return bRet;
	}

	Troop* poTroop = (Troop* )m_poOwner;
	if(poTroop == NULL || poTroop->GetObjType() != GAMEOBJ_TROOP)
	{
		LOGERR("poTroop is NULL");
		return bRet;
	}

	if (iType < 1)
	{
		LOGERR("Cheats Type < 1");
		return bRet;
	}

	if((poTroop->m_bActiveSkillCheatsType & (1 << (iType - 1))) != 0)
	{
		bRet = true;
	}

	return bRet;
}

float LuaDataAgt::GetActiveSkillCheatsValue(int iType)
{
	float fRet = 0;

	if(iType <= 0 || iType > MAX_SKILL_CHEATS_NUM)
	{
		return fRet;
	}

	if(!IsHaveActiveSkillCheats(iType))
	{
		return fRet;
	}

	Troop* poTroop = (Troop* )m_poOwner;
	if(poTroop == NULL || poTroop->GetObjType() != GAMEOBJ_TROOP)
	{
		LOGERR("poTroop is NULL");
		return fRet;
	}

	RESCHEATS* poResCheats = poTroop->m_oGeneral.m_poActiveSkill->m_poResCheats[iType -1];
	if(poResCheats == NULL)
	{
		LOGERR("poResCheats is NULL");
		return fRet;
	}

	int iCheatsLevel = poTroop->m_dwActiveCheatsLevel[iType -1];
	if(iCheatsLevel == 0)
	{
		return fRet;
	}

	fRet = poResCheats->m_cheatsStarPar[iCheatsLevel - 1];

	return fRet;
}

int LuaDataAgt::GetActiveSkillCheatsLevel()
{
	int iRet = 1;
	Troop* poTroop = (Troop* )m_poOwner;
	if(poTroop == NULL || poTroop->GetObjType() != GAMEOBJ_TROOP)
	{
		LOGERR("poTroop is NULL");
		return iRet;
	}

	iRet = poTroop->m_dwActiveCheatsLevel[0];
	return iRet;
}

bool LuaDataAgt::IsFateInTeam(int iFateId)
{
    RESGENERALFATE* poResGeneralFate = CGameDataMgr::Instance().GetResGeneralFateMgr().Find(iFateId);
    if (NULL == poResGeneralFate)
    {
        LOGERR("FateId<%d> not found", iFateId);
        return false;
    }
    //�������佫�Ƿ���������
    FightPlayer* poPlayer = ((Troop*)m_poOwner)->m_poFightPlayer;
    bool bAllHad = true;
    for (int i=0; i<RES_MAX_GCARD_FATE_RELEVANCE_NUM; i++)
    {
        if (0 == poResGeneralFate->m_generalIds[i])
        {
            continue;
        }
        if (!poPlayer->IsGeneralInTeam(poResGeneralFate->m_generalIds[i]))
        {
            bAllHad = false;
            break;
        }
    }
    return bAllHad;
}

int LuaDataAgt::GetCountryID()
{
	int iRet = 0;
	Troop* poTroop = (Troop* )m_poOwner;
	if(poTroop == NULL || poTroop->GetObjType() != GAMEOBJ_TROOP)
	{
		LOGERR("poTroop is NULL");
		return iRet;
	}

	RESGENERAL* rstResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(poTroop->m_dwGeneralId);
	if (NULL == rstResGeneral)
	{
		LOGERR("GeneralId<%d> not found", poTroop->m_dwGeneralId);
		return iRet;
	}

	iRet = rstResGeneral->m_bCountryId;

	return iRet;
}

int LuaDataAgt::GetGeneralSex()
{
	int iRet = 0;
	Troop* poTroop = (Troop* )m_poOwner;
	if(poTroop == NULL || poTroop->GetObjType() != GAMEOBJ_TROOP)
	{
		LOGERR("poTroop is NULL");
		return iRet;
	}

	RESGENERAL* rstResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(poTroop->m_dwGeneralId);
	if (NULL == rstResGeneral)
	{
		LOGERR("GeneralId<%d> not found", poTroop->m_dwGeneralId);
		return iRet;
	}

	iRet = rstResGeneral->m_bSex;

	return iRet;
}

int LuaDataAgt::GetSelfTroopCountOnBattle()
{
	int iRet = 0;
	if (m_poOwner == NULL)
	{
		LOGERR("m_poOwner is NULL");
		return iRet;
	}

	ListFightObj_t& rListTroop = m_poOwner->m_chGroup == PLAYER_GROUP_DOWN ? m_poOwner->m_poDungeon->m_listTroopDown : m_poOwner->m_poDungeon->m_listTroopUp;
	ListFightObj_t::iterator iter = rListTroop.begin();
	for (; iter != rListTroop.end(); iter++)
	{
		Troop* poTroop = (Troop*)(*iter);
		if (poTroop->m_iHpCur <= 0 || poTroop->m_bIsRetreat || poTroop->m_oBuffManager.HasBuffInType(BUFF_TYPE_REVIVE))
		{
			continue;
		}

		iRet++;
	}

	return iRet;
}

float LuaDataAgt::GetResTacticsBuffValue(uint32_t dwBuffId)
{
    int iRet = 0;
    Troop* poTroop = (Troop* )m_poOwner;
    if(poTroop == NULL || poTroop->GetObjType() != GAMEOBJ_TROOP)
    {
        LOGERR("poTroop is NULL");
        return iRet;
    }

    if(poTroop->m_poTactics == NULL)
    {
        LOGERR("BuffId<%u>, m_poTactics is NULL", dwBuffId);
        return iRet;
    }

    return poTroop->m_poTactics->GetAddValue(dwBuffId);
}


