#pragma once
#include <list>
#include <map>
#include "define.h"

class Dungeon;
class FightObj;
class LuaDataAgt;

// ֵ�ı�����
class VALUE_CHG_TYPE
{
public:
	enum T
	{
		NONE = 0,               // ������

		// λ�����
		CHG_POS_BEGIN = 100,
		CHG_POS_MOVE,           // �����ƶ�
		CHG_POS_TOWER,          // �t��������
		CHG_POS_BARRIER,        // դ������
		CHG_POS_END,

		// Ѫ�����
		CHG_HP_BEGIN = 200,
		CHG_HP_COMM,				// ͨ��
		CHG_HP_ATK_CITYGATE,        // ������
		CHG_HP_ATK_CITYWALL,        // ����ǽ
		CHG_HP_ATK_CITYBACK,        // ��ǽ����
		CHG_HP_ATK_CITYGATEBACK,    // ���ŷ���
		CHG_HP_ATK_TOWER,           // ���t����
		CHG_HP_ATK_BARRIER,         // ��դ��
		CHG_HP_ATK_NORMAL,          // �ױ�
		CHG_HP_ATK_SHOOT,           // ���
		CHG_HP_ATK_SPEAR,           // ǹ��
		CHG_HP_ATK_RUSH,            // ͻ��
		CHG_HP_ATK_FACE,            // ӭ��
		CHG_HP_ATK_AMBUSH,          // ����
		CHG_HP_SOLO,                // ����
		CHG_HP_REVIVE,              // ����
		CHG_HP_GENERALSKILL,        // �佫����
		CHG_HP_PASSIVESKILL,        // ��������
		CHG_HP_MASTERSKILL,         // ��ʦ����
		CHG_HP_BUFF,                // Buff
		CHG_HP_SKILLCHEATS,         // �����ؼ�
		CHG_HP_END,

		// �ٶ����
		CHG_SPEED_BEGIN = 300,
		CHG_SPEED_ARMYSKILL,            // ���ּ�
		CHG_SPEED_GENERALSKILL,         // �佫����
		CHG_SPEED_ATK_NORMAL,           // �ױ�
		CHG_SPEED_END,

		// ��������
		CHG_COND_BEGIN = 400,
		CHG_COND_ATK_NORMAL,            // �ױ�
		CHG_COND_ATK_ARMYSKILL,         // ���ּ�
		CHG_COND_ATK_CITY,              // ����
		CHG_COND_ATK_OBSTACLE,          // ���ϰ�
		CHG_COND_ATK_LOCK,              // ��������
		CHG_COND_ATK_CITYPAUSE,         // ������ͣ


		// �ܷ�ʹ��ĳ��Ǳ��
		CHG_COND_SOLO,              // �Ƿ���Ե���
		CHG_COND_ARMYSKILL,         // ���ּ���ʹ������
		CHG_COND_GENERALSKILL,      // �佫����ʹ������
		CHG_COND_MASTERSKILL,       // ��ʦ����ʹ������
		CHG_COND_END,

		// �¼�����
		CHG_TRIG_BEGIN = 500,
		CHG_TRIG_GENERALSKILL,          // �佫���ܴ���
		CHG_TRIG_GENERALSKILL_END,      // �佫���ܴ������
		CHG_TRIG_MASTERSKILL,           // ��ʦ���ܴ���
		CHG_TRIG_MASTERSKILL_END,       // ��ʦ���ܴ������
		CHG_TRIG_BUFF_ADD_BEGIN,        // ���Buffǰ����
		CHG_TRIG_BUFF_ADD_END,          // ���Buff�󴥷�
		CHG_TRIG_BUFF_DEL,              // ɾ��Buff����
		CHG_TRIG_TROOP_DEAD_DONE,       // �����������
		CHG_TRIG_TROOP_RETREAT_DONE,    // ���ӻس����
		CHG_TRIG_TROOP_OUTCITY_DONE,    // ���ӳ������
		CHG_TRIG_END,
	};
};

// ���������ͣ���VALUE_CHG_TYPE�ٹ���
class FILTER_TYPE
{
public:
	enum T
	{
		NONE = 0,
		POS,		// λ��
		HP,			// Ѫ��
		SPEED,		// �ٶ�
		COND,		// ����
		TRIG,		// ����
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
