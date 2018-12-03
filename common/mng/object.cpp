#include "object.h"
#include "ObjectUpdatorMgr.h"


// 清理,注意不是销毁(析构)
void IObject::Clear()
{	
	if( !list_node_empty(&m_stUptNode) )	
	{
		ObjectUpdatorMgr::Instance().Unschedule( this );		
	}

    m_dwID = 0;
	// type, m_wMemBytes 不需要归0
}
