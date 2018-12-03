#pragma once
#include "CloneBattleTeam.h"
#include <set>


class CloneBattleMatch
{
public:
    struct MatchNode
    {
        CloneBattleTeam* poTeamInfo;
        uint32_t m_dwValue;
        uint64_t m_ullId;
        bool operator < (const MatchNode& l) const
        {
            return (m_dwValue < l.m_dwValue) || (m_dwValue == l.m_dwValue && m_ullId < l.m_ullId);
        }
    };
    
public:
    CloneBattleTeam* Match(uint32_t dwValue);
    void Clear();
    void Insert(CloneBattleTeam* poTeamInfo);
    void Delete(CloneBattleTeam* poTeamInfo);

private:
    std::set<MatchNode> m_MatchSet;
    MatchNode m_stTempNode;
    std::set<MatchNode>::const_iterator m_MatchSetIter;
};

