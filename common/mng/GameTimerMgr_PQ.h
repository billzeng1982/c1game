#pragma once

/*
	基于加了索引的优先队列的 timer manager
	另一种选择: 使用multimap来做基础数据结构, 效率可能会高一点

	DelTimer()时，延迟回收，需要等外部逻辑在一帧跑完再进行回收操作
*/


#include "GameTimer.h"
#include "../algo/IndexedPriorityQ.h"
#include "../utils/og_comm.h"
#include "../utils/functional.h"
#include <list>

// 模版特化
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
	注意: Init时传入OnReleaseTimer回调，在回调里主要释放GameTimer
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

	// pTimer需要在外面设置好
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
	OnReleaseTimerCb_t	m_fOnReleaseTimer; // 通知应用回收定时器对象
	std::list<GameTimer*> m_oReleasedList;
};

