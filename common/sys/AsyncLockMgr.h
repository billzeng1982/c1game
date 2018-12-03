#pragma once

#include "AsyncLock.h"
#include "mempool.h"
#include "IndexedPriorityQ.h"
#include "comm_func.h"
#include "og_comm.h"

#include <map>
using namespace std;

template<> 
class Comparer<AsyncLock*>
{
public:
    int Compare( AsyncLock* x, AsyncLock* y )
    {
		return CmpTimeVal( x->GetTimeoutTime(), y->GetTimeoutTime() );
    }
};


template<> 
class GetItemKey<AsyncLock*, uint64_t>
{
public: 
    uint64_t GetKey( AsyncLock* elem ) 
    { 
        return elem->GetLockID();
    }
};

class AsyncLockMgr
{
	typedef void(*fReleaseAsyncLockObsvr_t)(AsyncLockObsvr* pObsvr); 
	typedef map<uint64_t, AsyncLock*> AsyncLockMap_t;

public:
	const static int DEFAULT_LOCK_NUM = 1000;
	const static int ASYNCLOCK_DFT_TIMEOUT = 5; // second
	
public:
	AsyncLockMgr()
	{
		m_pAsyncLockTimoutQ = NULL;
		m_oAsyncLockMap.clear();
		m_fReleaseAsyncLockObsvr = NULL;
	}
	
	~AsyncLockMgr()
	{
		SAFE_DEL(m_pAsyncLockTimoutQ);
	}

	bool Init( int iMaxLockNum, fReleaseAsyncLockObsvr_t fReleaseAsyncLockObsvr );

	// find lock不存在, 直接执行且AddLock. 若存在, 则Block
	AsyncLock* FindLock( uint64_t ullLockID );
	// 把锁加入m_oAsyncLockMap, 表明该锁在使用了
	AsyncLock* AddLock( uint64_t ullLockID, int iTimeoutSec = ASYNCLOCK_DFT_TIMEOUT );

	// 把LockObsvr阻塞在该lock的等待队列上	
	void Block( AsyncLock* pAsyncLock, AsyncLockObsvr* poLockObsvr );
	void UnBlock( uint64_t ullLockID, void* pData );

	void Update();

	void ReleaseAsyncLockObsvr( AsyncLockObsvr* pObsvr )
	{
		m_fReleaseAsyncLockObsvr( pObsvr );
	}

private:
	void _OnLockTimeout( AsyncLock* pAsyncLock );
	void _DelLock( AsyncLock* pAsyncLock );

private:	
	IndexedPriorityQ< AsyncLock*, uint64_t /*lock id*/ >* m_pAsyncLockTimoutQ;	 // lock超时队列
	AsyncLockMap_t m_oAsyncLockMap;
	CMemPool<AsyncLock> m_oAsyncLockPool;
	fReleaseAsyncLockObsvr_t m_fReleaseAsyncLockObsvr;
};


