#pragma once
#include <list>
#include <map>
#include "define.h"

class FightObj;

class IFormula
{
public:
	IFormula() { }
	virtual ~IFormula() { }

public:
	virtual int DoFormula(int iFormulaType, int iFormulaPara, FightObj* poSource, FightObj* poTarget, int iDamageRef) = 0;
};

class FormulaManager
{
private:
	struct FormulaKey
	{
		int m_iFormulaType;
		int m_iFormulaPara;

		bool operator <(const FormulaKey& other) const
		{
			if (m_iFormulaType < other.m_iFormulaType)
			{
				return true;
			}
			else if (m_iFormulaType == other.m_iFormulaType)
			{
				return m_iFormulaPara < other.m_iFormulaPara;
			}

			return false;
		}
	};

	struct FormulaVal
	{
		int m_iPriority;
		IFormula* m_poFormula;

		bool operator ==(const FormulaVal& other) const
		{
			return m_iPriority == other.m_iPriority && m_poFormula == other.m_poFormula;
		}

		bool operator <(const FormulaVal& other) const
		{
			return m_iPriority < other.m_iPriority;
		}
	};

	typedef std::list<FormulaVal> ListFormula_t;
	typedef std::map<FormulaKey, ListFormula_t*> MapFormula_t;
	MapFormula_t m_dictFormulas;

public:
	FormulaManager() {}
	virtual ~FormulaManager()
	{
		this->Clear();
	}
	void Clear();

public:
	void AddFormula(int iFormulaType, int iFormulaPara, int iPriority, IFormula* poFormula);

	void DelFormula(int iFormulaType, int iFormulaPara, int iPriority, IFormula* poFormula);

	void DelFormula(IFormula* poFormula);

	int DoFormula(int iFormulaType, int iFormulaPara, FightObj* poSource, FightObj* poTarget, int iDamageRef);
};
