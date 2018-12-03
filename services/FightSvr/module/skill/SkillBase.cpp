#include "LogMacros.h"
#include "../fightobj/FightObj.h"
#include "../fightobj/Troop.h"
#include "../luainterface/LuaBinding.h"
#include "../fightobj/FightPlayer.h"
#include "../../player/Player.h"
#include "SkillBase.h"

SkillBase::SkillBase()
{
	this->_Construct();
}

SkillBase::~SkillBase()
{

}

void SkillBase::Clear()
{
	this->_Construct();
	IObject::Clear();
}

void SkillBase::_Construct()
{
	m_poOwner = NULL;
	m_bEnable = false;

	m_strLuaTable.clear();
	m_strLuaScript.clear();

	for (int i=0; i<SHARE_COUNT; i++)
	{
		m_shareValue[i] = 0;
	}
}

bool SkillBase::Init(FightObj* poOwner, const std::string& rStrLuaTable, const std::string& rStrLuaScript)
{
	m_poOwner = poOwner;

	m_bEnable = true;

	m_strLuaTable = rStrLuaTable;
	m_strLuaScript = rStrLuaScript;

	for (int i=0; i<SHARE_COUNT; i++)
	{
		m_shareValue[i] = 0;
	}

	return true;
}

void SkillBase::Update(int dt)
{
	IObject::Update(dt);

	if (!m_bEnable)
	{
		return;
	}
}

void SkillBase::LuaRunBorn()
{
	// 调用脚本RunBorn处理
	SkillBase* poSkillBase = m_poOwner->m_oLuaDataAgt.m_poSkillBase;
	m_poOwner->m_oLuaDataAgt.m_poSkillBase = this;
	LuaBinding::Instance().func_runBorn(m_strLuaTable.c_str(), m_strLuaScript.c_str(), &m_poOwner->m_oLuaDataAgt);
	m_poOwner->m_oLuaDataAgt.m_poSkillBase = poSkillBase;
}

void SkillBase::LuaRunDead()
{
	// 调用脚本RunDead处理
	SkillBase* poSkillBase = m_poOwner->m_oLuaDataAgt.m_poSkillBase;
	m_poOwner->m_oLuaDataAgt.m_poSkillBase = this;
	LuaBinding::Instance().func_runDead(m_strLuaTable.c_str(), m_strLuaScript.c_str(), &m_poOwner->m_oLuaDataAgt);
	m_poOwner->m_oLuaDataAgt.m_poSkillBase = poSkillBase;
}

bool SkillBase::LuaCheckCond(int iAIId /*= 0*/)
{
	// 调用脚本CheckCond处理
	bool bRet = false;
	SkillBase* poSkillBase = m_poOwner->m_oLuaDataAgt.m_poSkillBase;
	m_poOwner->m_oLuaDataAgt.m_poSkillBase = this;
	bRet = LuaBinding::Instance().func_checkCond(m_strLuaTable.c_str(), m_strLuaScript.c_str(), &m_poOwner->m_oLuaDataAgt, iAIId);
	m_poOwner->m_oLuaDataAgt.m_poSkillBase = poSkillBase;

	return bRet;
}


void SkillBase::LuaTakeEffect()
{
	// 调用脚本TakeEffect处理
	SkillBase* poSkillBase = m_poOwner->m_oLuaDataAgt.m_poSkillBase;
	m_poOwner->m_oLuaDataAgt.m_poSkillBase = this;
	LuaBinding::Instance().func_takeEffect(m_strLuaTable.c_str(), m_strLuaScript.c_str(), &m_poOwner->m_oLuaDataAgt);
	m_poOwner->m_oLuaDataAgt.m_poSkillBase = poSkillBase;
}

