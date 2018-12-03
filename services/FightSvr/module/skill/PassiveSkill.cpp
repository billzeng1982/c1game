#include "LogMacros.h"
#include "string.h"
#include <sstream>
#include "../../framework/GameObjectPool.h"
#include "../fightobj/FightObj.h"
#include "../luainterface/LuaBinding.h"
#include "PassiveSkill.h"

PassiveSkill::PassiveSkill()
{
	this->_Construct();
}

PassiveSkill::~PassiveSkill()
{

}

void PassiveSkill::Clear()
{
	this->_Construct();
	SkillBase::Clear();
}

void PassiveSkill::_Construct()
{
	m_dwSkillId = 0;
	m_poResPassiveSkill = NULL;

	m_fSkillCD = 0;
}

PassiveSkill* PassiveSkill::Get()
{
	return GET_GAMEOBJECT(PassiveSkill, GAMEOBJ_PASSIVESKILL);
}

void PassiveSkill::Release(PassiveSkill* pObj)
{
	RELEASE_GAMEOBJECT(pObj);
}

bool PassiveSkill::Init(FightObj* poOwner, const std::string& rStrLuaTable, const std::string& rStrLuaScript)
{
	return SkillBase::Init(poOwner, rStrLuaTable, rStrLuaScript);
}

bool PassiveSkill::Init(uint32_t dwSkillId, FightObj* poOwner)
{
	m_dwSkillId = dwSkillId;

	m_poResPassiveSkill = CGameDataMgr::Instance().GetResPassiveSkillMgr().Find(dwSkillId);
	if (m_poResPassiveSkill == NULL)
	{
		LOGERR("ResPassiveSkill not found.");
		return false;
	}

	m_fSkillCD = 0;

	// 脚本初始化
	std::stringstream ss;
	ss << "PassiveSkill_" << m_poResPassiveSkill->m_dwScriptId;
	std::string strLuaTable = ss.str();

	ss.clear();
	ss.str("");
	ss << CWorkDir::string() << "/gamedata/Scripts/PassiveSkill/PassiveSkill_" << m_poResPassiveSkill->m_dwScriptId << ".txt";
	std::string strLuaScript = ss.str();
	this->Init(poOwner, strLuaTable, strLuaScript);

	// 调用脚本runBorn函数
	LuaRunBorn();
	return true;
}

void PassiveSkill::Update(int dt)
{
	SkillBase::Update(dt);

	if (!m_bEnable)
	{
		return;
	}

	if (LuaCheckCond())
	{
		LuaTakeEffect();
	}
}
