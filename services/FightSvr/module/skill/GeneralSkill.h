#pragma once
#include "define.h"
#include "object.h"
#include "../../gamedata/GameDataMgr.h"
#include "FilterManager.h"
#include "FormulaManager.h"
#include "SkillBase.h"

class FightObj;
class Troop;

class GeneralSkill : public SkillBase
{
public:
	GeneralSkill();
	virtual ~GeneralSkill();
	virtual void Clear();
private:
	void _Construct();

public:
	static GeneralSkill* Get();
	static void Release(GeneralSkill* pObj);

public:
	virtual bool Init(FightObj* poOwner, const std::string& rStrLuaTable, const std::string& rStrLuaScript);

	bool Init(uint32_t dwSkillId, Troop* poTroop);

private:
	uint32_t m_dwSkillId;
	Troop* m_poTroop;

public:
	float m_fSkillCD;

	float m_fMoraleNeedConf;   // 所需士气配置值
	float m_fMoraleNeedCurr;   // 所需士气当前值

	RESGENERALSKILL* m_poResGeneralSkill;
	RESCHEATS* m_poResCheats[MAX_SKILL_CHEATS_NUM];
};

