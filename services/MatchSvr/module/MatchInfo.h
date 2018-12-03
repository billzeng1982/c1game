#pragma once

#include "define.h"
#include "object.h"
#include "singleton.h"
#include "common_proto.h"
#include "MatchTimer.h"
#include <sys/time.h>
#include "../gamedata/GameDataMgr.h"

class MatchInfo : public IObject
{
public:
    MatchInfo();
    virtual ~MatchInfo();

    virtual void Clear();
    void _Construct();

    PKGMETA::DT_FIGHT_PLAYER_INFO m_stPlayerInfo;

    time_t m_ulFakeMatchWaitTimeSec;
    uint16_t m_wWaitCount;
    MatchTimer* m_pTimer;
    RESMATCH* m_pResMatch;
    bool m_bIsMatched;
};

