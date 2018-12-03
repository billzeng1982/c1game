#pragma once
#include "define.h"
#include "object.h"
#include "../../gamedata/GameDataMgr.h"
#include "FilterManager.h"
#include "FormulaManager.h"
#include "SkillBase.h"

class General;
class FightObj;
class Troop;

class PassiveSkill : public SkillBase
{
public:
	PassiveSkill();
	virtual ~PassiveSkill();
	virtual void Clear();
private:
	void _Construct();

public:
	static PassiveSkill* Get();
	static void Release(PassiveSkill* pObj);

public:
	virtual bool Init(FightObj* poOwner, const std::string& rStrLuaTable, const std::string& rStrLuaScript);

	bool Init(uint32_t dwSkillId, FightObj* poOwner);
	virtual void Update(int dt);

private:
	uint32_t m_dwSkillId;

public:
	float m_fSkillCD;
	RESPASSIVESKILL* m_poResPassiveSkill;
};
