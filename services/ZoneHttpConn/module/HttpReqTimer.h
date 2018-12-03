#pragma once

#ifndef HTTP_REQ_TIMER_H_
#define HTTP_REQ_TIMER_H_

#include "mng/GameTimer.h"
#include "http/HttpServer.h"
#include "protocol/PKGMETA/ss_proto.h"

class HttpReqTimer : public GameTimer
{
public:
    HttpReqTimer();
    ~HttpReqTimer();

    void Clear();
    void AttatchParam(uint32_t dwReqId, uint64_t ullMsgId);
    void OnTimeout();

private:
    void _Construct();

private:
    uint32_t m_dwReqId;
    uint64_t m_ullMsgId;
    PKGMETA::SSPKG m_stSsPkg;
};

#endif
