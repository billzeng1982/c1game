#include "ObjectUpdator.h"
#include "og_comm.h"
#include "GameTime.h"
#include <strings.h>

ObjectUpdator::ObjectUpdator()
{
    m_iIdleUptCount = 0;
    m_iBusyUptCount = 0;
    m_iUptFreq = 0;
    m_wObjType = 0;
    m_fUpdateCb = NULL;
    m_iObjNum = 0;
    bzero( &m_tvUptStartTime, sizeof(m_tvUptStartTime) );

    INIT_LIST_HEAD( &m_stUpdateList );
    m_pCurIter = &m_stUpdateList;
}

void ObjectUpdator::Update( bool bIdle )
{
    struct timeval tvCurTime = { 0 };

    if( 0 == m_iObjNum )
    {
        return;
    }

    tvCurTime = *(CGameTime::Instance().GetCurrTime());

	unsigned long timePass = MsPass(&tvCurTime, &m_tvUptStartTime);
    if( timePass < (unsigned long)m_iUptFreq )
    { 
        return;
    }

    int iUptCount = bIdle ? m_iIdleUptCount : m_iBusyUptCount;
    assert( iUptCount > 0 );

    if( this->_IsIterEnd() )
    {
        this->_IterBegin();
    }

    IObject* pObj = NULL;
    int i = 0;
    for( ; i < iUptCount && !this->_IsIterEnd(); i++, this->_IterNext() )
    {
        pObj = this->_IterCurr();

		if (m_fUpdateCb != NULL)
		{
			m_fUpdateCb( pObj, timePass );
		}
    }

    if( this->_IsIterEnd() )
    {
        m_tvUptStartTime = tvCurTime;
    }
}

void ObjectUpdator::Schedule( IObject* pObj )
{
    list_add_tail( &pObj->m_stUptNode, &m_stUpdateList);
    m_iObjNum++;
	if (m_iObjNum == 1)
	{
		// init UptStartTime
		m_tvUptStartTime = *(CGameTime::Instance().GetCurrTime());
	}
}

void ObjectUpdator::UnSchedule( IObject* pObj )
{
    if( &pObj->m_stUptNode == m_pCurIter )
    {
        m_pCurIter = pObj->m_stUptNode.next;
    }
    
    list_del( &pObj->m_stUptNode );
    --m_iObjNum;
    assert( m_iObjNum >= 0 );
}

unsigned long ObjectUpdator::GetMsPassInterval()
{
	return MsPass(CGameTime::Instance().GetCurrTime(), &m_tvUptStartTime);
}