void SkillBase::SetFilter(int iFilterType, int iGroup, int iPriority)
{
	if (iGroup != PKGMETA::PLAYER_GROUP_NONE)
	{
		ListFightObj_t* poListTroop = m_poOwner->m_poDungeon->GetTroopListByGroup(iGroup);
		ListFightObj_t::iterator iter = poListTroop->begin();
		for (; iter != poListTroop->end(); iter++)
		{
			FightObj* poTroop = *iter;
			m_poOwner->m_poDungeon->m_oFilterManager.AddFilter(iFilterType, poTroop, iPriority, this);
		}
	}
	else
	{
		m_poOwner->m_poDungeon->m_oFilterManager.AddFilter(iFilterType, m_poOwner, iPriority, this);
	}
}

void SkillBase::DoFilter(int iFilterType, FightObj* poFilterOwner, FilterValue* poValue)
{
	// 调用脚本Filter处理
	SkillBase* poSkillBase = m_poOwner->m_oLuaDataAgt.m_poSkillBase;
	m_poOwner->m_oLuaDataAgt.m_poSkillBase = this;
	LuaBinding::Instance().func_doFilter(m_strLuaTable.c_str(), m_strLuaScript.c_str(), &m_poOwner->m_oLuaDataAgt, iFilterType, &poFilterOwner->m_oLuaDataAgt, poValue);
	m_poOwner->m_oLuaDataAgt.m_poSkillBase = poSkillBase;
}

void SkillBase::SetFormula(int iFormulaType, int iFormulaPara, int iPriority)
{
    if (m_poOwner->m_chType == FIGHTOBJ_TROOP)
    {
        Troop* poTroop = (Troop*)m_poOwner;
        LOGRUN("Dungeon<%u> Player<%s> TroopId<%d> GeneralId<%d> FormulaManager(%p) AddFormula FormulaType-%d FormulaPara-%d Priority-%d Formula-%p",
                m_poOwner->m_poDungeon->m_dwDungeonId, m_poOwner->m_poFightPlayer->m_szName, m_poOwner->m_bId, poTroop->m_dwGeneralId,
                &(m_poOwner->m_oFormulaManager), iFormulaType, iFormulaPara, iPriority, this);
    }
    else
    {
        LOGRUN("Dungeon<%u> FightObjType<%d> FightObjId<%d> FormulaManager(%p) AddFormula FormulaType-%d FormulaPara-%d Priority-%d Formula-%p",
                m_poOwner->m_poDungeon->m_dwDungeonId, m_poOwner->m_chType, m_poOwner->m_bId,
                &(m_poOwner->m_oFormulaManager), iFormulaType, iFormulaPara, iPriority, this);
    }

	m_poOwner->m_oFormulaManager.AddFormula(iFormulaType, iFormulaPara, iPriority, this);
}

int SkillBase::DoFormula(int iFormulaType, int iFormulaPara, FightObj* poSource, FightObj* poTarget, int iDamageRef)
{
	// 调用脚本Formula处理
	int iRet = 0;

	SkillBase* poSkillBase = m_poOwner->m_oLuaDataAgt.m_poSkillBase;
	m_poOwner->m_oLuaDataAgt.m_poSkillBase = this;
	iRet = LuaBinding::Instance().func_doFormula(m_strLuaTable.c_str(), m_strLuaScript.c_str(), &m_poOwner->m_oLuaDataAgt, iFormulaType, iFormulaPara,
		&poSource->m_oLuaDataAgt, &poTarget->m_oLuaDataAgt, iDamageRef);
	m_poOwner->m_oLuaDataAgt.m_poSkillBase = poSkillBase;

	return iRet;
}

float SkillBase::GetShareValue(int iIdx)
{
	if (iIdx < 0)
	{
		return m_shareValue[0];
	}
	else if (iIdx >= SHARE_COUNT)
	{
		return m_shareValue[iIdx - 1];
	}
	else
	{
		return m_shareValue[iIdx];
	}
}

void SkillBase::SetShareValue(int iIdx, float fValue)
{
	if (iIdx < 0)
	{
		m_shareValue[0] = fValue;
	}
	else if (iIdx >= SHARE_COUNT)
	{
		m_shareValue[iIdx - 1] = fValue;
	}
	else
	{
		m_shareValue[iIdx] = fValue;
	}
}
