#include "LogMacros.h"
#include "string.h"
#include <sstream>
#include "../../framework/GameObjectPool.h"
#include "../fightobj/FightObj.h"
#include "../luainterface/LuaBinding.h"
#include "GeneralSkill.h"

GeneralSkill::GeneralSkill()
{
	this->_Construct();
}

GeneralSkill::~GeneralSkill()
{

}

void GeneralSkill::Clear()
{
	this->_Construct();
	SkillBase::Clear();
}

void GeneralSkill::_Construct()
{
	m_dwSkillId = 0;
	m_poTroop = NULL;
	m_poResGeneralSkill = NULL;

	m_fSkillCD = 0;
}

GeneralSkill* GeneralSkill::Get()
{
	return GET_GAMEOBJECT(GeneralSkill, GAMEOBJ_GENERALSKILL);
}

void GeneralSkill::Release(GeneralSkill* pObj)
{
	RELEASE_GAMEOBJECT(pObj);
}

bool GeneralSkill::Init(FightObj* poOwner, const std::string& rStrLuaTable, const std::string& rStrLuaScript)
{
	return SkillBase::Init(poOwner, rStrLuaTable, rStrLuaScript);
}

bool GeneralSkill::Init(uint32_t dwSkillId, Troop* poTroop)
{
	m_dwSkillId = dwSkillId;
	m_poTroop = poTroop;

	m_poResGeneralSkill = CGameDataMgr::Instance().GetResGeneralSkillMgr().Find(dwSkillId);
	if (m_poResGeneralSkill == NULL)
	{
		LOGERR("ResGeneralSkill not found.");
		return false;
	}

	// 技能秘籍1
	if(m_poResGeneralSkill->m_wCheats1 > 0)
	{
		m_poResCheats[0] = CGameDataMgr::Instance().GetResCheatsMgr().Find(m_poResGeneralSkill->m_wCheats1);
		if(m_poResCheats[0] == NULL)
		{
			LOGERR("ResCheats 1 found.m_bActiveSkillCheatsType is %d",poTroop->m_bActiveSkillCheatsType);
			return false;
		}
	}

	// 技能秘籍2
    if(m_poResGeneralSkill->m_wCheats2 > 0)
	{
		m_poResCheats[1] = CGameDataMgr::Instance().GetResCheatsMgr().Find(m_poResGeneralSkill->m_wCheats2);
		if(m_poResCheats[1] == NULL)
		{
			LOGERR("ResCheats 2 found. m_bActiveSkillCheatsType is %d",poTroop->m_bActiveSkillCheatsType);
			return false;
		}
	}

	m_fSkillCD = 0;
	m_fMoraleNeedConf = m_poResGeneralSkill->m_wMoraleValue;
	m_fMoraleNeedCurr = m_poResGeneralSkill->m_wMoraleValue;

	// 脚本初始化
	std::stringstream ss;
	ss << "GeneralSkill_" << m_poResGeneralSkill->m_dwScriptId;
	std::string strLuaTable = ss.str();

	ss.clear();
	ss.str("");
	ss << CWorkDir::string() << "/gamedata/Scripts/GeneralSkill/GeneralSkill_" << m_poResGeneralSkill->m_dwScriptId << ".txt";
	std::string strLuaScript = ss.str();
	this->Init(poTroop, strLuaTable, strLuaScript);

	// 调用脚本runBorn函数
	LuaRunBorn();

	return true;
}
