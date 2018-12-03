#pragma once
#include "define.h"
#include "object.h"
#include "../../gamedata/GameDataMgr.h"
#include "FilterManager.h"
#include "FormulaManager.h"
#include "SkillBase.h"

class FightObj;

class Buff : public SkillBase
{
public:
	uint32_t m_dwBuffId;
	RESBUFF* m_poResBuff;

	int m_iLevel;			// Buff等级
	int m_iType;			// Buff类型
	
	bool m_bIsDead;
	
	FightObj* m_poSource;	// Buff来源
	std::list<FightObj*> m_listOwner;	// Buff关联所有者
	
public:
	float m_fLifeTimeConf;
	int   m_iLifeTimeCurr; // ms
	
	float m_fEffectCDConf;
	int   m_iEffectCDCurr; // ms

	float m_fEffectCDRatio;

public:
	Buff();
	virtual ~Buff();
	virtual void Clear();
private:
	void _Construct();

public:
	static Buff* Get();
	static void Release(Buff* pObj);

public:
	virtual bool Init(FightObj* poOwner, const std::string& rStrLuaTable, const std::string& rStrLuaScript);

	bool Init(uint32_t dwBuffId, FightObj* poOwner, FightObj* poSource, std::list<FightObj*>* poOwnerList);

	void Refresh(FightObj* poSource, std::list<FightObj*>* poOwnerList);

	virtual void Update(int dt);

	void DoDead();

	void AddOwner(FightObj* poOwner);

	void DelOwner(FightObj* poOwner);

private:
	void _RunBorn(FightObj* poSource, std::list<FightObj*>* poOwnerList);
	void _RunDead();
};
