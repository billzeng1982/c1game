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
	uint32_t m_dwSkillId;		// ��ʦ��ID
	uint8_t m_bSkillLevel;		// ��ʦ���ȼ�
	float m_fPowerProgress;		// ��ʦ�����ܽ���

	float m_fMSAddRatioTime;	// ��ʦ��ʱ������
	float m_fMSAddRatioPower;	// ��ʦ����������

private:
	FightPlayer* m_poFightPlayer;
	RESMASTERSKILL* m_poResMasterSkill;

public:
	bool m_bEnable;
	float m_fSkillCD;
};

