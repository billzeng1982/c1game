#include "object.h"
#include "ObjectUpdatorMgr.h"


// ����,ע�ⲻ������(����)
void IObject::Clear()
{	
	if( !list_node_empty(&m_stUptNode) )	
	{
		ObjectUpdatorMgr::Instance().Unschedule( this );		
	}

    m_dwID = 0;
	// type, m_wMemBytes ����Ҫ��0
}
