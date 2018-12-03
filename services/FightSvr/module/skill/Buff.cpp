#include "LogMacros.h"
#include "string.h"
#include <sstream>
#include "../../framework/GameObjectPool.h"
#include "../fightobj/FightObj.h"
#include "../luainterface/LuaBinding.h"
#include "Buff.h"

Buff::Buff()
{
	this->_Construct();
}

Buff::~Buff()
{

}

void Buff::Clear()
{
	this->_Construct();
	SkillBase::Clear();
}

void Buff::_Construct()
{
	m_dwBuffId = 0;
	m_poResBuff = NULL;

	m_iLevel = 0;

	m_bIsDead = false;

	m_poSource = NULL;

	m_listOwner.clear();

	m_fLifeTimeConf = 0;
	m_iLifeTimeCurr = 0;

	m_fEffectCDConf = 0;
	m_iEffectCDCurr = 0;
	m_fEffectCDRatio = 0;
}

Buff* Buff::Get()
{
	return GET_GAMEOBJECT(Buff, GAMEOBJ_BUFF);
}

void Buff::Release(Buff* pObj)
{
	RELEASE_GAMEOBJECT(pObj);
}

bool Buff::Init(FightObj* poOwner, const std::string& rStrLuaTable, const std::string& rStrLuaScript)
{
	return SkillBase::Init(poOwner, rStrLuaTable, rStrLuaScript);
}

bool Buff::Init(uint32_t dwBuffId, FightObj* poOwner, FightObj* poSource, std::list<FightObj*>* poOwnerList)
{
	m_dwBuffId = dwBuffId;
	m_poResBuff = CGameDataMgr::Instance().GetResBuffMgr().Find(dwBuffId);
	if (m_poResBuff == NULL)
	{
		LOGERR("ResBuff not found.");
		return false;
	}

	m_iLevel = 0;
	m_iType = m_poResBuff->m_bType;

	m_fLifeTimeConf = m_poResBuff->m_fLifeTime;
	m_fEffectCDConf = m_poResBuff->m_fEffectCD;
	m_fEffectCDRatio = m_poResBuff->m_fEffectCDRatio;

	// 脚本初始化
	std::stringstream ss;
	ss << "Buff_" << m_poResBuff->m_dwScriptId;
	std::string strLuaTable = ss.str();

	ss.clear();
	ss.str("");
	ss << CWorkDir::string() << "/gamedata/Scripts/Buff/Buff_" << m_poResBuff->m_dwScriptId << ".txt";
	std::string strLuaScript = ss.str();
	this->Init(poOwner, strLuaTable, strLuaScript);

	_RunBorn(poSource, poOwnerList);

	return true;
}

void Buff::Refresh(FightObj* poSource, std::list<FightObj*>* poOwnerList)
{
	// 执行脚本runDead，撤销上次状态
	_RunDead();

	// 执行脚本runBorn，并刷新
	_RunBorn(poSource, poOwnerList);
}

void Buff::DoDead()
{
	_RunDead();

	// 等级重置
	m_iLevel = 0;
}

void Buff::Update(int dt)
{
	SkillBase::Update(dt);

	m_iLifeTimeCurr -= dt;
	m_iEffectCDCurr -= dt;

	if (m_poOwner != NULL)
	{
		if (m_iLifeTimeCurr < 0)
		{
			// 驱散
			return;
		}

		if (m_iEffectCDCurr < 0)
		{
			// 生效
			m_iEffectCDCurr = m_poResBuff->m_fEffectCD*1000;
		}
	}
}

void Buff::AddOwner(FightObj* poOwner)
{
	m_listOwner.push_back(poOwner);
}

void Buff::DelOwner(FightObj* poOwner)
{
	m_listOwner.remove(poOwner);
}

void Buff::_RunBorn(FightObj* poSource, std::list<FightObj*>* poOwnerList)
{
	m_bIsDead = false;

	m_poSource = poSource;

	m_listOwner.clear();
	if (poSource != NULL)
	{
		m_listOwner.push_back(m_poOwner);
	}
	else
	{
		m_listOwner.assign(poOwnerList->begin(), poOwnerList->end());
	}

	// 等级增长，需要在脚本runBorn执行之前
	m_iLevel++;
	if (m_iLevel > m_poResBuff->m_bLevelMax)
	{
		m_iLevel = m_poResBuff->m_bLevelMax;
	}

	// 调用诞生脚本
	LuaRunBorn();

	// 生命周期，生效周期初始化
	m_iLifeTimeCurr = (int)(m_poResBuff->m_fLifeTime*1000);
	m_iEffectCDCurr = (int)(m_fEffectCDConf * m_fEffectCDRatio*1000);
}

void Buff::_RunDead()
{
	std::list<FightObj*>::iterator iter = m_listOwner.begin();
	for (; iter != m_listOwner.end(); iter++)
	{
		if (*iter != m_poOwner)
		{
			// 从其他人关联链表中删除该关联者
			Buff* poBuff = (*iter)->m_oBuffManager.GetBuff(m_dwBuffId);
			if (poBuff != NULL) poBuff->DelOwner(m_poOwner);
		}
	}

	m_listOwner.clear();

	// 调用驱散脚本
	LuaRunDead();

	// 过滤器删除
	m_poOwner->m_poDungeon->m_oFilterManager.DelFilter(this);

	// 公式删除
	m_poOwner->m_oFormulaManager.DelFormula(this);
}

