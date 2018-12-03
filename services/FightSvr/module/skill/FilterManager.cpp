#include "LogMacros.h"
#include "common_proto.h"
#include "../fightobj/FightObj.h"
#include "../fightobj/Troop.h"
#include "FilterManager.h"

LuaDataAgt* FilterValue::GetSource()
{
	if (m_poSource != NULL)
	{
		return &m_poSource->m_oLuaDataAgt;
	}

	return NULL;
}

LuaDataAgt* FilterValue::GetTarget()
{
	if (m_poTarget != NULL)
	{
		return &m_poTarget->m_oLuaDataAgt;
	}
	
	return NULL;
}

void FilterManager::Clear()
{
	MapFilter_t::iterator iter = m_dictFilters.begin();
	for (; iter != m_dictFilters.end(); iter++)
	{
		delete iter->second;
		iter->second = NULL;
	}
	m_dictFilters.clear();
}

void FilterManager::AddFilter(int iFilterType, FightObj* poFilterOwner, int iPriority, IFilter* poFilter)
{
	FilterKey key;
	key.m_iFilterType = iFilterType;
	key.m_poFilterOwnerObj = poFilterOwner;

	FilterVal val;
	val.m_iPriority = iPriority;
	val.m_poFilter = poFilter;

	ListFilter_t* pList = m_dictFilters[key];
	if (pList == NULL)
	{
		pList = new ListFilter_t();
		pList->push_back(val);

		m_dictFilters[key] = pList;
	}
	else
	{
		ListFilter_t::iterator iter = pList->begin();
		for (; iter != pList->end(); iter++)
		{
			if (*iter == val)
			{
				LOGERR("filter already exist.");
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

void FilterManager::DelFilter(int iFilterType, FightObj* poFilterOwner, int iPriority, IFilter* poFilter)
{
	FilterKey key;
	key.m_iFilterType = iFilterType;
	key.m_poFilterOwnerObj = poFilterOwner;

	FilterVal val;
	val.m_iPriority = iPriority;
	val.m_poFilter = poFilter;

	ListFilter_t* pList = m_dictFilters[key];
	if (pList != NULL)
	{
		ListFilter_t::iterator iter = pList->begin();
		for (; iter != pList->end(); iter++)
		{
			if (*iter == val)
			{
				iter = pList->erase(iter);
				break;
			}
		}
	}
}

void FilterManager::DelFilter(IFilter* poFilter)
{
	MapFilter_t::iterator it1 = m_dictFilters.begin();
	for (; it1 != m_dictFilters.end(); it1++)
	{
		ListFilter_t* pList = it1->second;
		if (pList == NULL)
		{
			continue;
		}
		ListFilter_t::iterator it2 = pList->begin();
		for (; it2 != pList->end(); /*do nothing*/)
		{
			if (it2->m_poFilter == poFilter)
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

void FilterManager::DoFilter(int iFilterType, FilterValue* poValue)
{
	// 发起对象上的过滤器处理，可能对伤害加强
	_DoFilter(iFilterType, poValue->m_poSource, poValue);

	// 防止过滤两次
	if (poValue->m_poSource != poValue->m_poTarget)
	{
		// 目标对象上的过滤器处理，可能对伤害减免
		_DoFilter(iFilterType, poValue->m_poTarget, poValue);
	}
}

void FilterManager::_DoFilter(int iFilterType, FightObj* poFilterOwner, FilterValue* poValue)
{
	if (poFilterOwner == NULL)
	{
		return;
	}

	FilterKey key;
	key.m_iFilterType = iFilterType;
	key.m_poFilterOwnerObj = poFilterOwner;

	ListFilter_t* pList = m_dictFilters[key];
	if (pList != NULL)
	{
		ListFilter_t::iterator iter = pList->begin();
		for (; iter != pList->end(); iter++)
		{
			iter->m_poFilter->DoFilter(iFilterType, poFilterOwner, poValue);
		}
	}
}
