#pragma once
#include <list>
#include <map>
#include "define.h"

class Dungeon;
class FightObj;
class LuaDataAgt;

// 值改变类型
class VALUE_CHG_TYPE
{
public:
	enum T
	{
		NONE = 0,               // 无类型

		// 位置相关
		CHG_POS_BEGIN = 100,
		CHG_POS_MOVE,           // 正常移动
		CHG_POS_TOWER,          // 瞭望塔反弹
		CHG_POS_BARRIER,        // 栅栏反弹
		CHG_POS_END,

		// 血量相关
		CHG_HP_BEGIN = 200,
		CHG_HP_COMM,				// 通用
		CHG_HP_ATK_CITYGATE,        // 攻城门
		CHG_HP_ATK_CITYWALL,        // 攻城墙
		CHG_HP_ATK_CITYBACK,        // 城墙反伤
		CHG_HP_ATK_CITYGATEBACK,    // 城门反伤
		CHG_HP_ATK_TOWER,           // 攻瞭望塔
		CHG_HP_ATK_BARRIER,         // 攻栅栏
		CHG_HP_ATK_NORMAL,          // 白兵
		CHG_HP_ATK_SHOOT,           // 射击
		CHG_HP_ATK_SPEAR,           // 枪击
		CHG_HP_ATK_RUSH,            // 突击
		CHG_HP_ATK_FACE,            // 迎击
		CHG_HP_ATK_AMBUSH,          // 伏兵
		CHG_HP_SOLO,                // 单挑
		CHG_HP_REVIVE,              // 复活
		CHG_HP_GENERALSKILL,        // 武将技能
		CHG_HP_PASSIVESKILL,        // 被动技能
		CHG_HP_MASTERSKILL,         // 军师技能
		CHG_HP_BUFF,                // Buff
		CHG_HP_SKILLCHEATS,         // 技能秘籍
		CHG_HP_END,

		// 速度相关
		CHG_SPEED_BEGIN = 300,
		CHG_SPEED_ARMYSKILL,            // 兵种技
		CHG_SPEED_GENERALSKILL,         // 武将技能
		CHG_SPEED_ATK_NORMAL,           // 白兵
		CHG_SPEED_END,

		// 攻击条件
		CHG_COND_BEGIN = 400,
		CHG_COND_ATK_NORMAL,            // 白兵
		CHG_COND_ATK_ARMYSKILL,         // 兵种技
		CHG_COND_ATK_CITY,              // 攻城
		CHG_COND_ATK_OBSTACLE,          // 攻障碍
		CHG_COND_ATK_LOCK,              // 攻击索敌
		CHG_COND_ATK_CITYPAUSE,         // 攻城暂停


		// 能否使能某项潜能
		CHG_COND_SOLO,              // 是否可以单挑
		CHG_COND_ARMYSKILL,         // 兵种技能使能条件
		CHG_COND_GENERALSKILL,      // 武将技能使能条件
		CHG_COND_MASTERSKILL,       // 军师技能使能条件
		CHG_COND_END,

		// 事件触发
		CHG_TRIG_BEGIN = 500,
		CHG_TRIG_GENERALSKILL,          // 武将技能触发
		CHG_TRIG_GENERALSKILL_END,      // 武将技能触发完成
		CHG_TRIG_MASTERSKILL,           // 军师技能触发
		CHG_TRIG_MASTERSKILL_END,       // 军师技能触发完成
		CHG_TRIG_BUFF_ADD_BEGIN,        // 添加Buff前触发
		CHG_TRIG_BUFF_ADD_END,          // 添加Buff后触发
		CHG_TRIG_BUFF_DEL,              // 删除Buff触发
		CHG_TRIG_TROOP_DEAD_DONE,       // 部队死亡完成
		CHG_TRIG_TROOP_RETREAT_DONE,    // 部队回城完成
		CHG_TRIG_TROOP_OUTCITY_DONE,    // 部队出城完成
		CHG_TRIG_END,
	};
};

// 过滤器类型，对VALUE_CHG_TYPE再归类
class FILTER_TYPE
{
public:
	enum T
	{
		NONE = 0,
		POS,		// 位置
		HP,			// 血量
		SPEED,		// 速度
		COND,		// 条件
		TRIG,		// 触发
	};
};

class FilterValue
{
public:
	FightObj* m_poSource;
	FightObj* m_poTarget;

	int m_iValueChgType;
	int m_iValueChgPara;

	float m_fFoo1;
	float m_fFoo2;
	float m_fFoo3;
	float m_fFoo4;
	float m_fFoo5;

public:
	LuaDataAgt* GetSource();

	LuaDataAgt* GetTarget();
};

class IFilter
{
public:
	virtual ~IFilter() {};
	virtual void DoFilter(int iFilterType, FightObj* poOwner, FilterValue* poValue) = 0;
};

class FilterManager
{
private:
	struct FilterKey
	{
		int m_iFilterType;
		FightObj* m_poFilterOwnerObj;

		bool operator <(const FilterKey& other) const
		{
			if (m_iFilterType < other.m_iFilterType)
			{
				return true;
			}
			else if (m_iFilterType == other.m_iFilterType)
			{
				return m_poFilterOwnerObj < other.m_poFilterOwnerObj;
			}

			return false;
		}
	};

	struct FilterVal
	{
		int m_iPriority;
		IFilter* m_poFilter;

		bool operator ==(const FilterVal& other) const
		{
			return m_iPriority == other.m_iPriority && m_poFilter == other.m_poFilter;
		}

		bool operator <(const FilterVal& other) const
		{
			return m_iPriority < other.m_iPriority;
		}
	};

	typedef std::list<FilterVal> ListFilter_t;
	typedef std::map<FilterKey, ListFilter_t*> MapFilter_t;
	MapFilter_t m_dictFilters;

public:
	FilterManager() {}
	virtual ~FilterManager()
	{
		this->Clear();
	}
	void Clear();

public:
	void AddFilter(int iFilterType, FightObj* poFilterOwner, int iPriority, IFilter* poFilter);

	void DelFilter(int iFilterType, FightObj* poFilterOwner, int iPriority, IFilter* poFilter);

	void DelFilter(IFilter* poFilter);

	void DoFilter(int iFilterType, FilterValue* poValue);

private:
	void _DoFilter(int iFilterType, FightObj* poFilterOwner, FilterValue* poValue);
};
