#include <math.h>
#include "LogMacros.h"
#include "GameTime.h"
#include "../../framework/GameObjectPool.h"
#include "../../dungeon/DungeonState.h"
#include "FightObj.h"
#include "CpuSampleStats.h"

FightObj::FightObj()
{
	m_oLuaDataAgt.Init(this);
	m_oBuffManager.Init(this);
	this->_Construct();
}

FightObj::~FightObj()
{

}

void FightObj::Clear()
{
	this->_Construct();
	IObject::Clear();
}

void FightObj::_Construct()
{
	m_poDungeon = NULL;

	m_chGroup = PLAYER_GROUP_NONE;
	m_chType = FIGHTOBJ_NONE;
	m_bId = 0;

	// Buff清除
	m_oBuffManager.Clear();
	// 公式清除
	m_oFormulaManager.Clear();

	// 初始化
	m_iHpCur = 0;
	m_fCurSpeedRatio = 1.0f;

}

bool FightObj::Init(Dungeon* poDungeon, FightPlayer* poFightPlayer, int8_t chType, uint8_t bId)
{
	m_poDungeon = poDungeon;
	m_poFightPlayer = poFightPlayer;

	if (poFightPlayer == NULL)
	{
		m_chGroup = PLAYER_GROUP_NONE;
	}
	else
	{
		m_chGroup = poFightPlayer->m_chGroup;
	}
	m_chType = chType;
	m_bId = bId;

	m_oPassiveSkill4Formula.Init(SKILL_FORMULA, this);
	return true;
}

void FightObj::ChgHp(int iValueChgType, int iValueChgPara, FightObj* poSource, int iDamageRef, int& iHpChgBefore, int& iHpChgAfter, int& iDamageFxSrc, int& iDamageFxTar)
{
	if (poSource == NULL)
	{
		LOGERR("chgHp source obj is null.");
		return;
	}

	poSource->m_oLuaDataAgt.SetDamageFxType(DAMAGE_FX_NONE);
	this->m_oLuaDataAgt.SetDamageFxType(DAMAGE_FX_NONE);

	int iHpChg = iDamageRef;
	do
	{
		if (iValueChgType == VALUE_CHG_TYPE::NONE)
		{
			// 无效的伤害类型
			iHpChg = 0;
			break;
		}

		if (m_iHpCur <= 0 && iValueChgType != VALUE_CHG_TYPE::CHG_HP_REVIVE)
		{
			// 死亡后，只能接受复活加血
			iHpChg = 0;
			iValueChgType = VALUE_CHG_TYPE::NONE;	// 无效HpChg，不再接收过滤
			break;
		}

		// 有公式，取出公式，计算伤害
		FormulaManager* poFormulaMgr = NULL;
		if (iValueChgType == VALUE_CHG_TYPE::CHG_HP_BUFF)
		{
			// Buff伤害使用Target上的公式
			poFormulaMgr = &this->m_oFormulaManager;
		}
		else
		{
			poFormulaMgr = &poSource->m_oFormulaManager;
		}

		if (poFormulaMgr != NULL)
		{
			iHpChg = poFormulaMgr->DoFormula(iValueChgType, iValueChgPara, poSource, this, iDamageRef);
		}
		else
		{
			LOGERR("FormulaManager not found.");
		}

		LOGRUN("after doFormula, iHpChg = %d", iHpChg);
	} while (false);

	if (iValueChgType != VALUE_CHG_TYPE::NONE)
	{
		// 调用过滤
		FilterValue val;
		val.m_iValueChgType = iValueChgType;
		val.m_poSource = poSource;
		val.m_poTarget = this;
		val.m_fFoo1 = iHpChg;
		m_poDungeon->m_oFilterManager.DoFilter(FILTER_TYPE::HP, &val);

		iHpChg = (int)val.m_fFoo1;
		LOGRUN("after doFilter, iHpChg = %d", iHpChg);
	}

	// 结果校验，修正
	if (iHpChgBefore != m_iHpCur)
	{
		// 改变前值修正
		LOGRUN("iHpChgBefore modified by server, from %d to %d", iHpChgBefore, m_iHpCur);
		iHpChgBefore = m_iHpCur;
	}

	// 服务器上血量加减
	m_iHpCur += iHpChg;

	if (m_iHpCur < 0)
	{
		m_iHpCur = 0;
	}
	else if (m_iHpCur > m_oCurrentRtData.m_arrAttrValue[ATTR_HP])
	{
		m_iHpCur = m_oCurrentRtData.m_arrAttrValue[ATTR_HP];
	}

	if (iHpChgAfter != m_iHpCur)
	{
		// 改变后值修正
		LOGRUN("iHpChgAfter modified by server, from %d to %d", iHpChgAfter, m_iHpCur);
		iHpChgAfter = m_iHpCur;
	}

	iDamageFxSrc = poSource->m_oLuaDataAgt.GetDamageFxType();
	iDamageFxTar = this->m_oLuaDataAgt.GetDamageFxType();
}

