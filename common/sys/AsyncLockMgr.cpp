#include "AsyncLockMgr.h"
#include "LogMacros.h"


bool AsyncLockMgr::Init( int iMaxLockNum, fReleaseAsyncLockObsvr_t fReleaseAsyncLockObsvr )
{
    assert( fReleaseAsyncLockObsvr != NULL );

    if( 0 == iMaxLockNum )
    {
        iMaxLockNum = DEFAULT_LOCK_NUM;
    }

    m_fReleaseAsyncLockObsvr = fReleaseAsyncLockObsvr;

    if( m_oAsyncLockPool.CreatePool( iMaxLockNum ) < 0 )
    {
        LOGERR("Create async lock pool failed!");
        return false;
    }

    m_pAsyncLockTimoutQ = new IndexedPriorityQ< AsyncLock*, uint64_t >( iMaxLockNum, new Comparer<AsyncLock*>(), new GetItemKey<AsyncLock*, uint64_t> );
    if( !m_pAsyncLockTimoutQ )
    {
        LOGERR( "create async lock timeout queue failed!" );
        return false;
    }

    return true;
}

AsyncLock* AsyncLockMgr::FindLock( uint64_t ullLockID )
{
    AsyncLockMap_t::iterator it = m_oAsyncLockMap.find(ullLockID);
    if( it == m_oAsyncLockMap.end() )
    {
        return NULL;
    }
    return it->second;
}

// 把锁加入m_oAsyncLockMap, 表明该锁在使用了
AsyncLock* AsyncLockMgr::AddLock( uint64_t ullLockID, int iTimeoutSec )
{
    if( 0 == ullLockID )
    {
        LOGERR("LockID == 0!");
        return NULL;
    }

    AsyncLock* poLock = this->FindLock(ullLockID);
    if( poLock )
    {  
        // already exists
        return poLock;
    }

    //new lock
    poLock = m_oAsyncLockPool.NewData( );
    if( !poLock )
    {
        LOGERR("Get new lock failed!");
        return NULL;
    }
    
    poLock->Init( ullLockID, iTimeoutSec, this );

    m_oAsyncLockMap.insert( AsyncLockMap_t::value_type(ullLockID, poLock) );

    m_pAsyncLockTimoutQ->Push( poLock );

    return poLock;
}

// 把LockObsvr阻塞在该lock的等待队列上	
void AsyncLockMgr::Block( AsyncLock* pAsyncLock, AsyncLockObsvr* poLockObsvr )
{
    if( NULL == pAsyncLock || NULL == poLockObsvr)
    {
        assert( false );
        return;
    }

    if( pAsyncLock->GetLockID() == 0 )
    {
        LOGERR("Invalid lock!");
        assert( false );
        return;
    }
          
    pAsyncLock->Block( poLockObsvr );
}

void AsyncLockMgr::UnBlock( uint64_t ullLockID, void* pData )
{
    AsyncLock* pAsyncLock = this->FindLock( ullLockID );

    if( !pAsyncLock )
    {
        LOGERR( "Lock id <%lu> doesn't exist!", ullLockID );
        return;
    }

    pAsyncLock->UnBlock(pData);
    
    this->_DelLock( pAsyncLock );
}

void AsyncLockMgr::Update()
{
    int iMaxUptCount = 10;
    int iUptCount = 0;

    // 检查lock是否超时
    while( !m_pAsyncLockTimoutQ->Empty() && iUptCount < iMaxUptCount )
    {
        AsyncLock* pAsyncLock = m_pAsyncLockTimoutQ->Top();
        if( TvBefore( pAsyncLock->GetTimeoutTime(), CGameTime::Instance().GetCurrTime() ) )
        {
            // timeout
            ++iUptCount;
            m_pAsyncLockTimoutQ->Pop();

            this->_OnLockTimeout( pAsyncLock );
        }
        else
        {
            break;
        }
    }    
}

void AsyncLockMgr::_OnLockTimeout( AsyncLock* pAsyncLock )
{
    if( !pAsyncLock )
    {
        return;
    }

    pAsyncLock->OnTimeout();

    this->_DelLock( pAsyncLock );
}

void AsyncLockMgr::_DelLock( AsyncLock* pAsyncLock )
{
    assert( pAsyncLock->GetLockID() > 0 );

    AsyncLockMap_t::iterator it = m_oAsyncLockMap.find( pAsyncLock->GetLockID() );
	if( it != m_oAsyncLockMap.end() ) 
    {
		m_oAsyncLockMap.erase(it);
	}

    m_pAsyncLockTimoutQ->Erase( pAsyncLock->GetLockID() );

    pAsyncLock->Reset();
    m_oAsyncLockPool.DeleteData(pAsyncLock);
}

