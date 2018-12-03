#include "UDPBlock.h"

UDPBlock::UDPBlock()
{
    this->_Construct();
}

void UDPBlock::Clear()
{
    this->_Construct();
    IObject::Clear();
}

void UDPBlock::_Construct()
{
    m_pEpoll = NULL;
	m_pConn = NULL;
	m_pClient = NULL;
	
	m_dwTimerId = 0;
	m_pszBuff = NULL;
	m_iHnd = 0;
	m_iLen = 0;

    return;
}


