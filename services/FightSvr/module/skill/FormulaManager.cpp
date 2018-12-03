#include "LogMacros.h"
#include "../fightobj/FightObj.h"
#include "FormulaManager.h"

void FormulaManager::Clear()
{
	MapFormula_t::iterator iter = m_dictFormulas.begin();
	for (; iter != m_dictFormulas.end(); iter++)
	{
		delete iter->second;
		iter->second = NULL;
	}

	m_dictFormulas.clear();
}

void FormulaManager::AddFormula(int iFormulaType, int iFormulaPara, int iPriority, IFormula* poFormula)
{
	FormulaKey key;
	key.m_iFormulaType = iFormulaType;
	key.m_iFormulaPara = iFormulaPara;

	FormulaVal val;
	val.m_iPriority = iPriority;
	val.m_poFormula = poFormula;

	ListFormula_t* pList = m_dictFormulas[key];
	if (pList == NULL)
	{
		pList = new ListFormula_t();
		pList->push_back(val);

		m_dictFormulas[key] = pList;
	}
	else
	{
		ListFormula_t::iterator iter = pList->begin();
		for (; iter != pList->end(); iter++)
		{
			if (*iter == val)
			{
				LOGERR("formula already exist.");
				return;
			}
			else if (*iter < val)
			{
				pList->insert(iter, val);
				break;
			}
		}

		if (iter == pList->end())
		{
			pList->push_back(val);
		}
	}
}

void FormulaManager::DelFormula(int iFormulaType, int iFormulaPara, int iPriority, IFormula* poFormula)
{
    LOGRUN("FormulaManager<%p> del Formula-%p FormulaType-%d FormulaPara-%d",
            this, poFormula, iFormulaType, iFormulaPara);

	FormulaKey key;
	key.m_iFormulaType = iFormulaType;
	key.m_iFormulaPara = iFormulaPara;

	FormulaVal val;
	val.m_iPriority = iPriority;
	val.m_poFormula = poFormula;

	ListFormula_t* pList = m_dictFormulas[key];
	if (pList != NULL)
	{
		ListFormula_t::iterator iter = pList->begin();
		for (; iter != pList->end(); iter++)
		{
			if (*iter == val)
			{
				pList->erase(iter);
				break;
			}
		}
	}
}

void FormulaManager::DelFormula(IFormula* poFormula)
{
    LOGRUN("FormulaManager<%p> del Formula-%p", this, poFormula);
	MapFormula_t::iterator it1 = m_dictFormulas.begin();
	for (; it1 != m_dictFormulas.end(); it1++)
	{
		ListFormula_t* pList = it1->second;
		if (pList == NULL)
		{
			continue;
		}

		ListFormula_t::iterator it2 = pList->begin();
		for (; it2 != pList->end(); /*do nothing*/)
		{
			if (it2->m_poFormula == poFormula)
			{
				it2 = pList->erase(it2);
			}
			else
			{
				it2++;
			}
		}
	}
}

int FormulaManager::DoFormula(int iFormulaType, int iFormulaPara, FightObj* poSource, FightObj* poTarget, int iDamageRef)
{
	FormulaKey key;
	key.m_iFormulaType = iFormulaType;
	key.m_iFormulaPara = iFormulaPara;

	ListFormula_t* pList = m_dictFormulas[key];
	if (pList == NULL || pList->empty())
	{
		LOGERR("formula not found, iFormulaType = %d, iFormulaPara = %d", iFormulaType, iFormulaPara);
		return 0;
	}
	else
	{
		return pList->front().m_poFormula->DoFormula(iFormulaType, iFormulaPara, poSource, poTarget, iDamageRef);
	}
}

