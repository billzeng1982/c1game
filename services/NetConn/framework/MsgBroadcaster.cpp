#include "MsgBroadcaster.h"
#include <assert.h>
#include "LogMacros.h"
#include "../TCP/ConnTCP.h"
#include "../ConnUtil.h"

bool MsgBroadcaster::Init( CConnTCP* poConnTcp )
{
    assert( poConnTcp );

    m_poConnTcp = poConnTcp;
    CMemPool<SClient>* poClientPool = poConnTcp->GetClientPool();

    poClientPool->RegisterSlicedIter( &m_oClientIter );

    if( !m_oMsgQueue.Init( BROADCAST_MSG_BUF_SIZE ) )
    {
        LOGERR( "Init msg queue failed!" );
        return false;
    }

    return true;
}

void MsgBroadcaster::Update( bool bIdle )
{
    if( m_oMsgQueue.IsEmpty() )
    {
        return;
    }

    int iUptNum = bIdle ? 30 : 10;

    if ( m_oClientIter.IsEnd() )
    {
        m_oClientIter.Begin();
    }

    char* pszScPkg = NULL;
    uint32_t dwScPkgLen = 0;
    int iRet = m_oMsgQueue.PeekMsg( &pszScPkg, &dwScPkgLen);
    if( iRet != THREAD_Q_SUCESS )
    {
        LOGERR("PeekMsg failed! ret <%d>", iRet);
        m_oMsgQueue.PopMsg();
        m_oClientIter.Begin();
        return;
    }

    SClient* pstClient = NULL;
    for (int i = 0; i < iUptNum && !m_oClientIter.IsEnd(); i++, m_oClientIter.Next())
    {
        pstClient = m_oClientIter.CurrItem();
        if (NULL == pstClient)
        {
            assert( false );
            continue;
        }

        if (pstClient->m_iState == CLIENT_STATE_INGAME)
        {
            m_poConnTcp->SendToClient( pstClient, pszScPkg );
        }
    }

    if( m_oClientIter.IsEnd() )
    {
        m_oMsgQueue.PopMsg();
    }
}

bool MsgBroadcaster::AddMsg( char* pMsgBuf, uint32_t dwMsgLen )
{
    int iRet = m_oMsgQueue.WriteMsg( pMsgBuf, dwMsgLen );
    if( iRet != THREAD_Q_SUCESS )
    {
        LOGERR("Add broadcast msg failed! ret <%d>", iRet);
        return false;
    }

    return true;
}


