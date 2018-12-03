#include "CoGetDataTransMgr.h"

uint32_t CoGetDataTransMgr::_GetNextTransID()
{
    m_dwTransIDSeq++;
    if( 0 == m_dwTransIDSeq )
    {
        m_dwTransIDSeq = 1;
    }
    return m_dwTransIDSeq;
}

bool CoGetDataTransMgr::Init( int iMaxTrans )
{
    if( iMaxTrans <= 0 )
    {
        iMaxTrans = DEFAULT_MAX_TRANS_NUM;
    }

    if( m_oTransPool.CreatePool( iMaxTrans ) < 0 )
    {
        return false;
    }

    return true;
}

CoGetDataTrans* CoGetDataTransMgr::New()
{
    CoGetDataTrans* poTrans = m_oTransPool.NewData();
    if( !poTrans )
    {
        LOGERR("Get new CoGetDataTrans failed!");
        return NULL;
    }

    poTrans->SetTransID( this->_GetNextTransID() );
    m_oId2TransMap.insert( Id2TransMap_t::value_type( poTrans->GetTransID(), poTrans ) );

    return poTrans;
}

void CoGetDataTransMgr::Release( CoGetDataTrans* poTrans )
{
    if( !poTrans )
    {
        return;
    }

    Id2TransMap_t::iterator it = m_oId2TransMap.find( poTrans->GetTransID() );
    if( it != m_oId2TransMap.end() )
    {
        m_oId2TransMap.erase( it );
    }

    poTrans->Clear();

    m_oTransPool.DeleteData( poTrans );

    //LOGRUN("After release, trans pool has <%d> free nodes", m_oTransPool.GetFreeNum() );
}

CoGetDataTrans* CoGetDataTransMgr::Find( uint32_t dwTransID )
{
    Id2TransMap_t::iterator it = m_oId2TransMap.find( dwTransID );
    if( it != m_oId2TransMap.end() )
    {
        return it->second;
    }
    return NULL;
}