void FightObj::AfterChgHp(int iValueChgType, int iValueChgPara, FightObj* poSource, int iHpChgBefore, int iHpChgAfter)
{

}

float FightObj::GetSpeed()
{
	return m_fCurSpeedRatio * m_oCurrentRtData.m_arrAttrValue[ATTR_SPEED];
}

void FightObj::Move(PKGMETA::DT_POS_INFO &rstPosInfo)
{
	if(UPDATE_POS_MOVE_TO == rstPosInfo.m_bType)
	{
		this->_UpdatePos();
		m_stDstPos.set(rstPosInfo.m_stDestPos.m_iPosX / 100.0f, 0, rstPosInfo.m_stDestPos.m_iPosY / 100.0f);

#if 0
		// 开放服务器位置纠正,但是由于目前服务器没有Update，服务器的时间不会暂停，所以目前的UpdatePos是有问题的，不启用位置纠正
		rstPosInfo.m_stCurrPos.m_iPosX = (int)(m_stCurPos.x * 100.0f);
		rstPosInfo.m_stCurrPos.m_iPosY = (int)(m_stCurPos.z * 100.0f);
		rstPosInfo.m_llTimeStamp = m_llCurStampTime;
#endif
	}
	else if(UPDATE_POS_SET == rstPosInfo.m_bType)
	{
		m_stCurPos.set(rstPosInfo.m_stDestPos.m_iPosX / 100.0f, 0, rstPosInfo.m_stDestPos.m_iPosY / 100.0f);
		m_stDstPos = m_stCurPos;
        m_llLastStampTime = rstPosInfo.m_llTimeStamp;
	}
}

void FightObj::UpdateSpeed(PKGMETA::DT_SPEED_INFO &rstSpeedInfo)
{
	// 更新位置
	this->_UpdatePos();

	float fRatio = rstSpeedInfo.m_nSpeedRatio / 100.0f;
	if(fRatio < 0.0f)
	{
		m_fCurSpeedRatio = m_fCurSpeedRatio / (-fRatio);
	}
	else if(fRatio > 0.0f)
	{
		m_fCurSpeedRatio *= fRatio;
	}
	else
	{
		m_fCurSpeedRatio = 1.0f;
	}
}

void FightObj::_UpdatePos()
{
	m_llCurStampTime = (int64_t)(CGameTime::Instance().GetCurrSecond()*1000 + CGameTime::Instance().GetCurrMsInSec());

	float fTimePass = (m_llCurStampTime - m_llLastStampTime) / 1000.0f;
	float fSpeed = this->GetSpeed();
	Vector3 stDirection = (m_stDstPos - m_stCurPos).normalized();

	Vector3 stMove = stDirection * (fSpeed * fTimePass);
	if (stMove.sqrtMagnitude() >= (m_stDstPos - m_stCurPos).sqrtMagnitude())
	{
		m_stCurPos = m_stDstPos;
	}
	else
	{
		m_stCurPos = m_stCurPos + stMove;
	}

	m_llLastStampTime = m_llCurStampTime;
}
