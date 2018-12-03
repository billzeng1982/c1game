#pragma once

/*
	服务器异步访问琐，比如多个请求请求同一数据，在请求端使用异步锁，防止数据覆盖等问题
*/

#include "define.h"
#include "list.h"
#include <time.h>

class AsyncLock;
class AsyncLockMgr;


// 阻塞在lock上的请求包装成observer, 解锁时触发, 具体逻辑由子类实现
class AsyncLockObsvr
{
	friend class AsyncLock;

public:
	AsyncLockObsvr() 
	{
		INIT_LIST_NODE(&m_stWaitListNode);
	}
	
	virtual ~AsyncLockObsvr() {}
	virtual void OnUnlock( void* pData ) = 0;
	virtual void OnTimeout() {}

	virtual void Reset()
	{
		INIT_LIST_NODE(&m_stWaitListNode);
	}

protected:
	struct list_head m_stWaitListNode;
};


class AsyncLock
{
public:
	AsyncLock();
	~AsyncLock(){}

	void Reset();

	// 阻塞
	void Block( AsyncLockObsvr* pObsvr );
	// 唤醒
	void UnBlock( void* pData );
	// 超时
	void OnTimeout();

	void Init( uint64_t ullLockID, int iTimeoutSec, AsyncLockMgr* poAsyncLockMgr );

	struct timeval* GetTimeoutTime() { return &m_stTimeoutTime; }
	uint64_t GetLockID() { return m_ullLockID; }

private:
	uint64_t m_ullLockID;
	struct list_head m_stWaitList; // 等待解锁队列
	struct timeval   m_stTimeoutTime; // 超时时间
	AsyncLockMgr*    m_poAsyncLockMgr;
};

