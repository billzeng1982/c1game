#include "HttpReqTimer.h"
#include "framework/ZoneHttpConnMsgLayer.h"

HttpReqTimer::HttpReqTimer()
{
    _Construct();
}

HttpReqTimer::~HttpReqTimer()
{
}

void HttpReqTimer::Clear()
{
    this->_Construct();
    GameTimer::Clear();
}

void HttpReqTimer::AttatchParam(uint32_t dwReqId, uint64_t ullMsgId)
{
    m_dwReqId = dwReqId;
    m_ullMsgId = ullMsgId;
}

void HttpReqTimer::OnTimeout()
{
    m_stSsPkg.m_stHead.m_wMsgId = m_ullMsgId + 1;
    m_stSsPkg.m_stHead.m_ullReservId = m_dwReqId;
    ZoneHttpConnMsgLayer::Instance().GetHttpControler().SendTimeoutRsp(m_stSsPkg);
    ZoneHttpConnMsgLayer::Instance().GetHttpControler().DeleteReq(m_dwReqId);
}

void HttpReqTimer::_Construct()
{
    m_dwReqId = 0;
    m_ullMsgId = 0;
}
