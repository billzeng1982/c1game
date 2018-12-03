#include "CloneBattleMatch.h"
#include "GameTime.h"
#include "LogMacros.h"

using namespace std;
CloneBattleTeam* CloneBattleMatch::Match(uint32_t dwValue)
{
    
    if (m_MatchSet.empty())
    {
        return NULL;
    }
    m_stTempNode.m_dwValue = dwValue;
    m_stTempNode.m_ullId = 1;

    //查找大于等于m_dwValue的节点
    m_MatchSetIter = m_MatchSet.lower_bound(m_stTempNode);

    if (m_MatchSetIter == m_MatchSet.end())
    {
        m_MatchSetIter--;
        return m_MatchSetIter->poTeamInfo;
    }
    std::set<MatchNode>::const_reverse_iterator m_ReverMatchSetIterLow(m_MatchSetIter);
    //m_MatchSetIterLow = m_MatchSetIter--;   //上一个节点,比目标值小
    //正向迭代器转为反向迭代器后,会指向正想迭代器的上一个元素,相当于执行了m_MatchSetIter--
    if (m_ReverMatchSetIterLow == m_MatchSet.rend())
    {
        return m_MatchSetIter->poTeamInfo;
    }

    if ((dwValue - m_ReverMatchSetIterLow->m_dwValue)  >  (m_MatchSetIter->m_dwValue - dwValue) )
    {
        return m_ReverMatchSetIterLow->poTeamInfo;
    }
    else
    {
        return m_MatchSetIter->poTeamInfo;
    }
}

void CloneBattleMatch::Clear()
{
    m_MatchSet.clear();
}

void CloneBattleMatch::Insert(CloneBattleTeam * poTeamInfo)
{
    if (poTeamInfo->GetAvrLi() == 0)
    {
        //战力为0不添加
        return;
    }
    m_stTempNode.m_dwValue = poTeamInfo->GetAvrLi();
    m_stTempNode.m_ullId = poTeamInfo->GetTeamId();
    m_stTempNode.poTeamInfo = poTeamInfo;
    m_MatchSet.insert(m_stTempNode);
    //LOGWARN("TeamId<%lu> Add in MatchList Type<%hhu>, Num<%u>", poTeamInfo->GetTeamId(), poTeamInfo->GetBossType(), m_MatchSet.size());
}

void CloneBattleMatch::Delete(CloneBattleTeam* poTeamInfo)
{
    m_stTempNode.m_dwValue = poTeamInfo->GetAvrLi();
    m_stTempNode.m_ullId = poTeamInfo->GetTeamId();
    m_stTempNode.poTeamInfo = poTeamInfo;
    m_MatchSetIter = m_MatchSet.find(m_stTempNode);
    if (m_MatchSetIter == m_MatchSet.end())
    {
        //LOGERR("Id<%lu> delete failed", poTeamInfo->GetTeamId());
        return;
    }
    m_MatchSet.erase(m_MatchSetIter);
    //LOGWARN("TeamId<%lu> Del in MatchList Type<%hhu>, Num<%u>", poTeamInfo->GetTeamId(), poTeamInfo->GetBossType(), m_MatchSet.size());
}

