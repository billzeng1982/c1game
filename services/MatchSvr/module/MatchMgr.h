#pragma once

#include "define.h"
#include "singleton.h"
#include "common_proto.h"
#include "MatchBase.h"

using namespace PKGMETA;

class Match6v6Mgr : public MatchBase, public TSingleton<Match6v6Mgr>
{
public:
    Match6v6Mgr()
    {
        m_bMatchType = MATCH_TYPE_6V6;
    }
    virtual ~Match6v6Mgr(){}
};

class MatchWeekMgr : public MatchBase, public TSingleton<MatchWeekMgr>
{
public:
    MatchWeekMgr()
    {
        m_bMatchType = MATCH_TYPE_WEEKEND_LEAGUE;
    }
    virtual ~MatchWeekMgr(){}

};

class MatchLeisureMgr : public MatchBase, public TSingleton<MatchLeisureMgr>
{
public:
    MatchLeisureMgr()
    {
        m_bMatchType = MATCH_TYPE_LEISURE;
    }
    virtual ~MatchLeisureMgr(){}

};

class MatchDailyChallengeMgr : public MatchBase, public TSingleton<MatchDailyChallengeMgr>
{
public:
    MatchDailyChallengeMgr()
    {
        m_bMatchType = MATCH_TYPE_DAILY_CHALLENGE;
    }
    virtual ~MatchDailyChallengeMgr() {}
};

class MatchPeakArenaMgr : public MatchBase, public TSingleton<MatchPeakArenaMgr>
{
public:
    MatchPeakArenaMgr()
    {
        m_bMatchType = MATCH_TYPE_PEAK_ARENA;
    }
    virtual ~MatchPeakArenaMgr() {}
};