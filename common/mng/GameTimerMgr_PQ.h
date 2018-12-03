#pragma once

/*
	���ڼ������������ȶ��е� timer manager
	��һ��ѡ��: ʹ��multimap�����������ݽṹ, Ч�ʿ��ܻ��һ��

	DelTimer()ʱ���ӳٻ��գ���Ҫ���ⲿ�߼���һ֡�����ٽ��л��ղ���
*/


#include "GameTimer.h"
#include "../algo/IndexedPriorityQ.h"
#include "../utils/og_comm.h"
#include "../utils/functional.h"
#include <list>

// ģ���ػ�
template<>
class Comparer<GameTimer*>
{
public:
	int Compare( GameTimer* x, GameTimer* y )
	{
		return CmpTimeVal( x->GetFireTime(), y->GetFireTime() );
	}
};

template<>
class GetItemKey<GameTimer*, uint32_t>
{
public:
	uint32_t GetKey( GameTimer* elem )
    {
        return elem->GetObjID();
    }
};

typedef void (*OnReleaseTimerCb_t)(GameTimer*);
//typedef RayE::Function1<void, GameTimer*> OnReleaseTimerCb_t;

/*
	ע��: Initʱ����OnReleaseTimer�ص����ڻص�����Ҫ�ͷ�GameTimer
*/
class GameTimerMgr_PQ
{
protected:
	static const int MAX_DEAL_NUM_PER_FRAME = 20;

public:
	GameTimerMgr_PQ( )
	{
		m_pPriorityQ = NULL;
		m_oReleasedList.clear();
	}

	~GameTimerMgr_PQ();

	bool Init( int iInitTimerNum, OnReleaseTimerCb_t fOnReleaseTimer );

	void Update();

	// pTimer��Ҫ���������ú�
	void AddTimer( GameTimer* pTimer );
	GameTimer* FindTimer( uint32_t dwTimeID );
	void DelTimer( uint32_t dwTimeID );

	void ModTimerPeriod( uint32_t dwTimeID, uint32_t dwPeroidMs );
	void ModTimerMaxFireCount( uint32_t dwTimeID, int iCount );
    void ModTimerFireTime( uint32_t dwTimeID, struct timeval& tvFireTime);

private:
	void _Push2ReleasedList( GameTimer* pTimer);

private:
	IndexedPriorityQ<GameTimer*, uint32_t>* m_pPriorityQ;
	OnReleaseTimerCb_t	m_fOnReleaseTimer; // ֪ͨӦ�û��ն�ʱ������
	std::list<GameTimer*> m_oReleasedList;
};

