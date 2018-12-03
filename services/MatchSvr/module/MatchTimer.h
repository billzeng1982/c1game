#pragma once

#include "GameTimer.h"
#include "mempool.h"
#include "common_proto.h"
#include "ss_proto.h"

class MatchInfo;
class MatchBase;
class MatchTimer : public GameTimer
{
public:
    MatchInfo* m_poInfo;
    MatchBase* m_pstMgr;

public:
    MatchTimer();
    virtual ~MatchTimer();

    virtual void Clear();

    void AttachParam(MatchInfo* poInfo, MatchBase* pstMgr);

protected:
    void _Construct();

public:
    virtual void OnTimeout();
};


