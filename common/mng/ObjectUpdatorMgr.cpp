#include "ObjectUpdatorMgr.h"


ObjectUpdator* ObjectUpdatorMgr::RegisterObjectUpdator( unsigned short wObjType )
{
    return this->RegisterObjectUpdator( wObjType, DFT_IDLE_UPT_COUNT, DFT_BUSY_UPT_COUNT, DFT_UPDATE_FREQ );
}

ObjectUpdator* ObjectUpdatorMgr::RegisterObjectUpdator( unsigned short wObjType, int iIdleUptCount, int iBusyUptCount, int iUptFreq )
{
    ObjectUpdatorMap_t::iterator it = m_ObjectUpdatorMap.find( wObjType );
    if( it != m_ObjectUpdatorMap.end() )
    {
        return it->second;
    }

    ObjectUpdator* poUpdator = new ObjectUpdator();
    poUpdator->m_wObjType = wObjType;
    poUpdator->m_iIdleUptCount = iIdleUptCount;
    poUpdator->m_iBusyUptCount = iBusyUptCount;
    poUpdator->m_iUptFreq = iUptFreq;

    m_ObjectUpdatorMap.insert( ObjectUpdatorMap_t::value_type( wObjType, poUpdator ) );

    return poUpdator;
}

// 把object添加进update流程
void ObjectUpdatorMgr::Schedule( IObject* pObj )
{
    if( pObj->GetObjID() == 0 )
    {
        assert( false );
        return;
    }
    
    ObjectUpdatorMap_t::iterator it = m_ObjectUpdatorMap.find( pObj->GetObjType() );
    if( it == m_ObjectUpdatorMap.end() )
    {
        assert( false );
        return;
    }

    it->second->Schedule( pObj );
}

void ObjectUpdatorMgr::Unschedule( IObject* pObj )
{
    if( list_node_empty(&pObj->m_stUptNode) )
    {
        assert(false);
        return;
    }

    ObjectUpdatorMap_t::iterator it = m_ObjectUpdatorMap.find( pObj->GetObjType() );
    if( it == m_ObjectUpdatorMap.end() )
    {
        assert( false );
        return;
    }

    it->second->UnSchedule( pObj );
}


void ObjectUpdatorMgr::Update( bool bIdle )
{
    ObjectUpdatorMap_t::iterator it = m_ObjectUpdatorMap.begin();
    for( ; it != m_ObjectUpdatorMap.end(); it++ )
    {
        ObjectUpdator* poUpdator = it->second;
        poUpdator->Update(bIdle);
    }
}

ObjectUpdator* ObjectUpdatorMgr::GetObjUpdatorPtr(unsigned int dwObjType)
{
    ObjectUpdatorMap_t::iterator it = m_ObjectUpdatorMap.find(dwObjType);
    if( it == m_ObjectUpdatorMap.end() )
    {
        return NULL;
    }

    return it->second;
}


