#pragma once

/*
	�������첽����������������������ͬһ���ݣ��������ʹ���첽������ֹ���ݸ��ǵ�����
*/

#include "define.h"
#include "list.h"
#include <time.h>

class AsyncLock;
class AsyncLockMgr;


// ������lock�ϵ������װ��observer, ����ʱ����, �����߼�������ʵ��
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

	// ����
	void Block( AsyncLockObsvr* pObsvr );
	// ����
	void UnBlock( void* pData );
	// ��ʱ
	void OnTimeout();

	void Init( uint64_t ullLockID, int iTimeoutSec, AsyncLockMgr* poAsyncLockMgr );

	struct timeval* GetTimeoutTime() { return &m_stTimeoutTime; }
	uint64_t GetLockID() { return m_ullLockID; }

private:
	uint64_t m_ullLockID;
	struct list_head m_stWaitList; // �ȴ���������
	struct timeval   m_stTimeoutTime; // ��ʱʱ��
	AsyncLockMgr*    m_poAsyncLockMgr;
};

