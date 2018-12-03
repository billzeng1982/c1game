#include "UDPTimer.h"
#include "UDPBlockPool.h"
#include <math.h>
#include "UDPBlock.h"
#include "../ConnUtil.h"
#include "LogMacros.h"
#include "CommBusLayer.h"
#include "CEpoll.h"

#define RTO_GROW_RATE 1.4f

UDPTimer::UDPTimer()
{
	_Construct();
}

UDPTimer::~UDPTimer()
{

}

void UDPTimer::Clear()
{
    _Construct();

	GameTimer::Clear();
}

void UDPTimer::_Construct()
{
    m_pBlock = NULL;
}

void UDPTimer::AttachParam(UDPBlock* pBlock, int iMaxResendTime)
{
    m_pBlock = pBlock;
    m_iMaxResendTime = iMaxResendTime;
}

void UDPTimer::OnTimeout()
{
    // 定时重传
	SClient* pClient = m_pBlock->m_pClient;

	assert(pClient != NULL);

    struct timeval tSendTime = *(CGameTime::Instance().GetCurrTime());
    m_pBlock->m_tBlockTimeStamp = tSendTime;

    int iRet = pClient->m_oSocketUDP.Send(m_pBlock->m_pszBuff, SCPKG_LEN(m_pBlock->m_pszBuff), pClient->m_dwIP, pClient->m_wPort);

    m_pBlock->m_bIsResend = true;

    pClient->m_wRTO = uint16_t((float)pClient->m_wRTO * RTO_GROW_RATE);

    if (pClient->m_wRTO > m_iMaxResendTime)
    {
        pClient->m_wRTO = m_iMaxResendTime;
    }

    if(pClient->m_wRTO>0)
    {
        this->m_dwPeriodMs = pClient->m_wRTO;
    }else
    {
        assert(false);
    }

    LOGRUN("resend client sid-%u, seq-%u, msid-%d, timer firecount-%d, RTO-%d, Send iRet = %d",
        pClient->m_dwSessionId, SCPKG_SEQNUM(m_pBlock->m_pszBuff), SCPKG_MSGID(m_pBlock->m_pszBuff), this->GetFireCount(), pClient->m_wRTO, iRet);
}

