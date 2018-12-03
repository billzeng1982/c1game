#pragma  once
#include <list>
#include "object.h"
#include "vector3.h"
#include "cs_proto.h"
#include "ov_res_public.h"
#include "../../dungeon/Dungeon.h"
#include "../luainterface/LuaDataAgt.h"
#include "../skill/BuffManager.h"
#include "../skill/FormulaManager.h"
#include "../skill/PassiveSkill.h"

class FightPlayer;

class FightObjRuntimeData
{
public:
	 // 基础属性
	float m_arrAttrValue[MAX_ATTR_ADD_NUM];
	float m_arrAttrValueLimit[MAX_ATTR_ADD_NUM];

	// 兵种属性
	float m_fArmyAttackCityFirstCD;
	float m_fArmyAttackCityCD;
	float m_fArmyAttackCityRatio;

	float m_fArmyAttackCD;
	float m_fArmyDamageCD;

	float m_fArmyAttackMoveSpeedRatio;

public:
	float GetAttribute(int iType)
	{
		if (iType >= 0 && iType < MAX_ATTR_ADD_NUM)
		{
			return m_arrAttrValue[iType];
		}

		return 0;
	}

	void ChgAttribute(int iType, float fValue)
	{
		if (iType >= 0 && iType < MAX_ATTR_ADD_NUM)
		{
			m_arrAttrValue[iType] += fValue;
		}
	}

	float GetAttributeLimit(int iType)
	{
		if (iType >= 0 && iType < MAX_ATTR_ADD_NUM)
		{
			return m_arrAttrValueLimit[iType];
		}

		return 0;
	}

	void ChgAttributeLimit(int iType, float fValue)
	{
		if (iType >= 0 && iType < MAX_ATTR_ADD_NUM)
		{
			m_arrAttrValueLimit[iType] += fValue;
		}
	}
    void AddAllAttr(float * arrAttrValue)
    {
        if (arrAttrValue == NULL)
        {
            return;
        }
        for (int i = 0; i < MAX_ATTR_ADD_NUM; i++)
        {
            m_arrAttrValue[i] += arrAttrValue[i];
        }
    }
};

class FightObj : public IObject
{
public:
	FightObj();
	virtual ~FightObj();
	virtual void Clear();

private:
	void _Construct();

protected:
	bool Init(Dungeon* poDungeon, FightPlayer* poFightPlayer, int8_t chType, uint8_t bId);

public:
	virtual void Update(int dt) { }
	virtual void ChgHp(int iValueChgType, int iValueChgPara, FightObj* poSource, int iDamageRef, int& iHpChgBefore, int& iHpChgAfter, int& iDamageFxSrc, int& iDamageFxTar);
	virtual void AfterChgHp(int iValueChgType, int iValueChgPara, FightObj* poSource, int iHpChgBefore, int iHpChgAfter);

	virtual float GetSpeed();
	virtual void Move(PKGMETA::DT_POS_INFO &rstPosInfo);				// 移动操作
	virtual void UpdateSpeed(PKGMETA::DT_SPEED_INFO &rstSpeedInfo);		// 速度改变

private:
	void _UpdatePos();

public:
	Dungeon* m_poDungeon;
	FightPlayer* m_poFightPlayer;

	int8_t m_chGroup;
	int8_t m_chType;
	uint8_t m_bId;

	FightObjRuntimeData m_oOriginRtData;
	FightObjRuntimeData m_oCurrentRtData;

	LuaDataAgt m_oLuaDataAgt;

	BuffManager m_oBuffManager;

	FormulaManager m_oFormulaManager;

	// 特殊被动
	PassiveSkill m_oPassiveSkill4Formula;	// 公式被动

public:
	int m_iHpCur;						// 当前生命
	float m_fCurSpeedRatio;				// 当前速度比例

	Vector3 m_stCurPos;
	Vector3 m_stDstPos;

protected:
	int64_t m_llCurStampTime;		// 毫秒
	int64_t m_llLastStampTime;		// 毫秒
};

