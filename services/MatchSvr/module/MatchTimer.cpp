#include "MatchTimer.h"
#include "LogMacros.h"
#include "MatchInfo.h"
#include "MatchMgr.h"
#include "MatchBase.h"
#include "GameTime.h"

using namespace PKGMETA;

MatchTimer::MatchTimer()
{
    _Construct();
}

MatchTimer::~MatchTimer()
{

}

void MatchTimer::Clear()
{
    _Construct();
    GameTimer::Clear();
}

void MatchTimer::_Construct()
{

    m_poInfo = NULL;
    m_pstMgr = NULL;
}

void MatchTimer::AttachParam( MatchInfo* poInfo, MatchBase* pstMgr)
{
    m_pstMgr = pstMgr;
    m_poInfo = poInfo;
}

void MatchTimer::OnTimeout()
{
    LOGRUN("timeout timer id-%u", this->GetObjID());
    if (!m_poInfo || m_poInfo->m_bIsMatched)
    {
        this->m_iFireCount = this->m_iMaxFireCount;
        return;
    }
    m_poInfo->m_wWaitCount++;
    m_pstMgr->MatchStartTimer(*m_poInfo, m_poInfo->m_wWaitCount);

    return;
}

