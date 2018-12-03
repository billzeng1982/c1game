#include "MsgLayerBase_r.h"

IMsgBase_r* MsgLayerBase_r::GetServerMsgHandler( int iMsgId )
{
	MsgHandlerMap_r_t::iterator it = m_oSsMsgHandlerMap.find( iMsgId );
	return ( it == m_oSsMsgHandlerMap.end()) ? NULL : it->second;
}
