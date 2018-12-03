#include "LogMacros.h"
#include "Troop.h"
#include "General.h"
#include "../skill/GeneralSkill.h"
#include "../skill/PassiveSkill.h"
#include "../../framework/GameObjectPool.h"

General::General()
{
	m_poActiveSkill = NULL;
}

General::~General()
{

}

void General::Clear()
{
	GeneralSkill::Release(m_poActiveSkill);
	m_poActiveSkill = NULL;

	std::vector<PassiveSkill*>::iterator it = m_listPassiveSkill.begin();
	for (; it!=m_listPassiveSkill.end(); it++)
	{
		PassiveSkill::Release(*it);
	}

	m_listPassiveSkill.clear();

	m_dictPassiveSkillId.clear();
}

bool General::Init(Troop* poTroop)
{
	m_poTroop = poTroop;

	do
	{
		if (!_InitActiveSkill())
		{
			break;
		}

		if (!_InitDefaultPassiveSkill())
		{
			break;
		}

		if (!_InitPassiveSkill())
		{
			break;
		}
	} while (false);

	return true;
}

void General::Update(int dt)
{
	std::vector<PassiveSkill*>::iterator it = m_listPassiveSkill.begin();
	for (; it!=m_listPassiveSkill.end(); it++)
	{
		(*it)->Update(dt);
	}
}

bool General::_InitActiveSkill()
{
	// 主动技能
	m_poActiveSkill = GeneralSkill::Get();
	if (m_poActiveSkill == NULL)
	{
		LOGERR("m_poActiveSkill is null");
		return false;
	}

	uint32_t dwASkillId = m_poTroop->m_dwActiveSkillId;

	bool bRet = m_poActiveSkill->Init(dwASkillId, m_poTroop);
	if (!bRet)
	{
		LOGERR("m_poActiveSkill init failed, id = %u", dwASkillId);
		return false;
	}

	return true;
}

bool General::_InitDefaultPassiveSkill()
{
	// 固有被动技能
	for (int i=0; i<m_poTroop->m_poResGeneral->m_bPassiveSkillNumBorn; i++)
	{
		uint32_t id = m_poTroop->m_poResGeneral->m_passiveSkillIdBorn[i];
		if (id > 0)
		{
			PassiveSkill* poPassiveSkill = PassiveSkill::Get();
			if (poPassiveSkill == NULL)
			{
				LOGERR("poPassiveSkill is null");
				return false;
			}

			bool bRet = poPassiveSkill->Init(id, m_poTroop);
			if (!bRet)
			{
				LOGERR("poPassiveSkill init failed, id = %u", id);
				return false;
			}

			m_listPassiveSkill.push_back(poPassiveSkill);
			m_dictPassiveSkillId[id] = 1;
		}
	}
	return true;
}

bool General::_InitPassiveSkill()
{
	// 星级被动技能
	for (int i = 0; i < m_poTroop->m_bPassiveSkillCnt; i++)
	{
		uint32_t id = m_poTroop->m_dwPassiveSkillId[i];

		PassiveSkill* poPassiveSkill = PassiveSkill::Get();
		if (poPassiveSkill == NULL)
		{
			LOGERR("poPassiveSkill is null");
			return false;
		}

		bool bRet = poPassiveSkill->Init(id, m_poTroop);
		if (!bRet)
		{
			LOGERR("poPassiveSkill init failed, id = %u", id);
			return false;
		}

		m_listPassiveSkill.push_back(poPassiveSkill);
		m_dictPassiveSkillId[id] = m_poTroop->m_szPassiveSkillLevel[i];
	}

	return true;
}

