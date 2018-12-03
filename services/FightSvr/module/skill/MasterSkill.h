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
class FightPlayer;

class MasterSkill : public SkillBase
{
public:
	MasterSkill();
	virtual ~MasterSkill();
	virtual void Clear();
private:
	void _Construct();

public:
	static MasterSkill* Get();
	static void Release(MasterSkill* pObj);

public:
	virtual bool Init(FightObj* poOwner, const std::string& rStrLuaTable, const std::string& rStrLuaScript);

	bool Init(uint32_t dwSkillId, uint8_t bSkillLevel, FightPlayer* poFightPlayer);
    float GetAddValue();
public:
	uint32_t m_dwSkillId;		// 军师技ID
	uint8_t m_bSkillLevel;		// 军师技等级
	float m_fPowerProgress;		// 军师技充能进度

	float m_fMSAddRatioTime;	// 军师技时间增益
	float m_fMSAddRatioPower;	// 军师技能量增益

private:
	FightPlayer* m_poFightPlayer;
	RESMASTERSKILL* m_poResMasterSkill;

public:
	bool m_bEnable;
	float m_fSkillCD;
};

