#pragma once
#include "define.h"
class FightObj;
class SkillBase;

class TARGET_GROUP
{
public:
	enum T
	{
		NONE = 0,
		SELF,
		ENEMY,
	};
};

class VALUE_TYPE
{
public:
	enum T
	{
		CURRENT = 0,
		ORIGIN_CONF,
		RUNTIME_CONF,
	};
};

class LuaDataAgt
{
private:
	FightObj* m_poOwner;
	int m_iDamageFxType;

public:
	SkillBase* m_poSkillBase;

public:
	bool Init(FightObj* poOwner);
	bool Equal(LuaDataAgt* poOther);

public:
	void SetFilter(int iFilterType, int iOwnerGroup, int iPriority);
	void SetFormula(int iFormulaType, int iFormulaPara, int iPriority);

	float GetShareValue(int iIdx);
	void SetShareValue(int iIdx, float fValue);

public:
	// 游戏对象三基本属性
	int GetGoGroup();
	int GetGoType();
	int GetGoId();

	// 兵种属性
	int GetArmyType();
	int GetArmyPhase();
	float GetArmyRestrain(LuaDataAgt* poSource, LuaDataAgt* poTarget);

	// 血
	int GetHp(int iValueType);
	void ChgHp(int iValueType, float fValue, LuaDataAgt* poSource, int iValueChgType, int iValueChgPara);

	void ChgTowerHp(int iTargetType, int iValueType, float fValue);
	void ChgBarrierHp(int iTargetType, int iValueType, float fValue);

	// 移动速度
	float GetMoveSpeed(int iValueType);
	void ChgSpeedByRatio(float fValue);

	// 物理攻击
	int GetStrAtk(int iValueType);
	void ChgStrAtk(int iValueType, float fValue);

	// 物理防御
	int GetStrDef(int iValueType);
	void ChgStrDef(int iValueType, float fValue);

	// 法术攻击
	int GetWitAtk(int iValueType);
	void ChgWitAtk(int iValueType, float fValue);

	// 法力防御
	int GetWitDef(int iValueType);
	void ChgWitDef(int iValueType, float fValue);

	// 特殊属性
	float GetAttribute(int iValueType, int iAttrType);
	void ChgAttribute(int iValueType, int iAttrType, float fValue);

	// 特殊属性上限
	float GetAttributeLimit(int iValueType, int iAttrType);
	void ChgAttributeLimit(int iValueType, int iAttrType, float fValue);

	float GetSiegeBack();
	float GetSiegeBackRatio();

	float GetArmyBaseValueBase();
	float GetArmyBaseValueGrow();
	float GetArmyBaseValue();

public:
	void SetSkillEnable(bool bEnable);
	float GetBuffLifeTime(int iValueType);
	void SetBuffLifeTime(int iValueType, float fValue);

	float GetBuffEffectCD(int iValueType);
	void SetBuffEffectCD(int iValueType, float fValue);

	int GetBuffLevel();

	bool HasBuff(uint32_t dwBuffId);
	bool HasBuffInType(int iType);

	LuaDataAgt* GetBuffSource();
	int GetBuffOwnerListCnt();

	bool IsAllTroopDead(int iGroup, LuaDataAgt* poExcept);

	bool IsAttackCity();

	int GetSelfTroopCountOnBattle();

public:
	int GetMSLevel4Skill();
	float GetMSProgress4Skill();

	int GetMSLevel4Buff();
	float GetMSProgress4Buff();

	float GetMSAddRatioTime4Buff();
	void ChgMSAddRatioTime4Buff(float fValue);

	float GetMSAddRatioTime();
	void ChgMSAddRatioTime(float fValue);

	float GetMSAddRatioPower4Buff();
	void ChgMSAddRatioPower4Buff(float fValue);

	float GetMSAddRatioPower();
	void ChgMSAddRatioPower(float fValue);

	float GetMSAddValue4Buff();
	float GetMSAddValue4Skill();

public:
	float GetActiveSkillMorale(int iValueType);
	void ChgActiveSkillMorale(int iValueType, float fValue);

	int GetActiveSkillLevel();
	
	float GetActiveSkillBaseValue(int iParID);
	float GetActiveSkillRateValue(int iParID);
	float GetActiveSkillAdditionValue(int iParID);

    float GetPassiveSkillBaseValue(int iSkillId, int iParID);

	bool IsHaveActiveSkillCheats(int iType);
	float GetActiveSkillCheatsValue(int iType);
	int GetActiveSkillCheatsLevel();
    bool IsFateInTeam(int iFateId);

public:
	int GetDamageFxType() { return m_iDamageFxType; }
	void SetDamageFxType(int iDamageFxType) { m_iDamageFxType = iDamageFxType; }

	//武将国别与性别
	int GetCountryID();
	int GetGeneralSex();

public:
    float GetResTacticsBuffValue(uint32_t dwBuffId);
};

