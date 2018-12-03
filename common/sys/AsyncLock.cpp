#include "AsyncLock.h"
#include "AsyncLockMgr.h"
#include <assert.h>
#include "GameTime.h"

AsyncLock::AsyncLock()
{
    m_ullLockID = 0;
    bzero( &m_stTimeoutTime, sizeof(m_stTimeoutTime) );
    m_poAsyncLockMgr = NULL;
    INIT_LIST_HEAD(&m_stWaitList);
}


void AsyncLock::Reset()
{ 
    // release observers
    list_head* pHead = &m_stWaitList;
    list_head* pCurr = NULL;
    list_head* pNext = NULL;
    list_for_each_safe( pCurr, pNext, pHead )
    {
        AsyncLockObsvr* pObsvr = (AsyncLockObsvr *)list_entry( pCurr, AsyncLockObsvr, m_stWaitListNode);
        list_del( pCurr );
        
        // do release
        m_poAsyncLockMgr->ReleaseAsyncLockObsvr( pObsvr );
    }

    m_ullLockID = 0;
    bzero( &m_stTimeoutTime, sizeof(m_stTimeoutTime) );
    m_poAsyncLockMgr = NULL;
}


void AsyncLock::Init( uint64_t ullLockID, int iTimeoutSec, AsyncLockMgr* poAsyncLockMgr )
{
    m_ullLockID = ullLockID;
    m_poAsyncLockMgr = poAsyncLockMgr;

    m_stTimeoutTime = *(CGameTime::Instance().GetCurrTime());
    m_stTimeoutTime.tv_sec += iTimeoutSec;
}

// ×èÈû
void AsyncLock::Block( AsyncLockObsvr* pObsvr )
{
    if( !pObsvr )
    {
        assert( false );
        return;
    }

    list_add_tail( &(pObsvr->m_stWaitListNode), &m_stWaitList );
}

void AsyncLock::UnBlock( void* pData )
{
    list_head* pHead = &m_stWaitList;
    list_head* pCurr = NULL;
    list_head* pNext = NULL;
    list_for_each_safe( pCurr, pNext, pHead )
    {
        AsyncLockObsvr* pObsvr = (AsyncLockObsvr *)list_entry( pCurr, AsyncLockObsvr, m_stWaitListNode);
        pObsvr->OnUnlock( pData );
    }
}

void AsyncLock::OnTimeout()
{
    list_head* pHead = &m_stWaitList;
    list_head* pCurr = NULL;
    list_head* pNext = NULL;
    list_for_each_safe( pCurr, pNext, pHead )
    {
        AsyncLockObsvr* pObsvr = (AsyncLockObsvr *)list_entry( pCurr, AsyncLockObsvr, m_stWaitListNode);
        pObsvr->OnTimeout();
    }
}

