#include "BuffManager.h"
#include "LogMacros.h"
#include "define.h"
#include "../../gamedata/GameDataMgr.h"

void BuffManager::Clear()
{
	MapId2Buff_t::iterator iter = m_dictBuffAll.begin();
	for (; iter != m_dictBuffAll.end(); iter++)
	{
		Buff::Release(iter->second);
	}

	m_dictBuffAll.clear();
	m_dictBuffIncompatible.clear();
}

bool BuffManager::Init(FightObj* poOwner)
{
	m_poOwner = poOwner;
	return true;
}

void BuffManager::Update(float dt)
{
	MapId2Buff_t::iterator iter = m_dictBuffAll.begin();
	for (; iter != m_dictBuffAll.end(); /*do nothing*/)
	{
		Buff* poBuff = iter->second;
		assert (poBuff != NULL);
		if (poBuff->m_bIsDead)
		{
			poBuff->DoDead();

			// ��Buff������ɾ����Buff
			m_dictBuffAll.erase(iter++);

			// �ӻ�������ɾ����Buff
			m_dictBuffIncompatible.erase(poBuff->m_dwBuffId);
		}
		else
		{
			//poBuff->Update(dt);

			iter++;
		}
	}
}

bool BuffManager::HasBuff(uint32_t dwBuffId)
{
	return m_dictBuffAll.find(dwBuffId) != m_dictBuffAll.end();
}

bool BuffManager::HasBuffInType(int iType)
{
	MapId2Buff_t::iterator iter = m_dictBuffAll.begin();
	for (; iter != m_dictBuffAll.end(); iter++)
	{
		if (iter->second->m_iType == iType)
		{
			return true;
		}
	}

	return false;
}

Buff* BuffManager::GetBuff(uint32_t dwBuffId)
{
	MapId2Buff_t::iterator iter = m_dictBuffAll.find(dwBuffId);

    if (iter != m_dictBuffAll.end())
    {
        return iter->second;
    }
    else
    {
        return NULL;
    }
}

void BuffManager::AddBuff(uint32_t dwBuffId, FightObj* poSource, std::list<FightObj*>* poOwnerList)
{
	RESBUFF* poResBuff = NULL;
	RESBUFF* poResBuffOld = NULL;

	Buff* poBuffOld = this->GetBuff(dwBuffId);
	if (poBuffOld != NULL)
	{
		poResBuff = poBuffOld->m_poResBuff;
		poResBuffOld = poBuffOld->m_poResBuff;
	}
	else
	{
		poResBuff = CGameDataMgr::Instance().GetResBuffMgr().Find(dwBuffId);
	}

	if (poResBuff == NULL)
	{
		LOGERR("ResBuff not found.");
		return;
	}

	bool bReplace = false;

	int iIncompatibleGroup = poResBuff->m_wIncompatibleGroup;	// �������
	int iPriority = poResBuff->m_wPriority;						// ���ȼ�

	// �¼�BuffId
	uint32_t dwNewBuffId = dwBuffId;

	if (iIncompatibleGroup != 0)
	{
		// ����
		if (m_dictBuffIncompatible.find(iIncompatibleGroup) != m_dictBuffIncompatible.end())
		{
			// �û���������Buff
			poBuffOld = m_dictBuffIncompatible[iIncompatibleGroup];
			poResBuffOld = poBuffOld->m_poResBuff;
			if (iPriority >= poResBuffOld->m_wPriority)
			{
				// ��Buff���ȼ�����Buff���ȼ���
				// ��Buff���ȼ�����Buff���ȼ���ͬ���������滻ǰ�ߣ�������
				bReplace = true;
			}
		}
		else
		{
			// ������û��Buff����Buff����ֱ�����
			bReplace = true;
		}
	}
	else
	{
		// ���棬��Buff����ֱ�����
		bReplace = true;
	}

	if (bReplace)
	{
		this->_ReplaceBuff(poBuffOld, dwNewBuffId, poSource, poOwnerList);
	}
}

void BuffManager::DelBuff(uint32_t dwBuffId, int iBuffType)
{
	if (dwBuffId == 0)
	{
		MapId2Buff_t::iterator iter = m_dictBuffAll.begin();
		for (; iter != m_dictBuffAll.end(); iter++)
		{
			Buff* poBuff = iter->second;
			if (poBuff->m_poResBuff->m_bDelBySpecified)
			{
				// ��Buff��ָ��ɾ������������ɾ��
			}
			else
			{
				// ��Buff����ָ��ɾ������������ɾ��
				if (iBuffType == (int)BUFF_TYPE_NONE)
				{
					// ������е�Buff����ָ��Buff
					poBuff->m_bIsDead = true;
				}
				else if (iBuffType == poBuff->m_iType)
				{
					// ���ָ�����͵�Buff����ָ��Buff
					poBuff->m_bIsDead = true;
				}
			}
		}
	}
	else
	{
		Buff* poBuff = this->GetBuff(dwBuffId);
		if (poBuff != NULL)
		{
			poBuff->m_bIsDead = true;
		}
	}

	// �������ȵ���Update�������Buff
	this->Update(0);
}

void BuffManager::_ReplaceBuff(Buff* poBuffOld, uint32_t dwNewBuffId, FightObj* poSource, std::list<FightObj*>* poOwnerList)
{
	if (poBuffOld != NULL && poBuffOld->m_dwBuffId == dwNewBuffId)
	{
		// Buffˢ��
		if (poBuffOld->m_poResBuff->m_bCanOverride != 0 || poBuffOld->m_bIsDead)
		{
			// ���Ը��ǣ�ˢ��
			// �ո�����������δ�����Ƴ���ֱ�Ӹ���
			poBuffOld->Refresh(poSource, poOwnerList);
		}
	}
	else
	{
		// Buff�滻
		this->_DelBuff(poBuffOld);

		Buff* poBuffNew = Buff::Get();
		poBuffNew->Init(dwNewBuffId, m_poOwner, poSource, poOwnerList);

		m_dictBuffAll[dwNewBuffId] = poBuffNew;
		if (poBuffNew->m_poResBuff->m_wIncompatibleGroup != 0)
		{
			m_dictBuffIncompatible[poBuffNew->m_poResBuff->m_wIncompatibleGroup] = poBuffNew;
		}
	}
}

void BuffManager::_DelBuff(Buff* poBuff)
{
	if (poBuff != NULL)
	{
		poBuff->DoDead();

		// ��Buff������ɾ����Buff
		m_dictBuffAll.erase(poBuff->m_dwBuffId);

		// �ӻ�������ɾ����Buff
		m_dictBuffIncompatible.erase(poBuff->m_dwBuffId);
	}
}
