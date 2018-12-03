#include "UDPClientBuffIndex.h"

void UDPClientBuffIndex::Clear()
{
    this->_Construct();
}

void UDPClientBuffIndex::_Construct()
{
    m_oSendedMap.clear();
	m_oRecvMap.clear();
    m_ullTimestampMs = 0;
    m_oAckList.clear();
    
    return;
}

