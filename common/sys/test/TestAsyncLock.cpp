#include <stdio.h>
#include <stdlib.h>
#include "AsyncLockMgr.h"
#include "pal/tstring.h"

namespace TestAsyncLock
{
    void ReleaseAsyncLockObsvr( AsyncLockObsvr* pObsvr)
    {
        printf("ReleaseAsyncLockObsvr\n");
        pObsvr->Reset();
        delete pObsvr;
    }


    class AddFriendObsvr : public AsyncLockObsvr
    {
    public:
        AddFriendObsvr() { m_szFriendName[0] = '\0'; }     
        virtual ~AddFriendObsvr() {}

        virtual void OnUnlock( void* pData )
        {
            STRNCPY(m_szFriendName, (char*)pData, 32);
            printf("Friend Name: %s\n", m_szFriendName);
        }
        
        virtual void OnTimeout()
        {
            printf( "AddFriendObsvr: timeout !! \n" );
        }

    	virtual void Reset()
    	{
    	    printf("AddFriendObsvr::Reset() called\n");
            m_szFriendName[0] = '\0';
    		AsyncLockObsvr::Reset();
    	}
    
    private:
        char m_szFriendName[32];
    };
};

int main()
{
    AsyncLockMgr oAysncLockMgr;

    if( !oAysncLockMgr.Init( 10 , TestAsyncLock::ReleaseAsyncLockObsvr ) )
    {
        printf("init failed!\n");
        return 0;
    }

    uint64_t ullUin = 1234;
    AsyncLock* pLock =  oAysncLockMgr.AddLock( ullUin, 2 );
    if( !pLock )
    {
        assert( false );
        return 0;
    }

    TestAsyncLock::AddFriendObsvr* pObsvr = new TestAsyncLock::AddFriendObsvr();
    oAysncLockMgr.Block(pLock, pObsvr);

    sleep(1);

    char szFriendName[32];
    snprintf( szFriendName, sizeof(szFriendName), "%s", "billzeng" );
    oAysncLockMgr.UnBlock( ullUin, szFriendName );


    // timeout test
    printf("==================== timeout test ====================\n");

    CGameTime::Instance().UpdateTime();
    
    pLock =  oAysncLockMgr.AddLock( ullUin, 2 );
    if( !pLock )
    {
        assert( false );
        return 0;
    }
    
    pObsvr = new TestAsyncLock::AddFriendObsvr();
    oAysncLockMgr.Block(pLock, pObsvr);

    sleep(3);

    CGameTime::Instance().UpdateTime();
    oAysncLockMgr.Update();

    oAysncLockMgr.UnBlock( ullUin, szFriendName ); // should not triggered

    return 0;
}


