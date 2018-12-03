#pragma once
#include "define.h"
#include "object.h"
#include "string.h"
#include "FilterManager.h"
#include "FormulaManager.h"

class FightObj;

class IShareValue
{
public:
	virtual ~IShareValue() {};
	virtual float GetShareValue(int iIdx) = 0;
	virtual void SetShareValue(int iIdx, float fValue) = 0;
};

class SkillBase : public IObject, public IFilter, public IFormula, public IShareValue
{
public:
	FightObj* m_poOwner;	// 所有者
	bool m_bEnable;			// 是否使能

	std::string m_strLuaTable;
	std::string m_strLuaScript;
	
public:
	static const int SHARE_COUNT = 5;
	float m_shareValue[SHARE_COUNT];

public:
	SkillBase();
	virtual ~SkillBase();
	virtual void Clear();
private:
	void _Construct();

public:
	virtual bool Init(FightObj* poOwner, const std::string& rStrLuaTable, const std::string& rStrLuaScript);
	virtual void Update(int dt);

public:
	void LuaRunBorn();
	void LuaRunDead();
	bool LuaCheckCond(int iAIId = 0);
	void LuaTakeEffect();

	void SetFilter(int iFilterType, int iGroup, int iPriority);
	virtual void DoFilter(int iFilterType, FightObj* poFilterOwner, FilterValue* poValue);

	void SetFormula(int iFormulaType, int iFormulaPara, int iPriority);
	virtual int DoFormula(int iFormulaType, int iFormulaPara, FightObj* poSource, FightObj* poTarget, int iDamageRef);

	virtual float GetShareValue(int iIdx);
	virtual void SetShareValue(int iIdx, float fValue);
};
